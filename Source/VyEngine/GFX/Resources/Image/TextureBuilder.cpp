#include <VyEngine/GFX/Resources/Image/TextureBuilder.h>

#include <VyEngine/GFX/Resources/Image/Decoder/ImageDecoder.h>
#include <VyEngine/GFX/Resources/Image/TextureLoader.h>
#include <VyEngine/GFX/Resources/Image/TextureUploader.h>
#include <VyEngine/GFX/Resources/Image/Texture.h>

#include <VyEngine/VK/Context.h>

namespace Vy
{
    TextureBuilder::TextureBuilder()
    {}

    //// Input sources ////

    /// <summary>
    /// small setter for file path, save the extension
    /// </summary>
    /// <param name="path"></param>
    /// <returns></returns>
    TextureBuilder& TextureBuilder::fromFile(const TString& path)
    {
        m_Path = path;

        // Basic file extension check
        const TString ext = imDecoder::getExtension(m_Path);

        if (ext == "ktx2")
        {
            m_Source = ESourceType::Ktx2;
        }
        else if (ext == "ktx")
        {
            m_Source = ESourceType::Ktx1;
        }
        else if (ext == "hdr")
        {
            m_Source = ESourceType::Hdr;
        }
        else
        {
            m_Source = ESourceType::Stb;
        }

        return *this;
    }

    /// <summary>
    /// small setter for KTX2 file path
    /// </summary>
    TextureBuilder& TextureBuilder::fromKTX2(const TString& path)
    {
        m_Path   = path;
        m_Source = ESourceType::Ktx2;

        return *this;
    }

    /// <summary>
    /// small setter for KTX file path
    /// </summary>
    TextureBuilder& TextureBuilder::fromKTX(const TString& path)
    {
        m_Path   = path;
        m_Source = ESourceType::Ktx1;

        return *this;
    }

    /// <summary>
    /// small setter for HDR file path
    /// </summary>
    TextureBuilder& TextureBuilder::fromHDR(const TString& path)
    {
        m_Path   = path;
        m_Source = ESourceType::Hdr;

        return *this;
    }

    /// <summary>
    /// small setter for STB file path
    /// </summary>
    TextureBuilder& TextureBuilder::fromSTB(const TString& path)
    {
        m_Path = path;
        m_Source = ESourceType::Stb;

        return *this;
    }


    //// Options ////

    TextureBuilder& TextureBuilder::fromVector(const TVector<TVector<TVector<float>>>& textureArray)
    {
        m_Source = ESourceType::FloatArray;

        m_ArrayH = static_cast<U32>(textureArray      .size());
        m_ArrayW = static_cast<U32>(textureArray[0]   .size());
        m_ArrayD = static_cast<U32>(textureArray[0][0].size());

        if (m_ArrayH == 0 || m_ArrayW == 0 || m_ArrayD == 0)
        {
            VY_THROW_RUNTIME_ERROR("TextureBuilder: fromVector: input array has invalid dimensions");
        }

        m_ArrayPixels.resize(m_ArrayW * m_ArrayH * m_ArrayD);

        size_t index = 0;

        for (size_t y = 0; y < m_ArrayH; ++y) 
        {
            for (size_t x = 0; x < m_ArrayW; ++x) 
            {
                for (size_t c = 0; c < m_ArrayD; ++c) 
                {
                    m_ArrayPixels[index++] = textureArray[y][x][c];
                }
            }
        }

        return *this;
    }

    TextureBuilder& TextureBuilder::fromCharBuffer(TVector<unsigned char> buffer, const size_t width, const size_t height, const size_t channel, const size_t mipLevel)
    {
        m_CharBuffer = buffer;

        m_ArrayH         = static_cast<U32>(height);
        m_ArrayW         = static_cast<U32>(width);
        m_ArrayD         = static_cast<U32>(channel);
        m_ArrayMipLevels = static_cast<U32>(mipLevel);

        if (m_ArrayMipLevels) m_UseMipmaps = true;

        m_Source = ESourceType::RawBuffer;

        return *this;
    }

    /// <summary>
    /// small setter to enable/disable sRGB sampling
    /// </summary>
    TextureBuilder& TextureBuilder::withSRGB(bool enable)
    {
        m_UseSRGB = enable;

        return *this;
    }

    /// <summary>
    /// small setter to enable/disable mipmaps
    /// </summary>
    TextureBuilder& TextureBuilder::withMipmaps(bool enable)
    {
        m_UseMipmaps = enable;

        return *this;
    }

    /// <summary>
    /// small setter to force cubemap creation
    /// </summary>
    TextureBuilder& TextureBuilder::asCubemap(bool enable)
    {
        m_ForceCubemap = enable;

        return *this;
    }

    /// <summary>
    /// small setter for minification filter
    /// </summary>
    TextureBuilder& TextureBuilder::withMinFilter(VkFilter f)
    {
        m_MinFilter = f;


        return *this;
    }

    /// <summary>
    /// small setter for magnification filter
    /// </summary>
    TextureBuilder& TextureBuilder::withMagFilter(VkFilter f)
    {
        m_MagFilter = f;

        return *this;
    }

    /// <summary>
    /// small setter for sampler wrap mode
    /// </summary>
    TextureBuilder& TextureBuilder::withWrap(VkSamplerAddressMode mode)
    {
        m_WrapMode = mode;

        return *this;
    }

    /// <summary>
    /// builds the texture based on the set options
    /// </summary>
    Unique<VyTextureObject> TextureBuilder::build()
    {
        switch (m_Source) 
        {
            case ESourceType::RawBuffer:  return buildFromCharBuffer();
            case ESourceType::FloatArray: return buildFromArray();
            case ESourceType::Stb:
            case ESourceType::Hdr:
            case ESourceType::Ktx1:
            case ESourceType::Ktx2:
                return m_ForceCubemap ? buildCubemap() : build2D();

            default: 
            {
                VY_THROW_RUNTIME_ERROR("TextureBuilder: No valid source set");
            }
        }
    }

    /// <summary>
    /// builds a 2D texture
    /// </summary>
    Unique<VyTextureObject> TextureBuilder::build2D()
    {
        if (m_Path.empty())
        {
            VY_THROW_RUNTIME_ERROR("TextureBuilder: No input path set");
        }

        auto texture = TextureLoader::load(m_Path, m_UseMipmaps);

        // Update sampler parameters.
        texture->updateSampler(m_MinFilter, m_MagFilter, m_WrapMode);

        return texture;
    }
    

    /// <summary>
    /// builds a cubemap texture
    /// </summary>
    Unique<VyTextureObject> TextureBuilder::buildCubemap()
    {
        if (m_Path.empty())
        {
            VY_THROW_RUNTIME_ERROR("TextureBuilder: No input path set");
        }
        
        auto texture = TextureLoader::loadCubemap(m_Path);

        // Update sampler parameters
        texture->updateSampler(m_MinFilter, m_MagFilter, m_WrapMode);

        return texture;
    }


    Unique<VyTextureObject> TextureBuilder::buildFromArray()
    {
        if (m_ArrayPixels.empty())
        {
            VY_THROW_RUNTIME_ERROR("TextureBuilder: Array source is empty");
        }

        VyDecodedImage img{};
        {
            img.Width        = m_ArrayW;
            img.Height       = m_ArrayH;
            img.Channels     = m_ArrayD;
            img.IsFloat      = true;
            img.IsCompressed = false;
            img.Pixels32     = m_ArrayPixels;
            img.MipLevels    = 1;

            img.Format = 
                (m_ArrayD == 1) ? VK_FORMAT_R32_SFLOAT :
                (m_ArrayD == 2) ? VK_FORMAT_R32G32_SFLOAT :
                (m_ArrayD == 3) ? VK_FORMAT_R32G32B32_SFLOAT :
                                VK_FORMAT_R32G32B32A32_SFLOAT;
        }

        return TextureUploader::upload2D(img, false, m_UseSRGB);
    }


    Unique<VyTextureObject> TextureBuilder::buildFromCharBuffer()
    {
        if (m_CharBuffer.empty())
        {
            VY_THROW_RUNTIME_ERROR("TextureBuilder: char buffer source is empty");
        }

        VyDecodedImage img{};
        {
            img.Width        = m_ArrayW;
            img.Height       = m_ArrayH;
            img.Channels     = m_ArrayD;
            img.IsFloat      = false;
            img.IsCompressed = false;
            img.Pixels8      = m_CharBuffer;
            img.MipLevels    = m_ArrayMipLevels;

            img.Format = 
                (m_ArrayD == 1) ? VK_FORMAT_R8_UNORM :
                (m_ArrayD == 2) ? VK_FORMAT_R8G8_UNORM :
                (m_ArrayD == 3) ? VK_FORMAT_R8G8B8_UNORM :
                                VK_FORMAT_R8G8B8A8_UNORM;
        }

        return TextureUploader::upload2D(img, m_UseMipmaps, m_UseSRGB);
    }


    // Unique<VyTextureObject> TextureBuilder::fromTextureInfo(VkImageCreateInfo imageInfo, VkImageViewCreateInfo viewInfo, VkSamplerCreateInfo samplerInfo, VkImageLayout initImageLayout, U32 layerCount)
    // {
    //     Unique<VyTextureObject> texture = MakeUnique<VyTextureObject>();
        
    //     texture->m_Extent = { imageInfo.extent.width, imageInfo.extent.height };

	// 	VmaAllocationCreateInfo allocInfo{};
    //     {
    //         allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
	// 		allocInfo.flags = 0;
    //     }

    //     // create image
    //     VyContext::device().createImage(texture->m_Image, texture->m_Allocation, imageInfo, allocInfo);

    //     // create image view
    //     viewInfo.image = texture->m_Image;
    //     if (vkCreateImageView(VyContext::device(), &viewInfo, nullptr, &texture->m_View) != VK_SUCCESS) 
    //     {
    //         VY_THROW_RUNTIME_ERROR("failed to create texture image view!");
    //     }

    //     // create sampler
    //     if (vkCreateSampler(VyContext::device(), &samplerInfo, nullptr, &texture->m_Sampler) != VK_SUCCESS) 
    //     {
    //         VY_THROW_RUNTIME_ERROR("failed to create texture sampler!");
    //     }

    //     if (initImageLayout != VK_IMAGE_LAYOUT_UNDEFINED)
    //     {
    //         VyContext::device().transitionImageLayout(
    //             texture->m_Image, 
    //             imageInfo.format,
    //             VK_IMAGE_LAYOUT_UNDEFINED, 
    //             initImageLayout, 
    //             layerCount
    //         );
    //     }

    //     texture->m_IsLoaded = true;

    //     return texture;
    // }
}