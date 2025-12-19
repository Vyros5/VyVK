#pragma once

#include <Vy/GFX/Backend/Image/Image.h>
#include <Vy/GFX/Backend/Image/ImageView.h>
#include <Vy/GFX/Backend/Image/Sampler.h>

#include <Vy/GFX/Context.h>

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

        const VkAttachmentDescription& description() const { return m_Data.Description; }
        VkAttachmentDescription&       description()       { return m_Data.Description; }

    private:
        VyAttachmentData m_Data;
    };

    void testFn()
    {
        VyAttachment depthAttachment = VyAttachment::Builder(VY_ATTACHMENT_DEPTH)
            .setFormat (VK_FORMAT_D32_SFLOAT)
            .setSamples(VK_SAMPLE_COUNT_1_BIT)
            .setLoadOpClear()
            .setStoreOpStore()
            .setStencilLoadOpDontCare()
            .setStencilStoreOpDontCare()
            .setFinalLayoutDepthStencilAttachment()
        .build();

        VkAttachmentReference depthReference{};
        {
            depthReference.attachment = 0;
            depthReference.layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        }

        VkSubpassDescription subpass{};
        {
            subpass.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
            subpass.colorAttachmentCount    = 0;
            subpass.pDepthStencilAttachment = &depthReference;
        }

        std::array<VkSubpassDependency, 2> dependencies;
        {
            dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
            dependencies[0].dstSubpass = 0;
            dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            dependencies[0].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
            dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
            dependencies[0].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
            
            dependencies[1].srcSubpass = 0;
            dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
            dependencies[1].srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
            dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            dependencies[1].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
        }

        VkRenderPassCreateInfo renderPassInfo{};
        {
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
            renderPassInfo.attachmentCount = 1;
            renderPassInfo.pAttachments = &depthAttachment.description();;
            renderPassInfo.subpassCount = 1;
            renderPassInfo.pSubpasses = &subpass;
            renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
            renderPassInfo.pDependencies = dependencies.data();
        }

         
    }
}