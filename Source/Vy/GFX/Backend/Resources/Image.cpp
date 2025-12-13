#include <Vy/GFX/Backend/Resources/Image.h>

#include <Vy/GFX/Context.h>


namespace Vy
{
    VyImage::VyImage(const VyImageCreateInfo& desc) :
		m_Extent    { desc.Extent      },
		m_Format    { desc.Format      },
		m_MipLevels { desc.MipLevels   },
		m_LayerCount{ desc.ArrayLayers }
    {
        create(desc);
    }


	VyImage::VyImage(VyImage&& other) noexcept
	{
		m_Image      = other.m_Image;
		m_Allocation = other.m_Allocation;

		m_Format     = other.m_Format;
		m_Extent     = other.m_Extent;
		m_MipLevels  = other.m_MipLevels;
		m_LayerCount = other.m_LayerCount;
		m_Layout     = other.m_Layout;

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

			m_Image      = other.m_Image;
			m_Allocation = other.m_Allocation;

			m_Format     = other.m_Format;
			m_Extent     = other.m_Extent;
			m_MipLevels  = other.m_MipLevels;
			m_LayerCount = other.m_LayerCount;
			m_Layout     = other.m_Layout;

			other.m_Image      = VK_NULL_HANDLE;
			other.m_Allocation = VK_NULL_HANDLE;
		}

		return *this;
	}


	void VyImage::create(const VyImageCreateInfo& desc)
	{
		m_Format     = desc.Format;
		m_Extent     = desc.Extent;
		m_MipLevels  = desc.MipLevels;
		m_LayerCount = desc.ArrayLayers;
		m_Layout     = VK_IMAGE_LAYOUT_UNDEFINED;

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

		if (desc.MipLevels == 0) // Auto-calculate mip levels if set to 0.
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









// 	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 	// ~~	  Image View	
// 	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// 	//--------------------------------------------------
// 	//    Constructor & Destructor
// 	//--------------------------------------------------
// 	void ImageView::destroy() const
// 	{
// 		if (m_ImageView)
// 		{
// 			VyContext::destroy(m_ImageView);
// 		}
// 	}


// 	//--------------------------------------------------
// 	//    Accessors & Mutators
// 	//--------------------------------------------------
// 	const VkImageView& ImageView::handle() const { return m_ImageView; }
// 	const Image&       ImageView::image()  const { return *m_pOwnerImage; }

// 	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 	// ~~	  Image	
// 	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// 	//--------------------------------------------------
// 	//    Constructor & Destructor
// 	//--------------------------------------------------
// 	Image::Image(Image&& other) noexcept
// 	{
// 		m_Image         = std::move(other.m_Image);
// 		m_ImageInfo     = std::move(other.m_ImageInfo);
// 		m_vImageViews   = std::move(other.m_vImageViews);
// 		m_ImageMemory   = std::move(other.m_ImageMemory);
// 		m_CurrentLayout = std::move(other.m_CurrentLayout);

// 		other.m_vImageViews.clear();

// 		other.m_CurrentLayout = VK_IMAGE_LAYOUT_UNDEFINED;
// 		other.m_Image         = VK_NULL_HANDLE;
// 		other.m_ImageMemory   = VK_NULL_HANDLE;
// 		other.m_ImageInfo     = {};
// 	}

// 	Image& Image::operator=(Image&& other) noexcept
// 	{
// 		if (this == &other)
// 		{
// 			return *this;
// 		}

// 		m_Image = std::move(other.m_Image);
// 		other.m_Image = VK_NULL_HANDLE;
// 		m_vImageViews = std::move(other.m_vImageViews);
// 		other.m_vImageViews.clear();
// 		m_ImageMemory = std::move(other.m_ImageMemory);
// 		other.m_ImageMemory = VK_NULL_HANDLE;
// 		m_CurrentLayout = std::move(other.m_CurrentLayout);
// 		other.m_CurrentLayout = VK_IMAGE_LAYOUT_UNDEFINED;
// 		m_ImageInfo = std::move(other.m_ImageInfo);
// 		other.m_ImageInfo = {};

// 		return *this;
// 	}


// 	void Image::destroy()
// 	{
// 		destroyAllViews();

// 		if (m_ImageMemory && m_Image)
// 		{
// 			VyContext::destroy(m_Image, m_ImageMemory);
// 		}
// 	}


// 	void Image::destroyAllViews()
// 	{
// 		for (auto& view : m_vImageViews)
// 		{
// 			view.destroy();
// 		}
		
// 		m_vImageViews.clear();
// 	}


// 	void Image::destroyViewsFrom(U32 firstViewToRemove)
// 	{
// 		if (firstViewToRemove > static_cast<U32>(m_vImageViews.size() - 1))
// 		{
// 			return;
// 		}

// 		for (U32 i{ firstViewToRemove }; i < m_vImageViews.size(); ++i)
// 		{
// 			m_vImageViews[i].destroy();
// 		}

// 		m_vImageViews.erase(m_vImageViews.begin() + firstViewToRemove, m_vImageViews.end());
// 	}


// 	ImageView& Image::CreateView(VkImageAspectFlags aspectFlags, VkImageViewType viewType, U32 baseMip, U32 mipCount, U32 baseLayer, U32 layerCount)
// 	{
// 		m_vImageViews.emplace_back();
// 		m_vImageViews.back().m_pOwnerImage = this;

// 		VkImageViewCreateInfo viewInfo{};
// 		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
// 		viewInfo.image = m_Image;
// 		viewInfo.viewType = viewType;
// 		viewInfo.format = m_ImageInfo.format;
// 		viewInfo.subresourceRange.aspectMask = aspectFlags;
// 		viewInfo.subresourceRange.baseMipLevel = baseMip;
// 		viewInfo.subresourceRange.levelCount = mipCount;
// 		viewInfo.subresourceRange.baseArrayLayer = baseLayer;
// 		viewInfo.subresourceRange.layerCount = layerCount;
// 		m_vImageViews.back().m_Info = viewInfo;

// 		if (vkCreateImageView(VyContext::device(), &viewInfo, nullptr, &m_vImageViews.back().m_ImageView) != VK_SUCCESS)
// 		{
// 			throw std::runtime_error("Failed to create Image View!");
// 		}

// 		return m_vImageViews.back();
// 	}

// 	//--------------------------------------------------
// 	//    Accessors & Mutators
// 	//--------------------------------------------------
// 	const VkImage& Image::handle()								const { return m_Image; }
// 	const ImageView& Image::view(U32 idx)			const { return m_vImageViews.at(idx); }
// 	U32 Image::viewCount()									const { return static_cast<U32>(m_vImageViews.size()); }
// 	const std::vector<ImageView>& Image::allViews()	const { return m_vImageViews; }

// 	U32 Image::mipLevels()				const		{ return m_ImageInfo.mipLevels; }
// 	U32 Image::layerCount()			const		{ return m_ImageInfo.arrayLayers; }

// 	VkFormat Image::format()				const		{ return m_ImageInfo.format; }
// 	VkExtent3D Image::extent3D()			const		{ return m_ImageInfo.extent; }
// 	VkExtent2D Image::extent2D()			const		{ return VkExtent2D{ m_ImageInfo.extent.width, m_ImageInfo.extent.height}; }
// 	VkImageLayout Image::currentLayout()	const		{ return m_CurrentLayout; }


// 	//--------------------------------------------------
// 	//    Commands
// 	//--------------------------------------------------
// 	void Image::transitionLayout(
// 		const VkCommandBuffer& cmdBuffer, 
// 		VkImageLayout          newLayout,
// 		VkAccessFlags2         srcAccess, 
// 		VkPipelineStageFlags2  srcStage,
// 		VkAccessFlags2         dstAccess, 
// 		VkPipelineStageFlags2  dstStage,
// 		U32                    baseMip, 
// 		U32                    mipCount, 
// 		U32                    baseLayer, 
// 		U32                    layerCount)
// 	{
// 		VkImageMemoryBarrier2 barrier{ VKInit::imageMemoryBarrier2() };
// 		{
// 			barrier.pNext = nullptr;
			
// 			barrier.image = m_Image;

// 			barrier.oldLayout = m_CurrentLayout;
// 			barrier.newLayout = newLayout;
			
// 			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
// 			barrier.srcAccessMask       = srcAccess;
// 			barrier.srcStageMask        = srcStage;
			
// 			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
// 			barrier.dstAccessMask       = dstAccess;
// 			barrier.dstStageMask        = dstStage;

// 			barrier.subresourceRange.aspectMask = VKUtil::aspectFromFormat(m_ImageInfo.format);

// 			barrier.subresourceRange.baseMipLevel   = baseMip;
// 			barrier.subresourceRange.levelCount     = mipCount;
// 			barrier.subresourceRange.baseArrayLayer = baseLayer;
// 			barrier.subresourceRange.layerCount     = layerCount;
// 		}

// 		VkDependencyInfo dependencyInfo{ VKInit::dependencyInfo() };
// 		{
// 			dependencyInfo.dependencyFlags          = 0;
// 			dependencyInfo.pNext                    = nullptr;
// 			dependencyInfo.memoryBarrierCount       = 0;
// 			dependencyInfo.pMemoryBarriers          = nullptr;
// 			dependencyInfo.bufferMemoryBarrierCount = 0;
// 			dependencyInfo.pBufferMemoryBarriers    = nullptr;
// 			dependencyInfo.imageMemoryBarrierCount  = 1;
// 			dependencyInfo.pImageMemoryBarriers     = &barrier;
// 		}

// 		vkCmdPipelineBarrier2(cmdBuffer, &dependencyInfo);

// 		m_CurrentLayout = newLayout;
// 	}


// 	void Image::insertBarrier(
// 		const VkCommandBuffer& cmdBuffer,
// 		VkAccessFlags2 srcAccess, 
// 		VkPipelineStageFlags2 srcStage,
// 		VkAccessFlags2 dstAccess, 
// 		VkPipelineStageFlags2 dstStage)
// 	{
// 		transitionLayout(cmdBuffer, m_CurrentLayout, srcAccess, srcStage, dstAccess, dstStage, 0, mipLevels(), 0, layerCount());
// 	}


// 	void Image::generateMipMaps(U32 texW, U32 texH, U32 mips, U32 layers, VkImageLayout finalLayout)
// 	{
// 		auto cmdBuffer = VyContext::device().beginSingleTimeCommands();
// 		{
// 			transitionLayout(cmdBuffer, 
// 				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
// 				0, 
// 				VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT,
// 				VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
// 				0, mips, 
// 				0, layers
// 			);
			
// 			generateMipMaps(cmdBuffer, texW, texH, mips, layers, finalLayout);
// 		}
// 		VyContext::device().endSingleTimeCommands(cmdBuffer);
// 	}


// 	void Image::generateMipMaps(const VkCommandBuffer& cmdBuffer, U32 texW, U32 texH, U32 mips, U32 layers, VkImageLayout finalLayout)
// 	{
// 		// -- Support --
// 		VkFormat imageFormat = format();

// 		VkFormatProperties formatProperties;
// 		vkGetPhysicalDeviceFormatProperties(VyContext::physicalDevice(), imageFormat, &formatProperties);

// 		if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
// 		{
// 			throw std::runtime_error("texture image format does not support linear blitting!");
// 		}
// 		if (!(formatProperties.linearTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
// 		{
// 			throw std::runtime_error("texture image format does not support linear blitting!");
// 		}

// 		// -- Setup --
// 		VkAccessFlags2        finalDstAccessFlags{};
// 		VkPipelineStageFlags2 finalDstStageFlags{};

// 		if (finalLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
// 		{
// 			finalDstAccessFlags = VK_ACCESS_TRANSFER_WRITE_BIT;
// 			finalDstStageFlags  = VK_PIPELINE_STAGE_TRANSFER_BIT;
// 		}
// 		else if (finalLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
// 		{
// 			finalDstAccessFlags = VK_ACCESS_SHADER_READ_BIT;
// 			finalDstStageFlags  = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
// 		}
// 		else if (finalLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
// 		{
// 			finalDstAccessFlags = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
// 			finalDstStageFlags  = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
// 		}

// 		// -- Blit --
// 		VkImageMemoryBarrier2 barrier{ VKInit::imageMemoryBarrier2() };
// 		{
// 			barrier.pNext = nullptr;
			
// 			barrier.image = m_Image;

// 			barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;				// Gets Overwritten
// 			barrier.newLayout = VK_IMAGE_LAYOUT_UNDEFINED;				// Gets Overwritten
			
// 			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
// 			barrier.dstStageMask        = VK_PIPELINE_STAGE_2_TRANSFER_BIT;	// Gets Overwritten
// 			barrier.dstAccessMask       = 0;									// Gets Overwritten
			
// 			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
// 			barrier.srcStageMask        = VK_PIPELINE_STAGE_2_TRANSFER_BIT;	// Gets Overwritten
// 			barrier.srcAccessMask       = 0;									// Gets Overwritten
			
// 			barrier.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
// 			barrier.subresourceRange.baseMipLevel   = 0;					// Gets Overwritten
// 			barrier.subresourceRange.levelCount     = 1;
// 			barrier.subresourceRange.baseArrayLayer = 0;
// 			barrier.subresourceRange.layerCount     = layers;
// 		}

// 		I32 mipWidth  = static_cast<I32>(texW);
// 		I32 mipHeight = static_cast<I32>(texH);

// 		for (U32 mip{ 1 }; mip < mips; ++mip)
// 		{
// 			// -- Setup Barrier --
// 			barrier.subresourceRange.baseMipLevel = mip - 1;

// 			barrier.oldLayout     = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
// 			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
// 			barrier.srcStageMask  = VK_PIPELINE_STAGE_TRANSFER_BIT;

// 			barrier.newLayout     = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
// 			barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
// 			barrier.dstStageMask  = VK_PIPELINE_STAGE_TRANSFER_BIT;


// 			VkDependencyInfo dependencyInfo{ VKInit::dependencyInfo() };
// 			{
// 				dependencyInfo.dependencyFlags          = 0;
// 				dependencyInfo.pNext                    = nullptr;
// 				dependencyInfo.memoryBarrierCount       = 0;
// 				dependencyInfo.pMemoryBarriers          = nullptr;
// 				dependencyInfo.bufferMemoryBarrierCount = 0;
// 				dependencyInfo.pBufferMemoryBarriers    = nullptr;
// 				dependencyInfo.imageMemoryBarrierCount  = 1;
// 				dependencyInfo.pImageMemoryBarriers     = &barrier;
// 			}

// 			vkCmdPipelineBarrier2(cmdBuffer, &dependencyInfo);

// 			// -- Blit -- 
// 			VkImageBlit2 blit{ VKInit::imageBlit2() };
// 			{
// 				// src
// 				blit.srcOffsets[0] = { .x = 0, .y = 0, .z = 0 };
// 				blit.srcOffsets[1] = { .x = mipWidth, .y = mipHeight, .z = 1 };

// 				blit.srcSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
// 				blit.srcSubresource.mipLevel       = mip - 1;
// 				blit.srcSubresource.baseArrayLayer = 0;
// 				blit.srcSubresource.layerCount     = layers;
				
// 				// dst
// 				blit.dstOffsets[0] = { .x = 0, .y = 0, .z = 0 };
// 				blit.dstOffsets[1] = { .x = mipWidth > 1 ? mipWidth / 2 : 1, .y = mipHeight > 1 ? mipHeight / 2 : 1, .z = 1 };

// 				blit.dstSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
// 				blit.dstSubresource.mipLevel       = mip;
// 				blit.dstSubresource.baseArrayLayer = 0;
// 				blit.dstSubresource.layerCount     = layers;
// 			}

// 			VkBlitImageInfo2 blitInfo{ VKInit::blitImageInfo2() }; 
// 			{
// 				blitInfo.pNext = nullptr;
				
// 				blitInfo.srcImage       = m_Image;
// 				blitInfo.srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
				
// 				blitInfo.dstImage       = m_Image;
// 				blitInfo.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
				
// 				blitInfo.regionCount = 1;
// 				blitInfo.pRegions    = &blit;
				
// 				blitInfo.filter      = VK_FILTER_LINEAR;
// 			}

// 			vkCmdBlitImage2(cmdBuffer, &blitInfo);

// 			// -- Barrier --
// 			barrier.oldLayout     = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
// 			barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
// 			barrier.srcStageMask  = VK_PIPELINE_STAGE_TRANSFER_BIT;

// 			barrier.newLayout     = finalLayout;
// 			barrier.dstAccessMask = finalDstAccessFlags;
// 			barrier.dstStageMask  = finalDstStageFlags;


// 			dependencyInfo = { VKInit::dependencyInfo() };
// 			{
// 				dependencyInfo.dependencyFlags          = 0;
// 				dependencyInfo.pNext                    = nullptr;
// 				dependencyInfo.memoryBarrierCount       = 0;
// 				dependencyInfo.pMemoryBarriers          = nullptr;
// 				dependencyInfo.bufferMemoryBarrierCount = 0;
// 				dependencyInfo.pBufferMemoryBarriers    = nullptr;
// 				dependencyInfo.imageMemoryBarrierCount  = 1;
// 				dependencyInfo.pImageMemoryBarriers     = &barrier;
// 			}

// 			vkCmdPipelineBarrier2(cmdBuffer, &dependencyInfo);

// 			// -- Next Mip --
// 			if (mipWidth  > 1) mipWidth  /= 2;
// 			if (mipHeight > 1) mipHeight /= 2;
// 		}

// 		// -- Barrier --
// 		barrier.subresourceRange.baseMipLevel = mips - 1;

// 		barrier.oldLayout     = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
// 		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
// 		barrier.srcStageMask  = VK_PIPELINE_STAGE_TRANSFER_BIT;

// 		barrier.newLayout     = finalLayout;
// 		barrier.dstAccessMask = finalDstAccessFlags;
// 		barrier.dstStageMask  = finalDstStageFlags;


// 		VkDependencyInfo dependencyInfo{ VKInit::dependencyInfo() };
// 		{
// 			dependencyInfo.dependencyFlags          = 0;
// 			dependencyInfo.pNext                    = nullptr;
// 			dependencyInfo.memoryBarrierCount       = 0;
// 			dependencyInfo.pMemoryBarriers          = nullptr;
// 			dependencyInfo.bufferMemoryBarrierCount = 0;
// 			dependencyInfo.pBufferMemoryBarriers    = nullptr;
// 			dependencyInfo.imageMemoryBarrierCount  = 1;
// 			dependencyInfo.pImageMemoryBarriers     = &barrier;
// 		}

// 		vkCmdPipelineBarrier2(cmdBuffer, &dependencyInfo);

// 		m_CurrentLayout = finalLayout;
// 	}


// 	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 	// ~~	  ImageBuilder	
// 	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// 	//--------------------------------------------------
// 	//    Constructor & Destructor
// 	//--------------------------------------------------
// 	ImageBuilder::ImageBuilder()
// 	{
// 		// Setup Defaults
// 		m_ImageInfo = { VKInit::imageCreateInfo() };
// 		{
// 			m_ImageInfo.imageType     = VK_IMAGE_TYPE_2D;					// CAN CHANGE
// 			m_ImageInfo.extent.width  = 0;									// REQUIRED CHANGE
// 			m_ImageInfo.extent.height = 0;									// REQUIRED CHANGE
// 			m_ImageInfo.extent.depth  = 1;									// CAN CHANGE
// 			m_ImageInfo.mipLevels     = 1;									// CAN CHANGE
// 			m_ImageInfo.arrayLayers   = 1;									// CAN CHANGE
// 			m_ImageInfo.format        = VK_FORMAT_UNDEFINED;						// REQUIRED CHANGE
// 			m_ImageInfo.tiling        = VK_IMAGE_TILING_OPTIMAL;					// CAN CHANGE
// 			m_ImageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;			// CAN'T CHANGE
// 			m_ImageInfo.usage         = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;		// CAN CHANGE
// 			m_ImageInfo.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;			// CAN CHANGE
// 			m_ImageInfo.samples       = VK_SAMPLE_COUNT_1_BIT;					// CAN CHANGE
// 			m_ImageInfo.flags         = 0;											// CAN CHANGE
// 		}

// 		m_AllocInfo = {};
// 		{
// 			m_AllocInfo.usage         = VMA_MEMORY_USAGE_AUTO;						// CAN'T CHANGE
// 			m_AllocInfo.requiredFlags = 0;									// CAN CHANGE
// 		}

// 		m_PreMadeImage   = VK_NULL_HANDLE;								// CAN CHANGE
// 		m_pName          = nullptr;												// CAN CHANGE
// 		m_UseInitialData = false;										// CAN CHANGE
// 		m_pData          = nullptr;												// CAN CHANGE
// 		m_InitDataSize   = 0;												// CAN CHANGE
// 		m_InitDataHeight = 0;											// CAN CHANGE
// 		m_InitDataWidth  = 0;											// CAN CHANGE
// 		m_InitDataOffset = 0;											// CAN CHANGE
// 		m_FinalLayout    = VK_IMAGE_LAYOUT_UNDEFINED;						// CAN CHANGE
// 	}


// 	//--------------------------------------------------
// 	//    Builder
// 	//--------------------------------------------------
// 	ImageBuilder& ImageBuilder::debugName(const char* name) { m_pName = name; return *this; }
// 	ImageBuilder& ImageBuilder::width(U32 width) { m_ImageInfo.extent.width = width; return *this; }
// 	ImageBuilder& ImageBuilder::height(U32 height) { m_ImageInfo.extent.height = height; return *this; }
// 	ImageBuilder& ImageBuilder::depth(U32 depth) { m_ImageInfo.extent.depth = depth; return *this; }
// 	ImageBuilder& ImageBuilder::extent(VkExtent3D extent) { m_ImageInfo.extent = extent; return *this; }
// 	ImageBuilder& ImageBuilder::format(VkFormat format) { m_ImageInfo.format = format; return *this; }
// 	ImageBuilder& ImageBuilder::tiling(VkImageTiling tiling) { m_ImageInfo.tiling = tiling; return *this; }
// 	ImageBuilder& ImageBuilder::usageFlags(VkImageUsageFlags usage) { m_ImageInfo.usage = usage; return *this; }
// 	ImageBuilder& ImageBuilder::createFlags(VkImageCreateFlags flags) { m_ImageInfo.flags |= flags; return *this; }
// 	ImageBuilder& ImageBuilder::memoryProperties(VkMemoryPropertyFlags properties) { m_AllocInfo.requiredFlags = properties; return *this; }
// 	ImageBuilder& ImageBuilder::mipLevels(U32 levels) { m_ImageInfo.mipLevels = levels; return *this; }
// 	ImageBuilder& ImageBuilder::arrayLayers(U32 layers) { m_ImageInfo.arrayLayers = layers; return *this; }
// 	ImageBuilder& ImageBuilder::sampleCount(VkSampleCountFlagBits sampleCount){ m_ImageInfo.samples = sampleCount; return *this; }
// 	ImageBuilder& ImageBuilder::sharingMode(VkSharingMode sharingMode) { m_ImageInfo.sharingMode = sharingMode; return *this; }
// 	ImageBuilder& ImageBuilder::imageType(VkImageType type) { m_ImageInfo.imageType = type; return *this; }
	
// 	ImageBuilder& ImageBuilder::initialData(
// 		void*         pData, 
// 		U32           offset, 
// 		U32           width, 
// 		U32           height, 
// 		U32           dataSize,
// 		VkImageLayout finalLayout)
// 	{
// 		m_UseInitialData   = true;

// 		m_pData            = pData;
// 		m_InitDataOffset   = offset;
// 		m_InitDataSize     = dataSize;
// 		m_InitDataHeight   = height;
// 		m_InitDataWidth    = width;
// 		m_ImageInfo.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		
// 		if (m_ImageInfo.mipLevels > 1)
// 		{
// 			m_ImageInfo.usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
// 		}

// 		m_FinalLayout = finalLayout;

// 		return *this;
// 	}


// 	ImageBuilder& ImageBuilder::preMadeImage(VkImage image)
// 	{
// 		m_PreMadeImage = image;

// 		return *this;
// 	}


// 	void ImageBuilder::build(Image& image) const
// 	{
// 		image.m_ImageInfo     = m_ImageInfo;
// 		image.m_CurrentLayout = m_ImageInfo.initialLayout;
// 		image.m_Image         = m_PreMadeImage;

// 		if (image.m_Image == VK_NULL_HANDLE)
// 		{
// 			if (vmaCreateImage(VyContext::allocator(), &m_ImageInfo, &m_AllocInfo, &image.m_Image, &image.m_ImageMemory, nullptr) != VK_SUCCESS)
// 			{
// 				throw std::runtime_error("Failed to create Image!");
// 			}
// 		}

// 		if (m_UseInitialData)
// 		{
// 			VyBuffer stagingBuffer{ VyBuffer::stagingBuffer(m_InitDataSize) };

// 			stagingBuffer.singleWrite(m_pData, m_InitDataSize, m_InitDataOffset);

// 			auto cmdBuffer = VyContext::device().beginSingleTimeCommands();
// 			{
// 				image.transitionLayout(cmdBuffer, 
// 					VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
// 					0, 
// 					VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT,
// 					VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
// 					0, m_ImageInfo.mipLevels, 
// 					0, m_ImageInfo.arrayLayers
// 				);

// 				VyContext::device().copyBufferToImage(cmdBuffer, 
// 					stagingBuffer.handle(), 
// 					image.handle(), 
// 					m_InitDataWidth, 
// 					m_InitDataHeight, 
// 					1, 
// 					0
// 				);

// 				image.generateMipMaps(cmdBuffer, 
// 					m_InitDataWidth, 
// 					m_InitDataHeight, 
// 					m_ImageInfo.mipLevels, 
// 					m_ImageInfo.arrayLayers, 
// 					m_FinalLayout
// 				);
// 			}
// 			VyContext::device().endSingleTimeCommands(cmdBuffer);
// 		}

// 		if (m_pName)
// 		{
// 			// Debugger::SetDebugObjectName(reinterpret_cast<U64>(image.handle()), VK_OBJECT_TYPE_IMAGE, m_pName);
// 		}
// 	}



// 	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 	//? ~~	  Sampler	
// 	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// 	//--------------------------------------------------
// 	//    Constructor & Destructor
// 	//--------------------------------------------------
// 	void Sampler::destroy()	const 
// 	{ 
// 		VyContext::destroy(m_Sampler);
// 	}

// 	//--------------------------------------------------
// 	//    Accessors & Mutators
// 	//--------------------------------------------------
// 	const VkSampler& Sampler::handle() const { return m_Sampler; }



// 	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 	//? ~~	  SamplerBuilder	
// 	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// 	//--------------------------------------------------
// 	//    Constructor & Destructor
// 	//--------------------------------------------------
// 	SamplerBuilder::SamplerBuilder()
// 	{
// 		m_CreateInfo = { VKInit::samplerCreateInfo() };
// 		{
// 			m_CreateInfo.magFilter = VK_FILTER_LINEAR;								//? CAN CHANGE
// 			m_CreateInfo.minFilter = VK_FILTER_LINEAR;								//? CAN CHANGE
// 			m_CreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;				//! REQUIRED CHANGE
// 			m_CreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;				//! REQUIRED CHANGE
// 			m_CreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;				//! REQUIRED CHANGE
// 			m_CreateInfo.anisotropyEnable = VK_FALSE;								//? CAN CHANGE
// 			m_CreateInfo.maxAnisotropy = 0;											//? CAN CHANGE
// 			m_CreateInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;			//? CAN CHANGE
// 			m_CreateInfo.unnormalizedCoordinates = VK_FALSE;						// CAN'T CHANGE
// 			m_CreateInfo.compareEnable = VK_FALSE;									//? CAN CHANGE
// 			m_CreateInfo.compareOp = VK_COMPARE_OP_ALWAYS;							//? CAN CHANGE
// 			m_CreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;				//? CAN CHANGE
// 			m_CreateInfo.mipLodBias = 0.0f;											//? CAN CHANGE
// 			m_CreateInfo.minLod = 0.0f;												//? CAN CHANGE
// 			m_CreateInfo.maxLod = VK_LOD_CLAMP_NONE;								//? CAN CHANGE
// 		}
// 	}


// 	//--------------------------------------------------
// 	//    Builder
// 	//--------------------------------------------------
// 	SamplerBuilder& SamplerBuilder::filters(VkFilter magFilter, VkFilter minFilter)
// 	{
// 		m_CreateInfo.magFilter = magFilter;
// 		m_CreateInfo.minFilter = minFilter;
// 		return *this;
// 	}
	
// 	SamplerBuilder& SamplerBuilder::addressMode(VkSamplerAddressMode mode)
// 	{
// 		m_CreateInfo.addressModeU = mode;
// 		m_CreateInfo.addressModeV = mode;
// 		m_CreateInfo.addressModeW = mode;
// 		return *this;
// 	}
	
// 	SamplerBuilder& SamplerBuilder::enableAnisotropy(float maxAnisotropy)
// 	{
// 		m_CreateInfo.anisotropyEnable = VK_TRUE;
// 		m_CreateInfo.maxAnisotropy    = maxAnisotropy;
// 		return *this;
// 	}
	
// 	SamplerBuilder& SamplerBuilder::borderColor(VkBorderColor color)
// 	{
// 		m_CreateInfo.borderColor = color;
// 		return *this;
// 	}
	
// 	SamplerBuilder& SamplerBuilder::enableCompare(VkCompareOp op)
// 	{
// 		m_CreateInfo.compareEnable = VK_TRUE;
// 		m_CreateInfo.compareOp     = op;
// 		return *this;
// 	}
	
// 	SamplerBuilder& SamplerBuilder::mipmapMode(VkSamplerMipmapMode mode)
// 	{
// 		m_CreateInfo.mipmapMode = mode;
// 		return *this;
// 	}

// 	SamplerBuilder& SamplerBuilder::mipLevels(float bias, float minMip, float maxMip)
// 	{
// 		m_CreateInfo.mipLodBias = bias;
// 		m_CreateInfo.minLod     = minMip;
// 		m_CreateInfo.maxLod     = maxMip;
// 		return *this;
// 	}

// 	void SamplerBuilder::build(Sampler& sampler) const
// 	{
// 		if (vkCreateSampler(VyContext::device(), &m_CreateInfo, nullptr, &sampler.m_Sampler) != VK_SUCCESS)
// 		{
// 			throw std::runtime_error("Failed to create Sampler!");
// 		}
// 	}
// }















// namespace Vy
// {
//     VyImage::VyImage(const VyImageDesc& desc) :
// 		m_Extent    { desc.Extent     },
// 		m_Format    { desc.Format     },
// 		m_MipLevels { desc.MipLevels  },
// 		m_LayerCount{ desc.LayerCount }
//     {
//         create(desc);
//     }


// 	VyImage::VyImage(VyImage&& other) noexcept
// 	{
// 		m_Image      = other.m_Image;
// 		m_Allocation = other.m_Allocation;

// 		m_Format     = other.m_Format;
// 		m_Extent     = other.m_Extent;
// 		m_MipLevels  = other.m_MipLevels;
// 		m_LayerCount = other.m_LayerCount;
// 		m_Layout     = other.m_Layout;

// 		other.m_Image      = VK_NULL_HANDLE;
// 		other.m_Allocation = VK_NULL_HANDLE;
// 	}


// 	VyImage::~VyImage()
// 	{
// 		destroy();
// 	}


// 	VyImage& VyImage::operator=(VyImage&& other) noexcept
// 	{
// 		if (this != &other)
// 		{
// 			destroy();

// 			m_Image      = other.m_Image;
// 			m_Allocation = other.m_Allocation;

// 			m_Format     = other.m_Format;
// 			m_Extent     = other.m_Extent;
// 			m_MipLevels  = other.m_MipLevels;
// 			m_LayerCount = other.m_LayerCount;
// 			m_Layout     = other.m_Layout;

// 			other.m_Image      = VK_NULL_HANDLE;
// 			other.m_Allocation = VK_NULL_HANDLE;
// 		}

// 		return *this;
// 	}
    

//     void VyImage::create(const VyImageDesc& desc)
//     {
// 		m_Format     = desc.Format;
// 		m_Extent     = desc.Extent;
// 		m_MipLevels  = desc.MipLevels;
// 		m_LayerCount = desc.LayerCount;
// 		m_Layout     = VK_IMAGE_LAYOUT_UNDEFINED;

// 		if (desc.MipLevels == VyImageDesc::kCalculateMipLevels)
// 		{
// 			U32 maxDim = std::max(desc.Extent.width, std::max(desc.Extent.height, desc.Extent.depth));
			
//             m_MipLevels = static_cast<U32>(std::floor(std::log2(maxDim))) + 1;
// 		}

// 		VkImageCreateInfo imageInfo{ VKInit::imageCreateInfo() };
//         {
//             imageInfo.imageType     = desc.Type;
//             imageInfo.extent        = m_Extent;

//             // The number of mip levels (1 means no mip mapping).
//             // A full mipmap chain would be `1 + log2(max(width, height, depth))` levels.
//             imageInfo.mipLevels     = m_MipLevels;

//             // The number of layers in the image (1 means that it's a regular image).
//             // Values > 1 are used for array textures (e.g., for cube maps, 3D texture atlases, or layered framebuffers).
//             imageInfo.arrayLayers   = m_LayerCount;

//             // Specifies the format of the image (color depth, channels, etc.).
//             // The format affects memory usage and compatibility.
//             imageInfo.format        = m_Format;

//             // Specifies how image data is stored in memory.
//             // VK_IMAGE_TILING_LINEAR:  Texels are laid out in row-major order (similar to CPU memory).
//             // VK_IMAGE_TILING_OPTIMAL: Texels are laid out in an implementation-defined order for optimal access for GPU.
//             imageInfo.tiling        = desc.Tiling;

//             // Specifies the layout of the image data on the GPU at the start.
//             // VK_IMAGE_LAYOUT_UNDEFINED: Not usable by the GPU and the very first transition will discard the texels.
//             // We don't care about the texels now that the image is empty, we will change the layout later and then copy
//             // the pixels to this vkImage.
//             imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

//             // Specifies the number of samples per pixel (used for anti-aliasing).
//             // We don't use MSAA here so we leave it at one sample.
//             imageInfo.samples       = VK_SAMPLE_COUNT_1_BIT;

//             // Defines how the image is shared between queues.
//             // We want this image to be used only by one queue family (in this case the graphics queue)
//             imageInfo.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;

//             // Additional flags for special image types (e.g., cube maps, sparse images).
//             imageInfo.flags         = desc.Flags;

//             // Specifies the way the image will be used.
//             // Multiple values can be combined using bitwise OR.
//             // VkImageUsageFlagBits: `https://registry.khronos.org/vulkan/specs/latest/man/html/VkImageUsageFlagBits.html#_description`
//             imageInfo.usage         = desc.Usage;
			
// 			if (m_MipLevels > 1)
// 			{
// 				imageInfo.usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
// 			}
// 		}

// 		VmaAllocationCreateInfo allocInfo{};
//         {
//             allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
//         }

// 		VyContext::device().createImage(m_Image, m_Allocation, imageInfo, allocInfo);
//     }


// 	void VyImage::upload(const VyBuffer& srcBuffer)
// 	{
// 		VkCommandBuffer cmdBuffer = VyContext::device().beginSingleTimeCommands();
// 		{
//             // Transition image layout to be optimal for receiving data.
//             VKCmd::transitionImageLayout(
//                 cmdBuffer, 
//                 m_Image, 
//                 m_Format, 
//                 m_Layout, 
//                 VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
//                 m_MipLevels, 
//                 m_LayerCount
//             );

//             // Copy the contents of the image (from the buffer) to the vkImage (m_Image).
//             VKCmd::copyBufferToImage(
//                 cmdBuffer, 
//                 srcBuffer.handle(), 
//                 m_Image, 
//                 m_Extent, 
//                 m_LayerCount
//             );

//             // Generate mipmaps.
// 			generateMipmaps(cmdBuffer, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

// 		}
// 		VyContext::device().endSingleTimeCommands(cmdBuffer);
// 	}


// 	void VyImage::upload(const void* data, VkDeviceSize size)
// 	{
// 		VyBuffer stagingBuffer{ VyBuffer::stagingBuffer(size) };

// 		stagingBuffer.singleWrite(data);
		
//         upload(stagingBuffer);
// 	}
    

// 	void VyImage::copyFrom(VkCommandBuffer cmdBuffer, const VyBuffer& srcBuffer)
// 	{
//         // Transition image layout to be optimal for receiving data.
// 		VKCmd::transitionImageLayout(
// 			cmdBuffer, 
// 			m_Image, 
// 			m_Format, 
// 			m_Layout, 
// 			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
// 			m_MipLevels, 
// 			m_LayerCount
// 		);
		
//         // Copy the contents of the image (from the buffer) to the vkImage.
// 		VKCmd::copyBufferToImage(
// 			cmdBuffer, 
// 			srcBuffer.handle(), 
// 			m_Image, 
// 			m_Extent, 
// 			m_LayerCount
// 		);
		
//         // Transition image layout again to be optimal for shader read access.
// 		VKCmd::transitionImageLayout(
// 			cmdBuffer, 
// 			m_Image, 
// 			m_Format, 
// 			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
// 			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 
// 			m_MipLevels, 
// 			m_LayerCount
// 		);
// 	}


// 	void VyImage::resize(VkExtent3D extent, VkImageUsageFlags usage)
// 	{
// 		destroy();

// 		VyImageDesc desc{
// 			.Format     = m_Format,
// 			.Extent     = extent,
// 			.MipLevels  = m_MipLevels,
// 			.LayerCount = m_LayerCount,
// 			.Usage      = usage
// 		};

// 		create(desc);
// 	}


// 	void VyImage::transitionLayout(VkImageLayout newLayout)
// 	{
// 		if (m_Layout == newLayout)
// 		{
// 			return;
// 		}

// 		VkCommandBuffer cmdBuffer = VyContext::device().beginSingleTimeCommands();
// 		{
// 			transitionLayout(cmdBuffer, newLayout);
// 		}
// 		VyContext::device().endSingleTimeCommands(cmdBuffer);
// 	}


// 	void VyImage::transitionLayout(VkCommandBuffer cmdBuffer, VkImageLayout newLayout)
// 	{
// 		if (m_Layout == newLayout)
// 		{
// 			return;
// 		}

// 		VKCmd::transitionImageLayout(
// 			cmdBuffer, 
// 			m_Image, 
// 			m_Format, 
// 			m_Layout, 
// 			newLayout, 
// 			m_MipLevels, 
// 			m_LayerCount
// 		);

// 		m_Layout = newLayout;
// 	}


// 	VkImageMemoryBarrier VyImage::transitionLayoutDeferred(VkImageLayout newLayout)
// 	{
// 		VkImageMemoryBarrier barrier{ VKInit::imageMemoryBarrier() };
// 		{
// 			barrier.oldLayout           = m_Layout;
// 			barrier.newLayout           = newLayout;
// 			barrier.image               = m_Image;

// 			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
// 			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

// 			barrier.subresourceRange.aspectMask     = VKUtil::aspectFlags(m_Format);
// 			barrier.subresourceRange.baseMipLevel   = 0;
// 			barrier.subresourceRange.levelCount     = m_MipLevels;
// 			barrier.subresourceRange.baseArrayLayer = 0;
// 			barrier.subresourceRange.layerCount     = m_LayerCount;

// 			barrier.srcAccessMask = VKUtil::srcAccessMask(barrier.oldLayout);
// 			barrier.dstAccessMask = VKUtil::dstAccessMask(barrier.newLayout);
// 		}

// 		m_Layout = newLayout;

// 		return barrier;
// 	}


// 	void VyImage::generateMipmaps(VkCommandBuffer cmdBuffer, VkImageLayout finalLayout)
// 	{
//         // Transition the base level to be optimal for transfer destination.
// 		transitionLayout(cmdBuffer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

// 		VkImageMemoryBarrier barrier{ VKInit::imageMemoryBarrier() };
// 		{
// 			barrier.image               = m_Image;

// 			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
// 			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

// 			barrier.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
// 			barrier.subresourceRange.levelCount     = 1;
// 			barrier.subresourceRange.baseArrayLayer = 0;
// 			barrier.subresourceRange.layerCount     = m_LayerCount;
            
//             // Generate mipmaps by blitting from one mip level to the next.
//             for (U32 i = 1; i < m_MipLevels; i++)
//             {
//                 barrier.subresourceRange.baseMipLevel = i - 1;
                
//                 barrier.oldLayout     = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
//                 barrier.newLayout     = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
                
//                 barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
//                 barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                
//                 vkCmdPipelineBarrier(cmdBuffer,
//                     VK_PIPELINE_STAGE_TRANSFER_BIT, 
//                     VK_PIPELINE_STAGE_TRANSFER_BIT, 
//                     0,
//                     0, nullptr,
//                     0, nullptr,
//                     1, 
//                     &barrier
//                 );
                
//                 VkImageBlit blit{};
//                 {
// 					// [ Source ]
// 					{
// 						blit.srcOffsets[0]                 = { 0, 0, 0 };
// 						blit.srcOffsets[1].x               = static_cast<I32>(m_Extent.width  >> (i - 1));
// 						blit.srcOffsets[1].y               = static_cast<I32>(m_Extent.height >> (i - 1));
// 						blit.srcOffsets[1].z               = 1;
						
// 						blit.srcSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
// 						blit.srcSubresource.mipLevel       = i - 1;
// 						blit.srcSubresource.baseArrayLayer = 0;
// 						blit.srcSubresource.layerCount     = m_LayerCount;
// 					}
                    
// 					// [ Destination ]
// 					{
// 						blit.dstOffsets[0]                 = { 0, 0, 0 };
// 						blit.dstOffsets[1].x               = static_cast<I32>(m_Extent.width  >> i);
// 						blit.dstOffsets[1].y               = static_cast<I32>(m_Extent.height >> i);
// 						blit.dstOffsets[1].z               = 1;

// 						blit.dstSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
// 						blit.dstSubresource.mipLevel       = i;
// 						blit.srcSubresource.baseArrayLayer = 0;
// 						blit.dstSubresource.layerCount     = m_LayerCount;
// 					}
//                 }
                
//                 vkCmdBlitImage(cmdBuffer,
//                     m_Image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
//                     m_Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
//                     1, 
//                     &blit,
//                     VK_FILTER_LINEAR
//                 );
                
//                 // Transition the previous mip level to be optimal for reading in the next iteration or final layout.
//                 barrier.oldLayout     = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
//                 barrier.newLayout     = finalLayout;
//                 barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
//                 barrier.dstAccessMask = VKUtil::dstAccessMask(finalLayout);
                
//                 vkCmdPipelineBarrier(cmdBuffer,
//                     VK_PIPELINE_STAGE_TRANSFER_BIT, 
//                     VKUtil::dstStage(barrier.dstAccessMask), 
//                     0,
//                     0, nullptr,
//                     0, nullptr,
//                     1, 
//                     &barrier
//                 );
//             }
            
//             // Transition the last mip level to the final layout.
//             barrier.subresourceRange.baseMipLevel = m_MipLevels - 1;
            
//             barrier.oldLayout     = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
//             barrier.newLayout     = finalLayout;
//             barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
//             barrier.dstAccessMask = VKUtil::dstAccessMask(finalLayout);
//         } // barrier end

// 		vkCmdPipelineBarrier(cmdBuffer,
// 			VK_PIPELINE_STAGE_TRANSFER_BIT, 
// 			VKUtil::dstStage(barrier.dstAccessMask), 
// 			0,
// 			0, nullptr,
// 			0, nullptr,
// 			1, 
// 			&barrier
// 		);

// 		m_Layout = finalLayout;
// 	}

	
// 	void VyImage::destroy()
// 	{
// 		VyContext::destroy(m_Image, m_Allocation);
		
// 		m_Image      = VK_NULL_HANDLE;
// 		m_Allocation = VK_NULL_HANDLE;
// 	}
// }