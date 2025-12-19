#pragma once

#include <Vy/GFX/Backend/Image/Image.h>
#include <Vy/GFX/Backend/Image/ImageView.h>
#include <Vy/GFX/Backend/Image/Sampler.h>

#include <Vy/GFX/Context.h>

// Custom define for better code readability
#define VK_FLAGS_NONE 0

namespace Vy
{
    using VyAttachmentFlags = U8;

    enum VyAttachmentFlagBits : VyAttachmentFlags
    {
        VY_ATTACHMENT_NONE    = 0,
        VY_ATTACHMENT_COLOR   = VY_BIT(0),
        VY_ATTACHMENT_DEPTH   = VY_BIT(1),
        VY_ATTACHMENT_STENCIL = VY_BIT(2),
        VY_ATTACHMENT_INPUT   = VY_BIT(3),

        VY_ATTACHMENT_DEPTH_STENCIL = VY_ATTACHMENT_DEPTH | VY_ATTACHMENT_STENCIL
    };

    VY_ENUM_CLASS_FLAG(U8, VyAttachmentFlagBits)


    struct VyAttachmentData
    {
        VkAttachmentDescription Description{};
        VyAttachmentFlags       Flags;
    };

    class VyAttachment
    {
    public:
        class Builder
        {
        public:
            Builder(VyAttachmentFlags flags)
            {
                VY_ASSERT(flags,                                                               "Attachment must have at least one type flag");
                VY_ASSERT(!((flags & VY_ATTACHMENT_COLOR) && (flags & VY_ATTACHMENT_DEPTH)),   "Attachment must be color or depth, not both");
                VY_ASSERT(!((flags & VY_ATTACHMENT_COLOR) && (flags & VY_ATTACHMENT_STENCIL)), "Attachment must be color or stencil, not both");

                m_Data.Description = {};
                {
                    m_Data.Description.format         = VK_FORMAT_R8G8B8A8_SRGB;
                    m_Data.Description.samples        = VK_SAMPLE_COUNT_1_BIT;
                    m_Data.Description.loadOp         = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                    m_Data.Description.storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                    m_Data.Description.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                    m_Data.Description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                    m_Data.Description.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
                    m_Data.Description.finalLayout    = VK_IMAGE_LAYOUT_UNDEFINED;
                    m_Data.Description.flags          = 0;
                }

                if (flags & VY_ATTACHMENT_COLOR)
                {
                    m_Data.Description.loadOp  = VK_ATTACHMENT_LOAD_OP_CLEAR;
                    m_Data.Description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                }

                if (flags & VY_ATTACHMENT_DEPTH)
                {
                    m_Data.Description.loadOp  = VK_ATTACHMENT_LOAD_OP_CLEAR;
                    m_Data.Description.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                }
                
                if (flags & VY_ATTACHMENT_STENCIL)
                {
                    m_Data.Description.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_CLEAR;
                    m_Data.Description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                }
            }
            
            // -------------------------------------------------------------------------------------------------------------
            // Property Setters

            Builder& setFormat        (VkFormat              value) { m_Data.Description.format         = value; return *this; }
            Builder& setSamples       (VkSampleCountFlagBits value) { m_Data.Description.samples        = value; return *this; }
            Builder& setLoadOp        (VkAttachmentLoadOp    value) { m_Data.Description.loadOp         = value; return *this; }
            Builder& setStoreOp       (VkAttachmentStoreOp   value) { m_Data.Description.storeOp        = value; return *this; }
            Builder& setStencilLoadOp (VkAttachmentLoadOp    value) { m_Data.Description.stencilLoadOp  = value; return *this; }
            Builder& setStencilStoreOp(VkAttachmentStoreOp   value) { m_Data.Description.stencilStoreOp = value; return *this; }
            Builder& setInitialLayout (VkImageLayout         value) { m_Data.Description.initialLayout  = value; return *this; }
            Builder& setFinalLayout   (VkImageLayout         value) { m_Data.Description.finalLayout    = value; return *this; }
            Builder& setLayouts(VkImageLayout initialLayout, VkImageLayout finalLayout) 
            { 
                setInitialLayout(initialLayout); 
                setFinalLayout(finalLayout); 
                return *this; 
            }

            // -------------------------------------------------------------------------------------------------------------
            // Specialized Setters

            // Load
            Builder& setLoadOpLoad    () { return setLoadOp(VK_ATTACHMENT_LOAD_OP_LOAD     ); }
            Builder& setLoadOpClear   () { return setLoadOp(VK_ATTACHMENT_LOAD_OP_CLEAR    ); }
            Builder& setLoadOpDontCare() { return setLoadOp(VK_ATTACHMENT_LOAD_OP_DONT_CARE); }

            // Store
            Builder& setStoreOpStore   () { return setStoreOp(VK_ATTACHMENT_STORE_OP_STORE    ); }
            Builder& setStoreOpDontCare() { return setStoreOp(VK_ATTACHMENT_STORE_OP_DONT_CARE); }

            // Stencil Load
            Builder& setStencilLoadOpLoad    () { return setStencilLoadOp(VK_ATTACHMENT_LOAD_OP_LOAD     ); }
            Builder& setStencilLoadOpClear   () { return setStencilLoadOp(VK_ATTACHMENT_LOAD_OP_CLEAR    ); }
            Builder& setStencilLoadOpDontCare() { return setStencilLoadOp(VK_ATTACHMENT_LOAD_OP_DONT_CARE); }

            // Stencil Store
            Builder& setStencilStoreOpStore   () { return setStencilStoreOp(VK_ATTACHMENT_STORE_OP_STORE    ); }
            Builder& setStencilStoreOpDontCare() { return setStencilStoreOp(VK_ATTACHMENT_STORE_OP_DONT_CARE); }

            // Initial Layout
            Builder& setInitialLayoutColorAttachment() { return setInitialLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL); }
            
            // Final Layout
            Builder& setFinalLayoutColorAttachment       () { return setFinalLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL        ); }
            Builder& setFinalLayoutDepthStencilAttachment() { return setFinalLayout(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL); }
            Builder& setFinalLayoutShaderReadOnly        () { return setFinalLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL        ); }
            Builder& setFinalLayoutPresentSrc            () { return setFinalLayout(VK_IMAGE_LAYOUT_PRESENT_SRC_KHR                 ); }
            Builder& setFinalLayoutTransferSrc           () { return setFinalLayout(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL            ); }
            Builder& setFinalLayoutTransferDst           () { return setFinalLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL            ); }

            // -------------------------------------------------------------------------------------------------------------
            // Return Expressions

            VyAttachment build() const
            {
                return VyAttachment{ m_Data };
            }

            Unique<VyAttachment> buildUnique() const
            {
                return MakeUnique<VyAttachment>( m_Data );
            }

            Shared<VyAttachment> buildShared() const
            {
                return MakeShared<VyAttachment>( m_Data );
            }

        private:
            VyAttachmentData m_Data{};
            
            friend class VyAttachment;
        };

    public:

        // VyAttachment() = default;

        VyAttachment(const VyAttachmentData& data) :
            m_Data{ data }
        {
        }

        ~VyAttachment()
        {
        }

        const VkAttachmentDescription& descrption() const { return m_Data.Description; }
        VkAttachmentDescription&       descrption()       { return m_Data.Description; }

    private:
        VyAttachmentData m_Data;
    };


}



namespace Vy
{
    class VyRenderPass;

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

        VyFramebuffer() = default;

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





    /**
	 * @brief Encapsulates a single frame buffer attachment
	 */
	// struct FramebufferAttachment 
    // {
	// 	VyImage                 Image           {};
	// 	VyImageView             View            {};
	// 	VkFormat                Format          {};
	// 	VkImageSubresourceRange SubresourceRange{};
	// 	VkAttachmentDescription Description     {};

	// 	/**
	// 	 * @brief Returns true if the attachment has a depth component
	// 	 */
	// 	bool hasDepth() 
    //     {
	// 		TVector<VkFormat> formats = {
    //             VK_FORMAT_D16_UNORM,
    //             VK_FORMAT_X8_D24_UNORM_PACK32,
    //             VK_FORMAT_D32_SFLOAT,
    //             VK_FORMAT_D16_UNORM_S8_UINT,
    //             VK_FORMAT_D24_UNORM_S8_UINT,
    //             VK_FORMAT_D32_SFLOAT_S8_UINT,
    //         };

	// 		return std::find(formats.begin(), formats.end(), Format) != std::end(formats);
	// 	}

	// 	/**
	// 	 * @brief Returns true if the attachment has a stencil component
	// 	 */
	// 	bool hasStencil() 
    //     {
	// 		TVector<VkFormat> formats = {
    //             VK_FORMAT_S8_UINT,
    //             VK_FORMAT_D16_UNORM_S8_UINT,
    //             VK_FORMAT_D24_UNORM_S8_UINT,
    //             VK_FORMAT_D32_SFLOAT_S8_UINT,
    //         };

	// 		return std::find(formats.begin(), formats.end(), Format) != std::end(formats);
	// 	}

	// 	/**
	// 	 * @brief Returns true if the attachment is a depth and/or stencil attachment
	// 	 */
	// 	bool isDepthStencil() 
    //     {
	// 		return (hasDepth() || hasStencil());
	// 	}
	// };

    // // ============================================================================================

	// /**
	//  * @brief Describes the attributes of an attachment to be created
	//  */
	// struct AttachmentCreateInfo 
    // {
	// 	U32 Width; 
    //     U32 Height;
	// 	U32 LayerCount = 1;
	// 	VkFormat Format{};
	// 	VkImageUsageFlags Usage{};
	// 	VkAttachmentLoadOp LoadOp{};
	// 	VkAttachmentStoreOp StoreOp{};
	// 	VkAttachmentLoadOp StencilLoadOp{};
	// 	VkAttachmentStoreOp StencilStoreOp{};
	// 	VkImageLayout InitialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	// 	VkSampleCountFlagBits ImageSampleCount = VK_SAMPLE_COUNT_1_BIT;
	// 	VkImageCreateFlags Flags{};
	// };

	// /**
	//  * @brief Describes the attributes of an attachment to be added
	//  */
	// struct AttachmentAddInfo 
    // {
	// 	U32 Width; 
    //     U32 Height;
	// 	U32 LayerCount;
	// 	VkFormat Format;
	// 	VkAttachmentLoadOp LoadOp;
	// 	VkAttachmentStoreOp StoreOp;
	// 	VkAttachmentLoadOp StencilLoadOp;
	// 	VkAttachmentStoreOp StencilStoreOp;
	// 	VkImageUsageFlags Usage;
	// 	VkImageLayout InitialLayout;
	// 	VkSampleCountFlagBits ImageSampleCount = VK_SAMPLE_COUNT_1_BIT;
	// 	VyImageView View;
	// };

	// /**
	//  * @brief Encapsulates a complete Vulkan framebuffer with an arbitrary number and combination of attachments
	//  */
	// struct Framebuffer 
    // {
	// private:
	// 	U32 m_Width; 
    //     U32 m_Height;
	// 	VkFramebuffer m_Framebuffer{};
	// 	VkRenderPass m_RenderPass{};
	// 	VySampler m_Sampler{};
	// 	TVector<FramebufferAttachment> m_Attachments{};

	// public:

    //     VkFramebuffer framebuffer() const { return m_Framebuffer; }
    //     VkRenderPass renderPass() const {return m_RenderPass; }
    //     VkSampler sampler() const {return m_Sampler.handle(); }
    //     U32 width() const {return m_Width; }
    //     U32 height() const {return m_Height; }

	// 	/**
	// 	 * Default constructor
	// 	 */
	// 	Framebuffer() = default;

	// 	/**
	// 	 * Destroy and free Vulkan resources used for the framebuffer and all of its attachments
	// 	 */
	// 	~Framebuffer() 
    //     {
	// 		for (auto& attachment : m_Attachments) 
    //         {
	// 			// Only destroy attachments if they are not empty (attachments may just contain loaded data from another Framebuffer).
	// 			if (attachment.Image) 
    //             {
	// 				// vkDestroyImage(VyContext::device(), attachment.image, nullptr);
	// 				// vkDestroyImageView(VyContext::device(), attachment.view, nullptr);
	// 				// vkFreeMemory(VyContext::device(), attachment.memory, nullptr);
	// 			}
	// 		}
	// 		vkDestroyRenderPass(VyContext::device(), m_RenderPass, nullptr);
	// 		vkDestroyFramebuffer(VyContext::device(), m_Framebuffer, nullptr);
	// 	}


	// 	/**
	// 	 * Will empty the framebuffer, allowing for safe recreation.
	// 	 */
	// 	void clearFramebuffer() 
    //     {
	// 		for (auto& attachment : m_Attachments) 
    //         {
	// 			// Only destroy attachments if they are not empty (attachments may just contain loaded data from another Framebuffer).
	// 			if (attachment.Image) 
    //             {
	// 				// vkDestroyImage(VyContext::device(), attachment.image, nullptr);
	// 				// vkDestroyImageView(VyContext::device(), attachment.view, nullptr);
	// 				// vkFreeMemory(VyContext::device(), attachment.memory, nullptr);
	// 			}
	// 		}
	// 		m_Attachments.clear();

	// 		vkDestroyRenderPass(VyContext::device(), m_RenderPass, nullptr);
	// 		vkDestroyFramebuffer(VyContext::device(), m_Framebuffer, nullptr);
	// 	}

	// 	/**
	// 	 * Create a new attachment described by createinfo to the framebuffer's attachment list.
	// 	 * This will also create a image and image view to go with the attachment.
	// 	 *
	// 	 * @param createinfo Structure that specifies the framebuffer to be constructed
	// 	 *
	// 	 * @return Index of the new attachment
	// 	 */
	// 	U32 createAttachment(AttachmentCreateInfo createinfo) 
    //     {
	// 		FramebufferAttachment attachment;

	// 		// TODO: Is there a better way to save the width and height for the Framebuffer?
	// 		m_Width  = createinfo.Width;
	// 		m_Height = createinfo.Height;

	// 		attachment.Format = createinfo.Format;

	// 		VkImageAspectFlags aspectMask = VK_FLAGS_NONE;

	// 		// Select aspect mask and layout depending on usage

	// 		// Color attachment
	// 		if (createinfo.Usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) 
    //         {
	// 			aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	// 		}

	// 		// Depth (and/or stencil) attachment
	// 		if (createinfo.Usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) 
    //         {
	// 			if (attachment.hasDepth()) 
    //             {
	// 				aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	// 			}
	// 			if (attachment.hasStencil()) 
    //             {
	// 				aspectMask = aspectMask | VK_IMAGE_ASPECT_STENCIL_BIT;
	// 			}
	// 		}

	// 		assert(aspectMask > 0);

    //         attachment.Image = VyImage::Builder{}
    //             .imageType(VK_IMAGE_TYPE_2D)
    //             .format(createinfo.Format)
    //             .extent(VkExtent2D{createinfo.Width, createinfo.Height})
    //             .mipLevels(1)
    //             .arrayLayers(createinfo.LayerCount)
    //             .sampleCount(createinfo.ImageSampleCount)
    //             .tiling(VK_IMAGE_TILING_OPTIMAL)
    //             .usage(createinfo.Usage)
    //             .createFlags(createinfo.Flags)
    //             .memoryUsage(VMA_MEMORY_USAGE_AUTO)
    //         .build();

	// 		attachment.SubresourceRange = {};
	// 		attachment.SubresourceRange.aspectMask = aspectMask;
	// 		attachment.SubresourceRange.levelCount = 1;
	// 		attachment.SubresourceRange.layerCount = createinfo.LayerCount;

    //         auto viewType = (createinfo.LayerCount == 1) 
    //             ? VK_IMAGE_VIEW_TYPE_2D 
    //             : (createinfo.LayerCount == 6) 
    //                 ? VK_IMAGE_VIEW_TYPE_CUBE
    //                 : VK_IMAGE_VIEW_TYPE_2D_ARRAY;

    //         attachment.View = VyImageView::Builder{}
    //             .viewType   (viewType)
    //             .format     (createinfo.Format)
    //             .aspectMask (aspectMask)
    //             .mipLevels  (0, 1)
    //             .arrayLayers(0, createinfo.LayerCount)
    //         .build(attachment.Image);

	// 		// Fill attachment description
	// 		attachment.Description = {};
	// 		attachment.Description.samples = createinfo.ImageSampleCount;
	// 		attachment.Description.loadOp = createinfo.LoadOp;
	// 		// attachment.Description.storeOp = (createinfo.Usage & VK_IMAGE_USAGE_SAMPLED_BIT) ? VK_ATTACHMENT_STORE_OP_STORE : VK_ATTACHMENT_STORE_OP_DONT_CARE;
	// 		attachment.Description.storeOp = createinfo.StoreOp;
	// 		attachment.Description.stencilLoadOp = createinfo.StencilLoadOp;
	// 		attachment.Description.stencilStoreOp = createinfo.StencilStoreOp;
	// 		attachment.Description.format = createinfo.Format;
	// 		attachment.Description.initialLayout = createinfo.InitialLayout;
	// 		// Final layout
	// 		// If not, final layout depends on attachment type
	// 		if (attachment.hasDepth() || attachment.hasStencil()) 
    //         {
	// 			attachment.Description.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
	// 		} 
    //         else {
	// 			attachment.Description.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	// 		}

	// 		m_Attachments.push_back(attachment);

	// 		return static_cast<U32>(m_Attachments.size() - 1);
	// 	}

	// 	/**
	// 	 * Add a load attachment described by createinfo to the framebuffer's attachment list.
	// 	 * This will not create an image or image view. Suitable for using attachments/images from previous render passes.
	// 	 *
	// 	 * @param addInfo Structure that specifies the framebuffer to be constructed
	// 	 *
	// 	 * @return Index of the new attachment
	// 	 */
	// 	U32 addLoadAttachment(AttachmentAddInfo addInfo) 
    //     {
	// 		FramebufferAttachment attachment;

	// 		m_Width  = addInfo.Width;
	// 		m_Height = addInfo.Height;
	// 		attachment.Format = addInfo.Format;
	// 		attachment.View   = std::move(addInfo.View);

	// 		VkImageAspectFlags aspectMask = VK_FLAGS_NONE;

	// 		// Select aspect mask and layout depending on usage

	// 		// Color attachment
	// 		if (addInfo.Usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) 
    //         {
	// 			aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	// 		}

	// 		// Depth (and/or stencil) attachment
	// 		if (addInfo.Usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) 
    //         {
	// 			if (attachment.hasDepth()) 
    //             {
	// 				aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	// 			}
	// 			if (attachment.hasStencil()) 
    //             {
	// 				aspectMask = aspectMask | VK_IMAGE_ASPECT_STENCIL_BIT;
	// 			}
	// 		}

	// 		attachment.SubresourceRange = {};
	// 		attachment.SubresourceRange.aspectMask = aspectMask;
	// 		attachment.SubresourceRange.levelCount = 1;
	// 		attachment.SubresourceRange.layerCount = addInfo.LayerCount;

	// 		assert(aspectMask > 0);

	// 		// Fill attachment description
	// 		attachment.Description = {};
	// 		attachment.Description.samples = addInfo.ImageSampleCount;
	// 		// attachment.Description.loadOp = (addInfo.Usage & (VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)) ? VK_ATTACHMENT_LOAD_OP_LOAD : VK_ATTACHMENT_LOAD_OP_CLEAR;
	// 		// attachment.Description.storeOp = (addInfo.Usage & VK_IMAGE_USAGE_SAMPLED_BIT) ? VK_ATTACHMENT_STORE_OP_STORE : VK_ATTACHMENT_STORE_OP_DONT_CARE;
	// 		attachment.Description.loadOp = addInfo.LoadOp;
	// 		attachment.Description.storeOp = addInfo.StoreOp;
	// 		attachment.Description.stencilLoadOp = addInfo.StencilLoadOp;
	// 		attachment.Description.stencilStoreOp = addInfo.StencilStoreOp;
	// 		attachment.Description.format = addInfo.Format;
	// 		attachment.Description.initialLayout = addInfo.InitialLayout;

	// 		// Final layout
	// 		// If not, final layout depends on attachment type
	// 		if (attachment.hasDepth() || attachment.hasStencil()) 
    //         {
	// 			attachment.Description.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
	// 		} 
    //         else {
	// 			attachment.Description.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	// 		}

	// 		m_Attachments.push_back(attachment);

	// 		return static_cast<U32>(m_Attachments.size() - 1);
	// 	}

	// 	/**
	// 	 * Creates a default sampler for sampling from any of the framebuffer attachments
	// 	 * Applications are free to create their own samplers for different use cases
	// 	 *
	// 	 * @param magFilter Magnification filter for lookups
	// 	 * @param minFilter Minification filter for lookups
	// 	 * @param adressMode Addressing mode for the U,V and W coordinates
	// 	 *
	// 	 * @return VkResult for the sampler creation
	// 	 */
	// 	VkResult createSampler(VkFilter magFilter, VkFilter minFilter, VkSamplerAddressMode adressMode) 
    //     {
    //         m_Sampler = VySampler::Builder{}
    //             .filters      (magFilter, minFilter)
    //             .mipmapMode   (VK_SAMPLER_MIPMAP_MODE_LINEAR)
    //             .addressMode  (adressMode)
    //             .enableCompare(VK_COMPARE_OP_LESS)
    //             .borderColor  (VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK)
    //             .lodRange     (0.0f, 1.0f)
    //             .mipLodBias   (0.0f)
    //         .build();
	// 	}

	// 	/**
	// 	 * Creates a default render pass setup with one sub pass
	// 	 *
	// 	 * @return VK_SUCCESS if all resources have been created successfully
	// 	 */
	// 	VkResult createRenderPass() 
    //     {
	// 		TVector<VkAttachmentDescription> attachmentDescriptions;
			
    //         for (auto& attachment : m_Attachments) 
    //         {
	// 			attachmentDescriptions.push_back(attachment.Description);
	// 		};

	// 		// Collect attachment references
	// 		TVector<VkAttachmentReference> colorReferences;
	// 		VkAttachmentReference depthReference{};
	// 		bool hasDepth = false;
	// 		bool hasColor = false;

	// 		U32 attachmentIndex = 0;

	// 		for (auto& attachment : m_Attachments) 
    //         {
	// 			if (attachment.isDepthStencil()) 
    //             {
	// 				// Only one depth attachment allowed
	// 				assert(!hasDepth);
	// 				depthReference.attachment = attachmentIndex;
	// 				depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	// 				hasDepth = true;
	// 			} else {
	// 				colorReferences.push_back({attachmentIndex, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});
	// 				hasColor = true;
	// 			}
	// 			attachmentIndex++;
	// 		};

	// 		// Default render pass setup uses only one subpass
	// 		VkSubpassDescription subpass{};
	// 		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	// 		if (hasColor) {
	// 			subpass.pColorAttachments = colorReferences.data();
	// 			subpass.colorAttachmentCount = static_cast<U32>(colorReferences.size());
	// 		}
	// 		if (hasDepth) {
	// 			subpass.pDepthStencilAttachment = &depthReference;
	// 		}

	// 		// Use subpass dependencies for attachment layout transitions
	// 		TArray<VkSubpassDependency, 2> dependencies{};

	// 		dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	// 		dependencies[0].dstSubpass = 0;
	// 		dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	// 		dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	// 		dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	// 		dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	// 		dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	// 		dependencies[1].srcSubpass = 0;
	// 		dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	// 		dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	// 		dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	// 		dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	// 		dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	// 		dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	// 		// Create render pass
	// 		VkRenderPassCreateInfo renderPassInfo{};
	// 		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	// 		renderPassInfo.pAttachments = attachmentDescriptions.data();
	// 		renderPassInfo.attachmentCount = static_cast<U32>(attachmentDescriptions.size());
	// 		renderPassInfo.subpassCount = 1;
	// 		renderPassInfo.pSubpasses = &subpass;
	// 		renderPassInfo.dependencyCount = 2;
	// 		renderPassInfo.pDependencies = dependencies.data();
	// 		VK_CHECK(vkCreateRenderPass(VyContext::device(), &renderPassInfo, nullptr, &m_RenderPass));

	// 		TVector<VkImageView> attachmentViews;
	// 		for (auto& attachment : m_Attachments) 
    //         {
	// 			attachmentViews.push_back(attachment.View);
	// 		}

	// 		// Find. max number of layers across attachments
	// 		U32 maxLayers = 0;
	// 		for (auto& attachment : m_Attachments) 
    //         {
	// 			if (attachment.SubresourceRange.layerCount > maxLayers) 
    //             {
	// 				maxLayers = attachment.SubresourceRange.layerCount;
	// 			}
	// 		}

	// 		VkFramebufferCreateInfo framebufferInfo{};
	// 		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	// 		framebufferInfo.renderPass = m_RenderPass;
	// 		framebufferInfo.pAttachments = attachmentViews.data();
	// 		framebufferInfo.attachmentCount = static_cast<U32>(attachmentViews.size());
	// 		framebufferInfo.width = m_Width;
	// 		framebufferInfo.height = m_Height;
	// 		framebufferInfo.layers = maxLayers;
	// 		VK_CHECK(vkCreateFramebuffer(VyContext::device(), &framebufferInfo, nullptr, &m_Framebuffer));

	// 		return VK_SUCCESS;
	// 	}

	// 	/**
	// 	 * Creates a default multi-view render pass setup with one sub pass
	// 	 *
	// 	 * @return VK_SUCCESS if all resources have been created successfully
	// 	 */
	// 	VkResult createMultiViewRenderPass(U32 multiViewCount) 
    //     {
	// 		TVector<VkAttachmentDescription> attachmentDescriptions;
			
    //         for (auto& attachment : m_Attachments) 
    //         {
	// 			attachmentDescriptions.push_back(attachment.Description);
	// 		};

	// 		// Collect attachment references
	// 		TVector<VkAttachmentReference> colorReferences;
	// 		VkAttachmentReference          depthReference{};

	// 		bool hasDepth = false;
	// 		bool hasColor = false;

	// 		U32 attachmentIndex = 0;

	// 		for (auto& attachment : m_Attachments) 
    //         {
	// 			if (attachment.isDepthStencil()) 
    //             {
	// 				// Only one depth attachment allowed
	// 				assert(!hasDepth);

	// 				depthReference.attachment = attachmentIndex;
	// 				depthReference.layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	// 				hasDepth = true;
	// 			} 
    //             else {
	// 				colorReferences.push_back({ attachmentIndex, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
	// 				hasColor = true;
	// 			}

	// 			attachmentIndex++;
	// 		};

	// 		// Default render pass setup uses only one subpass
	// 		VkSubpassDescription subpass{};
    //         {
    //             subpass.pipelineBindPoint           = VK_PIPELINE_BIND_POINT_GRAPHICS;

    //             if (hasColor) 
    //             {
    //                 subpass.pColorAttachments       = colorReferences.data();
    //                 subpass.colorAttachmentCount    = static_cast<U32>(colorReferences.size());
    //             }
    //             if (hasDepth) 
    //             {
    //                 subpass.pDepthStencilAttachment = &depthReference;
    //             }
    //         }

	// 		// Use subpass dependencies for attachment layout transitions
	// 		TArray<VkSubpassDependency, 2> dependencies{};
    //         {
    //             dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    //             dependencies[0].dstSubpass = 0;
    //             dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    //             dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    //             dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    //             dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    //             dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
                
    //             dependencies[1].srcSubpass = 0;
    //             dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
    //             dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    //             dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    //             dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    //             dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    //             dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
    //         }

	// 		// Create render pass
	// 		VkRenderPassCreateInfo renderPassInfo{};
    //         {
    //             renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    //             renderPassInfo.pAttachments = attachmentDescriptions.data();
    //             renderPassInfo.attachmentCount = static_cast<U32>(attachmentDescriptions.size());
    //             renderPassInfo.subpassCount = 1;
    //             renderPassInfo.pSubpasses = &subpass;
    //             renderPassInfo.dependencyCount = 2;
    //             renderPassInfo.pDependencies = dependencies.data();
    //         }

	// 		/*
	// 		    Setup multiview info for the renderpass
	// 		*/

	// 		/*
	// 		    Bit mask that specifies which view rendering is broadcast to
	// 		    0011 = Broadcast to first and second view (layer)
	// 		*/
	// 		const U32 kViewMask = 0b00111111;

	// 		/*
	// 		    Bit mask that specifies correlation between views
	// 		    An implementation may use this for optimizations (concurrent render)
	// 		*/
	// 		const U32 kCorrelationMask = 0b00111111;

	// 		VkRenderPassMultiviewCreateInfo renderPassMultiviewCreateInfo{};
    //         {
    //             renderPassMultiviewCreateInfo.sType        = VK_STRUCTURE_TYPE_RENDER_PASS_MULTIVIEW_CREATE_INFO;
    //             renderPassMultiviewCreateInfo.subpassCount = 1;
    //             renderPassMultiviewCreateInfo.pViewMasks   = &kViewMask;
    //             // renderPassMultiviewCreateInfo.correlationMaskCount = 1;
    //             // renderPassMultiviewCreateInfo.pCorrelationMasks = &kCorrelationMask;
    //         }

	// 		renderPassInfo.pNext = &renderPassMultiviewCreateInfo;
	// 		VK_CHECK(vkCreateRenderPass(VyContext::device(), &renderPassInfo, nullptr, &m_RenderPass));

	// 		TVector<VkImageView> attachmentViews;
	// 		for (auto& attachment : m_Attachments) 
    //         {
	// 			attachmentViews.push_back(attachment.View);
	// 		}

	// 		// Find. max number of layers across attachments
	// 		U32 maxLayers = 0;
	// 		for (auto& attachment : m_Attachments) 
    //         {
	// 			if (attachment.SubresourceRange.layerCount > maxLayers) 
    //             {
	// 				maxLayers = attachment.SubresourceRange.layerCount;
	// 			}
	// 		}

	// 		VkFramebufferCreateInfo framebufferInfo{};
    //         {
    //             framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    //             framebufferInfo.renderPass = m_RenderPass;
    //             framebufferInfo.pAttachments = attachmentViews.data();
    //             framebufferInfo.attachmentCount = static_cast<U32>(attachmentViews.size());
    //             framebufferInfo.width = m_Width;
    //             framebufferInfo.height = m_Height;
    //             framebufferInfo.layers = 1;
    //         }

    //         VK_CHECK(vkCreateFramebuffer(VyContext::device(), &framebufferInfo, nullptr, &m_Framebuffer));

	// 		return VK_SUCCESS;
	// 	}
	// };












    // class FrameBufferBuilder;
	// class FrameBuffer
	// {
	// public:
	// 	using Builder = FrameBufferBuilder;

	// 	FrameBuffer():
	// 		m_Framebuffer(VK_NULL_HANDLE) {}

	// 	FrameBuffer(const VkFramebufferCreateInfo& info, VkAllocationCallbacks* allocator = nullptr)
	// 	{
    //         VK_CHECK(vkCreateFramebuffer(VyContext::device(), &info, allocator, &m_Framebuffer));
	// 	}

	// 	FrameBuffer(const FrameBuffer&) = delete;
	// 	FrameBuffer& operator=(const FrameBuffer&) = delete;

	// 	FrameBuffer(FrameBuffer&& other) noexcept
	// 	{
	// 		m_Framebuffer	= other.m_Framebuffer;
	// 		other.m_Framebuffer	= VK_NULL_HANDLE;
	// 	}

	// 	FrameBuffer& operator=(FrameBuffer&& other) noexcept
	// 	{
	// 		m_Framebuffer	= other.m_Framebuffer;
	// 		other.m_Framebuffer	= VK_NULL_HANDLE;
	// 		return *this;
	// 	}

	// 	void release(VkDevice device, VkAllocationCallbacks* allocator = nullptr)
	// 	{
	// 		if (m_Framebuffer != VK_NULL_HANDLE)
	// 		{
	// 			vkDestroyFramebuffer(device, m_Framebuffer, allocator);
	// 			m_Framebuffer = VK_NULL_HANDLE;
	// 		}
	// 	}

	// 	inline operator VkFramebuffer() const noexcept { return m_Framebuffer; }
	// 	inline const VkFramebuffer* operator&() const noexcept { return &m_Framebuffer; }

	// 	inline VkFramebuffer handle() const noexcept { return m_Framebuffer; }
	// 	inline const VkFramebuffer* handlePtr() const noexcept { return &m_Framebuffer; }

	// private:
	// 	VkFramebuffer m_Framebuffer;
	// };

	// class FrameBufferBuilder
	// {
	// public:
	// 	FrameBufferBuilder()
	// 	{
	// 		info.sType			 = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	// 		info.pNext			 = nullptr;
	// 		info.flags			 = 0;
	// 		info.renderPass		 = VK_NULL_HANDLE;
	// 		info.attachmentCount = 0;
	// 		info.pAttachments	 = nullptr;
	// 		info.width			 = 0;
	// 		info.height			 = 0;
	// 		info.layers			 = 1;
	// 	}

	// 	FrameBufferBuilder& setRenderPass(VkRenderPass render_pass)
	// 	{
	// 		info.renderPass = render_pass;
	// 		return *this;
	// 	}

	// 	FrameBufferBuilder& setAttachments(const VkImageView* attachments, U32 attachment_count)
	// 	{
	// 		info.attachmentCount = attachment_count;
	// 		info.pAttachments	 = attachments;
	// 		return *this;
	// 	}

	// 	FrameBufferBuilder& setSize(U32 width, U32 height)
	// 	{
	// 		info.width	= width;
	// 		info.height = height;
	// 		return *this;
	// 	}

	// 	FrameBufferBuilder& setSize(VkExtent2D extent)
	// 	{
	// 		return setSize(extent.width, extent.height);
	// 	}

	// 	FrameBufferBuilder& setLayers(U32 layers)
	// 	{
	// 		info.layers = layers;
	// 		return *this;
	// 	}

	// 	const VkFramebufferCreateInfo& getCreateInfo() { return info; }

	// 	FrameBuffer build(VkAllocationCallbacks* allocator = nullptr)
	// 	{
	// 		return FrameBuffer(info, allocator);
	// 	}

	// private:
	// 	VkFramebufferCreateInfo info;
	// };


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
    //             vkDestroyImageView(VyContext::device(), views[i], nullptr);
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


