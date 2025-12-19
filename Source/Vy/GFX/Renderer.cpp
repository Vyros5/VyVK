#include <Vy/GFX/Renderer.h>

#include <Vy/GFX/Context.h>
// namespace Vy
// {
//     // =====================================================================================================================

//     Renderer::Renderer(VyWindow& window) : 
//         m_Window            { window                          },
//         m_VyContext         { VyContext::initialize(m_Window) }
//     {
//         recreateSwapchain();
//         createCommandBuffers();
//     }


//     Renderer::~Renderer() 
//     { 
//         freeCommandBuffers(); 
//     }


// // =========================================================================================================================
// #pragma region [ Cmd Buffers ]
// // =========================================================================================================================

//     void Renderer::createCommandBuffers() 
//     {
//         m_CommandBuffers.resize( MAX_FRAMES_IN_FLIGHT );

// 		// Primary Command Buffers (VK_COMMAND_BUFFER_LEVEL_PRIMARY) can be submitted to a queue to then be executed by the GPU. 
//         // However, it cannot be called by other command buffers.

//         // Secondary Command Buffers (VK_COMMAND_BUFFER_LEVEL_SECONDARY) cannot be submitted to a queue for execution. 
//         // But, it can be called by other command buffers.
        
//         VkCommandBufferAllocateInfo allocInfo{ VKInit::commandBufferAllocateInfo() };
//         {
//             allocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
//             allocInfo.commandPool        = VyContext::device().commandPool();
//             allocInfo.commandBufferCount = static_cast<U32>(m_CommandBuffers.size());
//         }

//         VK_CHECK(vkAllocateCommandBuffers(VyContext::device(), &allocInfo, m_CommandBuffers.data()));
//     }

//     // =====================================================================================================================

//     void Renderer::freeCommandBuffers() 
//     {
//         vkFreeCommandBuffers(
//             VyContext::device(),
//             VyContext::device().commandPool(),
//             static_cast<U32>(m_CommandBuffers.size()),
//             m_CommandBuffers.data()
//         );
        
//         m_CommandBuffers.clear();
//     }

// #pragma endregion Cmd Buffers


// // =========================================================================================================================
// #pragma region [ Swapchain ]
// // =========================================================================================================================

//     void Renderer::recreateSwapchain() 
//     {
//         auto extent = m_Window.windowExtent();

//         // While the window is minimized...
//         while (extent.width == 0 || extent.height == 0) 
//         {
//             extent = m_Window.windowExtent();
            
//             // While one of the windows dimensions is 0 (e.g. during minimization), wait until otherwise.
//             // https://www.glfw.org/docs/3.3/group__window.html#ga554e37d781f0a997656c26b2c56c835e
//             glfwWaitEvents();
//         }

//         // Wait until the current swapchain is no longer being used before (re)creating it.
//         VyContext::waitIdle();

//         // First-Time swapchain creation
//         if (m_Swapchain == nullptr) 
//         {
//             // Create new swapchain with new extents.
//             m_Swapchain = MakeUnique<VySwapchain>(extent, m_DesiredPresentMode);
//         } 
//         else // Recreate swapchain.
//         {
//             Shared<VySwapchain> oldSwapchain = std::move(m_Swapchain);

//             // Create new swapchain with new extents and pass through the old swap chain if it exists.
//             m_Swapchain = MakeUnique<VySwapchain>(extent, oldSwapchain, m_DesiredPresentMode);

//             // Check if the old and new swap chains are compatible.
//             if (!oldSwapchain->compareSwapFormats( *m_Swapchain.get()) ) 
//             {
//                 VY_THROW_RUNTIME_ERROR("Swap chain image (format, color space, or size) has changed, not handled yet!");
//             }
//         }
//     }

// #pragma endregion Swapchain


// // =========================================================================================================================
// #pragma region [ Recording ]
// // =========================================================================================================================

//     // Start recording the current command buffer and check that the current frame buffer is still valid.
//     VkCommandBuffer Renderer::beginFrame() 
//     {
//         VY_ASSERT(!m_IsFrameStarted, "Can't call `beginFrame` while frame is already in progress.");

//         // Get the index of the frame buffer to render to next.
//         auto result = m_Swapchain->acquireNextImage(&m_CurrentImageIndex);
        
//         // Recreate swapchain if window was resized.
//         // VK_ERROR_OUT_OF_DATE_KHR occurs when the surface is no longer compatible with the swapchain (e.g. after window is resized).
//         if (result == VK_ERROR_OUT_OF_DATE_KHR) 
//         {
//             VY_DEBUG_TAG("VyRenderer", "beginFrame: Swapchain resized");

//             recreateSwapchain();

//             return VK_NULL_HANDLE;
//         }

//         // VK_SUBOPTIMAL_KHR may be returned if the swapchain no longer matches the surface properties exactly (e.g. if the window was resized).
//         VY_ASSERT(result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR, "Failed to aquire swapchain image!");

//         m_IsFrameStarted = true;
        
//         auto cmdBuffer = currentCommandBuffer();
        
//         // Start one-time command buffer recording.
//         VkCommandBufferBeginInfo beginInfo{ VKInit::commandBufferBeginInfo() };
//         {
//             beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
//         }
        
// 		VK_CHECK(vkResetCommandBuffer(cmdBuffer, 0));
// 		VK_CHECK(vkBeginCommandBuffer(cmdBuffer, &beginInfo));
        
//         return cmdBuffer;
//     }

//     // =====================================================================================================================

//     // Stop recording the current command buffer and submit it to the render queue.
//     void Renderer::endFrame() 
//     {
//         VY_ASSERT(m_IsFrameStarted, "Can't call `endFrame` while frame is not in progress.");
        
//         auto cmdBuffer = currentCommandBuffer();
        
//         // Stop command buffer recording.
//         if (vkEndCommandBuffer(cmdBuffer) != VK_SUCCESS) 
//         {
//             VY_THROW_RUNTIME_ERROR("Failed to record command buffer!");
//         }

//         // Submit command buffer.
// 		// Vulkan will execute the commands in this command buffer
// 		// to output that information to the selected frame buffer.
//         auto result = m_Swapchain->submitCommandBuffers(&cmdBuffer, &m_CurrentImageIndex);

//         // Check again if window was resized during command buffer recording / submitting and recreate swapchain if so.
//         if (result == VK_ERROR_OUT_OF_DATE_KHR || 
//             result == VK_SUBOPTIMAL_KHR        || 
//             m_Window.wasWindowResized()) 
//         {
//             VY_DEBUG_TAG("VyRenderer", "endFrame: Swapchain resized");

//             m_Window.resetWindowResizedFlag();
            
//             recreateSwapchain();
//         } 
//         else if (result != VK_SUCCESS) 
//         {
//             VY_THROW_RUNTIME_ERROR("Failed to present swapchain image!");
//         }

//         m_IsFrameStarted = false;

//         // Advance to the next frame.
//         m_CurrentFrameIndex = (m_CurrentFrameIndex + 1) % MAX_FRAMES_IN_FLIGHT;

//         // [ Flush Deletion Queue ]
//         VyContext::flushDeletionQueue(m_CurrentFrameIndex);
//     }

// #pragma endregion Recording


// // =========================================================================================================================
// #pragma region [ Render Pass ]
// // =========================================================================================================================

//     // Start the render pass in order to then record commands to it.
//     void Renderer::beginPresentRenderPass(VkCommandBuffer cmdBuffer) 
//     {
//         TVector<VkClearValue> clearValues{ 2 };
//         {
//             clearValues[0].color        = { 0.02f, 0.02f, 0.02f, 1.0f };
//             clearValues[1].depthStencil = { 1.0f, 0 };
//         }

//         VkRect2D scissor{};
//         {
//             scissor.offset = { 0, 0 };
//             scissor.extent = m_Swapchain->swapchainExtent();
//         }

// 		VkViewport viewport{};
//         {
//             viewport.x        = 0.0f;
//             viewport.y        = 0.0f;
//             viewport.width    = static_cast<float>(m_Swapchain->swapchainExtent().width);
//             viewport.height   = static_cast<float>(m_Swapchain->swapchainExtent().height);
//             viewport.minDepth = 0.0f;
//             viewport.maxDepth = 1.0f;
//         }

//         VyRenderInfo renderInfo{};
//         {
//             renderInfo.RenderPass = m_Swapchain->renderPass().handle();

//             // The frame buffer the render pass will write to.
//             renderInfo.Framebuffer = m_Swapchain->frameBuffer(m_CurrentImageIndex).handle();

//             // Specify the swap chain extent and not the window extent because for
//             // high density displays (e.g. Apple's Retina displays), the size of the
//             // window will not be 1:1 with the size of the swap chain.
//             renderInfo.Scissor  = scissor;
//             renderInfo.Viewport = viewport;

//             // Inital values for the frame buffer attatchments to be cleared to.
//             // This corresponds to how we've structured our render pass: Index 0 = color attatchment, Index 1 = Depth Attatchment.
//             renderInfo.ClearValues = clearValues;
//         }

//         // Begin render pass.
//         beginRenderPass(cmdBuffer, renderInfo);
//     }


//     void Renderer::beginRenderPass(VkCommandBuffer cmdBuffer, VyRenderInfo renderInfo)
//     {
//         VY_ASSERT(m_IsFrameStarted,                    "Can't begin render pass when frame is not in progress.");
//         VY_ASSERT(cmdBuffer == currentCommandBuffer(), "Can't begin render pass on command buffer from a different frame.");

// 		// 1st Command: Begin render pass.
// 		VkRenderPassBeginInfo renderPassInfo{ VKInit::renderPassBeginInfo() };
//         {
//             renderPassInfo.renderPass  = renderInfo.RenderPass;
//             renderPassInfo.framebuffer = renderInfo.Framebuffer;
            
//             // Defines the area in which the shader loads and stores will take place.
//             renderPassInfo.renderArea.offset = { 0, 0 };

//             // Here we specify the swap chain extent and not the window extent because for
//             // high density displays (e.g. Apple's Retina displays), the size of the
//             // window will not be 1:1 with the size of the swap chain.
//             renderPassInfo.renderArea.extent = {static_cast<U32>(renderInfo.Viewport.width), static_cast<U32>(renderInfo.Viewport.height)};
            
//             // What inital values we want our frame buffer attatchments to be cleared to.
//             // This corresponds to how we've structured our render pass: Index 0 = color
//             // attatchment, Index 1 = Depth Attatchment.
//             renderPassInfo.clearValueCount = static_cast<U32>(renderInfo.ClearValues.size());
//             renderPassInfo.pClearValues    = renderInfo.ClearValues.data();
//         }
            
//         // Begin the render pass instance and start recording commands to that render
//         // pass. VK_SUBPASS_CONTENTS_INLINE signifies that all the commands we want to
//         // execute will be embedded directly into this primary command buffer.
//         // VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS signifies that all the
// 		// commands we want to execute will come from secondary command buffers. This
// 		// means we cannot mix command types and have a primary command buffer that
// 		// has both inline commands and secondary command buffers.
// 		vkCmdBeginRenderPass(cmdBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

// 		vkCmdSetViewport(cmdBuffer, 0, 1, &renderInfo.Viewport);
// 		vkCmdSetScissor (cmdBuffer, 0, 1, &renderInfo.Scissor);
//     }
    

//     // End the render pass when commands have been recorded.
//     void Renderer::endRenderPass(VkCommandBuffer cmdBuffer) const
//     {
//         VY_ASSERT(m_IsFrameStarted,                    "Can't end render pass when frame is not in progress.");
//         VY_ASSERT(cmdBuffer == currentCommandBuffer(), "Can't end render pass on command buffer from a different frame.");
        
//         vkCmdEndRenderPass(cmdBuffer);
//     }

// #pragma endregion Render Pass

// }



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
            clearValues[0].color        = { 0.02f, 0.02f, 0.02f, 1.0f };
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


// =========================================================================================================================
#pragma region [ Render Pass ]
// =========================================================================================================================

    // void VyRenderer::beginShadowSwapchainRenderPass(VkCommandBuffer cmdBuffer) 
    // {
    //     VY_ASSERT(m_IsFrameStarted,                    "Can't begin shadow swapchain render pass when frame is not in progress.");
    //     VY_ASSERT(cmdBuffer == currentCommandBuffer(), "Can't begin shadow swapchain render pass on command buffer from a different frame.");

    //     TArray<VkClearValue, 2> clearValues{};
    //     {
    //         clearValues[0].color        = { 0.02f, 0.02f, 0.02f, 1.0f };
    //         clearValues[1].depthStencil = { 1.0f, 0 };
    //     }

    //     // Begin render pass.
    //     VkRenderPassBeginInfo renderPassInfo{ VKInit::renderPassBeginInfo() };
    //     {
    //         renderPassInfo.renderPass        = m_Swapchain->shadowRenderPass().handle();

    //         // The frame buffer this render pass will write to.
    //         renderPassInfo.framebuffer       = m_Swapchain->shadowMapFramebuffer().handle();
            
    //         // Defines the area in which the shader loads and stores will take place.
    //         renderPassInfo.renderArea.offset = { 0, 0 };

    //         renderPassInfo.renderArea.extent = m_Swapchain->shadowMapExtent();

    //         renderPassInfo.clearValueCount   = static_cast<U32>(clearValues.size());
    //         renderPassInfo.pClearValues      = clearValues.data();
    //     }


    //     vkCmdBeginRenderPass(cmdBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    //     // Set viewport and scissor rect.
    //     auto shadowMapExtent = m_Swapchain->shadowMapExtent();

    //     VkViewport viewport{};
    //     {
    //         viewport.x        = 0.0f;
    //         viewport.y        = static_cast<float>(shadowMapExtent.height);
    //         viewport.y        = 0;
    //         viewport.width    = static_cast<float>(shadowMapExtent.width );
    //         viewport.height   = static_cast<float>(shadowMapExtent.height);
    //         viewport.minDepth = 0.0f;
    //         viewport.maxDepth = 1.0f;
    //     }
        
    //     VkRect2D scissor{};
    //     {
    //         scissor.offset = { 0, 0 };
    //         scissor.extent = shadowMapExtent;
    //     }

    //     vkCmdSetViewport(cmdBuffer, 0, 1, &viewport);
    //     vkCmdSetScissor (cmdBuffer, 0, 1, &scissor );
    // }



    // void VyRenderer::createOffscreenResources()
    // {
    //     m_OffscreenFramebuffer = std::make_unique<VyHZBFramebuffer>(m_Swapchain->swapchainExtent(), MAX_FRAMES_IN_FLIGHT, true);
    // }


    // void VyRenderer::beginOffscreenRenderPass(VkCommandBuffer cmdBuffer)
    // {
    //     VY_ASSERT(m_IsFrameStarted,                    "Can't begin offscreen render pass when frame is not in progress.");
    //     VY_ASSERT(cmdBuffer == currentCommandBuffer(), "Can't begin offscreen render pass on command buffer from a different frame.");

    //     m_OffscreenFramebuffer->beginRenderPass(cmdBuffer, m_CurrentFrameIndex);

    //     // Set viewport and scissor rect.
    //     VKCmd::viewport(cmdBuffer, m_Swapchain->swapchainExtent());
    //     VKCmd::scissor (cmdBuffer, m_Swapchain->swapchainExtent());
    // }


    // void VyRenderer::endOffscreenRenderPass(VkCommandBuffer cmdBuffer) const
    // {
    //     VY_ASSERT(m_IsFrameStarted,                    "Can't end offscreen render pass when frame is not in progress.");
    //     VY_ASSERT(cmdBuffer == currentCommandBuffer(), "Can't end offscreen render pass on command buffer from a different frame.");

    //     m_OffscreenFramebuffer->endRenderPass(cmdBuffer);
    // }


    // VkDescriptorImageInfo VyRenderer::offscreenDescriptorImageInfo(int index) const
    // {
    //     return m_OffscreenFramebuffer->descriptorImageInfo(index);
    // }


    // VkDescriptorImageInfo VyRenderer::depthDescriptorImageInfo(int index) const
    // {
    //     VkDescriptorImageInfo info{};
    //     {
    //         info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    //         info.imageView   = m_OffscreenFramebuffer->depthImageView(index);
    //         info.sampler     = m_OffscreenFramebuffer->depthSampler();
    //     }

    //     return info;
    // }


    // void VyRenderer::generateOffscreenMipmaps(VkCommandBuffer cmdBuffer)
    // {
    //     m_OffscreenFramebuffer->generateMipmaps(cmdBuffer, m_CurrentFrameIndex);
    // }


    // void VyRenderer::createHZBPipeline()
    // {
    //     // 1. Create Descriptor Set Layout
    //     if (m_HZBSetLayout == VK_NULL_HANDLE)
    //     {
    //         m_HZBSetLayout = VyDescriptorSetLayout::Builder{}
    //             // Binding 0: Input Depth (Sampler)
    //             .addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_COMPUTE_BIT)

    //             // Binding 1: Output Depth (Storage Image)
    //             .addBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,          VK_SHADER_STAGE_COMPUTE_BIT)
    //         .buildUnique();
    //     }

    //     // 2. Create Compute Pipeline
    //     if (m_HZBPipeline == VK_NULL_HANDLE)
    //     {
    //         m_HZBPipeline = VyPipeline::ComputeBuilder{}
    //             .addDescriptorSetLayout(*m_HZBSetLayout)
    //             .setShaderStage("HIZGenerate.comp.spv")
    //         .buildUnique();
    //     }

    //     // 3. Create Descriptor Pool and Sets
    //     // We need sets for each frame and each mip transition.
    //     // Mip levels can be retrieved from m_OffscreenFramebuffer (assuming it's used for depth)
    //     // Wait, we need to know which framebuffer has the depth.
    //     // Assuming m_OffscreenFramebuffer is used.
    //     if (!m_OffscreenFramebuffer) return;

    //     // Get mip levels from framebuffer (we need to expose it or calculate it)
    //     // FrameBuffer calculates mipLevels based on extent.
    //     VkExtent2D extent    = m_Swapchain->swapchainExtent();
    //     U32        mipLevels = static_cast<U32>(std::floor(std::log2(std::max(extent.width, extent.height)))) + 1;

    //     // Need sets for (mipLevels - 1) transitions per frame.
    //     U32 setsPerFrame = mipLevels - 1;

    //     if (setsPerFrame == 0) return;

    //     U32 totalSets = setsPerFrame * MAX_FRAMES_IN_FLIGHT;

    //     VY_INFO_TAG("VyRenderer", "HZB Setup: MipLevels< {0} >, SetsPerFrame< {1} >, TotalSets< {2} >", mipLevels, setsPerFrame, totalSets);

    //     // HZB Descriptor Pool
    //     if (m_HZBDescriptorPool == VK_NULL_HANDLE)
    //     {
    //         m_HZBDescriptorPool = VyDescriptorPool::Builder{}
    //             .setMaxSets (totalSets)
    //             .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, totalSets)
    //             .addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,          totalSets)
    //         .buildUnique();
    //     }

    //     // Allocate and Update Sets.
    //     m_HZBDescriptorSets.resize( MAX_FRAMES_IN_FLIGHT );

    //     for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    //     {
    //         m_HZBDescriptorSets[i].resize( setsPerFrame );

    //         TVector<VkDescriptorSetLayout> layouts( setsPerFrame, *m_HZBSetLayout );

    //         VkDescriptorSetAllocateInfo allocInfo{ VKInit::descriptorSetAllocateInfo() };
    //         {
    //             allocInfo.descriptorPool     = m_HZBDescriptorPool->handle();
    //             allocInfo.descriptorSetCount = setsPerFrame;
    //             allocInfo.pSetLayouts        = layouts.data();
    //         }

    //         VkResult allocResult = vkAllocateDescriptorSets(VyContext::device(), &allocInfo, m_HZBDescriptorSets[i].data());
    //         if (allocResult != VK_SUCCESS)
    //         {
    //             VY_THROW_RUNTIME_ERROR("Failed to allocate HZB descriptor sets! Error: " + std::to_string(allocResult));
    //         }

    //         // Update sets for each mip transition: Input Mip k -> Output Mip k+1
    //         for (U32 m = 0; m < setsPerFrame; m++)
    //         {
    //             VkImageView inputView  = m_OffscreenFramebuffer->depthMipImageView(i, m);
    //             VkImageView outputView = m_OffscreenFramebuffer->depthMipImageView(i, m + 1);

    //             if (inputView == VK_NULL_HANDLE || outputView == VK_NULL_HANDLE)
    //             {
    //                 VY_ERROR_TAG("VyRenderer", "Null View Handle! Frame {0}, Mip {1} -> {2}", i, m, (m + 1));
    //             }

    //             // Input: Mip m
    //             VkDescriptorImageInfo inputInfo{};
    //             {
    //                 inputInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; // We will transition to this
    //                 inputInfo.imageView   = inputView;
    //                 inputInfo.sampler     = m_OffscreenFramebuffer->depthSampler();
    //             }

    //             // Output: Mip m+1
    //             VkDescriptorImageInfo outputInfo{};
    //             {
    //                 outputInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL; // Storage image needs GENERAL
    //                 outputInfo.imageView   = outputView;
    //                 outputInfo.sampler     = VK_NULL_HANDLE;
    //             }

    //             VY_INFO_TAG("VyRenderer", "Updating HZB Set: Frame {0}, Mip {1}", i, m);

    //             VyDescriptorWriter{ *m_HZBSetLayout, *m_HZBDescriptorPool }
    //                 .writeImage(0, &inputInfo )
    //                 .writeImage(1, &outputInfo)
    //             .update(m_HZBDescriptorSets[i][m]);
    //         }
    //     }
    // }


    // void VyRenderer::generateDepthPyramid(VkCommandBuffer cmdBuffer)
    // {
    //     if (!m_OffscreenFramebuffer) return;

    //     VkExtent2D extent    = m_Swapchain->swapchainExtent();
    //     U32        mipLevels = static_cast<U32>(std::floor(std::log2(std::max(extent.width, extent.height)))) + 1;
        
    //     if (mipLevels < 2) return;

    //     // Transition Mip 0 to SHADER_READ_ONLY_OPTIMAL (it was DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
    //     // And all other mips to GENERAL (for writing)

    //     VkImageMemoryBarrier barrier{ VKInit::imageMemoryBarrier() };
    //     {
    //         barrier.image                           = m_OffscreenFramebuffer->depthImage(m_CurrentFrameIndex);
    //         barrier.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
    //         barrier.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;

    //         barrier.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_DEPTH_BIT;
    //         barrier.subresourceRange.baseArrayLayer = 0;
    //         barrier.subresourceRange.layerCount     = 1;
    //     }

    //     // 1. Transition Mip 0: Write -> Read
    //     {
    //         barrier.subresourceRange.baseMipLevel = 0;
    //         barrier.subresourceRange.levelCount   = 1;

    //         barrier.oldLayout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    //         barrier.newLayout     = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            
    //         barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    //         barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    //     }

    //     vkCmdPipelineBarrier(cmdBuffer,
    //         VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
    //         VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0,
    //         0, nullptr,
    //         0, nullptr,
    //         1, &barrier
    //     );

    //     // 2. Transition Mips 1..N: Undefined -> General (for writing)
    //     {
    //         barrier.subresourceRange.baseMipLevel = 1;
    //         barrier.subresourceRange.levelCount   = mipLevels - 1;

    //         barrier.oldLayout     = VK_IMAGE_LAYOUT_UNDEFINED;
    //         barrier.newLayout     = VK_IMAGE_LAYOUT_GENERAL;
            
    //         barrier.srcAccessMask = 0;
    //         barrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
    //     }

    //     vkCmdPipelineBarrier(cmdBuffer, 
    //         VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 
    //         VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 
    //         0, nullptr, 
    //         0, nullptr, 
    //         1, &barrier
    //     );

    //     m_HZBPipeline->bind(cmdBuffer);

    //     I32 mipWidth  = extent.width;
    //     I32 mipHeight = extent.height;

    //     for (U32 i = 0; i < mipLevels - 1; i++)
    //     {
    //         // Dispatch for Mip i -> Mip i+1
    //         // Output size is Mip i+1 size
    //         mipWidth  = mipWidth > 1 ? mipWidth / 2 : 1;
    //         mipHeight = mipHeight > 1 ? mipHeight / 2 : 1;

    //         m_HZBPipeline->bindDescriptorSet(cmdBuffer, 
    //             0, 
    //             m_HZBDescriptorSets[m_CurrentFrameIndex][i]
    //         );

    //         // Group size is 32x32
    //         vkCmdDispatch(cmdBuffer, (mipWidth + 31) / 32, (mipHeight + 31) / 32, 1);

    //         // Barrier: Wait for Mip i+1 write to finish before it becomes Mip i for next iteration
    //         // Transition Mip i+1 from GENERAL (Write) to SHADER_READ_ONLY_OPTIMAL (Read)

    //         VkImageMemoryBarrier mipBarrier{ VKInit::imageMemoryBarrier() };
    //         {
    //             mipBarrier.image = m_OffscreenFramebuffer->depthImage(m_CurrentFrameIndex);

    //             mipBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    //             mipBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

    //             mipBarrier.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_DEPTH_BIT;
    //             mipBarrier.subresourceRange.baseArrayLayer = 0;
    //             mipBarrier.subresourceRange.layerCount     = 1;
    //             mipBarrier.subresourceRange.baseMipLevel   = i + 1;
    //             mipBarrier.subresourceRange.levelCount     = 1;
                
    //             mipBarrier.oldLayout     = VK_IMAGE_LAYOUT_GENERAL;
    //             mipBarrier.newLayout     = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                
    //             mipBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
    //             mipBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    //         }

    //         vkCmdPipelineBarrier(cmdBuffer,
    //             VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
    //             VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0,
    //             0, nullptr,
    //             0, nullptr,
    //             1, &mipBarrier
    //         );
    //     }

    //     // Finally, transition Mip 0 back to DEPTH_STENCIL_ATTACHMENT_OPTIMAL?
    //     // Or leave it as READ_ONLY if we are done with it for this frame?
    //     // If we use it for next frame's culling, READ_ONLY is fine.
    //     // But at the start of next frame, render pass will transition it to ATTACHMENT_OPTIMAL (loadOp=CLEAR).
    //     // So we are good.
    // }

    // https://github.com/TNtube/Cardia/blob/main/Cardia/src/Cardia/Renderer/Renderer.cpp

	// void VyRenderer::beginRenderPass(const VyFramebuffer& frameBuffer, const VyRenderPass& renderPass) const
	// {
    //     VY_ASSERT(m_IsFrameStarted, "Can't begin render pass when frame is not in progress.");

    //     auto cmdBuffer = currentCommandBuffer();

    //     TArray<VkClearValue, 2> clearValues{};
    //     {
    //         clearValues[0].color        = { 0.02f, 0.02f, 0.02f, 1.0f };
    //         clearValues[1].depthStencil = { 1.0f, 0 };
    //     }

	// 	VkRenderPassBeginInfo renderPassInfo{ VKInit::renderPassBeginInfo() };
    //     {
    //         renderPassInfo.renderPass        = renderPass .handle();
    //         renderPassInfo.framebuffer       = frameBuffer.handle();
            
    //         renderPassInfo.renderArea.offset = { 0, 0 };
    //         renderPassInfo.renderArea.extent = m_Swapchain->swapchainExtent();
            
    //         renderPassInfo.clearValueCount   = static_cast<U32>(clearValues.size());
    //         renderPassInfo.pClearValues      = clearValues.data();
    //     }

	// 	vkCmdBeginRenderPass(cmdBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	// 	const auto extent = m_Swapchain->swapchainExtent();

    //     VkViewport viewport{};
    //     {
    //         viewport.x        = 0.0f;
    //         viewport.y        = 0.0f;
    //         viewport.width    = static_cast<float>(extent.width );
    //         viewport.height   = static_cast<float>(extent.height);
    //         viewport.minDepth = 0.0f;
    //         viewport.maxDepth = 1.0f;
    //     }

    //     VkRect2D scissor{};
    //     {
    //         scissor.offset = { 0, 0 };
    //         scissor.extent = extent;
    //     }

    //     vkCmdSetViewport(cmdBuffer, 0, 1, &viewport);
    //     vkCmdSetScissor (cmdBuffer, 0, 1, &scissor );
	// }

#pragma endregion Render Pass
}