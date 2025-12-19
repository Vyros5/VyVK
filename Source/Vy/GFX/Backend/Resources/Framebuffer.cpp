#include <Vy/GFX/Backend/Resources/Framebuffer.h>
#include <Vy/GFX/Backend/Resources/RenderPass.h>

#include <Vy/GFX/Context.h>

#include <iostream>

namespace Vy
{
    VyFramebuffer::VyFramebuffer(
        const VyRenderPass&     renderPass, 
        const VyFramebufferDesc& desc
    ) : 
        m_Extent({ desc.Width, desc.Height })
    {
        if (desc.Attachments.empty()) 
        {
            VY_THROW_RUNTIME_ERROR("VyFramebuffer must have at least one attachment");
        }

        VkFramebufferCreateInfo framebufferInfo{ VKInit::framebufferCreateInfo() };
        {
            framebufferInfo.flags           = desc.Flags;
            
            framebufferInfo.renderPass      = renderPass.handle();
            
            framebufferInfo.attachmentCount = static_cast<U32>(desc.Attachments.size());
            framebufferInfo.pAttachments    = desc.Attachments.data();

            framebufferInfo.width           = m_Extent.width;
            framebufferInfo.height          = m_Extent.height;
            framebufferInfo.layers          = desc.Layers;
        }

        if (vkCreateFramebuffer(VyContext::device(), &framebufferInfo, nullptr, &m_Framebuffer) != VK_SUCCESS) 
        {
            VY_THROW_RUNTIME_ERROR("Failed to create framebuffer");
        }
    }


    VyFramebuffer::~VyFramebuffer() 
    {
        destroy();
    }


	VyFramebuffer::VyFramebuffer(VyFramebuffer&& other) noexcept
	{
		m_Framebuffer = other.m_Framebuffer;
		m_Extent      = other.m_Extent;

		other.m_Framebuffer = VK_NULL_HANDLE;
	}


	VyFramebuffer& VyFramebuffer::operator=(VyFramebuffer&& other) noexcept
	{
		m_Framebuffer = other.m_Framebuffer;
		m_Extent      = other.m_Extent;

		other.m_Framebuffer = VK_NULL_HANDLE;
		
        return *this;
	}


    void VyFramebuffer::destroy()
    {
        if (m_Framebuffer != VK_NULL_HANDLE) 
        {
            vkDestroyFramebuffer(VyContext::device(), m_Framebuffer, nullptr);

            m_Framebuffer = VK_NULL_HANDLE;
        }
    }
}