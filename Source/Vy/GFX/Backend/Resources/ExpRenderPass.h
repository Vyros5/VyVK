#pragma once

#include <Vy/GFX/Backend/Image/Image.h>
#include <Vy/GFX/Backend/Image/ImageView.h>
#include <Vy/GFX/Backend/Image/Sampler.h>
#include <Vy/GFX/Backend/Resources/Framebuffer.h>
#include <Vy/GFX/Context.h>

namespace Vy
{
    class RenderPass;

    // using VyAttachmentFlags = U8;

    // enum VyAttachmentFlagBits : VyAttachmentFlags
    // {
    //     VY_ATTACHMENT_NONE    = 0,
    //     VY_ATTACHMENT_COLOR   = VY_BIT(0),
    //     VY_ATTACHMENT_DEPTH   = VY_BIT(1),
    //     VY_ATTACHMENT_STENCIL = VY_BIT(2),
    //     VY_ATTACHMENT_INPUT   = VY_BIT(3),

    //     VY_ATTACHMENT_DEPTH_STENCIL = VY_ATTACHMENT_DEPTH | VY_ATTACHMENT_STENCIL
    // };

    // VY_ENUM_CLASS_FLAG(U8, VyAttachmentFlagBits)


    // struct VyAttachmentRef
    // {
    //     const U32 Index;

    //     VyAttachmentRef(U32 index) : 
    //         Index(index) 
    //     {
    //     }
    // };

    // class VyAttachment
    // {
    // public:
    //    class Builder
    //     {
    //     public:
    //         Builder(U32 index, const VyAttachmentFlags flags
    //         )
    //         {
    //             VY_ASSERT(flags, "Attachment must have at least one type flag");
    //             VY_ASSERT(!((flags & VY_ATTACHMENT_COLOR) && (flags & VY_ATTACHMENT_DEPTH)),   "Attachment must be color or depth, not both");
    //             VY_ASSERT(!((flags & VY_ATTACHMENT_COLOR) && (flags & VY_ATTACHMENT_STENCIL)), "Attachment must be color or stencil, not both");

	// 			m_Description = {};
    //             {
    //                 m_Description.flags          = 0;
    //                 m_Description.format         = VK_FORMAT_R8G8B8A8_SRGB;
    //                 m_Description.samples        = VK_SAMPLE_COUNT_1_BIT;
    //                 m_Description.loadOp         = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    //                 m_Description.storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    //                 m_Description.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    //                 m_Description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    //                 m_Description.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    //                 m_Description.finalLayout    = VK_IMAGE_LAYOUT_UNDEFINED;
    //             }

    //             if (flags & VY_ATTACHMENT_COLOR)
    //             {
    //                 m_Description.loadOp  = VK_ATTACHMENT_LOAD_OP_CLEAR;
    //                 m_Description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    //             }

    //             if (flags & VY_ATTACHMENT_DEPTH)
    //             {
    //                 m_Description.loadOp  = VK_ATTACHMENT_LOAD_OP_CLEAR;
    //                 m_Description.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    //             }
                
    //             if (flags & VY_ATTACHMENT_STENCIL)
    //             {
    //                 m_Description.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_CLEAR;
    //                 m_Description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    //             }
    //         }

    //         // const VkAttachmentDescription& getDescription() const { return m_Description; }
    //         // const VkClearValue&            getClearValue()  const { return m_ClearValue;  }
            
    //         // Property Setters

    //         // Builder& setClearValue    (const VkClearValue&   value) { m_ClearValue                 = value; return *this; }
    //         Builder& setFormat        (VkFormat              value) { m_Description.format         = value; return *this; }
    //         Builder& setSamples       (VkSampleCountFlagBits value) { m_Description.samples        = value; return *this; }
    //         Builder& setLoadOp        (VkAttachmentLoadOp    value) { m_Description.loadOp         = value; return *this; }
    //         Builder& setStoreOp       (VkAttachmentStoreOp   value) { m_Description.storeOp        = value; return *this; }
    //         Builder& setStencilLoadOp (VkAttachmentLoadOp    value) { m_Description.stencilLoadOp  = value; return *this; }
    //         Builder& setStencilStoreOp(VkAttachmentStoreOp   value) { m_Description.stencilStoreOp = value; return *this; }
    //         Builder& setInitialLayout (VkImageLayout         value) { m_Description.initialLayout  = value; return *this; }
    //         Builder& setFinalLayout   (VkImageLayout         value) { m_Description.finalLayout    = value; return *this; }

    //         Builder& setLayouts(VkImageLayout initialLayout, VkImageLayout finalLayout) 
    //         { 
    //             setInitialLayout(initialLayout); 
    //             setFinalLayout(finalLayout); 
    //             return *this; 
    //         }

    //         // Specialized Setters

    //         // Load
    //         Builder& setLoadOpLoad    () { return setLoadOp(VK_ATTACHMENT_LOAD_OP_LOAD     ); }
    //         Builder& setLoadOpClear   () { return setLoadOp(VK_ATTACHMENT_LOAD_OP_CLEAR    ); }
    //         Builder& setLoadOpDontCare() { return setLoadOp(VK_ATTACHMENT_LOAD_OP_DONT_CARE); }

    //         // Store
    //         Builder& setStoreOpStore   () { return setStoreOp(VK_ATTACHMENT_STORE_OP_STORE    ); }
    //         Builder& setStoreOpDontCare() { return setStoreOp(VK_ATTACHMENT_STORE_OP_DONT_CARE); }

    //         // Stencil Load
    //         Builder& setStencilLoadOpLoad    () { return setStencilLoadOp(VK_ATTACHMENT_LOAD_OP_LOAD     ); }
    //         Builder& setStencilLoadOpClear   () { return setStencilLoadOp(VK_ATTACHMENT_LOAD_OP_CLEAR    ); }
    //         Builder& setStencilLoadOpDontCare() { return setStencilLoadOp(VK_ATTACHMENT_LOAD_OP_DONT_CARE); }

    //         // Stencil Store
    //         Builder& setStencilStoreOpStore   () { return setStencilStoreOp(VK_ATTACHMENT_STORE_OP_STORE    ); }
    //         Builder& setStencilStoreOpDontCare() { return setStencilStoreOp(VK_ATTACHMENT_STORE_OP_DONT_CARE); }

    //         // Initial Layout
    //         Builder& setInitialLayoutColorAttachment() { return setInitialLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL); }
            
    //         // Final Layout
    //         Builder& setFinalLayoutColorAttachment       () { return setFinalLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL        ); }
    //         Builder& setFinalLayoutDepthStencilAttachment() { return setFinalLayout(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL); }
    //         Builder& setFinalLayoutShaderReadOnly        () { return setFinalLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL        ); }
    //         Builder& setFinalLayoutPresentSrc            () { return setFinalLayout(VK_IMAGE_LAYOUT_PRESENT_SRC_KHR                 ); }
    //         Builder& setFinalLayoutTransferSrc           () { return setFinalLayout(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL            ); }
    //         Builder& setFinalLayoutTransferDst           () { return setFinalLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL            ); }

    //         // Return Expressions
    //         // Builder& endAttachment()
    //         // {
    //         //     return m_Builder;
    //         // }

    //     private:
    //         VkAttachmentDescription m_Description{};
    //         // VyAttachment             m_Attachment{};
    //         // VkClearValue			 m_ClearValue;
    //         TVector<VkFormat>        m_Formats;

    //         friend class VyAttachment;
    //     };

    //     VyAttachmentRef build() const
    //     {

    //     }

    // private:
    //     VyAttachmentRef m_Ref;
    // };



    class RenderPass
    {
    public:
        class Builder;

        // =================================================================================================================
        // MARK: Attachment

        struct Attachment
        {
            VkAttachmentDescription Description{};
            VyAttachmentFlags       Flags;
        };

        class AttachmentBuilder
        {
        public:
            AttachmentBuilder(
                Builder&          builder, 
                VyAttachmentFlags flags
            ) : 
                m_Builder{ builder }
            {
                VY_ASSERT(flags,                                                               "Attachment must have at least one type flag");
                VY_ASSERT(!((flags & VY_ATTACHMENT_COLOR) && (flags & VY_ATTACHMENT_DEPTH)),   "Attachment must be color or depth, not both");
                VY_ASSERT(!((flags & VY_ATTACHMENT_COLOR) && (flags & VY_ATTACHMENT_STENCIL)), "Attachment must be color or stencil, not both");

				m_Attachment.Description = {};
                {
                    m_Attachment.Description.flags          = 0;
                    m_Attachment.Description.format         = VK_FORMAT_R8G8B8A8_SRGB;
                    m_Attachment.Description.samples        = VK_SAMPLE_COUNT_1_BIT;
                    m_Attachment.Description.loadOp         = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                    m_Attachment.Description.storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                    m_Attachment.Description.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                    m_Attachment.Description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                    m_Attachment.Description.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
                    m_Attachment.Description.finalLayout    = VK_IMAGE_LAYOUT_UNDEFINED;
                }

                if (flags & VY_ATTACHMENT_COLOR)
                {
                    m_Attachment.Description.loadOp  = VK_ATTACHMENT_LOAD_OP_CLEAR;
                    m_Attachment.Description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                }

                if (flags & VY_ATTACHMENT_DEPTH)
                {
                    m_Attachment.Description.loadOp  = VK_ATTACHMENT_LOAD_OP_CLEAR;
                    m_Attachment.Description.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                }
                
                if (flags & VY_ATTACHMENT_STENCIL)
                {
                    m_Attachment.Description.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_CLEAR;
                    m_Attachment.Description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                }
            }

            // const VkAttachmentDescription& getDescription() const { return m_Description; }
            // const VkClearValue&            getClearValue()  const { return m_ClearValue;  }
            
            // Property Setters

            // AttachmentBuilder& setClearValue    (const VkClearValue&   value) { m_ClearValue                 = value; return *this; }
            AttachmentBuilder& setFormat        (VkFormat              value) { m_Attachment.Description.format         = value; return *this; }
            AttachmentBuilder& setSamples       (VkSampleCountFlagBits value) { m_Attachment.Description.samples        = value; return *this; }
            AttachmentBuilder& setLoadOp        (VkAttachmentLoadOp    value) { m_Attachment.Description.loadOp         = value; return *this; }
            AttachmentBuilder& setStoreOp       (VkAttachmentStoreOp   value) { m_Attachment.Description.storeOp        = value; return *this; }
            AttachmentBuilder& setStencilLoadOp (VkAttachmentLoadOp    value) { m_Attachment.Description.stencilLoadOp  = value; return *this; }
            AttachmentBuilder& setStencilStoreOp(VkAttachmentStoreOp   value) { m_Attachment.Description.stencilStoreOp = value; return *this; }
            AttachmentBuilder& setInitialLayout (VkImageLayout         value) { m_Attachment.Description.initialLayout  = value; return *this; }
            AttachmentBuilder& setFinalLayout   (VkImageLayout         value) { m_Attachment.Description.finalLayout    = value; return *this; }

            AttachmentBuilder& setLayouts(VkImageLayout initialLayout, VkImageLayout finalLayout) 
            { 
                setInitialLayout(initialLayout); 
                setFinalLayout(finalLayout); 
                return *this; 
            }

            // Specialized Setters

            // Load
            AttachmentBuilder& setLoadOpLoad    () { return setLoadOp(VK_ATTACHMENT_LOAD_OP_LOAD     ); }
            AttachmentBuilder& setLoadOpClear   () { return setLoadOp(VK_ATTACHMENT_LOAD_OP_CLEAR    ); }
            AttachmentBuilder& setLoadOpDontCare() { return setLoadOp(VK_ATTACHMENT_LOAD_OP_DONT_CARE); }

            // Store
            AttachmentBuilder& setStoreOpStore   () { return setStoreOp(VK_ATTACHMENT_STORE_OP_STORE    ); }
            AttachmentBuilder& setStoreOpDontCare() { return setStoreOp(VK_ATTACHMENT_STORE_OP_DONT_CARE); }

            // Stencil Load
            AttachmentBuilder& setStencilLoadOpLoad    () { return setStencilLoadOp(VK_ATTACHMENT_LOAD_OP_LOAD     ); }
            AttachmentBuilder& setStencilLoadOpClear   () { return setStencilLoadOp(VK_ATTACHMENT_LOAD_OP_CLEAR    ); }
            AttachmentBuilder& setStencilLoadOpDontCare() { return setStencilLoadOp(VK_ATTACHMENT_LOAD_OP_DONT_CARE); }

            // Stencil Store
            AttachmentBuilder& setStencilStoreOpStore   () { return setStencilStoreOp(VK_ATTACHMENT_STORE_OP_STORE    ); }
            AttachmentBuilder& setStencilStoreOpDontCare() { return setStencilStoreOp(VK_ATTACHMENT_STORE_OP_DONT_CARE); }

            // Initial Layout
            AttachmentBuilder& setInitialLayoutColorAttachment() { return setInitialLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL); }
            
            // Final Layout
            AttachmentBuilder& setFinalLayoutColorAttachment       () { return setFinalLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL        ); }
            AttachmentBuilder& setFinalLayoutDepthStencilAttachment() { return setFinalLayout(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL); }
            AttachmentBuilder& setFinalLayoutShaderReadOnly        () { return setFinalLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL        ); }
            AttachmentBuilder& setFinalLayoutPresentSrc            () { return setFinalLayout(VK_IMAGE_LAYOUT_PRESENT_SRC_KHR                 ); }
            AttachmentBuilder& setFinalLayoutTransferSrc           () { return setFinalLayout(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL            ); }
            AttachmentBuilder& setFinalLayoutTransferDst           () { return setFinalLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL            ); }

            // Return Expressions
            Builder& endAttachment()
            {
                return m_Builder;
            }

        private:
            Builder&                 m_Builder;
            Attachment               m_Attachment{};
            // VkClearValue			 m_ClearValue;
            TVector<VkFormat>        m_Formats;

            friend class Builder;
        };

        // =================================================================================================================
        // MARK: Subpass

        class SubpassBuilder
        {
        public:
            SubpassBuilder(Builder& builder, VkPipelineBindPoint bindPoint) :
                m_Builder{ builder }
            {
                m_Description.pipelineBindPoint     = bindPoint;
                m_DepthStencilAttachment.attachment = kMaxU32;
            }

            SubpassBuilder& addColorAttachment(
                U32           attachmentIndex,
                VkImageLayout layout       = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                U32           resolveIndex = kMaxU32)
            {
                m_ColorAttachments.push_back(VkAttachmentReference{ attachmentIndex, layout });

                if (resolveIndex != kMaxU32)
                {
                    m_ResolveAttachments.push_back(VkAttachmentReference{ resolveIndex, layout });

                    VY_ASSERT(m_ResolveAttachments.size() == m_ColorAttachments.size(), "Mismatched color and resolve attachments");
                }

                return *this;
            }

            SubpassBuilder& addColorAttachment(
                U32 attachmentIndex, 
                U32 resolveIndex)
            {
                return addColorAttachment(attachmentIndex, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, resolveIndex);
            }

            SubpassBuilder& addInputAttachment(
                U32           attachmentIndex, 
                VkImageLayout layout)
            {
                m_InputAttachments.push_back(VkAttachmentReference{ attachmentIndex, layout });
                return *this;
            }

            SubpassBuilder& addPreserveAttachment(U32 attachmentIndex)
            {
                m_PreserveAttachments.push_back(attachmentIndex);
                return *this;
            }

            SubpassBuilder& setDepthStencilAttachment(
                U32           attachmentIndex, 
                VkImageLayout layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
            {
                m_DepthStencilAttachment = VkAttachmentReference{ attachmentIndex, layout };
                return *this;
            }

            SubpassBuilder& setFlags(VkSubpassDescriptionFlags flags)
            {
                m_Description.flags = flags;
                return *this;
            }

            Builder& endSubpass()
            {
                m_Description.colorAttachmentCount    = m_ColorAttachments.size();
                m_Description.pColorAttachments       = m_ColorAttachments.empty() ? nullptr : m_ColorAttachments.data();
                
                m_Description.inputAttachmentCount    = m_InputAttachments.size();
                m_Description.pInputAttachments       = m_InputAttachments.empty() ? nullptr : m_InputAttachments.data();
                
                m_Description.preserveAttachmentCount = m_PreserveAttachments.size();
                m_Description.pPreserveAttachments    = m_PreserveAttachments.empty() ? nullptr : m_PreserveAttachments.data();
                
                m_Description.pResolveAttachments     = m_ResolveAttachments.empty()  ? nullptr : m_ResolveAttachments.data();
                m_Description.pDepthStencilAttachment = m_DepthStencilAttachment.attachment == kMaxU32 ? nullptr : &m_DepthStencilAttachment;

                return m_Builder;
            }

        private:

            Builder&                       m_Builder;
            VkSubpassDescription           m_Description{};
            TVector<VkAttachmentReference> m_ColorAttachments{};
            TVector<VkAttachmentReference> m_InputAttachments{};
            TVector<U32>                   m_PreserveAttachments{};
            TVector<VkAttachmentReference> m_ResolveAttachments{};
            VkAttachmentReference          m_DepthStencilAttachment{};

            friend class Builder;
        };

        // =================================================================================================================
        // MARK: Dependency

        class DependencyBuilder
        {
        public:
            DependencyBuilder(Builder& builder, U32 srcSubpassIndex, U32 dstSubpassIndex)
                : m_Builder{ builder }
            {
                m_Dependency.srcSubpass = srcSubpassIndex;
                m_Dependency.dstSubpass = dstSubpassIndex;
            }

            DependencyBuilder& setSrcStageMask(VkPipelineStageFlags srcStageMask, bool exclusive = false)
            {
                if (exclusive)
                {
                    m_Dependency.srcStageMask = srcStageMask;
                }
                else
                {
                    m_Dependency.srcStageMask |= srcStageMask;
                }
                
                return *this;
            }

            DependencyBuilder& setDstStageMask(VkPipelineStageFlags dstStageMask, bool exclusive = false)
            {
                if (exclusive)
                {
                    m_Dependency.dstStageMask = dstStageMask;
                }
                else
                {
                    m_Dependency.dstStageMask |= dstStageMask;
                }

                return *this;
            }

            DependencyBuilder& setStageMasks(VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, bool exclusive = false)
            {
                setSrcStageMask(srcStageMask, exclusive);
                setDstStageMask(dstStageMask, exclusive);

                return *this;
            }

            DependencyBuilder& setSrcAccessMask(VkAccessFlags srcAccessMask, bool exclusive = false)
            {
                if (exclusive)
                {
                    m_Dependency.srcAccessMask = srcAccessMask;
                }
                else
                {
                    m_Dependency.srcAccessMask |= srcAccessMask;
                }
                
                return *this;
            }

            DependencyBuilder& setDstAccessMask(VkAccessFlags dstAccessMask, bool exclusive = false)
            {
                if (exclusive)
                {
                    m_Dependency.dstAccessMask = dstAccessMask;
                }
                else
                {
                    m_Dependency.dstAccessMask |= dstAccessMask;
                }

                return *this;
            }

            DependencyBuilder& setAccessMasks(VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask, bool exclusive = false)
            {
                setSrcAccessMask(srcAccessMask, exclusive);
                setDstAccessMask(dstAccessMask, exclusive);

                return *this;
            }

            DependencyBuilder& setFlags(VkDependencyFlags flags)
            {
                m_Dependency.dependencyFlags = flags;

                return *this;
            }

            Builder& endDependency()
            {
                return m_Builder;
            }

        private:

            Builder&            m_Builder;
            VkSubpassDependency m_Dependency{};

            friend class Builder;
        };

        // =================================================================================================================
        // MARK: Builder

        class Builder
        {
        public:
            Builder()
            {
            }

            RenderPass::AttachmentBuilder& beginAttachment(VyAttachmentFlags flags)
            {
                return m_Attachments.emplace_back( *this, flags );
            }

            RenderPass::SubpassBuilder& beginSubpass(VkPipelineBindPoint bindPoint)
            {
                return m_Subpasses.emplace_back( *this, bindPoint );
            }

            RenderPass::DependencyBuilder& beginDependency(U32 srcSubpass, U32 dstSubpass)
            {
                return m_Dependencies.emplace_back( *this, srcSubpass, dstSubpass );
            }

            Builder& setFlags(VkRenderPassCreateFlags flags)
            {
                m_CreateFlags = flags;
                return *this;
            }

            Builder& addFlags(VkRenderPassCreateFlags flags)
            {
                m_CreateFlags |= flags;
                return *this;
            }

            Builder& removeFlags(VkRenderPassCreateFlags flags)
            {
                m_CreateFlags &= ~flags;
                return *this;
            }

            RenderPass build() const
            {
                if (m_Subpasses.empty())
                {
                    VY_THROW_RUNTIME_ERROR("RenderPass must have at least one subpass");
                }

                TVector<RenderPass::Attachment>  attachments;
                TVector<VkAttachmentDescription> attachDescriptions;

                for (const AttachmentBuilder& attachment : m_Attachments)
                {
                    TVector<VkFormat> formats = attachment.m_Formats;

                    if (formats.empty())
                    {
                        // Color
                        if (attachment.m_Attachment.Flags & VY_ATTACHMENT_COLOR)
                        {
                            formats.emplace_back(VK_FORMAT_B8G8R8A8_SRGB);
                        }
                        // Depth Stencil
                        else if ((attachment.m_Attachment.Flags & VY_ATTACHMENT_DEPTH  ) &&
                                 (attachment.m_Attachment.Flags & VY_ATTACHMENT_STENCIL) ||
                                 (attachment.m_Attachment.Flags & VY_ATTACHMENT_DEPTH_STENCIL))
                        {
                            formats.emplace_back(VK_FORMAT_D32_SFLOAT_S8_UINT);
                        }
                        // Depth
                        else if (attachment.m_Attachment.Flags & VY_ATTACHMENT_DEPTH)
                        {
                            formats.emplace_back(VK_FORMAT_D32_SFLOAT);
                        }
                        // Stencil
                        else if (attachment.m_Attachment.Flags & VY_ATTACHMENT_STENCIL)
                        {
                            formats.emplace_back(VK_FORMAT_S8_UINT);
                        }
                    }

                    VkFormatFeatureFlags flags = 0;

                    // Color
                    if (attachment.m_Attachment.Flags & VY_ATTACHMENT_COLOR)
                    {
                        flags = VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT;
                    }
                    // Depth Stencil
                    else if ((attachment.m_Attachment.Flags & VY_ATTACHMENT_DEPTH  ) ||
                             (attachment.m_Attachment.Flags & VY_ATTACHMENT_STENCIL) ||
                             (attachment.m_Attachment.Flags & VY_ATTACHMENT_DEPTH_STENCIL))
                    {
                        flags = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
                    }

                    VkFormat fmt = VyContext::device().findSupportedFormat(formats, VK_IMAGE_TILING_OPTIMAL, flags);

                    if (fmt == VK_FORMAT_UNDEFINED)
                    {
                        VY_THROW_RUNTIME_ERROR("Unsupported format");
                    }

                    Attachment att = attachment.m_Attachment;
                    att.Description.format = fmt;

                    attachments.emplace_back(att);
                    attachDescriptions.emplace_back(att.Description);
                }

                TVector<VkSubpassDescription> subpassDescriptions;
                for (const auto& subpass : m_Subpasses)
                {
                    subpassDescriptions.emplace_back(subpass.m_Description);
                }

                TVector<VkSubpassDependency> subpassDependencies;
                for (const auto& dependency : m_Dependencies)
                {
                    subpassDependencies.emplace_back(dependency.m_Dependency);
                }

                VkRenderPassCreateInfo createInfo{ VKInit::renderPassCreateInfo() };
                {
                    createInfo.attachmentCount = (U32)attachDescriptions .size();
                    createInfo.pAttachments    =      attachDescriptions .data();

                    createInfo.subpassCount    = (U32)subpassDescriptions.size();
                    createInfo.pSubpasses      =      subpassDescriptions.data();
                    
                    createInfo.dependencyCount = (U32)subpassDependencies.size();
                    createInfo.pDependencies   =      subpassDependencies.data();
                    
                    createInfo.flags           = m_CreateFlags;
                }
                
                VkRenderPass renderPass;
                VK_CHECK(vkCreateRenderPass(VyContext::device(), &createInfo, nullptr, &renderPass));

                return RenderPass{ renderPass, attachments };
            }

        private:
            // VkRenderPassCreateInfo           m_Info;
            VkRenderPassCreateFlags                m_CreateFlags = 0;
            TVector<RenderPass::AttachmentBuilder> m_Attachments {};
		    TVector<RenderPass::SubpassBuilder>    m_Subpasses   {};
		    TVector<RenderPass::DependencyBuilder> m_Dependencies{};
        };

    public:

        // =================================================================================================================
        // MARK: RenderPass

        RenderPass() = default;

        // RenderPass(VkRenderPassCreateInfo& createInfo);

        RenderPass(const VkRenderPass renderPass, TVector<Attachment> attachments) :
            m_RenderPass { renderPass  }, 
            m_Attachments{ attachments }
        {
        }

        ~RenderPass()
        {
            destroy();
        }

        operator     VkRenderPass()        const { return m_RenderPass; }
		VY_NODISCARD VkRenderPass handle() const { return m_RenderPass; }
        
        operator bool() const
        {
            return m_RenderPass != VK_NULL_HANDLE;
        }

    private:

        void destroy()
        {
            vkDestroyRenderPass(VyContext::device(), m_RenderPass, nullptr);

            m_RenderPass = VK_NULL_HANDLE;
        }

        VkRenderPass                    m_RenderPass{ VK_NULL_HANDLE };
        TVector<RenderPass::Attachment> m_Attachments;
    };


    void testFunc()
    {
        RenderPass::Builder builder{};

        auto renderpass = builder

            // Color Attachment ( 0 )
            .beginAttachment(VY_ATTACHMENT_COLOR)
                .setFormat(VK_FORMAT_R32G32B32A32_SFLOAT)
                .setFinalLayoutPresentSrc()
            .endAttachment()

            // Depth Stencil Attachment ( 1 )
            .beginAttachment(VY_ATTACHMENT_DEPTH | VY_ATTACHMENT_STENCIL)
                .setFormat(VK_FORMAT_D32_SFLOAT_S8_UINT)
                .setFinalLayoutDepthStencilAttachment()
            .endAttachment()
            
            // Subpass ( 0 )
            .beginSubpass(VK_PIPELINE_BIND_POINT_GRAPHICS)
                .addColorAttachment       ( 0 )
                .setDepthStencilAttachment( 1 )
            .endSubpass()

            // Dependency
            .beginDependency(VK_SUBPASS_EXTERNAL, 0)
                .setSrcAccessMask(0)
                .setDstAccessMask(VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT)
                .setSrcStageMask(VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT)
                .setDstStageMask(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT)
            .endDependency()

        .build();

    }


  
}