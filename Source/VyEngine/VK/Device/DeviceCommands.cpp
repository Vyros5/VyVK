#include <VyEngine/VK/Device/Device.h>

namespace Vy
{
// =========================================================================================================================
#pragma region [ Cmd ]
// =========================================================================================================================

	void VyDevice::imageMemoryBarrier(
		VkCommandBuffer         cmdBuffer,
		VkImage                 image,
		VkImageLayout           oldLayout,
		VkImageLayout           newLayout,
		VkAccessFlags           srcAcessMask,
		VkAccessFlags           dstAccessMask,
		VkPipelineStageFlags    srcStageMask,
		VkPipelineStageFlags    dstStageMask,
		VkImageSubresourceRange subresourceRange,
		U32                     srcQueueFamilyIndex,
		U32                     dstQueueFamilyIndex)
	{
        VkImageMemoryBarrier barrier{ VKInit::imageMemoryBarrier() };
		{
			barrier.image               = image;
			barrier.oldLayout           = oldLayout;
			barrier.newLayout           = newLayout;
			barrier.srcAccessMask       = srcAcessMask;
			barrier.dstAccessMask       = dstAccessMask;
			barrier.srcQueueFamilyIndex = srcQueueFamilyIndex;
			barrier.dstQueueFamilyIndex = dstQueueFamilyIndex;
			barrier.subresourceRange    = subresourceRange;
		}

        vkCmdPipelineBarrier(cmdBuffer,
            srcStageMask,
            dstStageMask,
            0x0,
            0, nullptr,
            0, nullptr,
            1, &barrier
        );
	}


	void VyDevice::imageMemoryBarrier(
		VkImage                 image,
		VkImageLayout           oldLayout,
		VkImageLayout           newLayout,
		VkAccessFlags           srcAcessMask,
		VkAccessFlags           dstAccessMask,
		VkPipelineStageFlags    srcStageMask,
		VkPipelineStageFlags    dstStageMask,
		VkImageSubresourceRange subresourceRange,
		U32                     srcQueueFamilyIndex,
		U32                     dstQueueFamilyIndex)
	{
		auto cmdBuffer = beginSingleTimeCommands();
		{
			imageMemoryBarrier(
				cmdBuffer,
				image,
				oldLayout,
				newLayout,
				srcAcessMask,
				dstAccessMask,
				srcStageMask,
				dstStageMask,
				subresourceRange,
				srcQueueFamilyIndex,
				dstQueueFamilyIndex
			);
		}
		endSingleTimeCommands(cmdBuffer);
	}


    // =====================================================================================================================


	void VyDevice::copyBuffer(
		VkCommandBuffer cmdBuffer, 
		VkBuffer        srcBuffer, 
		VkBuffer        dstBuffer, 
		VkDeviceSize    size)
	{
		recordCopyBuffer(cmdBuffer, srcBuffer, dstBuffer, size);
	}


	void VyDevice::copyBuffer(
		VkBuffer        srcBuffer, 
		VkBuffer        dstBuffer, 
		VkDeviceSize    size)
	{
		VkCommandBuffer cmdBuffer = beginSingleTimeCommands();
		{
			recordCopyBuffer(cmdBuffer, srcBuffer, dstBuffer, size);
		}
		endSingleTimeCommands(cmdBuffer);
	}


	void VyDevice::recordCopyBuffer(
		VkCommandBuffer cmdBuffer, 
		VkBuffer        srcBuffer, 
		VkBuffer        dstBuffer, 
		VkDeviceSize    size)
	{
		VkBufferCopy copyRegion{};
		{
			copyRegion.srcOffset = 0;
			copyRegion.dstOffset = 0;
			copyRegion.size      = size;
		}

		vkCmdCopyBuffer(cmdBuffer, 
			srcBuffer, 
			dstBuffer, 
			1, &copyRegion
		);
	}


    // =====================================================================================================================


	void VyDevice::copyBufferToImage(
		VkCommandBuffer cmdBuffer, 
		VkBuffer        buffer, 
		VkImage         image, 
		U32             width, 
		U32             height, 
		U32             layerCount /* 1 */, 
		U32             mipLevel   /* 1 */)
	{
		recordCopyBufferToImage(cmdBuffer, 
			buffer, image, 
			width,  height, 
			layerCount, 
			mipLevel
		);
	}


	void VyDevice::copyBufferToImage(
		VkBuffer        buffer, 
		VkImage         image, 
		U32             width, 
		U32             height, 
		U32             layerCount /* 1 */, 
		U32             mipLevel   /* 1 */)
	{
		VkCommandBuffer cmdBuffer = beginSingleTimeCommands();
		{
			recordCopyBufferToImage(cmdBuffer, 
				buffer, image, 
				width,  height, 
				layerCount, 
				mipLevel
			);
		}
		endSingleTimeCommands(cmdBuffer);
	}


	void VyDevice::recordCopyBufferToImage(
		VkCommandBuffer cmdBuffer, 
		VkBuffer        buffer, 
		VkImage         image, 
		U32             width, 
		U32             height, 
		U32             layerCount, 
		U32             mipLevel)
	{
		VkBufferImageCopy region{};
		{
			region.bufferOffset      = 0;
			region.bufferRowLength   = 0;
			region.bufferImageHeight = 0;

			region.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
			region.imageSubresource.mipLevel       = mipLevel - 1;
			region.imageSubresource.baseArrayLayer = 0;
			region.imageSubresource.layerCount     = layerCount;
			
			region.imageOffset = { 0, 0, 0 };
			region.imageExtent = { width, height, 1 };
		}

		vkCmdCopyBufferToImage(cmdBuffer, 
			buffer, 
			image, 
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
			1, &region
		);
	}


    // =====================================================================================================================


	void VyDevice::copyImageToBuffer(
		VkCommandBuffer cmdBuffer, 
		VkImage         image, 
		VkBuffer        buffer, 
		U32             width, 
		U32             height, 
		U32             layerCount /* 1 */, 
		U32             mipLevel   /* 1 */)
	{
		recordCopyImageToBuffer(cmdBuffer, 
			image, buffer, 
			width, height, 
			layerCount, 
			mipLevel
		);
	}


	void VyDevice::copyImageToBuffer(
		VkImage         image, 
		VkBuffer        buffer, 
		U32             width, 
		U32             height, 
		U32             layerCount /* 1 */, 
		U32             mipLevel   /* 1 */)
	{
		VkCommandBuffer cmdBuffer = beginSingleTimeCommands();
		{
			recordCopyImageToBuffer(cmdBuffer, 
				image, buffer, 
				width, height, 
				layerCount, 
				mipLevel
			);
		}
		endSingleTimeCommands(cmdBuffer);
	}


	void VyDevice::recordCopyImageToBuffer(
		VkCommandBuffer cmdBuffer, 
		VkImage         image, 
		VkBuffer        buffer, 
		U32             width, 
		U32             height, 
		U32             layerCount, 
		U32             mipLevel)
	{
		VkBufferImageCopy region{};
		{
			region.bufferOffset      = 0;
			region.bufferRowLength   = 0;
			region.bufferImageHeight = 0;
			
			region.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
			region.imageSubresource.mipLevel       = mipLevel - 1;
			region.imageSubresource.baseArrayLayer = 0;
			region.imageSubresource.layerCount     = layerCount;
			
			region.imageOffset = { 0, 0, 0 };
			region.imageExtent = { width, height, 1 };
		}

		vkCmdCopyImageToBuffer(cmdBuffer, 
			image, 
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, 
			buffer, 
			1, &region
		);
	}


    // =====================================================================================================================


	void VyDevice::copyImage(
		VkCommandBuffer cmdBuffer, 
		VkImage         srcImage, 
		VkImage         dstImage, 
		U32             width, 
		U32             height, 
		U32             srcBaseLayerIndex /* 0 */, 
		U32             dstBaseLayerIndex /* 0 */, 
		U32             layerCount        /* 1 */)
	{
		recordCopyImage(cmdBuffer, 
			srcImage, dstImage, 
			width, height, 
			srcBaseLayerIndex, dstBaseLayerIndex, 
			layerCount
		);
	}


	void VyDevice::copyImage(
		VkImage         srcImage, 
		VkImage         dstImage, 
		U32             width, 
		U32             height, 
		U32             srcBaseLayerIndex /* 0 */, 
		U32             dstBaseLayerIndex /* 0 */, 
		U32             layerCount        /* 1 */)
	{
		VkCommandBuffer cmdBuffer = beginSingleTimeCommands();
		{
			recordCopyImage(cmdBuffer, 
				srcImage, dstImage, 
				width, height, 
				srcBaseLayerIndex, dstBaseLayerIndex, 
				layerCount
			);
		}
		endSingleTimeCommands(cmdBuffer);
	}


	void VyDevice::recordCopyImage(
		VkCommandBuffer cmdBuffer, 
		VkImage         srcImage, 
		VkImage         dstImage, 
		U32             width, 
		U32             height, 
		U32             srcBaseLayerIndex, 
		U32             dstBaseLayerIndex, 
		U32             layerCount)
	{
		VkImageCopy region{};
		{
			region.srcSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
			region.srcSubresource.mipLevel       = 0;
			region.srcSubresource.baseArrayLayer = srcBaseLayerIndex;
			region.srcSubresource.layerCount     = layerCount;
			region.srcOffset                     = { 0, 0, 0 };
			
			region.dstSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
			region.dstSubresource.mipLevel       = 0;
			region.dstSubresource.baseArrayLayer = dstBaseLayerIndex;
			region.dstSubresource.layerCount     = layerCount;
			region.dstOffset                     = { 0, 0, 0 };

			region.extent = { width, height, 1 };
		}

		vkCmdCopyImage(cmdBuffer,
			srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1, &region
		);
	}


    // =====================================================================================================================


	void VyDevice::transitionImageLayout(
		VkCommandBuffer cmdBuffer, 
		VkImage         image, 
		VkFormat        format, 
		VkImageLayout   oldLayout, 
		VkImageLayout   newLayout, 
		U32             baseLayerIndex /* 0 */, 
		U32             layerCount     /* 1 */, 
		U32             mipLevel       /* 1 */)
	{
		recordTransitionImageLayout(cmdBuffer, image, format, oldLayout, newLayout, baseLayerIndex, layerCount, mipLevel);
	}


	void VyDevice::transitionImageLayout(
		VkImage         image, 
		VkFormat        format, 
		VkImageLayout   oldLayout, 
		VkImageLayout   newLayout, 
		U32             baseLayerIndex /* 0 */, 
		U32             layerCount     /* 1 */, 
		U32             mipLevel       /* 1 */)
	{
		VkCommandBuffer cmdBuffer = beginSingleTimeCommands();
		{
			recordTransitionImageLayout(cmdBuffer, image, format, oldLayout, newLayout, baseLayerIndex, layerCount, mipLevel);
		}
		endSingleTimeCommands(cmdBuffer);
	}


	/**
	 * EXAMPLE TRANSITIONS:
	 *
	 * Transition for Rendering:
	 * transitionImageLayout(Image, Format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	 *
	 * Transition for Saving / copy to staging buffer:
	 * transitionImageLayout(Image, Format, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
	 *
	 * Transition for Sampling in the Shader
	 * transitionImageLayout(Image, Format, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	 *
	 * Transition for general Destination
	 * transitionImageLayout(Image, Format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	 */
	void VyDevice::recordTransitionImageLayout(
		VkCommandBuffer cmdBuffer, 
		VkImage         image, 
		VkFormat        format, 
		VkImageLayout   oldLayout, 
		VkImageLayout   newLayout, 
		U32             baseLayerIndex, 
		U32             layerCount, 
		U32             mipLevel)
	{
		VkImageMemoryBarrier barrier{ VKInit::imageMemoryBarrier() };
		{
			barrier.image = image;

			barrier.oldLayout = oldLayout;
			barrier.newLayout = newLayout;

			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            
            // Use format to determine the aspect mask.
            barrier.subresourceRange.aspectMask     = VKUtil::aspectFlags(format);
            
            barrier.subresourceRange.baseMipLevel   = 0;
            barrier.subresourceRange.levelCount     = mipLevel;
            barrier.subresourceRange.baseArrayLayer = baseLayerIndex;
            barrier.subresourceRange.layerCount     = layerCount;
		}

		VkPipelineStageFlags srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		VkPipelineStageFlags dstStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;

		// Handle layout transitions.
		// -------------------------------------------------------------------------
		// Old: Undefined   
		// New: TransferDst 
		if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && 
			newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
		{
			// SrcAccess: 0
			// DstAccess: TransferWrite
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			// SrcStage: Top
			// DstStage: Transfer
			srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		// -------------------------------------------------------------------------
		// Old: TransferDst
		// New: ShaderRead
		else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && 
				 newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			// SrcAccess: TransferWrite
			// DstAccess: ShaderRead
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			// SrcStage: Transfer
			// DstStage: FragmentShader
			srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		// -------------------------------------------------------------------------
		// Old: Undefined
		// New: ColorAttachment
		else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && 
				 newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
		{
			// SrcAccess: 0
			// DstAccess: ColorAttachmentWrite
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

			// SrcStage: Top
			// DstStage: ColorAttachmentOutput
			srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			dstStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		}
		// -------------------------------------------------------------------------
		// Old: ColorAttachment
		// New: TransferSrc
		else if (oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && 
				 newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
		{
			// SrcAccess: ColorAttachmentWrite
			// DstAccess: TransferRead
			barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

			// SrcStage: ColorAttachmentOutput
			// DstStage: Transfer
			srcStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		// -------------------------------------------------------------------------
		// Old: TransferSrc
		// New: ShaderRead
		else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL && 
				 newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			// SrcAccess: TransferRead
			// DstAccess: ShaderRead
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			// SrcStage: Transfer
			// DstStage: FragmentShader
			srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		// -------------------------------------------------------------------------
		// Old: ShaderRead
		// New: ColorAttachment
		else if (oldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && 
				 newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
		{
			// SrcAccess: ShaderRead
			// DstAccess: ColorAttachmentWrite
			barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
			barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

			// SrcStage: FragmentShader
			// DstStage: ColorAttachmentOutput
			srcStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			dstStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		}
		// -------------------------------------------------------------------------
		// Old: TransferDst
		// New: TransferSrc
		else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && 
				 newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
		{
			// SrcAccess: TransferWrite
			// DstAccess: TransferRead
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

			// SrcStage: Transfer
			// DstStage: Transfer
			srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		// -------------------------------------------------------------------------
		// Old: ColorAttachment
		// New: ShaderRead
		else if (oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && 
				 newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			// SrcAccess: ColorAttachmentWrite
			// DstAccess: ShaderRead
			barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			// SrcStage: ColorAttachmentOutput
			// DstStage: FragmentShader
			srcStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		// -------------------------------------------------------------------------
		// Old: ColorAttachment
		// New: TransferDst
		else if (oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && 
				 newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
		{
			// SrcAccess: ColorAttachmentWrite
			// DstAccess: TransferRead
			barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			// SrcStage: ColorAttachmentOutput
			// DstStage: Transfer
			srcStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		// -------------------------------------------------------------------------
		// Old: TransferDst
		// New: ColorAttachment
		else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && 
				 newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
		{
			// SrcAccess: TransferWrite
			// DstAccess: ColorAttachmentWrite 
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

			// SrcStage: Transfer
			// DstStage: ColorAttachmentOutput 
			srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			dstStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		}
		// -------------------------------------------------------------------------
		// Old: TransferSrc
		// New: ColorAttachment
		else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL && 
				 newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
		{
			// SrcAccess: TransferRead
			// DstAccess: ColorAttachmentWrite 
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

			// SrcStage: Transfer
			// DstStage: ColorAttachmentOutput 
			srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			dstStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		}
		// -------------------------------------------------------------------------
		// Old: ColorAttachment
		// New: TransferDst
		else if (oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && 
				 newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
		{
			// SrcAccess: ColorAttachmentWrite
			// DstAccess: TransferWrite
			barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			// SrcStage: ColorAttachmentOutput 
			// DstStage: Transfer
			srcStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		// -------------------------------------------------------------------------
		else
		{
			VY_THROW_INVALID_ARGUMENT("Unsupported layout transition!");
		}

		vkCmdPipelineBarrier(cmdBuffer,
			srcStage, dstStage,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);
	}


    // =====================================================================================================================

#pragma endregion Cmd
}