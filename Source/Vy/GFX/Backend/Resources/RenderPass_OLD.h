#pragma once

#include <Vy/GFX/Backend/Resources/Image.h>
#include <Vy/GFX/Backend/Resources/ImageView.h>
#include <Vy/GFX/Backend/Resources/Sampler.h>

// #ifdef IGNORE
//     #undef IGNORE
// #endif

// namespace Vy
// {
//     template<typename T>
//     class Pyramid 
//     {
//     private:

//         TVector<TVector<T>> pyramid;

//     public:

//         class View 
//         {
//         private:

//             const Pyramid& m_Pyramid;
//             int            m_Index;

//             friend class Pyramid;

//             View(const Pyramid& pyramid) : 
//                 m_Pyramid(pyramid), 
//                 m_Index  (0) 
//             {
//             }

//         public:

//             /**
//              * Move one layer up
//              */
//             void up() 
//             {
//                 m_Index++;
//             }

//             /**
//              * Returns a set of all the elements in the current layer
//              * and in all layers above it
//              */
//             TSet<T> collect() const 
//             {
//                 TSet<T> values;

//                 for (int i = m_Index; i < m_Pyramid.size(); i ++) 
//                 {
//                     for (const T& value : m_Pyramid.get(i)) 
//                     {
//                         values.insert(value);
//                     }
//                 }

//                 return values;
//             }
//         };

//         /**
//          * Move one layer up
//          */
//         void push() 
//         {
//             m_Pyramid.emplace_back();
//         }

//         /**
//          * Check if the pyramid has no layers (is empty)
//          */
//         bool empty() const 
//         {
//             return m_Pyramid.empty();
//         }

//         /**
//          * Get the number of layers in the pyramid
//          */
//         int size() const 
//         {
//             return m_Pyramid.size();
//         }

//         /**
//          * Get a vector of elements at the specified layer
//          */
//         const TVector<T>& get(int index) const 
//         {
//             return m_Pyramid.at(index);
//         }

//         /**
//          * Get the top layer (the layer currently being appended to)
//          */
//         const TVector<T>& top() const 
//         {
//             return m_Pyramid.back();
//         }

//         /**
//          * Add a element to the top layer
//          */
//         void append(const T& value) 
//         {
//             m_Pyramid.back().push_back(value);
//         }

//         /**
//          * Get a read-only iterator-like view of the pyramid
//          */
//         View view() const 
//         {
//             return {*this};
//         }
//     };





//     struct AttachmentOpType 
//     {
//         struct Load 
//         {
//             const VkAttachmentLoadOp VKLoad;

//             constexpr explicit Load(VkAttachmentLoadOp loadOp)
//                 : VKLoad(loadOp) 
//             {
//             }
//         };

//         struct Store 
//         {
//             const VkAttachmentStoreOp VKStore;

//             constexpr explicit Store(VkAttachmentStoreOp storeOp)
//                 : VKStore(storeOp) 
//             {
//             }
//         };

//         struct Both 
//         {
//             const VkAttachmentLoadOp  VKLoad;
//             const VkAttachmentStoreOp VKStore;

//             constexpr explicit Both(VkAttachmentLoadOp loadOp, VkAttachmentStoreOp storeOp)
//                 : VKLoad(loadOp), 
//                   VKStore(storeOp) 
//             {
//             }
//         };
//     };


//     template <typename T, typename S>
//     struct AttachmentOp 
//     {
// 	private:
// 		const S m_Values;

// 	public:

// 		template <typename ...Args>
// 		constexpr explicit AttachmentOp(Args... args)
// 		    : m_Values(args...) 
//         {
//         }

// 		template <typename R = VkAttachmentLoadOp> requires requires { m_Values.VKLoad; }
// 		constexpr auto load() const -> R {
// 			return m_Values.VKLoad;
// 		}

// 		template <typename R = VkAttachmentStoreOp> requires requires { m_Values.VKStore; }
// 		constexpr auto store() const -> R {
// 			return m_Values.VKStore;
// 		}
// };

//     struct ColorOp 
//     {
//         static constexpr AttachmentOp<ColorOp, AttachmentOpType::Load>  CLEAR  { VK_ATTACHMENT_LOAD_OP_CLEAR  };
//         static constexpr AttachmentOp<ColorOp, AttachmentOpType::Load>  LOAD   { VK_ATTACHMENT_LOAD_OP_LOAD   };
//         static constexpr AttachmentOp<ColorOp, AttachmentOpType::Store> STORE  { VK_ATTACHMENT_STORE_OP_STORE };
//         static constexpr AttachmentOp<ColorOp, AttachmentOpType::Both>  IGNORE { VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE };
//     };

//     struct StencilOp 
//     {
//         static constexpr AttachmentOp<StencilOp, AttachmentOpType::Load>  CLEAR  { VK_ATTACHMENT_LOAD_OP_CLEAR  };
//         static constexpr AttachmentOp<StencilOp, AttachmentOpType::Load>  LOAD   { VK_ATTACHMENT_LOAD_OP_LOAD   };
//         static constexpr AttachmentOp<StencilOp, AttachmentOpType::Store> STORE  { VK_ATTACHMENT_STORE_OP_STORE };
//         static constexpr AttachmentOp<StencilOp, AttachmentOpType::Both>  IGNORE { VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE };
//     };


//     /**
//      * Abstraction over the "Image-View-Sampler" triple in the context of full-screen images used
//      * in shaders as outputs (or inputs) such as the depth buffer or the various images used in deferred rendering
//      */
//     class Attachment 
//     {
// 	private:
// 		bool m_Allocated = false;

// 	public:

// 		struct Ref 
//         {
// 			const U32 Index;

// 			Ref(int index) : 
//                 Index(index) 
//             {
//             }
// 		};

// 		VkFormat             Format;
// 		VkImageUsageFlags    Usage;
// 		VkImageAspectFlags   Aspect;
// 		VkClearValue         Clear;
// 		VkFilter             Filter      = VK_FILTER_LINEAR;
// 		VkSamplerAddressMode AddressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT;
// 		VkBorderColor        BorderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;

// 		// VyImage        Image;
// 		// VyImageView    View;
// 		// VySampler      Sampler;
// 		const char*    DebugName;

// 	public:

// 		Attachment() = default;

// 		void allocate(VkExtent2D extent);

// 		void close();

//     };

//     using VyAttachmentRef = Vy::Attachment::Ref;



//     class AttachmentImageBuilder 
//     {
// 	private:

// 		Attachment attachment;

// 	public:

// 		static AttachmentImageBuilder begin()
//         {
//             return {};
//         }

// 	public:

// 		AttachmentImageBuilder& setFormat(VkFormat format)
//         {
//             attachment.Format = format;
//             return *this;
//         }
// 		AttachmentImageBuilder& setUsage(VkImageUsageFlags usage)
//         {
//             attachment.Usage = usage;
//             return *this;
//         }
// 		AttachmentImageBuilder& setAspect(VkImageAspectFlags aspect)
//         {
//             attachment.Aspect = aspect;
//             return *this;
//         }
// 		AttachmentImageBuilder& setFilter(VkFilter filter)
//         {
//             attachment.Filter = filter;
//             return *this;
//         }
// 		AttachmentImageBuilder& setMode(VkSamplerAddressMode mode)
//         {
//             attachment.AddressMode = mode;
//             return *this;
//         }
// 		AttachmentImageBuilder& setBorder(VkBorderColor border)
//         {
//             attachment.BorderColor = border;
//             return *this;
//         }
// 		AttachmentImageBuilder& setColorClearValue(float r, float g, float b, float a)
//         {
//             attachment.Clear.color = {.float32 = {r, g, b, a}};
//             return *this;
//         }
// 		AttachmentImageBuilder& setColorClearValue(int r, int g, int b, int a)
//         {
//             attachment.Clear.color = {.int32 = {r, g, b, a}};
//             return *this;
//         }
// 		AttachmentImageBuilder& setDepthClearValue(float depth, U32 stencil = 0)
//         {
//             attachment.Clear.depthStencil = {depth, stencil};
//             return *this;
//         }
// 		AttachmentImageBuilder& setDebugName(const char* name)
//         {
//         #if !defined(NDEBUG)
//             attachment.DebugName = name;
//         #endif
//             return *this;
//         }

// 		Attachment build() const
//         {
//             return attachment;
//         }
//     };



//     class RenPass
//     {
//     public:
//         class Builder;

//         // https://github.com/dark-tree/vlt3d/blob/master/src/client/renderer.cpp#L171
//         // https://github.com/dark-tree/vlt3d/blob/c9dff706737c2dff87763a7591870d6a20175c35/src/render/pass.hpp#L383

//         // -------------------------------------------------------------------------------------------------------
//         // RenderPass

//         // https://docs.vulkan.org/spec/latest/chapters/renderpass.html#VkRenderPassCreateInfo
//         // VkRenderPassCreateInfo
//         //     VkRenderPassCreateFlags           flags;

//         //     U32                          attachmentCount;
//         //     const VkAttachmentDescription*    pAttachments;
        
//         //     U32                          subpassCount;
//         //     const VkSubpassDescription*       pSubpasses;
        
//         //     U32                          dependencyCount;
//         //     const VkSubpassDependency*        pDependencies;

//         // -------------------------------------------------------------------------------------------------------
//         // Attachment

//         // https://docs.vulkan.org/spec/latest/chapters/renderpass.html#VkAttachmentDescription
//         // VkAttachmentDescription
//         //     VkAttachmentDescriptionFlags    flags;

//         //     VkFormat                        format;
        
//         //     VkSampleCountFlagBits           samples;
        
//         //     VkAttachmentLoadOp              loadOp;
//         //     VkAttachmentStoreOp             storeOp;
        
//         //     VkAttachmentLoadOp              stencilLoadOp;
//         //     VkAttachmentStoreOp             stencilStoreOp;
        
//         //     VkImageLayout                   initialLayout;
//         //     VkImageLayout                   finalLayout;

//         struct AttachmentDesc
//         {
//             VkFormat              Format;
//             VkSampleCountFlagBits Samples;
//             VkAttachmentLoadOp    LoadOp;
//             VkAttachmentStoreOp   StoreOp;
//             VkAttachmentLoadOp    StencilLoadOp;
//             VkAttachmentStoreOp   StencilStoreOp;
//             VkImageLayout         InitialLayout;
//             VkImageLayout         FinalLayout;
//         };


//     class AttachmentBuilder 
//     {
// 	private:

// 		Builder&                m_Builder;
// 		VkAttachmentDescription m_Description {};

// 		friend class Builder;

// 		VkAttachmentDescription finalize() 
//         {
// 			return m_Description;
// 		}

// 		AttachmentBuilder& input(VkAttachmentLoadOp color, VkAttachmentLoadOp stencil, VkImageLayout layout) 
//         {
// 			m_Description.loadOp = color;
// 			m_Description.stencilLoadOp = stencil;
// 			m_Description.initialLayout = layout;
// 			return *this;
// 		}

// 		AttachmentBuilder& output(VkAttachmentStoreOp color, VkAttachmentStoreOp stencil, VkImageLayout layout) 
//         {
// 			m_Description.storeOp = color;
// 			m_Description.stencilStoreOp = stencil;
// 			m_Description.finalLayout = layout;
// 			return *this;
// 		}

// 	public:

// 		AttachmentBuilder(Builder& builder, VkFormat format, VkSampleCountFlagBits samples)
// 		    : m_Builder(builder) 
//         {
// 			m_Description.format = format;
// 			m_Description.samples = samples;
// 		}

// 		/**
// 		 * Describes how the attachment data should be treated on load at the start of a render pass
// 		 *
// 		 * @param color describes what should happen to the color data
// 		 * @param stencil describes what should happen to the stencil data
// 		 * @param layout describes what layout should be used
// 		 */
// 		template <typename C, typename S>
// 		AttachmentBuilder& begin(AttachmentOp<ColorOp, C> color, AttachmentOp<StencilOp, S> stencil, VkImageLayout layout) 
//         {
// 			return input(color.load(), stencil.load(), layout);
// 		}

// 		template <typename C>
// 		AttachmentBuilder& begin(AttachmentOp<ColorOp, C> color, VkImageLayout layout) 
//         {
// 			return begin(color, StencilOp::IGNORE, layout);
// 		}

// 		/**
// 		 * Describes how the attachment data should be treated after the render pass
// 		 *
// 		 * @param color describes what should happen to the color data
// 		 * @param stencil describes what should happen to the stencil data
// 		 * @param layout describes what layout should be used
// 		 */
// 		template <typename C, typename S>
// 		AttachmentBuilder& end(AttachmentOp<ColorOp, C> color, AttachmentOp<StencilOp, S> stencil, VkImageLayout layout) 
//         {
// 			return output(color.store(), stencil.store(), layout);
// 		}

// 		template <typename C>
// 		AttachmentBuilder& end(AttachmentOp<ColorOp, C> color, VkImageLayout layout) 
//         {
// 			return end(color, StencilOp::IGNORE, layout);
// 		}

// 		Attachment::Ref next() 
//         {
// 			return m_Builder.addAttachment( *this );
// 		}
//     };

//         // -------------------------------------------------------------------------------------------------------
//         // Subpass

//         // https://docs.vulkan.org/spec/latest/chapters/renderpass.html#VkSubpassDescription
//         // VkSubpassDescription
//         //     VkSubpassDescriptionFlags       flags;
//         //     VkPipelineBindPoint             pipelineBindPoint;

//         //     U32                        inputAttachmentCount;
//         //     const VkAttachmentReference*    pInputAttachments;
        
//         //     U32                        colorAttachmentCount;
//         //     const VkAttachmentReference*    pColorAttachments;
        
//         //     const VkAttachmentReference*    pResolveAttachments;
//         //     const VkAttachmentReference*    pDepthStencilAttachment;
        
//         //     U32                        preserveAttachmentCount;
//         //     const U32*                 pPreserveAttachments;

//         struct SubpassDesc
//         {
//             VkPipelineBindPoint   BindPoint;
//             // VkSubpassDescription           m_Description{};
//             TVector<VkAttachmentReference> m_InputAttachments{};
//             TVector<VkAttachmentReference> m_ColorAttachments{};
//             TVector<VkAttachmentReference> m_ResolveAttachments{};
//             VkAttachmentReference          m_DepthStencilAttachment{};
//             TVector<U32>                   m_PreserveAttachments{};
//         };

//         // -------------------------------------------------------------------------------------------------------
//         // Dependency

//         // https://docs.vulkan.org/spec/latest/chapters/renderpass.html#VkSubpassDependency
//         // VkSubpassDependency
//         //     U32                srcSubpass;
//         //     U32                dstSubpass;

//         //     VkPipelineStageFlags    srcStageMask;
//         //     VkPipelineStageFlags    dstStageMask;
        
//         //     VkAccessFlags           srcAccessMask;
//         //     VkAccessFlags           dstAccessMask;
        
//         //     VkDependencyFlags       dependencyFlags;

//         class Subpass 
//         {
//         private:
//             U32         m_Attachments;
//             const char* m_Name;

//         public:
//             Subpass() = default;

//             Subpass(U32 attachments, const char* name) : 
//                 m_Attachments(attachments), 
//                 m_Name(name) 
//             {
//             }

//             U32 getAttachmentCount() const 
//             {
//                 return m_Attachments;
//             }

//             const char* getDebugName() const 
//             {
//                 return m_Name;
//             }
//         };

        

//         class SubpassBuilder 
//         {
//         private:
//             friend class Builder;

//             Builder&             m_Builder;
//             VkSubpassDescription m_Description{};
//             U32                  m_AttachmentCount;
//             Pyramid<U32>&        m_Preserves;
//             TSet<U32>            m_References;
//             const char*          m_DebugName;

//             TVector<VkAttachmentReference> m_InputAttachments;
//             TVector<VkAttachmentReference> m_ColorAttachments;
//             TVector<VkAttachmentReference> m_ResolveAttachments;
//             VkAttachmentReference          m_DepthAttachment;

//         private:
//             VkAttachmentReference getReference(U32 attachmentIndex, VkImageLayout layout) 
//             {
//                 if (attachmentIndex >= m_AttachmentCount) 
//                 {
//                     VY_THROW_RUNTIME_ERROR("Attachment index " + std::to_string(attachmentIndex) + 
//                         " out of bounds, only " + std::to_string(m_AttachmentCount) + " have been defined up to this point!");
//                 }

//                 m_Preserves.append(attachmentIndex);

//                 VkAttachmentReference reference{};
//                 {
//                     reference.attachment = attachmentIndex;
//                     reference.layout     = layout;
//                 }

//                 return reference;
//             }


//             bool shouldPreserve(U32 attachment) const 
//             {
//                 return !m_References.contains(attachment);
//             }


//             VkSubpassDescription finalize(const TVector<U32>& preserveAttachments, TVector<Subpass>& subpassAttachments) 
//             {
//                 U32 inputCount   = m_InputAttachments  .size();
//                 U32 colorCount   = m_ColorAttachments  .size();
//                 U32 resolveCount = m_ResolveAttachments.size();

//                 if (resolveCount != 0 && resolveCount != colorCount) 
//                 {
//                     VY_THROW_RUNTIME_ERROR("Invalid number of resolve attachments! Must be 0 or equal to the number of color attachments!");
//                 }

//                 // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkSubpassDescription.html
//                 m_Description.inputAttachmentCount = inputCount;
//                 m_Description.pInputAttachments    = m_InputAttachments.data();
//                 m_Description.colorAttachmentCount = colorCount;
//                 m_Description.pColorAttachments    = m_ColorAttachments.data();

//                 m_Description.pResolveAttachments     = m_ResolveAttachments.data();
//                 m_Description.pDepthStencilAttachment = &m_DepthAttachment;

//                 m_Description.preserveAttachmentCount = (U32)preserveAttachments.size();
//                 m_Description.pPreserveAttachments    = preserveAttachments.data();

//                 // this is here so that the renderpass can retain the information about
//                 // how many attachments were there for each subpass - this is then used during pipeline
//                 // creation to setup blending for each attachment
//                 subpassAttachments.push_back({ colorCount, m_DebugName });

//                 return m_Description;
//             }

//         public:

//             SubpassBuilder(
//                 Builder&            builder, 
//                 VkPipelineBindPoint bindPoint, 
//                 U32                 attachmentCount, 
//                 Pyramid<U32>&       preserve, 
//                 const char*         name
//             ) : 
//                 m_Builder(builder), 
//                 m_AttachmentCount(attachmentCount), 
//                 m_Preserves(preserve) 
//             {
//                 m_Description.pipelineBindPoint = bindPoint;
//                 m_DepthAttachment.attachment    = VK_ATTACHMENT_UNUSED;

//             #if !defined(NDEBUG)
//                 this->m_DebugName = name;
//             #endif
//             }

//             /// attachments that are read from a shader
//             SubpassBuilder& addInput(VyAttachmentRef attachment, VkImageLayout targetLayout) 
//             {
//                 m_References.insert(attachment.Index);

//                 m_InputAttachments.push_back( getReference(attachment.Index, targetLayout) );

//                 return *this;
//             }

//             /// attachment for color data
//             SubpassBuilder& addOutput(VyAttachmentRef attachment, VkImageLayout targetLayout)
//             {
//                 m_References.insert(attachment.Index);

//                 m_ColorAttachments.push_back(getReference( attachment.Index, targetLayout) );
                
//                 return *this;
//             }

//             /// attachment for depth and stencil data
//             SubpassBuilder& addDepth(VyAttachmentRef attachment, VkImageLayout targetLayout) 
//             {
//                 m_References.insert(attachment.Index);

//                 m_DepthAttachment = getReference( attachment.Index, targetLayout );
                
//                 return *this;
//             }

//             /// attachments used for multisampling color attachments
//             SubpassBuilder& addResolve(VyAttachmentRef attachment, VkImageLayout targetLayout) 
//             {
//                 m_References.insert(attachment.Index);
            
//                 m_ResolveAttachments.push_back( getReference(attachment.Index, targetLayout) );
            
//                 return *this;
//             }

//             Builder& next() 
//             {
//                 return m_Builder.addSubpass( *this );
//             }
//         };



//         /**
//          * Create a dependency between two locations in the pipeline,
//          * that dependency is specified using two subpass indices, two pipeline stages, and two actions
//          */
//         class DependencyBuilder 
//         {
//         public:

//             DependencyBuilder(Builder& builder, VkDependencyFlags flags) :
//                 m_Builder{ builder }
//             {
//                 m_Description.dependencyFlags = flags;
//             }

//             /**
//              * @brief Specify the point that must be reached
//              *
//              * Specify, the operation of a stage, of a subpass, that will be waited upon,
//              * before the operation defined using then() can begin execution, this operation must be completed.
//              *
//              * @param subpassIndex  the index of the subpass in which is the stage and operation to wait for
//              * @param srcStageMask  the stage in which is the operation to wait for
//              * @param srcAccessMask the operation to wait for, pass 0 to just wait for stage
//              */
//             DependencyBuilder& input(U32 subpassIndex, VkPipelineStageFlags srcStageMask, VkAccessFlags srcAccessMask)
//             {
//                 m_Description.srcSubpass    = subpassIndex;
//                 m_Description.srcStageMask  = srcStageMask;
//                 m_Description.srcAccessMask = srcAccessMask;

//                 return *this;
//             }

//             /**
//              * @brief Specify the point that must wait
//              *
//              * Specify, the operation of a stage, of a subpass, that will wait on this dependency,
//              * before this operation can begin execution, the point defined using first() must be completed.
//              *
//              * @param subpassIndex  the index of the subpass in which is the stage and operation to wait for
//              * @param dstStageMask  the stage in which is the operation to wait for
//              * @param dstAccessMask the operation to wait for, pass 0 to just wait for stage
//              */
//             DependencyBuilder& output(U32 subpassIndex, VkPipelineStageFlags dstStageMask, VkAccessFlags dstAccessMask)
//             {
//                 m_Description.dstSubpass    = subpassIndex;
//                 m_Description.dstStageMask  = dstStageMask;
//                 m_Description.dstAccessMask = dstAccessMask;

//                 return *this;
//             }

//             /// Continue building the Render Pass
//             Builder& next()
//             {
//                 return m_Builder.addDependency( *this );
//             }

//         private:

//             VkSubpassDependency finalize()
//             {
//                 return m_Description;
//             }
            
//             Builder&            m_Builder;
//             VkSubpassDependency m_Description{};

//             friend class Builder;
//         };


//         // class Dependency
//         // {
//         // public:
//         //     Dependency(
//         //         Builder& builder, 
//         //         U32      srcSubpassIndex, 
//         //         U32      dstSubpassIndex
//         //     ) : 
//         //         m_Builder{ builder }
//         //     {
//         //         m_Dependency.srcSubpass = srcSubpassIndex;
//         //         m_Dependency.dstSubpass = dstSubpassIndex;
//         //     }

//         //     Dependency& setSrcStageMask(VkPipelineStageFlags srcStageMask, bool exclusive = false)
//         //     {
//         //         if (exclusive)
//         //         {
//         //             m_Dependency.srcStageMask = srcStageMask;
//         //         }
//         //         else
//         //         {
//         //             m_Dependency.srcStageMask |= srcStageMask;
//         //         }
                
//         //         return *this;
//         //     }

//         //     Dependency& setDstStageMask(VkPipelineStageFlags dstStageMask, bool exclusive = false)
//         //     {
//         //         if (exclusive)
//         //         {
//         //             m_Dependency.dstStageMask = dstStageMask;
//         //         }
//         //         else
//         //         {
//         //             m_Dependency.dstStageMask |= dstStageMask;
//         //         }

//         //         return *this;
//         //     }

//         //     Dependency& setStageMasks(VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, bool exclusive = false)
//         //     {
//         //         setSrcStageMask(srcStageMask, exclusive);
//         //         setDstStageMask(dstStageMask, exclusive);

//         //         return *this;
//         //     }

//         //     Dependency& setSrcAccessMask(VkAccessFlags srcAccessMask, bool exclusive = false)
//         //     {
//         //         if (exclusive)
//         //         {
//         //             m_Dependency.srcAccessMask = srcAccessMask;
//         //         }
//         //         else
//         //         {
//         //             m_Dependency.srcAccessMask |= srcAccessMask;
//         //         }
                
//         //         return *this;
//         //     }

//         //     Dependency& setDstAccessMask(VkAccessFlags dstAccessMask, bool exclusive = false)
//         //     {
//         //         if (exclusive)
//         //         {
//         //             m_Dependency.dstAccessMask = dstAccessMask;
//         //         }
//         //         else
//         //         {
//         //             m_Dependency.dstAccessMask |= dstAccessMask;
//         //         }

//         //         return *this;
//         //     }

//         //     Dependency& setAccessMasks(VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask, bool exclusive = false)
//         //     {
//         //         setSrcAccessMask(srcAccessMask, exclusive);
//         //         setDstAccessMask(dstAccessMask, exclusive);

//         //         return *this;
//         //     }

//         //     Dependency& setFlags(VkDependencyFlags flags)
//         //     {
//         //         m_Dependency.dependencyFlags = flags;

//         //         return *this;
//         //     }

//         //     // Builder& endDependency()
//         //     // {
//         //     //     return *m_Builder;
//         //     // }

//         // private:
//         //     Builder&            m_Builder;
//         //     VkSubpassDependency m_Dependency{};

//         //     friend class Builder;
//         // };
//         // -------------------------------------------------------------------------------------------------------

//         class Builder
//         {
//         public:
//             VyAttachmentRef addAttachment(AttachmentBuilder& builder) 
//             {
//                 m_Attachments.push_back( builder );

//                 return m_Attachments.size() - 1;
//             }

//             /**
//              * Adds and makes usable an attachment for subpasses in the render pass
//              */
//             AttachmentBuilder addAttachment(VkFormat format, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT) 
//             {
//                 return AttachmentBuilder{ *this, format, samples };
//             }

//             AttachmentBuilder addAttachment(const Attachment& attachment, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT) 
//             {
//                 m_Values.push_back( attachment.Clear );

//                 return addAttachment( attachment.Format, samples );
//             }

//         public:

//             Builder& addSubpass(SubpassBuilder& builder) 
//             {
//                 m_Subpasses.push_back( builder );

//                 return *this;
//             }

//             /**
//              * Adds a render pass sub-stage, subpasses are executed in order
//              */
//             SubpassBuilder addSubpass(VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS) 
//             {
//                 m_Preserves.push();

//                 return SubpassBuilder{ *this, bindPoint, (U32)m_Attachments.size(), m_Preserves, "Unnamed" };
//             }

//         public:

//             /**
//              * Add a data dependency to a subpass.
//              */
//             DependencyBuilder addDependency(VkDependencyFlags flags = 0) 
//             {
//                 return DependencyBuilder{ *this, flags };
//             }

//             Builder& addDependency(DependencyBuilder& builder) 
//             {
//                 m_Dependencies.push_back( builder );

//                 return *this;
//             }

//         public:

//             RenPass build(const char* name, Vec3 color) 
//             {
//                 TVector<VkAttachmentDescription> attachmentDescriptions;
//                 TVector<VkSubpassDescription>    subpassDescriptions;
//                 TVector<VkSubpassDependency>     dependencyDescriptions;
//                 TVector<TVector<U32>>            preserveIndices;
//                 TVector<Subpass>                 subpassInfo;

//                 auto view = m_Preserves.view();

//                 attachmentDescriptions.reserve(m_Attachments .size());
//                 subpassDescriptions   .reserve(m_Subpasses   .size());
//                 dependencyDescriptions.reserve(m_Dependencies.size());

//                 for (auto& attachment : m_Attachments) 
//                 {
//                     attachmentDescriptions.push_back(attachment.finalize());
//                 }

//                 for (auto& dependency : m_Dependencies) 
//                 {
//                     dependencyDescriptions.push_back(dependency.finalize());
//                 }

//                 for (auto& subpass : m_Subpasses) 
//                 {
//                     view.up();
//                     auto set = view.collect();

//                     preserveIndices.emplace_back();

//                     for (U32 attachment : set) 
//                     {
//                         if (subpass.shouldPreserve(attachment)) 
//                         {
//                             preserveIndices.back().push_back(attachment);
//                         }
//                     }

//                     subpassDescriptions.push_back(subpass.finalize(preserveIndices.back(), subpassInfo));
//                 }

//                 VkRenderPassCreateInfo createInfo{};
//                 {
//                     createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
                    
//                     createInfo.attachmentCount = (U32) attachmentDescriptions.size();
//                     createInfo.pAttachments = attachmentDescriptions.data();
//                     createInfo.subpassCount = (U32) subpassDescriptions.size();
//                     createInfo.pSubpasses = subpassDescriptions.data();
//                     createInfo.dependencyCount = (U32) dependencyDescriptions.size();
//                     createInfo.pDependencies = dependencyDescriptions.data();
//                 }

//                 VkRenderPass renderPass;
//                 if (vkCreateRenderPass(VyContext::device(), &createInfo, nullptr, &renderPass) != VK_SUCCESS) 
//                 {
//                     VY_THROW_RUNTIME_ERROR("Failed to create render pass!");
//                 }

//                 return RenPass{ renderPass, m_Values, subpassInfo, name, color };
//             }

//         private:
//             TVector<VkClearValue>      m_Values;
//             TVector<AttachmentBuilder> m_Attachments;
//             TVector<SubpassBuilder>    m_Subpasses;
//             TVector<DependencyBuilder> m_Dependencies;

// 		    Pyramid<U32> m_Preserves;
//         };

// 	public:

// 		// Based on: https://www.color-hex.com/color-palette/5361
// 		static constexpr Vec3 RED {255, 179, 180};
// 		static constexpr Vec3 ORANGE {255, 223, 180};
// 		static constexpr Vec3 YELLOW {255, 255, 180};
// 		static constexpr Vec3 GREEN {180, 255, 201};
// 		static constexpr Vec3 BLUE {180, 225, 255};

// 	public:
// 		VkRenderPass          m_RenderPass;
// 		TVector<VkClearValue> m_Values;
// 		TVector<Subpass>      m_Subpasses;

//     #if !defined(NDEBUG)
// 		Vec3   m_DebugColor;
// 		String m_DebugName;
//     #endif

// 	public:

// 		RenPass() = default;
// 		RenPass(
//             VkRenderPass           pass, 
//             TVector<VkClearValue>& values, 
//             TVector<Subpass>&      subpassInfos, 
//             const char*            name, 
//             Vec3                   color
//         ) : 
//             m_RenderPass(pass), 
//             m_Values(values), 
//             m_Subpasses(subpassInfos) 
//         {
// 			m_Values.shrink_to_fit();

// 			#if !defined(NDEBUG)
// 			// VulkanDebug::name(vk_device, VK_OBJECT_TYPE_RENDER_PASS, m_RenderPass, name);
// 			m_DebugName  = name;
// 			m_DebugColor = color;
// 			#endif
// 		}

// 		void close() 
//         {
// 			vkDestroyRenderPass(VyContext::device(), m_RenderPass, nullptr);// AllocatorCallbackFactory::named("RenderPass")
// 		}

// 		const Subpass& getSubpass(int subpass) const 
//         {
// 			return m_Subpasses[subpass];
// 		}

// 		int getSubpassCount() const 
//         {
// 			return m_Subpasses.size();
// 		}
//     };
// }
































namespace Vy::vk
{
    VkImageAspectFlags determineAspectMask( VkFormat format )
    {
        VkImageAspectFlags result = 0;

        switch( format ) {
            // Depth
            case VK_FORMAT_D16_UNORM:
            case VK_FORMAT_X8_D24_UNORM_PACK32:
            case VK_FORMAT_D32_SFLOAT:
            {
                result = VK_IMAGE_ASPECT_DEPTH_BIT;
            }
            break;

            // Stencil
            case VK_FORMAT_S8_UINT:
            {
                result = VK_IMAGE_ASPECT_STENCIL_BIT;
            }
            break;

            // Depth/Stencil
            case VK_FORMAT_D16_UNORM_S8_UINT:
            case VK_FORMAT_D24_UNORM_S8_UINT:
            case VK_FORMAT_D32_SFLOAT_S8_UINT:
            {
                result = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
            }
            break;

            // Assume everything else is Vec3
            default:
            {
                result = VK_IMAGE_ASPECT_COLOR_BIT;
            }
            break;
        }

        return result;
    }
}





















































//     class Framebuffer;
//     class CommandBuffer;
//     using CommandBufferRef = std::shared_ptr<CommandBuffer>;
//     using FramebufferRef = std::shared_ptr<Framebuffer>;

//     class RenderPass;
//     using RenderPassRef = std::shared_ptr<RenderPass>;

//     //! \class RenderPass
//     //!
//     //!
//     class RenderPass : public BaseDeviceObject, public std::enable_shared_from_this<RenderPass> {
//     public:

//         class Attachment;
//         class Options;
//         using AttachmentRef = std::shared_ptr<Attachment>;

//         //! \class Attachment
//         //!
//         //!
//         class Attachment 
//         {
//         public:
//             Attachment(
//                 VkFormat              format  = VK_FORMAT_UNDEFINED, 
//                 VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT)
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

//                 // Vec3
//                 if( vk::determineAspectMask( format ) & VK_IMAGE_ASPECT_COLOR_BIT ) 
//                 {
//                     mDescription.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
//                 }
//                 // Depth
//                 else if( vk::determineAspectMask( format ) & VK_IMAGE_ASPECT_DEPTH_BIT ) 
//                 {
//                     mDescription.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
//                     mClearValue.depthStencil.depth = 1.0f;
//                 }
//             }

//             virtual ~Attachment() 
//             {
//             }

//             static Attachment createColor(VkFormat format, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT)
//             {
//                 RenderPass::Attachment result = RenderPass::Attachment( format )
//                     .setSamples( samples )
//                     .setFinalLayout( VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL );
//                 return result;
//             }

//             static Attachment createDepthStencil(VkFormat format, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT)
//             {
//                 RenderPass::Attachment result = RenderPass::Attachment( format )
//                     .setSamples( samples )
//                     .setFinalLayout( VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL );
//                 return result;
//             }

//             const VkAttachmentDescription& getDescription() const { return mDescription; }

//             Attachment&	        setClearValue(const VkClearValue& value) { mClearValue = value; return *this; }
//             const VkClearValue&	getClearValue() const { return mClearValue; }

//             Attachment&	setFormat(VkFormat value) { mDescription.format = value; return *this; }

//             Attachment&	setSamples(VkSampleCountFlagBits value) { mDescription.samples = value; return *this; }

//             Attachment&	setLoadOp(VkAttachmentLoadOp value) { mDescription.loadOp = value; return *this; }
//             Attachment&	loadOpLoad    () { return setLoadOp(VK_ATTACHMENT_LOAD_OP_LOAD); }
//             Attachment&	loadOpClear   () { return setLoadOp(VK_ATTACHMENT_LOAD_OP_CLEAR); }
//             Attachment&	loadOpDontCare() { return setLoadOp(VK_ATTACHMENT_LOAD_OP_DONT_CARE); }

//             Attachment& setStoreOp(VkAttachmentStoreOp value) { mDescription.storeOp = value; return *this; }
//             Attachment& storeOpStore   () { return setStoreOp(VK_ATTACHMENT_STORE_OP_STORE); }
//             Attachment& storeOpDontCare() { return setStoreOp(VK_ATTACHMENT_STORE_OP_DONT_CARE); }

//             Attachment& setStencilLoadOp(VkAttachmentLoadOp value) { mDescription.stencilLoadOp = value; return *this; }
//             Attachment& stencilLoadOpLoad    () { return setStencilLoadOp(VK_ATTACHMENT_LOAD_OP_LOAD); }
//             Attachment& stencilLoadOpClear   () { return setStencilLoadOp(VK_ATTACHMENT_LOAD_OP_CLEAR); }
//             Attachment& stencilLoadOpDontCare() { return setStencilLoadOp(VK_ATTACHMENT_LOAD_OP_DONT_CARE); }

//             Attachment& setStencilStoreOp        (VkAttachmentStoreOp value) { mDescription.stencilStoreOp = value; return *this; }
//             Attachment& stencilStoreOpStore   () { return setStencilStoreOp(VK_ATTACHMENT_STORE_OP_STORE); }
//             Attachment& stencilStoreOpDontCare() { return setStencilStoreOp(VK_ATTACHMENT_STORE_OP_DONT_CARE); }

//             // Initial Layout
//             Attachment& setInitialLayout(VkImageLayout value) { mDescription.initialLayout = value; return *this; }
//             Attachment& initialLayoutColorAttachment() { return setInitialLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL); }
            
//             // Final Layout
//             Attachment& setFinalLayout(VkImageLayout value) { mDescription.finalLayout = value; return *this; }
//             Attachment& finalLayoutColorAttachment       () { return setFinalLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);   }
//             Attachment& finalLayoutDepthStencilAttachment() { return setFinalLayout(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);   }
//             Attachment& finalLayoutShaderReadOnly        () { return setFinalLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);   }
//             Attachment& finalLayoutPresentSrc            () { return setFinalLayout(VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);            }
//             Attachment& finalLayoutTransferSrc           () { return setFinalLayout(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);       }
//             Attachment& finalLayoutTransferDst           () { return setFinalLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);       }

//             Attachment& setLayouts(VkImageLayout initialLayout, VkImageLayout finalLayout) { setInitialLayout(initialLayout); setFinalLayout(finalLayout); return *this; }

//         private:
//             VkAttachmentDescription	mDescription;
//             VkClearValue			mClearValue;
//             friend class RenderPass;
//         };

//         //! \class Subpass
//         //!  All color and depth attachments in a subpass must have the same sample count.
//         //!
//         class Subpass {
//         public:
//             Subpass(VkPipelineBindPoint pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS)
//                 : mPipelineBindPoint(pipelineBindPoint) {}
//             virtual ~Subpass() {}

//             const TVector<U32>&	getColorAttachments() const { return mColorAttachments; }
//             const TVector<U32>&	getDepthStencilAttachment() const { return mDepthStencilAttachment; }

//             Subpass& addColorAttachment(U32 attachmentIndex, U32 resolveAttachmentIndex = VK_ATTACHMENT_UNUSED);
//             Subpass& addDepthStencilAttachment(U32 attachmentIndex);
//             Subpass& addPreserveAttachment(U32 attachmentIndex);
//             Subpass& addPreserveAttachments(const TVector<U32>& attachmentIndices);

//         private:
//             VkPipelineBindPoint		mPipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
//             TVector<U32>	mColorAttachments;			// 1 or more elements
//             TVector<U32>	mResolveAttachments;		// 1 or more elements
//             TVector<U32>	mDepthStencilAttachment;	// 1 element at most
//             TVector<U32>	mPreserveAttachments;		// 0 or more elements
//             friend class RenderPass;
//         };

//         //! \class SubpassDependency
//         //!
//         //!
//         class SubpassDependency {
//         public:
//             SubpassDependency(U32 srcSubpass, U32 dstSubpass);
//             virtual ~SubpassDependency() {}

//             SubpassDependency& setSrcSubpass(U32 subpass) { mDependency.srcSubpass = subpass; return *this; }
//             SubpassDependency& setDstSubpass(U32 subpass) { mDependency.dstSubpass = subpass; return *this; }
//             SubpassDependency& setSrcStageMask(VkPipelineStageFlags mask, bool exclusive = false);
//             SubpassDependency& setDstStageMask(VkPipelineStageFlags mask, bool exclusive = false);
//             SubpassDependency& setStageMasks(VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, bool exclusive = false);
//             SubpassDependency& setSrcAccessMask(VkAccessFlags mask, bool exclusive = false);
//             SubpassDependency& setDstAccessMask(VkAccessFlags mask, bool exclusive = false);
//             SubpassDependency& setAccessMasks(VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask, bool exclusive = false);

//         private:
//             VkSubpassDependency		mDependency;
//             friend class RenderPass;
//         };

//         //! \class Options
//         //!
//         //!
//         class Options {
//         public:

//             Options() {}
//             Options(VkFormat colorFormat, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT);
//             Options(VkFormat colorFormat, VkFormat depthStencilFormat, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT);
//             virtual ~Options() {}

//             Options& addAttachment(const Attachment& value) { mAttachments.push_back(value); return *this; }
//             Options& addSubPass(const Subpass& value) { mSubpasses.push_back(value); return *this; }
//             Options& addSubpassDependency(const SubpassDependency& value) { mSubpassDependencies.push_back(value); return *this; }
//             Options& addSubpassSelfDependency(U32 subpassIndex, VkPipelineStageFlags srcStageMask = vk::PipelineStageGraphicsBits, VkPipelineStageFlags dstStageMask = vk::PipelineStageGraphicsBits, VkAccessFlags srcAccessMask = vk::AccessFlagsBits, VkAccessFlags dstAccessMask = vk::AccessFlagsBits);

//         private:
//             TVector<Attachment>			mAttachments;
//             TVector<Subpass>			mSubpasses;
//             TVector<SubpassDependency>	mSubpassDependencies;
//             friend class RenderPass;
//         };

//         // ---------------------------------------------------------------------------------------------

//         RenderPass();
//         RenderPass(const RenderPass::Options& options, Device *device);
//         virtual ~RenderPass();

//         static RenderPassRef					create(const RenderPass::Options& options, Device *device = nullptr);

//         VkRenderPass							getRenderPass() const { return mRenderPass; }

//         const TVector<VkAttachmentDescription>&	getAttachmentDescriptors() const { return mAttachmentDescriptors; }
//         const TVector<VkClearValue>&			getAttachmentClearValues() const { return mAttachmentClearValues; }
//         void										setAttachmentClearValue(size_t attachmentIndex, const VkClearValue& clearValues);

//         const vk::CommandBufferRef&				getCommandBuffer() const { return mCommandBuffer; }
//         U32								getColorAttachmentCount() const { return static_cast<U32>(mAttachmentDescriptors.size()); }

//         VkSampleCountFlagBits					getSubpassSampleCount(U32 subpass) const;
//         U32								getSubpassColorAttachmentCount(U32 subpass) const;

//         TVector<VkClearAttachment>			getClearAttachments(U32 subpass, bool color = true, bool depthStencil = true) const;

//         void									beginRender(const vk::CommandBufferRef& cmdBuf, const vk::FramebufferRef& framebuffer);
//         void									endRender();
//         //! Same as beginRender but assumes command buffer has begun explicitly beforehand
//         void									beginRenderExplicit(const vk::CommandBufferRef& cmdBuf, const vk::FramebufferRef& framebuffer);
//         //! Same as endRender but assumes command buffer will end explicitly afterwards
//         void									endRenderExplicit();
//         void									nextSubpass();

//     private:
//         RenderPass::Options						mOptions;

//         VkRenderPass							mRenderPass = VK_NULL_HANDLE;
//         U32								mSubpass = 0;
//         TVector<VkSampleCountFlagBits>		mSubpassSampleCounts;
//         TVector<U32>					mBarrieredAttachmentIndices;

//         TVector<VkAttachmentDescription>	mAttachmentDescriptors;
//         TVector<VkClearValue>				mAttachmentClearValues;

//         vk::CommandBufferRef					mCommandBuffer;
//         vk::FramebufferRef						mFramebuffer;

//         void initialize(const RenderPass::Options& options);
//         void destroy(bool removeFromTracking = true);
//         friend class vk::Device;
//     };
// }

// namespace Vy
// {
//     /**
//      * @brief Represents a Vulkan render pass and its associated resources.
//      *
//      * Manages the configuration, creation, and destruction of Vulkan render passes, along with helper utilities for
//      * attachments, subpasses, and dependencies. Also supports resource creation for associated frame buffers and images.
//      */
//     class RenderPass
//     {
//     public:
//         struct Attachment
//         {
//             VkAttachmentDescription Description{};
//             VkImageUsageFlags Flags;
//             // Image::Flags Flags;
//         };

//         class Builder;

//     private:
//         class AttachmentBuilder
//         {
//         public:
//             AttachmentBuilder(Builder *pBuilder, VkImageUsageFlags flags);

//             AttachmentBuilder &SetLoadOperation(VkAttachmentLoadOp p_Operation,
//                                                 VkAttachmentLoadOp stencilOperation = VK_ATTACHMENT_LOAD_OP_MAX_ENUM);
//             AttachmentBuilder &SetStoreOperation(VkAttachmentStoreOp p_Operation,
//                                                 VkAttachmentStoreOp stencilOperation = VK_ATTACHMENT_STORE_OP_MAX_ENUM);

//             AttachmentBuilder &SetStencilLoadOperation(VkAttachmentLoadOp p_Operation);
//             AttachmentBuilder &SetStencilStoreOperation(VkAttachmentStoreOp p_Operation);

//             AttachmentBuilder &RequestFormat(VkFormat format);
//             AttachmentBuilder &AllowFormat(VkFormat format);

//             AttachmentBuilder &SetLayouts(VkImageLayout p_InitialLayout, VkImageLayout finalLayout);
//             AttachmentBuilder &SetInitialLayout(VkImageLayout layout);
//             AttachmentBuilder &SetFinalLayout(VkImageLayout layout);

//             AttachmentBuilder &SetSampleCount(VkSampleCountFlagBits sampleCount);
//             AttachmentBuilder &SetFlags(VkAttachmentDescriptionFlags flags);

//             Builder &EndAttachment();

//         private:
//             Builder*  m_Builder;
//             Attachment m_Attachment{};
//             TArray16<VkFormat> m_Formats;

//             friend class Builder;
//         };

//         class SubpassBuilder
//         {
//         public:
            
//             SubpassBuilder(Builder *pBuilder, VkPipelineBindPoint p_BindPoint);

//             SubpassBuilder &AddColorAttachment(U32 attachmentIndex,
//                                             VkImageLayout layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
//                                             U32 resolveIndex = std::numeric_limits<U32>::max());
//             SubpassBuilder &AddColorAttachment(U32 attachmentIndex, U32 resolveIndex);

//             SubpassBuilder &AddInputAttachment(U32 attachmentIndex, VkImageLayout layout);
//             SubpassBuilder &AddPreserveAttachment(U32 attachmentIndex);

//             SubpassBuilder &SetDepthStencilAttachment(U32 attachmentIndex,
//                                                     VkImageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

//             SubpassBuilder &SetFlags(VkSubpassDescriptionFlags flags);

//             Builder &EndSubpass();

//         private:
//             Builder*                       m_Builder;
//             VkSubpassDescription           m_Description{};
//             TArray8<VkAttachmentReference> m_ColorAttachments{};
//             TArray8<VkAttachmentReference> m_InputAttachments{};
//             TArray8<U32>                   m_PreserveAttachments{};
//             TArray8<VkAttachmentReference> m_ResolveAttachments{};
//             VkAttachmentReference          m_DepthStencilAttachment{};

//             friend class Builder;
//         };

//         class DependencyBuilder
//         {
//         public:
//             DependencyBuilder(Builder* pBuilder, U32 srcSubpassIndex, U32 dstSubpassIndex);

//             DependencyBuilder &SetStageMask(VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStage);
//             DependencyBuilder &SetAccessMask(VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask);

//             DependencyBuilder &SetFlags(VkDependencyFlags flags);

//             Builder &EndDependency();

//         private:
//             Builder *m_Builder;
//             VkSubpassDependency m_Dependency{};

//             friend class Builder;
//         };

//     public:
//         /**
//          * @brief A utility for constructing Vulkan render passes.
//          *
//          * Provides methods for configuring attachments, subpasses, and dependencies, while allowing fine-grained control
//          * over render pass creation flags and resource management.
//          */
//         class Builder
//         {
//         public:
//             Builder(const U32 p_ImageCount) : m_ImageCount(p_ImageCount)
//             {
//             }

//             RenderPass Build() const;

//             AttachmentBuilder &BeginAttachment(VkImageUsageFlags flags);

//             SubpassBuilder &BeginSubpass(VkPipelineBindPoint p_BindPoint);
//             DependencyBuilder &BeginDependency(U32 srcSubpassIndex, U32 dstSubpassIndex);

//             Builder &SetFlags(VkRenderPassCreateFlags flags);
//             Builder &AddFlags(VkRenderPassCreateFlags flags);
//             Builder &RemoveFlags(VkRenderPassCreateFlags flags);

//         private:
//             VkRenderPassCreateFlags m_Flags = 0;
//             U32 m_ImageCount;

//             TArray16<AttachmentBuilder> m_Attachments;
//             TArray8<SubpassBuilder>     m_Subpasses;
//             TArray8<DependencyBuilder>  m_Dependencies;
//         };

//         /**
//          * @brief Manages frame buffers and image views associated with a render pass.
//          *
//          * Can be created with the `CreateResources()` method, which generates frame buffers and image views for each
//          * attachment. The user is responsible for destroying the resources when they are no longer needed.
//          *
//          */
//         class Resources
//         {
//         public:
//             void Destroy();

//             VkImageView GetImageView(const U32 p_ImageIndex, const U32 attachmentIndex) const
//             {
//                 const U32 attachmentCount = m_Images.size() / m_FrameBuffers.size();
//                 return m_Images[p_ImageIndex * attachmentCount + attachmentIndex].GetImageView();
//             }
//             VkFramebuffer GetFrameBuffer(const U32 p_ImageIndex) const
//             {
//                 return m_FrameBuffers[p_ImageIndex];
//             }

//         private:
//             TArray128<VyImage>     m_Images;       // size: m_ImageCount * m_Attachments.size()
//             TArray8<VkFramebuffer> m_FrameBuffers; // size: m_ImageCount

//             friend class RenderPass;
//         };

//         struct Info
//         {
//             VmaAllocator Allocator;
//             TArray16<Attachment> Attachments;
//             U32 ImageCount;
//         };

//         RenderPass() = default;
//         RenderPass(const VkRenderPass renderPass, const Info &p_Info)
//             : m_RenderPass(renderPass), m_Info(p_Info)
//         {
//         }

//         void Destroy();

//         /**
//          * @brief Creates resources for the render pass, including frame buffers and image data.
//          *
//          * Populates frame buffers and associated images based on the provided extent and a user-defined image creation
//          * callback. The `RenderPass` class provides many high-level options for `ImageData` struct creation, including the
//          * case where the underlying resource is directly provided by a `SwapChain` image. See the
//          * `ImageFactory::CreateImage()` methods for more.
//          *
//          * @tparam F The type of the callback function used for creating image data.
//          * @param p_Extent The dimensions of the frame buffer.
//          * @param p_CreateImageData A callback function that generates image data for each attachment. Takes the image index
//          * and attachment index as arguments.
//          * @param frameBufferLayers The number of layers for each frame buffer (default: 1).
//          * @return A `Result` containing the created `Resources` or an error.
//          */
//         template <typename F>
//         Resources CreateResources(const VkExtent2D &p_Extent, F &&p_CreateImageData, U32 frameBufferLayers = 1)
//         {
//             Resources resources;
//             resources.m_Device = m_Device;
//             // VKIT_CHECK_TABLE_FUNCTION_OR_RETURN(m_Device.Table, vkCreateFramebuffer, Result<Resources>);

//             TArray16<VkImageView> attachments{m_Info.Attachments.size(), VK_NULL_HANDLE};
//             for (U32 i = 0; i < m_Info.ImageCount; ++i)
//             {
//                 for (U32 j = 0; j < attachments.size(); ++j)
//                 {
//                     const auto imresult = std::forward<F>(p_CreateImageData)(i, j);
//                     if (!imresult)
//                     {
//                         resources.Destroy();
//                         return Result<Resources>::Error(imresult.GetError());
//                     }

//                     const Image &imageData = imresult.GetValue();
//                     resources.m_Images.Append(imageData);
//                     attachments[j] = imageData.GetImageView();
//                 }
//                 VkFramebufferCreateInfo frameBufferInfo{};
//                 frameBufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
//                 frameBufferInfo.renderPass = m_RenderPass;
//                 frameBufferInfo.attachmentCount = attachments.size();
//                 frameBufferInfo.pAttachments = attachments.data();
//                 frameBufferInfo.width = p_Extent.width;
//                 frameBufferInfo.height = p_Extent.height;
//                 frameBufferInfo.layers = frameBufferLayers;

//                 VkFramebuffer frameBuffer;
//                 const VkResult result = m_Device.Table->CreateFramebuffer(m_Device, &frameBufferInfo,
//                                                                         m_Device.AllocationCallbacks, &frameBuffer);
//                 if (result != VK_SUCCESS)
//                 {
//                     resources.Destroy();
//                     return Result<Resources>::Error(result, "Failed to create the frame buffer");
//                 }

//                 resources.m_FrameBuffers.Append(frameBuffer);
//             }

//             return Result<Resources>::Ok(resources);
//         }

//         const Attachment &GetAttachment(const U32 attachmentIndex) const
//         {
//             return m_Info.Attachments[attachmentIndex];
//         }
//         const Info &GetInfo() const
//         {
//             return m_Info;
//         }

//         VkRenderPass GetHandle() const
//         {
//             return m_RenderPass;
//         }
//         operator VkRenderPass() const
//         {
//             return m_RenderPass;
//         }
//         operator bool() const
//         {
//             return m_RenderPass != VK_NULL_HANDLE;
//         }

//     private:
//         VkRenderPass m_RenderPass = VK_NULL_HANDLE;
//         Info m_Info;
//     };
// }


namespace Vy
{
    // class RenderPassBuilder;

	// class RenderPass
	// {
	// public:
	// 	using Builder = RenderPassBuilder;

	// 	RenderPass() : render_pass(VK_NULL_HANDLE) {}

	// 	RenderPass(VkDevice device, const VkRenderPassCreateInfo& info, VkAllocationCallbacks* allocator = nullptr)
	// 	{
	// 		// VK_FUNC_INVOKE(vkCreateRenderPass, "Failed to create RenderPass object", device, &info, allocator, &render_pass);
	// 	}

	// 	RenderPass(const RenderPass&) = delete;
	// 	RenderPass& operator=(const RenderPass&) = delete;

	// 	RenderPass(RenderPass&& other) noexcept
	// 	{
	// 		render_pass = other.render_pass;
	// 		other.render_pass = VK_NULL_HANDLE;
	// 	}

	// 	RenderPass& operator=(RenderPass&& other) noexcept
	// 	{
	// 		render_pass = other.render_pass;
	// 		other.render_pass = VK_NULL_HANDLE;
	// 		return *this;
	// 	}

	// 	void begin(VkFramebuffer framebuffer, VkCommandBuffer commandbuffer, const VkRect2D& area, VkSubpassContents subpass_contents = VkSubpassContents::VK_SUBPASS_CONTENTS_INLINE)
	// 	{
	// 		VkRenderPassBeginInfo beginInfo{};
	// 		beginInfo.sType		= VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	// 		beginInfo.renderPass	= render_pass;
	// 		beginInfo.framebuffer	= framebuffer;
	// 		beginInfo.renderArea   = area;

	// 		vkCmdBeginRenderPass(commandbuffer, &beginInfo, subpass_contents);
	// 	}

	// 	void release(VkDevice device, VkAllocationCallbacks* allocator = nullptr)
	// 	{
	// 		if (render_pass != VK_NULL_HANDLE)
	// 		{
	// 			vkDestroyRenderPass(device, render_pass, allocator);
	// 			render_pass = VK_NULL_HANDLE;
	// 		}
	// 	}

	// 	inline operator VkRenderPass() const noexcept { return render_pass; }
	// 	inline const VkRenderPass* operator&() const noexcept { return &render_pass; }

	// 	inline VkRenderPass handle() const noexcept { return render_pass; }
	// 	inline const VkRenderPass* handlePtr() const noexcept { return &render_pass; }

	// private:
	// 	VkRenderPass render_pass{ VK_NULL_HANDLE };
	// };



	// class RenderPassBuilder
	// {
	// public:
	// 	RenderPassBuilder() 
    //     {
	// 		createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	// 	}

	// 	RenderPassBuilder& addAttachment(const VkAttachmentDescription& description)
	// 	{
	// 		attachments.push_back(description);
	// 		return *this;
	// 	}

	// 	RenderPassBuilder& addAttachment(VkAttachmentDescription&& description)
	// 	{
	// 		attachments.emplace_back(std::move(description));
	// 		return *this;
	// 	}

	// 	RenderPassBuilder& setAttachments(TVector<VkAttachmentDescription> descriptions)
	// 	{
	// 		attachments = std::move(descriptions);
	// 		return *this;
	// 	}

	// 	template<size_t N>
	// 	RenderPassBuilder& setAttachments(const VkAttachmentDescription(&descriptions)[N])
	// 	{
	// 		attachments.assign(descriptions, descriptions + N);
	// 		return *this;
	// 	}

	// 	template<size_t N>
	// 	RenderPassBuilder& setAttachments(const std::array<VkAttachmentDescription, N>& descriptions)
	// 	{
	// 		attachments.assign(descriptions.begin(), descriptions.end());
	// 		return *this;
	// 	}

	// 	RenderPassBuilder& addDependency(const VkSubpassDependency& dependency)
	// 	{
	// 		dependencies.push_back(dependency);
	// 		return *this;
	// 	}

	// 	RenderPassBuilder& addDependency(VkSubpassDependency&& dependency)
	// 	{
	// 		dependencies.push_back(std::move(dependency));
	// 		return *this;
	// 	}

	// 	RenderPassBuilder& setDependencies(TVector<VkSubpassDependency> dependencies)
	// 	{
	// 		this->dependencies = std::move(dependencies);
	// 		return *this;
	// 	}

	// 	template<size_t N>
	// 	RenderPassBuilder& setDependencies(const VkSubpassDependency(&dependencies)[N])
	// 	{
	// 		this->dependencies.assign(dependencies, dependencies + N);
	// 		return *this;
	// 	}

	// 	template<size_t N>
	// 	RenderPassBuilder& setDependencies(const std::array<VkSubpassDependency, N>& dependencies)
	// 	{
	// 		this->dependencies.assign(dependencies.begin(), dependencies.end());
	// 		return *this;
	// 	}

	// 	RenderPassBuilder& addSubpass(const VkSubpassDescription& subpasse)
	// 	{
	// 		subpasses.push_back(subpasse);
	// 		return *this;
	// 	}

	// 	RenderPassBuilder& addSubpass(VkSubpassDescription&& subpasse)
	// 	{
	// 		subpasses.push_back(std::move(subpasse));
	// 		return *this;
	// 	}

	// 	RenderPass build(VkDevice device, VkAllocationCallbacks* allocator = nullptr) const
	// 	{
	// 		createInfo.sType			= VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	// 		createInfo.attachmentCount = static_cast<U32>(attachments.size());
	// 		createInfo.pAttachments	= attachments.data();
	// 		createInfo.subpassCount	= static_cast<U32>(subpasses.size());
	// 		createInfo.pSubpasses		= subpasses.data();
	// 		createInfo.dependencyCount = static_cast<U32>(dependencies.size());
	// 		createInfo.pDependencies   = dependencies.data();

	// 		return RenderPass(device, createInfo, allocator);
	// 	}

	// private:
	// 	TVector<VkSubpassDescription>	 subpasses;
	// 	TVector<VkSubpassDependency>	 dependencies;
	// 	TVector<VkAttachmentDescription> attachments;
	// 	mutable VkRenderPassCreateInfo		createInfo{};
	// };


    // // struct Img
    // // {
    //     using VyImageFlags = U8;
        
    //     enum VyImageFlagBits : VyImageFlags
    //     {
    //         VY_IMAGE_NONE               = 0,
    //         VY_IMAGE_COLOR_ATTACHMENT   = 1 << 0,
    //         VY_IMAGE_DEPTH_ATTACHMENT   = 1 << 1,
    //         VY_IMAGE_STENCIL_ATTACHMENT = 1 << 2,
    //         VY_IMAGE_INPUT_ATTACHMENT   = 1 << 3,
    //         VY_IMAGE_SAMPLED            = 1 << 4,
    //     };
    // // };

    class VKRenderPass
    {
    public:
        class Builder;
        
        class Attachment 
        {
        public:
            Attachment(
                Builder*              pBuilder,
                VkFormat              format  = VK_FORMAT_UNDEFINED, 
                VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT
            ) :
                m_Builder{ pBuilder }
            {
                mDescription = {};
                {
                    mDescription.format 		= format;
                    mDescription.samples 		= samples;
                    mDescription.loadOp 		= VK_ATTACHMENT_LOAD_OP_CLEAR;
                    mDescription.storeOp 		= VK_ATTACHMENT_STORE_OP_STORE;
                    mDescription.stencilLoadOp 	= VK_ATTACHMENT_LOAD_OP_CLEAR;
                    mDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
                    mDescription.initialLayout 	= VK_IMAGE_LAYOUT_UNDEFINED;
                    mDescription.finalLayout 	= VK_IMAGE_LAYOUT_UNDEFINED;
                }

                {
                    mClearValue.color.float32[0]     = 0.0f;
                    mClearValue.color.float32[1]     = 0.0f;
                    mClearValue.color.float32[2]     = 0.0f;
                    mClearValue.color.float32[3]     = 0.0f;
                    mClearValue.depthStencil.depth   = 0.0f;
                    mClearValue.depthStencil.stencil = 0;
                }

                // Vec3
                if( vk::determineAspectMask( format ) & VK_IMAGE_ASPECT_COLOR_BIT ) 
                {
                    mDescription.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                }
                // Depth
                else if( vk::determineAspectMask( format ) & VK_IMAGE_ASPECT_DEPTH_BIT ) 
                {
                    mDescription.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                    mClearValue.depthStencil.depth = 1.0f;
                }
            }

            virtual ~Attachment() 
            {
            }

            static Attachment createColor(
                Builder*              pBuilder, 
                VkFormat              format, 
                VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT)
            {
                VKRenderPass::Attachment result = VKRenderPass::Attachment( pBuilder, format )
                    .setSamples( samples )
                    .setFinalLayout( VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL );
                return result;
            }

            static Attachment createDepthStencil(
                Builder*              pBuilder, 
                VkFormat              format, 
                VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT)
            {
                VKRenderPass::Attachment result = VKRenderPass::Attachment( pBuilder, format )
                    .setSamples( samples )
                    .setFinalLayout( VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL );
                return result;
            }

            const VkAttachmentDescription& getDescription() const { return mDescription; }

            Attachment&	        setClearValue(const VkClearValue& value) { mClearValue = value; return *this; }
            const VkClearValue&	getClearValue() const { return mClearValue; }

            Attachment&	setFormat(VkFormat value) { mDescription.format = value; return *this; }

            Attachment&	setSamples(VkSampleCountFlagBits value) { mDescription.samples = value; return *this; }

            Attachment&	setLoadOp(VkAttachmentLoadOp value) { mDescription.loadOp = value; return *this; }
            Attachment& setLoadOpLoad    () { return setLoadOp(VK_ATTACHMENT_LOAD_OP_LOAD); }
            Attachment& setLoadOpClear   () { return setLoadOp(VK_ATTACHMENT_LOAD_OP_CLEAR); }
            Attachment& setLoadOpDontCare() { return setLoadOp(VK_ATTACHMENT_LOAD_OP_DONT_CARE); }

            Attachment& setStoreOp(VkAttachmentStoreOp value) { mDescription.storeOp = value; return *this; }
            Attachment& setStoreOpStore   () { return setStoreOp(VK_ATTACHMENT_STORE_OP_STORE); }
            Attachment& setStoreOpDontCare() { return setStoreOp(VK_ATTACHMENT_STORE_OP_DONT_CARE); }

            Attachment& setStencilLoadOp(VkAttachmentLoadOp value) { mDescription.stencilLoadOp = value; return *this; }
            Attachment& setStencilLoadOpLoad    () { return setStencilLoadOp(VK_ATTACHMENT_LOAD_OP_LOAD); }
            Attachment& setStencilLoadOpClear   () { return setStencilLoadOp(VK_ATTACHMENT_LOAD_OP_CLEAR); }
            Attachment& setStencilLoadOpDontCare() { return setStencilLoadOp(VK_ATTACHMENT_LOAD_OP_DONT_CARE); }

            Attachment& setStencilStoreOp        (VkAttachmentStoreOp value) { mDescription.stencilStoreOp = value; return *this; }
            Attachment& setStencilStoreOpStore   () { return setStencilStoreOp(VK_ATTACHMENT_STORE_OP_STORE); }
            Attachment& setStencilStoreOpDontCare() { return setStencilStoreOp(VK_ATTACHMENT_STORE_OP_DONT_CARE); }

            // Initial Layout
            Attachment& setInitialLayout(VkImageLayout value) { mDescription.initialLayout = value; return *this; }
            Attachment& setInitialLayoutColorAttachment() { return setInitialLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL); }
            
            // Final Layout
            Attachment& setFinalLayout(VkImageLayout value) { mDescription.finalLayout = value; return *this; }
            Attachment& setFinalLayoutColorAttachment       () { return setFinalLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);   }
            Attachment& setFinalLayoutDepthStencilAttachment() { return setFinalLayout(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);   }
            Attachment& setFinalLayoutShaderReadOnly        () { return setFinalLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);   }
            Attachment& setFinalLayoutPresentSrc            () { return setFinalLayout(VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);            }
            Attachment& setFinalLayoutTransferSrc           () { return setFinalLayout(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);       }
            Attachment& setFinalLayoutTransferDst           () { return setFinalLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);       }

            Attachment& setLayouts(VkImageLayout initialLayout, VkImageLayout finalLayout) 
            { 
                setInitialLayout(initialLayout); 
                setFinalLayout(finalLayout); 
                return *this; 
            }

            Builder& endAttachment()
            {
                return *m_Builder;
            }

        private:
            Builder*                m_Builder;
            VkAttachmentDescription	mDescription;
            VkClearValue			mClearValue;

            friend class RenderPass;
        };


        class Subpass
        {
        public:
            Subpass(Builder* pBuilder, VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS)
                : m_Builder{ pBuilder }
            {
                m_Description.pipelineBindPoint     = bindPoint;
                m_DepthStencilAttachment.attachment = kMaxU32;
            }

            // const TVector<U32>& getColorAttachments()       const { return mColorAttachments; }
            // const TVector<U32>& getDepthStencilAttachment() const { return mDepthStencilAttachment; }


            // Subpass& addDepthStencilAttachment( U32 attachmentIndex );
            // Subpass& addPreserveAttachment( U32 attachmentIndex );
            // Subpass& addPreserveAttachments( const TVector<U32>& attachmentIndices);
        
            Subpass& addColorAttachment(
                U32           attachmentIndex,
                VkImageLayout layout       = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                U32           resolveIndex = kMaxU32)
            {
                m_ColorAttachments.push_back(VkAttachmentReference{ attachmentIndex, layout });

                if (resolveIndex != kMaxU32)
                {
                    m_ResolveAttachments.push_back(VkAttachmentReference{ resolveIndex, layout });

                    VY_ASSERT(m_ResolveAttachments.size() == m_ColorAttachments.size(),
                        "Mismatched color and resolve attachments");
                }

                return *this;
            }

            Subpass& addColorAttachment(
                U32 attachmentIndex, 
                U32 resolveIndex)
            {
                return addColorAttachment(attachmentIndex, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, resolveIndex);
            }

            Subpass& addInputAttachment(U32 attachmentIndex, VkImageLayout layout)
            {
                m_InputAttachments.push_back(VkAttachmentReference{ attachmentIndex, layout });
                return *this;
            }

            Subpass& addPreserveAttachment(const U32 attachmentIndex)
            {
                m_PreserveAttachments.push_back(attachmentIndex);
                return *this;
            }

            Subpass& setDepthStencilAttachment(
                const U32           attachmentIndex,
                const VkImageLayout layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
            {
                m_DepthStencilAttachment = VkAttachmentReference{ attachmentIndex, layout };
                return *this;
            }

            Subpass& setFlags(VkSubpassDescriptionFlags flags)
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
                
                m_Description.pResolveAttachments     = m_ResolveAttachments.empty() ? nullptr : m_ResolveAttachments.data();
                m_Description.pDepthStencilAttachment = m_DepthStencilAttachment.attachment == kMaxU32 ? nullptr : &m_DepthStencilAttachment;

                return *m_Builder;
            }

        private:
            Builder*                       m_Builder;
            VkSubpassDescription           m_Description{};
            TVector<VkAttachmentReference> m_ColorAttachments{};
            TVector<VkAttachmentReference> m_InputAttachments{};
            TVector<U32>                   m_PreserveAttachments{};
            TVector<VkAttachmentReference> m_ResolveAttachments{};
            VkAttachmentReference          m_DepthStencilAttachment{};

            // VkPipelineBindPoint		mPipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
            // TVector<U32>	mColorAttachments;			// 1 or more elements
            // TVector<U32>	mResolveAttachments;		// 1 or more elements
            // TVector<U32>	mDepthStencilAttachment;	// 1 element at most
            // TVector<U32>	mPreserveAttachments;		// 0 or more elements

            friend class Builder;
        };


        class Dependency
        {
        public:
            Dependency(Builder* pBuilder, U32 srcSubpassIndex, U32 dstSubpassIndex)
                : m_Builder{ pBuilder }
            {
                m_Dependency.srcSubpass = srcSubpassIndex;
                m_Dependency.dstSubpass = dstSubpassIndex;
            }

            Dependency& setSrcStageMask(VkPipelineStageFlags srcStageMask, bool exclusive = false)
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

            Dependency& setDstStageMask(VkPipelineStageFlags dstStageMask, bool exclusive = false)
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

            Dependency& setStageMasks(VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, bool exclusive = false)
            {
                setSrcStageMask(srcStageMask, exclusive);
                setDstStageMask(dstStageMask, exclusive);

                return *this;
            }

            Dependency& setSrcAccessMask(VkAccessFlags srcAccessMask, bool exclusive = false)
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

            Dependency& setDstAccessMask(VkAccessFlags dstAccessMask, bool exclusive = false)
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

            Dependency& setAccessMasks(VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask, bool exclusive = false)
            {
                setSrcAccessMask(srcAccessMask, exclusive);
                setDstAccessMask(dstAccessMask, exclusive);

                return *this;
            }

            Dependency& setFlags(VkDependencyFlags flags)
            {
                m_Dependency.dependencyFlags = flags;

                return *this;
            }

            Builder& endDependency()
            {
                return *m_Builder;
            }

        private:
            Builder*            m_Builder;
            VkSubpassDependency m_Dependency{};

            friend class Builder;
        };


        class Builder
        {
        public:
            Builder() = default; //: m_ImageCount(p_ImageCount)
            // {
            // }

            VKRenderPass build() const
            {
                if (m_Subpasses.empty())
                {
                    VY_THROW_RUNTIME_ERROR("RenderPass must have at least one subpass");
                }

                TVector<Attachment> attachments;
                TVector<VkAttachmentDescription> attDescriptions;

                
            }

            Attachment& beginAttachment(
                VkFormat              format  = VK_FORMAT_UNDEFINED, 
                VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT)
            {
                return m_Attachments.emplace_back( this, format, samples );
            }

            Subpass& beginSubpass(VkPipelineBindPoint bindPoint)
            {
                return m_Subpasses.emplace_back( this, bindPoint );
            }

            Dependency& beginDependency(U32 srcSubpassIndex, U32 dstSubpassIndex)
            {
                return m_Dependencies.emplace_back( this, srcSubpassIndex, dstSubpassIndex );
            }

            Builder &SetFlags(VkRenderPassCreateFlags flags);
            Builder &AddFlags(VkRenderPassCreateFlags flags);
            Builder &RemoveFlags(VkRenderPassCreateFlags flags);


        private:
            VkRenderPassCreateFlags m_Flags = 0;
            U32 m_ImageCount;

            TVector<Attachment> m_Attachments{};
            TVector<Subpass>    m_Subpasses{};
            TVector<Dependency> m_Dependencies{};
        };

    private:
        TVector<Attachment> m_Attachments;

    };
}



 //     struct Attachment
    //     {
    //         VkAttachmentDescription Description{};
    //         VyImageFlags            Flags;
    //     };

    // private:
    //     class AttachmentBuilder
    //     {
    //     public:
    //         AttachmentBuilder(Builder* pBuilder, const VyImageFlags flags)
    //             : m_Builder(pBuilder)
    //         {
    //             VY_ASSERT(flags, "Attachment must have at least one type flag");

    //             VY_ASSERT(!((flags & VY_IMAGE_COLOR_ATTACHMENT) && (flags & VY_IMAGE_DEPTH_ATTACHMENT)),
    //                 "Attachment must be color or depth, not both");

    //             VY_ASSERT(!((VY_IMAGE_COLOR_ATTACHMENT) && (flags & VY_IMAGE_STENCIL_ATTACHMENT)),
    //                 "Attachment must be color or stencil, not both");

    //             m_Attachment.Description.samples        = VK_SAMPLE_COUNT_1_BIT;
    //             m_Attachment.Description.loadOp         = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    //             m_Attachment.Description.storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    //             m_Attachment.Description.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    //             m_Attachment.Description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    //             m_Attachment.Description.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    //             m_Attachment.Description.finalLayout    = VK_IMAGE_LAYOUT_UNDEFINED;
    //             m_Attachment.Description.flags          = 0;
    //             m_Attachment.Flags                      = flags;

    //             if (flags & VY_IMAGE_COLOR_ATTACHMENT)
    //             {
    //                 m_Attachment.Description.loadOp  = VK_ATTACHMENT_LOAD_OP_CLEAR;
    //                 m_Attachment.Description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    //             }
    //             if (flags & VY_IMAGE_DEPTH_ATTACHMENT)
    //             {
    //                 m_Attachment.Description.loadOp  = VK_ATTACHMENT_LOAD_OP_CLEAR;
    //                 m_Attachment.Description.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    //             }
    //             if (flags & VY_IMAGE_STENCIL_ATTACHMENT)
    //             {
    //                 m_Attachment.Description.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_CLEAR;
    //                 m_Attachment.Description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    //             }

    //         }

    //         AttachmentBuilder &SetLoadOperation(VkAttachmentLoadOp p_Operation,
    //                                             VkAttachmentLoadOp stencilOperation = VK_ATTACHMENT_LOAD_OP_MAX_ENUM);
    //         AttachmentBuilder &SetStoreOperation(VkAttachmentStoreOp p_Operation,
    //                                             VkAttachmentStoreOp stencilOperation = VK_ATTACHMENT_STORE_OP_MAX_ENUM);

    //         AttachmentBuilder &SetStencilLoadOperation(VkAttachmentLoadOp p_Operation);
    //         AttachmentBuilder &SetStencilStoreOperation(VkAttachmentStoreOp p_Operation);

    //         AttachmentBuilder &RequestFormat(VkFormat format);
    //         AttachmentBuilder &AllowFormat(VkFormat format);

    //         AttachmentBuilder &SetLayouts(VkImageLayout p_InitialLayout, VkImageLayout finalLayout);
    //         AttachmentBuilder &SetInitialLayout(VkImageLayout layout);
    //         AttachmentBuilder &SetFinalLayout(VkImageLayout layout);

    //         AttachmentBuilder &SetSampleCount(VkSampleCountFlagBits sampleCount);
    //         AttachmentBuilder &SetFlags(VkAttachmentDescriptionFlags flags);

    //         Builder &EndAttachment();

    //     private:
    //         Builder*  m_Builder;
    //         Attachment m_Attachment{};
    //         TVector<VkFormat> m_Formats;

    //         friend class Builder;
    //     };