#pragma once


#include <VyEngine/VK/Core/VKCore.h>
#include <VyLib/STL/Path.h>

namespace Vy
{
    namespace imDecoder 
    {
        /**
         * @brief Return lowercase extension without dot.
         * 
         * @param path path to texture file.
         * 
         * @return lowercase extension without dot.
         */
        inline TString getExtension(const TString& path) 
        {
            auto pos = path.find_last_of('.');

            if (pos == TString::npos)
            {
                return {};
            }

            TString ext = path.substr(pos + 1);

            std::transform(ext.begin(), ext.end(), ext.begin(),
                [](unsigned char c) 
                { 
                    return std::tolower(c); 
                }
            );
            
            return ext;
        }
    }

    struct VyDecodedImage 
    {
        int Width       = 0;
        int Height      = 0;
        int Channels    = 4;
        int ArrayLayers = 1;

        VkFormat           Format    = VK_FORMAT_UNDEFINED;
        VkImageAspectFlags ImageFlag = 0;

        bool IsFloat      = false;
        bool IsCompressed = false;
        bool IsCubemap    = false;

        TVector<unsigned char> Pixels8;
        TVector<float>         Pixels32;
        TVector<U8>            CompressedData;

        size_t       DataSize = 0; 
        TVector<U32> MipSizes;
        TVector<U32> MipOffsets;
        U32          MipLevels = 1;

    };

    struct VyDecodedCubemap 
    {
        // 6 faces in order: +X, -X, +Y, -Y, +Z, -Z
        TArray<VyDecodedImage, 6> Faces;
    };


    class VyImageDecoder 
    {
    public:
        virtual ~VyImageDecoder() = default;

        virtual bool canDecode(const TPath& path) const = 0;

        virtual VyDecodedImage decode(const TPath& path) const = 0;
    };
}