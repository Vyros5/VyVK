#include <VyEngine/GFX/Resources/Image/Decoder/KTXDecoder.h>

#include <ktx.h>
#include <ktxvulkan.h>

namespace Vy
{
    struct CopyEntry 
    { 
        ktx_size_t Offset; 
        ktx_size_t Size; 
        U32        FaceIndex; 
    };

    namespace 
    {
        /// <summary>
        /// check if a ktxTexture1 is valid as a 2D texture
        /// </summary>
        void validateKtx2D(ktxTexture1* pTexture)
        {
            if (pTexture->numFaces != 1)
            {
                ktxTexture_Destroy(ktxTexture(pTexture));
                VY_THROW_RUNTIME_ERROR("KTX decode error: file contains multiple faces (cubemap?)");
            }

            if (pTexture->numLayers > 1)
            {
                ktxTexture_Destroy(ktxTexture(pTexture));
                VY_THROW_RUNTIME_ERROR("KTX decode error: array textures not supported in this loader");
            }

            // Must be a 2D image
            if (pTexture->numDimensions != 2)
            {
                ktxTexture_Destroy(ktxTexture(pTexture));
                VY_THROW_RUNTIME_ERROR("KTX decode error: only 2D textures are supported");
            }
        }

        /// <summary>
        /// check if a ktxTexture1 is valid as a cubemap
        /// </summary>
        void validateKtxCubemap(ktxTexture1* pTexture)
        {
            if (!pTexture->isCubemap)
            {
                ktxTexture_Destroy(ktxTexture(pTexture));
                VY_THROW_RUNTIME_ERROR("KTX decode error: KTX file is not cubemap");
            }

            if (pTexture->numFaces != 6)
            {
                ktxTexture_Destroy(ktxTexture(pTexture));
                VY_THROW_RUNTIME_ERROR("KTX decode error: KTX texture is not a cubemap (numFaces != 6)");
            }

            if (pTexture->numLayers != 1)
            {
                ktxTexture_Destroy(ktxTexture(pTexture));
                VY_THROW_RUNTIME_ERROR("KTX decode error: KTX cubemap has invalid number of array layers (must be 1)");
            }
        }

        /// <summary>
        /// check if the resulting cubemap faces are compatible
        /// </summary>
        void validateCubemap(const VyDecodedCubemap& cube)
        {
            const auto& ref = cube.Faces[0];

            for (int i = 1; i < 6; i++)
            {
                const auto& f = cube.Faces[ i ];

                if (f.Width != ref.Width || f.Height != ref.Height)
                {
                    VY_THROW_RUNTIME_ERROR("KTX decode error: Cubemap faces must have identical dimensions.");
                }

                if (f.Format != ref.Format)
                {
                    VY_THROW_RUNTIME_ERROR("KTX decode error: Cubemap faces must have identical VkFormat.");
                }

                if (f.IsCompressed != ref.IsCompressed)
                {
                    VY_THROW_RUNTIME_ERROR("KTX decode error: Cubemap faces must have identical compression state.");
                }

                if (f.MipLevels != ref.MipLevels)
                {
                    VY_THROW_RUNTIME_ERROR("KTX decode error: Cubemap faces must have same number of mip levels.");
                }

                if (f.IsFloat != ref.IsFloat)
                {
                    VY_THROW_RUNTIME_ERROR("KTX decode error: Cubemap faces must be all-float or all-uint8.");
                }
            }
        }

        /// <summary>
        /// calculate image offset for given level/layer/face
        /// </summary>
        ktx_size_t calculateImageOffset(ktxTexture1* kTexture, const U32 level, const U32 layer, const U32 face) 
        {
            ktx_size_t offset = 0;
            KTX_error_code e = ktxTexture_GetImageOffset(ktxTexture(kTexture), level, layer, face, &offset);
            
            if (e != KTX_SUCCESS) 
            {
                ktxTexture_Destroy(ktxTexture(kTexture));
            
                VY_THROW_RUNTIME_ERROR("KTX2Decoder::decodeCubemap - failed to get image offset.");
            }

            return offset;
        }

        /// <summary>
        /// calculate image size for given level
        /// </summary>
        ktx_size_t calculateImageSize(ktxTexture1* kTexture, const U32 level) 
        {
            ktx_size_t size = ktxTexture_GetImageSize(ktxTexture(kTexture), level);
        
            if (size == 0) 
            {
                ktxTexture_Destroy(ktxTexture(kTexture));
            
                VY_THROW_RUNTIME_ERROR("KTX2Decoder::decodeCubemap - invalid image size.");
            }
            
            return size;
        }


        /// <summary>
        /// gather image copy entries for all levels/layers/faces
        /// </summary>
        TVector<CopyEntry> gatherImageCopyEntries(ktxTexture1* kTexture, const U32 levels, const U32 layers)
        {
            TVector<CopyEntry> entries;

            entries.reserve(levels * std::max<U32>(1, layers) * 6);

            for (U32 level = 0; level < levels; ++level)
            {
                for (U32 layer = 0; layer < std::max<U32>(1, layers); ++layer)
                {
                    for (U32 face = 0; face < 6; ++face)
                    {
                        ktx_size_t offset = calculateImageOffset(kTexture, level, layer, face);
                        ktx_size_t size   = calculateImageSize(kTexture, level);
                        
                        entries.push_back({ offset, size, face });
                    }
                }
            }

            if (entries.empty()) 
            {
                VY_THROW_RUNTIME_ERROR("KTXDecoder::decodeCubemap - no image entries found");
            }

            return entries;
        }

        /// <summary>
        /// load ktxTexture1 from file path
        /// </summary>
        ktxTexture1* getKtxTextureFromfile(const TString& path)
        {
            ktxTexture1* pTexture = nullptr;

            KTX_error_code result = ktxTexture1_CreateFromNamedFile(
                path.c_str(),
                KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT,// KTX_TEXTURE_CREATE_NO_FLAGS,
                &pTexture
            );

            if (result != KTX_SUCCESS || pTexture == nullptr) 
            {
                VY_THROW_RUNTIME_ERROR("KTX decode error: Failed to load texture: " + path);
            }

            return pTexture;
        }


        /// <summary>
        /// find defined texture Format in ktxTexture, throws if undefined
        /// </summary>
        VkFormat findVkFormat(ktxTexture1* pTexture)
        {
            VkFormat format = ktxTexture1_GetVkFormat(pTexture);

            if (format == VK_FORMAT_UNDEFINED)
            {
                ktxTexture_Destroy(ktxTexture(pTexture));

                VY_THROW_RUNTIME_ERROR("KTX decode error: unsupported texture format");
            }

            return format;
        }

        /// <summary>
        /// get offset of specific image in ktxTexture
        /// </summary>
        ktx_size_t getKtxImageOffset(ktxTexture1* pTexture, U32 level, U32 layer, U32 face)
        {
            ktx_size_t offset;

            KTX_error_code err = ktxTexture_GetImageOffset(ktxTexture(pTexture), level, layer, face, &offset);
            
            if (err != KTX_SUCCESS)
            {
                ktxTexture_Destroy(ktxTexture(pTexture));

                VY_THROW_RUNTIME_ERROR("KTX decode error: Failed to get KTX cubemap face offset");
            }

            return offset;
        }
    }

    /// <summary>
    /// Determines whether the provided file path has the KTX file extension
    /// </summary>
    /// <param name="path">file path or name to check</param>
    bool KTXDecoder::canDecode(const TPath& path) const
    {
        return imDecoder::getExtension(path.string()) == "ktx";
    }


    VyDecodedImage KTXDecoder::decode(const TPath& path) const
    {
        VyDecodedImage img{};

        // Load via KTX library
        ktxTexture1* pTexture = getKtxTextureFromfile(path.string());

        validateKtx2D(pTexture);

        // Basic info
        img.Width        = pTexture->baseWidth;
        img.Height       = pTexture->baseHeight;
        img.MipLevels    = pTexture->numLevels;
        img.IsCompressed = true;
        img.Format       = findVkFormat(pTexture);

        // ktxTexture_GetData returns a pointer to the raw block of texture data
        ktx_size_t totalSize = ktxTexture_GetDataSize(ktxTexture(pTexture));
        img.DataSize = static_cast<size_t>(totalSize);


        U8* data = (U8*)pTexture->pData;
        
        if (!data || totalSize == 0) 
        {
            ktxTexture_Destroy(ktxTexture(pTexture));
        
            VY_THROW_RUNTIME_ERROR("KTX decode error: Empty or invalid compressed dat");
        }

        img.CompressedData.resize(totalSize);
        std::memcpy(img.CompressedData.data(), data, totalSize);


        // get mipmaps sizes and offsets
        img.MipOffsets.resize(img.MipLevels);
        img.MipSizes  .resize(img.MipLevels);

        for (U32 level = 0; level < img.MipLevels; level++) 
        {
            ktx_size_t offset = getKtxImageOffset(pTexture, level, 0, 0);
            ktx_size_t size   = ktxTexture_GetImageSize(ktxTexture(pTexture), level);

            img.MipOffsets[ level ] = static_cast<U32>(offset);
            img.MipSizes  [ level ] = static_cast<U32>(size);
        }

        // Cleanup
        ktxTexture_Destroy(ktxTexture(pTexture));

        return img;
    }


    VyDecodedCubemap KTXDecoder::decodeCubemap(const TPath& path) const
    {
        // load image data
        ktxTexture1* kTexture = getKtxTextureFromfile(path.string());

        // Validate cubemap compatibility
        validateKtxCubemap(kTexture);

        const U32 baseW  = static_cast<U32>(kTexture->baseWidth);
        const U32 baseH  = static_cast<U32>(kTexture->baseHeight);
        const U32 levels = static_cast<U32>(kTexture->numLevels);
        const U32 layers = static_cast<U32>(kTexture->numLayers ? kTexture->numLayers : 1);
        VkFormat  format = static_cast<VkFormat>(ktxTexture1_GetVkFormat(kTexture));

        VyDecodedCubemap outCube;

        // initialize faces
        for (U32 face = 0; face < 6; ++face) 
        {
            VyDecodedImage& faceImg = outCube.Faces[face];

            faceImg.Width        = baseW;
            faceImg.Height       = baseH;
            faceImg.MipLevels    = levels;
            faceImg.Format       = format;
            faceImg.IsCompressed = true;
        }

        TVector<CopyEntry> entries = gatherImageCopyEntries(kTexture, levels, layers);
        for (const auto& e : entries) 
        {
            VyDecodedImage& faceImg = outCube.Faces[ e.FaceIndex ];

            faceImg.CompressedData.resize(faceImg.CompressedData.size() + e.Size);
        }


        const U8* srcBase = reinterpret_cast<const U8*>(ktxTexture_GetData(ktxTexture(kTexture)));
        if (srcBase == nullptr) 
        {
            VY_THROW_RUNTIME_ERROR("KTX decode error: ktx texture data pointer is null for: " + path.string());
        }

        TArray<size_t, 6> writeOffsets = { 0, 0, 0, 0, 0, 0 };

        for (const auto& e : entries) 
        {
            VyDecodedImage& faceImg   = outCube.Faces[ e.FaceIndex ];
            size_t          dstOffset = writeOffsets [ e.FaceIndex ];

            // Bounds check before copying
            if (dstOffset + static_cast<size_t>(e.Size) > faceImg.CompressedData.size()) 
            {
                VY_THROW_RUNTIME_ERROR("KTX decode error: write would overflow face buffer for :  " + path.string());
            }

            std::memcpy(faceImg.CompressedData.data() + dstOffset, srcBase + e.Offset, static_cast<size_t>(e.Size));

            faceImg.MipOffsets.push_back(static_cast<U32>(dstOffset));
            faceImg.MipSizes  .push_back(static_cast<U32>(e.Size));

            writeOffsets[ e.FaceIndex ] += static_cast<size_t>(e.Size);
        }


        for (U32 f = 0; f < 6; ++f) 
        {
            if (writeOffsets[f] != outCube.Faces[f].CompressedData.size()) 
            {
                VY_THROW_RUNTIME_ERROR("KTX decode error:  written size mismatch for face " + std::to_string(f));
            }
            
            outCube.Faces[f].DataSize = outCube.Faces[f].CompressedData.size();
        }

        ktxTexture_Destroy(ktxTexture(kTexture));

        validateCubemap(outCube);

        return outCube;
    }
}