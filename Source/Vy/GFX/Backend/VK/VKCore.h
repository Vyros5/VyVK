#pragma once

/**
 * @brief Core file include for all Vulkan related files.
 * 
 */

/**
 * Volk Include
 * Ensure only volk is used for vulkan function loading.
 * NEVER include <vulkan/vulkan.h> directly in any file, use this header instead.
 */

#define VK_NO_PROTOTYPES
#include <volk.h>

/**
 * VMA Include
 * Must be included AFTER Volk.
 */
#include <vma/vk_mem_alloc.h>


// #include <Vy/GFX/Config.h>
#include <VyLib/VyLib.h>
#include <VyLib/STL/Path.h>

#define VK_CHECK(f)                                                              \
{                                                                                \
	VkResult vkResult = (f);                                                     \
    if (vkResult != VK_SUCCESS)                                                  \
    {                                                                            \
        VY_ERROR_TAG("Vulkan", "Error: {}", Vy::VKUtil::resultString(vkResult)); \
        VY_THROW_RUNTIME_ERROR("");                                              \
    }                                                                            \
}


#define VK_RENDERER_DEBUG

    // ----------------------------------------------------------------------------------
    // GLSL / GLM Type       | Vulkan VkFormat Equivalent         | Size (Bytes) | Notes
    // ----------------------------------------------------------------------------------
    // float                 | VK_FORMAT_R32_SFLOAT               | 4            | 1 × 32-bit float
    // vec2 / glm::vec2      | VK_FORMAT_R32G32_SFLOAT            | 8            | 2 × 32-bit floats
    // vec3 / glm::vec3      | VK_FORMAT_R32G32B32_SFLOAT         | 12           | 3 × 32-bit floats (no padding in VkFormat)
    // vec4 / glm::vec4      | VK_FORMAT_R32G32B32A32_SFLOAT      | 16           | 4 × 32-bit floats
    // int                   | VK_FORMAT_R32_SINT                 | 4            | 1 × 32-bit signed integer
    // ivec2 / glm::ivec2    | VK_FORMAT_R32G32_SINT              | 8            | 2 × 32-bit signed integers
    // ivec3 / glm::ivec3    | VK_FORMAT_R32G32B32_SINT           | 12           | 3 × 32-bit signed integers
    // ivec4 / glm::ivec4    | VK_FORMAT_R32G32B32A32_SINT        | 16           | 4 × 32-bit signed integers
    // uint                  | VK_FORMAT_R32_UINT                 | 4            | 1 × 32-bit unsigned integer
    // uvec2 / glm::uvec2    | VK_FORMAT_R32G32_UINT              | 8            | 2 × 32-bit unsigned integers
    // uvec3 / glm::uvec3    | VK_FORMAT_R32G32B32_UINT           | 12           | 3 × 32-bit unsigned integers
    // uvec4 / glm::uvec4    | VK_FORMAT_R32G32B32A32_UINT        | 16           | 4 × 32-bit unsigned integers
    // ----------------------------------------------------------------------------------
    // Note:
    // - There is no VK_FORMAT for 3-element types with alignment padding like GLSL's std140 vec3.
    //   In vertex formats, VK_FORMAT_R32G32B32_* is valid and used, but memory alignment should
    //   still be handled carefully when packing structs in C++.
    // - Avoid using vec3 in UBOs unless you pad to vec4 alignment manually.
    // ----------------------------------------------------------------------------------

    
namespace Vy
{
    /** 
     *  When swapping out images for our frames, we can have multiple frames 'in flight', 
     *  meaning frames that act as additional memory buffers. In our case we allow for two additional
     *  frames in flight for added image buffering.
     **/
    static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

    
    // Descriptor Related
    using BindingIndex = U32;
	using SetIndex     = U32;


#pragma region [ Structures ]

    /**
     * @struct SwapchainSupportDetails
     * @brief Stores details about the swap chain support for a given Vulkan surface.
     *
     * This structure is used to query and store information about the swap chain capabilities
     * of a physical device for a specific surface. It contains details necessary for creating
     * an optimal swap chain configuration.
     */
    struct SwapchainSupportDetails 
    {
        /**
         * @brief Specifies the surface capabilities.
         *
         * This field contains details about the swap chain's constraints and capabilities,
         * including:
         * - The minimum and maximum number of images the swap chain can support.
         * - The current width and height of the surface.
         * - The supported transforms (e.g., rotation, mirroring).
         * - Supported image usage flags (e.g., rendering, storage, transfer).
         *
         * It is retrieved using `vkGetPhysicalDeviceSurfaceCapabilitiesKHR`.
         */
        VkSurfaceCapabilitiesKHR Capabilities{};

        /**
         * @brief A list of supported surface formats.
         *
         * Each format specifies a combination of:
         * - A color format (e.g., `VK_FORMAT_B8G8R8A8_UNORM`), which determines the color depth and arrangement.
         * - A color space (e.g., `VK_COLOR_SPACE_SRGB_NONLINEAR_KHR`), which defines how colors are interpreted.
         *
         * The application needs to select a format compatible with both the swap chain and the rendering pipeline.
         * This list is retrieved using `vkGetPhysicalDeviceSurfaceFormatsKHR`.
         */
        TVector<VkSurfaceFormatKHR> Formats{};

        /**
         * @brief A list of supported presentation modes.
         *
         * Presentation modes determine how images are presented to the screen. Common modes include:
         * - `VK_PRESENT_MODE_IMMEDIATE_KHR`    : Frames are presented immediately, possibly causing screen tearing.
         * - `VK_PRESENT_MODE_FIFO_KHR`         : Uses a queue (V-Sync), ensuring no tearing but with potential input latency.
         * - `VK_PRESENT_MODE_MAILBOX_KHR`      : A triple-buffering approach reducing latency while avoiding tearing.
         * - `VK_PRESENT_MODE_FIFO_RELAXED_KHR` : Similar to FIFO but allows late frames to be presented immediately.
         *
         * The application selects the best mode based on performance and latency requirements.
         * This list is retrieved using `vkGetPhysicalDeviceSurfacePresentModesKHR`.
         */
        TVector<VkPresentModeKHR> PresentModes{};
    };



    /**
     * @struct QueueFamilyIndices
     * @brief Stores indices of queue families needed for Vulkan operations.
     *
     * This structure helps in identifying queue families that support graphics and presentation.
     * Vulkan devices can have multiple queue families, and different operations (such as rendering
     * and presentation) may require separate queue families.
     */
    struct QueueFamilyIndices 
    {
        /**
         * @brief Index of the queue family that supports graphics operations.
         *
         * This queue family must support `VK_QUEUE_GRAPHICS_BIT`, meaning it can be used
         * for rendering commands.
         */
        Optional<U32> GraphicsFamily;

        /**
         * @brief Index of the queue family that supports presentation to a surface.
         *
         * This queue family must be capable of presenting rendered images to a Vulkan surface.
         * It is determined using `vkGetPhysicalDeviceSurfaceSupportKHR`.
         */
        Optional<U32> PresentFamily;


        Optional<U32> ComputeFamily;


        // Optional<U32> TransferFamily;

        /**
         * @brief Checks if the required queue families have been found.
         *
         * @return `true` if all the queue families are valid.
         */
        bool isComplete() 
        {
            return 
                GraphicsFamily.has_value() && 
                PresentFamily .has_value();// &&
                // ComputeFamily .has_value() && 
                // TransferFamily.has_value();
        }
    };


    // Used in swapchain.
    // https://docs.vulkan.org/refpages/latest/refpages/source/VK_KHR_present_id.html
    struct VyPresentIdState
    {
        bool Enabled = false;
        U64  Next    = 1;
    };


	struct VKFeatures
	{
		VkPhysicalDeviceFeatures2        core{};
		VkPhysicalDeviceVulkan11Features v11{};
		VkPhysicalDeviceVulkan12Features v12{};
		VkPhysicalDeviceVulkan13Features v13{};
        VkPhysicalDeviceVulkan14Features v14{};
	};

#pragma endregion Structures


    // Forward Declaration
    class VyTexture;


#pragma region [ VKUtil ]

    namespace VKUtil
    {
        String printExtensionsList(const TVector<VkExtensionProperties>& extensions, size_t numColumns);

        /// @brief Returns the result as a string
        StringView resultString(VkResult result);


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
            if (isDepthFormat(f))   a |= VK_IMAGE_ASPECT_DEPTH_BIT;
            if (isStencilFormat(f)) a |= VK_IMAGE_ASPECT_STENCIL_BIT;
            if (!a)                 a |= VK_IMAGE_ASPECT_COLOR_BIT;

            return a;
        }


        // ----------------------------- usage presets --------------------------------

        enum class ImgUse 
        {
            Undefined,
            TransferSrc,
            TransferDst,
            ColorAttachment,
            DepthStencilAttachment,
            DepthStencilReadOnly,
            ShaderRead,      // sampled/readonly image
            ShaderWrite,     // storage image writes (GENERAL)
            Present
        };

        struct ImgUseInfo 
        {
            VkPipelineStageFlags2 Stage  = 0;
            VkAccessFlags2        Access = 0;
            VkImageLayout         Layout = VK_IMAGE_LAYOUT_UNDEFINED;
        };

        // Defaults are conservative and work across graphics/compute
        inline ImgUseInfo info_for(ImgUse u) noexcept 
        {
            switch (u) 
            {
            case ImgUse::Undefined:
                return { 
                    VK_PIPELINE_STAGE_2_NONE, 
                    0, 
                    VK_IMAGE_LAYOUT_UNDEFINED 
                };
            case ImgUse::TransferSrc:
                return { 
                    VK_PIPELINE_STAGE_2_TRANSFER_BIT, 
                    VK_ACCESS_2_TRANSFER_READ_BIT, 
                    VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL 
                };
            case ImgUse::TransferDst:
                return { 
                    VK_PIPELINE_STAGE_2_TRANSFER_BIT, 
                    VK_ACCESS_2_TRANSFER_WRITE_BIT, 
                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL 
                };
            case ImgUse::ColorAttachment:
                return { 
                    VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                    VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT,
                    VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL // Vulkan 1.3+ generalized attachment layout 
                }; 
            case ImgUse::DepthStencilAttachment:
                return { 
                    VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
                    VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
                    VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL 
                };
            case ImgUse::DepthStencilReadOnly:
                return { 
                    VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
                    VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
                    VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL 
                };
            case ImgUse::ShaderRead:
                return { 
                    VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT | VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
                    VK_ACCESS_2_SHADER_SAMPLED_READ_BIT, // sampled/read-only
                    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL 
                };
            case ImgUse::ShaderWrite:
                return { 
                    VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT | VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
                    VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT, // storage writes
                    VK_IMAGE_LAYOUT_GENERAL 
                };
            case ImgUse::Present:
                // dst stage can be NONE for present; access must be 0
                return { 
                    VK_PIPELINE_STAGE_2_NONE, 
                    0, 
                    VK_IMAGE_LAYOUT_PRESENT_SRC_KHR 
                };
            }

            return {};
        }



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
        
        VkRenderingAttachmentInfo renderingAttachmentInfo(
            const VyTexture&   texture, 
            VkAttachmentLoadOp loadOp, 
            VkClearValue       clearValue = { 0.0f, 0.0f, 0.0f, 0.0f }
        );

        VkShaderModule createShaderModule(VkDevice device, const TVector<char>& code);
        VkShaderModule createShaderModule(VkDevice device, const Path& path);

        VkPipelineShaderStageCreateInfo createShaderStage(VkShaderStageFlagBits stage, VkShaderModule module, const char* entryPoint);

        SwapchainSupportDetails getSwapchainSupport(const VkPhysicalDevice& device, const VkSurfaceKHR& surface);

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


#pragma region [ VKCmd ]

    namespace VKCmd
    {
        void copyBufferToImage(VkCommandBuffer cmd, VkBuffer buffer, VkImage image, VkExtent2D extent);
        void copyBufferToImage(VkCommandBuffer cmd, VkBuffer buffer, VkImage image, VkExtent3D extent, U32 layerCount);

        void dispatch(VkCommandBuffer cmd, VkExtent2D extent, VkExtent2D groupSize);
        void dispatch(VkCommandBuffer cmd, VkExtent3D extent, VkExtent3D groupSize);

        void pipelineBarrier(
            VkCommandBuffer    cmd, 
            VkImage            image, 
            VkImageLayout      oldLayout, 
            VkImageLayout      newLayout,
            VkImageAspectFlags aspectMask
        );

        void pipelineBarrier(
            VkCommandBuffer                      cmdBuffer, 
            VkPipelineStageFlags                 srcStage, 
            VkPipelineStageFlags                 dstStage, 
            const TVector<VkImageMemoryBarrier>& barriers
        );

        void scissor(VkCommandBuffer cmdBuffer, VkExtent2D extent);

        void transitionImageLayout(
            VkCommandBuffer cmd, 
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

        void viewport(VkCommandBuffer cmdBuffer, VkExtent2D extent);


        // Extensions

        void beginDebugUtilsLabel(VkCommandBuffer cmd, const char* label, const Vec4& color = Vec4{ 1.0f });
        void endDebugUtilsLabel(VkCommandBuffer cmd);
    }

#pragma endregion


#pragma region [ VKDbg ]

    namespace VKDbg
    {
        /**
         * @brief Begin labeling the commands in command buffer.
         * @param cmdBuffer command buffer
         * @param labelName label name
         * @param color label color
         */
        void cmdBeginLabel(VkCommandBuffer cmdBuffer, const char* labelName, Vec4 color);

        /**
         * @brief Insert label for the next command in command buffer.
         * @param cmdBuffer command buffer
         * @param labelName label name
         * @param color label color
         */
        void cmdInsertLabel(VkCommandBuffer cmdBuffer, const char* labelName, Vec4 color);

        /**
         * @brief
         * @param cmdBuffer end labeling of the commands in command buffer. Must be called after cmdBeginLabel.
         */
        void cmdEndLabel(VkCommandBuffer cmdBuffer);

        /**
         * @brief Begin labeling the commands in the queue.
         * @param queue queue handle
         * @param labelName label name
         * @param color label color
         */
        void queueBeginLabel(VkQueue queue, const char* labelName, Vec4 color);

        /**
         * @brief Insert label for the next command in the queue.
         * @param queue queue handle
         * @param labelName label name
         * @param color label color
         */
        void queueInsertLabel(VkQueue queue, const char* labelName, Vec4 color);

        /**
         * @brief end labeling of the commands in the queue. Must be called after queueBeginLabel.
         */
        void queueEndLabel(VkQueue queue);

        /**
         * @brief Add name to the vulkan object via it's handle.
         * @param objectType type of the object
         * @param objectHandle handle of the object
         * @param objectName name for the object
         */
        void setObjectName(VkObjectType objectType, U64 objectHandle, const char* objectName);

        /**
         * @brief Add arbitrary data to the vulkan object via it's handle.
         * @param objectType type of the object
         * @param objectHandle handle of the object
         * @param tagId id/name for the tag
         * @param tag struct of the data to add
         * @param tagSize size of the struct
         */
        void setObjectTag(VkObjectType objectType, U64 objectHandle, U64 tagId, void* tag, size_t tagSize);
    }

#pragma endregion


#pragma region [ VKInit ]

    namespace VKInit 
    {
        /**
         * Returns a default initialized VkApplicationInfo structure
         * @returns default initialized VkApplicationInfo
         * */
        inline VkApplicationInfo applicationInfo() 
        {
            VkApplicationInfo ret{};
            ret.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;

            return ret;
        }

        /**
         * Returns a default initialized VkApplicationInfo structure
         * @returns default initialized VkApplicationInfo
         * */
        inline VkComputePipelineCreateInfo computePipelineCreateInfo() 
        {
            VkComputePipelineCreateInfo ret{};
            ret.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;

            return ret;
        }

        /**
         * Returns a default initialized VkInstanceCreateInfo structure
         * @returns default initialized VkInstanceCreateInfo
         * */
        inline VkInstanceCreateInfo instanceCreateInfo() 
        {
            VkInstanceCreateInfo ret{};
            ret.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

            return ret;
        }

        /**
         * Returns a default initialized VkDebugUtilsMessengerCreateInfoEXT structure
         * @returns default initialized VkDebugUtilsMessengerCreateInfoEXT
         * */
        inline VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfoEXT() 
        {
            VkDebugUtilsMessengerCreateInfoEXT ret{};
            ret.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;

            return ret;
        }

        /**
         * Returns a default initialized VkDeviceCreateInfo structure
         * @returns default initialized VkDeviceCreateInfo
         * */
        inline VkDeviceCreateInfo deviceCreateInfo() 
        {
            VkDeviceCreateInfo ret{};
            ret.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

            return ret;
        }

        /**
         * Returns a default initialized VkDeviceQueueCreateInfo structure
         * @returns default initialized VkDeviceQueueCreateInfo
         * */
        inline VkDeviceQueueCreateInfo deviceQueueCreateInfo() 
        {
            VkDeviceQueueCreateInfo ret{};
            ret.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;

            return ret;
        }

        /**
         * Returns a default initialized VkSwapchainCreateInfoKHR structure
         * @returns default initialized VkSwapchainCreateInfoKHR
         * */
        inline VkSwapchainCreateInfoKHR swapchainCreateInfoKHR() 
        {
            VkSwapchainCreateInfoKHR ret{};
            ret.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;

            return ret;
        }

        /**
         * Returns a default initialized VkImageViewCreateInfo structure
         * @returns default initialized VkImageViewCreateInfo
         * */
        inline VkImageViewCreateInfo imageViewCreateInfo() 
        {
            VkImageViewCreateInfo ret{};
            ret.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;

            return ret;
        }

        /**
         * Returns a default initialized VkImageCreateInfo structure
         * @returns default initialized VkImageCreateInfo
         * */
        inline VkImageCreateInfo imageCreateInfo() 
        {
            VkImageCreateInfo ret{};
            ret.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;

            return ret;
        }

        /**
         * Returns a default initialized VkSemaphoreCreateInfo structure
         * @returns default initialized VkSemaphoreCreateInfo
         * */
        inline VkSemaphoreCreateInfo semaphoreCreateInfo() 
        {
            VkSemaphoreCreateInfo ret{};
            ret.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

            return ret;
        }


        inline VkSemaphoreSubmitInfo semaphoreSubmitInfo() 
        {
            VkSemaphoreSubmitInfo ret{};
            ret.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;

            return ret;
        }

        /**
         * Returns a default initialized VkFenceCreateInfo structure
         * @returns default initialized VkFenceCreateInfo
         * */
        inline VkFenceCreateInfo fenceCreateInfo() 
        {
            VkFenceCreateInfo ret{};
            ret.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

            return ret;
        }

        /**
         * Returns a default initialized VkMemoryAllocateInfo structure
         * @returns default initialized VkMemoryAllocateInfo
         * */
        inline VkMemoryAllocateInfo memoryAllocateInfo() 
        {
            VkMemoryAllocateInfo ret{};
            ret.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;

            return ret;
        }

        /**
         * Returns a default initialized VkDescriptorPoolCreateInfo structure
         * @returns default initialized VkDescriptorPoolCreateInfo
         * */
        inline VkDescriptorPoolCreateInfo descriptorPoolCreateInfo() 
        {
            VkDescriptorPoolCreateInfo ret{};
            ret.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;

            return ret;
        }

        /**
         * Returns a default initialized VkCommandPoolCreateInfo structure
         * @returns default initialized VkCommandPoolCreateInfo
         * */
        inline VkCommandPoolCreateInfo commandPoolCreateInfo() 
        {
            VkCommandPoolCreateInfo ret{};
            ret.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;

            return ret;
        }

        /**
         * Returns a default initialized VkCommandBufferAllocateInfo structure
         * @returns default initialized VkCommandBufferAllocateInfo
         * */
        inline VkCommandBufferAllocateInfo commandBufferAllocateInfo() 
        {
            VkCommandBufferAllocateInfo ret{};
            ret.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;

            return ret;
        }

        /**
         * Returns a default initialized VkCommandBufferBeginInfo structure
         * @returns default initialized VkCommandBufferBeginInfo
         * */
        inline VkCommandBufferBeginInfo commandBufferBeginInfo() 
        {
            VkCommandBufferBeginInfo ret{};
            ret.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

            return ret;
        }


        inline VkCommandBufferSubmitInfo commandBufferSubmitInfo() 
        {
            VkCommandBufferSubmitInfo ret{};
            ret.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;

            return ret;
        }

        /**
         * Returns a default initialized VkRenderPassBeginInfo structure
         * @returns default initialized VkRenderPassBeginInfo
         * */
        inline VkRenderPassBeginInfo renderPassBeginInfo() 
        {
            VkRenderPassBeginInfo ret{};
            ret.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;

            return ret;
        }

        /**
         * Returns a default initialized VkRenderPassCreateInfo structure
         * @returns default initialized VkRenderPassCreateInfo
         * */
        inline VkRenderPassCreateInfo renderPassCreateInfo() 
        {
            VkRenderPassCreateInfo ret{};
            ret.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;

            return ret;
        }

        /**
         * Returns a default initialized VkFramebufferCreateInfo structure
         * @returns default initialized VkFramebufferCreateInfo
         * */
        inline VkFramebufferCreateInfo framebufferCreateInfo() 
        {
            VkFramebufferCreateInfo ret{};
            ret.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;

            return ret;
        }

        /**
         * Returns a default initialized VkDescriptorSetLayoutCreateInfo structure
         * @returns default initialized VkDescriptorSetLayoutCreateInfo
         * */
        inline VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo() 
        {
            VkDescriptorSetLayoutCreateInfo ret{};
            ret.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;

            return ret;
        }

        /**
         * Returns a default initialized VkSubmitInfo structure
         * @returns default initialized VkSubmitInfo
         * */
        inline VkSubmitInfo submitInfo() 
        {
            VkSubmitInfo ret{};
            ret.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

            return ret;
        }


        inline VkSubmitInfo2 submitInfo2() 
        {
            VkSubmitInfo2 ret{};
            ret.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;

            return ret;
        }


        inline VkDependencyInfo dependencyInfo()
        {
            VkDependencyInfo ret{};
            ret.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;

            return ret;
        }


        /**
         * Returns a default initialized VkShaderModuleCreateInfo structure
         * @returns default initialized VkShaderModuleCreateInfo
         * */
        inline VkShaderModuleCreateInfo shaderModuleCreateInfo() 
        {
            VkShaderModuleCreateInfo ret{};
            ret.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;

            return ret;
        }

        /**
         * Returns a default initialized VkPipelineLayoutCreateInfo structure
         * @returns default initialized VkPipelineLayoutCreateInfo
         * */
        inline VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo() 
        {
            VkPipelineLayoutCreateInfo ret{};
            ret.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

            return ret;
        }

        /**
         * Returns a default initialized VkSamplerCreateInfo structure
         * @returns default initialized VkSamplerCreateInfo
         * */
        inline VkSamplerCreateInfo samplerCreateInfo() 
        {
            VkSamplerCreateInfo ret{};
            ret.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;

            return ret;
        }


        /**
         * Returns a default initialized VkPresentInfoKHR structure
         * @returns default initialized VkPresentInfoKHR
         * */
        inline VkPresentInfoKHR presentInfoKHR() 
        {
            VkPresentInfoKHR ret{};
            ret.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

            return ret;
        }

        /**
         * Returns a default initialized VkBufferCreateInfo structure
         * @returns default initialized VkBufferCreateInfo
         * */
        inline VkBufferCreateInfo bufferCreateInfo() 
        {
            VkBufferCreateInfo ret{};
            ret.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;

            return ret;
        }

        /**
         * Returns a default initialized VkImageMemoryBarrier structure
         * @returns default initialized VkImageMemoryBarrier
         * */
        inline VkImageMemoryBarrier imageMemoryBarrier() 
        {
            VkImageMemoryBarrier ret{};
            ret.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;

            return ret;
        }

        inline VkImageMemoryBarrier2 imageMemoryBarrier2() 
        {
            VkImageMemoryBarrier2 ret{};
            ret.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;

            return ret;
        }

        inline VkImageBlit2 imageBlit2() 
        {
            VkImageBlit2 ret{};
            ret.sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2;

            return ret;
        }

        inline VkBlitImageInfo2 blitImageInfo2() 
        {
            VkBlitImageInfo2 ret{};
            ret.sType = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2;

            return ret;
        }


        /**
         * Returns a default initialized VkDescriptorSetAllocateInfo structure
         * @returns default initialized VkDescriptorSetAllocateInfo
         * */
        inline VkDescriptorSetAllocateInfo descriptorSetAllocateInfo() 
        {
            VkDescriptorSetAllocateInfo ret{};
            ret.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;

            return ret;
        }

        /**
         * Returns a default initialized VkWriteDescriptorSet structure
         * @returns default initialized VkWriteDescriptorSet
         * */
        inline VkWriteDescriptorSet writeDescriptorSet() 
        {
            VkWriteDescriptorSet ret{};
            ret.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;

            return ret;
        }

        /**
         * Returns a default initialized VkPhysicalDeviceVulkan13Features structure
         * @returns default initialized VkPhysicalDeviceVulkan13Features
         * */
        inline VkPhysicalDeviceVulkan13Features physicalDeviceVulkan13Features() 
        {
            VkPhysicalDeviceVulkan13Features ret{};
            ret.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;

            return ret;
        }

        /**
         * Returns a default initialized VkPhysicalDeviceFeatures2 structure
         * @returns default initialized VkPhysicalDeviceFeatures2
         * */
        inline VkPhysicalDeviceFeatures2 physicalDeviceFeatures2() 
        {
            VkPhysicalDeviceFeatures2 ret{};
            ret.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;

            return ret;
        }

        inline VkRenderingInfo renderingInfo()
        {
            VkRenderingInfo ret{};
            ret.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;

            return ret;
        }

        inline VkDescriptorSetLayoutBindingFlagsCreateInfo descriptorSetLayoutBindingFlagsCreateInfo()
        {
            VkDescriptorSetLayoutBindingFlagsCreateInfo ret{};
            ret.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;

            return ret;
        }

        inline VkBufferDeviceAddressInfo bufferDeviceAddressInfo()
        {
            VkBufferDeviceAddressInfo ret{};
            ret.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;

            return ret;
        }

        // MARK: Pipeline

        /**
         * Returns a default initialized VkPipelineShaderStageCreateInfo structure
         * @returns default initialized VkPipelineShaderStageCreateInfo
         * */
        inline VkPipelineShaderStageCreateInfo pipelineShaderStageCreateInfo() 
        {
            VkPipelineShaderStageCreateInfo ret{};
            ret.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;

            return ret;
        }

        /**
         * Returns a default initialized VkGraphicsPipelineCreateInfo structure
         * @returns default initialized VkGraphicsPipelineCreateInfo
         * */
        inline VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo() 
        {
            VkGraphicsPipelineCreateInfo ret{};
            ret.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;

            return ret;
        }

        inline VkPipelineRenderingCreateInfo pipelineRenderingCreateInfo()
        {
            VkPipelineRenderingCreateInfo ret{};
            ret.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;

            return ret;
        }

        inline VkPipelineViewportStateCreateInfo pipelineViewportStateCreateInfo() 
        {
            VkPipelineViewportStateCreateInfo ret{};
            ret.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;

            return ret;
        }

        inline VkPipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo()
        {
            VkPipelineInputAssemblyStateCreateInfo ret{};
            ret.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;

            return ret;
        }

        inline VkPipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo()
        {
            VkPipelineRasterizationStateCreateInfo ret{};
            ret.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;

            return ret;
        }

        inline VkPipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo()
        {
            VkPipelineMultisampleStateCreateInfo ret{};
            ret.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;

            return ret;
        }

        inline VkPipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo()
        {
            VkPipelineColorBlendStateCreateInfo ret{};
            ret.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;

            return ret;
        }

        inline VkPipelineDepthStencilStateCreateInfo pipelineDepthStencilStateCreateInfo()
        {
            VkPipelineDepthStencilStateCreateInfo ret{};
            ret.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;

            return ret;
        }

        inline VkPipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo()
        {
            VkPipelineDynamicStateCreateInfo ret{};
            ret.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;

            return ret;
        }



        /**
         * Returns a default initialized VkPipelineVertexInputStateCreateInfo structure
         * @returns default initialized VkPipelineVertexInputStateCreateInfo
         * */
        inline VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo() 
        {
            VkPipelineVertexInputStateCreateInfo ret{};
            ret.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

            return ret;
        }




        // ========================================================================================


        VY_NODISCARD inline VkImageSubresourceRange imageSubresourceRange(VkImageAspectFlags aspectMask) 
        {
            VkImageSubresourceRange subImage{};
            {
                subImage.aspectMask     = aspectMask;
                subImage.baseMipLevel   = 0;
                subImage.levelCount     = 1;
                subImage.baseArrayLayer = 0;
                subImage.layerCount     = 1;
            }

            return subImage;
        }

        VY_NODISCARD inline VkDescriptorSetLayoutBinding descriptorSetLayoutBinding(VkDescriptorType type, VkShaderStageFlags stageFlags, U32 binding) 
        {
            VkDescriptorSetLayoutBinding layoutBinding{};
            {
                layoutBinding.binding            = binding;
                layoutBinding.descriptorCount    = 1;
                layoutBinding.descriptorType     = type;
                layoutBinding.pImmutableSamplers = nullptr;
                layoutBinding.stageFlags         = stageFlags;
            }

            return layoutBinding;
        }

        inline VkPushConstantRange pushConstantRange(VkShaderStageFlags stageFlags, U32 size, U32 offset) 
        {
            VkPushConstantRange pushConstantRange{};
            {
                pushConstantRange.stageFlags = stageFlags;
                pushConstantRange.offset     = offset;
                pushConstantRange.size       = size;
            }

            return pushConstantRange;
        }
    }

#pragma endregion VKInit
}