#pragma once


#include <Vy/GFX/Backend/Image/Image.h>
#include <Vy/GFX/Backend/Image/ImageView.h>
#include <Vy/GFX/Backend/Image/Sampler.h>

namespace Vy
{
	enum class VyAttachmentType
	{
		Color,
		Depth,
		DepthStencil,
		Resolve
	};

	struct VyAttachmentDescription
	{
		VyAttachmentType Type{};
		VkFormat Format{};
		VkSampleCountFlagBits Samples = VK_SAMPLE_COUNT_1_BIT;
		VkAttachmentLoadOp LoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		VkAttachmentStoreOp StoreOp = VK_ATTACHMENT_STORE_OP_STORE;
		VkAttachmentLoadOp StencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		VkAttachmentStoreOp StencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		VkImageLayout InitialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		VkImageLayout FinalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		VkClearValue ClearValue{};
	};

	struct VySubpassDescription
	{
		TVector <U32> ColorAttachments;
		Optional<U32> DepthStencilAttachment;
		TVector <U32> InputAttachments;
		TVector <U32> ResolveAttachments;
		TVector <U32> PreserveAttachments;
	};

	struct VyDependencyDescription 
	{
		U32                  SrcSubpass;
		U32                  DstSubpass;
		VkPipelineStageFlags SrcStageMask;
		VkPipelineStageFlags DstStageMask;
		VkAccessFlags        SrcAccessMask;
		VkAccessFlags        DstAccessMask;
		VkDependencyFlags    DependencyFlags;
	};


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


	class VulkanRenderPass
	{
	public:
		explicit VulkanRenderPass(String debugName);

		~VulkanRenderPass();

		void addAttachment(const VyAttachmentDescription& attachment);

		void addSubpass(const VySubpassDescription& subpass);

		void addDependency(const VyDependencyDescription& dependency);

		void build();

		void beginPass(VkCommandBuffer cmdBuffer, VkRenderPassBeginInfo beginInfo, VkExtent2D extent);
		
		void endPass(VkCommandBuffer cmdBuffer);

		VkRenderPass handle() const { return m_RenderPass; }

		U32 colorAttachmentCount();
		
		static bool formatIsDepth(VkFormat format);

		const TVector<VyAttachmentDescription>& attachmentDescriptions() const { return m_Attachments; }

	private:
		String m_DebugName;
		TVector<VyAttachmentDescription> m_Attachments;
		TVector<VySubpassDescription> m_Subpasses;
		TVector<VkSubpassDependency> m_Dependencies;
		TVector<VkClearValue> m_AttachmentClearValues;
		VkRenderPass m_RenderPass = VK_NULL_HANDLE;

		void createRenderPass();
	};
}


// namespace Vy
// {

// 	struct FrameBufferAttachment
// 	{
// 		VyImage       Image;
// 		VyImageView   View;
// 		VySampler     Sampler;
// 		VkFormat      Format;
// 		VkImageLayout Layout;

// 		VkDescriptorImageInfo descriptor() const
// 		{
// 			return VkDescriptorImageInfo{ Sampler, View, Layout };
// 		}
// 	};

// 	class VRenderPass
// 	{
// 	public:
// 		VRenderPass(U32 width, U32 height) : 
// 			m_Width { width  },
// 			m_Height{ height }
// 		{
// 		}

// 		virtual ~VRenderPass()
// 		{
// 			vkDestroyRenderPass(VyContext::device(), m_RenderPass, nullptr);
// 			vkDestroyFramebuffer(VyContext::device(), m_Framebuffer, nullptr);
// 		}
// 		VkRenderPass handle() const { return m_RenderPass; }

// 		virtual void createRenderPass()
// 		{
// 			auto attachments    = attachmentDescriptions();
// 			auto attachmentRefs = colorAttachmentRefs();
// 			auto depthRef       = depthAttachmentRef();
// 			auto subpassDescs   = subpassDescriptions(attachmentRefs, depthRef);
// 			auto subpassDeps    = subpassDependencies();

// 			VkRenderPassCreateInfo renderPassInfo{ VKInit::renderPassCreateInfo() };
// 			{
// 				renderPassInfo.attachmentCount = static_cast<U32>(attachments.size());
// 				renderPassInfo.pAttachments    = attachments.data();
				
// 				renderPassInfo.subpassCount    = static_cast<U32>(subpassDescs.size());
// 				renderPassInfo.pSubpasses      = subpassDescs.data();

// 				renderPassInfo.dependencyCount = static_cast<U32>(subpassDeps.size());
// 				renderPassInfo.pDependencies   = subpassDeps.data();
// 			}

// 			if (vkCreateRenderPass(VyContext::device(), &renderPassInfo, nullptr, &m_RenderPass) != VK_SUCCESS)
// 			{
// 				throw std::runtime_error("Failed to create render pass!");
// 			}
// 		}

// 		void createFramebuffer()
// 		{
// 			auto attachments = imageViews();

// 			VkFramebufferCreateInfo framebufferInfo{ VKInit::framebufferCreateInfo() };
// 			{
// 				framebufferInfo.renderPass      = m_RenderPass;
// 				framebufferInfo.attachmentCount = static_cast<U32>(attachments.size());
// 				framebufferInfo.pAttachments    = attachments.data();
// 				framebufferInfo.width           = m_Width;
// 				framebufferInfo.height          = m_Height;
// 				framebufferInfo.layers          = 1;
// 			}

// 			if (vkCreateFramebuffer(VyContext::device(), &framebufferInfo, nullptr, &m_Framebuffer) != VK_SUCCESS)
// 			{
// 				throw std::runtime_error("Failed to create framebuffer!");
// 			}
// 		}

// 		void beginRenderPass(VkCommandBuffer commandBuffer)
// 		{
// 			auto clearVals = this->clearValues();

// 			VkRenderPassBeginInfo renderPassInfo{ VKInit::renderPassBeginInfo() };
// 			{
// 				renderPassInfo.renderPass        = m_RenderPass;
// 				renderPassInfo.framebuffer       = m_Framebuffer;
				
// 				renderPassInfo.renderArea.offset = { 0, 0 };
// 				renderPassInfo.renderArea.extent = { m_Width, m_Height };
// 				renderPassInfo.clearValueCount   = (U32)clearVals.size();
// 				renderPassInfo.pClearValues      = clearVals.data();
// 			}

// 			vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

// 			VKCmd::viewport(commandBuffer, VkExtent2D{ m_Width, m_Height });
// 			VKCmd::scissor (commandBuffer, VkExtent2D{ m_Width, m_Height });
// 		}

// 		void endRenderPass(VkCommandBuffer commandBuffer)
// 		{
// 			vkCmdEndRenderPass(commandBuffer);
// 		}

// 		void nextSubpass(VkCommandBuffer commandBuffer)
// 		{
// 			vkCmdNextSubpass(commandBuffer, VK_SUBPASS_CONTENTS_INLINE);
// 		}

// 	protected:
// 		virtual TVector<VkAttachmentDescription> attachmentDescriptions() const = 0 ;
// 		virtual TVector<VkSubpassDescription>    subpassDescriptions(const TVector<VkAttachmentReference>& attachmentRefs, const VkAttachmentReference& depthRef) const = 0;
// 		virtual TVector<VkSubpassDependency>     subpassDependencies() const = 0;
// 		virtual TVector<VkAttachmentReference>   colorAttachmentRefs() const = 0;
// 		virtual VkAttachmentReference            depthAttachmentRef() const { return VkAttachmentReference{}; }
// 		virtual TVector<VkImageView>             imageViews() const = 0;
// 		virtual TVector<VkClearValue>            clearValues() const = 0;

// 		void createAttachment(FrameBufferAttachment* pAttachment, VkFormat format, VkImageUsageFlags usage, VkImageLayout imageLayout)
// 		{
// 			pAttachment->Format = format;
// 			pAttachment->Layout = imageLayout;

// 			VkImageAspectFlags aspectMask = 0;
// 			if (usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
// 			{
// 				aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
// 			}
// 			if (usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
// 			{
// 				aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

// 				if (format >= VK_FORMAT_D16_UNORM_S8_UINT)
// 				{
// 					aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
// 				}
// 			}

// 			// Attachment Image
// 			{
// 				auto builder = VyImage::Builder{};
// 				{
// 					builder.imageType(VK_IMAGE_TYPE_2D);
// 					builder.format(format);
// 					builder.extent(VkExtent2D{ m_Width, m_Height });
// 					builder.mipLevels(1);
// 					builder.arrayLayers(1);
// 					builder.tiling(VK_IMAGE_TILING_OPTIMAL);
// 					builder.usage(usage);
// 					builder.imageLayout(VK_IMAGE_LAYOUT_UNDEFINED);
// 					builder.sampleCount(VK_SAMPLE_COUNT_1_BIT);
// 					builder.sharingMode(VK_SHARING_MODE_EXCLUSIVE);
// 				}
				
// 				pAttachment->Image = builder.build();
// 			}

// 			// Attachment Image View
// 			{
// 				auto builder = VyImageView::Builder{};
// 				{
// 					builder.viewType(VK_IMAGE_VIEW_TYPE_2D);
// 					builder.format(format);
// 					builder.aspectMask(aspectMask);
// 					builder.mipLevels(0, 1);
// 					builder.arrayLayers(0, 1);
// 				}

// 				pAttachment->View = builder.build(pAttachment->Image);
// 			}

// 			// Attachment Sampler
// 			if(usage & VK_IMAGE_USAGE_SAMPLED_BIT)
// 			{
// 				auto builder = VySampler::Builder{};
// 				{
// 					builder.filters(VK_FILTER_NEAREST);
// 					builder.mipmapMode(VK_SAMPLER_MIPMAP_MODE_LINEAR);
// 					builder.addressMode(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
// 					builder.borderColor(VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE);
// 					builder.lodRange(0.0f, 1.0f);
// 				}

// 				pAttachment->Sampler = builder.build();
// 			}
// 		}

// 	protected:
// 		U32           m_Width  = 0;
// 		U32           m_Height = 0;
// 		VkRenderPass  m_RenderPass{VK_NULL_HANDLE};
// 		VkFramebuffer m_Framebuffer{ VK_NULL_HANDLE };
// 	};


// 	class ShadowPass : public VRenderPass
// 	{
// 	private:
// 		FrameBufferAttachment m_ShadowMap;

// 	protected:
// 		TVector<VkAttachmentDescription> attachmentDescriptions() const override;
// 		TVector<VkSubpassDescription> subpassDescriptions(const TVector<VkAttachmentReference>& attachmentRefs, const VkAttachmentReference& depthRef) const override;
// 		TVector<VkSubpassDependency> subpassDependencies() const override;
// 		TVector<VkAttachmentReference> colorAttachmentRefs() const override;
// 		VkAttachmentReference depthAttachmentRef() const override;
// 		TVector<VkImageView>  imageViews() const override;
// 		TVector<VkClearValue> clearValues() const override;

// 	public:
// 		ShadowPass(U32 w, U32 h);
// 		virtual ~ShadowPass();

// 		VkDescriptorImageInfo shadowMapImageInfo() { return m_ShadowMap.descriptor(); }
// 	};
// }