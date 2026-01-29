#include <VyEngine/VK/Image/Image.h>

#include <VyEngine/VK/Context.h>

namespace Vy
{
    VyImage::VyImage(const TString& name, const VyImageInfo& info) :
        m_DebugName{ name + "_image" },
        m_Info     { info            }
    {
        this->create( info );
    }


    VyImage::~VyImage()
    {
        this->destroy();
    }


    VyImage::VyImage(VyImage&& other) noexcept
    {
        this->swap(other);
    }


    VyImage& VyImage::operator=(VyImage&& other) noexcept
    {
        this->swap(other);

        return *this;
    }


    void VyImage::swap(VyImage& other)
    {
        std::swap(m_Image,         other.m_Image);
        std::swap(m_ImageMemory,   other.m_ImageMemory);
        std::swap(m_Info,          other.m_Info);
        // std::swap(m_MipLevels,     other.m_MipLevels);
        std::swap(m_CurrentLayout, other.m_CurrentLayout);
        std::swap(m_DebugName,     other.m_DebugName);
    }


    void VyImage::create(const VyImageInfo& info)
    {
		if (info.MipLevels == VyImageInfo::CALCULATE_MIP_LEVELS)
		{
			U32 maxDim = std::max(info.Extent.width, std::max(info.Extent.height, info.Extent.depth));
			
            m_Info.MipLevels = static_cast<U32>(std::floor(std::log2(maxDim))) + 1;
		}

        auto imageInfo = VKInit::imageCreateInfo();
        {
            imageInfo.imageType     = info.ImageType;
            imageInfo.extent        = info.Extent;
            imageInfo.mipLevels     = m_Info.MipLevels;
            imageInfo.arrayLayers   = info.ArrayLayers;
            imageInfo.format        = info.Format;
            imageInfo.tiling        = info.Tiling;
            imageInfo.initialLayout = info.InitialLayout;
            imageInfo.usage         = info.Usage;
            imageInfo.sharingMode   = info.SharingMode;
            imageInfo.samples       = info.Samples;
            imageInfo.flags         = info.Flags;
        }

		if (m_Info.MipLevels > 1)
		{
			imageInfo.usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		}

		VmaAllocationCreateInfo allocInfo{};
        {
            allocInfo.usage = info.MemoryUsage;
            allocInfo.flags = info.AllocFlags;
        }

		VK_CHECK_SUCCESS(vmaCreateImage(
			VyContext::allocator(), 
			&imageInfo, 
			&allocInfo, 
			&m_Image, 
			&m_ImageMemory, 
			nullptr
		), "Failed to create image!");

        VKDbg::setObjectName(m_Image, m_DebugName.c_str());
    }


    void VyImage::upload(const VyBuffer& srcBuffer)
    {
		VkCommandBuffer cmdBuffer = VyContext::beginCommands();
		{
			// Transition image layout to be optimal for receiving data.
			VKCmd::transitionImageLayout(
				cmdBuffer, 
				m_Image, 
				m_Info.Format, 
				m_CurrentLayout, 
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
				m_Info.MipLevels, 
				m_Info.ArrayLayers
			);
			
			// Copy the contents of the image (from the buffer) to the vkImage.
			VKCmd::copyBufferToImage(
				cmdBuffer, 
				srcBuffer.handle(), 
				m_Image, 
				m_Info.Extent, 
				m_Info.ArrayLayers
			);
			
            if (m_Info.MipLevels > 1)
            {
                // Transition to shader-read-only and generate mipmaps.
                generateMipmaps(
                    cmdBuffer, 
                    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
                );
            }
            else
            {
                transitionLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
            }
		}
		VyContext::endCommands(cmdBuffer);
    }


	void VyImage::upload(const void* pData, VkDeviceSize size)
	{
		VyBuffer stagingBuffer{ VyBuffer::stagingBuffer("image_upload", size) };

		stagingBuffer.singleWrite( pData );
		
		upload( stagingBuffer );
	}


	void VyImage::copyFrom(VkCommandBuffer cmdBuffer, const VyBuffer& srcBuffer, bool toShaderReadOnly)
	{
        // Transition image layout to be optimal for receiving data.
        transitionLayout(cmdBuffer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        // Copy the contents of the image (from the buffer) to the vkImage.
		VKCmd::copyBufferToImage(
			cmdBuffer, 
			srcBuffer.handle(), 
			m_Image, 
            m_Info.Extent, 
            m_Info.ArrayLayers
		);
		
		if (toShaderReadOnly)
		{
			// Transition image layout again to be optimal for shader read access.
            transitionLayout(cmdBuffer, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		}
	}


	void VyImage::copyFrom(const VyBuffer& srcBuffer, bool toShaderReadOnly)
	{
		VkCommandBuffer cmdBuffer = VyContext::beginCommands();
		{
			copyFrom(cmdBuffer, srcBuffer, toShaderReadOnly);
		}
		VyContext::endCommands(cmdBuffer);
	}


	void VyImage::transitionLayout(VkImageLayout newLayout)
	{
		if (m_CurrentLayout == newLayout)
		{
			return;
		}

		VkCommandBuffer cmdBuffer = VyContext::beginCommands();
		{
			transitionLayout(cmdBuffer, newLayout);
		}
		VyContext::endCommands(cmdBuffer);
	}


	void VyImage::transitionLayout(VkCommandBuffer cmdBuffer, VkImageLayout newLayout)
	{
		if (m_CurrentLayout == newLayout)
		{
			return;
		}

		VKCmd::transitionImageLayout(
			cmdBuffer, 
			m_Image, 
			m_Info.Format, 
			m_CurrentLayout, 
			newLayout, 
            m_Info.MipLevels, 
            m_Info.ArrayLayers
		);

		m_CurrentLayout = newLayout;
	}


	void VyImage::generateMipmaps(VkCommandBuffer cmdBuffer, VkImageLayout finalLayout)
	{
		// Check if image format supports linear blitting.
        VkFormat imageFormat = this->format();
        
		VkFormatProperties formatProperties;
		vkGetPhysicalDeviceFormatProperties(VyContext::physicalDevice(), imageFormat, &formatProperties);

		if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
		{
			VY_THROW_RUNTIME_ERROR("Image format does not support linear blitting!");
		}
        if (!(formatProperties.linearTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
        {
            VY_THROW_RUNTIME_ERROR("Image format does not support linear blitting!");
        }

        // Transition the base level to be optimal for transfer destination.
		transitionLayout(cmdBuffer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        auto mipCount = this->mipLevels();
        auto layers   = this->layerCount();
        auto texW     = this->width();
        auto texH     = this->height();

		VkImageMemoryBarrier barrier{ VKInit::imageMemoryBarrier() };
		{
			barrier.image = m_Image;

			barrier.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
			barrier.subresourceRange.levelCount     = 1;
			barrier.subresourceRange.baseArrayLayer = 0;
			barrier.subresourceRange.layerCount     = layers;
            
            // Generate mipmaps by blitting from one mip level to the next.
            for (U32 mip = 1; mip < mipCount; mip++)
            {
                barrier.subresourceRange.baseMipLevel = mip - 1;
                
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
                    1, &barrier
                );
                
                VkImageBlit blit{};
                {
					// [ Source ]
					{
						blit.srcOffsets[0]                 = { 0, 0, 0 };
						blit.srcOffsets[1].x               = static_cast<I32>(texW >> (mip - 1));
						blit.srcOffsets[1].y               = static_cast<I32>(texH >> (mip - 1));
						blit.srcOffsets[1].z               = 1;
						
						blit.srcSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
						blit.srcSubresource.mipLevel       = mip - 1;
						blit.srcSubresource.baseArrayLayer = 0;
						blit.srcSubresource.layerCount     = layers;
					}
                    
					// [ Destination ]
					{
						blit.dstOffsets[0]                 = { 0, 0, 0 };
						blit.dstOffsets[1].x               = static_cast<I32>(texW >> mip);
						blit.dstOffsets[1].y               = static_cast<I32>(texH >> mip);
						blit.dstOffsets[1].z               = 1;

						blit.dstSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
						blit.dstSubresource.mipLevel       = mip;
						blit.srcSubresource.baseArrayLayer = 0;
						blit.dstSubresource.layerCount     = layers;
					}
                }
                
                vkCmdBlitImage(cmdBuffer,
                    m_Image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                    m_Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    1, &blit,
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
                    1, &barrier
                );
            }
            
            // Transition the last mip level to the final layout.
            barrier.subresourceRange.baseMipLevel = mipCount - 1;
            
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
			1, &barrier
		);

		m_CurrentLayout = finalLayout;
	}


	void VyImage::generateMipmaps(VkImageLayout finalLayout)
	{
		VkCommandBuffer cmdBuffer = VyContext::beginCommands();
		{
			generateMipmaps(cmdBuffer, finalLayout);
		}
		VyContext::endCommands(cmdBuffer);
	}


    void VyImage::generateMipmaps(VkCommandBuffer cmdBuffer, U32 texW, U32 texH, U32 mipCount, U32 layerCount, VkImageLayout finalLayout)
    {
        // Check if image format supports linear blitting.
        VkFormat imageFormat = this->format();
        
		VkFormatProperties formatProperties;
		vkGetPhysicalDeviceFormatProperties(VyContext::physicalDevice(), imageFormat, &formatProperties);

		if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
		{
			VY_THROW_RUNTIME_ERROR("Image format does not support linear blitting!");
		}
        
        if (!(formatProperties.linearTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
        {
            VY_THROW_RUNTIME_ERROR("Image format does not support linear blitting!");
        }

        if (m_CurrentLayout != VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
        {
            VY_WARN_TAG("VyImage", "Image layout not set to transfer_dst prior to calling generateMipmaps(). Transitioning layout...");

            transitionLayout(cmdBuffer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        }

		VkImageMemoryBarrier barrier{ VKInit::imageMemoryBarrier() };
		{
			barrier.image = m_Image;

			barrier.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
			barrier.subresourceRange.levelCount     = 1;
			barrier.subresourceRange.baseArrayLayer = 0;
			barrier.subresourceRange.layerCount     = layerCount;
            
            // Generate mipmaps by blitting from one mip level to the next.
            for (U32 mip = 1; mip < mipCount; mip++)
            {
                barrier.subresourceRange.baseMipLevel = mip - 1;
                
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
                    1, &barrier
                );
                
                VkImageBlit blit{};
                {
					// [ Source ]
					{
						blit.srcOffsets[0]                 = { 0, 0, 0 };
						blit.srcOffsets[1].x               = static_cast<I32>(texW >> (mip - 1));
						blit.srcOffsets[1].y               = static_cast<I32>(texH >> (mip - 1));
						blit.srcOffsets[1].z               = 1;
						
						blit.srcSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
						blit.srcSubresource.mipLevel       = mip - 1;
						blit.srcSubresource.baseArrayLayer = 0;
						blit.srcSubresource.layerCount     = layerCount;
					}
                    
					// [ Destination ]
					{
						blit.dstOffsets[0]                 = { 0, 0, 0 };
						blit.dstOffsets[1].x               = static_cast<I32>(texW >> mip);
						blit.dstOffsets[1].y               = static_cast<I32>(texH >> mip);
						blit.dstOffsets[1].z               = 1;

						blit.dstSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
						blit.dstSubresource.mipLevel       = mip;
						blit.srcSubresource.baseArrayLayer = 0;
						blit.dstSubresource.layerCount     = layerCount;
					}
                }
                
                vkCmdBlitImage(cmdBuffer,
                    m_Image,  VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                    m_Image,  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    1, &blit, VK_FILTER_LINEAR
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
                    1, &barrier
                );
            }
            
            // Transition the last mip level to the final layout.
            barrier.subresourceRange.baseMipLevel = mipCount - 1;
            
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
			1, &barrier
		);

        m_CurrentLayout = finalLayout;
    }


    void VyImage::generateMipmaps(U32 texW, U32 texH, U32 mipCount, U32 layerCount, VkImageLayout finalLayout)
    {
        auto cmdBuffer = VyContext::beginCommands();
        {
            generateMipmaps(cmdBuffer, texW, texH, mipCount, layerCount, finalLayout);
        }
        VyContext::endCommands(cmdBuffer);
    }


	void VyImage::destroy()
	{
		VyContext::destroy(m_Image, m_ImageMemory);
        // VyContext::destroy(m_ImageView);
		
		m_Image       = VK_NULL_HANDLE;
		m_ImageMemory = VK_NULL_HANDLE;
        // m_ImageView   = VK_NULL_HANDLE;
	}




    

    VyImage::Builder::Builder()
    {
        // m_PreMadeImage   = VK_NULL_HANDLE;
        // m_UseInitialData = false;
        // m_pData          = nullptr;
        // m_InitDataSize   = 0;
        // m_InitDataHeight = 0;
        // m_InitDataWidth  = 0;
        // m_InitDataOffset = 0;
        // m_FinalLayout    = VK_IMAGE_LAYOUT_UNDEFINED;
    }


    VyImage::Builder& 
    VyImage::Builder::setName(const TString& name)
    {
        m_Name = name;

        return *this;
    }
    
    VyImage::Builder& 
    VyImage::Builder::setWidth(U32 width)
    {
        m_Info.Extent.width = width;

        return *this;
    }
    
    VyImage::Builder& 
    VyImage::Builder::setHeight(U32 height)
    {
        m_Info.Extent.height = height;

        return *this;
    }
    
    VyImage::Builder& 
    VyImage::Builder::setDepth(U32 depth)
    {
        m_Info.Extent.depth = depth;

        return *this;
    }
    
    VyImage::Builder& 
    VyImage::Builder::setExtent(VkExtent2D extent)
    {
        m_Info.Extent = VkExtent3D{ extent.width, extent.height, 1 };

        return *this;
    }
    
    VyImage::Builder& 
    VyImage::Builder::setExtent(VkExtent3D extent)
    {
        m_Info.Extent = extent;

        return *this;
    }
    
    VyImage::Builder& 
    VyImage::Builder::setExtent(U32 width, U32 height, U32 depth)
    {
        m_Info.Extent = VkExtent3D{ width, height, depth };

        return *this;
    }
    
    VyImage::Builder& 
    VyImage::Builder::setImageType(VkImageType type)
    {
        m_Info.ImageType = type;

        return *this;
    }
    
    VyImage::Builder& 
    VyImage::Builder::setLevels(U32 levels)
    {
        m_Info.MipLevels = levels;

        return *this;
    }
    
    VyImage::Builder& 
    VyImage::Builder::setLayers(U32 layers)
    {
        m_Info.ArrayLayers = layers;

        return *this;
    }
    
    VyImage::Builder& 
    VyImage::Builder::setFormat(VkFormat format)
    {
        m_Info.Format = format;

        return *this;
    }
    
    VyImage::Builder& 
    VyImage::Builder::setLayout(VkImageLayout layout)
    {
        m_Info.InitialLayout = layout;

        return *this;
    }
    
    VyImage::Builder& 
    VyImage::Builder::setTiling(VkImageTiling tiling)
    {
        m_Info.Tiling = tiling;

        return *this;
    }
    
    VyImage::Builder& 
    VyImage::Builder::setSamples(VkSampleCountFlagBits sampleCount)
    {
        m_Info.Samples = sampleCount;

        return *this;
    }
    
    VyImage::Builder& 
    VyImage::Builder::setSharing(VkSharingMode sharingMode)
    {
        m_Info.SharingMode = sharingMode;

        return *this;
    }
    
    VyImage::Builder& 
    VyImage::Builder::setUsage(VkImageUsageFlags flags)
    {
        m_Info.Usage = flags;

        return *this;
    }

    VyImage::Builder& 
    VyImage::Builder::addUsage(VkImageUsageFlags flags)
    {
        m_Info.Usage |= flags;

        return *this;
    }
    
    VyImage::Builder& 
    VyImage::Builder::setFlags(VkImageCreateFlags flags)
    {
        m_Info.Flags = flags;

        return *this;
    }
    
    VyImage::Builder& 
    VyImage::Builder::setMemoryUsage(VmaMemoryUsage usage)
    {
        m_Info.MemoryUsage = usage;

        return *this;
    }
    
    VyImage::Builder& 
    VyImage::Builder::setAllocFlags(VmaAllocationCreateFlags flags)
    {
        m_Info.AllocFlags = flags;

        return *this;
    }

    // VyImage::Builder& 
    // VyImage::Builder::initialData(
    //     void*         data, 
    //     U32           offset, 
    //     U32           width, 
    //     U32           height, 
    //     U32           dataSize,
    //     VkImageLayout finalLayout)
    // {
    //     m_UseInitialData = true;
    //     m_pData          = data;
    //     m_InitDataOffset = offset;
    //     m_InitDataSize   = dataSize;
    //     m_InitDataHeight = height;
    //     m_InitDataWidth  = width;
    //     m_FinalLayout    = finalLayout;
        
    //     m_Info.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;

    //     if (m_Info.mipLevels > 1)
    //     {
    //         m_Info.usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    //     }
    
    //     return *this;
    // }

    // VyImage::Builder& 
    // VyImage::Builder::setPreMadeImage(VkImage image)
    // {
    //     m_PreMadeImage = image;

    //     return *this;
    // }

	VyImage 
	VyImage::Builder::build() const 
	{
		VY_ASSERT(m_Info.Extent.width > 0 && m_Info.Extent.height > 0 && m_Info.Extent.depth > 0, 
			"Image extent dimensions must be greater than 0.");

		return VyImage{ m_Name, m_Info };
	}

	Unique<VyImage> 
	VyImage::Builder::buildPtr() const 
	{
		VY_ASSERT(m_Info.Extent.width > 0 && m_Info.Extent.height > 0 && m_Info.Extent.depth > 0,
			"Image extent dimensions must be greater than 0.");

		return MakeUnique<VyImage>( m_Name, m_Info );
	}

    // VyImage 
    // VyImage::Builder::build() const
    // {
    //     VyImage image;

    //     image.m_Info     = m_Info;
    //     image.m_CurrentLayout = m_Info.initialLayout;
    //     image.m_Image         = m_PreMadeImage;
    //     image.m_AllocInfo     = m_AllocInfo;

    //     if (image.m_Image == VK_NULL_HANDLE)
    //     {
    //         VyContext::device().createImage(image.m_Image, image.m_ImageMemory, m_Info, m_AllocInfo);
    //     }

    //     if (m_UseInitialData)
    //     {
    //         VyBuffer stagingBuffer{ VyBuffer::stagingBuffer("image_builder_init_data", m_InitDataSize) };

    //         stagingBuffer.singleWrite(m_pData, m_InitDataSize, m_InitDataOffset);

    //         auto cmdBuffer = VyContext::beginCommands();
    //         {
    //             // Transition image layout to be optimal for receiving data.
    //             image.transitionLayout(cmdBuffer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
                
    //             // Copy the contents of the image (from the buffer) to the vkImage.
    //             VyContext::device().copyBufferToImage(cmdBuffer, stagingBuffer.handle(), image.handle(), m_InitDataWidth, m_InitDataHeight, 1, 0);
                
    //             // Generate mipmaps with initial data.
    //             image.generateMipmaps(cmdBuffer, m_InitDataWidth, m_InitDataHeight, m_Info.mipLevels, m_Info.arrayLayers, m_FinalLayout);
    //         }
    //         VyContext::endCommands(cmdBuffer);
    //     }

    //     if (m_pName)
    //     {
    //         // Debugger::setDebugObjectName(reinterpret_cast<uint64_t>(image.handle()), VK_OBJECT_TYPE_IMAGE, m_pName);
    //     }

    //     return image;
    // }
}