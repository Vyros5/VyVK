#pragma once


#include <Vy/GFX/Backend/Image/Image.h>
#include <Vy/GFX/Backend/Image/ImageView.h>
#include <Vy/GFX/Backend/Image/Sampler.h>

namespace Vy
{
    struct VyRenderPassDesc
	{
		TVector<VkAttachmentDescription> AttachmentDescriptions;
		VkSubpassDescription             SubpassDescription;
		VkSubpassDependency              SubpassDependency;
	};

	class VyRenderPass
	{
	public:
		VyRenderPass(const VyRenderPassDesc& desc);

        VyRenderPass(VkRenderPassCreateInfo& createInfo);

		virtual ~VyRenderPass();
		
        VyRenderPass(const VyRenderPass& other)            = delete;
		VyRenderPass& operator=(const VyRenderPass& other) = delete;

		VyRenderPass(VyRenderPass&& other)            noexcept;
		VyRenderPass& operator=(VyRenderPass&& other) noexcept;

        operator     VkRenderPass()              { return m_RenderPass; }
		VY_NODISCARD VkRenderPass handle() const { return m_RenderPass; }

        void destroy();
        
	private:
		VkRenderPass m_RenderPass{ VK_NULL_HANDLE };
	};
}