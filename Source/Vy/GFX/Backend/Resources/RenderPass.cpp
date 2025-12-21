#include <Vy/GFX/Backend/Resources/RenderPass.h>

#include <Vy/GFX/Context.h>

namespace Vy
{
    VyRenderPass::VyRenderPass(const VyRenderPassDesc& desc)
	{
		VkRenderPassCreateInfo renderPassInfo{ VKInit::renderPassCreateInfo() };
        {
            renderPassInfo.attachmentCount = static_cast<U32>(desc.AttachmentDescriptions.size());
            renderPassInfo.pAttachments    = desc.AttachmentDescriptions.data();

            renderPassInfo.subpassCount    = 1;
            renderPassInfo.pSubpasses      = &desc.SubpassDescription;
            
            renderPassInfo.dependencyCount = 1;
            renderPassInfo.pDependencies   = &desc.SubpassDependency;
        }

		if (vkCreateRenderPass(VyContext::device(), &renderPassInfo, nullptr, &m_RenderPass) != VK_SUCCESS) 
        {
			VY_THROW_RUNTIME_ERROR("Failed to create render pass");
		}
	}


    VyRenderPass::VyRenderPass(VkRenderPassCreateInfo& createInfo)
    {
		if (vkCreateRenderPass(VyContext::device(), &createInfo, nullptr, &m_RenderPass) != VK_SUCCESS) 
        {
			VY_THROW_RUNTIME_ERROR("Failed to create render pass");
		}
    }


	VyRenderPass::~VyRenderPass()
	{
		destroy();
	}


    VyRenderPass::VyRenderPass(VyRenderPass&& other) noexcept : 
        m_RenderPass(other.m_RenderPass)
    {
        other.m_RenderPass = VK_NULL_HANDLE;
    }


    VyRenderPass& VyRenderPass::operator=(VyRenderPass&& other) noexcept
    {
        if (this == &other)
        {
            return *this;
        }

        m_RenderPass = other.m_RenderPass;

        other.m_RenderPass = VK_NULL_HANDLE;

        return *this;
    }


    void VyRenderPass::destroy()
    {
        if (m_RenderPass != VK_NULL_HANDLE)
        {
            vkDestroyRenderPass(VyContext::device(), m_RenderPass, nullptr);

            m_RenderPass = VK_NULL_HANDLE;
        }
    }
}


namespace Vy
{
    VulkanRenderPass::VulkanRenderPass(String debugName)
        : m_DebugName(std::move(debugName))
    {
    }

    VulkanRenderPass::~VulkanRenderPass()
    {
        if (m_RenderPass != VK_NULL_HANDLE)
        {
            vkDestroyRenderPass(VyContext::device(), m_RenderPass, nullptr);
        }
    }

    void VulkanRenderPass::addAttachment(const VyAttachmentDescription& attachment)
    {
        m_Attachments.push_back(attachment);
    }

    void VulkanRenderPass::addSubpass(const VySubpassDescription& subpass)
    {
        m_Subpasses.push_back(subpass);
    }

    void VulkanRenderPass::addDependency(const VyDependencyDescription& dependency)
    {
        VkSubpassDependency subpassDependency{};
        {
            subpassDependency.srcSubpass      = dependency.SrcSubpass;
            subpassDependency.dstSubpass      = dependency.DstSubpass;
            subpassDependency.srcStageMask    = dependency.SrcStageMask;
            subpassDependency.dstStageMask    = dependency.DstStageMask;
            subpassDependency.srcAccessMask   = dependency.SrcAccessMask;
            subpassDependency.dstAccessMask   = dependency.DstAccessMask;
            subpassDependency.dependencyFlags = dependency.DependencyFlags;
        }

        m_Dependencies.push_back(subpassDependency);
    }


    void VulkanRenderPass::build()
    {
        createRenderPass();
    }


    void VulkanRenderPass::createRenderPass()
    {
        TVector<VkAttachmentDescription> attachmentDescriptions;

        for (const auto &attachment: m_Attachments)
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

            attachmentDescriptions.push_back(desc);
        }

        TVector<VkSubpassDescription>           subpassDescriptions;

        TVector<TVector<VkAttachmentReference>> colorReferences;
        TVector<VkAttachmentReference>          depthReferences;
        TVector<TVector<VkAttachmentReference>> inputReferences;
        TVector<TVector<VkAttachmentReference>> resolveReferences;

        for (const auto &subpass: m_Subpasses)
        {
            VkSubpassDescription desc{};
            desc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

            // Color
            {
                colorReferences.emplace_back();
                for (U32 attachment: subpass.ColorAttachments)
                {
                    colorReferences.back().push_back({
                        attachment, 
                        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
                    });
                }
                
                desc.colorAttachmentCount = static_cast<U32>(colorReferences.back().size());
                desc.pColorAttachments    = colorReferences.back().data();
            }

            // Depth Stencil
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

            // Input
            {
                inputReferences.emplace_back();
                for (U32 attachment: subpass.InputAttachments)
                {
                    inputReferences.back().push_back({
                        attachment, 
                        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
                    });
                }
                
                desc.inputAttachmentCount = static_cast<U32>(inputReferences.back().size());
                desc.pInputAttachments    = inputReferences.back().data();
            }

            //  Resolve
            {
                resolveReferences.emplace_back();
                for (U32 attachment: subpass.ResolveAttachments)
                {
                    resolveReferences.back().push_back({
                        attachment, 
                        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
                    });
                }
                
                desc.pResolveAttachments = resolveReferences.back().data();
            }

            // Preserve
            {
                desc.preserveAttachmentCount = static_cast<U32>(subpass.PreserveAttachments.size());
                desc.pPreserveAttachments    = subpass.PreserveAttachments.data();
            }


            subpassDescriptions.push_back(desc);
        }

        VkRenderPassCreateInfo renderPassInfo{ VKInit::renderPassCreateInfo() };
        {
            renderPassInfo.attachmentCount = static_cast<U32>(attachmentDescriptions.size());
            renderPassInfo.pAttachments    = attachmentDescriptions.data();

            renderPassInfo.subpassCount    = static_cast<U32>(subpassDescriptions.size());
            renderPassInfo.pSubpasses      = subpassDescriptions.data();
            
            renderPassInfo.dependencyCount = static_cast<U32>(m_Dependencies.size());
            renderPassInfo.pDependencies   = m_Dependencies.data();
        }

        VK_CHECK(vkCreateRenderPass(VyContext::device(), &renderPassInfo, nullptr, &m_RenderPass));
    }


    U32 VulkanRenderPass::colorAttachmentCount()
    {
        U32 counter = 0;
        for(auto& attachmentDescription: m_Attachments)
        {
            if(attachmentDescription.Type == VyAttachmentType::Color)
            {
                counter++;
            }
        }

        return counter;
    }


    void VulkanRenderPass::beginPass(
        VkCommandBuffer       cmdBuffer, 
        VkRenderPassBeginInfo beginInfo, 
        VkExtent2D            extent)
    {
        if(m_AttachmentClearValues.size() != m_Attachments.size())
        {
            m_AttachmentClearValues.resize(m_Attachments.size());
        }

        for(int i = 0; i < m_Attachments.size(); ++i)
        {
            m_AttachmentClearValues[i] = m_Attachments[i].ClearValue;
        }

        beginInfo.pClearValues    = m_AttachmentClearValues.data();
        beginInfo.clearValueCount = m_AttachmentClearValues.size();

        vkCmdBeginRenderPass(cmdBuffer, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport{};
        {
            viewport.x        = 0.0f;
            viewport.y        = 0.0f;
            viewport.width    = static_cast<float>(extent.width);
            viewport.height   = static_cast<float>(extent.height);
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;
        }
        
        VkRect2D scissor{};
        {
            scissor.offset = {0, 0};
            scissor.extent = extent;
        }

        vkCmdSetViewport(cmdBuffer, 0, 1, &viewport);
        vkCmdSetScissor (cmdBuffer, 0, 1, &scissor );
    }

    void VulkanRenderPass::endPass(VkCommandBuffer cmdBuffer)
    {
        vkCmdEndRenderPass(cmdBuffer);
    }

    bool VulkanRenderPass::formatIsDepth(VkFormat format)
    {
        return 
            format == VK_FORMAT_D32_SFLOAT || 
            format == VK_FORMAT_D32_SFLOAT_S8_UINT || 
            format == VK_FORMAT_D24_UNORM_S8_UINT;
    }
}






// namespace Vy
// {
//     TVector<VkAttachmentDescription> ShadowPass::attachmentDescriptions() const
//     {
//         VkAttachmentDescription depthAttachment{};
//         {
//             depthAttachment.format = VyContext::device().findDepthFormat();
            
//             depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
//             depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
//             depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
//             depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
//             depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
//             depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
//             depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
//         }

//         return { depthAttachment };
//     }

//     TVector<VkSubpassDescription> ShadowPass::subpassDescriptions(const TVector<VkAttachmentReference>& attachmentRefs, const VkAttachmentReference& depthRef) const
//     {
//         VkSubpassDescription subpass{};
//         {
//             subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
//             subpass.colorAttachmentCount = 0;
//             subpass.pColorAttachments = VK_NULL_HANDLE;
//             subpass.pDepthStencilAttachment = &attachmentRefs[0];
//         }
        
//         return { subpass };
//     }

//     TVector<VkSubpassDependency> ShadowPass::subpassDependencies() const
//     {
//         std::array<VkSubpassDependency, 2> dependencies;
//         {
//             dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
//             dependencies[0].dstSubpass = 0;
//             dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
//             dependencies[0].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
//             dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
//             dependencies[0].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
//             dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
            
//             dependencies[1].srcSubpass = 0;
//             dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
//             dependencies[1].srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
//             dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
//             dependencies[1].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
//             dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
//             dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
//         }

//         return { dependencies[0], dependencies[1] };
//     }

//     TVector<VkAttachmentReference> ShadowPass::colorAttachmentRefs() const
//     {
//         VkAttachmentReference attachmentRef{};
//         {
//             attachmentRef.attachment = 0;
//             attachmentRef.layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
//         }

//         return {attachmentRef};
//     }

//     VkAttachmentReference ShadowPass::depthAttachmentRef() const
//     {
//         VkAttachmentReference attachmentRef{};
//         {
//             attachmentRef.attachment = 0;
//             attachmentRef.layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
//         }

//         return attachmentRef;
//     }

//     TVector<VkImageView> ShadowPass::imageViews() const
//     {
//         return { m_ShadowMap.View.handle() };
//     }

//     TVector<VkClearValue> ShadowPass::clearValues() const
//     {
//         VkClearValue clearColor;
//         {
//             clearColor.depthStencil = { 1.0f, 0 };
//         }

//         return { clearColor };
//     }

//     ShadowPass::ShadowPass(U32 w, U32 h)
//         : VRenderPass(w, h)
//     {
//         VkFormat format = VyContext::device().findDepthFormat();

//         createAttachment(&m_ShadowMap, format, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL);

//         createRenderPass();
//         createFramebuffer();
//     }

//     ShadowPass::~ShadowPass()
//     {

//     }
// }