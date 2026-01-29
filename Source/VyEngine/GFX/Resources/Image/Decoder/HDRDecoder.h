#pragma once

#include <VyEngine/GFX/Resources/Image/Decoder/ImageDecoder.h>

namespace Vy
{
    class HDRDecoder final : public VyImageDecoder 
    {
    public:

        /// <summary>
        ///  check is the decoder can decode the given file
        /// </summary>
        /// <param name="path">file path or name to check</param>
        bool canDecode(const TPath& path) const override;

        /// <summary>
        /// Loads an image from the given file path using stb_image and returns it as a VyDecodedImage with 32-bit RGBA pixels
        /// </summary>
        /// <param name="path">path to the image file to load</param>
        /// <returns>VyDecodedImage</returns>
        VyDecodedImage decode(const TPath& path) const override;
    };
}