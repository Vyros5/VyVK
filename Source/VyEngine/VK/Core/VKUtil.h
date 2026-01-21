#pragma once

#include <VyEngine/VK/Core/Include/vkImpl.h>
#include <VyEngine/VK/Core/Include/vk_enum_str.h>

#include <VyLib/VyLib.h>
#include <VyLib/STL/Path.h>

namespace Vy
{
#pragma region [ VKUtil ]

    namespace VKUtil
    {
        TString printExtensionsList(const TVector<VkExtensionProperties>& extensions, size_t numColumns);

        /// @brief Returns the result as a string
        TStringView resultString(VkResult result);


        I32 bytesPerPixel(VkFormat format);

        /// @brief Returns true if the format is a depth or depth-stencil format otherwise false
        // bool isDepthFormat(VkFormat format);

        inline size_t alignSize(size_t size, size_t alignment) 
        {
            return (size + alignment - 1) & ~(alignment - 1);
        }

        inline bool isDepthFormat(VkFormat format) 
        {
            TVector<VkFormat> depthFormats = {
                VK_FORMAT_D16_UNORM,
                VK_FORMAT_X8_D24_UNORM_PACK32,
                VK_FORMAT_D32_SFLOAT,
                VK_FORMAT_D16_UNORM_S8_UINT,
                VK_FORMAT_D24_UNORM_S8_UINT,
                VK_FORMAT_D32_SFLOAT_S8_UINT,
            };

            return std::ranges::find(depthFormats, format) != std::end(depthFormats);
        }

        inline bool isStencilFormat(VkFormat format) 
        {
            TVector<VkFormat> stencilFormats = {
                VK_FORMAT_S8_UINT,
                VK_FORMAT_D16_UNORM_S8_UINT,
                VK_FORMAT_D24_UNORM_S8_UINT,
                VK_FORMAT_D32_SFLOAT_S8_UINT,
            };

            return std::ranges::find(stencilFormats, format) != std::end(stencilFormats);
        }

        inline bool isDepthStencil(VkFormat format) 
        {
            return (isDepthFormat(format) || isStencilFormat(format));
        }

        // https://github.com/mkacere/HolyVulkanEngine/blob/master/HolyVulkanEngine/include/hvk/gfx/hvk_barriers.hpp
        inline VkImageAspectFlags aspectFromFormat(VkFormat f) 
        {
            VkImageAspectFlags a = 0;
            if (isDepthFormat  (f)) a |= VK_IMAGE_ASPECT_DEPTH_BIT;
            if (isStencilFormat(f)) a |= VK_IMAGE_ASPECT_STENCIL_BIT;
            if (!a)                 a |= VK_IMAGE_ASPECT_COLOR_BIT;

            return a;
        }

        VkAccessFlags        getAccessMask(VkImageLayout layout);
        VkPipelineStageFlags getStageMask(VkImageLayout layout);

        VkAccessFlags srcAccessMask(VkImageLayout layout);
        VkAccessFlags dstAccessMask(VkImageLayout layout);

        VkPipelineStageFlags srcStage(VkAccessFlags access);
        VkPipelineStageFlags dstStage(VkAccessFlags access);

        VkImageAspectFlags aspectFlags(VkFormat format);

        VkRenderingAttachmentInfo renderingAttachmentInfo(
            VkImageView        imageView, 
            VkImageLayout      layout, 
            VkAttachmentLoadOp loadOp, 
            VkClearValue       clearValue
        );
        
        // VkRenderingAttachmentInfo renderingAttachmentInfo(
        //     const VyTexture&   texture, 
        //     VkAttachmentLoadOp loadOp, 
        //     VkClearValue       clearValue = { 0.0f, 0.0f, 0.0f, 0.0f }
        // );

        VkShaderModule createShaderModule(VkDevice device, const TVector<char>& code);
        VkShaderModule createShaderModule(VkDevice device, const TPath& path);

        VkPipelineShaderStageCreateInfo createShaderStage(VkShaderStageFlagBits stage, VkShaderModule module, const char* entryPoint);

        bool hasGraphicsQueue(const VkQueueFamilyProperties& queueFamily);
        bool hasComputeQueue (const VkQueueFamilyProperties& queueFamily);
        
        bool hasPresentQueue(
            const VkPhysicalDevice&        device, 
            const U32                      queueFamilyIndex, 
            const VkSurfaceKHR&            surface, 
            const VkQueueFamilyProperties& queueFamilyProperties
        );
    }

#pragma endregion VKUtil
}