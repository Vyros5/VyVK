#include <VyEngine/VK/Core/VKCmd.h>

#include <VyEngine/VK/Core/Include/vkImpl.h>
#include <VyEngine/VK/Core/VKUtil.h>
#include <VyEngine/VK/Core/VKInit.h>

namespace Vy
{
    void VKCmd::copyBufferToImage(VkCommandBuffer cmdBuffer, VkBuffer buffer, VkImage image, VkExtent2D extent)
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
            region.imageExtent = { extent.width, extent.height, 1 };
        }

		vkCmdCopyBufferToImage(cmdBuffer,
			buffer, image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1, &region
        );
    }


    void VKCmd::copyBufferToImage(VkCommandBuffer cmdBuffer, VkBuffer buffer, VkImage image, VkExtent3D extent, U32 layerCount)
    {
		VkBufferImageCopy region{};
        {
            region.bufferOffset      = 0;
            region.bufferRowLength   = 0;
            region.bufferImageHeight = 0;
            
            region.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
            region.imageSubresource.mipLevel       = 0;
            region.imageSubresource.baseArrayLayer = 0;
            region.imageSubresource.layerCount     = layerCount;
            
            region.imageOffset = { 0, 0, 0 };
            region.imageExtent = extent;
        }

		vkCmdCopyBufferToImage(cmdBuffer,
			buffer, image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1, &region
        );
    }


    void VKCmd::dispatch(VkCommandBuffer cmdBuffer, VkExtent2D extent, VkExtent2D groupSize)
    {
		U32 groupCountX = (extent.width  + groupSize.width  - 1) / groupSize.width;
		U32 groupCountY = (extent.height + groupSize.height - 1) / groupSize.height;

		vkCmdDispatch(cmdBuffer, groupCountX, groupCountY, 1);
    }


    void VKCmd::dispatch(VkCommandBuffer cmdBuffer, VkExtent3D extent, VkExtent3D groupSize)
    {
		U32 groupCountX = (extent.width  + groupSize.width  - 1) / groupSize.width;
		U32 groupCountY = (extent.height + groupSize.height - 1) / groupSize.height;
		U32 groupCountZ = (extent.depth  + groupSize.depth  - 1) / groupSize.depth;

		vkCmdDispatch(cmdBuffer, groupCountX, groupCountY, groupCountZ);
    }


    void VKCmd::pipelineBarrier(
		VkCommandBuffer    cmdBuffer, 
		VkImage            image, 
		VkImageLayout      oldLayout, 
		VkImageLayout      newLayout, 
		VkImageAspectFlags aspectMask)
    {
		VkImageMemoryBarrier barrier{ VKInit::imageMemoryBarrier() };
        {
            barrier.oldLayout           = oldLayout;
            barrier.newLayout           = newLayout;
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.image               = image;
            
            barrier.subresourceRange.aspectMask     = aspectMask;
            barrier.subresourceRange.baseMipLevel   = 0;
            barrier.subresourceRange.levelCount     = 1;
            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.layerCount     = 1;
            
            barrier.srcAccessMask = VKUtil::srcAccessMask(oldLayout);
            barrier.dstAccessMask = VKUtil::dstAccessMask(newLayout);
        }

		vkCmdPipelineBarrier(cmdBuffer,
			VKUtil::srcStage(barrier.srcAccessMask), 
			VKUtil::dstStage(barrier.dstAccessMask), 
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);
    }


    void VKCmd::pipelineBarrier(
		VkCommandBuffer                      cmdBuffer, 
		VkPipelineStageFlags                 srcStage, 
		VkPipelineStageFlags                 dstStage, 
		const TVector<VkImageMemoryBarrier>& barriers)
    {
		if (barriers.empty())
			return;

		vkCmdPipelineBarrier(cmdBuffer,
			srcStage, dstStage, 0,
			0, nullptr,
			0, nullptr,
			static_cast<U32>(barriers.size()), barriers.data()
		);
    }

	void VKCmd::pipelineBarrier(
		VkCommandBuffer                       cmdBuffer, 
		VkPipelineStageFlags                  srcStage, 
		VkPipelineStageFlags                  dstStage,
		const TVector<VkBufferMemoryBarrier>& bufferBarriers, 
		const TVector<VkImageMemoryBarrier>&  imageBarriers)
	{
		if (bufferBarriers.empty() && imageBarriers.empty())
			return;

		vkCmdPipelineBarrier(cmdBuffer,
			srcStage, dstStage, 0,
			0, nullptr,
			static_cast<U32>(bufferBarriers.size()), bufferBarriers.data(),
			static_cast<U32>(imageBarriers.size()),  imageBarriers.data()
		);
	}

	

    void VKCmd::transitionImageLayout(
		VkCommandBuffer cmdBuffer, 
		VkImage         image, 
		VkFormat        format, 
		VkImageLayout   oldLayout, 
		VkImageLayout   newLayout, 
		U32             miplevels, 
		U32             layoutCount)
    {
		if (oldLayout == newLayout)
			return;

		VkImageMemoryBarrier barrier{ VKInit::imageMemoryBarrier() };
        {
            barrier.image               = image;

            barrier.oldLayout           = oldLayout;
            barrier.newLayout           = newLayout;

            barrier.subresourceRange.aspectMask     = VKUtil::aspectFlags(format);
            barrier.subresourceRange.baseMipLevel   = 0;
            barrier.subresourceRange.levelCount     = miplevels;
            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.layerCount     = layoutCount;

            barrier.srcAccessMask = VKUtil::srcAccessMask(barrier.oldLayout);
            barrier.dstAccessMask = VKUtil::dstAccessMask(barrier.newLayout);
        }

		VkPipelineStageFlags srcStage = VKUtil::srcStage(barrier.srcAccessMask);
		VkPipelineStageFlags dstStage = VKUtil::dstStage(barrier.dstAccessMask);

		vkCmdPipelineBarrier(cmdBuffer,
			srcStage, dstStage, 0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);
    }


	void VKCmd::transitionImageLayout(
		VkCommandBuffer         cmdbuffer,
		VkImage                 image,
		VkImageLayout           oldImageLayout,
		VkImageLayout           newImageLayout,
		VkImageSubresourceRange subresourceRange,
		VkPipelineStageFlags    srcStageMask,
		VkPipelineStageFlags    dstStageMask,
		U32                     srcQueueFamilyIndex,
		U32                     dstQueueFamilyIndex)
	{
		VkImageMemoryBarrier barrier{ VKInit::imageMemoryBarrier() };
        {
            barrier.oldLayout           = oldImageLayout;
            barrier.newLayout           = newImageLayout;
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.image               = image;

            barrier.subresourceRange    = subresourceRange;

			barrier.srcQueueFamilyIndex = srcQueueFamilyIndex;
			barrier.dstQueueFamilyIndex = dstQueueFamilyIndex;
		}

        // Source layouts (old)
        // Source access mask controls actions that have to be finished on the old layout
        // before it will be transitioned to the new layout
        switch (oldImageLayout) 
		{
            case VK_IMAGE_LAYOUT_UNDEFINED:
                // Image layout is undefined (or does not matter)
                // Only valid as initial layout
                // No flags required, listed only for completeness
                barrier.srcAccessMask = 0;
                break;

            case VK_IMAGE_LAYOUT_PREINITIALIZED:
                // Image is preinitialized
                // Only valid as initial layout for linear images, preserves memory contents
                // Make sure host writes have been finished
                barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
                break;

            case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
                // Image is a color attachment
                // Make sure any writes to the color buffer have been finished
                barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                break;

            case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
                // Image is a depth/stencil attachment
                // Make sure any writes to the depth/stencil buffer have been finished
                barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                break;

            case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
                // Image is a transfer source
                // Make sure any reads from the image have been finished
                barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                break;

            case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
                // Image is a transfer destination
                // Make sure any writes to the image have been finished
                barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                break;

            case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
                // Image is read by a shader
                // Make sure any shader reads from the image have been finished
                barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
                break;
            default:
                // Other source layouts aren't handled (yet)
                break;
        }

        // Target layouts (new)
        // Destination access mask controls the dependency for the new image layout
        switch (newImageLayout) 
		{
            case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
                // Image will be used as a transfer destination
                // Make sure any writes to the image have been finished
                barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                break;

            case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
                // Image will be used as a transfer source
                // Make sure any reads from the image have been finished
                barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                break;

            case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
                // Image will be used as a color attachment
                // Make sure any writes to the color buffer have been finished
                barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                break;

            case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
                // Image layout will be used as a depth/stencil attachment
                // Make sure any writes to depth/stencil buffer have been finished
                barrier.dstAccessMask = barrier.dstAccessMask | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                break;

            case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
                // Image will be read in a shader (sampler, input attachment)
                // Make sure any writes to the image have been finished
                if (barrier.srcAccessMask == 0) 
				{
                    barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
                }
                barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
                break;
            default:
                // Other source layouts aren't handled (yet)
                break;
        }

        // Put barrier inside setup command buffer
        vkCmdPipelineBarrier(cmdbuffer,
            srcStageMask, dstStageMask, 0,
            0, nullptr,
            0, nullptr,
            1, &barrier
        );
	}


    void VKCmd::scissor(VkCommandBuffer cmdBuffer, VkExtent2D extent)
    {
		VkRect2D scissor{};
        {
            scissor.offset = { 0, 0 };
            scissor.extent = extent;
        }

        vkCmdSetScissor(cmdBuffer, 0, 1, &scissor);
    }

    void VKCmd::viewport(VkCommandBuffer cmdBuffer, VkExtent2D extent)
    {
		VkViewport viewport{};
        {
            viewport.x        = 0.0f;
            viewport.y        = 0.0f;
            viewport.width    = static_cast<float>(extent.width);
            viewport.height   = static_cast<float>(extent.height);
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;
        }

		vkCmdSetViewport(cmdBuffer, 0, 1, &viewport);
    }


    void VKCmd::beginDebugUtilsLabel(VkCommandBuffer cmdBuffer, const char* label, const Vec4& color)
    {
		// if constexpr (VY_ENABLE_VALIDATION)
		// {
			VkDebugUtilsLabelEXT labelInfo{};
            {
                labelInfo.sType      = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
                labelInfo.pLabelName = label;
                labelInfo.color[0]   = color.r;
                labelInfo.color[1]   = color.g;
                labelInfo.color[2]   = color.b;
                labelInfo.color[3]   = color.a;
            }

            vkCmdBeginDebugUtilsLabelEXT(cmdBuffer, &labelInfo);
		// }
    }


    void VKCmd::endDebugUtilsLabel(VkCommandBuffer cmdBuffer)
    {
		// if constexpr (VY_ENABLE_VALIDATION)
		// {
			vkCmdEndDebugUtilsLabelEXT(cmdBuffer);
		// }
    }
}