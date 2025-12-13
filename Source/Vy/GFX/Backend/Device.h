#pragma once

#include <Vy/GFX/Backend/VK/VKCore.h>
#include <Vy/Core/Window.h>

namespace Vy
{

#ifdef VY_DEBUG_MODE
    constexpr bool kEnableValidationLayers = false;
#else
	constexpr bool kEnableValidationLayers = false;
#endif

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
		static constexpr auto kValidationLayers = std::array{ "VK_LAYER_KHRONOS_validation"   };
		static constexpr auto kDeviceExtensions = std::array{ VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME };

 		VyDevice(const VyDevice&) = delete;
		 VyDevice(VyDevice&&)     = delete;

		 ~VyDevice();

		 VyDevice& operator=(const VyDevice&) = delete;
		 VyDevice& operator=(VyDevice&&)      = delete;

        operator           VkDevice()                                               { return m_Device; }
		VY_NODISCARD       VkDevice                    handle()               const { return m_Device; }
		VY_NODISCARD       VkInstance                  instance()             const { return m_Instance; }
		VY_NODISCARD       VkPhysicalDevice            physicalDevice()       const { return m_PhysicalDevice; }
		VY_NODISCARD       VmaAllocator                allocator()            const { return m_Allocator; }
		VY_NODISCARD       VkSurfaceKHR                surface()              const { return m_Surface; }
        VY_NODISCARD       VkCommandPool               commandPool()          const { return m_CommandPool; }
        VY_NODISCARD       VkQueue                     graphicsQueue()        const { return m_GraphicsQueue; }
        VY_NODISCARD       VkQueue                     presentQueue()         const { return m_PresentQueue; }
		VY_NODISCARD       VkQueue                     computeQueue()         const { return m_ComputeQueue; }
		VY_NODISCARD const VkPhysicalDeviceProperties& properties()           const { return m_Properties; }
		VY_NODISCARD const VKFeatures&                 features()             const { return m_Features; }
		VY_NODISCARD       VkSampleCountFlagBits       supportedSampleCount()       { return m_MsaaSamples; }
		VY_NODISCARD       bool                        supportsPresentId()    const { return m_PresentIdSupported; }

		/** 
		 * @brief Initializes the Vulkan device and related resources.
		 * 
		 * @note Called from 'VyContext::initialize(VyWindow& window)'.
		 * 
		 * @param window The VyWindow instance used to create the Vulkan surface.
		 */
		void initialize(VyWindow& window);

		/** 
		 * @brief Query the necessary details about the physical device's 
		 *        capabilities with respect to a specific window surface.
		 * 
		 * This method gathers information about the swap chain support,
		 * including surface capabilities, supported formats, and presentation modes.
		 * 
		 * @return A SwapchainSupportDetails structure containing the queried information.
		 */
		SwapchainSupportDetails querySwapchainSupport() const;
        
        /**
         * @brief Identifying suitable queue families on a given physical device.
         */
		QueueFamilyIndices findQueueFamilies(); //const;


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


		TVector<VkExtensionProperties> queryDeviceExtensionProperties(VkPhysicalDevice device);
		TVector<VkExtensionProperties> queryInstanceExtensionProperties();


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

        // Buffer Helper Functions
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
		* @brief Copies data from a source buffer to a destination buffer.
		*
		* @param srcBuffer The source buffer handle.
		* @param dstBuffer The destination buffer handle.
		* @param size      The size of the data to copy.
		*/
		// void copyBuffer(
		// 	VkBuffer     srcBuffer, 
		// 	VkBuffer     dstBuffer, 
		// 	VkDeviceSize size
		// ); //const;

		// void copyBuffer(
		// 	VkCommandBuffer cmdBuffer,
		// 	VkBuffer        srcBuffer, 
		// 	VkBuffer        dstBuffer, 
		// 	VkDeviceSize    size
		// );

		/**
		 * @brief Copies data from a buffer to an image.
		 * 
		 * This function copies data from a buffer to an image using a command buffer.
		 * 
		 * @param buffer         The source buffer handle.
		 * @param image          The destination image handle.
		 * @param width          The width of the image.
		 * @param height         The height of the image.
		 * @param layerCount     The number of image layers.
		 * @param baseArrayLayer (Optional) The first layer of the image array to copy to. Default is 0.
		 * @param depth          (Optional) The depth of the image. Default is 1.
		 */
		// void copyBufferToImage(
		// 	VkBuffer buffer, 
		// 	VkImage  image, 
		// 	U32      width, 
		// 	U32      height, 
		// 	U32      layerCount,
		// 	U32      baseArrayLayer = 0,
		// 	U32      depth          = 1
		// ) const;


		void copyBufferToImageCubemap(
			VkBuffer buffer, 
			VkImage  image, 
			U32      width, 
			U32      height, 
			U32      layerCount
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


		// The below methods can be executed all withing the same command buffer, so they can be all executed in one command together if need it
		void copyBuffer(
			VkCommandBuffer cmdBuffer, 
			VkBuffer srcBuffer, 
			VkBuffer dstBuffer, 
			VkDeviceSize size
		);
		
		void copyBufferToImage(
			VkCommandBuffer cmdBuffer, 
			VkBuffer buffer, 
			VkImage image, 
			U32 width, 
			U32 height, 
			U32 layerCount = 1, 
			U32 mipLevel = 1
		);
		
		void copyImageToBuffer(
			VkCommandBuffer cmdBuffer, 
			VkImage image, 
			VkBuffer buffer, 
			U32 width, 
			U32 height, 
			U32 layerCount = 1, 
			U32 mipLevel = 1
		);

		void copyImage(
			VkCommandBuffer cmdBuffer, 
			VkImage srcImage, 
			VkImage dstImage, 
			U32 width, 
			U32 height, 
			U32 srcBaseLayerIndex = 0, 
			U32 dstBaseLayerIndex = 0, 
			U32 layerCount = 1
		);

		void transitionImageLayout(
			VkCommandBuffer cmdBuffer, 
			VkImage image, 
			VkFormat format, 
			VkImageLayout oldLayout, 
			VkImageLayout newLayout, 
			U32 baseLayerIndex = 0, 
			U32 layerCount = 1, 
			U32 mipLevel = 1
		);

		// The below methods are executed withing a single time command buffer, so they are atomic
		void copyBuffer(
			VkBuffer srcBuffer, 
			VkBuffer dstBuffer, 
			VkDeviceSize size
		);
		
		void copyBufferToImage(
			VkBuffer buffer, 
			VkImage image, 
			U32 width, 
			U32 height, 
			U32 layerCount = 1, 
			U32 mipLevel = 1
		);

		void copyImageToBuffer(
			VkImage image, 
			VkBuffer buffer, 
			U32 width, 
			U32 height, 
			U32 layerCount = 1, 
			U32 mipLevel = 1
		);

			void copyImage(
			VkImage srcImage, 
			VkImage dstImage, 
			U32 width, 
			U32 height, 
			U32 srcBaseLayerIndex = 0, 
			U32 dstBaseLayerIndex = 0, 
			U32 layerCount = 1
		);
		
		void transitionImageLayout(
			VkImage image, 
			VkFormat format, 
			VkImageLayout oldLayout, 
			VkImageLayout newLayout, 
			U32 baseLayerIndex = 0, 
			U32 layerCount = 1, 
			U32 mipLevel = 1
		);
	
		/**
		* @brief Creates an image view for an image.
		*
		* This function creates an image view for an image, which is used to access the image data.
		*
		* @param viewInfo The image view create info.
		* @return The image view handle.
		*/
		VkImageView createImageView(const VkImageViewCreateInfo& viewInfo);


		VkSampler createSampler(const VkSamplerCreateInfo& samplerInfo);


		VkResult createImageView(
			VkImageView&                 imageView, 
			const VkImageViewCreateInfo& viewInfo
		);
		
		VkResult createImage(
			VkImage&                       image, 
			VmaAllocation&                 allocation, 
			const VkImageCreateInfo&       imageInfo, 
			const VmaAllocationCreateInfo& allocInfo
		);

		/** 
		 * @brief Transitions the layout of an image.
		 * 
		 * This function transitions the layout of an image from an old layout to a new layout.
		 * It records the necessary commands into the provided command buffer.
		 * 
		 * @param cmdBuffer   The command buffer to record the transition commands into.
		 * @param image       The image to transition.
		 * @param oldLayout   The current layout of the image.
		 * @param newLayout   The desired layout of the image.
		 * @param layerCount  (Optional) The number of layers in the image. Default is 1.
		 * @param baseLayer   (Optional) The first layer of the image to transition. Default is 0.
		 * @param levelCount  (Optional) The number of mip levels in the image. Default is 1.
		 * @param baseLevel   (Optional) The first mip level of the image to transition. Default is 0.
		 * @param aspectFlags (Optional) The aspect flags of the image. Default is VK_IMAGE_ASPECT_COLOR_BIT.
		 */
		void transitionImageLayout(
			VkCommandBuffer    cmdBuffer, 
			VkImage            image, 
			VkImageLayout      oldLayout, 
			VkImageLayout      newLayout, 
            U32                layerCount  = 1,
            U32                baseLayer   = 0,
            U32                levelCount  = 1,
            U32                baseLevel   = 0,
			VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT
		) const;


		/** 
		 * @brief Transitions the layout of an image.
		 * 
		 * This function transitions the layout of an image from an old layout to a new layout.
		 * It records the necessary commands into a command buffer and submits it for execution.
		 * 
		 * @param image       The image to transition.
		 * @param oldLayout   The current layout of the image.
		 * @param newLayout   The desired layout of the image.
		 * @param layerCount  (Optional) The number of layers in the image. Default is 1.
		 * @param baseLayer   (Optional) The first layer of the image to transition. Default is 0.
		 * @param levelCount  (Optional) The number of mip levels in the image. Default is 1.
		 * @param baseLevel   (Optional) The first mip level of the image to transition. Default is 0.
		 * @param aspectFlags (Optional) The aspect flags of the image. Default is VK_IMAGE_ASPECT_COLOR_BIT.
		 */
		void transitionImageLayout(
			VkImage            image, 
			VkImageLayout      oldLayout, 
			VkImageLayout      newLayout, 
            U32                layerCount  = 1,
            U32                baseLayer   = 0,
            U32                levelCount  = 1,
            U32                baseLevel   = 0,
			VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT
		) const;


		/** 
		 * @brief Transitions the layout of an image.
		 * 
		 * This function transitions the layout of an image from an old layout to a new layout.
		 * It records the necessary commands into a command buffer and submits it for execution.
		 * 
		 * @param image      The image to transition.
		 * @param format     The format of the image.
		 * @param oldLayout  The current layout of the image.
		 * @param newLayout  The desired layout of the image.
		 * @param mipLevels  (Optional) The number of mip levels in the image. Default is 1.
		 */
		void transitionImageLayout(
			VkImage            image, 
			VkFormat           format, 
			VkImageLayout      oldLayout, 
			VkImageLayout      newLayout, 
			U32                mipLevels   = 1
		) const;

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
		 * @brief Creates a graphics command pool.
		 *
		 * This function creates a command pool, which is used to allocate command buffers.
		 */
		void createCommandPool();

        // ========================================================================================

        /**
         * @brief Finds the queue families for a physical device.
         *
         * This function finds the graphics and present queue families for a physical device.
         *
         * @param device The physical device to find the queue families for.
		 * 
         * @return The queue family indices.
         */
		QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);// const;

        /**
         * @brief Checks if the validation layers are supported.
         *
         * This function checks if all the required validation layers are supported by the Vulkan instance.
         *
         * @return true if all layers are supported, false otherwise.
         */
        bool checkValidationLayerSupport();

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



        void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);


		VkSampleCountFlagBits getMaxSampleCount();


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


	private:

		VkInstance       m_Instance       = VK_NULL_HANDLE;
		VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
		VkDevice         m_Device         = VK_NULL_HANDLE;
		VmaAllocator     m_Allocator      = VK_NULL_HANDLE;
		
        VkDebugUtilsMessengerEXT m_DebugMessenger;

        VkCommandPool            m_CommandPool = VK_NULL_HANDLE;

		VkPhysicalDeviceProperties m_Properties{};
		VKFeatures                 m_Features  {};
		VkSurfaceKHR               m_Surface = VK_NULL_HANDLE;
		
		VkQueue		m_GraphicsQueue	{ VK_NULL_HANDLE };
		VkQueue		m_PresentQueue	{ VK_NULL_HANDLE };
		VkQueue		m_ComputeQueue	{ VK_NULL_HANDLE };

		VkSampleCountFlagBits m_MsaaSamples = VK_SAMPLE_COUNT_1_BIT;

		bool m_PresentIdSupported = false;
    };
}


	// // -- Helper Structs --
	// struct FrameSync
	// {
	// 	VkSemaphore ImageAvailable;
	// 	VkSemaphore RenderFinished;
	// 	VkFence     InFlight;
	// };

	// struct SemaphoreInfo
	// {
	// 	TVector<VkSemaphore>          WaitSemaphores;
	// 	TVector<VkPipelineStageFlags> WaitStages;
	// 	TVector<VkSemaphore>          SignalSemaphores;
	// };


	// //? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// //? ~~	  SyncManager	
	// //? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// class SyncManager final
	// {
	// public:
	// 	//--------------------------------------------------
	// 	//    Constructor & Destructor
	// 	//--------------------------------------------------
	// 	explicit SyncManager() = default;

	// 	~SyncManager() = default;
		
	// 	SyncManager(const SyncManager& other) = delete;
	// 	SyncManager(SyncManager&& other) noexcept = delete;
		
	// 	SyncManager& operator=(const SyncManager& other) = delete;
	// 	SyncManager& operator=(SyncManager&& other) noexcept = delete;

	// 	void create(U32 maxFramesInFlight);
	// 	void cleanup();

	// 	//--------------------------------------------------
	// 	//    Accessors & Mutators
	// 	//--------------------------------------------------
	// 	const FrameSync& getFrameSync(U32 frame) const;

	// 	//--------------------------------------------------
	// 	//    Makers
	// 	//--------------------------------------------------
	// 	const VkSemaphore& createSemaphore();
	// 	const VkFence&     createFence(bool signaled);

	// private:
	// 	U32					m_MaxFrames		{};

	// 	TVector<VkSemaphore>	m_Semaphores	{};
	// 	TVector<VkFence>		m_Fences		{};

	// 	TVector<FrameSync>		m_FrameSyncs	{};
	// };
	
	// class CommandBuffer final
	// {
	// public:
	// 	//--------------------------------------------------
	// 	//    Constructor & Destructor
	// 	//--------------------------------------------------
	// 	explicit CommandBuffer(VkCommandPool pool, VkCommandBuffer buffer);

	// 	~CommandBuffer() = default;
		
	// 	CommandBuffer(const CommandBuffer& other) = delete;
	// 	CommandBuffer(CommandBuffer&& other) noexcept;
		
	// 	CommandBuffer& operator=(const CommandBuffer& other) = delete;
	// 	CommandBuffer& operator=(CommandBuffer&& other) noexcept;

	// 	//--------------------------------------------------
	// 	//    Accessors & Mutators
	// 	//--------------------------------------------------
	// 	const VkCommandBuffer& handle() const;

	// 	//--------------------------------------------------
	// 	//    Commands
	// 	//--------------------------------------------------
	// 	void begin(VkCommandBufferUsageFlags usage = 0) const;
	// 	void end() const;
	// 	void submit(VkQueue queue, bool waitIdle, const SemaphoreInfo& semaphoreInfo = {}, VkFence fence = VK_NULL_HANDLE) const;
	// 	void reset() const;
	// 	void free();

	// private:
	// 	VkCommandBuffer m_CmdBuffer { VK_NULL_HANDLE };
	// 	VkCommandPool	m_PoolOwner	{ VK_NULL_HANDLE };
	// };


	// class CommandPool final
	// {
	// public:
	// 	//--------------------------------------------------
	// 	//    Constructor & Destructor
	// 	//--------------------------------------------------
	// 	explicit CommandPool() = default;

	// 	~CommandPool() = default;
		
	// 	CommandPool(const CommandPool& other) = delete;
	// 	CommandPool(CommandPool&& other) noexcept = delete;
		
	// 	CommandPool& operator=(const CommandPool& other) = delete;
	// 	CommandPool& operator=(CommandPool&& other) noexcept = delete;

	// 	CommandPool& create();
	// 	void destroy() const;

	// 	//--------------------------------------------------
	// 	//    Accessors & Mutators
	// 	//--------------------------------------------------
	// 	VkCommandPool& handle();
	// 	CommandBuffer& getBuffer(U32 bufferIdx);
	// 	CommandBuffer& allocateCmdBuffers(U32 count, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);

	// private:
	// 	VkCommandPool				m_CommandPool		{ VK_NULL_HANDLE};
	// 	TVector<CommandBuffer>	m_CommandBuffers	{ };
	// };