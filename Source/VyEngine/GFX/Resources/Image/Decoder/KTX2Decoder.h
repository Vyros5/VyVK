#pragma once

#include <VyEngine/GFX/Resources/Image/Decoder/ImageDecoder.h>

namespace Vy
{
    class KTX2Decoder final : public VyImageDecoder 
    {
    public:

        /// <summary>
        /// Determines whether the provided file path has the KTX2 file extension
        /// </summary>
        /// <param name="path">file path or name to check</param>
        /// <returns>true if the extracted extension is exactly "ktx2"</returns>
        bool canDecode(const TPath& path) const override;

        /// <summary>
        /// Loads a KTX2 texture file using the KTX library
        /// </summary>
        /// <param name="path">Path to the KTX2 file to decode</param>
        /// <returns>VyDecodedImage with metadata </returns>
        VyDecodedImage decode(const TPath& path) const override;

        /// <summary>
        /// Loads a KTX2 texture file using the KTX library into cubemap format
        /// </summary>
        /// <param name="path">Path to the KTX2 file to decode</param>
        /// <returns>VyDecodedImage with metadata </returns>
        VyDecodedCubemap decodeCubemap(const TPath& path) const;
    };
}