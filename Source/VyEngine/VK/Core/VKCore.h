#pragma once

#include <VyEngine/VK/Core/Include/vkImpl.h>
#include <VyEngine/VK/Core/Include/vk_enum_str.h>

#include <VyEngine/VK/Core/VKInit.h>
#include <VyEngine/VK/Core/VKCmd.h>
#include <VyEngine/VK/Core/VKUtil.h>

#include <VyLib/VyLib.h>
#include <VyLib/STL/Path.h>

// Custom define for better code readability
#define VK_FLAGS_NONE 0

// Default fence timeout in nanoseconds
#define DEFAULT_FENCE_TIMEOUT 100000000000

#define VK_CHECK(result)                                                          \
{                                                                                 \
	VkResult vkResult = (result);                                                 \
    if (vkResult != VK_SUCCESS)                                                   \
    {                                                                             \
        VY_ERROR_TAG("Vulkan", "Error: {}", STR_VK_RESULT(vkResult));             \
        VY_THROW_RUNTIME_ERROR("");                                               \
    }                                                                             \
}

#define VK_CHECK_SUCCESS(result, error_msg)                                       \
{                                                                                 \
    VkResult vkResult = (result);                                                 \
    if (vkResult != VK_SUCCESS)                                                   \
    {                                                                             \
        std::string vkErrorMsg   = STR_VK_RESULT(vkResult);                       \
        std::string userErrorMsg = (error_msg);                                   \
        VY_THROW_RUNTIME_ERROR(userErrorMsg + " [VkResult: " + vkErrorMsg + "]"); \
    }                                                                             \
}

#define VK_CHECK_RETURN_FALSE(result)                                             \
{                                                                                 \
    VkResult vkResult = (result);                                                 \
    if (vkResult != VK_SUCCESS)                                                   \
    {                                                                             \
        std::string vkErrorMsg = STR_VK_RESULT(vkResult);                         \
        VY_ERROR_TAG("Vulkan", "Error: {}", vkErrorMsg);                          \
        return false;                                                             \
    }                                                                             \
}



#define VK_RENDERER_DEBUG

    
namespace Vy
{
    /** 
     *  When swapping out images for our frames, we can have multiple frames 'in flight', 
     *  meaning frames that act as additional memory buffers. 
     **/
    static constexpr int MAX_FRAMES_IN_FLIGHT = 2;


#ifdef VY_DEBUG_MODE
    constexpr bool kEnableValidationLayers = true;
#else
	constexpr bool kEnableValidationLayers = false;
#endif

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
        TOptional<U32> GraphicsFamily;

        // bool HasMultipleGraphicsQueues  = false;

        /**
         * @brief Index of the queue family that supports presentation to a surface.
         *
         * This queue family must be capable of presenting rendered images to a Vulkan surface.
         * It is determined using `vkGetPhysicalDeviceSurfaceSupportKHR`.
         */
        TOptional<U32> PresentFamily;

        /**
         * @brief Index of the queue family that supports computational tasks and parallel processing on the GPU.
         */
        TOptional<U32> ComputeFamily;

        // bool HasDedicatedComputeFamily  = false;


        /**
         * @brief Index of the queue family that supports transfer commands like vkCmdCopyBuffer and vkCmdCopyImage.
         */
        TOptional<U32> TransferFamily;

        // bool HasDedicatedTransferFamily = false;

        /**
         * @brief Checks if the required queue families have been found.
         *
         * @return `true` if all the queue families are valid.
         */
        bool hasValidQueueSupport() const 
        { 
            return GraphicsFamily.has_value()
                && PresentFamily .has_value()
                // && ComputeFamily .has_value()
                // && TransferFamily.has_value()
            ;
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
		VkPhysicalDeviceFeatures2        Core{};
		VkPhysicalDeviceVulkan11Features V11{};
		VkPhysicalDeviceVulkan12Features V12{};
		VkPhysicalDeviceVulkan13Features V13{};
        VkPhysicalDeviceVulkan14Features V14{};
	};

#pragma endregion Structures


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

        void setObjectName(VkBuffer object, const char* objectName);
        void setObjectName(VkImage object, const char* objectName);
        void setObjectName(VkImageView object, const char* objectName);
        void setObjectName(VkSampler object, const char* objectName);
        void setObjectName(VkRenderPass object, const char* objectName);
        void setObjectName(VkFramebuffer object, const char* objectName);
        void setObjectName(VkPipeline object, const char* objectName);
        void setObjectName(VkPipelineLayout object, const char* objectName);
        void setObjectName(VkDescriptorSet object, const char* objectName);
        void setObjectName(VkDescriptorPool object, const char* objectName);
        void setObjectName(VkDescriptorSetLayout object, const char* objectName);
        void setObjectName(VkSemaphore object, const char* objectName);
        void setObjectName(VkFence object, const char* objectName);

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
}