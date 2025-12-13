#include <Vy/GFX/Resources/Texture.h>

#include <Vy/GFX/Context.h>

// #include <Vy/GFX/Backend/Pipeline.h>
// #include <Vy/GFX/Backend/Descriptors.h>
#include <Vy/GFX/Backend/Resources/Buffer.h>

#include <Vy/Engine.h>
#include <Vy/Globals.h>
// #include <VylIB/Util/Hash.h>

// #define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace Vy
{
	VySampledTexture::VySampledTexture(const String& filepath, bool srgb, bool flipY)
	{
		// Load image using stb_image
		int texChannels;

		if (flipY)
		{
			stbi_set_flip_vertically_on_load(true);
		}

		stbi_uc* pixels = stbi_load(filepath.c_str(), &m_Width, &m_Height, &texChannels, STBI_rgb_alpha);

		if (flipY)
		{
			stbi_set_flip_vertically_on_load(false);
		}

		if (!pixels)
		{
			throw std::runtime_error("Failed to load texture image: " + filepath);
		}

		VkDeviceSize imageSize = m_Width * m_Height * 4; // RGBA

		// Calculate mip levels
		m_MipLevels = static_cast<U32>(std::floor(std::log2(std::max(m_Width, m_Height)))) + 1;

		// Create staging buffer
		VyBuffer stagingBuffer{ VyBuffer::stagingBuffer(imageSize) };

		stagingBuffer.map();
		stagingBuffer.writeToBuffer(pixels);
		stagingBuffer.unmap();

		stbi_image_free(pixels);

		// Choose format based on whether this is an sRGB texture (color) or linear (data)
		VkFormat format = srgb ? VK_FORMAT_R8G8B8A8_SRGB : VK_FORMAT_R8G8B8A8_UNORM;

		// Create Vulkan image
        m_Image = VyImage::Builder{}
            .imageType  (VK_IMAGE_TYPE_2D)
            .format     (format)
            .extent     (VkExtent2D{ static_cast<U32>(m_Width), static_cast<U32>(m_Height) })
            .mipLevels  (m_MipLevels)
			.arrayLayers(1)
            .tiling     (VK_IMAGE_TILING_OPTIMAL)
            .usage      (VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
            .memoryUsage(VMA_MEMORY_USAGE_AUTO)
        .build();

		// Transition image layout and copy buffer to image
		transitionImageLayout(m_Image.handle(), format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, m_MipLevels);
		copyBufferToImage(stagingBuffer.handle(), m_Image.handle(), static_cast<U32>(m_Width), static_cast<U32>(m_Height));

		// Generate mipmaps (this also transitions to SHADER_READ_ONLY_OPTIMAL)
		generateMipmaps(m_Image, format, m_Width, m_Height, m_MipLevels);

		// Create image view and sampler
		createImageView(format);
		createSampler();

		VY_INFO_TAG("VySampledTexture", "Loaded: {0} ({1}x{2}, {3} mips)", filepath, m_Width, m_Height, m_MipLevels);
	}


	VySampledTexture::~VySampledTexture()
	{
	}


	// Private constructor for creating textures from memory
	VySampledTexture::VySampledTexture(const unsigned char* pixels, int width, int height, VkFormat format) : 
		m_Width { width  }, 
		m_Height{ height }
	{
		VkDeviceSize imageSize = m_Width * m_Height * 4; // RGBA

		m_MipLevels = 1; // No mipmaps for default textures

		// Create staging buffer
		VyBuffer stagingBuffer{ VyBuffer::stagingBuffer(imageSize) };

		stagingBuffer.map();
		stagingBuffer.writeToBuffer((void*)pixels);
		stagingBuffer.unmap();

		// Create Vulkan image
        m_Image = VyImage::Builder{}
            .imageType  (VK_IMAGE_TYPE_2D)
            .format     (format)
            .extent     (VkExtent2D{ static_cast<U32>(m_Width), static_cast<U32>(m_Height) })
            .mipLevels  (m_MipLevels)
			.arrayLayers(1)
            .tiling     (VK_IMAGE_TILING_OPTIMAL)
            .usage      (VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
            .memoryUsage(VMA_MEMORY_USAGE_AUTO)
        .build();

		// Transition image layout and copy buffer to image
		transitionImageLayout(m_Image.handle(), format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, m_MipLevels);
		copyBufferToImage(stagingBuffer.handle(), m_Image.handle(), static_cast<U32>(m_Width), static_cast<U32>(m_Height));
		
		transitionImageLayout(m_Image.handle(), format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, m_MipLevels);

		// Create image view and sampler
		createImageView(format);
		createSampler();
	}

	Shared<VySampledTexture> VySampledTexture::createWhiteTexture()
	{
		unsigned char whitePixel[4] = {255, 255, 255, 255};

		return MakeShared<VySampledTexture>(whitePixel, 1, 1, VK_FORMAT_R8G8B8A8_UNORM);
	}

	Shared<VySampledTexture> VySampledTexture::createNormalTexture()
	{
		// Flat normal pointing up in tangent space: (0, 0, 1) -> (128, 128, 255) in RGB
		unsigned char normalPixel[4] = {128, 128, 255, 255};

		return MakeShared<VySampledTexture>(normalPixel, 1, 1, VK_FORMAT_R8G8B8A8_UNORM);
	}


	void VySampledTexture::createImageView(VkFormat format)
	{
        m_View = VyImageView::Builder{}
            .viewType   (VK_IMAGE_VIEW_TYPE_2D)
            .format     (format)
            .aspectMask (VK_IMAGE_ASPECT_COLOR_BIT)
            .mipLevels  (0, m_MipLevels)
            .arrayLayers(0, 1)
        .build(m_Image);
	}


	void VySampledTexture::createSampler()
	{
        m_Sampler = VySampler::Builder{}
            .filters         (VK_FILTER_LINEAR)
            .mipmapMode      (VK_SAMPLER_MIPMAP_MODE_LINEAR)
            .addressMode     (VK_SAMPLER_ADDRESS_MODE_REPEAT)
            .borderColor     (VK_BORDER_COLOR_INT_OPAQUE_BLACK)
			.enableAnisotropy(true)
            .lodRange        (0.0f, static_cast<float>(m_MipLevels))
            .mipLodBias      (0.0f)
        .build();
	}


	void VySampledTexture::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, U32 mipLevels)
	{
		VkCommandBuffer commandBuffer = VyContext::device().beginSingleTimeCommands();

		VkImageMemoryBarrier barrier{};
		barrier.sType                           = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout                       = oldLayout;
		barrier.newLayout                       = newLayout;
		barrier.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
		barrier.image                           = image;
		barrier.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel   = 0;
		barrier.subresourceRange.levelCount     = mipLevels;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount     = 1;

		VkPipelineStageFlags sourceStage;
		VkPipelineStageFlags destinationStage;

		if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
		{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage      = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage      = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else
		{
			throw std::invalid_argument("Unsupported layout transition!");
		}

		vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

		VyContext::device().endSingleTimeCommands(commandBuffer);
	}


	void VySampledTexture::copyBufferToImage(VkBuffer buffer, VkImage image, U32 width, U32 height)
	{
		VkCommandBuffer commandBuffer = VyContext::device().beginSingleTimeCommands();

		VkBufferImageCopy region{};
		region.bufferOffset                    = 0;
		region.bufferRowLength                 = 0;
		region.bufferImageHeight               = 0;
		region.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel       = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount     = 1;
		region.imageOffset                     = {0, 0, 0};
		region.imageExtent                     = {width, height, 1};

		vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

		VyContext::device().endSingleTimeCommands(commandBuffer);
	}


	void VySampledTexture::generateMipmaps(VkImage image, VkFormat format, I32 width, I32 height, U32 mipLevels)
	{
		// Check if image format supports linear blitting
		VkFormatProperties formatProperties;
		vkGetPhysicalDeviceFormatProperties(VyContext::physicalDevice(), format, &formatProperties);

		if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
		{
			throw std::runtime_error("Texture image format does not support linear blitting!");
		}

		VkCommandBuffer commandBuffer = VyContext::device().beginSingleTimeCommands();

		VkImageMemoryBarrier barrier{};
		barrier.sType                           = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.image                           = image;
		barrier.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
		barrier.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount     = 1;
		barrier.subresourceRange.levelCount     = 1;

		I32 mipWidth  = width;
		I32 mipHeight = height;

		for (U32 i = 1; i < mipLevels; i++)
		{
		barrier.subresourceRange.baseMipLevel = i - 1;
		barrier.oldLayout                     = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout                     = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.srcAccessMask                 = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask                 = VK_ACCESS_TRANSFER_READ_BIT;

		vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

		VkImageBlit blit{};
		blit.srcOffsets[0]                 = {0, 0, 0};
		blit.srcOffsets[1]                 = {mipWidth, mipHeight, 1};
		blit.srcSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.srcSubresource.mipLevel       = i - 1;
		blit.srcSubresource.baseArrayLayer = 0;
		blit.srcSubresource.layerCount     = 1;
		blit.dstOffsets[0]                 = {0, 0, 0};
		blit.dstOffsets[1]                 = {mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1};
		blit.dstSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.dstSubresource.mipLevel       = i;
		blit.dstSubresource.baseArrayLayer = 0;
		blit.dstSubresource.layerCount     = 1;

		vkCmdBlitImage(commandBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_LINEAR);

		barrier.oldLayout     = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.newLayout     = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

		if (mipWidth > 1) mipWidth /= 2;
		if (mipHeight > 1) mipHeight /= 2;
		}

		barrier.subresourceRange.baseMipLevel = mipLevels - 1;
		barrier.oldLayout                     = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout                     = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask                 = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask                 = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

		VyContext::device().endSingleTimeCommands(commandBuffer);
	}


	size_t VySampledTexture::memorySize() const
	{
		// Calculate memory for base texture + all mipmaps
		// Format: RGBA8 (4 bytes per pixel) or sRGB8_A8 (also 4 bytes)
		size_t totalSize = 0;
		int    w         = m_Width;
		int    h         = m_Height;

		for (U32 level = 0; level < m_MipLevels; ++level)
		{
			totalSize += w * h * 4; // 4 bytes per pixel (RGBA8)
			w = std::max(1, w / 2);
			h = std::max(1, h / 2);
		}

		return totalSize;
	}
}










namespace Vy
{
    VyTexture::VyTexture() 
	{
        m_TextureImage      = VK_NULL_HANDLE;
        m_TextureSampler    = VK_NULL_HANDLE;
        m_TextureView       = VK_NULL_HANDLE;
        m_TextureAllocation = VK_NULL_HANDLE;

        m_DescriptorSetLayout = VyDescriptorSetLayout::Builder()
            .addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
            .buildUnique();

        VyContext::allocateSet(m_DescriptorSetLayout->handle(), m_DescriptorSet);
    };

    
	VyTexture::~VyTexture() 
	{
        destroyAll();
    }


    void VyTexture::destroyAll() 
	{
        if (m_TextureView != VK_NULL_HANDLE) 
		{
			VyContext::destroy(m_TextureView);
            
			m_TextureView = VK_NULL_HANDLE;
        }
		
		if (m_TextureSampler != VK_NULL_HANDLE) 
		{
			VyContext::destroy(m_TextureSampler);
			
			m_TextureSampler = VK_NULL_HANDLE;
		}
		
        if (m_TextureImage != VK_NULL_HANDLE) 
		{
			VyContext::destroy(m_TextureImage, m_TextureAllocation);

            m_TextureImage      = VK_NULL_HANDLE;
			m_TextureAllocation = VK_NULL_HANDLE;
        }
    }


    void VyTexture::destroy() 
	{
        if (m_TextureView != VK_NULL_HANDLE) 
		{
			VyContext::destroy(m_TextureView);

            m_TextureView = VK_NULL_HANDLE;
        }
		
		if (m_TextureSampler != VK_NULL_HANDLE) 
		{
			VyContext::destroy(m_TextureSampler);

			m_TextureSampler = VK_NULL_HANDLE;
		}

        if (m_TextureImage != VK_NULL_HANDLE) 
		{
			VyContext::destroy(m_TextureImage, m_TextureAllocation);

            m_TextureImage      = VK_NULL_HANDLE;
			m_TextureAllocation = VK_NULL_HANDLE;
        }

        if (m_DescriptorSet != VK_NULL_HANDLE) 
		{
            TVector<VkDescriptorSet> sets{ m_DescriptorSet };
            VyContext::releaseSets(sets);

            m_DescriptorSet = VK_NULL_HANDLE;
        }

        if (m_DescriptorSetLayout) 
		{
            m_DescriptorSetLayout.reset();
        }
    }


    void VyTexture::createTexture(const String& filepath, VkFormat format) 
	{
        createTextureImage(filepath);
        createTextureImageView(format);
        createTextureSampler();
        writeToDescriptorSet();
    }


    void VyTexture::createTexture(const U32 width, const U32 height, const VkFormat format, const VkImageUsageFlags usage, const VkSampleCountFlagBits samples) 
	{
        createTextureImage(width, height, format, usage, samples);
        createTextureImageView(format);
        createTextureSampler();
        writeToDescriptorSet();
    }


    void VyTexture::createHDRTexture(const String& filepath, VkFormat format) 
	{
        createHDRTextureImage(filepath);
        createHDRTextureImageView(format);
        createHDRTextureSampler();
        writeToDescriptorSet();
    }


    void VyTexture::createCubeMap(U32 width, U32 height, VkFormat format, VkImageUsageFlags usage) 
	{
        createCubemapImage(width, height, format, usage);
        createCubemapImageView(format);
        createCubemapSampler();
        writeToDescriptorSet();
    }


    void VyTexture::writeToDescriptorSet() 
	{
        VkDescriptorImageInfo imageInfo{};
		{
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.imageView   = m_TextureView;
			imageInfo.sampler     = m_TextureSampler;
		}
			
        VyDescriptorWriter writer(*m_DescriptorSetLayout, *VyContext::globalPool());
        writer.writeImage(0, &imageInfo);
        writer.update(m_DescriptorSet);
    }


    VkDescriptorImageInfo VyTexture::descriptorImageInfo() const 
	{
        VkDescriptorImageInfo imageInfo{};
		{
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.imageView   = this->textureImageView();
			imageInfo.sampler     = this->textureSampler();
		}

		return imageInfo;
    }


#pragma region Load
    
    void VyTexture::createTextureImage(const String& filepath) 
	{
        int texWidth, texHeight, texChannels;
        stbi_uc* pixels = stbi_load(filepath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        
		VkDeviceSize imageSize = texWidth * texHeight * 4;

        if (!pixels) 
		{
            VY_THROW_RUNTIME_ERROR("failed to load texture image!");
        }

        VyBuffer stagingBuffer{ VyBuffer::stagingBuffer(imageSize) };

        stagingBuffer.map();
        stagingBuffer.write((void*)pixels);

        stbi_image_free(pixels);

        createImage(texWidth, texHeight,
            VK_FORMAT_R8G8B8A8_UNORM,
            VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            m_TextureImage,
            m_TextureAllocation
		);

        transitionImageLayout(m_TextureImage,
            VK_FORMAT_R8G8B8A8_UNORM,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
		);

        copyBufferToImage(stagingBuffer.handle(),
            m_TextureImage,
            static_cast<U32>(texWidth),
            static_cast<U32>(texHeight)
		);

        stagingBuffer.unmap();
    
        generateMipmap(m_TextureImage, VK_FORMAT_R8G8B8A8_UNORM, texWidth, texHeight, m_MipLevels);
    }


    void VyTexture::createHDRTextureImage(const String& filepath)
	{
        stbi_set_flip_vertically_on_load(false);  // Disable automatic flip by stb_image

        int texWidth, texHeight, texChannels;
        float* pixels = stbi_loadf(filepath.c_str(), &texWidth, &texHeight, &texChannels, 4);
        
		if (!pixels) 
		{
            VY_THROW_RUNTIME_ERROR("failed to load HDR texture image!");
        }

        // fixes the stbi_set_flip_vertically_on_load flipping my UVs
        float* flippedPixels = new float[texWidth * texHeight * 4];

        for (int y = 0; y < texHeight; ++y) 
		{
            for (int x = 0; x < texWidth; ++x) 
			{
                int originalIdx = (y * texWidth + x) * 4;
                int flippedIdx  = ((texHeight - 1 - y) * texWidth + x) * 4;

                flippedPixels[flippedIdx]     = pixels[originalIdx];     // R
                flippedPixels[flippedIdx + 1] = pixels[originalIdx + 1]; // G
                flippedPixels[flippedIdx + 2] = pixels[originalIdx + 2]; // B
                flippedPixels[flippedIdx + 3] = pixels[originalIdx + 3]; // A
            }
        }

        VkDeviceSize imageSize = texWidth * texHeight * 4 * sizeof(float);

        VyBuffer stagingBuffer{ VyBuffer::stagingBuffer(imageSize) };

        // stagingBuffer.map();
        // stagingBuffer.write((void*)flippedPixels);
        // stagingBuffer.unmap();

		stagingBuffer.singleWrite(flippedPixels);

        delete[] flippedPixels;

        stbi_image_free(pixels);

        createImage(texWidth, texHeight,
            VK_FORMAT_R32G32B32A32_SFLOAT,
            VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            m_TextureImage,
            m_TextureAllocation
		);

        transitionImageLayout(m_TextureImage,
            VK_FORMAT_R32G32B32A32_SFLOAT,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        copyBufferToImage(stagingBuffer.handle(),
            m_TextureImage,
            static_cast<U32>(texWidth),
            static_cast<U32>(texHeight));

        generateMipmap(m_TextureImage, 
			VK_FORMAT_R32G32B32A32_SFLOAT, 
			texWidth, 
			texHeight, 
			m_MipLevels
		);
    }

#pragma endregion
	

#pragma region Image

    void VyTexture::createImage(
		U32               width, 
		U32               height, 
		VkFormat          format,
        VkImageTiling     tiling,
        VkImageUsageFlags usage,
        VkImage&          image,
        VmaAllocation&    allocation) 
	{
        m_MipLevels = static_cast<U32>(std::floor(std::log2(std::max(width, height)))) + 1;

        VkImageCreateInfo imageInfo{ VKInit::imageCreateInfo() };
		{
			imageInfo.imageType     = VK_IMAGE_TYPE_2D;
			imageInfo.extent.width  = width;
			imageInfo.extent.height = height;
			imageInfo.extent.depth  = 1;
			imageInfo.mipLevels     = m_MipLevels;
			imageInfo.arrayLayers   = 1;
			imageInfo.format        = format;
			imageInfo.tiling        = tiling;
			imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			imageInfo.usage         = usage;
			imageInfo.samples       = VK_SAMPLE_COUNT_1_BIT;
			imageInfo.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;
		}

		VmaAllocationCreateInfo allocInfo{};
        {
            allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
        }

		VyContext::device().createImage(image, allocation, imageInfo, allocInfo);
    }



    void VyTexture::createTextureImage(U32 width, U32 height, VkFormat format, VkImageUsageFlags usage, VkSampleCountFlagBits samples) 
	{
        m_Format = format;
        
        VkImageCreateInfo imageInfo{ VKInit::imageCreateInfo() };
		{
			imageInfo.imageType     = VK_IMAGE_TYPE_2D;
			imageInfo.extent.width  = width;
			imageInfo.extent.height = height;
			imageInfo.extent.depth  = 1;
			imageInfo.mipLevels     = 1;
			imageInfo.arrayLayers   = 1;
			imageInfo.format        = format;
			imageInfo.tiling        = VK_IMAGE_TILING_OPTIMAL;
			imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			imageInfo.usage         = usage | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
			imageInfo.samples       = samples;
			imageInfo.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;
		}

		VmaAllocationCreateInfo allocInfo{};
        {
            allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
        }

		VyContext::device().createImage(m_TextureImage, m_TextureAllocation, imageInfo, allocInfo);
    }


	void VyTexture::createCubemapImage(U32 width, U32 height, VkFormat format, VkImageUsageFlags usage) 
	{
        m_Format = format;

        VkImageCreateInfo imageInfo{ VKInit::imageCreateInfo() };
		{
			imageInfo.flags         = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
			imageInfo.imageType     = VK_IMAGE_TYPE_2D;
			imageInfo.extent.width  = width;
			imageInfo.extent.height = height;
			imageInfo.extent.depth  = 1;
			imageInfo.mipLevels     = 1;
			imageInfo.arrayLayers   = 6;
			imageInfo.format        = format;
			imageInfo.tiling        = VK_IMAGE_TILING_OPTIMAL;
			imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			imageInfo.usage         = usage | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
			imageInfo.samples       = VK_SAMPLE_COUNT_1_BIT;
			imageInfo.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;
		}

		VmaAllocationCreateInfo allocInfo{};
        {
            allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
        }

		VyContext::device().createImage(m_TextureImage, m_TextureAllocation, imageInfo, allocInfo);

        transitionImageLayout(m_TextureImage, 
			VK_FORMAT_R32G32B32A32_SFLOAT,
			VK_IMAGE_LAYOUT_UNDEFINED, 
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 
			6
		);
    }

#pragma endregion


#pragma region View

    void VyTexture::createTextureImageView(VkFormat format) 
	{
        m_Format = format;

        VkImageViewCreateInfo viewInfo{ VKInit::imageViewCreateInfo() };
		{
			viewInfo.image    = m_TextureImage;
			viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			viewInfo.format   = format;

			viewInfo.subresourceRange.aspectMask     = (format == VK_FORMAT_D32_SFLOAT) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
			viewInfo.subresourceRange.baseMipLevel   = 0;
			viewInfo.subresourceRange.levelCount     = 1;
			viewInfo.subresourceRange.baseArrayLayer = 0;
			viewInfo.subresourceRange.layerCount     = 1;
		}

        if (vkCreateImageView(VyContext::device(), &viewInfo, nullptr, &m_TextureView) != VK_SUCCESS) 
		{
            VY_THROW_RUNTIME_ERROR("failed to create texture image view!");
        }
    }


    void VyTexture::createCubemapImageView(VkFormat format) 
	{
        VkImageViewCreateInfo viewInfo{ VKInit::imageViewCreateInfo() };
		{
			viewInfo.image    = m_TextureImage;
			viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE; // CUBEMAP view type
			viewInfo.format   = format;

			viewInfo.subresourceRange.aspectMask     = (format == VK_FORMAT_D32_SFLOAT) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
			viewInfo.subresourceRange.baseMipLevel   = 0;
			viewInfo.subresourceRange.levelCount     = 1;
			viewInfo.subresourceRange.baseArrayLayer = 0;
			viewInfo.subresourceRange.layerCount     = 6;
		}

        if (vkCreateImageView(VyContext::device(), &viewInfo, nullptr, &m_TextureView) != VK_SUCCESS) 
		{
            VY_THROW_RUNTIME_ERROR("failed to create cubemap image view!");
        }
    }


    void VyTexture::createHDRTextureImageView(VkFormat format) 
	{
        m_Format = format;

        VkImageViewCreateInfo viewInfo{ VKInit::imageViewCreateInfo() };
		{
			viewInfo.image    = m_TextureImage;
			viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			viewInfo.format   = format;

			viewInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
			viewInfo.subresourceRange.baseMipLevel   = 0;
			viewInfo.subresourceRange.levelCount     = 1;
			viewInfo.subresourceRange.baseArrayLayer = 0;
			viewInfo.subresourceRange.layerCount     = 1;
		}

        if (vkCreateImageView(VyContext::device(), &viewInfo, nullptr, &m_TextureView) != VK_SUCCESS) 
		{
            VY_THROW_RUNTIME_ERROR("failed to create HDR texture image view!");
        }
    }

#pragma endregion


#pragma region Sampler
    
	void VyTexture::createTextureSampler() 
	{
        VkPhysicalDeviceProperties properties = VyContext::device().properties();

        VkSamplerCreateInfo samplerInfo{ VKInit::samplerCreateInfo() };
		{
			samplerInfo.magFilter               = VK_FILTER_LINEAR;
			samplerInfo.minFilter               = VK_FILTER_LINEAR;
			samplerInfo.addressModeU            = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			samplerInfo.addressModeV            = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			samplerInfo.addressModeW            = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			samplerInfo.anisotropyEnable        = VK_TRUE;
			samplerInfo.maxAnisotropy           = properties.limits.maxSamplerAnisotropy;
			samplerInfo.borderColor             = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
			samplerInfo.unnormalizedCoordinates = VK_FALSE;
			samplerInfo.compareEnable           = VK_FALSE;
			samplerInfo.compareOp               = VK_COMPARE_OP_ALWAYS;
			samplerInfo.mipmapMode              = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		}

        if (vkCreateSampler(VyContext::device(), &samplerInfo, nullptr, &m_TextureSampler) != VK_SUCCESS) 
		{
            VY_THROW_RUNTIME_ERROR("failed to create texture sampler!");
        }
    }


	void VyTexture::createCubemapSampler() 
	{
        VkPhysicalDeviceProperties properties = VyContext::device().properties();

        VkSamplerCreateInfo samplerInfo{ VKInit::samplerCreateInfo() };
		{
			samplerInfo.magFilter               = VK_FILTER_LINEAR;
			samplerInfo.minFilter               = VK_FILTER_LINEAR;
			samplerInfo.addressModeU            = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			samplerInfo.addressModeV            = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			samplerInfo.addressModeW            = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			samplerInfo.anisotropyEnable        = VK_TRUE;
			samplerInfo.maxAnisotropy           = properties.limits.maxSamplerAnisotropy;
			samplerInfo.borderColor             = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
			samplerInfo.unnormalizedCoordinates = VK_FALSE;
			samplerInfo.compareEnable           = VK_FALSE;
			samplerInfo.compareOp               = VK_COMPARE_OP_ALWAYS;
			samplerInfo.mipmapMode              = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		}

        if (vkCreateSampler(VyContext::device(), &samplerInfo, nullptr, &m_TextureSampler) != VK_SUCCESS) 
		{
            VY_THROW_RUNTIME_ERROR("failed to create cubemap sampler!");
        }
    }


	void VyTexture::createHDRTextureSampler() 
	{
        VkPhysicalDeviceProperties properties = VyContext::device().properties();

        VkSamplerCreateInfo samplerInfo{ VKInit::samplerCreateInfo() };
		{
			samplerInfo.magFilter               = VK_FILTER_LINEAR;
			samplerInfo.minFilter               = VK_FILTER_LINEAR;
			samplerInfo.addressModeU            = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			samplerInfo.addressModeV            = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			samplerInfo.addressModeW            = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			samplerInfo.anisotropyEnable        = VK_TRUE;
			samplerInfo.maxAnisotropy           = properties.limits.maxSamplerAnisotropy;
			samplerInfo.borderColor             = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
			samplerInfo.unnormalizedCoordinates = VK_FALSE;
			samplerInfo.compareEnable           = VK_FALSE;
			samplerInfo.compareOp               = VK_COMPARE_OP_ALWAYS;
			samplerInfo.mipmapMode              = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		}

        if (vkCreateSampler(VyContext::device(), &samplerInfo, nullptr, &m_TextureSampler) != VK_SUCCESS) 
		{
            VY_THROW_RUNTIME_ERROR("failed to create HDR texture sampler!");
        }
    }

#pragma endregion
    

#pragma region Color

    void VyTexture::createSolidColorTexture(Vec4 color) 
	{
        U8 pixelData[4] = {
            static_cast<U8>(color.r * 255.0f),
            static_cast<U8>(color.g * 255.0f),
            static_cast<U8>(color.b * 255.0f),
            static_cast<U8>(color.a * 255.0f)
        };

        VkDeviceSize imageSize = sizeof(pixelData);

        VyBuffer stagingBuffer{ VyBuffer::stagingBuffer(imageSize) };

        stagingBuffer.map();
        stagingBuffer.write((void*)pixelData);

        createImage(1, 1, 
			VK_FORMAT_R8G8B8A8_UNORM,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			// VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			m_TextureImage,
			m_TextureAllocation
		);

        transitionImageLayout(m_TextureImage, 
			VK_FORMAT_R8G8B8A8_UNORM,
			VK_IMAGE_LAYOUT_UNDEFINED, 
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
		);

        copyBufferToImage(stagingBuffer.handle(), m_TextureImage, 1, 1);

        transitionImageLayout(m_TextureImage, 
			VK_FORMAT_R8G8B8A8_UNORM,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
		);

        stagingBuffer.unmap();

        createTextureImageView(VK_FORMAT_R8G8B8A8_UNORM);

        createTextureSampler();
    }

    
	void VyTexture::createSolidColorCubemap(Vec4 color) 
	{
        const int kTextureSize = 1;

        U8 pixelData[4] = {
            static_cast<U8>(color.r * 255.0f),
            static_cast<U8>(color.g * 255.0f),
            static_cast<U8>(color.b * 255.0f),
            static_cast<U8>(color.a * 255.0f)
        };

        VkDeviceSize imageSize = sizeof(pixelData) * 6;

        VyBuffer stagingBuffer{ VyBuffer::stagingBuffer(imageSize) };

        stagingBuffer.map();

        for (int i = 0; i < 6; ++i) 
		{
            stagingBuffer.write((void*)pixelData);
        }

        createCubemapImage(kTextureSize, kTextureSize, 
			VK_FORMAT_R32G32B32A32_SFLOAT,
			VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT
		);

        transitionImageLayout(m_TextureImage, 
			VK_FORMAT_R32G32B32A32_SFLOAT,
			VK_IMAGE_LAYOUT_UNDEFINED, 
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
			6
		);

        copyBufferToImage(stagingBuffer.handle(), 
			m_TextureImage, 
			kTextureSize, 
			kTextureSize
		);

        transitionImageLayout(m_TextureImage, 
			VK_FORMAT_R32G32B32A32_SFLOAT,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 
			6
		);

        stagingBuffer.unmap();

        createCubemapImageView(VK_FORMAT_R32G32B32A32_SFLOAT);

        createCubemapSampler();
    }


    void VyTexture::UseFallbackTextures(TextureType type) 
	{
        switch (type) 
		{
            case TextureType::Albedo:
                createSolidColorTexture({1.0f, 1.0f, 1.0f, 1.0f});
                break;
            case TextureType::Normal:
                createSolidColorTexture({0.5f, 0.5f, 1.0f, 1.0f});
                break;
            case TextureType::Emissive:
                createSolidColorTexture({1.0f, 1.0f, 1.0f, 1.0f});
                break;
            case TextureType::MetallicRoughness:
                createSolidColorTexture({1.0f, 0.5f, 0.0f, 1.0f});
                break;
            case TextureType::AO:
                createSolidColorTexture({1.0f, 1.0f, 1.0f, 1.0f});
                break;
            case TextureType::Cubemap:
                createSolidColorCubemap({1.0f, 1.0f, 1.0f, 1.0f});
            default:
                break;
        }

        writeToDescriptorSet();
    }

#pragma endregion


#pragma region Mipmap

    void VyTexture::generateMipmap(VkImage image, VkFormat imageFormat, I32 texWidth, I32 texHeight, U32 mipLevels)
	{
		VkFormatProperties formatProperties;
        vkGetPhysicalDeviceFormatProperties(VyContext::physicalDevice(), imageFormat, &formatProperties);
        
		if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) 
		{
            VY_THROW_RUNTIME_ERROR("Texture image format does not support linear blitting!");
        }

        VkCommandBuffer cmdBuffer = VyContext::device().beginSingleTimeCommands();
		{
			VkImageMemoryBarrier barrier{ VKInit::imageMemoryBarrier() };
			{
				barrier.image                           = image;
				barrier.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
				barrier.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
				barrier.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
				barrier.subresourceRange.baseArrayLayer = 0;
				barrier.subresourceRange.layerCount     = 1;
				barrier.subresourceRange.levelCount     = 1;
			}

			I32 mipWidth  = texWidth;
			I32 mipHeight = texHeight;

			for (U32 i = 1; i < mipLevels; i++) 
			{
				barrier.subresourceRange.baseMipLevel = i - 1;

				barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
				barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
				
				barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

				vkCmdPipelineBarrier(cmdBuffer,
					VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
					0, nullptr,
					0, nullptr,
					1, &barrier
				);

				VkImageBlit blit{};
				{
					blit.srcOffsets[0] = {0, 0, 0};
					blit.srcOffsets[1] = {mipWidth, mipHeight, 1};
					blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
					blit.srcSubresource.mipLevel = i - 1;
					blit.srcSubresource.baseArrayLayer = 0;
					blit.srcSubresource.layerCount = 1;

					blit.dstOffsets[0] = {0, 0, 0};
					blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
					blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
					blit.dstSubresource.mipLevel = i;
					blit.dstSubresource.baseArrayLayer = 0;
					blit.dstSubresource.layerCount = 1;
				}

				vkCmdBlitImage(cmdBuffer,
					image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
					image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					1, &blit,
					VK_FILTER_LINEAR
				);

				barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
				barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

				barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
				barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

				vkCmdPipelineBarrier(cmdBuffer,
					VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
					0, nullptr,
					0, nullptr,
					1, &barrier
				);

				if (mipWidth  > 1) mipWidth  /= 2;
				if (mipHeight > 1) mipHeight /= 2;
			}

			barrier.subresourceRange.baseMipLevel = mipLevels - 1;

			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			vkCmdPipelineBarrier(cmdBuffer,
				VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
				0, nullptr,
				0, nullptr,
				1, &barrier
			);
		}
        VyContext::device().endSingleTimeCommands(cmdBuffer);
    }


    void VyTexture::createMipMappedCubemap(U32 width, U32 height, VkFormat format, VkImageUsageFlags usage) 
	{
        m_MipLevels = static_cast<U32>(std::floor(std::log2(std::max(width, height)))) + 1;
        m_Format    = format;

        VkImageCreateInfo imageInfo{ VKInit::imageCreateInfo() };
		{
			imageInfo.imageType     = VK_IMAGE_TYPE_2D;
			imageInfo.format        = VK_FORMAT_R32G32B32A32_SFLOAT;
			imageInfo.extent.width  = width;
			imageInfo.extent.height = height;
			imageInfo.extent.depth  = 1;
			imageInfo.mipLevels     = m_MipLevels;
			imageInfo.arrayLayers   = 6;
			imageInfo.samples       = VK_SAMPLE_COUNT_1_BIT;
			imageInfo.tiling        = VK_IMAGE_TILING_OPTIMAL;
			imageInfo.usage         = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
			imageInfo.flags         = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
			imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		}
			
		VmaAllocationCreateInfo allocInfo{};
        {
            allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
        }

		VyContext::device().createImage(m_TextureImage, m_TextureAllocation, imageInfo, allocInfo);


        VkImageViewCreateInfo viewInfo{ VKInit::imageViewCreateInfo() };
		{
			viewInfo.image    = m_TextureImage;
			viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
			viewInfo.format   = VK_FORMAT_R32G32B32A32_SFLOAT;
			
			viewInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
			viewInfo.subresourceRange.baseMipLevel   = 0;
			viewInfo.subresourceRange.levelCount     = m_MipLevels;
			viewInfo.subresourceRange.baseArrayLayer = 0;
			viewInfo.subresourceRange.layerCount     = 6;
		}

        if (vkCreateImageView(VyContext::device(), &viewInfo, nullptr, &m_TextureView) != VK_SUCCESS) 
		{
            VY_THROW_RUNTIME_ERROR("Failed to create cubemap image view!");
        }

        VkSamplerCreateInfo samplerInfo{ VKInit::samplerCreateInfo() };
		{
			samplerInfo.magFilter        = VK_FILTER_LINEAR;
			samplerInfo.minFilter        = VK_FILTER_LINEAR;
			samplerInfo.mipmapMode       = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			samplerInfo.addressModeU     = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			samplerInfo.addressModeV     = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			samplerInfo.addressModeW     = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			samplerInfo.minLod           = 0.0f;
			samplerInfo.maxLod           = static_cast<float>(m_MipLevels);
			samplerInfo.mipLodBias       = 0.0f;
			samplerInfo.anisotropyEnable = VK_FALSE;
		}

        if (vkCreateSampler(VyContext::device(), &samplerInfo, nullptr, &m_TextureSampler) != VK_SUCCESS) 
		{
            VY_THROW_RUNTIME_ERROR("Failed to create cubemap sampler!");
        }

        writeToDescriptorSet();

        VY_INFO("Created Mipmap Cubemap");
    }

#pragma endregion


#pragma region Util

	void VyTexture::copyBufferToImage(VkBuffer buffer, VkImage image, U32 width, U32 height) const 
	{
        VkCommandBuffer cmdBuffer = VyContext::device().beginSingleTimeCommands();
		{
			VkBufferImageCopy region{};
			{
				region.bufferOffset      = 0;
				region.bufferRowLength   = 0;
				region.bufferImageHeight = 0;

				region.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
				region.imageSubresource.mipLevel       = 0;
				region.imageSubresource.baseArrayLayer = 0;
				region.imageSubresource.layerCount     = 1;

				region.imageOffset = { 0, 0, 0 };
				region.imageExtent = { width, height, 1 };
			}

			vkCmdCopyBufferToImage(cmdBuffer, 
				buffer, 
				image, 
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
				1, 
				&region
			);
		}
        VyContext::device().endSingleTimeCommands(cmdBuffer);
    }


    void VyTexture::transitionImageLayout(
        VkImage       image,
        VkFormat      format,
        VkImageLayout oldLayout,
        VkImageLayout newLayout,
        U32           layers)
    {
        VkCommandBuffer cmdBuffer = VyContext::device().beginSingleTimeCommands();
		{
			VkImageMemoryBarrier barrier{ VKInit::imageMemoryBarrier() };
			{
				barrier.image               = image;
				barrier.oldLayout           = oldLayout;
				barrier.newLayout           = newLayout;
				barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

				barrier.subresourceRange.aspectMask     = VyContext::device().findAspectFlags(format);
				barrier.subresourceRange.baseMipLevel   = 0;
				barrier.subresourceRange.levelCount     = m_MipLevels;
				barrier.subresourceRange.baseArrayLayer = 0;
				barrier.subresourceRange.layerCount     = layers;
			}

			VkPipelineStageFlags srcStage;
			VkPipelineStageFlags dstStage;

			switch (oldLayout) 
			{
				case VK_IMAGE_LAYOUT_UNDEFINED:
					barrier.srcAccessMask = 0;
					srcStage              = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
					break;

				case VK_IMAGE_LAYOUT_PREINITIALIZED:
					barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
					srcStage              = VK_PIPELINE_STAGE_HOST_BIT;
					break;

				case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
					barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
					srcStage              = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
					break;

				case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
					barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
					srcStage              = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
					break;

				case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
					barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
					srcStage              = VK_PIPELINE_STAGE_TRANSFER_BIT;
					break;

				case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
					barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
					srcStage              = VK_PIPELINE_STAGE_TRANSFER_BIT;
					break;

				case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
					barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
					srcStage              = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
					break;
				default:
					VY_THROW_INVALID_ARGUMENT("unsupported layout transition!");
			}

			switch (newLayout) 
			{
				case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
					barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
					dstStage              = VK_PIPELINE_STAGE_TRANSFER_BIT;
					break;

				case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
					barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
					dstStage              = VK_PIPELINE_STAGE_TRANSFER_BIT;
					break;

				case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
					barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
					dstStage              = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
					break;

				case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
					barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
					dstStage              = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
					break;

				case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
					barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
					dstStage              = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
					break;

				default:
					VY_THROW_INVALID_ARGUMENT("unsupported layout transition!");
			}

			vkCmdPipelineBarrier(
				cmdBuffer,
				srcStage, dstStage,
				0,
				0, nullptr,
				0, nullptr,
				1, &barrier
			);
		}
        VyContext::device().endSingleTimeCommands(cmdBuffer);
    }

#pragma endregion
}

















// namespace Vy
// {
// 	bool hasStencilComponent(VkFormat format) 
// 	{
// 		return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
// 	}
// }

// namespace Vy
// {
// 	void Texture::reload()
// 	{
// 		if (!m_Handle.isValid())
// 		{
// 			return;
// 		}

// 		destroy();
// 		initFromPath();
// 	}


// 	Texture& Texture::operator=(Texture&& other) noexcept
// 	{
// 		std::swap(*this, other);

// 		return *this;
// 	}


// 	Texture::Texture(Texture&& other) noexcept : 
// 		VyAsset     { other.assetHandle()           }, 
// 		m_Image     { other.m_Image                 }, 
// 		m_ImageView { other.m_ImageView             },
// 		m_Allocation{ other.m_Allocation            }, 
// 		m_Sampler   { other.m_Sampler               },
// 		m_LayerCount{ other.m_LayerCount            }, 
// 		m_CreateInfo{ std::move(other.m_CreateInfo) }
// 	{
// 		other.m_Image      = VK_NULL_HANDLE;
// 		other.m_ImageView  = VK_NULL_HANDLE;
// 		other.m_Allocation = VK_NULL_HANDLE;
// 		other.m_Sampler    = VK_NULL_HANDLE;
// 		other.m_LayerCount = 0;
// 	}


// 	Texture::Texture(TextureCreateInfo info, VyAssetHandle assetHandle) : 
// 		VyAsset     { std::move(assetHandle) }, 
// 		m_CreateInfo{ info                   }
// 	{
// 		if (this->assetHandle().isValid()) 
// 		{
// 			initFromPath();
// 			return;
// 		}

// 		createImage(m_CreateInfo.Format, m_CreateInfo.UsageFlags, m_CreateInfo.AspectFlags);

// 		if (m_CreateInfo.Data) 
// 		{
// 			const VkDeviceSize imageSize = static_cast<VkDeviceSize>(m_CreateInfo.Extent.width) * static_cast<VkDeviceSize>(m_CreateInfo.Extent.height) * 4;

// 			// Create staging buffer
// 			VyBuffer stagingBuffer{ VyBuffer::stagingBuffer(imageSize) };

// 			stagingBuffer.singleWrite(m_CreateInfo.Data);

// 			transitionImageLayout(m_Image, 
// 				VK_FORMAT_R8G8B8A8_UNORM, 
// 				VK_IMAGE_ASPECT_COLOR_BIT, 
// 				VK_IMAGE_LAYOUT_UNDEFINED,           // Old Layout
// 				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL // New Layout
// 			);
			
// 			if (m_CreateInfo.Mode == TextureMode::CubeMap) 
// 			{
// 				VyContext::device().copyBufferToImageCubemap(
// 					stagingBuffer.handle(), 
// 					m_Image, 
// 					m_CreateInfo.Extent.width, 
// 					m_CreateInfo.Extent.height, 
// 					1
// 				);
// 			} 
// 			else 
// 			{
// 				VyContext::device().copyBufferToImage(
// 					stagingBuffer.handle(), 
// 					m_Image, 
// 					m_CreateInfo.Extent.width, 
// 					m_CreateInfo.Extent.height, 
// 					1
// 				);
// 			}

// 			transitionImageLayout(m_Image, 
// 				VK_FORMAT_R8G8B8A8_UNORM, 
// 				VK_IMAGE_ASPECT_COLOR_BIT, 
// 				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,    // Old Layout
// 				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL // New Layout
// 			);

// 			// We are not supposed to use that anymore, it is not managed by the texture.
// 			m_CreateInfo.Data = nullptr;
// 		}
// 	}


// 	Texture::~Texture()
// 	{
// 		destroy();
// 	}


// 	void Texture::initFromPath()
// 	{
// 		auto path = VyEngine::assetManager().absolutePathFromHandle(this->assetHandle());

// 		VY_INFO_TAG("VyTexture", "Loading texture from path `{0}`", path.string());

// 		int texWidth   {};
// 		int texHeight  {};
// 		int texChannels{};

// 		stbi_uc* pixels = stbi_load(path.string().c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

// 		unsigned char errorColor[] = { 255, 0, 255, 255 };

// 		if (!pixels)
// 		{
// 			auto* error = stbi_failure_reason();

// 			VY_ERROR_TAG("VyTexture", "Error: `{0}` at path `{1}`", error, path.string());
			
// 			texWidth  = 1;
// 			texHeight = 1;
// 			pixels    = errorColor;
// 		}

// 		m_CreateInfo.Extent = {static_cast<U32>(texWidth), static_cast<U32>(texHeight)};
		
// 		createImage(
// 			VK_FORMAT_R8G8B8A8_UNORM, 
// 			VK_IMAGE_USAGE_TRANSFER_DST_BIT, 
// 			VK_IMAGE_ASPECT_COLOR_BIT
// 		);

// 		const auto imageSize = static_cast<VkDeviceSize>(texWidth * texHeight * 4);

// 		// Create staging buffer
// 		VyBuffer stagingBuffer{ VyBuffer::stagingBuffer(imageSize) };

// 		stagingBuffer.singleWrite(pixels);

// 		if (memcmp(pixels, errorColor, sizeof(errorColor)) != 0)
// 		{
// 			stbi_image_free(pixels);
// 		}

// 		transitionImageLayout(m_Image, 
// 			VK_FORMAT_R8G8B8A8_UNORM, 
// 			VK_IMAGE_ASPECT_COLOR_BIT, 
// 			VK_IMAGE_LAYOUT_UNDEFINED,           // Old Layout
// 			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL // New Layout
// 		);
		
// 		if (m_CreateInfo.Mode == TextureMode::CubeMap) 
// 		{
// 			VyContext::device().copyBufferToImageCubemap(
// 				stagingBuffer.handle(), 
// 				m_Image, 
// 				m_CreateInfo.Extent.width, 
// 				m_CreateInfo.Extent.height, 
// 				1
// 			);
// 		} 
// 		else 
// 		{
// 			VyContext::device().copyBufferToImage(
// 				stagingBuffer.handle(), 
// 				m_Image, 
// 				m_CreateInfo.Extent.width, 
// 				m_CreateInfo.Extent.height, 
// 				1
// 			);
// 		}

// 		transitionImageLayout(m_Image, 
// 			VK_FORMAT_R8G8B8A8_UNORM, 
// 			VK_IMAGE_ASPECT_COLOR_BIT,           
// 			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,    // Old Layout
// 			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL // New Layout
// 		);
// 	}


// 	void Texture::destroy()
// 	{
// 		vkDeviceWaitIdle(VyContext::device());

// 		VyContext::destroy(m_Sampler);
// 		VyContext::destroy(m_ImageView);    
// 		VyContext::destroy(m_Image, m_Allocation);
// 	}


// 	void Texture::createImage(
// 		VkFormat           format, 
// 		VkImageUsageFlags  usageFlags, 
// 		VkImageAspectFlags aspectFlags)
// 	{
// 		m_CreateInfo.Extent.height = m_CreateInfo.Mode == TextureMode::CubeMap
// 			? m_CreateInfo.Extent.width
// 			: m_CreateInfo.Extent.height;

// 		m_LayerCount = m_CreateInfo.Mode == TextureMode::CubeMap ? 6 : 1;

// 		VkImageCreateInfo imageInfo{ VKInit::imageCreateInfo() };
// 		{
// 			imageInfo.imageType     = VK_IMAGE_TYPE_2D;
// 			imageInfo.extent.width  = m_CreateInfo.Extent.width;
// 			imageInfo.extent.height = m_CreateInfo.Extent.height;
// 			imageInfo.extent.depth  = 1;
// 			imageInfo.mipLevels     = 1;
// 			imageInfo.arrayLayers   = m_LayerCount;
// 			imageInfo.format        = format;
// 			imageInfo.tiling        = VK_IMAGE_TILING_OPTIMAL;
// 			imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
// 			imageInfo.usage         = usageFlags | VK_IMAGE_USAGE_SAMPLED_BIT;
// 			imageInfo.samples       = VK_SAMPLE_COUNT_1_BIT;
// 			imageInfo.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;
// 			imageInfo.flags         = m_CreateInfo.Mode == TextureMode::CubeMap 
// 				? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT 
// 				: 0;
// 		}

// 		VmaAllocationCreateInfo allocInfo{};
// 		{
// 			allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
// 		}

// 		VyContext::device().createImage(m_Image, m_Allocation, imageInfo, allocInfo);

// 		createImageView(format, aspectFlags);
// 		createTextureSampler();
// 	}


// 	void Texture::transitionImageLayout(
// 		VkImage            image, 
// 		VkFormat           format, 
// 		VkImageAspectFlags aspectFlags, 
// 		VkImageLayout      oldLayout, 
// 		VkImageLayout      newLayout) const
// 	{
// 		VkCommandBuffer cmdBuffer = VyContext::device().beginSingleTimeCommands();
// 		{
// 			VkImageMemoryBarrier barrier{ VKInit::imageMemoryBarrier() };

// 			barrier.image               = image;

// 			barrier.oldLayout           = oldLayout;
// 			barrier.newLayout           = newLayout;
			
// 			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
// 			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			
// 			barrier.subresourceRange.aspectMask     = aspectFlags;
// 			barrier.subresourceRange.baseMipLevel   = 0;
// 			barrier.subresourceRange.levelCount     = 1;
// 			barrier.subresourceRange.baseArrayLayer = 0;
// 			barrier.subresourceRange.layerCount     = m_LayerCount;

// 			if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) 
// 			{
// 				barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

// 				if (hasStencilComponent(format)) 
// 				{
// 					barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
// 				}
// 			}

// 			VkPipelineStageFlags srcStage;
// 			VkPipelineStageFlags dstStage;

// 			if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && 
// 				newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) 
// 			{
// 				barrier.srcAccessMask = 0;
// 				barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

// 				srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
// 				dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
// 			}

// 			else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && 
// 					 newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) 
// 			{
// 				barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
// 				barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

// 				srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
// 				dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
// 			}

// 			else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && 
// 					 newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) 
// 			{
// 				barrier.srcAccessMask = 0;
// 				barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

// 				srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
// 				dstStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
// 			}

// 			else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && 
// 					 newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
// 			{
// 				barrier.srcAccessMask = 0;
// 				barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

// 				srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
// 				dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
// 			}
// 			else 
// 			{
// 				VY_THROW_INVALID_ARGUMENT("Vulkan : unsupported layout transition !");
// 			}

// 			vkCmdPipelineBarrier(cmdBuffer,
// 				srcStage, dstStage,
// 				0,
// 				0, nullptr,
// 				0, nullptr,
// 				1, &barrier
// 			);
// 		}
// 		VyContext::device().endSingleTimeCommands(cmdBuffer);
// 	}


// 	void Texture::createImageView(VkFormat format, VkImageAspectFlags aspectFlags)
// 	{
// 		VkImageViewCreateInfo viewInfo{ VKInit::imageViewCreateInfo() };
// 		{
// 			viewInfo.image    = m_Image;
// 			viewInfo.viewType = m_CreateInfo.Mode == TextureMode::CubeMap ? VK_IMAGE_VIEW_TYPE_CUBE : VK_IMAGE_VIEW_TYPE_2D;
// 			viewInfo.format   = format;

// 			viewInfo.subresourceRange.aspectMask     = aspectFlags;
// 			viewInfo.subresourceRange.baseMipLevel   = 0;
// 			viewInfo.subresourceRange.levelCount     = 1;
// 			viewInfo.subresourceRange.baseArrayLayer = 0;
// 			viewInfo.subresourceRange.layerCount     = m_LayerCount;
// 		}

// 		if (vkCreateImageView(VyContext::device(), &viewInfo, nullptr, &m_ImageView) != VK_SUCCESS) 
// 		{
// 			VY_THROW_RUNTIME_ERROR("failed to create texture image view!");
// 		}
// 	}

	
// 	void Texture::createTextureSampler() 
// 	{
// 		VkSamplerCreateInfo samplerInfo{ VKInit::samplerCreateInfo() };
// 		{
// 			samplerInfo.magFilter    = VK_FILTER_LINEAR;
// 			samplerInfo.minFilter    = VK_FILTER_LINEAR;
// 			samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
// 			samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
// 			samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			
// 			samplerInfo.anisotropyEnable = VK_TRUE;
// 			samplerInfo.maxAnisotropy    = VyContext::device().properties().limits.maxSamplerAnisotropy;
			
// 			samplerInfo.borderColor             = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
// 			samplerInfo.unnormalizedCoordinates = VK_FALSE;
			
// 			samplerInfo.compareEnable = VK_FALSE;
// 			samplerInfo.compareOp     = VK_COMPARE_OP_ALWAYS;
			
// 			samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
// 			samplerInfo.mipLodBias = 0.0f;
// 			samplerInfo.minLod     = 0.0f;
// 			samplerInfo.maxLod     = 0.0f;
// 		}
	
// 		if (vkCreateSampler(VyContext::device(), &samplerInfo, nullptr, &m_Sampler) != VK_SUCCESS)
// 		{
// 			VY_THROW_RUNTIME_ERROR("failed to create texture sampler!");
// 		}
// 	}


// 	VkDescriptorImageInfo Texture::descriptorImageInfo() const
// 	{
// 		VkDescriptorImageInfo imageBufferInfo;
// 		{
// 			imageBufferInfo.sampler     = m_Sampler;
// 			imageBufferInfo.imageView   = m_ImageView;
// 			imageBufferInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
// 		}

// 		return imageBufferInfo;
// 	}


// 	Unique<Texture> Texture::Builder::buildUnique() const
// 	{
// 		return MakeUnique<Texture>(m_TextureCreateInfo, m_AssetHandle);
// 	}
// }







	

	// // // #include <future>
// namespace Vy
// {
//     LveTexture::LveTexture(
// 		const String& textureFilepath, 
// 		VkFormat format, 
// 		VkImageViewType viewType, 
// 		VkImageLayout layout)
// 	{
//         createImage(textureFilepath, format, viewType, layout);
//         createView(viewType);
//         createSampler();

//         updateDescriptor();
//     }


//     LveTexture::LveTexture(
//         VkFormat format,
//         VkExtent3D extent,
//         VkImageUsageFlags usage,
//         VkSampleCountFlagBits sampleCount)
//     {
//       VkImageAspectFlags aspectMask = 0;
//       VkImageLayout imageLayout;

// 		m_Format = format;
// 		m_Extent = extent;

// 		if (usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) 
// 		{
// 			aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
// 			imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
// 		}
// 		if (usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) 
// 		{
// 			aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
// 			imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
// 		}

//       VkImageCreateInfo imageInfo{ VKInit::imageCreateInfo() };
//       imageInfo.imageType = VK_IMAGE_TYPE_2D;
//       imageInfo.format = format;
//       imageInfo.extent = extent;
//       imageInfo.mipLevels = 1;
//       imageInfo.arrayLayers = 1;
//       imageInfo.samples = sampleCount;
//       imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
//       imageInfo.usage = usage;
//       imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
//       device.createImageWithInfo(
//           imageInfo,
//           VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
//           m_Image,
//           m_Allocation);

//       VkImageViewCreateInfo viewInfo{ VKInit::imageViewCreateInfo() };
//       viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
//       viewInfo.format = format;
//       viewInfo.subresourceRange = {};
//       viewInfo.subresourceRange.aspectMask = aspectMask;
//       viewInfo.subresourceRange.baseMipLevel = 0;
//       viewInfo.subresourceRange.levelCount = 1;
//       viewInfo.subresourceRange.baseArrayLayer = 0;
//       viewInfo.subresourceRange.layerCount = 1;
//       viewInfo.image = m_Image;
//       if (vkCreateImageView(device.device(), &viewInfo, nullptr, &m_View) != VK_SUCCESS) {
//         VY_THROW_RUNTIME_ERROR("Failed to create texture image view!");
//       }

//       // Sampler should be seperated out
//       if (usage & VK_IMAGE_USAGE_SAMPLED_BIT) {
//         // Create sampler to sample from the attachment in the fragment shader
//         VkSamplerCreateInfo samplerInfo{ VKInit::samplerCreateInfo() };
//         samplerInfo.magFilter = VK_FILTER_LINEAR;
//         samplerInfo.minFilter = VK_FILTER_LINEAR;
//         samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
//         samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
//         samplerInfo.addressModeV = samplerInfo.addressModeU;
//         samplerInfo.addressModeW = samplerInfo.addressModeU;
//         samplerInfo.mipLodBias = 0.0f;
//         samplerInfo.maxAnisotropy = 1.0f;
//         samplerInfo.minLod = 0.0f;
//         samplerInfo.maxLod = 1.0f;
//         samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;

//         if (vkCreateSampler(device.device(), &samplerInfo, nullptr, &m_Sampler) != VK_SUCCESS) {
//           VY_THROW_RUNTIME_ERROR("Failed to create sampler!");
//         }

//         VkImageLayout samplerImageLayout = imageLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
//                                                ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
//                                                : VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
//         m_Descriptor.sampler = m_Sampler;
//         m_Descriptor.imageView = m_View;
//         m_Descriptor.imageLayout = samplerImageLayout;
//       }
//     }

//     LveTexture::~LveTexture() {
//         vkDestroySampler(VyContext::device(), m_Sampler, nullptr);
//         vkDestroyImageView(VyContext::device(), m_View, nullptr);
//         vkDestroyImage(VyContext::device(), m_Image, nullptr);
//         vkFreeMemory(VyContext::device(), m_Allocation, nullptr);
//       }

//     Unique<LveTexture> LveTexture::createTextureFromFile(
// 		const String& filepath,
// 		VkFormat format,
// 		VkImageViewType viewType,
// 		VkImageLayout layout) 
// 	{
//         return std::make_unique<LveTexture>(filepath, format, viewType, layout);
//     }

//     void LveTexture::updateDescriptor() {
//         m_Descriptor.sampler = m_Sampler;
//         m_Descriptor.imageView = m_View;
//         m_Descriptor.imageLayout = m_Layout;
//     }


//     void LveTexture::createImage(const String& filepath, VkFormat format, VkImageViewType viewType, VkImageLayout layout) 
// 	{
//         int texWidth, texHeight, texChannels;
        
//         stbi_uc *pixels;
//         // read as default RGBA
//         if(stbi_is_hdr(filepath.c_str())) {
//             float *tmp = stbi_loadf(filepath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
//             pixels = reinterpret_cast<stbi_uc*>(tmp);
//         }
//         else {
//             pixels = stbi_load(filepath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
//         }
        
//         VkDeviceSize imageSize = texWidth * texHeight * 4;

//         if (!pixels) {
//             VY_THROW_RUNTIME_ERROR("failed to load texture image!");
//         }
    
//         m_MipLevels = 1;
        
//         VyBuffer stagingBuffer{ VyBuffer::stagingBuffer(imageSize) };
		
//         stagingBuffer.map();
// 		stagingBuffer.write(pixels);
//         stagingBuffer.unmap();
        
//         stbi_image_free(pixels);


//         m_Format = format;
//         m_Layout = layout;
//         m_ViewType = viewType;
//         m_Extent = {static_cast<U32>(texWidth), static_cast<U32>(texHeight), 1};

//         // image create info
//         VkImageCreateInfo imageInfo{ VKInit::imageCreateInfo() };
// 		{
// 			imageInfo.imageType = VK_IMAGE_TYPE_2D;
// 			imageInfo.extent = m_Extent;
// 			imageInfo.mipLevels = m_MipLevels;
// 			imageInfo.arrayLayers = m_LayerCount;
// 			imageInfo.format = m_Format;
// 			imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
// 			imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
// 			imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
// 			imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
// 			imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
// 		}

// 		VmaAllocationCreateInfo allocInfo{};
//         {
//             allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
//         }

// 		VyContext::device().createImage(m_Image, m_Allocation, imageInfo, allocInfo);

//         VyContext::device().transitionImageLayout(
//           m_Image,
//           m_Format,
//           VK_IMAGE_LAYOUT_UNDEFINED,
//           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
//           m_MipLevels,
//           m_LayerCount);
        
//         VyContext::device().copyBufferToImage(
//             stagingBuffer,
//             m_Image,
//             static_cast<U32>(texWidth),
//             static_cast<U32>(texHeight),
//             m_LayerCount);
       
//         // comment this out if using mips
//         VyContext::device().transitionImageLayout(
//           m_Image,
//           m_Format,
//           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
//           m_Layout,
//           m_MipLevels,
//           m_LayerCount);
        
//         // If we generate mip maps then the final image will alerady be READ_ONLY_OPTIMAL
//         // mDevice.generateMipmaps(m_Image, m_Format, texWidth, texHeight, m_MipLevels);
//         //m_Layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
//     }


//     void LveTexture::createView(VkImageViewType viewType) 
// 	{
//         VkImageViewCreateInfo viewInfo{ VKInit::imageViewCreateInfo() };
//         viewInfo.image = m_Image;
//         viewInfo.viewType = viewType;
//         viewInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
//         viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//         viewInfo.subresourceRange.baseMipLevel = 0;
//         viewInfo.subresourceRange.levelCount = m_MipLevels;
//         viewInfo.subresourceRange.baseArrayLayer = 0;
//         viewInfo.subresourceRange.layerCount = m_LayerCount;

//         if (vkCreateImageView(VyContext::device(), &viewInfo, nullptr, &m_View) != VK_SUCCESS) {
//             VY_THROW_RUNTIME_ERROR("failed to create texture image view!");
//         }
//     }

//     void LveTexture::createSampler() 
// 	{
//         VkSamplerCreateInfo samplerInfo{ VKInit::samplerCreateInfo() };
//         samplerInfo.magFilter = VK_FILTER_LINEAR;
//         samplerInfo.minFilter = VK_FILTER_LINEAR;

//         samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
//         samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
//         samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

//         samplerInfo.anisotropyEnable = VK_TRUE;
//         samplerInfo.maxAnisotropy = 16.0f;
//         samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
//         samplerInfo.unnormalizedCoordinates = VK_FALSE;

//         // these fields useful for percentage close filtering for shadow maps
//         samplerInfo.compareEnable = VK_FALSE;
//         samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

//         samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
//         samplerInfo.mipLodBias = 0.0f;
//         samplerInfo.minLod = 0.0f;
//         samplerInfo.maxLod = static_cast<float>(m_MipLevels);

//         if (vkCreateSampler(VyContext::device(), &samplerInfo, nullptr, &m_Sampler) != VK_SUCCESS) {
//             VY_THROW_RUNTIME_ERROR("failed to create texture sampler!");
//         }
//     }

//     void LveTexture::transitionLayout(VkCommandBuffer cmdBuffer, VkImageLayout oldLayout, VkImageLayout newLayout) 
// 	{
//       VkImageMemoryBarrier barrier{};
//       barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
//       barrier.oldLayout = oldLayout;
//       barrier.newLayout = newLayout;

//       barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
//       barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

//       barrier.image = m_Image;
//       barrier.subresourceRange.baseMipLevel = 0;
//       barrier.subresourceRange.levelCount = m_MipLevels;
//       barrier.subresourceRange.baseArrayLayer = 0;
//       barrier.subresourceRange.layerCount = m_LayerCount;

//       if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
//         barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
//         if (m_Format == VK_FORMAT_D32_SFLOAT_S8_UINT || m_Format == VK_FORMAT_D24_UNORM_S8_UINT) {
//           barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
//         }
//       } else {
//         barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//       }

//       VkPipelineStageFlags srcStage;
//       VkPipelineStageFlags dstStage;

//       if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
//         barrier.srcAccessMask = 0;
//         barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

//         srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
//         dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
//       } else if (
//           oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) {
//         barrier.srcAccessMask = 0;
//         barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

//         srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
//         dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
//       } else if (
//           oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
//           newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
//         barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
//         barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

//         srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
//         dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
//       } else if (
//           oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
//           newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
//         barrier.srcAccessMask = 0;
//         barrier.dstAccessMask =
//             VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

//         srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
//         dstStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
//       } else if (
//           oldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL &&
//           newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
//         // This says that any cmd that acts in color output or after (dstStage)
//         // that needs read or write access to a resource
//         // must wait until all previous read accesses in fragment shader
//         barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
//         barrier.dstAccessMask =
//             VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;

//         srcStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
//         dstStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
//       } else {
//         VY_THROW_INVALID_ARGUMENT("unsupported layout transition!");
//       }
//       vkCmdPipelineBarrier(
//           cmdBuffer,
//           srcStage,
//           dstStage,
//           0,
//           0,
//           nullptr,
//           0,
//           nullptr,
//           1,
//           &barrier);
//     }
// }






// 	template <typename T>
// 	void VyTexture::validateFormatCompatibility(VkFormat format)
// 	{
// 		// Handle U8 formats
// 		if constexpr (std::is_same<T, U8>::value)
// 		{
// 			if (format != VK_FORMAT_R8_UNORM       && format != VK_FORMAT_R8_SNORM       &&
// 				format != VK_FORMAT_R8_UINT        && format != VK_FORMAT_R8_SRGB        &&
// 				format != VK_FORMAT_R8G8_UNORM     && format != VK_FORMAT_R8G8_SNORM     &&
// 				format != VK_FORMAT_R8G8_UINT      && format != VK_FORMAT_R8G8_SRGB      &&
// 				format != VK_FORMAT_R8G8B8_UNORM   && format != VK_FORMAT_R8G8B8_SNORM   &&
// 				format != VK_FORMAT_R8G8B8_UINT    && format != VK_FORMAT_R8G8B8_SRGB    &&
// 				format != VK_FORMAT_R8G8B8A8_UNORM && format != VK_FORMAT_R8G8B8A8_SNORM &&
// 				format != VK_FORMAT_R8G8B8A8_UINT  && format != VK_FORMAT_R8G8B8A8_SRGB)
// 			{
// 				VY_THROW_INVALID_ARGUMENT("Format is incompatible with U8 data type.");
// 			}
// 		}
// 		// Handle int8_t formats
// 		else if constexpr (std::is_same<T, int8_t>::value)
// 		{
// 			if (format != VK_FORMAT_R8_SINT     && format != VK_FORMAT_R8G8_SINT &&
// 				format != VK_FORMAT_R8G8B8_SINT && format != VK_FORMAT_R8G8B8A8_SINT)
// 			{
// 				VY_THROW_INVALID_ARGUMENT("Format is incompatible with int8_t data type.");
// 			}
// 		}
// 		// Handle float formats
// 		else if constexpr (std::is_same<T, float>::value)
// 		{
// 			if (format != VK_FORMAT_R16_SFLOAT       && format != VK_FORMAT_R16G16_SFLOAT       &&
// 				format != VK_FORMAT_R16G16B16_SFLOAT && format != VK_FORMAT_R16G16B16A16_SFLOAT &&
// 				format != VK_FORMAT_R32_SFLOAT       && format != VK_FORMAT_R32G32_SFLOAT       &&
// 				format != VK_FORMAT_R32G32B32_SFLOAT && format != VK_FORMAT_R32G32B32A32_SFLOAT)
// 			{
// 				VY_THROW_INVALID_ARGUMENT("Format is incompatible with float data type.");
// 			}
// 		}
// 		else if constexpr (std::is_same<T, uint16_t>::value)
// 		{
// 			if (format != VK_FORMAT_R16_UNORM          && format != VK_FORMAT_R16_SNORM          && 
// 				format != VK_FORMAT_R16_UINT           && format != VK_FORMAT_R16_SINT           && 
// 				format != VK_FORMAT_R16_SFLOAT         &&
// 				format != VK_FORMAT_R16G16_UNORM       && format != VK_FORMAT_R16G16_SNORM       && 
// 				format != VK_FORMAT_R16G16_UINT        && format != VK_FORMAT_R16G16_SINT        && 
// 				format != VK_FORMAT_R16G16_SFLOAT      &&
// 				format != VK_FORMAT_R16G16B16_UNORM    && format != VK_FORMAT_R16G16B16_SNORM    && 
// 				format != VK_FORMAT_R16G16B16_UINT     && format != VK_FORMAT_R16G16B16_SINT     && 
// 				format != VK_FORMAT_R16G16B16_SFLOAT   &&
// 				format != VK_FORMAT_R16G16B16A16_UNORM && format != VK_FORMAT_R16G16B16A16_SNORM && 
// 				format != VK_FORMAT_R16G16B16A16_UINT  && format != VK_FORMAT_R16G16B16A16_SINT  && 
// 				format != VK_FORMAT_R16G16B16A16_SFLOAT)
// 			{
// 				VY_THROW_INVALID_ARGUMENT("Format is incompatible with uint16_t data type.");
// 			}
// 		}
// 		// Handle I32 formats
// 		else if constexpr (std::is_same<T, I32>::value)
// 		{
// 			if (format != VK_FORMAT_R32_SINT       && format != VK_FORMAT_R32G32_SINT &&
// 				format != VK_FORMAT_R32G32B32_SINT && format != VK_FORMAT_R32G32B32A32_SINT)
// 			{
// 				VY_THROW_INVALID_ARGUMENT("Format is incompatible with I32 data type.");
// 			}
// 		}
// 		else
// 		{
// 			VY_THROW_INVALID_ARGUMENT("Unsupported data type.");
// 		}
// 	}
// }


// namespace Vy
// {
//     VyTextureDesc VyTextureDesc::texture2D(U32 width, U32 height, VkFormat format)
//     {
//         return VyTextureDesc{

//             .Image = VyImageDesc{

//                 .Format    = format,
//                 .Extent    = VkExtent3D{ width, height, 1 },
//                 .MipLevels = VyImageDesc::kCalculateMipLevels
//             }
//         };
//     }


// 	VyTextureDesc VyTextureDesc::cubemap(U32 width, U32 height, VkFormat format)
// 	{
// 		return VyTextureDesc{

// 			.Image = VyImageDesc{

// 				.Format       = format,
// 				.Extent       = VkExtent3D{ width, height, 1},
// 				.MipLevels    = 1, // m_MipLevels
// 				.LayerCount   = 6, // NUMBER_OF_CUBEMAP_IMAGES
// 				.Usage        = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
// 				.Type         = VK_IMAGE_TYPE_2D,
// 				.Tiling       = VK_IMAGE_TILING_OPTIMAL,
// 				.Flags        = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT,
// 			},
// 			.View = VyImageViewDesc{

// 				.BaseMipLevel = 0,
// 				.LevelCount   = 1, // m_MipLevels
// 				.BaseLayer    = 0,
// 				.LayerCount   = 6, // NUMBER_OF_CUBEMAP_IMAGES
// 				.Type         = VK_IMAGE_VIEW_TYPE_CUBE
// 			},
// 			.Sampler = VySamplerDesc{

// 				.MagFilter    = VK_FILTER_NEAREST,
// 				.MinFilter    = VK_FILTER_NEAREST,
// 				.AddressMode  = VK_SAMPLER_ADDRESS_MODE_REPEAT,
// 				.MipmapMode   = VK_SAMPLER_MIPMAP_MODE_LINEAR,
// 				.MaxLod       = 1, // m_MipLevels
// 				.Anisotropy   = true
// 			}
// 		};
// 	}


//     // VyTextureDesc VyTextureDesc::cubemap(U32 size, VkFormat format)
//     // {
//     //     return VyTextureDesc{
//     //         .Image = VyImageDesc{
//     //             .Format     = format,
//     //             .Extent     = VkExtent3D{ size, size, 1 },
//     //             .MipLevels  = VyImageDesc::kCalculateMipLevels,
// 	// 			.LayerCount = 6,
// 	// 			.Usage      = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
// 	// 			.Type       = VK_IMAGE_TYPE_2D,
// 	// 			.Flags      = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT,
//     //         },
//     //         .View = VyImageViewDesc{
//     //             .Type       = VK_IMAGE_VIEW_TYPE_CUBE,
//     //         },
//     //     };
//     // }


// 	VyTexture::VyTexture(const VyTextureDesc& desc) :
// 		m_Image  { desc.Image                        },
// 		m_View   { desc.View,    m_Image             },
// 		m_Sampler{ desc.Sampler, m_Image.mipLevels() }
// 	{

// 	}


// 	Shared<VyTexture> VyTexture::load(const Path& file, VkFormat format)
// 	{
// 		if (!FS::exists(file))
// 		{
// 			VY_FATAL("Texture file does not exist: '{}'", file.string());
// 			VY_ASSERT(false, "Texture file does not exist");
// 		}

// 		// if (file.extension() == ".hdr")
// 		// {
// 			// return VyTexture::loadCubemap(file);
// 		// }

// 		return VyTexture::loadTexture2D(file, format);
// 	}


// 	// Shared<VyTexture> VyTexture::load(const TVector<Path>& files, bool flip /*true*/)
// 	// {
// 	// 	for (auto file : files)
// 	// 	{
// 	// 		if (!FS::exists(file))
// 	// 		{
// 	// 			VY_FATAL("Texture file does not exist: '{}'", file.string());
// 	// 			VY_ASSERT(false, "Texture file does not exist");
// 	// 		}
// 	// 	}

// 	// 	return VyTexture::loadCubemap(files, flip);
// 	// }


//     Shared<VyTexture> VyTexture::loadTexture2D(const Path& file, VkFormat overrideFormat)
//     {
// 		int stbWidth    = 0;
// 		int stbHeight   = 0;
// 		int stbChannels = 0;

// 		// Load Texture data.
// 		auto pixels = stbi_load(file.string().c_str(), &stbWidth, &stbHeight, &stbChannels, STBI_rgb_alpha);
		
//         if (!pixels)
// 		{
// 			VY_FATAL("Failed to load image: '{}'", file.string());
// 			VY_ASSERT(false, "Failed to load image");
// 		}

// 		// Determine Vulkan format
// 		VkFormat format = VK_FORMAT_R8G8B8A8_SRGB; // Default to SRGB 4 channels

// 		if (overrideFormat == VK_FORMAT_UNDEFINED)
// 		{
// 			if (stbChannels == 1)
// 			{
// 				format = VK_FORMAT_R8_UNORM; // Single channel (gray-scale)
// 			}

// 			else if (stbChannels == 3)
// 			{
// 				// // Convert RGB to RGBA
// 				U8* rgbaData = new U8[stbWidth * stbHeight * 4];

// 				for (I32 i = 0; i < stbWidth * stbHeight; ++i) 
// 				{
// 					rgbaData[i * 4 + 0] = pixels[i * 3 + 0];
// 					rgbaData[i * 4 + 1] = pixels[i * 3 + 1];
// 					rgbaData[i * 4 + 2] = pixels[i * 3 + 2];
// 					rgbaData[i * 4 + 3] = 255;
// 				}

// 				stbi_image_free(pixels); // original RGB

// 				pixels      = rgbaData;
// 				stbChannels = 4;
// 				format      = VK_FORMAT_R8G8B8A8_SRGB; // Convert RGB to RGBA
// 			}
// 		}
// 		else
// 		{
// 			format = overrideFormat;
// 		}

// 		U32 width  = static_cast<U32>(stbWidth);
// 		U32 height = static_cast<U32>(stbHeight);

// 		VkDeviceSize imageSize = 4 * static_cast<VkDeviceSize>(stbWidth) * static_cast<VkDeviceSize>(stbHeight);

// 		VyTextureDesc desc = VyTextureDesc::texture2D(width, height, format);
		
//         auto texture = MakeShared<VyTexture>(desc);
		
//         texture->image().upload(pixels, imageSize);

// 		stbi_image_free(pixels);

// 		return texture;
//     }


// 	// Shared<VyTexture> VyTexture::loadCubemap(const TVector<Path>& files, bool flip)
// 	// {
// 	// 	stbi_set_flip_vertically_on_load(flip);

//     //     VkDeviceSize layerSize;
//     //     VkDeviceSize imageSize;

// 	// 	int stbWidth    = 0;
// 	// 	int stbHeight   = 0;
// 	// 	int stbChannels = 0;

//     //     TVector<stbi_uc*> pixels;

//     //     for (int i = 0; i < NUMBER_OF_CUBEMAP_IMAGES; i++)
// 	// 	{
//     //         // load all faces
//     //         stbi_uc* img = stbi_load(files[i].string().c_str(), &stbWidth, &stbHeight, &stbChannels, 4); // 4 == STBI_rgb_alpha
            
//     //         if (!img)
//     //         {
//     //             VY_ERROR_TAG("VyCubemap", "Couldn't load texture from file {0}", files[i].string());
//     //         }

// 	// 		pixels.push_back(img);
// 	// 	}

//     //     layerSize = stbWidth  * stbHeight * 4;
//     //     imageSize = layerSize * NUMBER_OF_CUBEMAP_IMAGES;

//     //     VyBuffer stagingBuffer = VyBuffer{ VyBuffer::stagingBuffer(imageSize) };

//     //     stagingBuffer.map();
//     //     for(size_t i = 0; i < NUMBER_OF_CUBEMAP_IMAGES; i++) 
//     //     {
//     //         void* offset = static_cast<U8*>(stagingBuffer.mappedData()) + (i * layerSize);

//     //         std::memcpy(offset, pixels[i], layerSize);
//     //     }
//     //     stagingBuffer.unmap();

//     //     VkFormat format = /*m_SRGB ? VK_FORMAT_R8G8B8A8_SRGB :*/ VK_FORMAT_R8G8B8A8_UNORM;

// 	// 	VyTextureDesc desc = VyTextureDesc::cubemap(stbWidth, stbHeight, format);
		
// 	// 	auto texture = MakeShared<VyTexture>(desc);

// 	// 	texture->image().transitionLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

//     //     VyContext::device().copyBufferToImage(
//     //         stagingBuffer.handle(),
//     //         texture->image(),
//     //         static_cast<U32>(stbWidth),     
//     //         static_cast<U32>(stbHeight), 
//     //         NUMBER_OF_CUBEMAP_IMAGES     // layerCount
//     //     );

// 	// 	texture->image().transitionLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

//     //     for(auto& pixel : pixels) 
//     //     {
//     //         stbi_image_free(pixel);
//     //     }

// 	// 	return texture;
// 	// }

// 	// ============================================================================================


// 	VyTexture::VyTexture(VyTexture&& other) noexcept : 
// 		m_Image   { std::move(other.m_Image)   },
// 		m_View    { std::move(other.m_View)    },
// 		m_Sampler { std::move(other.m_Sampler) }
// 	{
// 	}


// 	VyTexture::~VyTexture()
// 	{
// 		destroy();
// 	}


// 	VyTexture& VyTexture::operator=(VyTexture&& other) noexcept
// 	{
// 		if (this != &other)
// 		{
// 			destroy();

// 			m_Image   = std::move(other.m_Image  );
// 			m_View    = std::move(other.m_View   );
// 			m_Sampler = std::move(other.m_Sampler);

// 		}

// 		return *this;
// 	}


// 	void VyTexture::resize(VkExtent3D newSize, VkImageUsageFlags usage)
// 	{
// 		// TODO: Rework this, does not work for all textures (e.g. cube maps).

// 		m_Image.resize(newSize, usage);

// 		VyImageViewDesc viewDesc{
// 			.BaseMipLevel = 0,
// 			.LevelCount   = m_Image.mipLevels(),
// 			.BaseLayer    = 0,
// 			.LayerCount   = m_Image.layerCount(),
// 			.Type         = VK_IMAGE_VIEW_TYPE_2D,
// 		};

// 		m_View = VyImageView{ viewDesc, m_Image };
// 	}


// 	void VyTexture::destroy()
// 	{
// 		VY_INFO("VyTexture::destroy() called.");
// 	}
// }



















// Shared<VyTexture> VyTexture::loadCubemap(const Path& file)
	// {
	// 	// HDR environment maps are stored as equirectangular images (longitude/latitude 2D image).
	// 	// To convert it to a cubemap, the image is sampled in a compute shader and written to the cubemap.

	// 	int stbWidth    = 0;
	// 	int stbHeight   = 0;
	// 	int stbChannels = 0;

	// 	auto pixels = stbi_loadf(file.string().c_str(), &stbWidth, &stbHeight, &stbChannels, STBI_rgb_alpha);
		
	// 	if (!pixels)
	// 	{
	// 		VY_FATAL("Failed to load image: '{}'", file.string());
	// 		VY_ASSERT(false, "Failed to load image");
	// 	}

	// 	// Upload data to staging buffer.
	// 	VkDeviceSize imageSize = 4 * sizeof(float) * static_cast<VkDeviceSize>(stbWidth) * static_cast<VkDeviceSize>(stbHeight);
		
	// 	VyBuffer stagingBuffer{ VyBuffer::stagingBuffer(imageSize) };
		
	// 	stagingBuffer.singleWrite(pixels);

	// 	stbi_image_free(pixels);

	// 	// Create Spherical image.
	// 	U32 width  = static_cast<U32>(stbWidth);
	// 	U32 height = static_cast<U32>(stbHeight);

	// 	auto sphericalDesc = VyTextureDesc::texture2D(width, height, VK_FORMAT_R32G32B32A32_SFLOAT);
    //     {
    //         sphericalDesc.Image.MipLevels = 1;
    //     }

	// 	VyTexture spherialImage{ sphericalDesc };

	// 	// Create Cubemap image.
	// 	U32 cubeSize = width / 4; // Cubemap needs 4 horizontal faces.

	// 	auto cubeDesc = VyTextureDesc::cubemap(cubeSize, VK_FORMAT_R16G16B16A16_SFLOAT);
    //     {
    //         cubeDesc.Image.Usage |= VK_IMAGE_USAGE_STORAGE_BIT;
    //     }

	// 	auto cubeMap = MakeShared<VyTexture>(cubeDesc);

	// 	// Create Pipeline Resources.
	// 	auto descriptorSetLayout = VyDescriptorSetLayout::Builder{}
	// 		.addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_COMPUTE_BIT)
	// 		.addBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,          VK_SHADER_STAGE_COMPUTE_BIT)
	// 		.build();

	// 	VyDescriptorSet descriptorSet{ descriptorSetLayout };

	// 	VyDescriptorWriter{ descriptorSetLayout }
	// 		.writeImage(0, spherialImage.descriptorImageInfo(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)) 
	// 		.writeImage(1, cubeMap     ->descriptorImageInfo(VK_IMAGE_LAYOUT_GENERAL))
	// 		.update(descriptorSet);

	// 	auto pipeline = VyPipeline::ComputeBuilder{}
	// 		.addDescriptorSetLayout(descriptorSetLayout)
	// 		.setShaderStage        (SHADER_DIR "IBL/EquirectToCube.comp.spv")
	// 		.build();

	// 	// Convert Spherical Image to Cubemap.
	// 	VkCommandBuffer cmdBuffer = m_Device.beginSingleTimeCommands();
	// 	{
    //         VKCmd::beginDebugUtilsLabel(cmdBuffer, "Equirectangular to Cubemap");
    //         {
    //             spherialImage.image().copyFrom(cmdBuffer, stagingBuffer);

    //             cubeMap->image().transitionLayout(cmdBuffer, VK_IMAGE_LAYOUT_GENERAL); // (Access: Shader Read | Write Bit)

    //             pipeline.bind(cmdBuffer);
    //             pipeline.bindDescriptorSet(cmdBuffer, 0, descriptorSet);

    //             constexpr U32 kGroupSize = 16;

    //             U32 groupCountX = (width  + kGroupSize - 1) / kGroupSize;
    //             U32 groupCountY = (height + kGroupSize - 1) / kGroupSize;

    //             vkCmdDispatch(cmdBuffer, groupCountX, groupCountY, 6);

    //             // Generate Mipmaps.
    //             cubeMap->image().generateMipmaps(cmdBuffer, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    //         }
    //         VKCmd::endDebugUtilsLabel(cmdBuffer);
    //     }
	// 	m_Device.endSingleTimeCommands(cmdBuffer);

    //     return cubeMap;
	// }


    // Shared<VyTexture> VyTexture::solidColorTexture2D(Vec4 color)
    // {
	// 	auto desc    = VyTextureDesc::texture2D(1, 1, VK_FORMAT_R32G32B32A32_SFLOAT);
	// 	auto texture = MakeShared<VyTexture>(desc);

	// 	texture->image().upload(std::addressof(color), sizeof(Vec4));
		
    //     return texture;
    // }


    // Shared<VyTexture> VyTexture::solidColorCubemap(Vec4 color)
    // {
	// 	TArray<Vec4, 6> colors{ color, color, color, color, color, color };

	// 	auto desc    = VyTextureDesc::cubemap(1, VK_FORMAT_R32G32B32A32_SFLOAT);
	// 	auto texture = MakeShared<VyTexture>(desc);

	// 	texture->image().upload(std::addressof(colors), sizeof(colors));
		
    //     return texture;
    // }


	// Shared<VyTexture> VyTexture::irradianceMap(const Shared<VyTexture>& skybox)
    // {
	// 	// Create Irradiance Map.
    //     // Diffuse Irradiance Image-Based-Lighting (IBL), or commonly referred to as Irradiance Maps.

	// 	constexpr U32 kIrradianceSize = 32;

	// 	auto textureDesc = VyTextureDesc::cubemap(kIrradianceSize, VK_FORMAT_R16G16B16A16_SFLOAT);
    //     {
    //         textureDesc.Image.Usage    |= VK_IMAGE_USAGE_STORAGE_BIT;
    //         textureDesc.Image.MipLevels = 1;
    //     }

	// 	auto irradiance = MakeShared<VyTexture>(textureDesc);

	// 	// Create Pipeline Resources
	// 	auto descriptorSetLayout = VyDescriptorSetLayout::Builder{}
	// 		.addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_COMPUTE_BIT)
	// 		.addBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,          VK_SHADER_STAGE_COMPUTE_BIT)
	// 		.build();

	// 	VyDescriptorSet descriptorSet{ descriptorSetLayout };

	// 	VyDescriptorWriter{ descriptorSetLayout }
	// 		.writeImage(0, skybox    ->descriptorImageInfo(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL))
	// 		.writeImage(1, irradiance->descriptorImageInfo(VK_IMAGE_LAYOUT_GENERAL))
	// 		.update(descriptorSet);

	// 	auto pipeline = VyPipeline::ComputeBuilder{}
	// 		.addDescriptorSetLayout(descriptorSetLayout)
	// 		.setShaderStage        (SHADER_DIR "IBL/IrradianceConvolution.comp.spv")
	// 		.build();

	// 	// Convert Skybox to Irradiance Map
	// 	VkCommandBuffer cmdBuffer = m_Device.beginSingleTimeCommands();
    //     {
    //         VKCmd::beginDebugUtilsLabel(cmdBuffer, "Irradiance Convolution");
    //         {
    //             // Transition Skybox ImageLayout to Shader-Read-Only, And Irradiance ImageLayout to General.

    //             skybox    ->image().transitionLayout(cmdBuffer, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL); // (Access: Shader Read Bit)
    //             irradiance->image().transitionLayout(cmdBuffer, VK_IMAGE_LAYOUT_GENERAL);                  // (Access: Shader Read | Write Bit)

    //             pipeline.bind(cmdBuffer);
    //             pipeline.bindDescriptorSet(cmdBuffer, 0, descriptorSet);

    //             constexpr U32 kGroupSize = 16;

    //             U32 width  = irradiance->image().width();
    //             U32 height = irradiance->image().height();
            
    //             U32 groupCountX = (width  + kGroupSize - 1) / kGroupSize;
    //             U32 groupCountY = (height + kGroupSize)     / kGroupSize;

    //             vkCmdDispatch(cmdBuffer, groupCountX, groupCountY, 6);

    //             // Transition Irradiance ImageLayout to Shader-Read-Only.
    //             irradiance->image().transitionLayout(cmdBuffer, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL); // (Access: Shader Read Bit)
    //         }
    //         VKCmd::endDebugUtilsLabel(cmdBuffer);
    //     }
	// 	m_Device.endSingleTimeCommands(cmdBuffer);

	// 	return irradiance;
    // }


	// Shared<VyTexture> VyTexture::prefilteredMap(const Shared<VyTexture>& skybox)
    // {
	// 	// Create Prefiltered Map
	// 	constexpr U32 kPrefilteredSize = 128;
	// 	constexpr U32 kMipLevelCount   = 5;

    //     // create VkImageViews to access specific levels of a VkImage that contains mipmaps. 
    //     // Mipmaps are a set of pre-scaled, lower-resolution textures for a single image, 
    //     // used to improve rendering performance and visual quality at different distances.

	// 	auto textureDesc = VyTextureDesc::cubemap(kPrefilteredSize, VK_FORMAT_R16G16B16A16_SFLOAT);
    //     {
    //         textureDesc.Image.Usage    |= VK_IMAGE_USAGE_STORAGE_BIT;
    //         textureDesc.Image.MipLevels = kMipLevelCount;
    //     }

	// 	auto prefiltered = MakeShared<VyTexture>(textureDesc);

	// 	// Create pipeline resources
	// 	auto descriptorSetLayout = VyDescriptorSetLayout::Builder{}
	// 		.addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_COMPUTE_BIT)
	// 		.addBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,          VK_SHADER_STAGE_COMPUTE_BIT)
	// 		.build();

	// 	struct PushConstants
	// 	{
	// 		float Roughness     =   0.0f;
	// 		float EnvResolution = 512.0f;

	// 	} pushConstants;

	// 	pushConstants.EnvResolution = static_cast<float>(skybox->image().width());

	// 	auto pipeline = VyPipeline::ComputeBuilder{}
	// 		.addDescriptorSetLayout(descriptorSetLayout)
	// 		.addPushConstantRange  (VK_SHADER_STAGE_COMPUTE_BIT, sizeof(pushConstants))
	// 		.setShaderStage        (SHADER_DIR "IBL/PrefilterEnvironment.comp.spv")
	// 		.build();

	// 	// Create Image Views for mip levels
	// 	TVector<VyImageView> mipImageViews;
    //     mipImageViews.reserve(kMipLevelCount);

    //     // Reserve Descriptor Sets for Mip ImageViews.
	// 	TVector<VyDescriptorSet> descriptorSets;
	// 	descriptorSets.reserve(kMipLevelCount);

	// 	for (U32 i = 0; i < kMipLevelCount; ++i)
	// 	{
    //         // Create an ImageView for each mip level of the prefiltered image.
    //         VyImageViewDesc viewDesc{
    //             .BaseMipLevel = i,
    //             .LevelCount   = 1,
    //             .BaseLayer    = 0,
    //             .LayerCount   = 6,
    //             .ViewType     = VK_IMAGE_VIEW_TYPE_CUBE
    //         };

    //         mipImageViews.emplace_back(viewDesc, prefiltered->image());

	// 		descriptorSets.emplace_back(descriptorSetLayout);

	// 		VyDescriptorWriter{ descriptorSetLayout }
	// 			.writeImage(0, skybox->descriptorImageInfo(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL))
	// 			.writeImage(1, VkDescriptorImageInfo{ VK_NULL_HANDLE, mipImageViews[i], VK_IMAGE_LAYOUT_GENERAL })
	// 			.update(descriptorSets[i]);
	// 	}

	// 	// Convert skybox to prefiltered map.
	// 	VkCommandBuffer cmdBuffer = m_Device.beginSingleTimeCommands();
    //     {
    //         VKCmd::beginDebugUtilsLabel(cmdBuffer, "Prefilter Environment");
    //         {
    //             // Transition Skybox ImageLayout to Shader-Read-Only, And Prefilter ImageLayout to General.

    //             skybox     ->image().transitionLayout(cmdBuffer, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL); // (Access: Shader Read Bit)
    //             prefiltered->image().transitionLayout(cmdBuffer, VK_IMAGE_LAYOUT_GENERAL);                  // (Access: Shader Read | Write Bit)

    //             pipeline.bind(cmdBuffer);
                
    //             for (U32 mip = 0; mip < kMipLevelCount; ++mip)
    //             {
    //                 pipeline.bindDescriptorSet(cmdBuffer, 0, descriptorSets[mip]);

    //                 pushConstants.Roughness = static_cast<float>(mip) / static_cast<float>(kMipLevelCount - 1);

    //                 pipeline.pushConstants(
    //                     cmdBuffer, 
    //                     VK_SHADER_STAGE_COMPUTE_BIT, 
    //                     std::addressof(pushConstants), 
    //                     sizeof(pushConstants)
    //                 );

    //                 constexpr U32 kGroupSize = 16;

    //                 U32 width  = prefiltered->image().width()  >> mip;
    //                 U32 height = prefiltered->image().height() >> mip;

    //                 U32 groupCountX = (width  + kGroupSize - 1) / kGroupSize;
    //                 U32 groupCountY = (height + kGroupSize - 1) / kGroupSize;

    //                 vkCmdDispatch(cmdBuffer, groupCountX, groupCountY, 6);
    //             }

    //             // Transition Prefiltered ImageLayout to Shader-Read-Only.
    //             prefiltered->image().transitionLayout(cmdBuffer, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL); // (Access: Shader Read Bit)
    //         }
    //         VKCmd::endDebugUtilsLabel(cmdBuffer);
    //     }
	// 	m_Device.endSingleTimeCommands(cmdBuffer);

	// 	return prefiltered;
    // }


	// Shared<VyTexture> VyTexture::BRDFLUT()
    // {
	// 	// Create BRDF LUT (Bidirectional reflectance distribution function - Lookup Tables).

	// 	constexpr U32 kLUTSize = 512;

	// 	auto textureDesc = VyTextureDesc::texture2D(kLUTSize, kLUTSize, VK_FORMAT_R16G16_SFLOAT);
	// 	{
    //         textureDesc.Image.MipLevels = 1;
    //         textureDesc.Image.Usage    |= VK_IMAGE_USAGE_STORAGE_BIT;

    //         textureDesc.Sampler.AddressMode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    //     }
		
    //     auto lut = MakeShared<VyTexture>(textureDesc);

	// 	// Create pipeline resources.
	// 	auto descriptorSetLayout = VyDescriptorSetLayout::Builder{}
	// 		.addBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
	// 		.build();

	// 	VyDescriptorSet descriptorSet{ descriptorSetLayout };

	// 	VyDescriptorWriter{ descriptorSetLayout }
	// 		.writeImage(0, lut->descriptorImageInfo(VK_IMAGE_LAYOUT_GENERAL))
	// 		.update(descriptorSet);

	// 	auto pipeline = VyPipeline::ComputeBuilder{}
	// 		.addDescriptorSetLayout(descriptorSetLayout)
	// 		.setShaderStage        (SHADER_DIR "IBL/BrdfLut.comp.spv")
	// 		.build();

	// 	// Generate BRDF LUT.
	// 	VkCommandBuffer cmdBuffer = m_Device.beginSingleTimeCommands();
	// 	{
    //         VKCmd::beginDebugUtilsLabel(cmdBuffer, "BRDF LUT Generation");
    //         {
    //             // Transition LUT Texture ImageLayout to General.
    //             lut->image().transitionLayout(cmdBuffer, VK_IMAGE_LAYOUT_GENERAL); // (Access: Shader Read | Write Bit)

    //             pipeline.bind(cmdBuffer);
    //             pipeline.bindDescriptorSet(cmdBuffer, 0, descriptorSet);

    //             constexpr U32 kGroupSize = 16;

    //             U32 groupCountX = (kLUTSize + kGroupSize - 1) / kGroupSize;
    //             U32 groupCountY = (kLUTSize + kGroupSize - 1) / kGroupSize;

    //             vkCmdDispatch(cmdBuffer, groupCountX, groupCountY, 1);

    //             // Transition LUT Texture ImageLayout to Shader-Read-Only.
    //             lut->image().transitionLayout(cmdBuffer, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL); // (Access: Shader Read Bit)
    //         }
    //         VKCmd::endDebugUtilsLabel(cmdBuffer);
    //     }
	// 	m_Device.endSingleTimeCommands(cmdBuffer);

	// 	return lut;
    // }









// namespace Vy
// {
//     const U32 TextureLibrary::MAX_TEXTURE_COUNT     = 500;
//     const U32 TextureLibrary::INVALID_TEXTURE_INDEX = 10001;


//     TextureLibrary::TextureLibrary() : 
// 		m_TextureSampler{ VK_NULL_HANDLE }
//     {
// 		CreateTextureSampler();
// 		CreateDescriptors();
//     }


//     TextureLibrary::~TextureLibrary()
//     {
//         if (m_TextureSampler)
//         {
//             vkDestroySampler(VyContext::device(), m_TextureSampler, nullptr);
//         }
//         if (m_ImageDescriptorSetLayout)
//         {
//             vkDestroyDescriptorSetLayout(VyContext::device(), m_ImageDescriptorSetLayout, nullptr);
//             vkDestroyDescriptorPool     (VyContext::device(), m_ImageDescriptorPool,      nullptr);
//         }

//         m_Textures.clear();
//     }


//     U32 TextureLibrary::QueueTextureLoad(const String& texturePath)
//     {
//         if (m_TextureMap.find(texturePath) != m_TextureMap.end())
//         {
//             return m_TextureMap[texturePath];
//         }

//         U32 index = m_NextIndex++;

//         m_TextureMap[ texturePath ] = index;

//         if (m_TexturesData.size() <= index)
//         {
//             m_TexturesData.resize(index + 1);
//         }

//         VyTextureLoadData& loadData = m_PendingTextureLoads.emplace_back();
// 		{
// 			loadData.FilePath    = texturePath;
// 			loadData.TargetIndex = index;
// 		}

// 		return index;
//     }


//     U32 TextureLibrary::QueueTextureLoad(const U8* textureData, U32 textureSize, const String& texturePath)
//     {
//         if (m_TextureMap.find(texturePath) != m_TextureMap.end())
//         {
//             return m_TextureMap[texturePath];
//         }

//         U32 index = m_NextIndex++;

//         m_TextureMap[ texturePath ] = index;

//         if (m_TexturesData.size() <= index)
//         {
//             m_TexturesData.resize(index + 1);
//         }

//         VyTextureLoadData& loadData = m_PendingTextureLoads.emplace_back();
// 		{
// 			loadData.Data        = textureData;
// 			loadData.Size        = textureSize;
// 			loadData.FilePath    = texturePath;
// 			loadData.TargetIndex = index;
// 		}

// 		return index;
//     }


//     bool TextureLibrary::LoadQueuedTextures()
//     {
//         if (m_PendingTextureLoads.empty())
//         {
//             return false;
//         }

//         VY_INFO("Loading {0} queued textures...", m_PendingTextureLoads.size());
//         VyTextureLoader::load(m_PendingTextureLoads);

//         VY_INFO("Finished loading textures. Creating GPU resources...");
//         if (m_Textures.size() < m_NextIndex)
//         {
//             m_Textures.resize(m_NextIndex);
//         }

//         for (auto& loadData : m_PendingTextureLoads)
//         {
//             U32 index = loadData.TargetIndex;

//             m_TexturesData[ index ]      = std::move(loadData.OutTexture);
//             m_TexturesData[ index ].Name = loadData.FilePath;

//             Unique<VyImageAsset> newTexture;
// 			m_TexturesData[ index ].ImageFormat = VK_FORMAT_R8G8B8A8_SRGB;
// 			newTexture = std::make_unique<VyTexture>(m_TexturesData[ index ]);
			
// 			CreateDescriptorSet(static_cast<VyTexture*>(newTexture.get()));

//             m_Textures[ index ] = std::move(newTexture);
//         }

//         VY_INFO("All queued textures loaded successfully!");
//         m_PendingTextureLoads.clear();

//         return true;
//     }


//     U32 TextureLibrary::CreateStorageImage(const String& imageName, U32 width, U32 height, VkFormat imageFormat, VkImageUsageFlags usage, VkImageLayout finalLayout)
//     {
//         // check if exists
//         auto it = m_TextureMap.find(imageName);
//         if (it != m_TextureMap.end())
//         {
//             return it->second;
//         }

//         U32 index = m_NextIndex++;
//         m_TextureMap[imageName] = index;
//         if (m_TexturesData.size() <= index)
//         {
//             m_TexturesData.resize(index + 1);
//         }

//         VyTextureData& textureData = m_TexturesData[ index ];
// 		{
// 			textureData.IsStorageImage = true;
// 			textureData.Width = width;
// 			textureData.Height = height;
// 			textureData.ImageFormat = imageFormat;
// 			textureData.Usage = usage;
// 			textureData.FinalLayout = finalLayout;
// 			textureData.Name = imageName;
// 		}

//         if (m_Textures.size() <= index)
// 		{
// 			m_Textures.resize(index + 1);
// 		}

//         m_Textures[ index ] = std::make_unique<VyTexture>(textureData);
//         CreateDescriptorSet(static_cast<VyTexture*>(m_Textures[ index ].get()), finalLayout);

//         return index;
//     }


//     VyImageAsset* TextureLibrary::GetTexture(U32 index)
//     {
//         if (index < m_Textures.size()) 
// 		{
//             return m_Textures[ index ].get();
//         }
        
// 		return nullptr;
//     }


//     VyImageAsset* TextureLibrary::GetTexture(const String& filePath)
//     {
//         auto it = m_TextureMap.find(filePath);
        
// 		if (it != m_TextureMap.end()) 
// 		{
//             return GetTexture(it->second);
//         }
        
// 		return nullptr;
//     }


//     VyImageAsset* TextureLibrary::GetTextureByIndex(U32 index)
//     {
//         if (index < m_Textures.size()) 
// 		{
//             return m_Textures[ index ].get();
//         }

//         VY_ERROR("Texture index out of range: {0}", index);

//         return m_Textures[0].get();
//     }


//     void TextureLibrary::CreateTextureSampler()
//     {
//         if (m_TextureSampler != VK_NULL_HANDLE)
//         {
//             return;
//         }

//         auto properties = VyContext::device().properties();

//         VkSamplerCreateInfo samplerInfo{ VKInit::samplerCreateInfo() };
// 		{
// 			samplerInfo.magFilter = VK_FILTER_LINEAR;
// 			samplerInfo.minFilter = VK_FILTER_LINEAR;
// 			samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
// 			samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
// 			samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
// 			samplerInfo.anisotropyEnable = VK_TRUE;
// 			samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
// 			samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
// 			samplerInfo.unnormalizedCoordinates = VK_FALSE;
// 			samplerInfo.compareEnable = VK_FALSE;
// 			samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
// 			samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
// 		}
			
//         if (vkCreateSampler(VyContext::device(), &samplerInfo, nullptr, &m_TextureSampler) != VK_SUCCESS)
//         {
//             VY_ERROR("Failed to create texture sampler");
//         }
//     }

	
//     void TextureLibrary::CreateDescriptors()
//     {
//         if (m_ImageDescriptorPool != VK_NULL_HANDLE)
//         {
//             return;
//         }

//         // 1. Define the layout binding
//         VkDescriptorSetLayoutBinding samplerLayoutBinding{};
// 		{
// 			samplerLayoutBinding.binding = 0; // The binding point in the shader (e.g., layout(binding = 0) ...)
// 			samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
// 			samplerLayoutBinding.descriptorCount = 1; // You're binding one sampler
// 			samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT; // Accessible in the fragment shader
// 			samplerLayoutBinding.pImmutableSamplers = nullptr; // Optional
// 		}

//         // 2. Create the descriptor set layout
//         VkDescriptorSetLayoutCreateInfo layoutInfo{};
// 		{
// 			layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
// 			layoutInfo.bindingCount = 1;
// 			layoutInfo.pBindings = &samplerLayoutBinding;
// 		}

//         if (vkCreateDescriptorSetLayout(VyContext::device(), &layoutInfo, nullptr, &m_ImageDescriptorSetLayout) != VK_SUCCESS)
//         {
//             VY_THROW_RUNTIME_ERROR("failed to create descriptor set layout!");
//         }

//         // 1. Define the pool size
//         VkDescriptorPoolSize poolSize{};
// 		{
// 			poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
// 			poolSize.descriptorCount = MAX_TEXTURE_COUNT; // Enough space for one descriptor of this type
// 		}

//         // 2. Create the descriptor pool info
//         VkDescriptorPoolCreateInfo poolInfo{};
// 		{
// 			poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
// 			poolInfo.poolSizeCount = 1;
// 			poolInfo.pPoolSizes = &poolSize;
// 			poolInfo.maxSets = MAX_TEXTURE_COUNT; // Max number of descriptor sets that can be allocated
// 		}

//         if (vkCreateDescriptorPool(VyContext::device(), &poolInfo, nullptr, &m_ImageDescriptorPool) != VK_SUCCESS) 
//         {
//             VY_THROW_RUNTIME_ERROR("failed to create descriptor pool!");
//         }
//     }


//     void TextureLibrary::CreateDescriptorSet(VyTexture* texture, VkImageLayout layout)
//     {
//         VkDescriptorSetAllocateInfo allocInfo{};
// 		{
// 			allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
// 			allocInfo.descriptorPool = m_ImageDescriptorPool;
// 			allocInfo.descriptorSetCount = 1;
// 			allocInfo.pSetLayouts = &m_ImageDescriptorSetLayout; // Use the layout from step 1
// 		}

//         if (vkAllocateDescriptorSets(VyContext::device(), &allocInfo, &texture->mDescriptorSet) != VK_SUCCESS)
//         {
//             VY_THROW_RUNTIME_ERROR("failed to allocate descriptor set!");
//         }

//         // 1. Populate the image info struct
//         VkDescriptorImageInfo imageInfo{ VKInit::imageCreateInfo() };
// 		{
// 			imageInfo.imageLayout = layout; // Layout the image will be in when sampled
// 			imageInfo.imageView = texture->view(); // Your VkImageView handle
// 			imageInfo.sampler = m_TextureSampler;     // Your VkSampler handle
// 		}

//         // 2. Populate the write descriptor struct
//         VkWriteDescriptorSet descriptorWrite{};
// 		{
// 			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
// 			descriptorWrite.dstSet = texture->mDescriptorSet; // The set to update (from step 3)
// 			descriptorWrite.dstBinding = 0;         // The binding to update (from step 1)
// 			descriptorWrite.dstArrayElement = 0;  // Start at index 0
// 			descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
// 			descriptorWrite.descriptorCount = 1;
// 			descriptorWrite.pImageInfo = &imageInfo; // Point to the image info struct
// 		}

//         // 3. Call vkUpdateDescriptorSets to perform the update
//         vkUpdateDescriptorSets(VyContext::device(), 1, &descriptorWrite, 0, nullptr);
//     }


//     void TextureLibrary::ClearAllBuffers()
//     {
//         if (m_TextureSampler)
//         {
//             vkDestroySampler(VyContext::device(), m_TextureSampler, nullptr);
//             m_TextureSampler = VK_NULL_HANDLE;
//         }

//         if (m_ImageDescriptorSetLayout)
//         {
//             vkDestroyDescriptorSetLayout(VyContext::device(), m_ImageDescriptorSetLayout, nullptr);
//             m_ImageDescriptorSetLayout = VK_NULL_HANDLE;
//         }

//         if (m_ImageDescriptorPool)
//         {
//             vkDestroyDescriptorPool(VyContext::device(), m_ImageDescriptorPool, nullptr);
//             m_ImageDescriptorPool = VK_NULL_HANDLE;
//         }

//         m_Textures.clear();
//     }


//     void TextureLibrary::RecreateAllBuffers()
//     {
//         m_Textures.resize(m_TexturesData.size());

//         for (U32 index = 0; index < m_TexturesData.size(); ++index)
//         {
//             VyTextureData& textureData = m_TexturesData[ index ];

//             if (textureData.Name.empty() && textureData.Pixels.size() == 0) 
// 				continue;

//             textureData.ImageFormat = VK_FORMAT_R8G8B8A8_SRGB;

//             if (textureData.IsStorageImage)
//             {
//                 textureData.ImageFormat = VK_FORMAT_R32G32B32A32_SFLOAT;
//             }

// 			// If you have textureData.isStorageImage flag, pass the proper final layout
// 			m_Textures[ index ] = std::make_unique<VyTexture>(textureData);
			
// 			CreateDescriptorSet(
// 				static_cast<VyTexture*>(m_Textures[ index ].get()),
// 				textureData.IsStorageImage 
// 					? textureData.FinalLayout 
// 					: VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
// 			);


//             // Ensure m_TextureMap has the same mapping still (this should be true if we didn't clear it).
//             m_TextureMap[textureData.Name] = index;
//         }
//     }
// }


















// namespace Vy
// {
// 	FileTextureSource::FileTextureSource(const String& filepath)
// 	{
// 		int width, height, channels;
// 		stbi_uc* stbi_pixels = stbi_load(filepath.c_str(), &width, &height, &channels, STBI_rgb_alpha);
        
// 		if (!stbi_pixels)
// 		{
// 			VY_FATAL("Failed to load image: '{}'", filepath);
// 			VY_ASSERT(false, "Failed to load image");
// 		}

// 		m_Width	       = width;
// 		m_Height       = height;
// 		m_BitsPerPixel = 4;

// 		U32 size = m_Width * m_Height * m_BitsPerPixel;
		
// 		m_Pixels.reserve(size);
// 		m_Pixels.insert(m_Pixels.end(), stbi_pixels, stbi_pixels + size);

// 		stbi_image_free(stbi_pixels);
// 	}


// 	FloatFileTextureSource::FloatFileTextureSource(const String& filepath)
// 	{
// 		int width, height, channels;
// 		float* stbi_pixels = stbi_loadf(filepath.c_str(), &width, &height, &channels, STBI_rgb_alpha);
        
// 		if (!stbi_pixels)
// 		{
// 			VY_FATAL("Failed to load image: '{}'", filepath);
// 			VY_ASSERT(false, "Failed to load image");
// 		}

// 		m_Width	       = width;
// 		m_Height       = height;
// 		m_BitsPerPixel = 4 * sizeof(float);

// 		U32 size = m_Width * m_Height * m_BitsPerPixel;

// 		m_Pixels.reserve(size);
// 		m_Pixels.insert(m_Pixels.end(), (U8*)stbi_pixels, (U8*)stbi_pixels + size);

// 		stbi_image_free(stbi_pixels);
// 	}


// 	SolidTextureSource::SolidTextureSource(Vec4 color, U32 width, U32 height)
// 	{
// 		m_Width	       = width;
// 		m_Height       = height;
// 		m_BitsPerPixel = 4;

// 		m_Pixels.resize(width * height * m_BitsPerPixel);
		
// 		for (int i = 0; i < width; i++) 
// 		{
// 			for (int j = 0; j < height; j++) 
// 			{
// 				m_Pixels[(i + j * width) * 4 + 0] = color.r * 255;
// 				m_Pixels[(i + j * width) * 4 + 1] = color.g * 255;
// 				m_Pixels[(i + j * width) * 4 + 2] = color.b * 255;
// 				m_Pixels[(i + j * width) * 4 + 3] = color.a * 255;
// 			}
// 		}
// 	}


// 	FloatSolidTextureSource::FloatSolidTextureSource(Vec4 color, U32 width, U32 height)
// 	{
// 		m_Width        = width;
// 		m_Height       = height;
// 		m_BitsPerPixel = 4 * sizeof(float);

// 		m_Pixels.resize(width * height * m_BitsPerPixel);
		
// 		for (int i = 0; i < width; i++) 
// 		{
// 			for (int j = 0; j < height; j++) 
// 			{
// 				*(float*)(m_Pixels.data() + sizeof(float) * ((i + j * width) * 4 + 0)) = color.r;
// 				*(float*)(m_Pixels.data() + sizeof(float) * ((i + j * width) * 4 + 1)) = color.g;
// 				*(float*)(m_Pixels.data() + sizeof(float) * ((i + j * width) * 4 + 2)) = color.b;
// 				*(float*)(m_Pixels.data() + sizeof(float) * ((i + j * width) * 4 + 3)) = color.a;
// 			}
// 		}
// 	}


// 	Texture::Builder& 
// 	Texture::Builder::addLayer(TextureSource&& source)
// 	{
// 		if (m_Width != -1 && m_Height != -1 && m_BitsPerPixel != -1) 
// 		{
// 			VY_ASSERT(
// 				m_Width == source.width() && 
// 				m_Height == source.height() && 
// 				m_BitsPerPixel == source.bitsPerPixel(),
// 				"Layers must have the same dimensions."
// 			);
// 		} 
// 		else 
// 		{
// 			m_Width	       = source.width();
// 			m_Height       = source.height();
// 			m_BitsPerPixel = source.bitsPerPixel();
// 		}

// 		m_Layers.push_back(std::move(source.m_Pixels));
// 		return *this;
// 	}


// 	Unique<Texture> 
// 	Texture::Builder::build()
// 	{
// 		VY_ASSERT(m_Layers.size() > 0, "Can't create texture without layers. See Texture::Builder::addLayer()");

// 		VkDeviceSize layerSize = m_Width * m_Height * m_BitsPerPixel;
// 		VkDeviceSize imageSize = layerSize * m_Layers.size();

// 		if (m_UseMipmaps)
// 		{
// 			m_MipmapCount = (std::floor(std::log2(std::max(m_Width, m_Height)))) + 1;
// 		}

//         VyBuffer stagingBuffer = VyBuffer{ VyBuffer::stagingBuffer(imageSize) };

//         stagingBuffer.map();
// 		for (int i = 0; i < m_Layers.size(); i++) 
// 		{ 
// 			stagingBuffer.singleWrite(m_Layers[i].data(), layerSize, layerSize * i); 
// 		}
//         stagingBuffer.flush();

// 		VkImageCreateInfo imageInfo{ VKInit::imageCreateInfo() };
// 		{
// 			imageInfo.imageType	     = VK_IMAGE_TYPE_2D;
// 			imageInfo.extent.width	 = m_Width;
// 			imageInfo.extent.height  = m_Height;
// 			imageInfo.extent.depth	 = 1;
// 			imageInfo.mipLevels	     = m_MipmapCount;
// 			imageInfo.arrayLayers	 = m_Layers.size();
// 			imageInfo.format		 = m_Format;
// 			imageInfo.tiling		 = VK_IMAGE_TILING_OPTIMAL;
// 			imageInfo.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
// 			imageInfo.usage		     = m_Usage;
// 			imageInfo.sharingMode	 = VK_SHARING_MODE_EXCLUSIVE;
// 			imageInfo.samples		 = VK_SAMPLE_COUNT_1_BIT;
// 			imageInfo.flags		     = (m_IsCubemap ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : 0);
// 		}

// 		VmaAllocationCreateInfo allocInfo{};
//         {
// 			allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
//         }

// 		m_Texture = std::make_unique<Texture>();

// 		VyContext::device().createImage(
// 			m_Texture->m_Image, 
// 			m_Texture->m_Allocation, 
// 			imageInfo, 
// 			allocInfo
// 		);

// 		m_Texture->transitionImageLayout(
// 			m_Format, 
// 			VK_IMAGE_LAYOUT_UNDEFINED, 
// 			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
// 			m_Layers.size(), 
// 			m_MipmapCount
// 		);

// 		VyContext::device().copyBufferToImage(
// 			stagingBuffer.handle(), 
// 			m_Texture->m_Image, 
// 			m_Width, 
// 			m_Height, 
// 			m_Layers.size()
// 		);

// 		if (m_UseMipmaps)
// 		{
// 			generateMipmaps();
// 		}
// 		else
// 		{
// 			m_Texture->transitionImageLayout(
// 				m_Format, 
// 				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
// 				m_Layout, 
// 				m_Layers.size(), 
// 				m_MipmapCount
// 			);
// 		}

// 		m_Texture->m_Layout	       = m_Layout;
// 		m_Texture->m_View          = createImageView();
// 		m_Texture->m_Sampler       = createSampler();

// 		m_Texture->m_Width		   = m_Width;
// 		m_Texture->m_Height		   = m_Height;
// 		m_Texture->m_BitsPerPixel  = m_BitsPerPixel;
// 		m_Texture->m_Layers		   = m_Layers.size();
// 		m_Texture->m_MipMapsLevels = m_MipmapCount;
// 		m_Texture->m_Format		   = m_Format;

// 		return std::move(m_Texture);
// 	}
	

// 	void Texture::Builder::generateMipmaps()
// 	{
// 		VkFormatProperties formatProperties;
// 		vkGetPhysicalDeviceFormatProperties(VyContext::physicalDevice(), m_Format, &formatProperties);

// 		VY_ASSERT(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT,
// 			"Can't generate mipmaps. device is not supported.");

// 		auto cmdBuffer = VyContext::device().beginSingleTimeCommands();

// 		VkImageMemoryBarrier barrier{ VKInit::imageMemoryBarrier() };
// 		{
// 			barrier.image							= m_Texture->m_Image;

// 			barrier.srcQueueFamilyIndex				= VK_QUEUE_FAMILY_IGNORED;
// 			barrier.dstQueueFamilyIndex				= VK_QUEUE_FAMILY_IGNORED;
			
// 			barrier.subresourceRange.aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT;
// 			barrier.subresourceRange.baseArrayLayer = 0;
// 			barrier.subresourceRange.layerCount		= m_Layers.size();
// 			barrier.subresourceRange.levelCount		= 1;

// 			int mipWidth  = m_Width;
// 			int mipHeight = m_Height;

// 			for (int i = 1; i < m_MipmapCount; i++) 
// 			{
// 				barrier.subresourceRange.baseMipLevel = i - 1;

// 				barrier.oldLayout     = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
// 				barrier.newLayout     = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;

// 				barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
// 				barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

// 				vkCmdPipelineBarrier(cmdBuffer, 
// 					VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 
// 					0, nullptr, 
// 					0, nullptr, 
// 					1, 
// 					&barrier
// 				);

// 				VkImageBlit blit{};
// 				{
// 					blit.srcSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
// 					blit.srcSubresource.mipLevel       = i - 1;
// 					blit.srcSubresource.baseArrayLayer = 0;
// 					blit.srcSubresource.layerCount     = m_Layers.size();;
// 					blit.srcOffsets[0]                 = { 0, 0, 0 };
// 					blit.srcOffsets[1].x               = static_cast<I32>(mipWidth  >> (i - 1));
// 					blit.srcOffsets[1].y               = static_cast<I32>(mipHeight >> (i - 1));
// 					blit.srcOffsets[1].z               = 1;
					
// 					blit.dstSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
// 					blit.dstSubresource.mipLevel       = i;
// 					blit.srcSubresource.baseArrayLayer = 0;
// 					blit.dstSubresource.layerCount     = m_Layers.size();;
// 					blit.dstOffsets[0]                 = { 0, 0, 0 };
// 					blit.dstOffsets[1].x               = static_cast<I32>(mipWidth  >> i);
// 					blit.dstOffsets[1].y               = static_cast<I32>(mipHeight >> i);
// 					blit.dstOffsets[1].z               = 1;


// 					// blit.srcOffsets[0]				   = { 0, 0, 0 };
// 					// blit.srcOffsets[1]				   = { mipWidth, mipHeight, 1 };
// 					// blit.srcSubresource.aspectMask	   = VK_IMAGE_ASPECT_COLOR_BIT;
// 					// blit.srcSubresource.mipLevel	   = i - 1;
// 					// blit.srcSubresource.baseArrayLayer = 0;
// 					// blit.srcSubresource.layerCount	   = m_Layers.size();
					
// 					// mipWidth  = mipWidth  >= 2 ? mipWidth  / 2 : 1;
// 					// mipHeight = mipHeight >= 2 ? mipHeight / 2 : 1;
					
// 					// blit.dstOffsets[0]				   = {0, 0, 0};
// 					// blit.dstOffsets[1]				   = {mipWidth, mipHeight, 1};
// 					// blit.dstSubresource.aspectMask	   = VK_IMAGE_ASPECT_COLOR_BIT;
// 					// blit.dstSubresource.mipLevel	   = i;
// 					// blit.dstSubresource.baseArrayLayer = 0;
// 					// blit.dstSubresource.layerCount	   = m_Layers.size();
// 				}

// 				vkCmdBlitImage(cmdBuffer, 
// 					m_Texture->m_Image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, 
// 					m_Texture->m_Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
// 					1, &blit, 
// 					VK_FILTER_LINEAR
// 				);

// 				barrier.subresourceRange.baseMipLevel = i - 1;

// 				barrier.oldLayout     = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
// 				barrier.newLayout     = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

// 				barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
// 				barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

// 				vkCmdPipelineBarrier(cmdBuffer, 
// 					VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 
// 					0, nullptr, 
// 					0, nullptr, 
// 					1, 
// 					&barrier
// 				);
// 			}

// 			barrier.subresourceRange.baseMipLevel = m_MipmapCount - 1;

// 			barrier.oldLayout     = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
// 			barrier.newLayout     = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

// 			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
// 			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
// 		}

// 		vkCmdPipelineBarrier(cmdBuffer, 
// 			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 
// 			0, nullptr, 
// 			0, nullptr, 
// 			1, 
// 			&barrier
// 		);

// 		VyContext::device().endSingleTimeCommands(cmdBuffer);
// 	}


// 	VkImageView 
// 	Texture::Builder::createImageView()
// 	{
// 		VkImageViewCreateInfo viewInfo { VKInit::imageViewCreateInfo() };
// 		{
// 			viewInfo.image     = m_Texture->m_Image;
// 			viewInfo.viewType  = (m_IsCubemap ? VK_IMAGE_VIEW_TYPE_CUBE : VK_IMAGE_VIEW_TYPE_2D);
// 			viewInfo.format    = m_Format;

// 			viewInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
// 			viewInfo.subresourceRange.baseMipLevel   = 0;
// 			viewInfo.subresourceRange.levelCount	 = m_MipmapCount;
// 			viewInfo.subresourceRange.baseArrayLayer = 0;
// 			viewInfo.subresourceRange.layerCount     = m_Layers.size();
// 		}

// 		VkImageView imageView;
// 		VK_CHECK(vkCreateImageView(VyContext::device(), &viewInfo, nullptr, &imageView));

// 		return imageView;
// 	}


// 	VkSampler 
// 	Texture::Builder::createSampler()
// 	{
// 		VkSampler sampler{};

// 		VkSamplerCreateInfo samplerInfo{ VKInit::samplerCreateInfo() };
// 		{
// 			samplerInfo.magFilter			    = m_MinMagFilter;
// 			samplerInfo.minFilter			    = m_MinMagFilter;
// 			samplerInfo.addressModeU		    = m_AddressMode;
// 			samplerInfo.addressModeV		    = m_AddressMode;
// 			samplerInfo.addressModeW		    = m_AddressMode;
// 			samplerInfo.anisotropyEnable	    = VK_TRUE;
// 			samplerInfo.maxAnisotropy		    = VyContext::device().properties().limits.maxSamplerAnisotropy;
// 			samplerInfo.borderColor			    = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
// 			samplerInfo.unnormalizedCoordinates = VK_FALSE;
// 			samplerInfo.compareEnable		    = VK_FALSE;
// 			samplerInfo.compareOp			    = VK_COMPARE_OP_ALWAYS;
// 			samplerInfo.mipmapMode			    = m_MipmapMode;
// 			samplerInfo.mipLodBias			    = 0.0f;
// 			samplerInfo.minLod				    = 0.0f;
// 			samplerInfo.maxLod				    = (float)m_MipmapCount;
// 		}

// 		VK_CHECK(vkCreateSampler(VyContext::device(), &samplerInfo, nullptr, &sampler));

// 		return sampler;
// 	}


// 	Texture::~Texture()
// 	{
// 		VyContext::destroy(m_Sampler);
// 		VyContext::destroy(m_View);
// 		VyContext::destroy(m_Image, m_Allocation);
// 	}


// 	VkDescriptorImageInfo 
// 	Texture::descriptorImageInfo() const
// 	{
// 		VkDescriptorImageInfo imageInfo{ VKInit::imageCreateInfo() };
// 		{
// 			imageInfo.imageLayout = m_Layout;
// 			imageInfo.imageView	  = m_View;
// 			imageInfo.sampler	  = m_Sampler;
// 		}

// 		return imageInfo;
// 	}


// 	void Texture::transitionImageLayout(VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, U32 layerCount, U32 mipmapCount)
// 	{
// 		auto cmdBuffer = VyContext::device().beginSingleTimeCommands();
// 		{
// 			VkImageMemoryBarrier barrier{ VKInit::imageMemoryBarrier() };
// 			{
// 				barrier.image							= m_Image;

// 				barrier.srcQueueFamilyIndex				= VK_QUEUE_FAMILY_IGNORED;
// 				barrier.dstQueueFamilyIndex				= VK_QUEUE_FAMILY_IGNORED;
				
// 				barrier.oldLayout						= oldLayout;
// 				barrier.newLayout						= newLayout;
				
// 				barrier.subresourceRange.aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT;
// 				barrier.subresourceRange.baseMipLevel	= 0;
// 				barrier.subresourceRange.levelCount		= mipmapCount;
// 				barrier.subresourceRange.baseArrayLayer = 0;
// 				barrier.subresourceRange.layerCount		= layerCount;
				
// 				barrier.srcAccessMask					= 0; // TODO
// 				barrier.dstAccessMask					= 0; // TODO
// 			}

// 			VkPipelineStageFlags srcStage;
// 			VkPipelineStageFlags dstStage;

// 			if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) 
// 			{
// 				barrier.srcAccessMask = 0;
// 				barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

// 				srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
// 				dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
// 			} 
// 			else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
// 					newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) 
// 			{
// 				barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
// 				barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

// 				srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
// 				dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
// 			} 
// 			else if (oldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && 
// 					newLayout == VK_IMAGE_LAYOUT_GENERAL) 
// 			{
// 				barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
// 				barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

// 				srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
// 				dstStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
// 			} 
// 			else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && 
// 					newLayout == VK_IMAGE_LAYOUT_GENERAL) 
// 			{
// 				barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
// 				barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

// 				srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
// 				dstStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
// 			} 
// 			else 
// 			{
// 				VY_THROW_INVALID_ARGUMENT("unsupported layout transition!");
// 			}

// 			vkCmdPipelineBarrier(cmdBuffer, srcStage, dstStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
// 		}
// 		VyContext::device().endSingleTimeCommands(cmdBuffer);

// 		m_Layout = newLayout;
// 	}
// }















// namespace Vy
// {
// 	VulkanImage::VulkanImage(const VyImageInfo& info, const DataBuffer& buffer) :
// 		m_Info(info) 
// 	{
// 		// set other members as VK_NULL_HANDLE
// 		m_Image = VK_NULL_HANDLE;
// 		m_Allocation = VK_NULL_HANDLE;
// 		m_View = VK_NULL_HANDLE;
// 		m_Sampler = VK_NULL_HANDLE;

// 		m_imageFormat = m_Info.Format;
// 	}


// 	VulkanImage::VulkanImage(const VkImageCreateInfo& imageInfo, VmaAllocationCreateFlags allocFlags) : 
// 		VulkanImage(VyImageInfo(imageInfo.extent.width, imageInfo.extent.height, 4), DataBuffer()) 
// 	{
// 		// create the image and allocate memory for it
// 		VmaAllocationCreateInfo allocInfo{};
//         {
//             allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
// 			allocInfo.flags = allocFlags;
//         }

// 		VyContext::device().createImage(m_Image, m_Allocation, imageInfo, allocInfo);
// 	}


// 	VulkanImage::~VulkanImage() 
// 	{
// 		if (m_View != VK_NULL_HANDLE) 
// 		{
// 			VyContext::destroy(m_View);
// 		}

// 		// TODO: Samplers should be a separate resource, not tied to the image.
// 		// because then if the image is destroyed, the sampler is also destroyed and
// 		// could be still used/destroyed_twice in other images.
// 		if (m_Sampler != VK_NULL_HANDLE) 
// 		{
// 			VyContext::destroy(m_Sampler);
// 		}

// 		VyContext::destroy(m_Image, m_Allocation);
// 	}


// 	VulkanImage& VulkanImage::createView(const VkImageViewCreateInfo& viewInfo) 
// 	{
// 		if (m_View != VK_NULL_HANDLE) 
// 		{
// 			VyContext::destroy(m_View);
// 		}

// 		m_View = VyContext::device().createImageView(viewInfo);

// 		return *this;
// 	}

	
// 	VulkanImage& VulkanImage::createSampler(const VkSamplerCreateInfo& samplerInfo) 
// 	{
// 		if (m_Sampler != VK_NULL_HANDLE) 
// 		{
// 			VyContext::destroy(m_Sampler);
// 		}

// 		m_Sampler = VyContext::device().createSampler(samplerInfo);

// 		return *this;
// 	}


// 	void VulkanImage::transitionImageLayout(
// 		VkImageLayout                          newLayout,
// 		VkPipelineStageFlags                   srcStage, 
// 		VkPipelineStageFlags                   dstStage,
// 		std::optional<VkImageSubresourceRange> subresourceRange) 
// 	{
// 		VkCommandBuffer cmdBuffer = VyContext::device().beginSingleTimeCommands();

// 		VkImageMemoryBarrier barrier{};
// 		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
// 		barrier.oldLayout = m_CurrentLayout;
// 		barrier.newLayout = newLayout;
// 		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED; // If you are using the barrier to transfer queue family ownership, then these two fields should be the indices of the queue families. 
// 		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED; // They must be set to VK_QUEUE_FAMILY_IGNORED if you don't want to do this (not the default value!).
// 		barrier.image = m_Image;
// 		if (subresourceRange.has_value()) {
// 			barrier.subresourceRange = subresourceRange.value();
// 		}
// 		else {
// 			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
// 			barrier.subresourceRange.baseMipLevel = 0; // if you are using mipMapping
// 			barrier.subresourceRange.levelCount = 1;
// 			barrier.subresourceRange.baseArrayLayer = 0; // if it's an array (1D image)
// 			barrier.subresourceRange.layerCount = 1;
// 		}

// 		// Source layouts (old)
// 		// Source access mask controls actions that have to be finished on the old layout
// 		// before it will be transitioned to the new layout
// 		switch (m_CurrentLayout) {
// 		case VK_IMAGE_LAYOUT_UNDEFINED:
// 			// Image layout is undefined (or does not matter)
// 			// Only valid as initial layout
// 			// No flags required, listed only for completeness
// 			barrier.srcAccessMask = 0;
// 			break;

// 		case VK_IMAGE_LAYOUT_PREINITIALIZED:
// 			// Image is preinitialized
// 			// Only valid as initial layout for linear images, preserves memory contents
// 			// Make sure host writes have been finished
// 			barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
// 			break;

// 		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
// 			// Image is a color attachment
// 			// Make sure any writes to the color buffer have been finished
// 			barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
// 			break;

// 		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
// 			// Image is a depth/stencil attachment
// 			// Make sure any writes to the depth/stencil buffer have been finished
// 			barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
// 			break;

// 		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
// 			// Image is a transfer source
// 			// Make sure any reads from the image have been finished
// 			barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
// 			break;

// 		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
// 			// Image is a transfer destination
// 			// Make sure any writes to the image have been finished
// 			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
// 			break;

// 		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
// 			// Image is read by a shader
// 			// Make sure any shader reads from the image have been finished
// 			barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
// 			break;
// 		case VK_IMAGE_LAYOUT_GENERAL:
// 			// Image is used as a general image
// 			// Make sure any writes to the image have been finished
// 			barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
// 			break;
// 		default:
// 			// Other source layouts aren't handled (yet)
// 			VY_THROW_RUNTIME_ERROR("Unsupported old image layout when transitioning");
// 			break;
// 		}

// 		// Target layouts (new)
// 		// Destination access mask controls the dependency for the new image layout
// 		switch (newLayout) {
// 		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
// 			// Image will be used as a transfer destination
// 			// Make sure any writes to the image have been finished
// 			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
// 			break;

// 		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
// 			// Image will be used as a transfer source
// 			// Make sure any reads from the image have been finished
// 			barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
// 			break;

// 		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
// 			// Image will be used as a color attachment
// 			// Make sure any writes to the color buffer have been finished
// 			barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
// 			break;

// 		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
// 			// Image layout will be used as a depth/stencil attachment
// 			// Make sure any writes to depth/stencil buffer have been finished
// 			barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
// 			break;

// 		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
// 			// Image will be read in a shader (sampler, input attachment)
// 			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
// 			break;
// 		case VK_IMAGE_LAYOUT_GENERAL:
// 			// Image will be used as a general image
// 			// Make sure any writes to the image have been finished
// 			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
// 			break;
// 		default:
// 			// Other source layouts aren't handled (yet)
// 			VY_THROW_RUNTIME_ERROR("Unsupported new image layout when transitioning");
// 			break;
// 		}

// 		vkCmdPipelineBarrier(
// 			cmdBuffer,
// 			srcStage, dstStage,
// 			0,
// 			0, nullptr,
// 			0, nullptr,
// 			1, &barrier
// 		);

// 		VyContext::device().endSingleTimeCommands(cmdBuffer);

// 		// save the layout of the image as it's immediatly transitioned
// 		setImageLayout(newLayout);
// 	}

// 	void VulkanImage::transitionImageLayout(VkCommandBuffer cmdBuffer, VkImageLayout newLayout, VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage, std::optional<VkImageSubresourceRange> subresourceRange)
// 	{
// 		VkImageMemoryBarrier barrier{};
// 		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
// 		barrier.oldLayout = m_CurrentLayout;
// 		barrier.newLayout = newLayout;
// 		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED; // If you are using the barrier to transfer queue family ownership, then these two fields should be the indices of the queue families. 
// 		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED; // They must be set to VK_QUEUE_FAMILY_IGNORED if you don't want to do this (not the default value!).
// 		barrier.image = m_Image;
// 		if (subresourceRange.has_value()) {
// 			barrier.subresourceRange = subresourceRange.value();
// 		}
// 		else {
// 			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
// 			barrier.subresourceRange.baseMipLevel = 0; // if you are using mipMapping
// 			barrier.subresourceRange.levelCount = 1;
// 			barrier.subresourceRange.baseArrayLayer = 0; // if it's an array
// 			barrier.subresourceRange.layerCount = 1;
// 		}

// 		// Source layouts (old)
// 		// Source access mask controls actions that have to be finished on the old layout
// 		// before it will be transitioned to the new layout
// 		switch (m_CurrentLayout) {
// 		case VK_IMAGE_LAYOUT_UNDEFINED:
// 			// Image layout is undefined (or does not matter)
// 			// Only valid as initial layout
// 			// No flags required, listed only for completeness
// 			barrier.srcAccessMask = 0;
// 			break;

// 		case VK_IMAGE_LAYOUT_PREINITIALIZED:
// 			// Image is preinitialized
// 			// Only valid as initial layout for linear images, preserves memory contents
// 			// Make sure host writes have been finished
// 			barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
// 			break;

// 		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
// 			// Image is a color attachment
// 			// Make sure any writes to the color buffer have been finished
// 			barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
// 			break;

// 		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
// 			// Image is a depth/stencil attachment
// 			// Make sure any writes to the depth/stencil buffer have been finished
// 			barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
// 			break;

// 		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
// 			// Image is a transfer source
// 			// Make sure any reads from the image have been finished
// 			barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
// 			break;

// 		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
// 			// Image is a transfer destination
// 			// Make sure any writes to the image have been finished
// 			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
// 			break;

// 		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
// 			// Image is read by a shader
// 			// Make sure any shader reads from the image have been finished
// 			barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
// 			break;
// 		case VK_IMAGE_LAYOUT_GENERAL:
// 			// Image is used as a general image
// 			// Make sure any writes to the image have been finished
// 			barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
// 			break;
// 		default:
// 			// Other source layouts aren't handled (yet)
// 			VY_THROW_RUNTIME_ERROR("Unsupported old image layout when transitioning");
// 			break;
// 		}

// 		// Target layouts (new)
// 		// Destination access mask controls the dependency for the new image layout
// 		switch (newLayout) {
// 		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
// 			// Image will be used as a transfer destination
// 			// Make sure any writes to the image have been finished
// 			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
// 			break;

// 		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
// 			// Image will be used as a transfer source
// 			// Make sure any reads from the image have been finished
// 			barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
// 			break;

// 		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
// 			// Image will be used as a color attachment
// 			// Make sure any writes to the color buffer have been finished
// 			barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
// 			break;

// 		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
// 			// Image layout will be used as a depth/stencil attachment
// 			// Make sure any writes to depth/stencil buffer have been finished
// 			barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
// 			break;

// 		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
// 			// Image will be read in a shader (sampler, input attachment)
// 			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
// 			break;
// 		case VK_IMAGE_LAYOUT_GENERAL:
// 			// Image will be used as a general image
// 			// Make sure any writes to the image have been finished
// 			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
// 			break;
// 		default:
// 			// Other source layouts aren't handled (yet)
// 			VY_THROW_RUNTIME_ERROR("Unsupported new image layout when transitioning");
// 			break;
// 		}

// 		vkCmdPipelineBarrier(
// 			cmdBuffer,
// 			srcStage, dstStage,
// 			0,
// 			0, nullptr,
// 			0, nullptr,
// 			1, &barrier
// 		);

// 		// save the layout of the image
// 		// in this case it's more strange
// 		// essentially we are recording to the command buffer to perfrom
// 		// the transition, so at the end of the gpu command buffer execution
// 		// the image will be in the layout of the last transition of the
// 		// command buffer
// 		setImageLayout(newLayout);
// 	}









// 	Unique<VyTexture2D> VyTexture2D::create(const VyImageInfo& info, const DataBuffer& buffer) 
// 	{
// 		return MakeUnique<VyTexture2D>(info, buffer);
// 	}

// 	VyTexture2D::VyTexture2D(const VyImageInfo& info, const DataBuffer& buffer) : 
// 		VulkanImage(info, buffer) 
// 	{
// 		createTextureImage(buffer);
// 		createTextureView();
// 		createTextureSampler();
// 	}


// 	void VyTexture2D::createTextureImage(const DataBuffer& buffer) 
// 	{
// 		VkDeviceSize imageSize = m_Info.Width * m_Info.Height * m_Info.Channels * VKUtil::bytesPerPixel(m_Info.Format);
// 		// create a staging buffer visible to the host and copy the pixels to it

// 		auto stagingBuffer = VyBuffer{ VyBuffer::stagingBuffer(imageSize) };

// 		// Unique<VulkanBuffer> stagingBuffer = createUnique<VulkanBuffer>(
// 		// 	m_context,
// 		// 	imageSize,
// 		// 	1,
// 		// 	VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
// 		// 	VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
// 		// );

// 		stagingBuffer.map();
// 		stagingBuffer.write(buffer.Bytes, imageSize, 0);
// 		stagingBuffer.unmap();

// 		// create an empty vkImage
// 		createImage(m_Info.Width, m_Info.Height,
// 			VK_IMAGE_TILING_OPTIMAL,
// 			// we want the image to be a transfer destination and sampled to be used in the shaders
// 			VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
// 			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
// 			m_Image
// 		);

// 		// we now change the layout of the image for better destination copy performance (VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
// 		transitionImageLayout(
// 			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
// 			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
// 			VK_PIPELINE_STAGE_TRANSFER_BIT
// 		);

// 		// then we copy the contents of the image (that were inside the stagingBuffer) into the vkImage
// 		VyContext::device().copyBufferToImage(
// 			stagingBuffer.handle(),
// 			m_Image,
// 			m_Info.Width,
// 			m_Info.Height,
// 			1
// 		);

// 		// finally, we change the image layout again to be accessed from the shaders (VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
// 		transitionImageLayout(
// 			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
// 			VK_PIPELINE_STAGE_TRANSFER_BIT,
// 			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT
// 		);
// 	}

// 	void VyTexture2D::createImage(U32 width, U32 height, VkImageTiling tiling,
// 		VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VmaAllocationCreateFlags allocFlags) 
// 	{

// 		VkImageCreateInfo imageInfo{ VKInit::imageCreateInfo() };
// 		imageInfo.imageType = VK_IMAGE_TYPE_2D;
// 		imageInfo.extent = { width, height, 1 };

// 		// The number of mip levels (1 means no mip mapping).
// 		// A full mipmap chain would be 1 + log2(max(width, height, depth)) levels
// 		imageInfo.mipLevels = 1;

// 		// The number of layers in the image (1 means that it's a regular image).
// 		// Values > 1 are used for array textures (e.g., for cube maps, 3D texture atlases, or layered framebuffers).
// 		imageInfo.arrayLayers = 1;

// 		// Specifies the format of the image (color depth, channels, etc.).
// 		// The format affects memory usage and compatibility
// 		imageInfo.format = m_imageFormat;

// 		// Specifies how image data is stored in memory.
// 		// VK_IMAGE_TILING_LINEAR: Texels are laid out in row-major order (similar to CPU memory).
// 		// VK_IMAGE_TILING_OPTIMAL: Texels are laid out in an implementation-defined order for optimal access for GPU.
// 		imageInfo.tiling = tiling;

// 		// The initialLayout specifies the layout of the image data on the GPU at the start.
// 		// VK_IMAGE_LAYOUT_UNDEFINED: Not usable by the GPU and the very first transition will discard the texels.
// 		// We don't care about the texels now that the image is empty, we will change the layout later and then copy
// 		// the pixels to this vkImage.
// 		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

// 		// Specifies the way the image will be used.
// 		// Multiple values can be combined using bitwise OR.
// 		// https://registry.khronos.org/vulkan/specs/latest/man/html/VkImageUsageFlagBits.html#_description
// 		imageInfo.usage = usage;

// 		// Specifies the number of samples per pixel (used for anti-aliasing).
// 		// We don't use MSAA here so we leave it at one sample.
// 		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;

// 		// Defines how the image is shared between queues.
// 		// We want this image to be used only by one queue family (in this case the graphics queue)
// 		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

// 		imageInfo.flags = 0; // optional

// 		VmaAllocationCreateInfo allocInfo{};
//         {
//             allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
// 			allocInfo.flags = allocFlags;
//         }

// 		VyContext::device().createImage(image, m_Allocation, imageInfo, allocInfo);
// 	}

	
// 	void VyTexture2D::createTextureView() 
// 	{
// 		VkImageViewCreateInfo viewInfo{ VKInit::imageViewCreateInfo() };
// 		viewInfo.image = m_Image;
// 		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
// 		viewInfo.format = m_imageFormat;
// 		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
// 		viewInfo.subresourceRange.baseMipLevel = 0;
// 		viewInfo.subresourceRange.levelCount = 1;
// 		viewInfo.subresourceRange.baseArrayLayer = 0;
// 		viewInfo.subresourceRange.layerCount = 1;

// 		m_View = VyContext::device().createImageView(viewInfo);
// 	}

// 	void VyTexture2D::createTextureSampler() {
// 		VkSamplerCreateInfo samplerInfo{ VKInit::samplerCreateInfo() };

// 		VkBool32 useUnnormalizedCoordinates = (m_Info.Flags & ImageFlags::UnnormalizedCoordinates) != ImageFlags::None ? VK_TRUE : VK_FALSE;

// 		// magFilter & minFilter determine how the texture is sampled when scaled up (mag) or down (min).
// 		// VK_FILTER_LINEAR: linear interpolation (blurry but smooth)
// 		// VK_FILTER_LINEAR: nearest neighbor interpolation (pixelated appearance)
// 		samplerInfo.magFilter = m_Info.Filtering;
// 		samplerInfo.minFilter = m_Info.Filtering;

// 		// addressMode (U-V-W): determine what happens when texture coordinates go beyond the image boundaries.
// 		// Example: https://vulkan-tutorial.com/images/texture_addressing.png
// 		// VK_SAMPLER_ADDRESS_MODE_REPEAT: repeats the texture
// 		// VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT: flips every repeat
// 		// VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE: stretches edge texels
// 		// VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER: uses a specified border color
// 		samplerInfo.addressModeU = useUnnormalizedCoordinates ? VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE : VK_SAMPLER_ADDRESS_MODE_REPEAT;
// 		samplerInfo.addressModeV = useUnnormalizedCoordinates ? VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE : VK_SAMPLER_ADDRESS_MODE_REPEAT;
// 		samplerInfo.addressModeW = useUnnormalizedCoordinates ? VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE : VK_SAMPLER_ADDRESS_MODE_REPEAT;

// 		// enable anisotropic filtering, which improves texture quality at oblique angles.
// 		// https://en.wikipedia.org/wiki/Anisotropic_filtering
// 		// If unnormalized coordinates are used, anisotropy is disabled. See Vulkan Specification
// 		if (useUnnormalizedCoordinates) {
// 			samplerInfo.anisotropyEnable = VK_FALSE;
// 		}
// 		else {
// 			samplerInfo.anisotropyEnable = VK_TRUE;
// 			samplerInfo.maxAnisotropy = VyContext::device().properties().limits.maxSamplerAnisotropy;
// 		}
		
// 		// which color to use when sampling outside the image borders (only if address mode is clamp to border)
// 		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;

// 		// VK_FALSE: Uses normalized texture coordinates [0,1) range, useful for giving same coords to different resolution textures.
// 		// VK_TRUE: Uses actual texel coordinates [0, texWidth) and [0, texHeight)]
// 		samplerInfo.unnormalizedCoordinates = useUnnormalizedCoordinates;

// 		// Those are mainly used for percentage-closer filtering on shadow maps.
// 		// https://developer.nvidia.com/gpugems/gpugems/part-ii-lighting-and-shadows/chapter-11-shadow-map-antialiasing
// 		samplerInfo.compareEnable = VK_FALSE;
// 		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

// 		// Mipmapping settings
// 		samplerInfo.mipmapMode = useUnnormalizedCoordinates ? VK_SAMPLER_MIPMAP_MODE_NEAREST : VK_SAMPLER_MIPMAP_MODE_LINEAR;
// 		samplerInfo.mipLodBias = 0.0f;
// 		samplerInfo.minLod = 0.0f;
// 		samplerInfo.maxLod = 0.0f;

// 		if (vkCreateSampler(VyContext::device(), &samplerInfo, nullptr, &m_Sampler) != VK_SUCCESS) {
// 			VY_THROW_RUNTIME_ERROR("failed to create texture sampler!");
// 		}
// 	}
// }














	

