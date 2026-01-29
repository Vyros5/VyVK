#include <VyEngine/GFX/Resources/Image/Decoder/KTX2Decoder.h>

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
        /// Maps the vkFormat field of a ktxTexture2 to a corresponding VkFormat
        /// </summary>
        /// <param name="pTexture">Pointer to a ktxTexture2 whose vkFormat value will be mapped to a Vulkan VkFormat.</param>
        /// <returns>The matching VkFormat for the texture's vkFormat when it is supported.</returns>
        VkFormat mapKtxFormat(ktxTexture2* pTexture) 
        {
            return static_cast<VkFormat>(pTexture->vkFormat);
        }


        /// <summary>
        /// check if a ktxTexture2 is valid as a cubemap
        /// </summary>
        void checkCubemapCompatibility(ktxTexture2* pTexture) 
        {
            if (!pTexture->isCubemap)
            {
                ktxTexture_Destroy(ktxTexture(pTexture));

                VY_THROW_RUNTIME_ERROR("KTX2 decode error: KTX file is not cubemap");
            }

            if (pTexture->numFaces != 6)
            {
                ktxTexture_Destroy(ktxTexture(pTexture));

                VY_THROW_RUNTIME_ERROR("KTX2 decode error: KTX texture is not a cubemap (numFaces != 6)");
            }

            if (pTexture->numLayers != 1)
            {
                ktxTexture_Destroy(ktxTexture(pTexture));

                VY_THROW_RUNTIME_ERROR("KTX2 decode error: KTX cubemap has invalid number of array layers (must be 1)");
            }
        }

        /// <summary>
        /// check if the resulting cubemap faces are compatible
        /// </summary>
        void validateResultCubemap(VyDecodedCubemap outCube) 
        {
            const VyDecodedImage& ref = outCube.Faces[0];

            for (int i = 1; i < 6; ++i) 
            {
                const VyDecodedImage& f = outCube.Faces[i];

                if (f.Width != ref.Width || f.Height != ref.Height)
                {
                    VY_THROW_RUNTIME_ERROR("KTX2 decode error: cubemap faces mismatch");
                }

                if (f.MipLevels != ref.MipLevels)
                {
                    VY_THROW_RUNTIME_ERROR("KTX2 decode error: mip level mismatch");
                }

                if (f.IsCompressed != ref.IsCompressed)
                {
                    VY_THROW_RUNTIME_ERROR("KTX2 decode error: compression mismatch");
                }
            }
        }

        /// <summary>
        /// calculate image offset for given level/layer/face
        /// </summary>
        ktx_size_t calculateImageOffset(ktxTexture2* pTexture, const U32 level, const U32 layer, const U32 face) 
        {
            ktx_size_t offset = 0;
        
            KTX_error_code e = ktxTexture_GetImageOffset(ktxTexture(pTexture), level, layer, face, &offset);
        
            if (e != KTX_SUCCESS) 
            {
                ktxTexture_Destroy(ktxTexture(pTexture));
            
                VY_THROW_RUNTIME_ERROR("KTX2 decode error: failed to get image offset");
            }

            return offset;
        }

        /// <summary>
        /// calculate image size for given level
        /// </summary>
        ktx_size_t calculateImageSize(ktxTexture2* pTexture, const U32 level) 
        {
            ktx_size_t size = ktxTexture_GetImageSize(ktxTexture(pTexture), level);
        
            if (size == 0) 
            {
                ktxTexture_Destroy(ktxTexture(pTexture));
            
                VY_THROW_RUNTIME_ERROR("KTX2 decode error: invalid image size");
            }
            
            return size;
        }

        /// <summary>
        /// gather image copy entries for all levels/layers/faces
        /// </summary>
        TVector<CopyEntry> gatherImageCopyEntries(ktxTexture2* pTexture, const U32 levels, const U32 layers)
        {
            TVector<CopyEntry> entries;

            entries.reserve(levels * std::max<U32>(1, layers) * 6);

            for (U32 level = 0; level < levels; ++level)
            {
                for (U32 layer = 0; layer < std::max<U32>(1, layers); ++layer)
                {
                    for (U32 face  = 0; face  < 6; ++face)
                    {
                        ktx_size_t offset = calculateImageOffset(pTexture, level, layer, face);
                        ktx_size_t size   = calculateImageSize(pTexture, level);
                        
                        entries.push_back({ offset, size, face });
                    }
                }
            }

            if (entries.empty()) 
            {
                VY_THROW_RUNTIME_ERROR("KTX2 decode error: no image entries found");
            }

            return entries;
        }

        ktxTexture2* getKtxTextureFromfile(const TString& path)
        {
            ktxTexture2* pTexture = nullptr;

            KTX_error_code result = ktxTexture2_CreateFromNamedFile(
                path.c_str(),
                KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT,
                &pTexture
            );

            if (result != KTX_SUCCESS || pTexture == nullptr) 
            {
                VY_THROW_RUNTIME_ERROR("KTX2 decode error: Failed to load texture: " + path);
            }

            return pTexture;
        }

        VkFormat transcodeBasisFormat(ktxTexture2* pTexture)
        {
            if (!ktxTexture2_NeedsTranscoding(pTexture)) 
            {
                VY_THROW_RUNTIME_ERROR("KTX2 decode error: KTX2 has no vkFormat and no Basis data — cannot decode ");
            }

            // Transcode using Basis Universal inside KTX2
            KTX_error_code tc = ktxTexture2_TranscodeBasis(
                pTexture,
                KTX_TTF_BC7_RGBA,     // Desktop default
                KTX_TF_HIGH_QUALITY   // optional
            );

            if (tc != KTX_SUCCESS) 
            {
                VY_THROW_RUNTIME_ERROR("KTX2 transcoding failed: " + TString(ktxErrorString(tc)));
            }

            return VK_FORMAT_BC7_UNORM_BLOCK;
        }
    }

    /// <summary>
    /// Determines whether the provided file path has the KTX2 file extension
    /// </summary>
    /// <param name="path">file path or name to check</param>
    bool KTX2Decoder::canDecode(const TPath& path) const
    {
        return imDecoder::getExtension(path.string()) == "ktx2";
    }

    /// <summary>
    /// Loads a KTX2 texture file using the KTX library and returns a VyDecodedImage containing its metadata and compressed data
    /// </summary>
    /// <param name="path">Path to the KTX2 file to decode</param>
    /// <returns>VyDecodedImage with metadata </returns>
    VyDecodedImage KTX2Decoder::decode(const TPath& path) const
    {
        VyDecodedImage img{};
        img.IsCompressed = true;

        // Load via KTX library
        ktxTexture2* pTexture = getKtxTextureFromfile(path.string());

        // Basic info
        img.Width     = pTexture->baseWidth;
        img.Height    = pTexture->baseHeight;
        img.MipLevels = pTexture->numLevels;
        

        // If the KTX2 contains Basis supercompressed data, we must transcode it first
        if (ktxTexture2_NeedsTranscoding(pTexture))
        {
            transcodeBasisFormat(pTexture);
        }

        img.Format = mapKtxFormat(pTexture);

        // Extract compressed data for the full texture 
        ktx_size_t totalSize = ktxTexture_GetDataSize(ktxTexture(pTexture));
        img.DataSize = static_cast<size_t>(totalSize);

        // copy data
        U8* pData = (U8*)pTexture->pData;

        if (!pData || totalSize == 0) 
        {
            ktxTexture_Destroy(ktxTexture(pTexture));
        
            VY_THROW_RUNTIME_ERROR("KTX2 decode error: Empty or invalid compressed data");
        }

        img.CompressedData.resize(totalSize);
        std::memcpy(img.CompressedData.data(), pData, totalSize);

        
        // get mipmaps sizes and offsets
        img.MipOffsets.resize(img.MipLevels);
        img.MipSizes  .resize(img.MipLevels);

        for (U32 level = 0; level < img.MipLevels; level++) 
        {    
            ktx_size_t offset = calculateImageOffset(pTexture, level, 0, 0);
            ktx_size_t size   = ktxTexture_GetImageSize(ktxTexture(pTexture), level);

            img.MipOffsets[ level ] = static_cast<U32>(offset);
            img.MipSizes  [ level ] = static_cast<U32>(size);
        }

        // Cleanup
        ktxTexture_Destroy(ktxTexture(pTexture));

        return img;
    }



    VyDecodedCubemap KTX2Decoder::decodeCubemap(const std::filesystem::path& path) const
    {
        // load image data
        ktxTexture2* pTexture = getKtxTextureFromfile(path.string());

        // Validate cubemap compatibility
        checkCubemapCompatibility(pTexture);

        // base dims & mips
        const U32 baseW   = static_cast<U32>(pTexture->baseWidth);
        const U32 baseH   = static_cast<U32>(pTexture->baseHeight);
        const U32 levels  = static_cast<U32>(pTexture->numLevels);
        const U32 layers  = static_cast<U32>(pTexture->numLayers ? pTexture->numLayers : 1);
        VkFormat  format  = static_cast<VkFormat>(pTexture->vkFormat);
        
        if (format == VK_FORMAT_UNDEFINED)
        {
            format = transcodeBasisFormat(pTexture);
        }

        VyDecodedCubemap outCube;

        // initialize faces
        for (U32 face = 0; face < 6; ++face) 
        {
            VyDecodedImage& faceImg = outCube.Faces[face];

            faceImg.Width        = baseW;
            faceImg.Height       = baseH;
            faceImg.MipLevels    = levels;
            faceImg.IsCompressed = true;
            faceImg.Format       = format;
        }

        TVector<CopyEntry> entries = gatherImageCopyEntries(pTexture, levels, layers);
        
        for (const auto& e : entries) 
        {
            VyDecodedImage& faceImg = outCube.Faces[ e.FaceIndex ];

            faceImg.CompressedData.resize(faceImg.CompressedData.size() + e.Size);
        }

        const U8* pSrcBase = reinterpret_cast<const U8*>(ktxTexture_GetData(ktxTexture(pTexture)));
        
        if (pSrcBase == nullptr) 
        {
            VY_THROW_RUNTIME_ERROR("KTX2 decode error: ktx texture data pointer is null for: " + path.string());
        }

        TArray<size_t, 6> writeOffsets = { 0, 0, 0, 0, 0, 0 };

        for (const auto& e : entries) 
        {
            VyDecodedImage& faceImg   = outCube.Faces[ e.FaceIndex ];
            size_t          dstOffset = writeOffsets [ e.FaceIndex ];

            // Bounds check before copying
            if (dstOffset + static_cast<size_t>(e.Size) > faceImg.CompressedData.size()) 
            {
                VY_THROW_RUNTIME_ERROR("KTX2 decode error: write would overflow face buffer for: " + path.string());
            }

            std::memcpy(faceImg.CompressedData.data() + dstOffset, pSrcBase + e.Offset, static_cast<size_t>(e.Size));

            //faceImg.DataSize = static_cast<size_t>(e.Size);
            faceImg.MipOffsets.push_back(static_cast<U32>(dstOffset));
            faceImg.MipSizes  .push_back(static_cast<U32>(e.Size));

            writeOffsets[ e.FaceIndex ] += static_cast<size_t>(e.Size);
        }


        for (U32 f = 0; f < 6; ++f) 
        {
            if (writeOffsets[f] != outCube.Faces[f].CompressedData.size()) 
            {
                VY_THROW_RUNTIME_ERROR("KTX2 decode error: written size mismatch for face " + std::to_string(f));
            }

            outCube.Faces[f].DataSize = outCube.Faces[f].CompressedData.size();
        }

        ktxTexture_Destroy(ktxTexture(pTexture));

        validateResultCubemap(outCube);

        return outCube;
    }
}