#pragma once

#include <VyEngine/Core/Window/Window.h>
#include <VyEngine/VK/Swapchain/Swapchain.h>

#include <VyEngine/VK/Pipeline/Pipeline.h>
#include <VyEngine/VK/Pipeline/Framebuffer.h>
#include <VyEngine/VK/Descriptors/Descriptors.h>

#include <VyEngine/GFX/Data/FrameInfo.h>
#include <iostream>

#include <VyEngine/VK/Context.h>

namespace Vy
{
    // Declarations
    class VyContext;

	struct FrameBufferAttachment 
    {
		VyImage     Image;
		VyImageView View;
		VkFormat    Format;

        const FrameBufferAttachment operator=(const FrameBufferAttachment&) = delete;
	};


	struct FrameBuffer 
    {
        VkExtent2D    Extent;
		VkFramebuffer Framebuffer;
		VkRenderPass  RenderPass;
		VySampler     Sampler;

		// One attachment for every component required for a deferred rendering setup
		FrameBufferAttachment Position;
        FrameBufferAttachment Normal;
        FrameBufferAttachment Albedo;
		FrameBufferAttachment Depth;

		const FrameBuffer operator=(const FrameBuffer&) = delete;
		
        ~FrameBuffer() 
        {
			std::cout << "Destroying FrameBuffer\n";
			vkDestroyFramebuffer(VyContext::device(), Framebuffer, nullptr);
			vkDestroyRenderPass(VyContext::device(), RenderPass, nullptr);
		}
	};


	struct OffscreenPass 
    {
        VkExtent2D            Extent;
		VkFramebuffer         Framebuffer;
		FrameBufferAttachment Color{};
		FrameBufferAttachment Depth{};
		VkRenderPass          RenderPass;
		VySampler             Sampler;
		U32                   RenderTargetHandle;
		VkDescriptorImageInfo ColorImageInfo;
		
        ~OffscreenPass() 
        {
			std::cout << "Destroying OffscreenPass\n";
			vkDestroyRenderPass(VyContext::device(), RenderPass, nullptr);
			vkDestroyFramebuffer(VyContext::device(), Framebuffer, nullptr);
		}
	};


    /**
     * @class Renderer
     * 
     * @brief Manages rendering operations, including swapchain management and command buffer handling.
     * This class encapsulates the logic for rendering to a window using a Vulkan swapchain. It handles the creation
     * and management of command buffers, frame synchronization, and swapchain recreation when necessary.
     */
    class VyRenderer 
    {
    public:

        /**
         * @brief Constructs a VyRenderer object.
         * 
         * Initializes the renderer by recreating the swapchain and creating command buffers.
         * 
         * @param window The window to render to.
         */
        VyRenderer(VyWindow& window);

        VyRenderer(const VyRenderer&) = delete;
        VyRenderer(VyRenderer&&)      = delete;

        /**
         * @brief Destructor for the VyRenderer class, frees the command buffers.
         */
        ~VyRenderer();

        VyRenderer& operator=(const VyRenderer&) = delete;
        VyRenderer& operator=(VyRenderer&&)      = delete;

        VySwapchain* getSwapchain() const { return m_Swapchain.get(); }

        /**
         * @brief Gets the Vulkan render pass associated with the swapchain.
         * 
         * @return The Vulkan render pass.
         */
        VkRenderPass swapchainRenderPass() const 
        { 
            return m_Swapchain->renderPass(); 
        }

        /**
         * @brief Gets the Vulkan render pass used for offscreen rendering.
         * 
         * @return The Vulkan render pass.
         */
        // VkRenderPass offscreenRenderPass() const 
        // { 
        //     return m_OffscreenFramebuffer->renderPass(); 
        // }

        // VkDescriptorImageInfo offscreenDescriptionImageInfo(int index) const;
        // VkDescriptorImageInfo depthDescriptionImageInfo(int index) const;

        /**
         * @brief Gets the aspect ratio (width / height) of the swapchain extent.
         * 
         * @return The aspect ratio of the swapchain extent.
         */
        float aspectRatio() const 
        { 
            return m_Swapchain->extentAspectRatio();
        }


        /**
         * @brief Gets the extent of the swapchain.
         * 
         * @return The extent of the swapchain extent. 
         */
        VkExtent2D swapchainExtent() const 
        { 
            return m_Swapchain->swapchainExtent(); 
        }


        /**
         * @brief Gets the color format of the swapchain.
         */
        VkFormat swapchainColorFormat() const 
        { 
            return m_Swapchain->swapchainColorFormat(); 
        }


        /**
         * @brief Gets the depth format of the swapchain.
         */
        VkFormat swapchainDepthFormat() const 
        { 
            return m_Swapchain->swapchainDepthFormat(); 
        }


        /**
         * @brief Checks if a frame is currently in progress.
         * 
         * @return True if a frame is in progress, false otherwise.
         */
        bool isFrameInProgress() const 
        { 
            return m_IsFrameStarted; 
        }


        bool wasSwapchainRecreated() const 
        { 
            return m_SwapchainRecreated; 
        }


        /**
         * @brief Gets the current command buffer.
         * 
         * @return The current command buffer.
         * 
         * @throws std::runtime_error if called when no frame is in progress.
         */
        VkCommandBuffer currentCommandBuffer() const 
        {
            VY_ASSERT(m_IsFrameStarted, "Cannot get command buffer when frame not in progress");
            VY_ASSERT(m_CommandBuffers[ m_CurrentFrameIndex ] != VK_NULL_HANDLE, "Command buffer not initialized");
            
            return m_CommandBuffers[ m_CurrentFrameIndex ];
        }


        /**
         * @brief Gets the current frame index.
         * 
         * @return The current frame index.
         * 
         * @throws std::runtime_error if called when no frame is in progress.
         */
        int frameIndex() const 
        {
            VY_ASSERT(m_IsFrameStarted, "Cannot get frame index while frame is not in progress");

            return m_CurrentFrameIndex;
        }


        /**
         * @brief Begins a new frame for rendering.
         * 
         * Acquires the next swapchain image, begins recording the command buffer, and returns it.
         * Handles swapchain recreation if it becomes out of date.
         * 
         * @return The command buffer for the new frame.
         * 
         * @throws std::runtime_error If acquiring the swapchain image or beginning the command buffer fails.
         */
        VkCommandBuffer beginFrame();


        /**
         * @brief Ends the current frame and presents the rendered image.
         * Ends recording the command buffer, submits it for execution, and presents the rendered image.
         * Handles swapchain recreation if it becomes out of date or if the window is resized.
         * 
         * @throws std::runtime_error If ending the command buffer or presenting the swapchain image fails.
         */
        void endFrame();


        /**
         * @brief Begins the swapchain render pass.
         * 
         * @param cmdBuffer The command buffer to record the render pass into.
         * 
         * @throws std::runtime_error if called when frame is not in progress or command buffer is from a different frame.
         */
        void beginSwapchainRenderPass(VkCommandBuffer cmdBuffer);
        
        
		/**
         * @brief Ends the current render pass.
         * 
         * @param cmdBuffer The command buffer to record the end of the current render pass into.
		 *
         * @throws std::runtime_error if called when frame is not in progress or command buffer is from a different frame.
		 */
        void endCurrentRenderPass(VkCommandBuffer cmdBuffer) const;
        

        // void beginOffscreenRenderPass(VkCommandBuffer cmdBuffer);
        // void endOffscreenRenderPass(VkCommandBuffer cmdBuffer) const;

        // void generateOffscreenMipmaps(VkCommandBuffer cmdBuffer);

		void setUpOffscreenRenderPass(U32 texW, U32 texH);
		void createOffscreenRenderPass(U32 texW, U32 texH);
		void beginOffscreenRenderPass(VkCommandBuffer cmdBuffer);

    private:

        /**
         * @brief Creates the command buffers used for rendering.
         * 
         * @throws std::runtime_error If command buffer allocation fails.
         */
        void createCommandBuffers();

        /**
         * @brief Frees the allocated command buffers.
         */
        void freeCommandBuffers();

        /**
         * @brief Recreates the swapchain, handles window resizing and initial swapchain creation.
         * 
         * @throws std::runtime_error If the swapchain image format, color space, or size has changed unexpectedly.
         */
        void recreateSwapchain();

        void createOffscreenResources();

		//Offscreen Render tasks
		void createOffscreenColorAttachment();
		void createOffscreenDepthsAttachment(VkFormat& depthsFormat);
		void createOffscreenAttachmentDescriptors(TArray<VkAttachmentDescription,2>& descriptors, VkFormat& depthsFormat);
		void createOffscreenSubpassDependencies(TArray<VkSubpassDependency, 2>& dependencies);
		void createOffscreenFramebuffer();

		void createAttachment(VkFormat format, VkImageUsageFlagBits usage, FrameBufferAttachment* pAttachment);
		void prepareDeferredRenderFramebuffer();

    private:

        VyWindow&                  m_Window;
        VyContext&                 m_VyContext;

        Unique<VySwapchain>        m_Swapchain;
        TVector<VkCommandBuffer>   m_CommandBuffers;

        U32                        m_CurrentImageIndex { 0 }; // Index of the current swapchain image.
        int                        m_CurrentFrameIndex { 0 }; // Index of the current frame.
        bool                       m_IsFrameStarted    { false };
        bool                       m_SwapchainRecreated{ false };

		OffscreenPass   m_OffscreenPass{};
		VkCommandBuffer m_DeferredCmdBuffer;
		FrameBuffer     m_DeferredRenderFramebuffer{};

        // Unique<VyFramebuffer>      m_OffscreenFramebuffer{ nullptr };

        // HZB Generation Resources
        // VkPipelineLayout              m_HZBPipelineLayout{VK_NULL_HANDLE};
        // Unique<VyPipeline>            m_HZBPipeline      { nullptr };
        // Unique<VyDescriptorSetLayout> m_HZBSetLayout     { nullptr };
        // Unique<VyDescriptorPool>      m_HZBDescriptorPool{ nullptr };
        // VkDescriptorSetLayout m_hzbSetLayout     { VK_NULL_HANDLE };
        // VkDescriptorPool      m_hzbDescriptorPool{ VK_NULL_HANDLE };

        // Sets for each frame and each mip transition
        // Outer: Frame, Inner: Mip Level
        // TVector<TVector<VkDescriptorSet>> m_HZBDescriptorSets;

        
    };
}