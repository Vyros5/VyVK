#include <Vy/GFX/Renderer.h>

#include <Vy/GFX/Context.h>

namespace Vy
{
    // =====================================================================================================================

    VyRenderer::VyRenderer(VyWindow& window) : 
        m_Window   { window                          },
        m_VyContext{ VyContext::initialize(m_Window) }
    {
        recreateSwapchain();
        createCommandBuffers();
    }


    VyRenderer::~VyRenderer() 
    { 
        freeCommandBuffers(); 
    }

    // =====================================================================================================================

    void VyRenderer::recreateSwapchain() 
    {
        auto extent = m_Window.windowExtent();

        // While the window is minimized...
        while (extent.width == 0 || extent.height == 0) 
        {
            extent = m_Window.windowExtent();
            
            // While one of the windows dimensions is 0 (e.g. during minimization), wait until otherwise.
            // https://www.glfw.org/docs/3.3/group__window.html#ga554e37d781f0a997656c26b2c56c835e
            glfwWaitEvents();
        }

        // Wait until the current swapchain is no longer being used before (re)creating it.
        VyContext::waitIdle();

        // First-Time swapchain creation
        if (m_Swapchain == nullptr) 
        {
            // Create new swapchain with new extents.
            m_Swapchain = MakeUnique<VySwapchain>(extent);
        } 
        else // Recreate swapchain.
        {
            Shared<VySwapchain> oldSwapchain = std::move(m_Swapchain);

            // Create new swapchain with new extents and pass through the old swap chain if it exists.
            m_Swapchain = MakeUnique<VySwapchain>(extent, oldSwapchain);

            // Check if the old and new swap chains are compatible.
            if (!oldSwapchain->compareSwapFormats( *m_Swapchain.get() )) 
            {
                VY_THROW_RUNTIME_ERROR("Swap chain image (format, color space, or size) has changed, not handled yet!");
            }
        }
    }


// =========================================================================================================================
#pragma region [ Cmd Buffers ]
// =========================================================================================================================

    void VyRenderer::createCommandBuffers() 
    {
        m_CommandBuffers.resize( MAX_FRAMES_IN_FLIGHT );

		// Primary Command Buffers (VK_COMMAND_BUFFER_LEVEL_PRIMARY) can be submitted to a queue to then be executed by the GPU. 
        // However, it cannot be called by other command buffers.

        // Secondary Command Buffers (VK_COMMAND_BUFFER_LEVEL_SECONDARY) cannot be submitted to a queue for execution. 
        // But, it can be called by other command buffers.
        
        VkCommandBufferAllocateInfo allocInfo{ VKInit::commandBufferAllocateInfo() };
        {
            allocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            allocInfo.commandPool        = VyContext::device().commandPool();
            allocInfo.commandBufferCount = static_cast<U32>(m_CommandBuffers.size());
        }

        VK_CHECK(vkAllocateCommandBuffers(VyContext::device(), &allocInfo, m_CommandBuffers.data()));
    }

    // =====================================================================================================================

    void VyRenderer::freeCommandBuffers() 
    {
        vkFreeCommandBuffers(
            VyContext::device(),
            VyContext::device().commandPool(),
            static_cast<U32>(m_CommandBuffers.size()),
            m_CommandBuffers.data()
        );
        
        m_CommandBuffers.clear();
    }

#pragma endregion Cmd Buffers


// =========================================================================================================================
#pragma region [ Recording ]
// =========================================================================================================================

    // Start recording the current command buffer and check that the current frame buffer is still valid.
    VkCommandBuffer VyRenderer::beginFrame() 
    {
        VY_ASSERT(!m_IsFrameStarted, 
            "Can't call `beginFrame` while frame is already in progress.");

        // Get the index of the frame buffer to render to next.
        auto result = m_Swapchain->acquireNextImage( &m_CurrentImageIndex );
        
        // Recreate swapchain if window was resized.
        // VK_ERROR_OUT_OF_DATE_KHR occurs when the surface is no longer compatible with the swapchain (e.g. after window is resized).
        if (result == VK_ERROR_OUT_OF_DATE_KHR) 
        {
            VY_DEBUG_TAG("VyRenderer", "beginFrame: Swapchain resized");

            recreateSwapchain();

            return VK_NULL_HANDLE;
        }

        // VK_SUBOPTIMAL_KHR may be returned if the swapchain no longer matches the surface properties exactly (e.g. if the window was resized).
        VY_ASSERT(result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR, 
            "Failed to aquire swapchain image!");

        m_IsFrameStarted = true;
        
        auto cmdBuffer = currentCommandBuffer();
        
        // Start one-time command buffer recording.
        VkCommandBufferBeginInfo beginInfo{ VKInit::commandBufferBeginInfo() };
        {
            beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        }
        
		VK_CHECK(vkResetCommandBuffer(cmdBuffer, 0));
		VK_CHECK(vkBeginCommandBuffer(cmdBuffer, &beginInfo));
        
        return cmdBuffer;
    }

    // =====================================================================================================================

    // Stop recording the current command buffer and submit it to the render queue.
    void VyRenderer::endFrame() 
    {
        VY_ASSERT(m_IsFrameStarted, 
            "Can't call `endFrame` while frame is not in progress.");
        
        auto cmdBuffer = currentCommandBuffer();
        
        // Stop command buffer recording.
        if (vkEndCommandBuffer(cmdBuffer) != VK_SUCCESS) 
        {
            VY_THROW_RUNTIME_ERROR("Failed to record command buffer!");
        }

        // Submit command buffer.
		// Vulkan will execute the commands in this command buffer
		// to output that information to the selected frame buffer.
        auto result = m_Swapchain->submitCommandBuffers( &cmdBuffer, &m_CurrentImageIndex );

        // Check again if window was resized during command buffer recording / submitting and recreate swapchain if so.
        if (result == VK_ERROR_OUT_OF_DATE_KHR || 
            result == VK_SUBOPTIMAL_KHR        || 
            m_Window.wasWindowResized()) 
        {
            VY_DEBUG_TAG("VyRenderer", "endFrame: Swapchain resized");

            m_Window.resetWindowResizedFlag();
            
            recreateSwapchain();
        } 
        else if (result != VK_SUCCESS) 
        {
            VY_THROW_RUNTIME_ERROR("Failed to present swapchain image!");
        }

        m_IsFrameStarted = false;

        // Advance to the next frame.
        m_CurrentFrameIndex = (m_CurrentFrameIndex + 1) % MAX_FRAMES_IN_FLIGHT;

        // [ Flush Deletion Queue ]
        VyContext::flushDeletionQueue( m_CurrentFrameIndex );
    }

#pragma endregion Recording


// =========================================================================================================================
#pragma region [ SC Render Pass ]
// =========================================================================================================================

    // Start the render pass in order to then record commands to it.
    void VyRenderer::beginSwapchainRenderPass(VkCommandBuffer cmdBuffer) 
    {
        VY_ASSERT(m_IsFrameStarted,                    "Can't begin swapchain render pass when frame is not in progress.");
        VY_ASSERT(cmdBuffer == currentCommandBuffer(), "Can't begin swapchain render pass on command buffer from a different frame.");

        TArray<VkClearValue, 2> clearValues{};
        {
            clearValues[0].color        = { 0.1f, 0.1f, 0.1f, 1.0f };
            clearValues[1].depthStencil = { 1.0f, 0 };
        }

        // Begin render pass.
        VkRenderPassBeginInfo renderPassInfo{ VKInit::renderPassBeginInfo() };
        {
            renderPassInfo.renderPass        = m_Swapchain->renderPass().handle();

            // The frame buffer this render pass will write to.
            renderPassInfo.framebuffer       = m_Swapchain->frameBuffer(m_CurrentImageIndex).handle();
            
            // Defines the area in which the shader loads and stores will take place.
            renderPassInfo.renderArea.offset = { 0, 0 };

            // Specify the swap chain extent and not the window extent because for
            // high density displays (e.g. Apple's Retina displays), the size of the
            // window will not be 1:1 with the size of the swap chain.
            renderPassInfo.renderArea.extent = m_Swapchain->swapchainExtent();
            
            // Inital values for the frame buffer attatchments to be cleared to.
            // This corresponds to how we've structured our render pass: Index 0 = color attatchment, Index 1 = Depth Attatchment.
            renderPassInfo.clearValueCount   = static_cast<U32>(clearValues.size());
            renderPassInfo.pClearValues      = clearValues.data();
        }

		// Begin the render pass instance and start recording commands to that render pass. 
		
        // VK_SUBPASS_CONTENTS_INLINE: Commands to be executed will be embedded directly into this primary command buffer.
		
        // VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS: Commands to be executed will come from secondary command buffers. 
        // (Cannot mix command types and have a primary command buffer that has both inline commands and secondary command buffers.)

        vkCmdBeginRenderPass(cmdBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        // Set viewport and scissor rect.
        VKCmd::viewport(cmdBuffer, m_Swapchain->swapchainExtent());
        VKCmd::scissor (cmdBuffer, m_Swapchain->swapchainExtent());
    }

    // =====================================================================================================================

    // End the render pass when commands have been recorded.
    void VyRenderer::endSwapchainRenderPass(VkCommandBuffer cmdBuffer) const
    {
        VY_ASSERT(m_IsFrameStarted,                    "Can't end swapchain render pass when frame is not in progress.");
        VY_ASSERT(cmdBuffer == currentCommandBuffer(), "Can't end swapchain render pass on command buffer from a different frame.");
        
        vkCmdEndRenderPass(cmdBuffer);
    }

#pragma endregion SC Render Pass

}