// #include <Vy/GFX/Backend/Resources/RenderPass.h>

// #include <Vy/GFX/Context.h>

// namespace Vy
// {
//     class RPass
//     {
//     public:
//         class Builder;

//         class Attachment
//         {
//         public:
//             Attachment(
//                 Builder*              pBuilder,
//                 VkFormat              format  = VK_FORMAT_UNDEFINED, 
//                 VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT
//             ) :
//                 m_Builder{ pBuilder }
//             {
//                 mDescription = {};
//                 {
//                     mDescription.format 		= format;
//                     mDescription.samples 		= samples;
//                     mDescription.loadOp 		= VK_ATTACHMENT_LOAD_OP_CLEAR;
//                     mDescription.storeOp 		= VK_ATTACHMENT_STORE_OP_STORE;
//                     mDescription.stencilLoadOp 	= VK_ATTACHMENT_LOAD_OP_CLEAR;
//                     mDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
//                     mDescription.initialLayout 	= VK_IMAGE_LAYOUT_UNDEFINED;
//                     mDescription.finalLayout 	= VK_IMAGE_LAYOUT_UNDEFINED;
//                 }

//                 {
//                     mClearValue.color.float32[0]     = 0.0f;
//                     mClearValue.color.float32[1]     = 0.0f;
//                     mClearValue.color.float32[2]     = 0.0f;
//                     mClearValue.color.float32[3]     = 0.0f;
//                     mClearValue.depthStencil.depth   = 0.0f;
//                     mClearValue.depthStencil.stencil = 0;
//                 }
//             }

//             virtual ~Attachment() 
//             {
//             }
//             Attachment&	setFormat(VkFormat value) { mDescription.format = value; return *this; }

//             Attachment&	setSamples(VkSampleCountFlagBits value) { mDescription.samples = value; return *this; }
//             Builder& endAttachment()
//             {
//                 return *m_Builder;
//             }
//         private:
//             Builder*                m_Builder;
//             VkAttachmentDescription	mDescription;
//             VkClearValue			mClearValue;

//             friend class RenderPass;
//         };


//         class Builder
//         {
//         public:
//             Builder() = default; //: m_ImageCount(p_ImageCount)
//             // {
//             // }

//             RPass build() const
//             {
//                 // if (m_Subpasses.empty())
//                 // {
//                 //     VY_THROW_RUNTIME_ERROR("RenderPass must have at least one subpass");
//                 // }

//                 TVector<Attachment> attachments;
//                 TVector<VkAttachmentDescription> attDescriptions;

                
//             }

//             Attachment& beginAttachment(
//                 VkFormat              format  = VK_FORMAT_UNDEFINED, 
//                 VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT)
//             {
//                 return m_Attachments.emplace_back( this, format, samples );
//             }

//             // Subpass& beginSubpass(VkPipelineBindPoint bindPoint)
//             // {
//             //     return m_Subpasses.emplace_back( this, bindPoint );
//             // }

//             // Dependency& beginDependency(U32 srcSubpassIndex, U32 dstSubpassIndex)
//             // {
//             //     return m_Dependencies.emplace_back( this, srcSubpassIndex, dstSubpassIndex );
//             // }

//             // Builder &SetFlags(VkRenderPassCreateFlags flags);
//             // Builder &AddFlags(VkRenderPassCreateFlags flags);
//             // Builder &RemoveFlags(VkRenderPassCreateFlags flags);


//         private:
//             VkRenderPassCreateFlags m_Flags = 0;
//             U32                     m_ImageCount;

//             TVector<Attachment> m_Attachments{};
//             // TVector<Subpass>    m_Subpasses{};
//             // TVector<Dependency> m_Dependencies{};
//         };

//     private:
//         TVector<Attachment> m_Attachments;
//     };

//     void test()
//     {
//         auto rp = RPass::Builder()
//             .beginAttachment()
//                 .setFormat(VK_FORMAT_R32G32B32A32_SFLOAT)
//             .endAttachment()
//         .build();

//         // rp
//     }
// }