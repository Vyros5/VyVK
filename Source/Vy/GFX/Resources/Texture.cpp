#include <Vy/GFX/Resources/Texture.h>

#include <Vy/GFX/Context.h>

// #include <Vy/GFX/Backend/Pipeline.h>
// #include <Vy/GFX/Backend/Descriptors.h>
#include <Vy/GFX/Backend/Buffer/Buffer.h>

// #include <Vy/Engine.h>
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
			VY_THROW_RUNTIME_ERROR("Failed to load texture image: " + filepath);
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
			VY_THROW_RUNTIME_ERROR("Texture image format does not support linear blitting!");
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