#include <VyEngine/VK/Renderer/Renderer.h>

#include <VyEngine/VK/Context.h>

namespace Vy
{
// =========================================================================================================================
#pragma region [ VyRenderer ]
// =========================================================================================================================

    VyRenderer::VyRenderer(VyWindow& window) : 
        m_Window   { window                            },
        m_VyContext{ VyContext::initialize( m_Window ) }
    {
        recreateSwapchain();
        createCommandBuffers();
    
		//Deferred Rendering
		prepareDeferredRenderFramebuffer();
    }

    // =====================================================================================================================

    VyRenderer::~VyRenderer() 
    { 
        freeCommandBuffers();
    }

#pragma endregion VyRenderer


// =========================================================================================================================
#pragma region [ Recording ]
// =========================================================================================================================

    // Start recording the current command buffer and check that the current frame buffer is still valid.
    VkCommandBuffer VyRenderer::beginFrame() 
    {
        VY_ASSERT(!m_IsFrameStarted, "Can't call `beginFrame` while frame is already in progress.");

        m_SwapchainRecreated = false;

        // Get the index of the frame buffer to render to next.
        auto result = m_Swapchain->acquireNextImage( &m_CurrentImageIndex );
        
        // Recreate swapchain if window was resized.
        // VK_ERROR_OUT_OF_DATE_KHR occurs when the surface is no longer compatible with the swapchain (e.g. after window is resized).
        if (result == VK_ERROR_OUT_OF_DATE_KHR) 
        {
            VY_TRACE_TAG("VyRenderer", "beginFrame: Swapchain resized");

            recreateSwapchain();

            return VK_NULL_HANDLE;
        }

        // VK_SUBOPTIMAL_KHR may be returned if the swapchain no longer matches 
        // the surface properties exactly (e.g. if the window was resized).
        VY_ASSERT(result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR, 
            "Failed to aquire swapchain image!");

        m_IsFrameStarted = true;

        auto cmdBuffer = m_CommandBuffers[ m_CurrentFrameIndex ];
        
        // VK_CHECK_SUCCESS(vkResetCommandBuffer(cmdBuffer, 0), 
        //     "Failed to reset command buffer!");

        // Record draw commands to each command buffers.
        VkCommandBufferBeginInfo beginInfo{ VKInit::commandBufferBeginInfo() };
        {
            // beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT; //VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        }
        
		VK_CHECK_SUCCESS(vkBeginCommandBuffer(cmdBuffer, &beginInfo), 
            "Failed to begin recording command buffer!");
        
        return cmdBuffer;
    }


    // =====================================================================================================================

    // Stop recording the current command buffer and submit it to the render queue.
    void VyRenderer::endFrame() 
    {
        VY_ASSERT(m_IsFrameStarted, "Can't call `endFrame` while frame is not in progress.");
        
        auto cmdBuffer = this->currentCommandBuffer();
        
        // Stop command buffer recording.
        VK_CHECK_SUCCESS(vkEndCommandBuffer(cmdBuffer), 
            "Failed to record command buffer!");

        // Submit command buffer.
		// Vulkan will execute the commands in this command buffer to output that information to the selected framebuffer.
        auto result = m_Swapchain->submitCommandBuffers( &cmdBuffer, &m_CurrentImageIndex );

        // Check again if window was resized during command buffer recording / submitting and recreate swapchain if so.
        if (result == VK_ERROR_OUT_OF_DATE_KHR || 
            result == VK_SUBOPTIMAL_KHR        || 
            m_Window.wasWindowResized()) 
        {
            VY_TRACE_TAG("VyRenderer", "endFrame: Swapchain resized");

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

        // [ Flush Deletion Queue ] (*IMPORTANT*)
        VyContext::flushDeletionQueue( m_CurrentFrameIndex );
    }

#pragma endregion Recording



// =========================================================================================================================
#pragma region [ Swapcahin ]
// =========================================================================================================================

    void VyRenderer::recreateSwapchain() 
    {
        auto extent = m_Window.windowExtent();

        // While the window is minimized...
        while (extent.width == 0 || extent.height == 0) 
        {
            extent = m_Window.windowExtent();
            
            // While one of the windows dimensions is 0 (e.g. during minimization), wait until otherwise.
            m_Window.waitEvents();
        }

        // Wait until the current swapchain is no longer being used before (re)creating it.
        VyContext::waitIdle();

        // First-Time swapchain creation
        if (m_Swapchain == nullptr) 
        {
            // Create new swapchain with new extents.
            m_Swapchain = MakeUnique<VySwapchain>( extent );
        } 
        else // Recreate swapchain.
        {
            Shared<VySwapchain> oldSwapchain = std::move( m_Swapchain );

            // Create new swapchain with new extents and pass through the old swapchain if it exists.
            m_Swapchain = MakeUnique<VySwapchain>( extent, oldSwapchain );

            // Check if the old and new swap chains are compatible.
            if (!oldSwapchain->compareSwapFormats( *m_Swapchain.get() )) 
            {
                VY_THROW_RUNTIME_ERROR("Swapchain image (format, color space, or size) has changed, not handled yet!");
            }
        }

        m_SwapchainRecreated = true;
    }

#pragma endregion Swapcahin


// =========================================================================================================================
#pragma region [ Cmd Buffers ]
// =========================================================================================================================

    void VyRenderer::createCommandBuffers() 
    {
        // Resize command buffer count to have one for each framebuffer.
        m_CommandBuffers.resize( MAX_FRAMES_IN_FLIGHT );

        VkCommandBufferAllocateInfo allocInfo{ VKInit::commandBufferAllocateInfo() };
        {
            allocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            allocInfo.commandPool        = VyContext::device().graphicsCommandPool();
            allocInfo.commandBufferCount = static_cast<U32>(m_CommandBuffers.size());
        }

        VK_CHECK_SUCCESS(vkAllocateCommandBuffers(VyContext::device(), &allocInfo, m_CommandBuffers.data()),
            "Failed to allocate command buffers!"
        );

        for (U32 i = 0; i < m_CommandBuffers.size(); i++)
        {
            TString name = std::format("frame_cmd_buffer_{}", i);

            VKDbg::setObjectName(VK_OBJECT_TYPE_COMMAND_BUFFER, (U64)m_CommandBuffers[ i ], name.c_str());
        }
    }

    // =====================================================================================================================

    void VyRenderer::freeCommandBuffers() 
    {
        if (m_CommandBuffers.empty()) 
        { 
            return; 
        }

        vkFreeCommandBuffers(VyContext::device(),
            VyContext::device().graphicsCommandPool(),
            static_cast<U32>(m_CommandBuffers.size()),
            m_CommandBuffers.data()
        );
        
        m_CommandBuffers.clear();
    }

#pragma endregion Cmd Buffers


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
            clearValues[ 0 ].color        = { 0.01f, 0.01f, 0.01f, 1.0f };
            clearValues[ 1 ].depthStencil = { 1.0f, 0 };
        }

        // Begin render pass.
        VkRenderPassBeginInfo renderPassInfo{ VKInit::renderPassBeginInfo() };
        {
            renderPassInfo.renderPass        = m_Swapchain->renderPass();

            // The frame buffer the render pass will write to.
            renderPassInfo.framebuffer       = m_Swapchain->frameBuffer( m_CurrentImageIndex );
            
            // Defines the area in which the shader loads and stores will take place.
            renderPassInfo.renderArea.offset = { 0, 0 };

            // Specify the swapchain extent and not the window extent because for high density displays, 
            // the size of the window will not be 1:1 with the size of the swapchain.
            renderPassInfo.renderArea.extent = m_Swapchain->swapchainExtent();
            
            // Inital values for the frame buffer attatchments to be cleared to.
            // This corresponds to how we've structured our render pass: Index 0 = color attatchment, Index 1 = Depth Attatchment.
            renderPassInfo.clearValueCount   = static_cast<U32>(clearValues.size());
            renderPassInfo.pClearValues      = clearValues.data();
        }
		
		// Begin the render pass instance and start recording commands to that render pass. 
        vkCmdBeginRenderPass(cmdBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        // Set viewport and scissor rect.
        VKCmd::viewport( cmdBuffer, m_Swapchain->swapchainExtent() );
        VKCmd::scissor ( cmdBuffer, m_Swapchain->swapchainExtent() );
    }

    // =====================================================================================================================

    // End the render pass when commands have been recorded.
    void VyRenderer::endCurrentRenderPass(VkCommandBuffer cmdBuffer) const
    {
        VY_ASSERT(m_IsFrameStarted,                    "Can't end the current render pass when frame is not in progress.");
        VY_ASSERT(cmdBuffer == currentCommandBuffer(), "Can't end the current render pass on command buffer from a different frame.");
        
        vkCmdEndRenderPass(cmdBuffer);
    }

#pragma endregion SC Render Pass


// =========================================================================================================================
#pragma region [ Rendering ]
// =========================================================================================================================
    

    void VyRenderer::beginOffscreenRenderPass(VkCommandBuffer cmdBuffer)
    {
        VY_ASSERT(m_IsFrameStarted,                    "Can't begin offscreen render pass when frame is not in progress.");
        VY_ASSERT(cmdBuffer == currentCommandBuffer(), "Can't begin offscreen render pass on command buffer from a different frame.");

        TArray<VkClearValue, 2> clearValues{};
        {
            clearValues[ 0 ].color        = { 0.0f, 0.0f, 0.0f, 1.0f }; // color attachment
            clearValues[ 1 ].depthStencil = { 1.0f, 0 };                // Depths stencil clear value
        }
        
		// Record draw commands to each command buffers
		VkRenderPassBeginInfo renderPassInfo{ VKInit::renderPassBeginInfo() };
        {
            renderPassInfo.renderPass        = m_OffscreenPass.RenderPass;
            renderPassInfo.framebuffer       = m_OffscreenPass.Framebuffer;
            
            renderPassInfo.renderArea.offset = { 0,0 };
            // Make sure to use the swapchain extent not the window extent
            // because the swapchain extent may be larger then window extent which is the case in Mac retina display
            renderPassInfo.renderArea.extent = m_OffscreenPass.Extent;
            
            // Set the color that the frame buffer 'attachments' will clear to 
            renderPassInfo.clearValueCount   = static_cast<U32>(clearValues.size());
            renderPassInfo.pClearValues      = clearValues.data();
        }

		vkCmdBeginRenderPass(cmdBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        // Set viewport and scissor rect.
        VKCmd::viewport(cmdBuffer, m_OffscreenPass.Extent);
        VKCmd::scissor (cmdBuffer, m_OffscreenPass.Extent);
    }


	void VyRenderer::setUpOffscreenRenderPass(U32 texW, U32 texH)
	{
		createOffscreenRenderPass( texW, texH );
		createOffscreenFramebuffer();
	}


	void VyRenderer::createOffscreenRenderPass(U32 texW, U32 texH)
	{
		m_OffscreenPass.Extent = { texW, texH };

		// Depth stencil attachment
		VkFormat fbDepthFormat;
		bool bValidDepthFormat = VyContext::device().getSupportedDepthsFormat( &fbDepthFormat );
		
        assert(bValidDepthFormat);

		createOffscreenColorAttachment();
		createOffscreenDepthsAttachment( fbDepthFormat );

		TArray<VkAttachmentDescription, 2> attchmentDescriptions{};
		createOffscreenAttachmentDescriptors( attchmentDescriptions, fbDepthFormat );

		VkAttachmentReference colorReference = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL         };
		VkAttachmentReference depthReference = { 1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };

		VkSubpassDescription subpass{};
        {
            subpass.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
            subpass.colorAttachmentCount    = 1;
            subpass.pColorAttachments       = &colorReference;
            subpass.pDepthStencilAttachment = &depthReference;
        }

		TArray<VkSubpassDependency, 2> dependencies{};
		createOffscreenSubpassDependencies( dependencies );
		
		// Create the actual renderpass
		VkRenderPassCreateInfo renderPassInfo{ VKInit::renderPassCreateInfo() };
        {
            renderPassInfo.attachmentCount = static_cast<U32>(attchmentDescriptions.size());
            renderPassInfo.pAttachments    = attchmentDescriptions.data();

            renderPassInfo.subpassCount    = 1;
            renderPassInfo.pSubpasses      = &subpass;

            renderPassInfo.dependencyCount = static_cast<U32>(dependencies.size());
            renderPassInfo.pDependencies   = dependencies.data();
        }

		VK_CHECK(vkCreateRenderPass(VyContext::device(), &renderPassInfo, nullptr, &m_OffscreenPass.RenderPass));
	}


    void VyRenderer::createOffscreenColorAttachment()
	{
        m_OffscreenPass.Color.Image = VyImage::Builder{}
			.setName       ("offscreen_color_attachment")
            .setImageType  (VK_IMAGE_TYPE_2D)
            .setFormat     (VK_FORMAT_R8G8B8A8_UNORM)
            .setExtent     (m_OffscreenPass.Extent)
            .setLevels     (1)
			.setLayers     (1)
			.setSamples    (VK_SAMPLE_COUNT_1_BIT)
            .setTiling     (VK_IMAGE_TILING_OPTIMAL)
			.setLayout     (VK_IMAGE_LAYOUT_UNDEFINED)
            .setUsage      (VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
            .setMemoryUsage(VMA_MEMORY_USAGE_AUTO)
        	.build();

        m_OffscreenPass.Color.View = VyImageView::Builder{}
            .setName    ("offscreen_color_attachment")
            .setViewType(VK_IMAGE_VIEW_TYPE_2D)
            .setFormat  (VK_FORMAT_R8G8B8A8_UNORM)
            .setAspect  (VK_IMAGE_ASPECT_COLOR_BIT)
            .setLevels  (0, 1)
            .setLayers  (0, 1)
        	.build( m_OffscreenPass.Color.Image );

        // Create sampler to sample from the attachment in the fragment shader
        m_OffscreenPass.Sampler = VySampler::Builder{}
            .setName         ("offscreen_color_attachment")
            .setFilters      (VK_FILTER_LINEAR)
            .setMipmapMode   (VK_SAMPLER_MIPMAP_MODE_LINEAR)
            .setWrap         (VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE)
            .setBorder       (VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE)
            .setLodRange     (0.0f, 1.0f)
            .setMipLodBias   (0.0f)
        	.build();
	}


	void VyRenderer::createOffscreenDepthsAttachment(VkFormat& depthsFormat)
	{
        m_OffscreenPass.Depth.Image = VyImage::Builder{}
			.setName       ("offscreen_depth_attachment")
            .setImageType  (VK_IMAGE_TYPE_2D)
            .setFormat     (depthsFormat)
            .setExtent     (m_OffscreenPass.Extent)
            .setLevels     (1)
			.setLayers     (1)
			.setSamples    (VK_SAMPLE_COUNT_1_BIT)
            .setTiling     (VK_IMAGE_TILING_OPTIMAL)
			.setLayout     (VK_IMAGE_LAYOUT_UNDEFINED)
            .setUsage      (VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
            .setMemoryUsage(VMA_MEMORY_USAGE_AUTO)
        	.build();

        auto viewBuilder = VyImageView::Builder{}
            .setName    ("offscreen_depth_attachment")
            .setViewType(VK_IMAGE_VIEW_TYPE_2D)
            .setFormat  (depthsFormat)
            .setAspect  (VK_IMAGE_ASPECT_DEPTH_BIT)
            .setLevels  (0, 1)
            .setLayers  (0, 1);
        	
		if (depthsFormat >= VK_FORMAT_D16_UNORM_S8_UINT) 
        {
            viewBuilder.addAspect(VK_IMAGE_ASPECT_STENCIL_BIT);
		}

        m_OffscreenPass.Depth.View = viewBuilder.build( m_OffscreenPass.Depth.Image );
	}


	void VyRenderer::createOffscreenAttachmentDescriptors(TArray<VkAttachmentDescription, 2>& descriptors, VkFormat& depthsFormat)
	{
		// Color attachment
        {
            descriptors[ 0 ].format         = VkFormat::VK_FORMAT_R8G8B8A8_UNORM;
            descriptors[ 0 ].samples        = VK_SAMPLE_COUNT_1_BIT;
            descriptors[ 0 ].loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
            descriptors[ 0 ].storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
            descriptors[ 0 ].stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            descriptors[ 0 ].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            descriptors[ 0 ].initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
            descriptors[ 0 ].finalLayout    = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        }

        // Depth attachment
        {
            descriptors[ 1 ].format         = depthsFormat;
            descriptors[ 1 ].samples        = VK_SAMPLE_COUNT_1_BIT;
            descriptors[ 1 ].loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
            descriptors[ 1 ].storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            descriptors[ 1 ].stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            descriptors[ 1 ].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            descriptors[ 1 ].initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
            descriptors[ 1 ].finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        }

		VkAttachmentReference colorReference = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL         };
		VkAttachmentReference depthReference = { 1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };
	}


	void VyRenderer::createOffscreenSubpassDependencies(TArray<VkSubpassDependency, 2>& dependencies)
	{
        {
            dependencies[ 0 ].srcSubpass      = VK_SUBPASS_EXTERNAL;
            dependencies[ 0 ].dstSubpass      = 0;

            dependencies[ 0 ].srcStageMask    = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            dependencies[ 0 ].dstStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            
            dependencies[ 0 ].srcAccessMask   = VK_ACCESS_SHADER_READ_BIT;
            dependencies[ 0 ].dstAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            
            dependencies[ 0 ].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
        }

        {
            dependencies[ 1 ].srcSubpass      = 0;
            dependencies[ 1 ].dstSubpass      = VK_SUBPASS_EXTERNAL;
            
            dependencies[ 1 ].srcStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            dependencies[ 1 ].dstStageMask    = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            
            dependencies[ 1 ].srcAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            dependencies[ 1 ].dstAccessMask   = VK_ACCESS_SHADER_READ_BIT;
            
            dependencies[ 1 ].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
        }
	}


	void VyRenderer::createOffscreenFramebuffer()
	{
		VkImageView attachments[ 2 ];
        {
            attachments[ 0 ] = m_OffscreenPass.Color.View.handle();
            attachments[ 1 ] = m_OffscreenPass.Depth.View.handle();
        }

		VkFramebufferCreateInfo framebufferInfo{ VKInit::framebufferCreateInfo() };
        {
            framebufferInfo.renderPass      = m_OffscreenPass.RenderPass;

            framebufferInfo.attachmentCount = 2;
            framebufferInfo.pAttachments    = attachments;
            
            framebufferInfo.width           = m_OffscreenPass.Extent.width;
            framebufferInfo.height          = m_OffscreenPass.Extent.height;
            
            framebufferInfo.layers          = 1;
        }

		VK_CHECK(vkCreateFramebuffer(VyContext::device(), &framebufferInfo, nullptr, &m_OffscreenPass.Framebuffer));
	}


	//Deferred rendering

	void VyRenderer::createAttachment(VkFormat format, VkImageUsageFlagBits usage, FrameBufferAttachment* pAttachment)
	{
		VkImageAspectFlags aspectMask = 0;
		VkImageLayout      imageLayout;

		pAttachment->Format = format;

		if (usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
		{
			aspectMask  = VK_IMAGE_ASPECT_COLOR_BIT;
			imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		}
		if (usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
		{
			aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

			if (format >= VK_FORMAT_D16_UNORM_S8_UINT)
            {
                aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
            }

            imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		}

		assert(aspectMask > 0);

        pAttachment->Image = VyImage::Builder{}
			.setName       ("deferred_attachment")
            .setImageType  (VK_IMAGE_TYPE_2D)
            .setFormat     (format)
            .setExtent     (m_DeferredRenderFramebuffer.Extent)
            .setLevels     (1)
			.setLayers     (1)
			.setSamples    (VK_SAMPLE_COUNT_1_BIT)
            .setTiling     (VK_IMAGE_TILING_OPTIMAL)
			.setLayout     (VK_IMAGE_LAYOUT_UNDEFINED)
            .setUsage      (usage | VK_IMAGE_USAGE_SAMPLED_BIT)
            .setMemoryUsage(VMA_MEMORY_USAGE_AUTO)
        	.build();

        pAttachment->View = VyImageView::Builder{}
            .setName    ("deferred_attachment")
            .setViewType(VK_IMAGE_VIEW_TYPE_2D)
            .setFormat  (format)
            .setAspect  (aspectMask)
            .setLevels  (0, 1)
            .setLayers  (0, 1)
            .build( pAttachment->Image );
	}


	void VyRenderer::prepareDeferredRenderFramebuffer()
	{
		// Note: Instead of using fixed sizes, one could also match the window size and recreate the attachments on resize
		m_DeferredRenderFramebuffer.Extent = { 2048, 2048 };

		// Color attachments

		// (World space) Positions
		createAttachment(
			VK_FORMAT_R16G16B16A16_SFLOAT,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
			&m_DeferredRenderFramebuffer.Position
        );

		// (World space) Normals
		createAttachment(
			VK_FORMAT_R16G16B16A16_SFLOAT,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
			&m_DeferredRenderFramebuffer.Normal
        );

		// Albedo (color)
		createAttachment(
			VK_FORMAT_R8G8B8A8_UNORM,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
			&m_DeferredRenderFramebuffer.Albedo
        );

		// Depth attachment
		VkFormat depthFormat = m_Swapchain->findDepthFormat();

		createAttachment(
			depthFormat,
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
			&m_DeferredRenderFramebuffer.Depth
        );

		// Set up separate renderpass with references to the color and depth attachments
		TArray<VkAttachmentDescription, 4> attachmentDescs = {};
        {
            // Init attachment properties
            for (U32 i = 0; i < 4; ++i)
            {
                attachmentDescs[ i ].samples        = VK_SAMPLE_COUNT_1_BIT;
                attachmentDescs[ i ].loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
                attachmentDescs[ i ].storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
                attachmentDescs[ i ].stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                attachmentDescs[ i ].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

                if (i == 3) // Depth
                {
                    attachmentDescs[ i ].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                    attachmentDescs[ i ].finalLayout   = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                }
                else // Color
                {
                    attachmentDescs[ i ].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                    attachmentDescs[ i ].finalLayout   = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                }
            }

            // Formats
            attachmentDescs[ 0 ].format = m_DeferredRenderFramebuffer.Position.Format;
            attachmentDescs[ 1 ].format = m_DeferredRenderFramebuffer.Normal.Format;
            attachmentDescs[ 2 ].format = m_DeferredRenderFramebuffer.Albedo.Format;
            attachmentDescs[ 3 ].format = m_DeferredRenderFramebuffer.Depth.Format;
        }

		TVector<VkAttachmentReference> colorReferences;
        {
            colorReferences.push_back({ 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
            colorReferences.push_back({ 1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
            colorReferences.push_back({ 2, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
        }

		VkAttachmentReference depthReference{};
        {
            depthReference.attachment = 3;
            depthReference.layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        }

		VkSubpassDescription subpass{};
        {
            subpass.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
            subpass.pColorAttachments       = colorReferences.data();
            subpass.colorAttachmentCount    = static_cast<U32>(colorReferences.size());
            subpass.pDepthStencilAttachment = &depthReference;
        }

		// Use subpass dependencies for attachment layout transitions
		TArray<VkSubpassDependency, 2> dependencies;
        {
            {
                dependencies[ 0 ].srcSubpass      = VK_SUBPASS_EXTERNAL;
                dependencies[ 0 ].dstSubpass      = 0;
                
                dependencies[ 0 ].srcStageMask    = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
                dependencies[ 0 ].dstStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                
                dependencies[ 0 ].srcAccessMask   = VK_ACCESS_MEMORY_READ_BIT;
                dependencies[ 0 ].dstAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                
                dependencies[ 0 ].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
            }

            {
                dependencies[ 1 ].srcSubpass      = 0;
                dependencies[ 1 ].dstSubpass      = VK_SUBPASS_EXTERNAL;
                
                dependencies[ 1 ].srcStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                dependencies[ 1 ].dstStageMask    = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
                
                dependencies[ 1 ].srcAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                dependencies[ 1 ].dstAccessMask   = VK_ACCESS_MEMORY_READ_BIT;
                
                dependencies[ 1 ].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
            }
        }

		VkRenderPassCreateInfo renderPassInfo{ VKInit::renderPassCreateInfo() };
        {
            renderPassInfo.pAttachments    = attachmentDescs.data();
            renderPassInfo.attachmentCount = static_cast<U32>(attachmentDescs.size());
        
            renderPassInfo.subpassCount    = 1;
            renderPassInfo.pSubpasses      = &subpass;
        
            renderPassInfo.dependencyCount = 2;
            renderPassInfo.pDependencies   = dependencies.data();
        }

		VK_CHECK(vkCreateRenderPass(VyContext::device(), &renderPassInfo, nullptr, &m_DeferredRenderFramebuffer.RenderPass));

		TArray<VkImageView, 4> attachments;
        {
            attachments[ 0 ] = m_DeferredRenderFramebuffer.Position.View.handle();
            attachments[ 1 ] = m_DeferredRenderFramebuffer.Normal.View  .handle();
            attachments[ 2 ] = m_DeferredRenderFramebuffer.Albedo.View  .handle();
            attachments[ 3 ] = m_DeferredRenderFramebuffer.Depth.View   .handle();
        }

		VkFramebufferCreateInfo framebufferInfo{ VKInit::framebufferCreateInfo() };
        {
            framebufferInfo.pNext           = nullptr;
            
            framebufferInfo.renderPass      = m_DeferredRenderFramebuffer.RenderPass;
            
            framebufferInfo.pAttachments    = attachments.data();
            framebufferInfo.attachmentCount = static_cast<U32>(attachments.size());

            framebufferInfo.width           = m_DeferredRenderFramebuffer.Extent.width;
            framebufferInfo.height          = m_DeferredRenderFramebuffer.Extent.height;

            framebufferInfo.layers          = 1;
        }

		VK_CHECK(vkCreateFramebuffer(VyContext::device(), &framebufferInfo, nullptr, &m_DeferredRenderFramebuffer.Framebuffer));

		// Create sampler to sample from the color attachments
        m_DeferredRenderFramebuffer.Sampler = VySampler::Builder{}
            .setName         ("deferred_framebuffer")
            .setFilters      (VK_FILTER_NEAREST)
            .setMipmapMode   (VK_SAMPLER_MIPMAP_MODE_LINEAR)
            .setWrap         (VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE)
            .setBorder       (VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE)
            .setLodRange     (0.0f, 1.0f)
            .setMipLodBias   (0.0f)
        	.build();
	}
    
    // void VyRenderer::endOffscreenRenderPass(VkCommandBuffer cmdBuffer) const
    // {
    //     VY_ASSERT(m_IsFrameStarted,                    "Can't end offscreen render pass when frame is not in progress.");
    //     VY_ASSERT(cmdBuffer == currentCommandBuffer(), "Can't end offscreen render pass on command buffer from a different frame.");

    //     m_OffscreenFramebuffer->endRenderPass(cmdBuffer);
    // }

    // // =====================================================================================================================

    // VkDescriptorImageInfo VyRenderer::offscreenDescriptionImageInfo(int index) const
    // {
    //     return m_OffscreenFramebuffer->descriptorImageInfo(index);
    // }


    // VkDescriptorImageInfo VyRenderer::depthDescriptionImageInfo(int index) const
    // {
    //     VkDescriptorImageInfo info{};
    //     {
    //         info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    //         info.imageView   = m_OffscreenFramebuffer->depthImageView(index);
    //         info.sampler     = m_OffscreenFramebuffer->depthSampler();
    //     }

    //     return info;
    // }

    // // =====================================================================================================================
    
    // void VyRenderer::createOffscreenResources()
    // {
    //     m_OffscreenFramebuffer = MakeUnique<VyFramebuffer>(
    //         m_Swapchain->swapchainExtent(), 
    //         MAX_FRAMES_IN_FLIGHT, 
    //         true /* Use Mipmaps */
    //     );
    // }


    // void VyRenderer::generateOffscreenMipmaps(VkCommandBuffer cmdBuffer)
    // {
    //     m_OffscreenFramebuffer->generateMipmaps(cmdBuffer, m_CurrentFrameIndex);
    // }

#pragma endregion Rendering
}