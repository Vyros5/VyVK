#include <VyEngine/GFX/Resources/Image/Decoder/STBDecoder.h>
#include <stb_image.h>

#include <VyEngine/GFX/Resources/Image/Decoder/HDRDecoder.h>
#include <VyEngine/GFX/Resources/Image/Decoder/KTX2Decoder.h>
#include <VyEngine/GFX/Resources/Image/Decoder/KTXDecoder.h>

namespace Vy
{
    namespace 
    {
        static const TArray<const char*, 6> FACE_PATTERNS_POS = {
            "posx", "negx", "posy", "negy", "posz", "negz"
        };

        static const TArray<const char*, 6> FACE_PATTERNS_ALT = {
            "right", "left", "top", "bottom", "front", "back"
        };

        static const TArray<const char*, 6> KTX_CUBE_FACE_ORDER = {
            "+X", "-X", "+Y", "-Y", "+Z", "-Z"
        };

        /// <summary>
        /// List available decoders for regular 2D images(directory loading)
        /// </summary>
        /// <returns>usable decoder</returns>
        TArray<Unique<VyImageDecoder>, 4> createDecoders()
        {
            TArray<Unique<VyImageDecoder>, 4> Decoders{
                MakeUnique<STBDecoder> (),
                MakeUnique<HDRDecoder> (),
                MakeUnique<KTX2Decoder>(),
                MakeUnique<KTXDecoder> ()
            };

            return Decoders;
        }

        /// <summary>
        /// select and use the right decoder a 2D image
        /// </summary>
        VyDecodedImage decodeSingle2D(const TString& path)
        {
            auto decoders = createDecoders();

            for (auto& pDecoder : decoders)
            {
                if (pDecoder->canDecode( path ))
                {
                    return pDecoder->decode( path );
                }
            }

            VY_THROW_RUNTIME_ERROR("No decoder found for: " + path);
        }
        

        /// <summary>
        /// simple getter to retrieve a list of files in a directory
        /// </summary>
        TVector<TPath> getFilesInDirectory(const TString& directoryPath)
        {
            TVector<TPath> files;

            for (auto& entry : FS::directory_iterator(directoryPath))
            {
                if (!entry.is_regular_file())
                {
                    continue;
                }

                files.push_back( entry.path() );
            }

            return files;
        }


        /// <summary>
        /// Searches a list of files for a set of cubemap face filenames using one of the provided filename patterns and returns the matched file paths in face order
        /// </summary>
        /// <param name="files">list of files to search in</param>
        /// <param name="patterns">vector of candidate patterns. Each element is an array of six C-style string substrings</param>
        /// <returns>full file paths as strings for the six cubemap faces, ordered according to the matched pattern</returns>
        static TArray<TString, 6> findFaceFiles(const TVector<TPath>& files, TVector<TArray<const char*, 6>> patterns)
        {
            TArray<TString, 6> faceFiles;
            bool bFound = false;

            for (const auto& patt : patterns)
            {
                bool bMatchPattern = true;

                for (int i = 0; i < 6; i++)
                {
                    auto it = std::find_if(files.begin(), files.end(),
                        [&](const TPath& p) 
                        {
                            TString lower = p.filename().string();

                            std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
                            
                            return lower.find(patt[i]) != TString::npos;
                        }
                    );

                    if (it == files.end())
                    {
                        bMatchPattern = false;

                        break;
                    }

                    faceFiles[i] = it->string();
                }

                if (bMatchPattern)
                {
                    bFound = true;

                    break;
                }
            }

            if (!bFound)
            {
                VY_THROW_RUNTIME_ERROR("Could not determine cubemap ordering from directory");
            }

            return faceFiles;
        }


        /// <summary>
        /// valideates that the six cubemap faces have identical dimensions and the same pixel format
        /// </summary>
        void validateCubemap(const VyDecodedCubemap& cube)
        {
            const auto& ref = cube.Faces[ 0 ];

            for (int i = 1; i < 6; i++)
            {
                const auto& f = cube.Faces[ i ];

                if (f.Width  != ref.Width || 
                    f.Height != ref.Height)
                {
                    VY_THROW_RUNTIME_ERROR("Cubemap faces must have identical dimensions.");
                }

                if (f.Format != ref.Format)
                {
                    VY_THROW_RUNTIME_ERROR("Cubemap faces must have identical VkFormat.");
                }

                if (f.IsCompressed != ref.IsCompressed)
                {
                    VY_THROW_RUNTIME_ERROR("Cubemap faces must have identical compression state.");
                }

                if (f.MipLevels != ref.MipLevels)
                {
                    VY_THROW_RUNTIME_ERROR("Cubemap faces must have same number of mip levels.");
                }

                if (f.IsFloat != ref.IsFloat)
                {
                    VY_THROW_RUNTIME_ERROR("Cubemap faces must be all-float or all-uint8.");
                }
            }
        }

    }

    /// <summary>
    /// Determines if the file extension is matching a format supported by the STB-based decoder
    /// </summary>
    /// <param name="path">path to extension that will be checked</param>
    bool STBDecoder::canDecode(const TPath& path) const
    {
        const TString ext = imDecoder::getExtension( path.string() );

        // STB-supported formats
        return (
            ext == "png" ||
            ext == "jpg" || ext == "jpeg" ||
            ext == "bmp" ||
            ext == "tga" ||
            ext == "gif" ||
            ext == "ppm" || ext == "pgm" || ext == "pnm"
        );
    }

    /// <summary>
    /// Loads an image from the given file path using stb_image and returns it as a VyDecodedImage with 8-bit RGBA pixels
    /// </summary>
    /// <param name="path">Filesystem path to the image file to load</param>
    /// <returns>VyDecodedImage</returns>
    VyDecodedImage STBDecoder::decode(const TPath& path) const
    {
        VyDecodedImage img{};
        img.IsFloat = false;

        int width    = 0;
        int height   = 0;
        int channels = 0;

        unsigned char* data = stbi_load(
            path.string().c_str(),
            &width,
            &height,
            &channels,
            STBI_rgb_alpha
        );

        if (!data) 
        {
            VY_THROW_RUNTIME_ERROR("STBDecoder: Failed to load image: " + path.string());
        }

        img.Width    = static_cast<U32>(width);
        img.Height   = static_cast<U32>(height);
        img.Channels = 4;

        const size_t size = static_cast<size_t>(width) * height * 4;

        img.Pixels8 = TVector<unsigned char>(size);
        std::memcpy(img.Pixels8.data(), data, size);

        stbi_image_free(data);
        
        return img;
    }


    /// <summary>
    /// create cubemap from directory of 6 images corresponding to cubemap faces
    /// </summary>
    /// <param name="directoryPath">directory containing the 6 faces</param>
    /// <returns>VyDecodedCubemap containing 6 VyDecodedImage</returns>
    VyDecodedCubemap STBDecoder::decodeCubemapFromDirectory(const TString& directoryPath) const
    {
        if (!FS::exists(directoryPath))
        {
            VY_THROW_RUNTIME_ERROR("Cubemap directory does not exist: " + directoryPath);
        }

        auto files = getFilesInDirectory(directoryPath);

        if (files.size() < 6)
        {
            VY_THROW_RUNTIME_ERROR("Cubemap directory must contain at least 6 images.");
        }

        auto faceFiles = findFaceFiles(
            files,
            { FACE_PATTERNS_POS, FACE_PATTERNS_ALT }
        );

        // Decode the 6 faces
        VyDecodedCubemap cubemap;
        for (int i = 0; i < 6; i++)
        {
            cubemap.Faces[i] = decodeSingle2D( faceFiles[ i ] );
        }

        validateCubemap( cubemap );

        return cubemap;
    }
}