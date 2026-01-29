#include <VyEngine/GFX/Resources/Image/Decoder/HDRDecoder.h>

#include <stb_image.h>

namespace Vy
{
    /// <summary>
    /// check is the decoder can decode the given file
    /// </summary>
    bool HDRDecoder::canDecode(const TPath& path) const
    {
        return imDecoder::getExtension(path.string()) == "hdr";
    }

    /// <summary>
    /// Loads an image from the given file path using stb_image and returns it as a VyDecodedImage with 32-bit RGBA pixels
    /// </summary>
    /// <param name="path">path to the image file to load</param>
    /// <returns>VyDecodedImage</returns>
    VyDecodedImage HDRDecoder::decode(const TPath& path) const
    {
        VyDecodedImage img{};
        img.IsFloat = true;

        int width    = 0;
        int height   = 0;
        int channels = 0;

        float* pData = stbi_loadf(
            path.string().c_str(),
            &width,
            &height,
            &channels,
            STBI_rgb_alpha
        );

        if (!pData) 
        {
            VY_THROW_RUNTIME_ERROR("HDRDecoder: Failed to load HDR image: " + path.string());
        }

        img.Width    = static_cast<U32>(width);
        img.Height   = static_cast<U32>(height);
        img.Channels = 4;

        const size_t floatCount = static_cast<size_t>(width) * height * 4;
        const size_t sizeBytes  = floatCount * sizeof(float);

        img.Pixels32 = TVector<float>(floatCount);
        std::memcpy(img.Pixels32.data(), pData, sizeBytes);

        stbi_image_free(pData);
        
        return img;
    }
}