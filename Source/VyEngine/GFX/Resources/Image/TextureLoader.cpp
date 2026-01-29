#include <VyEngine/GFX/Resources/Image/TextureLoader.h>

#include <VyEngine/GFX/Resources/Image/TextureUploader.h>

#include <VyEngine/GFX/Resources/Image/Decoder/STBDecoder.h>
#include <VyEngine/GFX/Resources/Image/Decoder/HDRDecoder.h>
#include <VyEngine/GFX/Resources/Image/Decoder/KTX2Decoder.h>
#include <VyEngine/GFX/Resources/Image/Decoder/KTXDecoder.h>

#include <algorithm>
#include <cctype>
#include <future>

namespace Vy
{
    //// TextureLoader Implementation ////

    /// <summary>
    /// Loads a texture from the specified file path and returns a GPU texture by forwarding to the right loader based on file extension
    /// </summary>
    /// <param name="device">Reference to the Device used to create/upload the texture on the GPU</param>
    /// <param name="path">Filesystem path to the texture file</param>
    /// <returns>loaded texture</returns>
    Unique<VyTextureObject> TextureLoader::load(const TPath& path, bool bUseMipmap) 
    {
        const TString ext = imDecoder::getExtension(path.string());

        if (ext == "hdr") 
        {
            return loadHDR(path, bUseMipmap);
        }

        if (ext == "ktx2") 
        {
            return loadKTX2(path, bUseMipmap);
        }
        
        if (ext == "ktx") 
        {
            return loadKTX(path, bUseMipmap);
        }

        // PNG/JPG/etc
        return load2D(path, bUseMipmap, /*bSRGB=*/true);
    }


    /// <summary>
    /// Loads a 2D STB texture 
    /// </summary>
    /// <param name="device">Reference to the Device used to create and upload GPU resources for the Texture</param>
    /// <param name="path">Path to the image file to load</param>
    /// <param name="bSRGB">If true, interpret the texture as sRGB; otherwise treat it as linear color space</param>
    /// <returns>created Texture</returns>
    Unique<VyTextureObject> TextureLoader::load2D(const TPath& path, bool bUseMipmap, bool bSRGB) 
    {
        STBDecoder decoder;

        if (!decoder.canDecode(path)) 
        {
            VY_THROW_RUNTIME_ERROR("TextureLoader::load2D - Unsupported 2D texture format: " + imDecoder::getExtension(path.string()));
        }

        VyDecodedImage img = decoder.decode(path);
        
        return loadFromDecoded(img, bUseMipmap, bSRGB);
    }


    /// <summary>
    /// Loads a 2D HDR texture 
    /// </summary>
    /// <param name="device">Reference to the Device used to create and upload GPU resources for the Texture</param>
    /// <param name="path">Path to the image file to load</param>
    /// <returns>created Texture</returns>
    Unique<VyTextureObject> TextureLoader::loadHDR(const TPath& path, bool bUseMipmap) 
    {
        HDRDecoder decoder;
        
        if (!decoder.canDecode(path)) 
        {
            VY_THROW_RUNTIME_ERROR("TextureLoader::loadHDR - Cannot decode HDR texture: " + path.string());
        }

        VyDecodedImage img = decoder.decode(path);

        if (!img.IsFloat) 
        {
            VY_THROW_RUNTIME_ERROR("TextureLoader::loadHDR - Decoder did not produce floating point image!");
        }

        // HDR = float texture, never sRGB
        return TextureUploader::upload2D(img, bUseMipmap, /*bSRGB=*/ false);
    }


    /// <summary>
    /// Loads a 2D KTX texture 
    /// </summary>
    /// <param name="device">Reference to the Device used to create and upload GPU resources for the Texture</param>
    /// <param name="path">Path to the image file to load</param>
    /// <returns>created Texture</returns>
    Unique<VyTextureObject> TextureLoader::loadKTX(const TPath& path, bool bUseMipmap)
    {
        KTXDecoder decoder;
        
        if (!decoder.canDecode(path)) 
        {
            VY_THROW_RUNTIME_ERROR("TextureLoader::loadKTX - Cannot decode KTX texture: " + path.string());
        }

        VyDecodedImage img = decoder.decode(path);

        // KTX contains its own format -> no bSRGB flag needed
        return TextureUploader::uploadCompressed2D(img);

    }


    /// <summary>
    /// Loads a 2D KTX2 texture 
    /// </summary>
    /// <param name="device">Reference to the Device used to create and upload GPU resources for the Texture</param>
    /// <param name="path">Path to the image file to load</param>
    /// <returns>created Texture</returns>
    Unique<VyTextureObject> TextureLoader::loadKTX2(const TPath& path, bool bUseMipmap)
    {
        KTX2Decoder decoder;
        
        if (!decoder.canDecode(path)) 
        {
            VY_THROW_RUNTIME_ERROR("TextureLoader::loadKTX2 - Cannot decode KTX2 texture: " + path.string());
        }

        VyDecodedImage img = decoder.decode(path);

        // KTX2 contains its own format -> no bSRGB flag needed
        return TextureUploader::uploadCompressed2D(img);
    }


    /// <summary>
    /// Loads a cubemap texture
    /// </summary>
    /// <param name="device">Device used to create GPU resources and upload the texture.</param>
    /// <returns>loaded cubemap texture</returns>
    Unique<VyTextureObject> TextureLoader::loadCubemap(const TPath& path) 
    {
        const TString ext = imDecoder::getExtension(path.string());

        if (ext == "hdr") 
        {
            VY_THROW_RUNTIME_ERROR("Cannot decode hdr as cubemap texture: not implemented yet");
        }

        if (ext == "ktx2") 
        {
            KTX2Decoder      decoder;
            VyDecodedCubemap cubemap = decoder.decodeCubemap(path);

            return loadFromDecoded(cubemap, /*bSRGB=*/false);
        }

        if (ext == "ktx") 
        {
            KTXDecoder       decoder;
            VyDecodedCubemap cubemap = decoder.decodeCubemap(path);

            return loadFromDecoded(cubemap, /*bSRGB=*/false);
        }

        return loadCubemapFromDir(path);
    }


    /// <summary>
    /// Loads a cubemap texture from a directory
    /// </summary>
    /// <param name="device">Device used to create GPU resources and upload the texture</param>
    /// <param name="directoryPath">Filesystem path to a directory containing the cubemap image files</param>
    /// <returns> loaded cubemap texture </returns>
    Unique<VyTextureObject> TextureLoader::loadCubemapFromDir(const TPath& directoryPath)
    {
        if (!FS::exists(directoryPath) || !FS::is_directory(directoryPath)) 
        {
            VY_THROW_RUNTIME_ERROR("Path is not a directory: " + directoryPath.string() + " cubemap should be directory or ktx/ktx2");
        }

        STBDecoder decoder;

        VyDecodedCubemap cubemap = decoder.decodeCubemapFromDirectory(directoryPath.string());
        
        return loadFromDecoded(cubemap, /*bSRGB=*/false);
    }


    /// <summary>
    /// Creates and uploads a Texture from a decoded image
    /// </summary>
    Unique<VyTextureObject> TextureLoader::loadFromDecoded(const VyDecodedImage& img, bool bUseMipmap, bool bSRGB) 
    {
        if (img.IsFloat) 
        {
            // Float -> HDR or EXR
            return TextureUploader::upload2D(img, bUseMipmap, false);
        }
        
        return TextureUploader::upload2D(img, bUseMipmap, bSRGB);
    }


    /// <summary>
    /// Creates and uploads a cubemap texture from decoded image data and returns the resulting Texture object
    /// </summary>
    /// <param name="cube">const reference to the VyDecodedCubemap containing the six faces image data to upload</param>
    /// <returns>uploaded cubemap Texture</returns>
    Unique<VyTextureObject> TextureLoader::loadFromDecoded(const VyDecodedCubemap& cube, bool bSRGB) 
    {
        return TextureUploader::uploadCubemap(cube);
    }

    
    //// async loading ////
    /// TODO
    std::future<Unique<VyTextureObject>> TextureLoader::loadAsync(const TPath& path, bool bSRGB) 
    {
        return std::async(std::launch::async, [&path, bSRGB]() 
        {
            return load2D(path, bSRGB);
        });
    }
}