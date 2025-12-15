#pragma once

#include <Vy/GFX/Backend/Resources/Image.h>
#include <Vy/GFX/Backend/Resources/ImageView.h>
#include <Vy/GFX/Backend/Resources/Sampler.h>
#include <Vy/GFX/Context.h>

// Custom define for better code readability
#define VK_FLAGS_NONE 0

namespace Vy
{
	class FrameBufferBuilder;
	class FrameBuffer
	{
	public:
		using Builder = FrameBufferBuilder;

		FrameBuffer():
			m_Framebuffer(VK_NULL_HANDLE) {}

		FrameBuffer(const VkFramebufferCreateInfo& info, VkAllocationCallbacks* allocator = nullptr)
		{
            VK_CHECK(vkCreateFramebuffer(VyContext::device(), &info, allocator, &m_Framebuffer));
		}

		FrameBuffer(const FrameBuffer&) = delete;
		FrameBuffer& operator=(const FrameBuffer&) = delete;

		FrameBuffer(FrameBuffer&& other) noexcept
		{
			m_Framebuffer	= other.m_Framebuffer;
			other.m_Framebuffer	= VK_NULL_HANDLE;
		}

		FrameBuffer& operator=(FrameBuffer&& other) noexcept
		{
			m_Framebuffer	= other.m_Framebuffer;
			other.m_Framebuffer	= VK_NULL_HANDLE;
			return *this;
		}

		void release(VkDevice device, VkAllocationCallbacks* allocator = nullptr)
		{
			if (m_Framebuffer != VK_NULL_HANDLE)
			{
				vkDestroyFramebuffer(device, m_Framebuffer, allocator);
				m_Framebuffer = VK_NULL_HANDLE;
			}
		}

		inline operator VkFramebuffer() const noexcept { return m_Framebuffer; }
		inline const VkFramebuffer* operator&() const noexcept { return &m_Framebuffer; }

		inline VkFramebuffer handle() const noexcept { return m_Framebuffer; }
		inline const VkFramebuffer* handlePtr() const noexcept { return &m_Framebuffer; }

	private:
		VkFramebuffer m_Framebuffer;
	};

	class FrameBufferBuilder
	{
	public:
		FrameBufferBuilder()
		{
			info.sType			 = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			info.pNext			 = nullptr;
			info.flags			 = 0;
			info.renderPass		 = VK_NULL_HANDLE;
			info.attachmentCount = 0;
			info.pAttachments	 = nullptr;
			info.width			 = 0;
			info.height			 = 0;
			info.layers			 = 1;
		}

		FrameBufferBuilder& setRenderPass(VkRenderPass render_pass)
		{
			info.renderPass = render_pass;
			return *this;
		}

		FrameBufferBuilder& setAttachments(const VkImageView* attachments, uint32_t attachment_count)
		{
			info.attachmentCount = attachment_count;
			info.pAttachments	 = attachments;
			return *this;
		}

		FrameBufferBuilder& setSize(uint32_t width, uint32_t height)
		{
			info.width	= width;
			info.height = height;
			return *this;
		}

		FrameBufferBuilder& setSize(VkExtent2D extent)
		{
			return setSize(extent.width, extent.height);
		}

		FrameBufferBuilder& setLayers(uint32_t layers)
		{
			info.layers = layers;
			return *this;
		}

		const VkFramebufferCreateInfo& getCreateInfo() { return info; }

		FrameBuffer build(VkAllocationCallbacks* allocator = nullptr)
		{
			return FrameBuffer(info, allocator);
		}

	private:
		VkFramebufferCreateInfo info;
	};
}

// namespace Vy
// {
    // struct FrameBufferAttachment 
    // {
    //     VkFormat format;
    //     VkExtent2D extent;
    //     VkImageUsageFlags usage;
    //     VkClearValue clearValue{};
    //     TVector<VkImage> images;
    //     //TVector<VkDeviceMemory> mems;
    //     TVector<VmaAllocation> allocs;
    //     TVector<VkImageView> views;

    //     void clear(Device& device)
    //     {
    //         for (int i = 0; i < images.size(); i++) {
    //             vkDestroyImageView(device.device(), views[i], nullptr);
    //             vmaDestroyImage(device.allocator(), images[i], allocs[i]);
    //         }
    //     }
    // };

    // struct RenderPassAttachment 
    // {
    //     FrameBufferAttachment& attachment;
    //     VkAttachmentDescription description{
    //         0,
    //         VK_FORMAT_UNDEFINED,
    //         VK_SAMPLE_COUNT_1_BIT,
    //         VK_ATTACHMENT_LOAD_OP_CLEAR,
    //         VK_ATTACHMENT_STORE_OP_STORE,
    //         VK_ATTACHMENT_LOAD_OP_DONT_CARE,
    //         VK_ATTACHMENT_STORE_OP_STORE,
    //         VK_IMAGE_LAYOUT_UNDEFINED,
    //         VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    //     };
    // };

    // class RenderPass 
    // {
    // public:
    //     struct Subpass 
    //     {
    //         U32 subpass;
    //         VkSubpassDescription subpassDescription;
    //         TVector<FrameBufferAttachment> colorAttachments;
    //         TVector<FrameBufferAttachment> inputAttachments;
    //         FrameBufferAttachment depthAttachment;
            
    //         // for setting up descriptor sets
    //         TVector<VkImageLayout> inputImageLayouts;

    //         struct CreateInfo 
    //         {
    //             TVector<VkAttachmentReference> colorRef;
    //             TVector<VkAttachmentReference> inputRef;
    //             bool hasDepth;
    //             VkAttachmentReference depthRef;
    //         } createInfo;

    //         VkSubpassDescription& build();
    //     };
        
    //     class Builder 
    //     {
    //     public:
    //         Builder(
    //             VySwapchain& swapChain,
    //             const TVector<RenderPassAttachment>& attachments
    //         );
            
    //         Builder& addSubPass(
    //             const TVector<U32>& colorAttachments,
    //             const TVector<U32>& inputAttachments,
    //             const int depthAttachment,
    //             bool depthTestOnly = false);

    //         Builder& addSubPass(
    //             const TVector<U32>& colorAttachments,
    //             const TVector<U32>& inputAttachments,
    //             const int depthAttachment,
    //             const TVector<VkImageLayout>& colorAttachmentLayouts,
    //             const TVector<VkImageLayout>& inputAttachmentLayouts,
    //             const VkImageLayout depthAttachmentLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

    //         std::unique_ptr<RenderPass> build();

    //     private:
    //         U32 _subpassCount{0};
    //         VySwapchain& _swapChain;
    //         TVector<FrameBufferAttachment> Attachments;
    //         TVector<VkAttachmentDescription> AttachmentDescriptions;
    //         TVector<Subpass> _subpasses;
    //         TVector<VkClearValue> _clearValues;
    //         TVector<VkSubpassDependency> _dependencies;
    //         //TVector<VkPipelineColorBlendAttachmentState> _blendAttachments;
            
    //         void setupDependencies();
            
    //         //void setupSubPassReferences();
            
    //         void createFramebuffers(
    //             TVector<VkFramebuffer>& frameBuffers,
    //             VkRenderPass renderPass,
    //             const TVector<FrameBufferAttachment>& attachMents)
    //         {
    //             if (frameBuffers.size() > 0) 
    //             {
    //                 throw std::runtime_error("Error: Frame Buffers are not empty. Destroy all frame buffers before creation.");
    //             }
    //             frameBuffers.resize(_swapChain.imageCount());
    //             U32 width{ std::numeric_limits<U32>::max() }, height{ std::numeric_limits<U32>::max() };
                
    //             for (auto att : attachMents) 
    //             {
    //                 if (att.extent.width < width) width = att.extent.width;
    //                 if (att.extent.height < height) height = att.extent.height;
    //             }
    //             for (int i = 0; i < _swapChain.imageCount(); i++) 
    //             {
    //                 TVector<VkImageView> imageViews;
    //                 imageViews.reserve(attachMents.size());
    //                 for (auto att : attachMents) 
    //                 {
    //                     if (att.views.size() == _swapChain.imageCount())
    //                     {
    //                         imageViews.push_back(att.views[i]);
    //                     }
    //                     else 
    //                     {
    //                         imageViews.push_back(att.views[0]);
    //                     }
    //                 }
    //                 //VkExtent2D swapChainExtent = _swapChain.getSwapChainExtent();
    //                 VkFramebufferCreateInfo framebufferInfo = {};
    //                 framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    //                 framebufferInfo.renderPass = renderPass;
    //                 framebufferInfo.attachmentCount = static_cast<U32>(attachMents.size());
    //                 framebufferInfo.pAttachments = imageViews.data();
    //                 //framebufferInfo.width = swapChainExtent.width;
    //                 //framebufferInfo.height = swapChainExtent.height;
    //                 framebufferInfo.width = width;
    //                 framebufferInfo.height = height;
    //                 framebufferInfo.layers = 1;

    //                 if (vkCreateFramebuffer(
    //                     _device.device(),
    //                     &framebufferInfo,
    //                     nullptr,
    //                     &frameBuffers[i]) != VK_SUCCESS) {
    //                     throw std::runtime_error("Error: Failed to create framebuffer!");
    //                 }
    //             }
    //         }
    //     };

    //     RenderPass(
    //         VySwapchain& swapChain)
    //         : _swapChain{ swapChain } {}

    //     ~RenderPass() 
    //     {
    //         vkDestroyRenderPass(_device.device(), _pass, nullptr);
            
    //         for (auto framebuffer : frameBuffers) 
    //         {
    //             vkDestroyFramebuffer(_device.device(), framebuffer, nullptr);
    //         }
    //     }

    //     RenderPass(const RenderPass&&) = delete;
    //     void operator=(const RenderPass&&) = delete;

    //     VkRenderPass renderPass() const { return _pass; }
    //     size_t imageCount() const { return attachments[0].images.size(); }

    //     TVector<VkFramebuffer> frameBuffers;
    //     TVector<Subpass> subpasses;
    //     TVector<FrameBufferAttachment> attachments;
    //     TVector<VkClearValue> clearValues;
    //     TVector<VkPipelineColorBlendAttachmentState> blendAttachments;
    // private:
    //     VySwapchain& _swapChain;
    //     VkRenderPass _pass;
    // };

    // class RenderPass final
// 	{
// 	public:
// 		//--------------------------------------------------
// 		//    Constructor & Destructor
// 		//--------------------------------------------------
// 		explicit RenderPass() = default;
// 		~RenderPass() = default;
		
//         RenderPass(const RenderPass& other) = delete;
// 		RenderPass(RenderPass&& other) noexcept = delete;

// 		RenderPass& operator=(const RenderPass& other) = delete;
// 		RenderPass& operator=(RenderPass&& other) noexcept = delete;
// 		void Destroy() const;

// 		//--------------------------------------------------
// 		//    Accessors & Mutators
// 		//--------------------------------------------------
// 		const VkRenderPass& GetHandle() const;
// 		U32 GetAttachmentCount() const;

// 	private:
// 		VkRenderPass m_RenderPass;
// 		U32     m_AttachmentCount;

// 		friend class RenderPassBuilder;
// 	};

// 	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 	//? ~~	  RenderPassBuilder
// 	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 	class RenderPassBuilder final
// 	{
// 	public:
// 		//--------------------------------------------------
// 		//    Constructor & Destructor
// 		//--------------------------------------------------
// 		RenderPassBuilder() = default;


// 		//--------------------------------------------------
// 		//    Builder
// 		//--------------------------------------------------
// 		RenderPassBuilder& NewSubpass()
//         {
//             m_SubPassIdx = static_cast<U32>(m_vSubPasses.size());
//             m_vSubPasses.emplace_back();
//             m_vSubPasses.back().colorAttachmentCount = 0;
//             m_vSubPasses.back().pColorAttachments = nullptr;
//             m_vSubPasses.back().pDepthStencilAttachment = nullptr;
//             m_vSubPasses.back().pResolveAttachments = nullptr;

//             return *this;
//         }
// 		RenderPassBuilder& SetBindPoint(VkPipelineBindPoint bindPoint) 
//         { 
//             m_vSubPasses.back().pipelineBindPoint = bindPoint; 
//             return *this; 
//         }

// 		RenderPassBuilder& NewAttachment() 
//         { 
//             m_vAttachmentDescriptions.emplace_back(); 
//             return *this; 
//         }
// 		RenderPassBuilder& SetFormat(VkFormat format)
//         { 
//             m_vAttachmentDescriptions.back().format = format; return *this; 
//         }

// 		RenderPassBuilder& SetSamples(VkSampleCountFlagBits samples)
//         { 
//             m_vAttachmentDescriptions.back().samples = samples; return *this; 
//         }

// 		RenderPassBuilder& SetLoadStoreOp(VkAttachmentLoadOp loadOp, VkAttachmentStoreOp storeOp)
//         {
//             m_vAttachmentDescriptions.back().loadOp = loadOp;
//             m_vAttachmentDescriptions.back().storeOp = storeOp;
//             return *this;
//         }
// 		RenderPassBuilder& SetStencilLoadStoreOp(VkAttachmentLoadOp loadOp, VkAttachmentStoreOp storeOp)
//         {
//             m_vAttachmentDescriptions.back().stencilLoadOp = loadOp;
//             m_vAttachmentDescriptions.back().stencilStoreOp = storeOp;
//             return *this;
//         }
		
//         RenderPassBuilder& SetInitialLayout(VkImageLayout layout)
//         { 
//             m_vAttachmentDescriptions.back().initialLayout = layout; 
//             return *this; 
//         }

// 		RenderPassBuilder& SetFinalLayout(VkImageLayout layout)
//         { 
//             m_vAttachmentDescriptions.back().finalLayout = layout; 
//             return *this; 
//         }
		
//         RenderPassBuilder& AddSubpassColorAttachment(U32 attachment)
//         {
//             if (m_vColorAttachmentRefs.size() <= m_SubPassIdx)
//                 m_vColorAttachmentRefs.emplace_back();
//             m_vColorAttachmentRefs.back().emplace_back(attachment, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
//             m_vSubPasses.back().colorAttachmentCount = static_cast<U32>(m_vColorAttachmentRefs.back().size());
//             m_vSubPasses.back().pColorAttachments = m_vColorAttachmentRefs.back().data();
//             return *this;
//         }
// 		RenderPassBuilder& SetSubpassResolveAttachment(U32 attachment)
//         {
//             m_ResolveAttachmentRef.emplace_back(attachment, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
//             m_vSubPasses.back().pResolveAttachments = &m_ResolveAttachmentRef.back();
//             return *this;
//         }
// 		RenderPassBuilder& SetSubpassDepthAttachment(U32 attachment)
//         {
//             m_vDepthAttachmentRefs.emplace_back(attachment, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
//             m_vSubPasses.back().pDepthStencilAttachment = &m_vDepthAttachmentRefs.back();
//             return *this;
//         }

		
//         RenderPassBuilder& NewDependency()
//         { 
//             m_vSubPassDependencies.emplace_back(); return *this; 
//         }

// 		RenderPassBuilder& dependencyFlag(VkDependencyFlags flags)
//         { 
//             m_vSubPassDependencies.back().dependencyFlags = flags; return *this; 
//         }

// 		RenderPassBuilder& srcSubPass(U32 subpass)
//         { 
//             m_vSubPassDependencies.back().srcSubpass = subpass; return *this; 
//         }

// 		RenderPassBuilder& dstSubPass(U32 subpass)
//         { 
//             m_vSubPassDependencies.back().dstSubpass = subpass; return *this; 
//         }

// 		RenderPassBuilder& srcMasks(VkPipelineStageFlags stageFlags, VkAccessFlags accessFlags)
//         {
//             m_vSubPassDependencies.back().srcStageMask = stageFlags;
//             m_vSubPassDependencies.back().srcAccessMask = accessFlags;
//             return *this;
//         }

// 		RenderPassBuilder& dstMasks(VkPipelineStageFlags stageFlags, VkAccessFlags accessFlags)
//         {
//             m_vSubPassDependencies.back().dstStageMask = stageFlags;
//             m_vSubPassDependencies.back().dstAccessMask = accessFlags;
//             return *this;
//         }

// 		void build(RenderPass& renderPass) const
//         {
//             renderPass.m_AttachmentCount = static_cast<U32>(m_vAttachmentDescriptions.size());

//             VkRenderPassCreateInfo renderPassInfo{};
//             {
//                 renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
//                 renderPassInfo.attachmentCount = renderPass.m_AttachmentCount;
//                 renderPassInfo.pAttachments = m_vAttachmentDescriptions.data();
//                 renderPassInfo.subpassCount = static_cast<U32>(m_vSubPasses.size());
//                 renderPassInfo.pSubpasses = m_vSubPasses.data();
//                 renderPassInfo.dependencyCount = static_cast<U32>(m_vSubPassDependencies.size());
//                 renderPassInfo.pDependencies = m_vSubPassDependencies.data();
//             }

//             if (vkCreateRenderPass(VyContext::device(), &renderPassInfo, nullptr, &renderPass.m_RenderPass) != VK_SUCCESS)
//             {
//                 throw std::runtime_error("Failed to created Render Pass!");
//             }
//         }


// 	private:
// 		TVector<VkAttachmentDescription>	m_vAttachmentDescriptions{};
// 		TVector<VkSubpassDescription>		m_vSubPasses{};
// 		TVector<VkSubpassDependency>		m_vSubPassDependencies{};
// 		U32								m_SubPassIdx{};

// 		TVector<VkAttachmentReference>				m_ResolveAttachmentRef{ };
// 		TVector<VkAttachmentReference>				m_vDepthAttachmentRefs{ };
// 		TVector<TVector<VkAttachmentReference>>	m_vColorAttachmentRefs{ };
// 	};
// }


// namespace Vy
// {
// //     class VulkanFramebuffer
// //     {
// //     public: 
// //         VkFramebuffer m_Framebuffer = 0;

// //         VulkanFramebuffer(){}

// //         VulkanFramebuffer(const VkFramebufferCreateInfo& info)
// //         {
// //             auto result = vkCreateFramebuffer(VyContext::device(), &info, nullptr, &m_Framebuffer);
// //             switch (result) {
// //             case VK_ERROR_OUT_OF_HOST_MEMORY:
// //                 throw std::runtime_error("vkCreateFramebuffer: failed, ran out of host memory");
// //             case VK_ERROR_OUT_OF_DEVICE_MEMORY:
// //                 throw std::runtime_error("vkCreateFramebuffer: failed, ran out of device memory");
// //             case VK_SUCCESS:
// //                 break;
// //             }
// //         }

// //         ~VulkanFramebuffer() 
// //         {
// //             if (m_Framebuffer != 0)
// //             {
// //                 vkDestroyFramebuffer(VyContext::device(), m_Framebuffer, nullptr);
// //             }
// //         }

// //         VulkanFramebuffer(const VulkanFramebuffer& other)            = delete;
// //         VulkanFramebuffer& operator=(const VulkanFramebuffer& other) = delete;

// //         VulkanFramebuffer(VulkanFramebuffer&& other) noexcept : 
// //             m_Framebuffer(other.m_Framebuffer)
// //         {
// //             other.m_Framebuffer = 0;
// //         }

// //         VulkanFramebuffer& operator=(VulkanFramebuffer&& other) noexcept
// //         {
// //             if (m_Framebuffer != 0)
// //             {
// //                 vkDestroyFramebuffer(VyContext::device(), m_Framebuffer, nullptr);
// //             }

// //             m_Framebuffer = other.m_Framebuffer;
            
// //             other.m_Framebuffer = 0;
// //         }
        

// //         class Builder
// //         {
// //         public:
// //             VkFramebufferCreateInfo m_Info;
// //             TVector<VkImageView>    m_Attachments;

// //             Builder()
// //             {
// //                 m_Info = VKInit::framebufferCreateInfo();
// //                 {
// //                     m_Info.pNext           = nullptr;
// //                     m_Info.flags           = 0;
// //                     m_Info.renderPass      = VK_NULL_HANDLE;
// //                     m_Info.attachmentCount = 0;
// //                     m_Info.pAttachments    = nullptr;
// //                     m_Info.width           = 0;
// //                     m_Info.height          = 0;
// //                     m_Info.layers          = 1;
// //                 }
// //             }

// //             Builder& attachment   (VyImageView& imageView)   { return addAttachment(imageView);           }
// //             Builder& addAttachment(VyImageView& imageView)   { return addAttachment(imageView.handle());  }
// //             Builder& attachment   (VkImageView& imageView)   { return addAttachment(imageView);           }
// //             Builder& addAttachment(VkImageView  imageView) 
// //             { 
// //                 m_Attachments.push_back(imageView); 
                
// //                 m_Info.pAttachments    = m_Attachments.data(); 
// //                 m_Info.attachmentCount = m_Attachments.size(); 
// //                 return *this; 
// //             }

// //             Builder& renderPass   (VyRenderPass& renderpass) { return setRenderPass(renderpass);          }
// //             Builder& setRenderPass(VyRenderPass& renderpass) { return setRenderPass(renderpass.handle()); }
// //             Builder& renderPass   (VkRenderPass& renderpass) { return setRenderPass(renderpass);          }
// //             Builder& setRenderPass(VkRenderPass  renderpass) 
// //             { 
// //                 m_Info.renderPass = renderpass; 
// //                 return *this; 
// //             }

// //             Builder& size  (U32 wid, U32 hgt) { return width(wid).height(hgt); }
// //             Builder& width (U32 wid)          { m_Info.width  = wid;   return *this; }
// //             Builder& height(U32 hgt)          { m_Info.height = hgt;   return *this; }
// //             Builder& layers(U32 count)        { m_Info.layers = count; return *this; }

// //             Builder& imageless() { return setFlags(VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT_KHR); }
// //             Builder& setFlags(VkFramebufferCreateFlags flags) { m_Info.flags  = flags; return *this; }

// //             Unique<VulkanFramebuffer> buildUnique() 
// //             {
// //                 return MakeUnique<VulkanFramebuffer>(m_Info);
// //             }

// //             VulkanFramebuffer build() 
// //             {
// //                 return VulkanFramebuffer(m_Info);
// //             }
            
// //             operator VulkanFramebuffer() 
// //             {
// //                 return build();
// //             }
// //         };
// //     };
// // }


// namespace Vy
// {
// 	/**
// 	 * @brief Encapsulates a single frame buffer attachment
// 	 */
// 	struct FramebufferAttachment 
//     {
// 		VkImage                 Image{};
// 		VkDeviceMemory          memory{};
// 		VkImageView             View{};
// 		VkFormat                Format{};
// 		VkImageSubresourceRange SubresourceRange{};
// 		VkAttachmentDescription Description{};

// 		/**
// 		 * @brief Returns true if the attachment has a depth component
// 		 */
// 		bool hasDepth() 
//         {
// 			TVector<VkFormat> formats =
// 			    {
// 			        VK_FORMAT_D16_UNORM,
// 			        VK_FORMAT_X8_D24_UNORM_PACK32,
// 			        VK_FORMAT_D32_SFLOAT,
// 			        VK_FORMAT_D16_UNORM_S8_UINT,
// 			        VK_FORMAT_D24_UNORM_S8_UINT,
// 			        VK_FORMAT_D32_SFLOAT_S8_UINT,
// 			    };
// 			return std::find(formats.begin(), formats.end(), Format) != std::end(formats);
// 		}

// 		/**
// 		 * @brief Returns true if the attachment has a stencil component
// 		 */
// 		bool hasStencil() 
//         {
// 			TVector<VkFormat> formats =
// 			    {
// 			        VK_FORMAT_S8_UINT,
// 			        VK_FORMAT_D16_UNORM_S8_UINT,
// 			        VK_FORMAT_D24_UNORM_S8_UINT,
// 			        VK_FORMAT_D32_SFLOAT_S8_UINT,
// 			    };
// 			return std::find(formats.begin(), formats.end(), Format) != std::end(formats);
// 		}

// 		/**
// 		 * @brief Returns true if the attachment is a depth and/or stencil attachment
// 		 */
// 		bool isDepthStencil() 
//         {
// 			return (hasDepth() || hasStencil());
// 		}
// 	};



// 	/**
// 	 * @brief Describes the attributes of an attachment to be created
// 	 */
// 	struct AttachmentCreateInfo 
//     {
// 		U32                   Width;
//         U32                   Height;
// 		U32                   LayerCount = 1;
// 		VkFormat              Format{};
// 		VkImageUsageFlags     Usage{};
// 		VkAttachmentLoadOp    LoadOp{};
// 		VkAttachmentStoreOp   StoreOp{};
// 		VkAttachmentLoadOp    StencilLoadOp{};
// 		VkAttachmentStoreOp   StencilStoreOp{};
// 		VkImageLayout         InitialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
// 		VkSampleCountFlagBits ImageSampleCount = VK_SAMPLE_COUNT_1_BIT;
// 		VkImageCreateFlags    Flags{};
// 	};

// 	/**
// 	 * @brief Describes the attributes of an attachment to be added
// 	 */
// 	struct AttachmentAddInfo 
//     {
// 		U32                   Width;
//         U32                   Height;
// 		U32                   LayerCount;
// 		VkFormat              Format;
// 		VkAttachmentLoadOp    LoadOp;
// 		VkAttachmentStoreOp   StoreOp;
// 		VkAttachmentLoadOp    StencilLoadOp;
// 		VkAttachmentStoreOp   StencilStoreOp;
// 		VkImageUsageFlags     Usage;
// 		VkImageLayout         InitialLayout;
// 		VkSampleCountFlagBits ImageSampleCount = VK_SAMPLE_COUNT_1_BIT;
// 		VkImageView           View;
// 	};

// 	/**
// 	 * @brief Encapsulates a complete Vulkan framebuffer with an arbitrary number and combination of attachments
// 	 */
// 	struct Framebuffer 
//     {
// 	public:
// 		U32                            m_Width;
//         U32                            m_Height;
// 		VkFramebuffer                  m_Framebuffer{};
// 		VkRenderPass                   m_RenderPass{};
// 		VkSampler                      m_Sampler{};
// 		TVector<FramebufferAttachment> m_Attachments{};

// 		/**
// 		 * Default constructor
// 		 */
// 		Framebuffer() {}

// 		/**
// 		 * Destroy and free Vulkan resources used for the framebuffer and all of its attachments
// 		 */
// 		~Framebuffer() 
//         {
// 			for (auto attachment : m_Attachments) 
//             {
// 				// Only destroy attachments if they are not empty (attachments may just contain loaded data from another Framebuffer).
// 				if (attachment.Image) 
//                 {
// 					vkDestroyImage(VyContext::device(), attachment.Image, nullptr);
// 					vkDestroyImageView(VyContext::device(), attachment.View, nullptr);
// 					vkFreeMemory(VyContext::device(), attachment.memory, nullptr);
// 				}
// 			}

// 			vkDestroySampler(VyContext::device(), m_Sampler, nullptr);
// 			vkDestroyRenderPass(VyContext::device(), m_RenderPass, nullptr);
// 			vkDestroyFramebuffer(VyContext::device(), m_Framebuffer, nullptr);
// 		}

// 		/**
// 		 * Will use the framebuffer data from an already existing framebuffer.
// 		 * To be used when multiple systems share the same render pass.
// 		 *
// 		 * @param frameBuffer A reference to the framebuffer to be looaded.
// 		 */
// 		void loadFramebuffer(Framebuffer& frameBuffer) 
//         {
// 		}

// 		/**
// 		 * Will empty the framebuffer, allowing for safe recreation.
// 		 */
// 		void clearFramebuffer() 
//         {
// 			for (auto attachment : m_Attachments) 
//             {
// 				// Only destroy attachments if they are not empty (attachments may just contain loaded data from another Framebuffer).
// 				if (attachment.Image) 
//                 {
// 					vkDestroyImage(VyContext::device(), attachment.Image, nullptr);
// 					vkDestroyImageView(VyContext::device(), attachment.View, nullptr);
// 					vkFreeMemory(VyContext::device(), attachment.memory, nullptr);
// 				}
// 			}
// 			m_Attachments.clear();

// 			vkDestroySampler(VyContext::device(), m_Sampler, nullptr);
// 			vkDestroyRenderPass(VyContext::device(), m_RenderPass, nullptr);
// 			vkDestroyFramebuffer(VyContext::device(), m_Framebuffer, nullptr);
// 		}

// 		/**
// 		 * Create a new attachment described by createinfo to the framebuffer's attachment list.
// 		 * This will also create a Image and Image View to go with the attachment.
// 		 *
// 		 * @param createinfo Structure that specifies the framebuffer to be constructed
// 		 *
// 		 * @return Index of the new attachment
// 		 */
// 		U32 createAttachment(AttachmentCreateInfo createinfo) 
//         {
// 			FramebufferAttachment attachment;

// 			// TODO: Is there a better way to save the width and height for the Framebuffer?
// 			m_Width  = createinfo.Width;
// 			m_Height = createinfo.Height;
// 			attachment.Format = createinfo.Format;

// 			VkImageAspectFlags aspectMask = VK_FLAGS_NONE;

// 			// Select aspect mask and layout depending on Usage

// 			// Color attachment
// 			if (createinfo.Usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) 
//             {
// 				aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
// 			}

// 			// Depth (and/or stencil) attachment
// 			if (createinfo.Usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) 
//             {
// 				if (attachment.hasDepth()) 
//                 {
// 					aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
// 				}
// 				if (attachment.hasStencil()) 
//                 {
// 					aspectMask = aspectMask | VK_IMAGE_ASPECT_STENCIL_BIT;
// 				}
// 			}

// 			assert(aspectMask > 0);

// 			VkImageCreateInfo imageInfo{};
// 			imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
// 			imageInfo.imageType = VK_IMAGE_TYPE_2D;
// 			imageInfo.format = createinfo.Format;
// 			imageInfo.extent.width = createinfo.Width;
// 			imageInfo.extent.height = createinfo.Height;
// 			imageInfo.extent.depth = 1;
// 			imageInfo.mipLevels = 1;
// 			imageInfo.arrayLayers = createinfo.LayerCount;
// 			imageInfo.samples = createinfo.ImageSampleCount;
// 			imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
// 			imageInfo.usage = createinfo.Usage;
// 			imageInfo.flags = createinfo.Flags;

// 			VkMemoryAllocateInfo memAlloc{};
// 			memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
// 			VkMemoryRequirements memReqs{};

// 			// Create image for this attachment
// 			VK_CHECK(vkCreateImage(VyContext::device(), &imageInfo, nullptr, &attachment.Image));
// 			vkGetImageMemoryRequirements(VyContext::device(), attachment.Image, &memReqs);
// 			memAlloc.allocationSize = memReqs.size;
// 			memAlloc.memoryTypeIndex = VyContext::device().findMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
// 			VK_CHECK(vkAllocateMemory(VyContext::device(), &memAlloc, nullptr, &attachment.memory));
// 			VK_CHECK(vkBindImageMemory(VyContext::device(), attachment.Image, attachment.memory, 0));

// 			attachment.SubresourceRange = {};
//             {
//                 attachment.SubresourceRange.aspectMask = aspectMask;
//                 attachment.SubresourceRange.levelCount = 1;
//                 attachment.SubresourceRange.layerCount = createinfo.LayerCount;
//             }

// 			VkImageViewCreateInfo imageView{};
//             {
//                 imageView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
//                 imageView.viewType = (createinfo.LayerCount == 1) ? VK_IMAGE_VIEW_TYPE_2D : (createinfo.LayerCount == 6) ? VK_IMAGE_VIEW_TYPE_CUBE
//                 : VK_IMAGE_VIEW_TYPE_2D_ARRAY;
//                 imageView.format = createinfo.Format;
//                 imageView.subresourceRange = attachment.SubresourceRange;
//                 imageView.image = attachment.Image;
//             }
// 			VK_CHECK(vkCreateImageView(VyContext::device(), &imageView, nullptr, &attachment.View));

// 			// Fill attachment Description
// 			attachment.Description = {};
//             {
//                 attachment.Description.samples = createinfo.ImageSampleCount;
//                 attachment.Description.loadOp = createinfo.LoadOp;
//                 // attachment.Description.storeOp = (createinfo.Usage & VK_IMAGE_USAGE_SAMPLED_BIT) ? VK_ATTACHMENT_STORE_OP_STORE : VK_ATTACHMENT_STORE_OP_DONT_CARE;
//                 attachment.Description.storeOp = createinfo.StoreOp;
//                 attachment.Description.stencilLoadOp = createinfo.StencilLoadOp;
//                 attachment.Description.stencilStoreOp = createinfo.StencilStoreOp;
//                 attachment.Description.format = createinfo.Format;
//                 attachment.Description.initialLayout = createinfo.InitialLayout;
//                 // Final layout
//                 // If not, final layout depends on attachment type
//                 if (attachment.hasDepth() || attachment.hasStencil()) 
//                 {
//                     attachment.Description.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
//                 } 
//                 else {
//                     attachment.Description.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
//                 }
//             }

// 			m_Attachments.push_back(attachment);

// 			return static_cast<U32>(m_Attachments.size() - 1);
// 		}

// 		/**
// 		 * Add a load attachment described by createinfo to the framebuffer's attachment list.
// 		 * This will not create an image or image View. Suitable for using attachments/images from previous render passes.
// 		 *
// 		 * @param addInfo Structure that specifies the framebuffer to be constructed
// 		 *
// 		 * @return Index of the new attachment
// 		 */
// 		U32 addLoadAttachment(AttachmentAddInfo addInfo) 
//         {
// 			FramebufferAttachment attachment;

// 			m_Width           = addInfo.Width;
// 			m_Height          = addInfo.Height;
// 			attachment.Format = addInfo.Format;
// 			attachment.View   = addInfo.View;

// 			VkImageAspectFlags aspectMask = VK_FLAGS_NONE;

// 			// Select aspect mask and layout depending on Usage

// 			// Color attachment
// 			if (addInfo.Usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) 
//             {
// 				aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
// 			}

// 			// Depth (and/or stencil) attachment
// 			if (addInfo.Usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) 
//             {
// 				if (attachment.hasDepth()) 
//                 {
// 					aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
// 				}
// 				if (attachment.hasStencil()) 
//                 {
// 					aspectMask = aspectMask | VK_IMAGE_ASPECT_STENCIL_BIT;
// 				}
// 			}

// 			attachment.SubresourceRange = {};
//             {
//                 attachment.SubresourceRange.aspectMask = aspectMask;
//                 attachment.SubresourceRange.levelCount = 1;
//                 attachment.SubresourceRange.layerCount = addInfo.LayerCount;
//             }

// 			assert(aspectMask > 0);

// 			// Fill attachment Description
// 			attachment.Description = {};
//             {
//                 attachment.Description.samples = addInfo.ImageSampleCount;
//                 // attachment.Description.loadOp = (addInfo.Usage & (VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)) ? VK_ATTACHMENT_LOAD_OP_LOAD : VK_ATTACHMENT_LOAD_OP_CLEAR;
//                 // attachment.Description.storeOp = (addInfo.Usage & VK_IMAGE_USAGE_SAMPLED_BIT) ? VK_ATTACHMENT_STORE_OP_STORE : VK_ATTACHMENT_STORE_OP_DONT_CARE;
//                 attachment.Description.loadOp = addInfo.LoadOp;
//                 attachment.Description.storeOp = addInfo.StoreOp;
//                 attachment.Description.stencilLoadOp = addInfo.StencilLoadOp;
//                 attachment.Description.stencilStoreOp = addInfo.StencilStoreOp;
//                 attachment.Description.format = addInfo.Format;
//                 attachment.Description.initialLayout = addInfo.InitialLayout;
//             }

// 			// Final layout
// 			// If not, final layout depends on attachment type
// 			if (attachment.hasDepth() || attachment.hasStencil()) 
//             {
// 				attachment.Description.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
// 			} 
//             else {
// 				attachment.Description.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
// 			}

// 			m_Attachments.push_back(attachment);

// 			return static_cast<U32>(m_Attachments.size() - 1);
// 		}

// 		/**
// 		 * Creates a default sampler for sampling from any of the framebuffer attachments
// 		 * Applications are free to create their own samplers for different use cases
// 		 *
// 		 * @param magFilter Magnification filter for lookups
// 		 * @param minFilter Minification filter for lookups
// 		 * @param adressMode Addressing mode for the U,V and W coordinates
// 		 *
// 		 * @return VkResult for the sampler creation
// 		 */
// 		VkResult createSampler(VkFilter magFilter, VkFilter minFilter, VkSamplerAddressMode adressMode) 
//         {
// 			VkSamplerCreateInfo samplerInfo{};
// 			samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
// 			samplerInfo.magFilter = magFilter;
// 			samplerInfo.minFilter = minFilter;
// 			samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
// 			samplerInfo.addressModeU = adressMode;
// 			samplerInfo.addressModeV = adressMode;
// 			samplerInfo.addressModeW = adressMode;
// 			samplerInfo.unnormalizedCoordinates = false;
// 			samplerInfo.mipLodBias = 0.0f;
// 			samplerInfo.maxAnisotropy = 1.0f;
// 			samplerInfo.minLod = 0.0f;
// 			samplerInfo.maxLod = 1.0f;
// 			samplerInfo.compareEnable = VK_TRUE;
// 			samplerInfo.compareOp = VK_COMPARE_OP_LESS;
// 			samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
// 			return vkCreateSampler(VyContext::device(), &samplerInfo, nullptr, &m_Sampler);
// 		}

// 		/**
// 		 * Creates a default render pass setup with one sub pass
// 		 *
// 		 * @return VK_SUCCESS if all resources have been created successfully
// 		 */
// 		VkResult createRenderPass() 
//         {
// 			TVector<VkAttachmentDescription> attachmentDescriptions;
			
//             for (auto& attachment : m_Attachments) 
//             {
// 				attachmentDescriptions.push_back(attachment.Description);
// 			};

// 			// Collect attachment references
// 			TVector<VkAttachmentReference> colorReferences;
// 			VkAttachmentReference depthReference{};
// 			bool hasDepth = false;
// 			bool hasColor = false;

// 			U32 attachmentIndex = 0;

// 			for (auto& attachment : m_Attachments) 
//             {
// 				if (attachment.isDepthStencil()) 
//                 {
// 					// Only one depth attachment allowed
// 					assert(!hasDepth);
// 					depthReference.attachment = attachmentIndex;
// 					depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
// 					hasDepth = true;
// 				} 
//                 else {
// 					colorReferences.push_back({attachmentIndex, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});
// 					hasColor = true;
// 				}

// 				attachmentIndex++;
// 			};

// 			// Default render pass setup uses only one subpass
// 			VkSubpassDescription subpass{};
// 			subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
// 			if (hasColor) 
//             {
// 				subpass.pColorAttachments = colorReferences.data();
// 				subpass.colorAttachmentCount = static_cast<U32>(colorReferences.size());
// 			}
// 			if (hasDepth) 
//             {
// 				subpass.pDepthStencilAttachment = &depthReference;
// 			}

// 			// Use subpass dependencies for attachment layout transitions
// 			std::array<VkSubpassDependency, 2> dependencies{};
//             {
//                 dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
//                 dependencies[0].dstSubpass = 0;
//                 dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
//                 dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
//                 dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
//                 dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
//                 dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
                
//                 dependencies[1].srcSubpass = 0;
//                 dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
//                 dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
//                 dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
//                 dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
//                 dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
//                 dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
//             }

// 			// Create render pass
// 			VkRenderPassCreateInfo renderPassInfo = {};
//             {
//                 renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
//                 renderPassInfo.pAttachments = attachmentDescriptions.data();
//                 renderPassInfo.attachmentCount = static_cast<U32>(attachmentDescriptions.size());
//                 renderPassInfo.subpassCount = 1;
//                 renderPassInfo.pSubpasses = &subpass;
//                 renderPassInfo.dependencyCount = 2;
//                 renderPassInfo.pDependencies = dependencies.data();
//             }

//             VK_CHECK(vkCreateRenderPass(VyContext::device(), &renderPassInfo, nullptr, &m_RenderPass));

// 			TVector<VkImageView> attachmentViews;
// 			for (auto attachment : m_Attachments) 
//             {
// 				attachmentViews.push_back(attachment.View);
// 			}

// 			// Find. max number of layers across attachments
// 			U32 maxLayers = 0;
// 			for (auto attachment : m_Attachments) 
//             {
// 				if (attachment.SubresourceRange.layerCount > maxLayers) 
//                 {
// 					maxLayers = attachment.SubresourceRange.layerCount;
// 				}
// 			}

// 			VkFramebufferCreateInfo framebufferInfo = {};
// 			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
// 			framebufferInfo.renderPass = m_RenderPass;
// 			framebufferInfo.pAttachments = attachmentViews.data();
// 			framebufferInfo.attachmentCount = static_cast<U32>(attachmentViews.size());
// 			framebufferInfo.width = m_Width;
// 			framebufferInfo.height = m_Height;
// 			framebufferInfo.layers = maxLayers;
// 			VK_CHECK(vkCreateFramebuffer(VyContext::device(), &framebufferInfo, nullptr, &m_Framebuffer));

// 			return VK_SUCCESS;
// 		}

// 		/**
// 		 * Creates a default multi-view render pass setup with one sub pass
// 		 *
// 		 * @return VK_SUCCESS if all resources have been created successfully
// 		 */
// 		VkResult createMultiViewRenderPass(U32 multiViewCount) 
//         {
// 			TVector<VkAttachmentDescription> attachmentDescriptions;
// 			for (auto& attachment : m_Attachments) 
//             {
// 				attachmentDescriptions.push_back(attachment.Description);
// 			};

// 			// Collect attachment references
// 			TVector<VkAttachmentReference> colorReferences;
// 			VkAttachmentReference depthReference{};
// 			bool hasDepth = false;
// 			bool hasColor = false;

// 			U32 attachmentIndex = 0;

// 			for (auto& attachment : m_Attachments) {
// 				if (attachment.isDepthStencil()) {
// 					// Only one depth attachment allowed
// 					assert(!hasDepth);
// 					depthReference.attachment = attachmentIndex;
// 					depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
// 					hasDepth = true;
// 				} else {
// 					colorReferences.push_back({attachmentIndex, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});
// 					hasColor = true;
// 				}
// 				attachmentIndex++;
// 			};

// 			// Default render pass setup uses only one subpass
// 			VkSubpassDescription subpass{};
// 			subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
// 			if (hasColor) {
// 				subpass.pColorAttachments = colorReferences.data();
// 				subpass.colorAttachmentCount = static_cast<U32>(colorReferences.size());
// 			}
// 			if (hasDepth) {
// 				subpass.pDepthStencilAttachment = &depthReference;
// 			}

// 			// Use subpass dependencies for attachment layout transitions
// 			std::array<VkSubpassDependency, 2> dependencies{};

// 			dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
// 			dependencies[0].dstSubpass = 0;
// 			dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
// 			dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
// 			dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
// 			dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
// 			dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

// 			dependencies[1].srcSubpass = 0;
// 			dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
// 			dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
// 			dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
// 			dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
// 			dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
// 			dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

// 			// Create render pass
// 			VkRenderPassCreateInfo renderPassInfo = {};
// 			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
// 			renderPassInfo.pAttachments = attachmentDescriptions.data();
// 			renderPassInfo.attachmentCount = static_cast<U32>(attachmentDescriptions.size());
// 			renderPassInfo.subpassCount = 1;
// 			renderPassInfo.pSubpasses = &subpass;
// 			renderPassInfo.dependencyCount = 2;
// 			renderPassInfo.pDependencies = dependencies.data();

// 			/*
// 			    Setup multiview info for the renderpass
// 			*/

// 			/*
// 			    Bit mask that specifies which view rendering is broadcast to
// 			    0011 = Broadcast to first and second view (layer)
// 			*/
// 			const U32 viewMask = 0b00111111;

// 			/*
// 			    Bit mask that specifies correlation between views
// 			    An implementation may use this for optimizations (concurrent render)
// 			*/
// 			const U32 correlationMask = 0b00111111;

// 			VkRenderPassMultiviewCreateInfo renderPassMultiviewCreateInfo{};
// 			renderPassMultiviewCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_MULTIVIEW_CREATE_INFO;
// 			renderPassMultiviewCreateInfo.subpassCount = 1;
// 			renderPassMultiviewCreateInfo.pViewMasks = &viewMask;
// 			// renderPassMultiviewCreateInfo.correlationMaskCount = 1;
// 			// renderPassMultiviewCreateInfo.pCorrelationMasks = &correlationMask;

// 			renderPassInfo.pNext = &renderPassMultiviewCreateInfo;
// 			VK_CHECK(vkCreateRenderPass(VyContext::device(), &renderPassInfo, nullptr, &m_RenderPass));

// 			TVector<VkImageView> attachmentViews;
// 			for (auto attachment : m_Attachments) {
// 				attachmentViews.push_back(attachment.View);
// 			}

// 			// Find. max number of layers across attachments
// 			U32 maxLayers = 0;
// 			for (auto attachment : m_Attachments) {
// 				if (attachment.SubresourceRange.layerCount > maxLayers) {
// 					maxLayers = attachment.SubresourceRange.layerCount;
// 				}
// 			}

// 			VkFramebufferCreateInfo framebufferInfo = {};
// 			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
// 			framebufferInfo.renderPass = m_RenderPass;
// 			framebufferInfo.pAttachments = attachmentViews.data();
// 			framebufferInfo.attachmentCount = static_cast<U32>(attachmentViews.size());
// 			framebufferInfo.width = m_Width;
// 			framebufferInfo.height = m_Height;
// 			framebufferInfo.layers = 1;
// 			VK_CHECK(vkCreateFramebuffer(VyContext::device(), &framebufferInfo, nullptr, &m_Framebuffer));

// 			return VK_SUCCESS;
// 		}
// 	};
// }























namespace Vy
{
    class VyFramebuffer2
    {
    public:
        struct Attachment
        {
            VkFormat          Format;
            VkImageUsageFlags Usage;
            VkImageLayout     FinalLayout;
        };

        VyFramebuffer2(VkExtent2D extent, U32 frameCount, bool useMipmaps = false);

        ~VyFramebuffer2();

        VyFramebuffer2(const VyFramebuffer2&)            = delete;
        VyFramebuffer2& operator=(const VyFramebuffer2&) = delete;

        void resize(VkExtent2D newExtent);

        VkRenderPass          renderPass() const { return m_RenderPass; }
        VkDescriptorImageInfo descriptorImageInfo(int index) const;

        void beginRenderPass(VkCommandBuffer cmdBuffer, int frameIndex);
        void endRenderPass(VkCommandBuffer cmdBuffer) const;

        void generateMipmaps(VkCommandBuffer cmdBuffer, int frameIndex);

        float getAspectRatio() const { return static_cast<float>(m_Extent.width) / static_cast<float>(m_Extent.height); }

        // Accessors for HZB
        VkImage     depthImage       (int frameIndex)               const { return m_DepthImages[frameIndex].handle(); }
        VkImageView depthImageView   (int frameIndex)               const { return m_DepthImageViews[frameIndex].handle(); }
        VkImageView depthMipImageView(int frameIndex, int mipLevel) const { return m_DepthMipImageViews[frameIndex][mipLevel].handle(); }
        VkSampler   depthSampler()                                  const { return m_DepthSampler.handle(); }

    private:
        void createRenderPass();
        void createImages();
        void createFramebuffers();
        void cleanup();

        VkExtent2D m_Extent;
        U32        m_FrameCount;
        bool       m_UseMipmaps;
        U32        m_MipLevels{1};

        VkRenderPass m_RenderPass{VK_NULL_HANDLE};

        // Color attachment
        TVector<VyImage>        m_ColorImages;
        TVector<VyImageView>    m_ColorImageViews;
        TVector<VyImageView>    m_ColorAttachmentImageViews;

        // Depth attachment
        TVector<VyImage>        m_DepthImages;
        TVector<VyImageView>    m_DepthImageViews;
        // Per-mip views for depth (for HZB generation)
        // Outer vector: frame index, Inner vector: mip level
        TVector<TVector<VyImageView>> m_DepthMipImageViews;

        TVector<VkFramebuffer> m_Framebuffers;
        VySampler              m_Sampler;
        VySampler              m_DepthSampler;
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
        // class Builder
        // {
        // public:
        //     Builder() = default;

        //     Builder& NewSubpass()
        //     {
        //         m_SubPassIdx = static_cast<U32>(m_vSubPasses.size());
        //         m_vSubPasses.emplace_back();
        //         m_vSubPasses.back().colorAttachmentCount = 0;
        //         m_vSubPasses.back().pColorAttachments = nullptr;
        //         m_vSubPasses.back().pDepthStencilAttachment = nullptr;
        //         m_vSubPasses.back().pResolveAttachments = nullptr;

        //         return *this;
        //     }
        //     Builder& SetBindPoint(VkPipelineBindPoint bindPoint) 
        //     { 
        //         m_vSubPasses.back().pipelineBindPoint = bindPoint; 
        //         return *this; 
        //     }

        //     Builder& NewAttachment() 
        //     { 
        //         m_vAttachmentDescriptions.emplace_back(); 
        //         return *this; 
        //     }
        //     Builder& SetFormat(VkFormat format)
        //     { 
        //         m_vAttachmentDescriptions.back().format = format; return *this; 
        //     }

        //     Builder& SetSamples(VkSampleCountFlagBits samples)
        //     { 
        //         m_vAttachmentDescriptions.back().samples = samples; return *this; 
        //     }

        //     Builder& SetLoadStoreOp(VkAttachmentLoadOp loadOp, VkAttachmentStoreOp storeOp)
        //     {
        //         m_vAttachmentDescriptions.back().loadOp = loadOp;
        //         m_vAttachmentDescriptions.back().storeOp = storeOp;
        //         return *this;
        //     }
        //     Builder& SetStencilLoadStoreOp(VkAttachmentLoadOp loadOp, VkAttachmentStoreOp storeOp)
        //     {
        //         m_vAttachmentDescriptions.back().stencilLoadOp = loadOp;
        //         m_vAttachmentDescriptions.back().stencilStoreOp = storeOp;
        //         return *this;
        //     }
            
        //     Builder& SetInitialLayout(VkImageLayout layout)
        //     { 
        //         m_vAttachmentDescriptions.back().initialLayout = layout; 
        //         return *this; 
        //     }

        //     Builder& SetFinalLayout(VkImageLayout layout)
        //     { 
        //         m_vAttachmentDescriptions.back().finalLayout = layout; 
        //         return *this; 
        //     }
            
        //     Builder& AddSubpassColorAttachment(U32 attachment)
        //     {
        //         if (m_vColorAttachmentRefs.size() <= m_SubPassIdx)
        //             m_vColorAttachmentRefs.emplace_back();
        //         m_vColorAttachmentRefs.back().emplace_back(attachment, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
        //         m_vSubPasses.back().colorAttachmentCount = static_cast<U32>(m_vColorAttachmentRefs.back().size());
        //         m_vSubPasses.back().pColorAttachments = m_vColorAttachmentRefs.back().data();
        //         return *this;
        //     }
        //     Builder& SetSubpassResolveAttachment(U32 attachment)
        //     {
        //         m_ResolveAttachmentRef.emplace_back(attachment, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
        //         m_vSubPasses.back().pResolveAttachments = &m_ResolveAttachmentRef.back();
        //         return *this;
        //     }
        //     Builder& SetSubpassDepthAttachment(U32 attachment)
        //     {
        //         m_vDepthAttachmentRefs.emplace_back(attachment, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
        //         m_vSubPasses.back().pDepthStencilAttachment = &m_vDepthAttachmentRefs.back();
        //         return *this;
        //     }

            
        //     Builder& newDependency()
        //     { 
        //         m_vSubPassDependencies.emplace_back(); return *this; 
        //     }

        //     Builder& dependencyFlag(VkDependencyFlags flags)
        //     { 
        //         m_vSubPassDependencies.back().dependencyFlags = flags; return *this; 
        //     }

        //     Builder& srcSubPass(U32 subpass)
        //     { 
        //         m_vSubPassDependencies.back().srcSubpass = subpass; return *this; 
        //     }

        //     Builder& dstSubPass(U32 subpass)
        //     { 
        //         m_vSubPassDependencies.back().dstSubpass = subpass; return *this; 
        //     }

        //     Builder& srcMasks(VkPipelineStageFlags stageFlags, VkAccessFlags accessFlags)
        //     {
        //         m_vSubPassDependencies.back().srcStageMask = stageFlags;
        //         m_vSubPassDependencies.back().srcAccessMask = accessFlags;
        //         return *this;
        //     }

        //     Builder& dstMasks(VkPipelineStageFlags stageFlags, VkAccessFlags accessFlags)
        //     {
        //         m_vSubPassDependencies.back().dstStageMask = stageFlags;
        //         m_vSubPassDependencies.back().dstAccessMask = accessFlags;
        //         return *this;
        //     }

        //     void build(RenderPass& renderPass) const
        //     {
        //         renderPass.m_AttachmentCount = static_cast<U32>(m_vAttachmentDescriptions.size());

        //         VkRenderPassCreateInfo renderPassInfo{};
        //         {
        //             renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        //             renderPassInfo.attachmentCount = renderPass.m_AttachmentCount;
        //             renderPassInfo.pAttachments = m_vAttachmentDescriptions.data();
        //             renderPassInfo.subpassCount = static_cast<U32>(m_vSubPasses.size());
        //             renderPassInfo.pSubpasses = m_vSubPasses.data();
        //             renderPassInfo.dependencyCount = static_cast<U32>(m_vSubPassDependencies.size());
        //             renderPassInfo.pDependencies = m_vSubPassDependencies.data();
        //         }

        //         if (vkCreateRenderPass(VyContext::device(), &renderPassInfo, nullptr, &renderPass.m_RenderPass) != VK_SUCCESS)
        //         {
        //             throw std::runtime_error("Failed to created Render Pass!");
        //         }
        //     }


        // private:
        //     TVector<VkAttachmentDescription>        m_vAttachmentDescriptions{};
        //     TVector<VkSubpassDescription>           m_vSubPasses{};
        //     TVector<VkSubpassDependency>            m_vSubPassDependencies{};
        //     U32                                     m_SubPassIdx{};

        //     TVector<VkAttachmentReference>          m_ResolveAttachmentRef{ };
        //     TVector<VkAttachmentReference>          m_vDepthAttachmentRefs{ };
        //     TVector<TVector<VkAttachmentReference>>	m_vColorAttachmentRefs{ };
        // };
        // class Builder
        // {
        // public:
        //     VkRenderPassCreateInfo           m_Info;
        //     TVector<VkAttachmentDescription> m_AttachementDescriptions;
        //     TVector<VkSubpassDescription>    m_SubpassDescriptions;
        //     TVector<VkSubpassDependency>     m_SubpassDependencies;

        //     Builder()
        //     {
        //         m_Info = VKInit::renderPassCreateInfo();
        //     }

        //     operator VyRenderPass() 
        //     {
        //         m_Info.attachmentCount = m_AttachementDescriptions.size();
        //         m_Info.pAttachments    = m_AttachementDescriptions.data();
        //         m_Info.subpassCount    = m_SubpassDescriptions.size();
        //         m_Info.pSubpasses      = m_SubpassDescriptions.data();
        //         m_Info.dependencyCount = m_SubpassDependencies.size();
        //         m_Info.pDependencies   = m_SubpassDependencies.data();

        //         for (auto& subpass_builder : m_SubpassBuilders)
        //         {
        //             subpass_builder.buildDescriptor();
        //         }

        //         return VyRenderPass(m_Info);
        //     }

        //     class Subpass;
        //     Builder::Subpass& addSubpass();

        //     class Attachment 
        //     {
        //     public:
        //         Builder&                 m_Builder;
        //         VkAttachmentDescription& m_Description;

        //         Attachment(Builder& builder, VkAttachmentDescription& description) : 
        //             m_Builder(builder) , 
        //             m_Description(description) 
        //         {
        //         }

        //         static void setDefaults(VkAttachmentDescription& attachmentDescription) 
        //         {
        //             attachmentDescription = {};
        //             {
        //                 attachmentDescription.flags          = 0;
        //                 attachmentDescription.format         = VK_FORMAT_R8G8B8A8_SRGB;
        //                 attachmentDescription.samples        = VK_SAMPLE_COUNT_1_BIT;
        //                 attachmentDescription.loadOp         = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        //                 attachmentDescription.storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        //                 attachmentDescription.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        //                 attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        //                 attachmentDescription.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
        //                 attachmentDescription.finalLayout    = VK_IMAGE_LAYOUT_UNDEFINED;
        //             }
        //         }

        //         // shortcuts

        //         Attachment& from(const VyImage& image) 
        //         { 
        //             return 
        //                  setFormat (image.format())
        //                 .setSamples(image.sampleCount()); 
        //         }

        //         // Color Depth
        //         Attachment& loadInitialColorDepth()     { return setLoadOp (VK_ATTACHMENT_LOAD_OP_LOAD);       }
        //         Attachment& clearInitialColorDepth()    { return setLoadOp (VK_ATTACHMENT_LOAD_OP_CLEAR);      }
        //         Attachment& dontCareInitialColorDepth() { return setLoadOp (VK_ATTACHMENT_LOAD_OP_DONT_CARE);  }
        //         Attachment& storeFinalColorDepth()      { return setStoreOp(VK_ATTACHMENT_STORE_OP_STORE);     }
        //         Attachment& dontCareFinalColorDepth()   { return setStoreOp(VK_ATTACHMENT_STORE_OP_DONT_CARE); }

        //         // Stencil
        //         Attachment& loadInitialStencil()        { return setStencilLoadOp (VK_ATTACHMENT_LOAD_OP_LOAD);       }
        //         Attachment& clearInitialStencil()       { return setStencilLoadOp (VK_ATTACHMENT_LOAD_OP_CLEAR);      }
        //         Attachment& dontCareInitialStencil()    { return setStencilLoadOp (VK_ATTACHMENT_LOAD_OP_DONT_CARE);  }
        //         Attachment& storeFinalStencil()         { return setStencilStoreOp(VK_ATTACHMENT_STORE_OP_STORE);     }
        //         Attachment& dontCareFinalStencil()      { return setStencilStoreOp(VK_ATTACHMENT_STORE_OP_DONT_CARE); }

        //         Attachment& colorAndDepthOp(VkAttachmentLoadOp loadOp, VkAttachmentStoreOp storeOp) { return setLoadOp(loadOp).setStoreOp(storeOp); }
        //         Attachment& stencilOp      (VkAttachmentLoadOp loadOp, VkAttachmentStoreOp storeOp) { return setStencilLoadOp(loadOp).setStencilStoreOp(storeOp); }
                
        //         // Initial Layout
        //         Attachment& initialLayoutColorAttachment()     { return setInitialLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL); }
                
        //         // Final Layout
        //         Attachment& finalLayoutColorAttachment()       { return setFinalLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);   }
        //         Attachment& finalLayoutShaderReadOnlyOptimal() { return setFinalLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);   }
        //         Attachment& finalLayoutPresentSrc()            { return setFinalLayout(VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);            }
        //         Attachment& finalLayoutTransferSrc()           { return setFinalLayout(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);       }
        //         Attachment& finalLayoutTransferDst()           { return setFinalLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);       }

        //         // property setters

        //         Attachment& setAttachmentFlags(VkAttachmentDescriptionFlags flags) { m_Description.flags          = flags;          return *this; }
        //         Attachment& setFormat         (VkFormat format)                    { m_Description.format         = format;         return *this; }
        //         Attachment& setSamples        (VkSampleCountFlagBits samples)      { m_Description.samples        = samples;        return *this; }
        //         Attachment& setLoadOp         (VkAttachmentLoadOp  loadOp)         { m_Description.loadOp         = loadOp;         return *this; }
        //         Attachment& setStoreOp        (VkAttachmentStoreOp storeOp)        { m_Description.storeOp        = storeOp;        return *this; }
        //         Attachment& setStencilLoadOp  (VkAttachmentLoadOp  stencilLoadOp)  { m_Description.stencilLoadOp  = stencilLoadOp;  return *this; }
        //         Attachment& setStencilStoreOp (VkAttachmentStoreOp stencilStoreOp) { m_Description.stencilStoreOp = stencilStoreOp; return *this; }
        //         Attachment& setInitialLayout  (VkImageLayout initialLayout)        { m_Description.initialLayout  = initialLayout;  return *this; }
        //         Attachment& setFinalLayout    (VkImageLayout finalLayout)          { m_Description.finalLayout    = finalLayout;    return *this; }

        //         // return expressions

        //         operator VyRenderPass()                                    { return m_Builder; }
        //         Builder& setRenderPassFlags(VkRenderPassCreateFlags flags) { return m_Builder.setRenderPassFlags(flags); }
        //         Builder::Attachment attachment(U32 index)                  { return m_Builder.attachment(index); }
        //         Builder::Subpass&   subpass(U32 index)                     { return m_Builder.subpass(index); }
        //         Builder::Attachment addAttachment()                        { return m_Builder.addAttachment(); }
        //         Builder::Attachment addAttachment(const VyImage& image)    { return m_Builder.addAttachment(image); }
        //         Builder::Subpass&   addSubpass()                           { return m_Builder.addSubpass(); }
        //     };


        //     class Subpass
        //     {
        //         friend class Builder;
                
        //     public:
        //         Builder&                       m_Builder;
        //         VkSubpassDescription&          m_Description;
        //         TVector<VkAttachmentReference> m_ColorAttachments;


        //         Subpass(Builder& builder, VkSubpassDescription& description) : 
        //             m_Builder(builder), 
        //             m_Description(description)
        //         {
        //             m_Description = {};
        //             m_Description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        //         }

        //         Subpass& writesColorAttachment(U32 index) 
        //         { 
        //             m_ColorAttachments.push_back({ index, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL }); 
                    
        //             return *this; 
        //         }
                
        //         Subpass& writesColorAttachment(U32 index, VkImageLayout layout) 
        //         { 
        //             m_ColorAttachments.push_back({ index, layout });

        //             return *this; 
        //         }

        //         operator VyRenderPass()                                    { return m_Builder; }
        //         Builder& setRenderPassFlags(VkRenderPassCreateFlags flags) { return m_Builder.setRenderPassFlags(flags); }
        //         Builder::Attachment attachment(U32 index)                  { return m_Builder.attachment(index); }
        //         Builder::Subpass&   subpass   (U32 index)                  { return m_Builder.subpass(index); }
        //         Builder::Attachment addAttachment()                        { return m_Builder.addAttachment(); }
        //         Builder::Attachment addAttachment(const VyImage& image)    { return m_Builder.addAttachment(image); }
        //         Builder::Subpass&   addSubpass()                           { return m_Builder.addSubpass(); }

        //     private:

        //         // must be called by parent builder before it builds
        //         void buildDescriptor() 
        //         {
        //             m_Description.colorAttachmentCount = m_ColorAttachments.size();
        //             m_Description.pColorAttachments    = m_ColorAttachments.data();
        //         }
        //     };

        //     // property setters

        //     Builder& setRenderPassFlags(VkRenderPassCreateFlags flags) 
        //     { 
        //         m_Info.flags = flags;

        //         return *this; 
        //     }

        //     // subexpressions

        //     Builder::Attachment addAttachment() 
        //     { 
        //         m_AttachementDescriptions.push_back({}); 
                
        //         Attachment::setDefaults(m_AttachementDescriptions.back()); 
                
        //         return Attachment( *this, m_AttachementDescriptions.back() ); 
        //     }
            
        //     Builder::Attachment addAttachment(const VyImage& image) 
        //     { 
        //         auto subexpression = addAttachment();

        //         subexpression.from(image);
            
        //         return subexpression;
        //     }

        //     Builder::Attachment attachment(U32 index)
        //     {
        //         while (m_AttachementDescriptions.size() <= index)
        //         {
        //             addAttachment();
        //         }

        //         return Attachment( *this, m_AttachementDescriptions[index] );
        //     }

        //     Builder::Subpass& subpass(U32 index) 
        //     {
        //         while (m_SubpassBuilders.size() <= index)
        //         {
        //             addSubpass();
        //         }

        //         return m_SubpassBuilders[index];
        //     }

        //     TVector<Subpass> m_SubpassBuilders;
        // };



		VyRenderPass(const VyRenderPassDesc& desc);

        VyRenderPass(VkRenderPassCreateInfo& createInfo)
        {
            VK_CHECK(vkCreateRenderPass(VyContext::device(), &createInfo, nullptr, &m_RenderPass));
        }

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

    // inline VyRenderPass::Builder::Subpass& VyRenderPass::Builder::addSubpass() 
    // {
    //     m_SubpassDescriptions.emplace_back();
    //     m_SubpassBuilders.emplace_back(*this, m_SubpassDescriptions.back());

    //     return m_SubpassBuilders.back();
    // }

    struct VyFramebufferDesc
    {
        U32                      Width;
        U32                      Height;
        U32                      Layers{ 1 };
        TVector<VkImageView>     Attachments;
        VkFramebufferCreateFlags Flags = 0;
    };

    // VkFramebuffer Wrapper Class
    // mainly used to save the VkFramebuffer creation info and the
    // resources linked to it (attachments)
    class VyFramebuffer 
    {
    public:
        static Shared<VyFramebuffer> create(const VyRenderPass& renderPass, const VyFramebufferDesc& desc) 
        {
            return MakeShared<VyFramebuffer>(renderPass, desc);
        }

        VyFramebuffer(const VyRenderPass& renderPass, const VyFramebufferDesc& desc);

        ~VyFramebuffer();

        VyFramebuffer(const VyFramebuffer& other)            = delete;
        VyFramebuffer& operator=(const VyFramebuffer& other) = delete;

		VyFramebuffer(VyFramebuffer&& other)            noexcept;
		VyFramebuffer& operator=(VyFramebuffer&& other) noexcept;

        operator     VkFramebuffer()                  { return m_Framebuffer; }
        VY_NODISCARD VkFramebuffer     handle() const { return m_Framebuffer; }
        VY_NODISCARD const VkExtent2D& extent() const { return m_Extent;      }
        
        void destroy();

    private:
        VkFramebuffer m_Framebuffer{ VK_NULL_HANDLE };
        VkExtent2D    m_Extent;
    };
}