#pragma once

#include <VyEngine/GFX/Resources/Image/Decoder/ImageDecoder.h>

namespace Vy
{
    class KTXDecoder final : public VyImageDecoder 
    {
    public:

        /// <summary>
        /// Determines whether the provided file path has the KTX file extension
        /// </summary>
        /// <param name="path">file path or name to check</param>
        bool canDecode(const TPath& path) const override;

        /// <summary>
        /// Loads a KTX texture file into a VyDecodedImage struct
        /// </summary>
        /// <param name="path">Path to the KTX file to decode</param>
        /// <returns>VyDecodedImage with metadata </returns>
        VyDecodedImage decode(const TPath& path) const override;

        /// <summary>
        /// Loads a KTX cubemap texture file into a DecodedCubemap struct
        /// </summary>
        /// <param name="path">Path to the KTX file to decode</param>
        /// <returns>VyDecodedImage with metadata </returns>
        VyDecodedCubemap decodeCubemap(const TPath& path) const;
    };
}