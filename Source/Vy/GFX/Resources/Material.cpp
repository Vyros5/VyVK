#include <Vy/GFX/Resources/Material.h>

// #include <Vy/Engine.h>

#include <Vy/GFX/Backend/Pipeline.h>
#include <Vy/GFX/Context.h>
#include <Vy/Globals.h>

#include <stb_image.h>

namespace Vy
{
    VyMaterial::VyMaterial()
    {
        createDefaultTexture();
    }


    VyMaterial::~VyMaterial() 
    {
    }


    void VyMaterial::createDefaultTexture() 
    {
        // Create a simple 1x1 white texture as default
        const U32 texWidth    = 1;
        const U32 texHeight   = 1;
        const U32 texChannels = 4;

        unsigned char pixels[4] = { 255, 255, 255, 255 }; // White pixel

        VkDeviceSize imageSize = texWidth * texHeight * texChannels;

        // Create staging buffer
        VyBuffer stagingBuffer{ VyBuffer::stagingBuffer(imageSize) };

        stagingBuffer.singleWrite(pixels);

        // Create image
        m_DefaultTextureImage = VyImage::Builder{}
            .imageType  (VK_IMAGE_TYPE_2D)
            .format     (VK_FORMAT_R8G8B8A8_SRGB)
            .extent     (VkExtent2D{ texWidth, texHeight })
            .mipLevels  (1)
			.arrayLayers(1)
            .tiling     (VK_IMAGE_TILING_OPTIMAL)
            .imageLayout(VK_IMAGE_LAYOUT_UNDEFINED)
            .usage      (VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
            .sampleCount(VK_SAMPLE_COUNT_1_BIT)
            .sharingMode(VK_SHARING_MODE_EXCLUSIVE)
            .memoryUsage(VMA_MEMORY_USAGE_AUTO)
        .build();

        // Transition image layout and copy buffer to image.
        m_DefaultTextureImage.copyFrom(stagingBuffer);

        // Create image view
        createTextureImageView(m_DefaultTextureImage, m_DefaultTextureImageView);
        
        // Create sampler
        createTextureSampler(m_DefaultTextureSampler);
    }


    void VyMaterial::loadAlbedoTexture(const String& filepath) 
    {
        try 
        {
            createTextureImage(filepath, m_AlbedoTextureImage);
            createTextureImageView(m_AlbedoTextureImage, m_AlbedoTextureImageView);
            createTextureSampler(m_AlbedoTextureSampler);
            
            m_HasAlbedoTexture = true;
        } 
        catch (const std::exception& e) 
        {
            VY_ERROR_TAG("VyMaterial", "Failed to load Albedo Texture: {}\n{}", filepath, e.what());

            m_HasAlbedoTexture = false;

            // Make the object neon pink if it cant load or find the material
            m_FailedAlbedo = true;
            m_Data.Albedo  = Vec3(1.0f, 0.0f, 1.0f); // pink
        }
    }


    void VyMaterial::loadNormalTexture(const String& filepath) 
    {
        try 
        {
            createTextureImage(filepath, m_NormalTextureImage);
            createTextureImageView(m_NormalTextureImage, m_NormalTextureImageView);
            createTextureSampler(m_NormalTextureSampler);

            m_HasNormalTexture = true;
        } 
        catch (const std::exception& e) 
        {
            VY_ERROR_TAG("VyMaterial", "Failed to load Normal Texture: {}\n{}", filepath, e.what());

            m_HasNormalTexture = false;
        }
    }

    
    void VyMaterial::loadRoughnessMap(const String& filepath) 
    {
        try 
        {
            createTextureImage(filepath, m_RoughnessTextureImage);
            createTextureImageView(m_RoughnessTextureImage, m_RoughnessTextureImageView);
            createTextureSampler(m_RoughnessTextureSampler);

            m_HasRoughnessTexture = true;
        } 
        catch (const std::exception& e) 
        {
            VY_ERROR_TAG("VyMaterial", "Failed to load Roughness Texture: {}\n{}", filepath, e.what());

            m_HasRoughnessTexture = false;
        }
    }

    
    void VyMaterial::loadMetallicMap(const String& filepath) 
    {
        try 
        {
            createTextureImage(filepath, m_MetallicTextureImage);
            createTextureImageView(m_MetallicTextureImage, m_MetallicTextureImageView);
            createTextureSampler(m_MetallicTextureSampler);

            m_HasMetallicTexture = true;
        } 
        catch (const std::exception& e) 
        {
            VY_ERROR_TAG("VyMaterial", "Failed to load Metallic Texture: {}\n{}", filepath, e.what());

            m_HasMetallicTexture = false;
        }
    }


    void VyMaterial::createTextureImage(const String& filepath, VyImage& image) 
    {
        String fullPath = ASSETS_DIR + filepath;
        
		int stbWidth    = 0;
		int stbHeight   = 0;
		int stbChannels = 0;

		// Load Texture data.
		auto pixels = stbi_load(fullPath.c_str(), &stbWidth, &stbHeight, &stbChannels, STBI_rgb_alpha);
		
		U32 width  = static_cast<U32>(stbWidth);
		U32 height = static_cast<U32>(stbHeight);

		VkDeviceSize imageSize = 4 * static_cast<VkDeviceSize>(stbWidth) * static_cast<VkDeviceSize>(stbHeight);

        if (!pixels)
		{
			VY_THROW_RUNTIME_ERROR("Failed to load texture image: " + fullPath + " - " + stbi_failure_reason());
		}

        // Create staging buffer
        VyBuffer stagingBuffer{ VyBuffer::stagingBuffer(imageSize) };

        stagingBuffer.singleWrite(pixels);

        stbi_image_free(pixels);

        // Create image
        image = VyImage::Builder{}
            .imageType  (VK_IMAGE_TYPE_2D)
            .format     (VK_FORMAT_R8G8B8A8_SRGB)
            .extent     (VkExtent2D{ static_cast<U32>(stbWidth), static_cast<U32>(stbHeight) })
            .mipLevels  (1)
			.arrayLayers(1)
            .tiling     (VK_IMAGE_TILING_OPTIMAL)
            .imageLayout(VK_IMAGE_LAYOUT_UNDEFINED)
            .usage      (VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
            .sampleCount(VK_SAMPLE_COUNT_1_BIT)
            .sharingMode(VK_SHARING_MODE_EXCLUSIVE)
            .memoryUsage(VMA_MEMORY_USAGE_AUTO)
        .build();

        // Transition image layout and copy buffer to image.
        image.copyFrom(stagingBuffer);
    }


    void VyMaterial::createTextureImageView(VyImage& image, VyImageView& imageView) 
    {
        imageView = VyImageView::Builder{}
            .viewType   (VK_IMAGE_VIEW_TYPE_2D)
            .format     (VK_FORMAT_R8G8B8A8_SRGB)
            .aspectMask (VK_IMAGE_ASPECT_COLOR_BIT)
            .mipLevels  (0, 1)
            .arrayLayers(0, 1)
        .build(image);
    }

    
    void VyMaterial::createTextureSampler(VySampler& sampler) 
    {
        sampler = VySampler::Builder{}
            .filters         (VK_FILTER_LINEAR)
            .mipmapMode      (VK_SAMPLER_MIPMAP_MODE_LINEAR)
            .addressMode     (VK_SAMPLER_ADDRESS_MODE_REPEAT)
            .borderColor     (VK_BORDER_COLOR_INT_OPAQUE_BLACK)
			.enableAnisotropy(true)
            .lodRange        (0.0f, 1.0f)
            .mipLodBias      (0.0f)
        .build();
    }


    void VyMaterial::updateDescriptorSet(VyDescriptorSetLayout& setLayout, VyDescriptorPool& pool) 
    {
        // Create descriptor set if not already created.
        if (m_DescriptorSet == VK_NULL_HANDLE) 
        {
            if (!pool.allocateDescriptorSet(setLayout.handle(), m_DescriptorSet)) 
            {
                VY_THROW_RUNTIME_ERROR("Failed to allocate material descriptor set!");
            }
        }

        VkDescriptorImageInfo albedoImageInfo{};
        {
            albedoImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            albedoImageInfo.imageView   = m_HasAlbedoTexture ? m_AlbedoTextureImageView.handle() : m_DefaultTextureImageView.handle();
            albedoImageInfo.sampler     = m_HasAlbedoTexture ? m_AlbedoTextureSampler  .handle() : m_DefaultTextureSampler  .handle();
        }

        VkDescriptorImageInfo normalImageInfo{};
        {
            normalImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            normalImageInfo.imageView   = m_HasNormalTexture ? m_NormalTextureImageView.handle() : m_DefaultTextureImageView.handle();
            normalImageInfo.sampler     = m_HasNormalTexture ? m_NormalTextureSampler  .handle() : m_DefaultTextureSampler  .handle();
        }

        VkDescriptorImageInfo roughnessImageInfo{};
        {
            roughnessImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            roughnessImageInfo.imageView   = m_HasRoughnessTexture ? m_RoughnessTextureImageView.handle() : m_DefaultTextureImageView.handle();
            roughnessImageInfo.sampler     = m_HasRoughnessTexture ? m_RoughnessTextureSampler  .handle() : m_DefaultTextureSampler  .handle();
        }

        VkDescriptorImageInfo metallicImageInfo{};
        {
            metallicImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            metallicImageInfo.imageView   = m_HasMetallicTexture ? m_MetallicTextureImageView.handle() : m_DefaultTextureImageView.handle();
            metallicImageInfo.sampler     = m_HasMetallicTexture ? m_MetallicTextureSampler  .handle() : m_DefaultTextureSampler  .handle();
        }

        VyDescriptorWriter{ setLayout, pool }
            .writeImage(0, &albedoImageInfo)
            .writeImage(1, &normalImageInfo)
            .writeImage(2, &roughnessImageInfo)
            .writeImage(3, &metallicImageInfo)
        .update(m_DescriptorSet);
    }
}