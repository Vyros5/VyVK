// #include <VyEngine/VK/Renderer/OffscreenRenderer.h>

// #include <VyEngine/VK/Context.h>

// namespace Vy
// {
//     VyOffscreenRenderer::VyOffscreenRenderer(VkExtent2D extent, VkFormat format, U32 imageLayerCount) :
// 	    m_ImageExtent{ extent }, 
//         m_ImageFormat{ format }
//     {
//         createOffscreenImage(imageLayerCount);
//         createRenderPass();
//         createFramebuffer();

//         createCommandBuffer();
//     }

//     // VyBuffer VyOffscreenRenderer::prepareImageCopy(VkCommandBuffer cmdBuffer)
//     // {
//     //     VkDeviceSize imageSize = m_ImageExtent.width * m_ImageExtent.height * 4; // Assuming 4 bytes per pixel (RGBA)

//     //     VyBuffer stagingBuffer = VyBuffer::stagingBuffer( "prep_img_copy", imageSize );

//     //     VyContext::device().copyImageToBuffer(m_OffscreenImage.handle(), stagingBuffer.handle(), m_ImageExtent.width, m_ImageExtent.height, 1);

//     //     return stagingBuffer;
//     // }


//     // TVector<U8> VyOffscreenRenderer::flushBufferToMemory(VyBuffer& stagingBuffer)
//     // {
//     // }



//     VyOffscreenRenderer::~VyOffscreenRenderer()
//     {
//         vkDestroyFramebuffer(VyContext::device(), m_Framebuffer, nullptr);
//         vkDestroyRenderPass(VyContext::device(), m_RenderPass, nullptr);
    
//         freeCommandBuffer();
//     }


//     VkCommandBuffer VyOffscreenRenderer::beginFrame() 
//     {
//         VY_ASSERT(!m_IsFrameStarted, "Can't call `beginFrame` while frame is already in progress.");

//         m_IsFrameStarted = true;

//         auto cmdBuffer = this->currentCommandBuffer();

//         VkCommandBufferBeginInfo beginInfo{ VKInit::commandBufferBeginInfo() };
        
// 		VK_CHECK_SUCCESS(vkBeginCommandBuffer(cmdBuffer, &beginInfo), 
//             "Failed to begin recording command buffer!");
        
//         return cmdBuffer;
//     }


//     void VyOffscreenRenderer::endFrame()
//     {
//         VY_ASSERT(m_IsFrameStarted, "Can't call `endFrame` while frame is not in progress.");
        
//         auto cmdBuffer = this->currentCommandBuffer();
        
//         // Stop command buffer recording.
//         VK_CHECK_SUCCESS(vkEndCommandBuffer(cmdBuffer), 
//             "Failed to record command buffer!");

//         // -----------------------------------------------------------------------------------------------------------------
//         // [ SUBMIT ]

//         VkSubmitInfo submitInfo{ VKInit::submitInfo() };
//         {
//             // Command buffer to submit for execution.
//             submitInfo.pCommandBuffers      = &m_CommandBuffer;
//             submitInfo.commandBufferCount   = 1;
//         }

//         // Submit command buffer to the graphics queue.
// 		VK_CHECK_SUCCESS(vkQueueSubmit(
//                 VyContext::device().graphicsQueue(), 
//                 1, 
//                 &submitInfo, 
//                 VK_NULL_HANDLE
//             ),
//             "Failed to submit queue!"
//         );

//         vkQueueWaitIdle(VyContext::device().graphicsQueue());

//         m_IsFrameStarted = false;
//     }


//     void VyOffscreenRenderer::beginOffscreenPass(VkCommandBuffer cmdBuffer)
//     {
//         VY_ASSERT(m_IsFrameStarted,                    "Can't begin swapchain render pass when frame is not in progress.");
//         VY_ASSERT(cmdBuffer == currentCommandBuffer(), "Can't begin swapchain render pass on command buffer from a different frame.");

//         VkClearValue clearValues{};
//         {
//             clearValues.color = { 0.0f, 0.0f, 0.0f, 1.0f };
//         }

//         // Begin render pass.
//         VkRenderPassBeginInfo renderPassInfo{ VKInit::renderPassBeginInfo() };
//         {
//             renderPassInfo.renderPass        = m_RenderPass;
//             renderPassInfo.framebuffer       = m_Framebuffer;
//             renderPassInfo.renderArea.offset = { 0, 0 };
//             renderPassInfo.renderArea.extent = m_ImageExtent;
//             renderPassInfo.clearValueCount   = 1;
//             renderPassInfo.pClearValues      = &clearValues;
//         }

//         vkCmdBeginRenderPass(cmdBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

//         // Set viewport and scissor rect.
//         VKCmd::viewport( cmdBuffer, m_ImageExtent );
//         VKCmd::scissor ( cmdBuffer, m_ImageExtent );
//     }


//     void VyOffscreenRenderer::endOffscreenPass(VkCommandBuffer cmdBuffer)
//     {
//         VY_ASSERT(m_IsFrameStarted,                    "Can't end the current render pass when frame is not in progress.");
//         VY_ASSERT(cmdBuffer == currentCommandBuffer(), "Can't end the current render pass on command buffer from a different frame.");
        
//         vkCmdEndRenderPass(cmdBuffer);
//     }


//     void VyOffscreenRenderer::createCommandBuffer()
//     {
//         VkCommandBufferAllocateInfo allocInfo{ VKInit::commandBufferAllocateInfo() };
//         {
//             allocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
//             allocInfo.commandPool        = VyContext::device().graphicsCommandPool();
//             allocInfo.commandBufferCount = 1;
//         }

//         VK_CHECK_SUCCESS(vkAllocateCommandBuffers(VyContext::device(), &allocInfo, &m_CommandBuffer),
//             "Failed to allocate command buffer!"
//         );

//         VKDbg::setObjectName(VK_OBJECT_TYPE_COMMAND_BUFFER, (U64)m_CommandBuffer, "offscreen_cmd_buffer");
//     }


//     void VyOffscreenRenderer::freeCommandBuffer()
//     {
//         vkFreeCommandBuffers(VyContext::device(), VyContext::device().graphicsCommandPool(), 1, &m_CommandBuffer);
//     }


//     void VyOffscreenRenderer::createOffscreenImage(U32 imageLayerCount)
//     {
//         // [ Image ]
//         {
//             auto builder = VyImage::Builder{};
//             builder.setName       ("offscreen");
//             builder.setImageType  (VK_IMAGE_TYPE_2D);
//             builder.setExtent     (m_ImageExtent);
//             builder.setLevels     (1);
//             builder.setLayers     (imageLayerCount);
//             builder.setFormat     (m_ImageFormat);
//             builder.setTiling     (VK_IMAGE_TILING_OPTIMAL);
//             builder.setLayout     (VK_IMAGE_LAYOUT_UNDEFINED);
//             builder.setUsage      (VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
//             // builder.setSamples    ( this->msaaSampleCountFlagBits() );
//             builder.setSharing    (VK_SHARING_MODE_EXCLUSIVE);
//             builder.setMemoryUsage(VMA_MEMORY_USAGE_AUTO);

//             m_OffscreenImage = builder.build();
//         }

//         // [ View ]
//         {
//             auto viewBuilder = VyImageView::Builder{};
//             {
//                 viewBuilder.setName    ("offscreen");
//                 viewBuilder.setViewType(VK_IMAGE_VIEW_TYPE_2D);
//                 viewBuilder.setFormat  (m_ImageFormat);
//                 viewBuilder.setAspect  (VK_IMAGE_ASPECT_COLOR_BIT);
//                 viewBuilder.setLayers  (0, 1);
//                 viewBuilder.setLevels  (0, 1);
//             }
            
//             m_OffscreenView = viewBuilder.build( m_OffscreenImage );
//         }
//     }


//     void VyOffscreenRenderer::createRenderPass()
//     {
//         VkAttachmentDescription colorAttachment{ VKInit::attachmentDescription() };
//         {
//             colorAttachment.format      = m_ImageFormat;
//             colorAttachment.samples     = VK_SAMPLE_COUNT_1_BIT;
//             colorAttachment.loadOp      = VK_ATTACHMENT_LOAD_OP_CLEAR;
//             colorAttachment.storeOp     = VK_ATTACHMENT_STORE_OP_STORE;
//             colorAttachment.finalLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
//         }

//         VkAttachmentReference colorAttachmentRef{};
//         {
//             colorAttachmentRef.attachment = 0;
//             colorAttachmentRef.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
//         }

//         VkSubpassDescription subpass{};
//         {
//             subpass.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;
//             subpass.colorAttachmentCount = 1;
//             subpass.pColorAttachments    = &colorAttachmentRef;
//         }

//         // Add subpass dependencies
//         TArray<VkSubpassDependency, 2> dependencies{};
//         {
//             // Dependency from external to subpass
//             dependencies[0].srcSubpass      = VK_SUBPASS_EXTERNAL;
//             dependencies[0].dstSubpass      = 0;
//             dependencies[0].srcStageMask    = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
//             dependencies[0].dstStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
//             dependencies[0].srcAccessMask   = VK_ACCESS_MEMORY_READ_BIT;
//             dependencies[0].dstAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
//             dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
            
//             // Dependency from subpass to external
//             dependencies[1].srcSubpass      = 0;
//             dependencies[1].dstSubpass      = VK_SUBPASS_EXTERNAL;
//             dependencies[1].srcStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
//             dependencies[1].dstStageMask    = VK_PIPELINE_STAGE_TRANSFER_BIT;
//             dependencies[1].srcAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
//             dependencies[1].dstAccessMask   = VK_ACCESS_MEMORY_READ_BIT;
//             dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
//         }

//         VkRenderPassCreateInfo renderPassInfo{ VKInit::renderPassCreateInfo() };
//         {
//             // Attachments
//             renderPassInfo.attachmentCount = 1;
//             renderPassInfo.pAttachments    = &colorAttachment;

//             // Subpasses
//             renderPassInfo.subpassCount    = 1;
//             renderPassInfo.pSubpasses      = &subpass;
            
//             // Dependencies
//             renderPassInfo.dependencyCount = static_cast<U32>(dependencies.size());
//             renderPassInfo.pDependencies   = dependencies.data();
//         }

//         VK_CHECK_SUCCESS(vkCreateRenderPass(VyContext::device(), &renderPassInfo, nullptr, &m_RenderPass), 
//             "Failed to create offscreen render pass!"
//         );

//         VKDbg::setObjectName(m_RenderPass, "offscreen_renderpass");
//     }


//     void VyOffscreenRenderer::createFramebuffer()
//     {
//         VkFramebufferCreateInfo framebufferInfo{ VKInit::framebufferCreateInfo() };
//         {
//             // Renderpass to use this framebuffer with.
//             framebufferInfo.renderPass      = m_RenderPass;
            
//             // Attachments
//             framebufferInfo.attachmentCount = 1;
//             framebufferInfo.pAttachments    = &m_OffscreenView.handle();

//             // Set size to match image size.
//             framebufferInfo.width           = m_ImageExtent.width;
//             framebufferInfo.height          = m_ImageExtent.height;

//             // Only one layer.
//             framebufferInfo.layers          = 1;

//             // No Flags
//             framebufferInfo.flags           = 0;
//         }

//         VK_CHECK_SUCCESS(vkCreateFramebuffer(VyContext::device(), &framebufferInfo, nullptr, &m_Framebuffer), 
//             "Failed to create offscreen framebuffer!"
//         );

//         VKDbg::setObjectName(m_Framebuffer, "offscreen_framebuffer");
//     }



// }