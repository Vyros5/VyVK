#include <VyEngine/VK/Pipeline/RenderPass.h>

#include <VyEngine/VK/Context.h>

namespace Vy
{
    VyRenderPass::VyRenderPass(TString debugName) : 
        m_DebugName( std::move( debugName ) )
    {
    }


    VyRenderPass::~VyRenderPass()
    {
        if (m_RenderPass != VK_NULL_HANDLE)
        {
            vkDestroyRenderPass(VyContext::device(), m_RenderPass, nullptr);
        }
    }


    void VyRenderPass::addAttachment(const AttachmentDescription& attachment)
    {
        m_Attachments.push_back( attachment );
    }


    void VyRenderPass::addSubpass(const SubpassDescription& subpass)
    {
        m_Subpasses.push_back( subpass );
    }


    void VyRenderPass::addDependency(const VkSubpassDependency& dependency)
    {
        m_Dependencies.push_back( dependency );
    }


    void VyRenderPass::addDependency(
        U32                  srcSubpass, 
        U32                  dstSubpass,
        VkPipelineStageFlags srcStageMask, 
        VkPipelineStageFlags dstStageMask,
        VkAccessFlags        srcAccessMask, 
        VkAccessFlags        dstAccessMask,
        VkDependencyFlags    dependencyFlags)
    {
        VkSubpassDependency dependency{};
        {
            dependency.srcSubpass      = srcSubpass;
            dependency.dstSubpass      = dstSubpass;
            dependency.srcStageMask    = srcStageMask;
            dependency.dstStageMask    = dstStageMask;
            dependency.srcAccessMask   = srcAccessMask;
            dependency.dstAccessMask   = dstAccessMask;
            dependency.dependencyFlags = dependencyFlags;
        }

        m_Dependencies.push_back( dependency );
    }


    void VyRenderPass::build()
    {
        createRenderPass();
    }


    void VyRenderPass::createRenderPass()
    {
        TVector<VkAttachmentDescription> attachmentDescriptions;

        for (const auto& attachment : m_Attachments)
        {
            VkAttachmentDescription desc{};
            {
                desc.format         = attachment.Format;
                desc.samples        = attachment.Samples;
                desc.loadOp         = attachment.LoadOp;
                desc.storeOp        = attachment.StoreOp;
                desc.stencilLoadOp  = attachment.StencilLoadOp;
                desc.stencilStoreOp = attachment.StencilStoreOp;
                desc.initialLayout  = attachment.InitialLayout;
                desc.finalLayout    = attachment.FinalLayout;
            }

            attachmentDescriptions.push_back( desc );
        }

        TVector<VkSubpassDescription>           subpassDescriptions;
        TVector<TVector<VkAttachmentReference>> colorReferences;
        TVector<VkAttachmentReference>          depthReferences;
        TVector<TVector<VkAttachmentReference>> inputReferences;
        TVector<TVector<VkAttachmentReference>> resolveReferences;

        for (const auto& subpass : m_Subpasses)
        {
            VkSubpassDescription desc{};
            desc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

            // [ Color Attachments ]
            {
                colorReferences.emplace_back();

                for (U32 attachmentIndex : subpass.ColorAttachments)
                {
                    colorReferences.back().push_back({
                        attachmentIndex, 
                        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
                    });
                }
                desc.colorAttachmentCount = static_cast<U32>(colorReferences.back().size());
                desc.pColorAttachments    = colorReferences.back().data();
            }

            // [ Depth Attachment ]
            {
                if (subpass.DepthStencilAttachment)
                {
                    depthReferences.push_back({
                        subpass.DepthStencilAttachment.value(), 
                        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
                    });
                    
                    desc.pDepthStencilAttachment = &depthReferences.back();
                }
            }

            // [ Input Attachments ]
            {
                inputReferences.emplace_back();

                for (U32 attachmentIndex : subpass.InputAttachments)
                {
                    inputReferences.back().push_back({
                        attachmentIndex, 
                        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
                    });
                }

                desc.inputAttachmentCount = static_cast<U32>(inputReferences.back().size());
                desc.pInputAttachments    = inputReferences.back().data();
            }

            // [ Resolve Attachments ]
            {
                resolveReferences.emplace_back();
                
                for (U32 attachmentIndex : subpass.ResolveAttachments)
                {
                    resolveReferences.back().push_back({
                        attachmentIndex, 
                        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
                    });
                }
                
                desc.pResolveAttachments = resolveReferences.back().data();
            }

            // [ Preserve Attachments ]
            {
                desc.preserveAttachmentCount = static_cast<U32>(subpass.PreserveAttachments.size());
                desc.pPreserveAttachments    = subpass.PreserveAttachments.data();
            }

            subpassDescriptions.push_back( desc );
        }

        VkRenderPassCreateInfo passInfo{ VKInit::renderPassCreateInfo() };
        {
            passInfo.attachmentCount = static_cast<U32>(attachmentDescriptions.size());
            passInfo.pAttachments    = attachmentDescriptions.data();
            
            passInfo.subpassCount    = static_cast<U32>(subpassDescriptions.size());
            passInfo.pSubpasses      = subpassDescriptions.data();
            
            passInfo.dependencyCount = static_cast<U32>(m_Dependencies.size());
            passInfo.pDependencies   = m_Dependencies.data();
        }

        VK_CHECK_SUCCESS(vkCreateRenderPass(VyContext::device(), &passInfo, nullptr, &m_RenderPass),
            "Failed to create renderpass!");
    }


    U32 VyRenderPass::colorAttachmentCount()
    {
        U32 counter = 0;
        for (auto& attachmentDescription: m_Attachments)
        {
            if (attachmentDescription.Type == AttachmentType::Color)
            {
                counter++;
            }
        }

        return counter;
    }


    void VyRenderPass::beginPass(
        VkCommandBuffer       cmdBuffer, 
        VkRenderPassBeginInfo beginInfo, 
        VkExtent2D            extent)
    {
        if(m_AttachmentClearValues.size() != m_Attachments.size())
        {
            m_AttachmentClearValues.resize( m_Attachments.size() );
        }

        for(int i = 0; i < m_Attachments.size(); ++i)
        {
            m_AttachmentClearValues[ i ] = m_Attachments[ i ].ClearValue;
        }

        beginInfo.pClearValues    = m_AttachmentClearValues.data();
        beginInfo.clearValueCount = m_AttachmentClearValues.size();

        vkCmdBeginRenderPass(cmdBuffer, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);

        VKCmd::viewport(cmdBuffer, extent);
        VKCmd::scissor (cmdBuffer, extent);
    }


    void VyRenderPass::endPass(VkCommandBuffer cmdBuffer)
    {
        vkCmdEndRenderPass(cmdBuffer);
    }


    void test()
    {
        VyRenderPass pass{ "test" };

        // Color attachment (swapchain image)
        AttachmentDescription colorAtt{};
        {
            colorAtt.Type          = AttachmentType::Color;
            colorAtt.Format        = VK_FORMAT_R32G32B32_SFLOAT;
            colorAtt.Samples       = VK_SAMPLE_COUNT_1_BIT;
            colorAtt.LoadOp        = VK_ATTACHMENT_LOAD_OP_CLEAR;
            colorAtt.StoreOp       = VK_ATTACHMENT_STORE_OP_STORE;
            colorAtt.InitialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            colorAtt.FinalLayout   = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
            colorAtt.ClearValue    = { .color = { 0.0f, 0.0f, 0.0f, 1.0f } };
        }

        pass.addAttachment( colorAtt );

        // Set up subpass
        SubpassDescription subpass;
        subpass.ColorAttachments = { 0 };

        pass.addSubpass( subpass );
    
        // Add a single dependency
        pass.addDependency(
            VK_SUBPASS_EXTERNAL, 
            0,
            
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            
            0, 
            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            
            VK_DEPENDENCY_BY_REGION_BIT
        );

        // Finalize
        pass.build();
    }
}


namespace Vy
{
    void VyDeferredRenderer::CreateGBufferRenderPass()
    {
        m_GBufferPass = std::make_unique<VyRenderPass>("G-Buffer Render Pass");

        // Position attachment
        m_GBufferPass->addAttachment({
            .Type          = AttachmentType::Color,
            .Format        = VK_FORMAT_R16G16B16A16_SFLOAT,
            .Samples       = VK_SAMPLE_COUNT_1_BIT,
            .LoadOp        = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .StoreOp       = VK_ATTACHMENT_STORE_OP_STORE,
            .InitialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .FinalLayout   = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            .ClearValue    = {.color = {0.0f, 0.0f, 0.0f, 1.0f}}
        });

        // Normal attachment
        m_GBufferPass->addAttachment({
            .Type          = AttachmentType::Color,
            .Format        = VK_FORMAT_R16G16B16A16_SFLOAT,
            .Samples       = VK_SAMPLE_COUNT_1_BIT,
            .LoadOp        = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .StoreOp       = VK_ATTACHMENT_STORE_OP_STORE,
            .InitialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .FinalLayout   = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            .ClearValue    = {.color = {0.0f, 0.0f, 0.0f, 1.0f}}
        });

        // Albedo attachment
        m_GBufferPass->addAttachment({
            .Type          = AttachmentType::Color,
            .Format        = VK_FORMAT_R8G8B8A8_UNORM,
            .Samples       = VK_SAMPLE_COUNT_1_BIT,
            .LoadOp        = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .StoreOp       = VK_ATTACHMENT_STORE_OP_STORE,
            .InitialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .FinalLayout   = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            .ClearValue    = {.color = {0.0f, 0.0f, 0.0f, 1.0f}}
        });

        // Depth attachment
        m_GBufferPass->addAttachment({
            .Type          = AttachmentType::Depth,
            .Format        = VK_FORMAT_D32_SFLOAT,
            .Samples       = VK_SAMPLE_COUNT_1_BIT,
            .LoadOp        = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .StoreOp       = VK_ATTACHMENT_STORE_OP_STORE,
            .InitialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .FinalLayout   = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            .ClearValue    = {.depthStencil = {1.0f, 0}}
        });

        // Set up subpass
        SubpassDescription subpass;
        {
            subpass.ColorAttachments       = { 0, 1, 2 }; // Position, Normal, Albedo
            subpass.DepthStencilAttachment = 3;           // Depth
        }

        m_GBufferPass->addSubpass( subpass );

        /*
        * External -> G-Buffer Color Attachments Writes
        *
        * The latest stage that worked on the color attachments of the G-Buffer that must be waited on is either the FRAGMENT_SHADER
        * (read by lighting pass) or BOTTOM_OF_THE_PIPE (? is this really necessary though ?) as a catch-all for the first frame
        * where nothing worked on the attachments to begin with.  Once the src work is completed, the earliest stage that this pass
        * will work on the attachment is in the COLOR_ATTACHMENT_OUTPUT stage where it will write to the attachment.
        */
        m_GBufferPass->addDependency(
            VK_SUBPASS_EXTERNAL, 
            0, 
            
            VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 
            
            VK_ACCESS_MEMORY_READ_BIT, 
            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            
            VK_DEPENDENCY_BY_REGION_BIT
        );

        /*
        * External -> G-Buffer Depth/Stencil Attachment Dependency
        *
        * The latest stage that worked on the depth/stencil attachment of the G-Buffer that must be waited on is the early/late
        * fragment stage tests where the attachments were written to.
        * Once the src work is completed, the earliest stage that this pass will work on the depth/stencil is in the same part of the
        * pipeline (early/late fragment tests) where the attachment could either be read from or written to.
        */
        m_GBufferPass->addDependency(
            VK_SUBPASS_EXTERNAL, 
            0,
            
            VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
            VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
            
            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT, 
            
            VK_DEPENDENCY_BY_REGION_BIT
        );

        /*
        * G-Buffer Color Attachment Writes -> G-Buffer Color Attachment Reads
        *
        * The latest stage that worked on the depth/stencil attachment of the G-Buffer that must be waited on is the early/late
        * fragment stage tests where the attachments were written to.
        * Once the src work is completed, the earliest stage that this pass will work on the depth/stencil is in the same part of the
        * pipeline (early/late fragment tests) where the attachment could either be read from or written to.
        */
        m_GBufferPass->addDependency(
            0, 
            VK_SUBPASS_EXTERNAL, 
            
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 
            
            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, 
            VK_ACCESS_SHADER_READ_BIT,
            
            VK_DEPENDENCY_BY_REGION_BIT
        );

        m_GBufferPass->build();
    }



    void VyDeferredRenderer::CreateCompositionRenderPass()
    {
        m_CompositionPass = std::make_unique<VyRenderPass>("Composition Render Pass");

        // Final color attachment (swapchain image)
        m_CompositionPass->addAttachment({
            .Type = AttachmentType::Color,
            // .Format =  ImageUtils::VyFormatToImageFormat(m_Renderer->VySwapchain().SwapchainImageFormat()),
            .Samples = VK_SAMPLE_COUNT_1_BIT,
            .LoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .StoreOp = VK_ATTACHMENT_STORE_OP_STORE,
            .InitialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .FinalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            .ClearValue = {.color = {0.0f, 0.0f, 0.0f, 1.0f}}
        });

        // Set up subpass
        SubpassDescription subpass;
        subpass.ColorAttachments = {0};	   // Final color output
        m_CompositionPass->addSubpass(subpass);

        /*
            Source Stage (srcStageMask):
                This says: "We need to wait for the src stage to finish working on this attachment (if it was being worked on)."
            Destination Stage (dstStageMask):
                This says: "The earliest pipeline stage in our render pass where we will start working on this attachment is the dst
        stage."
        */

        /*
        * External -> Swapchain Color Attachment Writes
        *
        * The latest stage that worked on the swapchain image that must be waited on is BOTTOM OF THE PIPE(?) stage.
        * Once the src work is completed (swapchain image acquired(?)), the earliest stage that this pass will
        * work on the attachment is in the COLOR_ATTACHMENT_OUTPUT stage where it will write to the swapchain image.
        */
        m_CompositionPass->addDependency(
            VK_SUBPASS_EXTERNAL, 
            0, 
            
            VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 
            
            0, 
            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, 
            
            VK_DEPENDENCY_BY_REGION_BIT
        );

        /*
        * Swapchain Color Attachment Writes -> Bottom of the Pipe (presentation)
        *
        * Wait on the color attachment writes to the swapchain image from the previous dependency.
        * Once the writes are complete, this pass will prepare the swapchain image for presentation,
        * which to my knowledge, is conservatively done by using VK_PIPELINE_STAGE_BOTTOM_OF_PIPE as the dst stage
        * with 0 as the dst access mask.
        */
        m_CompositionPass->addDependency(
            0, 
            VK_SUBPASS_EXTERNAL, 
            
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 
            
            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, 
            0, 
            
            VK_DEPENDENCY_BY_REGION_BIT
        );

        m_CompositionPass->build();
    }
}