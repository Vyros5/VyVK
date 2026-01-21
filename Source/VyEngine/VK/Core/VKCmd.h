#pragma once

#include <VyEngine/VK/Core/Include/vkImpl.h>
#include <VyEngine/VK/Core/Include/vk_enum_str.h>

#include <VyLib/VyLib.h>

namespace Vy
{
#pragma region [ VKCmd ]

    namespace VKCmd
    {
        void copyBufferToImage(VkCommandBuffer cmdBuffer, VkBuffer buffer, VkImage image, VkExtent2D extent);
        void copyBufferToImage(VkCommandBuffer cmdBuffer, VkBuffer buffer, VkImage image, VkExtent3D extent, U32 layerCount);

        void dispatch(VkCommandBuffer cmdBuffer, VkExtent2D extent, VkExtent2D groupSize);
        void dispatch(VkCommandBuffer cmdBuffer, VkExtent3D extent, VkExtent3D groupSize);

        void pipelineBarrier(
            VkCommandBuffer    cmdBuffer, 
            VkImage            image, 
            VkImageLayout      oldLayout, 
            VkImageLayout      newLayout,
            VkImageAspectFlags aspectMask
        );

        void pipelineBarrier(
            VkCommandBuffer                       cmdBuffer, 
            VkPipelineStageFlags                  srcStage, 
            VkPipelineStageFlags                  dstStage, 
            const TVector<VkImageMemoryBarrier>&  barriers
        );

		void pipelineBarrier(
            VkCommandBuffer                       cmdBuffer, 
            VkPipelineStageFlags                  srcStage, 
            VkPipelineStageFlags                  dstStage,
			const TVector<VkBufferMemoryBarrier>& bufferBarriers, 
            const TVector<VkImageMemoryBarrier>&  imageBarriers
        );

        void transitionImageLayout(
            VkCommandBuffer cmdBuffer, 
            VkImage         image, 
            VkFormat        format, 
            VkImageLayout   oldLayout, 
            VkImageLayout   newLayout, 
            U32             miplevels   = 1, 
            U32             layoutCount = 1
        );

        void transitionImageLayout(
            VkCommandBuffer         cmdbuffer,
            VkImage                 image,
            VkImageLayout           oldImageLayout,
            VkImageLayout           newImageLayout,
            VkImageSubresourceRange subresourceRange,
            VkPipelineStageFlags    srcStageMask        = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
            VkPipelineStageFlags    dstStageMask        = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
            U32                     srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            U32                     dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED
        );

        void scissor (VkCommandBuffer cmdBuffer, VkExtent2D extent);
        void viewport(VkCommandBuffer cmdBuffer, VkExtent2D extent);


        // Extensions

        void beginDebugUtilsLabel(VkCommandBuffer cmdBuffer, const char* label, const Vec4& color = Vec4{ 1.0f });
        void endDebugUtilsLabel(VkCommandBuffer cmdBuffer);
    }

#pragma endregion
}