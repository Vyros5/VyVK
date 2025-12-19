#include <Vy/GFX/Backend/Image/Image.h>

#include <Vy/GFX/Context.h>

namespace Vy
{
    VyImage::VyImage(const VyImageCreateInfo& desc) :
		m_Extent     { desc.Extent      },
		m_Format     { desc.Format      },
		m_MipLevels  { desc.MipLevels   },
		m_LayerCount { desc.ArrayLayers },
		m_SampleCount{ desc.Samples     }
    {
        create(desc);
    }


	VyImage::VyImage(VyImage&& other) noexcept
	{
		m_Image       = other.m_Image;
		m_Allocation  = other.m_Allocation;

		m_Format      = other.m_Format;
		m_Extent      = other.m_Extent;
		m_MipLevels   = other.m_MipLevels;
		m_LayerCount  = other.m_LayerCount;
		m_Layout      = other.m_Layout;
		m_SampleCount = other.m_SampleCount;

		other.m_Image      = VK_NULL_HANDLE;
		other.m_Allocation = VK_NULL_HANDLE;
	}


	VyImage::~VyImage()
	{
		destroy();
	}


	VyImage& VyImage::operator=(VyImage&& other) noexcept
	{
		if (this != &other)
		{
			destroy();

			m_Image       = other.m_Image;
			m_Allocation  = other.m_Allocation;

			m_Format      = other.m_Format;
			m_Extent      = other.m_Extent;
			m_MipLevels   = other.m_MipLevels;
			m_LayerCount  = other.m_LayerCount;
			m_Layout      = other.m_Layout;
			m_SampleCount = other.m_SampleCount;

			other.m_Image      = VK_NULL_HANDLE;
			other.m_Allocation = VK_NULL_HANDLE;
		}

		return *this;
	}


	void VyImage::create(const VyImageCreateInfo& desc)
	{
		m_Format      = desc.Format;
		m_Extent      = desc.Extent;
		m_MipLevels   = desc.MipLevels;
		m_LayerCount  = desc.ArrayLayers;
		m_Layout      = VK_IMAGE_LAYOUT_UNDEFINED;
		m_SampleCount = desc.Samples;

		m_CreateInfo = VKInit::imageCreateInfo();
		{
			m_CreateInfo.imageType     = desc.ImageType;
			m_CreateInfo.extent        = desc.Extent;

            // Specifies the format of the image (color depth, channels, etc.).
            // The format affects memory usage and compatibility.
			m_CreateInfo.format        = desc.Format;
			
            // The number of mip levels (1 means no mip mapping).
            // A full mipmap chain would be `1 + log2(max(width, height, depth))` levels.
			m_CreateInfo.mipLevels     = desc.MipLevels;

            // The number of layers in the image (1 means that it's a regular image).
            // Values > 1 are used for array textures (e.g., for cube maps, 3D texture atlases, or layered framebuffers).
			m_CreateInfo.arrayLayers   = desc.ArrayLayers;

            // Specifies the number of samples per pixel (used for anti-aliasing).
            // We don't use MSAA here so we leave it at one sample.
			m_CreateInfo.samples       = desc.Samples;

            // Specifies how image data is stored in memory.
            // VK_IMAGE_TILING_LINEAR:  Texels are laid out in row-major order (similar to CPU memory).
            // VK_IMAGE_TILING_OPTIMAL: Texels are laid out in an implementation-defined order for optimal access for GPU.
			m_CreateInfo.tiling        = desc.Tiling;
			
            // Defines how the image is shared between queues.
            // We want this image to be used only by one queue family (in this case the graphics queue)
			m_CreateInfo.sharingMode   = desc.SharingMode;
			
            // Specifies the layout of the image data on the GPU at the start.
            // VK_IMAGE_LAYOUT_UNDEFINED: Not usable by the GPU and the very first transition will discard the texels.
            // We don't care about the texels now that the image is empty, we will change the layout later and then copy
            // the pixels to this vkImage.
			m_CreateInfo.initialLayout = desc.InitialLayout;

			// Additional flags for special image types (e.g., cube maps, sparse images).
			m_CreateInfo.flags         = desc.Flags;

            // Specifies the way the image will be used.
            // Multiple values can be combined using bitwise OR.
            // VkImageUsageFlagBits: `https://registry.khronos.org/vulkan/specs/latest/man/html/VkImageUsageFlagBits.html#_description`
            m_CreateInfo.usage         = desc.Usage;
			
			if (m_MipLevels > 1)
			{
				m_CreateInfo.usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
			}
		}

		// Auto-calculate mip levels if set to 0.
		if (desc.MipLevels == 0)
		{
			U32 maxDim = std::max(desc.Extent.width, std::max(desc.Extent.height, desc.Extent.depth));
			
            m_MipLevels = static_cast<U32>(std::floor(std::log2(maxDim))) + 1;
		}

		VmaAllocationCreateInfo allocInfo{};
        {
            allocInfo.usage = desc.MemoryUsage;
			allocInfo.flags = desc.AllocFlags;
        }

		VyContext::device().createImage(m_Image, m_Allocation, m_CreateInfo, allocInfo);
	}


	void VyImage::copyFrom(VkCommandBuffer cmdBuffer, const VyBuffer& srcBuffer)
	{
        // Transition image layout to be optimal for receiving data.
		VKCmd::transitionImageLayout(
			cmdBuffer, 
			m_Image, 
			m_Format, 
			m_Layout, 
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
			m_MipLevels, 
			m_LayerCount
		);
		
        // Copy the contents of the image (from the buffer) to the vkImage.
		VKCmd::copyBufferToImage(
			cmdBuffer, 
			srcBuffer.handle(), 
			m_Image, 
			m_Extent, 
			m_LayerCount
		);
		
        // Transition image layout again to be optimal for shader read access.
		VKCmd::transitionImageLayout(
			cmdBuffer, 
			m_Image, 
			m_Format, 
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 
			m_MipLevels, 
			m_LayerCount
		);
	}


	void VyImage::copyFrom(const VyBuffer& srcBuffer)
	{
		VkCommandBuffer cmdBuffer = VyContext::device().beginSingleTimeCommands();
		{
			// Transition image layout to be optimal for receiving data.
			VKCmd::transitionImageLayout(
				cmdBuffer, 
				m_Image, 
				m_Format, 
				m_Layout, 
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
				m_MipLevels, 
				m_LayerCount
			);
			
			// Copy the contents of the image (from the buffer) to the vkImage.
			VKCmd::copyBufferToImage(
				cmdBuffer, 
				srcBuffer.handle(), 
				m_Image, 
				m_Extent, 
				m_LayerCount
			);
			
			// Transition image layout again to be optimal for shader read access.
			VKCmd::transitionImageLayout(
				cmdBuffer, 
				m_Image, 
				m_Format, 
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 
				m_MipLevels, 
				m_LayerCount
			);
		}
		VyContext::device().endSingleTimeCommands(cmdBuffer);
	}


	void VyImage::transitionLayout(VkImageLayout newLayout)
	{
		if (m_Layout == newLayout)
		{
			return;
		}

		VkCommandBuffer cmdBuffer = VyContext::device().beginSingleTimeCommands();
		{
			transitionLayout(cmdBuffer, newLayout);
		}
		VyContext::device().endSingleTimeCommands(cmdBuffer);
	}


	void VyImage::transitionLayout(VkCommandBuffer cmdBuffer, VkImageLayout newLayout)
	{
		if (m_Layout == newLayout)
		{
			return;
		}

		VKCmd::transitionImageLayout(
			cmdBuffer, 
			m_Image, 
			m_Format, 
			m_Layout, 
			newLayout, 
			m_MipLevels, 
			m_LayerCount
		);

		m_Layout = newLayout;
	}


	void VyImage::generateMipmaps(VkCommandBuffer cmdBuffer, VkImageLayout finalLayout)
	{
        // Transition the base level to be optimal for transfer destination.
		transitionLayout(cmdBuffer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		VkImageMemoryBarrier barrier{ VKInit::imageMemoryBarrier() };
		{
			barrier.image               = m_Image;

			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

			barrier.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
			barrier.subresourceRange.levelCount     = 1;
			barrier.subresourceRange.baseArrayLayer = 0;
			barrier.subresourceRange.layerCount     = m_LayerCount;
            
            // Generate mipmaps by blitting from one mip level to the next.
            for (U32 i = 1; i < m_MipLevels; i++)
            {
                barrier.subresourceRange.baseMipLevel = i - 1;
                
                barrier.oldLayout     = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                barrier.newLayout     = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
                
                barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                
                vkCmdPipelineBarrier(cmdBuffer,
                    VK_PIPELINE_STAGE_TRANSFER_BIT, 
                    VK_PIPELINE_STAGE_TRANSFER_BIT, 
                    0,
                    0, nullptr,
                    0, nullptr,
                    1, 
                    &barrier
                );
                
                VkImageBlit blit{};
                {
					// [ Source ]
					{
						blit.srcOffsets[0]                 = { 0, 0, 0 };
						blit.srcOffsets[1].x               = static_cast<I32>(m_Extent.width  >> (i - 1));
						blit.srcOffsets[1].y               = static_cast<I32>(m_Extent.height >> (i - 1));
						blit.srcOffsets[1].z               = 1;
						
						blit.srcSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
						blit.srcSubresource.mipLevel       = i - 1;
						blit.srcSubresource.baseArrayLayer = 0;
						blit.srcSubresource.layerCount     = m_LayerCount;
					}
                    
					// [ Destination ]
					{
						blit.dstOffsets[0]                 = { 0, 0, 0 };
						blit.dstOffsets[1].x               = static_cast<I32>(m_Extent.width  >> i);
						blit.dstOffsets[1].y               = static_cast<I32>(m_Extent.height >> i);
						blit.dstOffsets[1].z               = 1;

						blit.dstSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
						blit.dstSubresource.mipLevel       = i;
						blit.srcSubresource.baseArrayLayer = 0;
						blit.dstSubresource.layerCount     = m_LayerCount;
					}
                }
                
                vkCmdBlitImage(cmdBuffer,
                    m_Image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                    m_Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    1, 
                    &blit,
                    VK_FILTER_LINEAR
                );
                
                // Transition the previous mip level to be optimal for reading in the next iteration or final layout.
                barrier.oldLayout     = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
                barrier.newLayout     = finalLayout;
                barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                barrier.dstAccessMask = VKUtil::dstAccessMask(finalLayout);
                
                vkCmdPipelineBarrier(cmdBuffer,
                    VK_PIPELINE_STAGE_TRANSFER_BIT, 
                    VKUtil::dstStage(barrier.dstAccessMask), 
                    0,
                    0, nullptr,
                    0, nullptr,
                    1, 
                    &barrier
                );
            }
            
            // Transition the last mip level to the final layout.
            barrier.subresourceRange.baseMipLevel = m_MipLevels - 1;
            
            barrier.oldLayout     = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barrier.newLayout     = finalLayout;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VKUtil::dstAccessMask(finalLayout);
        } // barrier end

		vkCmdPipelineBarrier(cmdBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT, 
			VKUtil::dstStage(barrier.dstAccessMask), 
			0,
			0, nullptr,
			0, nullptr,
			1, 
			&barrier
		);

		m_Layout = finalLayout;
	}

	
	void VyImage::destroy()
	{
		VyContext::destroy(m_Image, m_Allocation);
		
		m_Image      = VK_NULL_HANDLE;
		m_Allocation = VK_NULL_HANDLE;
	}
}