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