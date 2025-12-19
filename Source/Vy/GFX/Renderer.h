#pragma once

#include <Vy/GFX/Context.h>
#include <Vy/GFX/Backend/Swapchain.h>

#include <Vy/Core/Window.h>
#include <Vy/GFX/Backend/Resources/Framebuffer.h>
#include <Vy/GFX/Backend/Resources/RenderPass.h>
#include <Vy/GFX/Backend/Pipeline.h>
#include <Vy/GFX/FrameInfo.h>

// namespace Vy
// {
// 	class Renderer 
//     {
// 	public:
// 		Renderer(VyWindow& window);

// 		~Renderer();

// 		Renderer(const Renderer&) = delete;
// 		Renderer& operator=(const Renderer&);

// 		Renderer(Renderer&&) = default;
// 		Renderer& operator=(Renderer&&) noexcept;

//         /**
//          * @brief Gets the Vulkan render pass associated with the swap chain.
//          * 
//          * @return The Vulkan render pass.
//          */
//         VyRenderPass& swapchainRenderPass() const 
//         { 
//             return m_Swapchain->renderPass(); 
//         }

        
//         /**
//          * @brief Gets the aspect ratio (width / height) of the swap chain extent.
//          * 
//          * @return The aspect ratio of the swap chain extent.
//          */
//         float aspectRatio() const 
//         { 
//             return m_Swapchain->extentAspectRatio();
//         }


//         VkExtent2D swapchainExtent() const 
//         { 
//             return m_Swapchain->swapchainExtent(); 
//         }


// 		U32 swapchainImageCount() const 
//         {
// 			return m_Swapchain->imageCount();
// 		}


// 		VkFormat swapchainImageFormat() 
//         {
// 			return m_Swapchain->swapchainImageFormat();
// 		}


//         /**
//          * @brief Checks if a frame is currently in progress.
//          * 
//          * @return True if a frame is in progress, false otherwise.
//          */
//         bool isFrameInProgress() const 
//         { 
//             return m_IsFrameStarted; 
//         }


//         /**
//          * @brief Gets the current command buffer.
//          * 
//          * @return The current command buffer.
//          * 
//          * @throws std::runtime_error if called when no frame is in progress.
//          */
//         VkCommandBuffer currentCommandBuffer() const 
//         {
//             VY_ASSERT(m_IsFrameStarted, "Cannot get command buffer when frame not in progress");
//             VY_ASSERT(m_CommandBuffers[m_CurrentFrameIndex] != VK_NULL_HANDLE, "Command buffer not initialized");
            
//             return m_CommandBuffers[m_CurrentFrameIndex];
//         }


//         /**
//          * @brief Gets the current frame index.
//          * 
//          * @return The current frame index.
//          * 
//          * @throws std::runtime_error if called when no frame is in progress.
//          */
//         int frameIndex() const 
//         {
//             VY_ASSERT(m_IsFrameStarted, "Cannot get frame index while frame is not in progress");

//             return m_CurrentFrameIndex;
//         }

//         /**
//          * @brief Begins a new frame for rendering.
//          * 
//          * Acquires the next swap chain image, begins recording the command buffer, and returns it.
//          * Handles swap chain recreation if it becomes out of date.
//          * 
//          * @return The command buffer for the new frame.
//          * 
//          * @throws std::runtime_error If acquiring the swap chain image or beginning the command buffer fails.
//          */
//         VkCommandBuffer beginFrame();


//         /**
//          * @brief Ends the current frame and presents the rendered image.
//          * Ends recording the command buffer, submits it for execution, and presents the rendered image.
//          * Handles swap chain recreation if it becomes out of date or if the window is resized.
//          * 
//          * @throws std::runtime_error If ending the command buffer or presenting the swap chain image fails.
//          */
//         void endFrame();


		
//         void beginRenderPass(VkCommandBuffer commandBuffer, VyRenderInfo renderInfo);
		
//         void beginPresentRenderPass(VkCommandBuffer commandBuffer);
		
//         void endRenderPass(VkCommandBuffer commandBuffer) const;
		
//         void recreateSwapchain();

// 	private:
// 		void createCommandBuffers();
// 		void freeCommandBuffers();

// 		VyWindow&                  m_Window;
//         VyContext&                 m_VyContext;
// 		Unique<VySwapchain>        m_Swapchain;
// 		TVector<VkCommandBuffer>   m_CommandBuffers;

// 		U32 m_CurrentImageIndex{};
// 		int m_CurrentFrameIndex{0};
// 		bool m_IsFrameStarted{false};
// 	};
// }

#define SHADOWMAP_DIM 4096

namespace Vy
{

    /**
     * @class Renderer
     * 
     * @brief Manages rendering operations, including swap chain management and command buffer handling.
     * This class encapsulates the logic for rendering to a window using a Vulkan swap chain. It handles the creation
     * and management of command buffers, frame synchronization, and swap chain recreation when necessary.
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


        /**
         * @brief Gets the Vulkan render pass associated with the swap chain.
         * 
         * @return The Vulkan render pass.
         */
        VyRenderPass& swapchainRenderPass() const 
        { 
            return m_Swapchain->renderPass(); 
        }

        
        /**
         * @brief Gets the aspect ratio (width / height) of the swap chain extent.
         * 
         * @return The aspect ratio of the swap chain extent.
         */
        float aspectRatio() const 
        { 
            return m_Swapchain->extentAspectRatio();
        }


        VkExtent2D swapchainExtent() const 
        { 
            return m_Swapchain->swapchainExtent(); 
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
            VY_ASSERT(m_CommandBuffers[m_CurrentFrameIndex] != VK_NULL_HANDLE, "Command buffer not initialized");
            
            return m_CommandBuffers[m_CurrentFrameIndex];
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
         * Acquires the next swap chain image, begins recording the command buffer, and returns it.
         * Handles swap chain recreation if it becomes out of date.
         * 
         * @return The command buffer for the new frame.
         * 
         * @throws std::runtime_error If acquiring the swap chain image or beginning the command buffer fails.
         */
        VkCommandBuffer beginFrame();


        /**
         * @brief Ends the current frame and presents the rendered image.
         * Ends recording the command buffer, submits it for execution, and presents the rendered image.
         * Handles swap chain recreation if it becomes out of date or if the window is resized.
         * 
         * @throws std::runtime_error If ending the command buffer or presenting the swap chain image fails.
         */
        void endFrame();


        /**
         * @brief Begins the swap chain render pass.
         * 
         * @param cmdBuffer The command buffer to record the render pass into.
         * 
         * @throws std::runtime_error if called when frame is not in progress or command buffer is from a different frame.
         */
        void beginSwapchainRenderPass(VkCommandBuffer cmdBuffer);


		/**
		 * @brief Ends the swap chain render pass.
         * 
		 * @param cmdBuffer The command buffer to record the end of the swap chain render pass into.
		 *
		 * @throws std::runtime_error if called when frame is not in progress or command buffer is from a different frame.
		 */
        void endSwapchainRenderPass(VkCommandBuffer cmdBuffer) const;

        // https://github.com/invzz/VulkanEngine/blob/6385247108ad82d91c9d80f191848d354079f5a9/include/Engine/Graphics/Renderer.hpp
        // VkRenderPass offscreenRenderPass() const 
        // { 
        //     return m_OffscreenFramebuffer->renderPass(); 
        // }

        // void beginShadowSwapchainRenderPass(VkCommandBuffer cmdBuffer);

        // VyRenderPass&        shadowMapRenderPass()     const { return m_Swapchain->shadowRenderPass();     }
        // VkImageView          shadowMapDepthImageView() const { return m_Swapchain->shadowDepthImageView(); }
        // VkSampler            shadowMapDepthSampler()   const { return m_Swapchain->shadowDepthSampler();   }
        // const VyFramebuffer& shadowMapFrameBuffer()    const { return m_Swapchain->shadowMapFramebuffer(); }
        // VkDescriptorImageInfo shadowMapDescriptorImageInfo() { return m_Swapchain->shadowDescriptorImageInfo(); }
        // VkDescriptorImageInfo offscreenDescriptorImageInfo(int index) const;
        // VkDescriptorImageInfo depthDescriptorImageInfo(int index) const;

        // void beginOffscreenRenderPass(VkCommandBuffer cmdBuffer);
        // void endOffscreenRenderPass(VkCommandBuffer cmdBuffer) const;
        
        // void generateOffscreenMipmaps(VkCommandBuffer cmdBuffer);
        // void generateDepthPyramid(VkCommandBuffer cmdBuffer);
        
        // void beginRenderPass(const VyFramebuffer& frameBuffer, const VyRenderPass& renderPass) const;
        
    private:
        
        // void createOffscreenResources();
        // void createHZBPipeline();

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
         * @brief Recreates the swap chain, handles window resizing and initial swap chain creation.
         * 
         * @throws std::runtime_error If the swap chain image format, color space, or size has changed unexpectedly.
         */
        void recreateSwapchain();


        

    private:

        VyWindow&                  m_Window;
        VyContext&                 m_VyContext;

        Unique<VySwapchain>        m_Swapchain;
        TVector<VkCommandBuffer>   m_CommandBuffers;

        // Unique<VyHZBFramebuffer>   m_OffscreenFramebuffer;

        U32                        m_CurrentImageIndex{ 0 }; // Index of the current swap chain image.
        int                        m_CurrentFrameIndex{ 0 }; // Index of the current frame.
        bool                       m_IsFrameStarted{ false };

        //-----------------------------------------------------------------------------------------
        // HZB (Hierarchical Z-Buffer) Generation Resources
        // Unique<VyPipeline>                m_HZBPipeline;
        
        // Unique<VyDescriptorSetLayout>     m_HZBSetLayout;
        
        // Unique<VyDescriptorPool>          m_HZBDescriptorPool;

        // // Sets for each frame and each mip transition. [Frame][Mip]
        // TVector<TVector<VkDescriptorSet>> m_HZBDescriptorSets;

        //-----------------------------------------------------------------------------------------
    };
}