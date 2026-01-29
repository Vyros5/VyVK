#pragma once

#include <future>

#include <VyEngine/VK/Core/VKCore.h>
#include <VyLib/STL/Path.h>

namespace Vy
{
    class VyTextureObject;
    struct VyDecodedImage;
    struct VyDecodedCubemap;

    class TextureLoader 
    {
    public:

        /// <summary>
        /// Loads a texture from the specified file path and returns a GPU texture by forwarding to the right loader based on file extension
        /// </summary>
        /// <param name="device">Reference to the Device used to create/upload the texture on the GPU</param>
        /// <param name="path">Filesystem path to the texture file</param>
        /// <returns>loaded texture</returns>
        static Unique<VyTextureObject> load(const TPath& path, bool bUseMipmap);

        /// <summary>
        /// Loads a cubemap texture
        /// </summary>
        /// <param name="device">Device used to create GPU resources and upload the texture.</param>
        /// <returns>loaded cubemap texture</returns>
        static Unique<VyTextureObject> loadCubemap(const TPath& Path);

        //// Explicit loaders ////

        /// <summary>
        /// Loads a 2D STB texture 
        /// </summary>
        /// <param name="device">Reference to the Device used to create and upload GPU resources for the Texture</param>
        /// <param name="path">Path to the image file to load</param>
        /// <param name="bSRGB">If true, interpret the texture as sRGB; otherwise treat it as linear color space</param>
        /// <returns>created Texture</returns>
        static Unique<VyTextureObject> load2D(const TPath& path, bool bUseMipmap, bool bSRGB = true);

        /// <summary>
        /// Loads a 2D HDR texture 
        /// </summary>
        /// <param name="device">Reference to the Device used to create and upload GPU resources for the Texture</param>
        /// <param name="path">Path to the image file to load</param>
        /// <returns>created Texture</returns>
        static Unique<VyTextureObject> loadHDR(const TPath& path, bool bUseMipmap);

        /// <summary>
        /// Loads a 2D KTX texture 
        /// </summary>
        /// <param name="device">Reference to the Device used to create and upload GPU resources for the Texture</param>
        /// <param name="path">Path to the image file to load</param>
        /// <returns>created Texture</returns>
        static Unique<VyTextureObject> loadKTX(const TPath& path, bool bUseMipmap);

        /// <summary>
        /// Loads a 2D KTX2 texture 
        /// </summary>
        /// <param name="device">Reference to the Device used to create and upload GPU resources for the Texture</param>
        /// <param name="path">Path to the image file to load</param>
        /// <returns>created Texture</returns>
        static Unique<VyTextureObject> loadKTX2(const TPath& path, bool bUseMipmap);

        // Async version TODO
        static std::future<Unique<VyTextureObject>> loadAsync(const TPath& path, bool bSRGB = true);

    private:
        /// <summary>
        /// Loads a cubemap texture from a directory
        /// </summary>
        /// <param name="device">Device used to create GPU resources and upload the texture</param>
        /// <param name="directoryPath">Filesystem path to a directory containing the cubemap image files</param>
        /// <returns> loaded cubemap texture </returns>
        static Unique<VyTextureObject> loadCubemapFromDir(const TPath& directoryPath);

        /// <summary>
        /// Creates and uploads a Texture from a decoded image
        /// </summary>
        static Unique<VyTextureObject> loadFromDecoded(const VyDecodedImage& img, bool bUseMipmap, bool bSRGB);

        /// <summary>
        /// Creates and uploads a cubemap texture from decoded image data and returns the resulting Texture object
        /// </summary>
        /// <param name="cube">const reference to the VyDecodedCubemap containing the six faces image data to upload</param>
        /// <returns>uploaded cubemap Texture</returns>
        static Unique<VyTextureObject> loadFromDecoded(const VyDecodedCubemap& cube, bool bSRGB);
    };
}