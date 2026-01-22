#pragma once

#include <VyEngine/VK/Core/VKCore.h>

#include <VyEngine/Core/Window/Window.h>
// https://github.com/radueduard/Coral-Engine/blob/master/src/core/device.cpp

namespace Vy
{
    /** 
     * @brief Represents a Vulkan device and its associated resources.
     * 
     * This class encapsulates the Vulkan instance, physical device, logical device,
     * memory allocator, command pools, and queues required for rendering operations.
     */
    class VyDevice 
    {
		friend class VyContext;

	public:
		static constexpr U32  kAPIVersion       = VK_API_VERSION_1_3;

		static constexpr auto kValidationLayers = std::array{ "VK_LAYER_KHRONOS_validation" };
		
		static constexpr auto kDeviceExtensions = std::array{ 
			VK_KHR_SWAPCHAIN_EXTENSION_NAME, 
			VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
			VK_KHR_MULTIVIEW_EXTENSION_NAME 
		};
		
	private:

		VkInstance       m_Instance           { VK_NULL_HANDLE };
		VkPhysicalDevice m_PhysicalDevice     { VK_NULL_HANDLE };
		VkDevice         m_Device             { VK_NULL_HANDLE };
		VmaAllocator     m_Allocator          { VK_NULL_HANDLE };
		VkSurfaceKHR     m_Surface            { VK_NULL_HANDLE };

        VkCommandPool    m_GraphicsCommandPool{ VK_NULL_HANDLE };
		// VkCommandPool    m_PresentCommandPool { VK_NULL_HANDLE };
		// VkCommandPool    m_TransferCommandPool{ VK_NULL_HANDLE };
		// VkCommandPool    m_ComputeCommandPool { VK_NULL_HANDLE };
		// VkCommandPool    m_ImmCommandPool{ VK_NULL_HANDLE };

		VkQueue		     m_GraphicsQueue      { VK_NULL_HANDLE };
		VkQueue		     m_PresentQueue       { VK_NULL_HANDLE };
		VkQueue		     m_TransferQueue      { VK_NULL_HANDLE };
		VkQueue		     m_ComputeQueue       { VK_NULL_HANDLE };

		QueueFamilyIndices m_QueueFamilyIndices;

        VkDebugUtilsMessengerEXT   m_DebugMessenger{ VK_NULL_HANDLE };
		VkPhysicalDeviceProperties m_Properties{};
		VKFeatures                 m_Features  {};
		VkPhysicalDeviceMemoryProperties m_MemProperties{};
		VkSampleCountFlagBits      m_MsaaSamples{ VK_SAMPLE_COUNT_1_BIT };

		bool m_PresentIdSupported{ false };

	public:

 		VyDevice(const VyDevice&) = delete;
        VyDevice(VyDevice&&)      = delete;

        ~VyDevice();

        VyDevice& operator=(const VyDevice&) = delete;
        VyDevice& operator=(VyDevice&&)      = delete;

	private:

		/** 
		 * @brief Initializes the Vulkan device and related resources.
		 * 
		 * @note Called from 'VyContext::initialize(VyWindow& window)'.
		 * 
		 * @param window The VyWindow instance used to create the Vulkan surface.
		 */
		void initialize(VyWindow& window);

// =========================================================================================================================
#pragma region [ Accessors ]
// =========================================================================================================================

	public:

        operator     VkDevice()                                   { return m_Device; }
		VY_NODISCARD VkDevice         handle()              const { return m_Device; }
		
        VY_NODISCARD VkInstance       instance()            const { return m_Instance; }
		VY_NODISCARD VkPhysicalDevice physicalDevice()      const { return m_PhysicalDevice; }
		VY_NODISCARD VmaAllocator     allocator()           const { return m_Allocator; }
		VY_NODISCARD VkSurfaceKHR     surface()             const { return m_Surface; }
        
        VY_NODISCARD VkCommandPool    graphicsCommandPool() const { return m_GraphicsCommandPool; }
		// VY_NODISCARD VkCommandPool    presentCommandPool () const { return m_PresentCommandPool; }
		// VY_NODISCARD VkCommandPool    transferCommandPool() const { return m_TransferCommandPool; }
		// VY_NODISCARD VkCommandPool    computeCommandPool () const { return m_GraphicsCommandPool; }
		
        VY_NODISCARD VkQueue          graphicsQueue()       const { return m_GraphicsQueue; }
        VY_NODISCARD VkQueue          presentQueue ()       const { return m_PresentQueue;  }
		VY_NODISCARD VkQueue          transferQueue()       const { return m_TransferQueue; }
		VY_NODISCARD VkQueue          computeQueue ()       const { return m_ComputeQueue;  }
		
        VY_NODISCARD U32              graphicsQueueIndex()  const { return m_QueueFamilyIndices.GraphicsFamily.value(); }
		VY_NODISCARD U32              presentQueueIndex ()  const { return m_QueueFamilyIndices.PresentFamily .value(); }
		VY_NODISCARD U32              transferQueueIndex()  const { return m_QueueFamilyIndices.TransferFamily.value(); }
		VY_NODISCARD U32              computeQueueIndex ()  const { return m_QueueFamilyIndices.ComputeFamily .value(); }

		VY_NODISCARD const VkPhysicalDeviceProperties&       properties()     const { return m_Properties;  }
		VY_NODISCARD const VkPhysicalDeviceMemoryProperties& memProperties()  const { return m_MemProperties;  }
		
        VY_NODISCARD const VkPhysicalDeviceLimits&     limits()               const { return m_Properties.limits;  }
		VY_NODISCARD const VKFeatures&                 features()             const { return m_Features;    }
		VY_NODISCARD       VkSampleCountFlagBits       supportedSampleCount()       { return m_MsaaSamples; }
		VY_NODISCARD       bool                        supportsPresentId()    const { return m_PresentIdSupported; }

#pragma endregion Accessors


// =========================================================================================================================
#pragma region [ Extra ]
// =========================================================================================================================

	public:

        U32 findMemoryType(U32 typeFilter, VkMemoryPropertyFlags properties);

        /**
         * @brief Determines the appropriate aspect flags for a given image format.
         * 
         * @param format The image format to evaluate.
		 * 
         * @return The corresponding VkImageAspectFlags. 
         */
        VkImageAspectFlags findAspectFlags(VkFormat format) const;


		/**
		 * @brief Finds a supported format for an image.
		 *
		 * @param candidates The list of candidate formats.
		 * @param tiling     The tiling mode.
		 * @param features   The required features.
		 * 
		 * @return The supported format.
		 */
        VkFormat findSupportedFormat(const TVector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) const;
        VkFormat findDepthFormat() const;
		bool     getSupportedDepthsFormat(VkFormat* pDepthFormat);
        VkSampleCountFlagBits getMaxUsableSampleCount();

#pragma endregion Extra


// =========================================================================================================================
#pragma region [ Queue Families & Swapchain ]
// =========================================================================================================================

	public:

        /**
         * @brief Finds the queue families for a physical device.
         *
         * This function finds the graphics and present queue families for a physical device.
         *
         * @param device The physical device to find the queue families for.
		 * 
         * @return The queue family indices.
         */
		QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) const;
		QueueFamilyIndices findQueueFamilies() const;

		U32 findFirstValidQueueFamily(VkPhysicalDevice device, VkQueueFlags flags) const;

        /**
         * @brief Queries the swap chain support details for a physical device.
         *
         * This function queries the swap chain support details for a physical device, including the surface capabilities,
         * formats, and present modes.
         *
         * @param device The physical device to query the swap chain support details for.
         * @return The swap chain support details.
         */
		SwapchainSupportDetails querySwapchainSupport(VkPhysicalDevice device) const;
		SwapchainSupportDetails querySwapchainSupport() const;


#pragma endregion Queue Families & Swapchain


// =========================================================================================================================
#pragma region [ Extensions & Features ]
// =========================================================================================================================

	private:

        TVector<VkExtensionProperties> queryDeviceExtensionProperties(VkPhysicalDevice device);
		
        TVector<VkExtensionProperties> queryInstanceExtensionProperties();

        /**
         * @brief Gets the required extensions for the Vulkan instance.
         *
         * This function gets the required extensions for the Vulkan instance, including the GLFW extensions
         * and the debug utils extension if validation layers are enabled.
         *
         * @return A vector of required extensions.
         */
		TVector<CString> queryRequiredInstanceExtensions() const;

        /**
         * @brief Checks if the validation layers are supported.
         *
         * This function checks if all the required validation layers are supported by the Vulkan instance.
         *
         * @return true if all layers are supported, false otherwise.
         */
        bool checkValidationLayerSupport();

        /**
         * @brief Checks if the required GLFW extensions are supported.
         *
         * This function checks if all the required GLFW extensions are supported by the Vulkan instance.
         */
		void checkGflwRequiredInstanceExtensions();

        /**
         * @brief Checks if the required device extensions are supported.
         *
         * This function checks if all the required device extensions are supported by the physical device.
		 * If some extensions are optional (for example, NVIDIA-specific extensions), they are ignored and
		 * removed form deviceExtensions vector.
         *
         * @param device The physical device to check.
         * @return true if all extensions are supported, false otherwise.
         */
		bool checkDeviceExtensionSupport(VkPhysicalDevice device);

		/**
		 * @brief Checks if the required device features are supported.
		 *
		 * This function checks if all the required device features are supported by the physical device.
		 *
		 * @param device The physical device to check.
		 * 
		 * @return true if all features are supported, false otherwise.
		 */
		bool checkDeviceFeatureSupport(VkPhysicalDevice device);

#pragma endregion Extensions & Features


// =========================================================================================================================
#pragma region [ Debug ]
// =========================================================================================================================

	private:

        void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);

#pragma endregion Debug


// =========================================================================================================================
#pragma region [ CmdBuffer ]
// =========================================================================================================================

	public:

        /**
         * @brief Allocate and immediately begin recording a command buffer intended for one-time submission.
         * 
         * Useful for operations that need to be executed once and then discarded, such as:
         * 
         *      - Copying data: Transferring data from a staging buffer to a device-local buffer or image.
         * 
         *      - Image transitions: Changing the layout of an image for different uses.
         * 
         *      - Generating mipmaps: Performing image processing operations like mipmap generation.
         * 
         * @return The VkCommandBuffer handle, allowing the caller to record commands into it. 
         */
		VkCommandBuffer beginSingleTimeCommands() const;

        /**
         * @brief Finalize and submit a command buffer that contains a series of commands intended to be executed only once.
         * 
         * @param cmdBuffer The VkCommandBuffer which contains the commands to be executed.
         */
		void endSingleTimeCommands(VkCommandBuffer cmdBuffer) const;


		VkCommandBuffer createCommandBuffer(VkCommandBufferLevel level, bool bBegin = false);
		VkCommandBuffer createCommandBuffer(VkCommandBufferLevel level, VkCommandPool pool, bool bBegin = false);

		/// Ends, submits and frees a one‑time command buffer
		void flushCommandBuffer(VkCommandBuffer cmdBuffer, VkQueue queue, bool bFree = true);
		void flushCommandBuffer(VkCommandBuffer cmdBuffer, VkQueue queue, VkCommandPool pool, bool bFree = true);

		// void immediateSubmit(TFunction<void(VkCommandBuffer cmd)>&& func) const;

#pragma endregion CmdBuffer



// =========================================================================================================================
#pragma region [ Resource ]
// =========================================================================================================================

	public:

		/** 
		 * @brief Creates a buffer and allocates memory for it using VMA.
		 * 
		 * @param buffer      The VkBuffer handle to be created.
		 * @param allocation  The VmaAllocation handle for the allocated memory.
		 * @param size        The size of the buffer to be created.
		 * @param bufferUsage The usage flags for the buffer.
		 * @param allocFlags  The allocation create flags for VMA.
		 * @param memoryUsage The memory usage type for VMA.
		 */
		void createBuffer(
			VkBuffer&                buffer, 
			VmaAllocation&           allocation, 
			VkDeviceSize             size, 
			VkBufferUsageFlags       bufferUsage, 
			VmaAllocationCreateFlags allocFlags, 
			VmaMemoryUsage           memoryUsage
		) const;

		/** 
		 * @brief Creates an image and allocates memory for it using VMA.
		 * 
		 * @param image      The VkImage handle to be created.
		 * @param allocation The VmaAllocation handle for the allocated memory.
		 * @param imageInfo  The VkImageCreateInfo structure describing the image to be created.
		 * @param allocInfo  The VmaAllocationCreateInfo structure describing the memory allocation parameters.
		 */
		void createImage(
			VkImage&                       image, 
			VmaAllocation&                 allocation, 
			const VkImageCreateInfo&       imageInfo, 
			const VmaAllocationCreateInfo& allocInfo
		) const;


#pragma endregion Resource

// =========================================================================================================================
#pragma region [ Main Functions ]
// =========================================================================================================================

    private:

		VyDevice() = default;

        /**
         * @brief Creates a Vulkan instance.
         *
         * This function creates a Vulkan instance, which is the entry point for all Vulkan commands.
         * It also sets up the validation layers if they are enabled.
         */
		void createInstance();

        void setupDebugMessenger();

		/**
		 * @brief Creates a Vulkan surface for the window.
		 *
		 * This function creates a Vulkan surface that is associated with the window.
		 *
		 * @param window The window for which to create the surface.
		 */
		void createSurface(VyWindow& window);

		/**
		 * @brief Selects a suitable physical device (GPU).
		 *
		 * This function selects a physical device that meets the required criteria for the application.
		 */
        void pickPhysicalDevice();

        /**
         * @brief Creates a logical device.
         *
         * This function creates a logical device, which is used to interact with the physical device.
         * It also creates the graphics and present queues.
         */
        void createLogicalDevice();

		/**
		 * @brief Creates a Vulkan memory allocator using VMA (Vulkan Memory Allocator).
		 *
		 * This function sets up the VMA allocator, which simplifies memory management for Vulkan resources.
		 */
		void createAllocator();

		/**
		 * @brief Creates the command pools.
		 *
		 * This function creates a command pool, which is used to allocate command buffers.
		 */
		void createCommandPools();

#pragma endregion Internal

// =========================================================================================================================
#pragma region [ Cmd ]
// =========================================================================================================================

		// The below methods can be executed all withing the same command buffer, so they can be all executed in one command together if need it
		// The below methods are executed withing a single time command buffer, so they are atomic
    public:

        void imageMemoryBarrier(
            VkCommandBuffer         cmdBuffer,
            VkImage                 image,
            VkImageLayout           oldLayout,
            VkImageLayout           newLayout,
            VkAccessFlags           srcAcessMask,
            VkAccessFlags           dstAccessMask,
            VkPipelineStageFlags    srcStageMask,
            VkPipelineStageFlags    dstStageMask,
            VkImageSubresourceRange subresourceRange,
            U32                     srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            U32                     dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED
        );


        void imageMemoryBarrier(
            VkImage                 image,
            VkImageLayout           oldLayout,
            VkImageLayout           newLayout,
            VkAccessFlags           srcAcessMask,
            VkAccessFlags           dstAccessMask,
            VkPipelineStageFlags    srcStageMask,
            VkPipelineStageFlags    dstStageMask,
            VkImageSubresourceRange subresourceRange,
            U32                     srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            U32                     dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED
        );


        void copyBuffer(
			VkCommandBuffer cmdBuffer, 
			VkBuffer        srcBuffer, 
			VkBuffer        dstBuffer, 
			VkDeviceSize    size
		);

		void copyBuffer(
			VkBuffer        srcBuffer, 
			VkBuffer        dstBuffer, 
			VkDeviceSize    size
		);


		void copyBufferToImage(
			VkCommandBuffer cmdBuffer, 
			VkBuffer        buffer, 
			VkImage         image, 
			U32             width, 
			U32             height, 
			U32             layerCount = 1, 
			U32             mipLevel   = 1
		);
		
		void copyBufferToImage(
			VkBuffer        buffer, 
			VkImage         image, 
			U32             width, 
			U32             height, 
			U32             layerCount = 1, 
			U32             mipLevel   = 1
		);


		void copyImageToBuffer(
			VkCommandBuffer cmdBuffer, 
			VkImage         image, 
			VkBuffer        buffer, 
			U32             width, 
			U32             height, 
			U32             layerCount = 1, 
			U32             mipLevel   = 1
		);

		void copyImageToBuffer(
			VkImage         image, 
			VkBuffer        buffer, 
			U32             width, 
			U32             height, 
			U32             layerCount = 1, 
			U32             mipLevel   = 1
		);


		void copyImage(
			VkCommandBuffer cmdBuffer, 
			VkImage         srcImage, 
			VkImage         dstImage, 
			U32             width, 
			U32             height, 
			U32             srcBaseLayerIndex = 0, 
			U32             dstBaseLayerIndex = 0, 
			U32             layerCount        = 1
		);

        void copyImage(
			VkImage         srcImage, 
			VkImage         dstImage, 
			U32             width, 
			U32             height, 
			U32             srcBaseLayerIndex = 0, 
			U32             dstBaseLayerIndex = 0, 
			U32             layerCount        = 1
		);
		

		void transitionImageLayout(
			VkCommandBuffer cmdBuffer, 
			VkImage         image, 
			VkFormat        format, 
			VkImageLayout   oldLayout, 
			VkImageLayout   newLayout, 
			U32             baseLayerIndex = 0, 
			U32             layerCount     = 1, 
			U32             mipLevel       = 1
		);

		void transitionImageLayout(
			VkImage         image, 
			VkFormat        format, 
			VkImageLayout   oldLayout, 
			VkImageLayout   newLayout, 
			U32             baseLayerIndex = 0, 
			U32             layerCount     = 1, 
			U32             mipLevel       = 1
		);

    private:

		void recordCopyBuffer(
			VkCommandBuffer cmdBuffer, 
			VkBuffer        srcBuffer, 
			VkBuffer        dstBuffer, 
			VkDeviceSize    size
		);
		
		void recordCopyBufferToImage(
			VkCommandBuffer cmdBuffer, 
			VkBuffer        buffer, 
			VkImage         image, 
			U32             width, 
			U32             height, 
			U32             layerCount, 
			U32             mipLevel
		);
		
		void recordCopyImageToBuffer(
			VkCommandBuffer cmdBuffer, 
			VkImage         image, 
			VkBuffer        buffer, 
			U32             width, 
			U32             height, 
			U32             layerCount, 
			U32             mipLevel
		);
		
		void recordCopyImage(
			VkCommandBuffer cmdBuffer, 
			VkImage         srcImage, 
			VkImage         dstImage, 
			U32             width, 
			U32             height, 
			U32             srcBaseLayerIndex, 
			U32             dstBaseLayerIndex, 
			U32             layerCount
		);
		
		
		void recordTransitionImageLayout(
			VkCommandBuffer cmdBuffer, 
			VkImage         image, 
			VkFormat        format, 
			VkImageLayout   oldLayout, 
			VkImageLayout   newLayout, 
			U32             baseLayerIndex, 
			U32             layerCount, 
			U32             mipLevel
		);

#pragma endregion Cmd
    };
}