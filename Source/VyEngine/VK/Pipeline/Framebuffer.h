#pragma once

#include <VyEngine/VK/Device/Device.h>

#include <VyEngine/VK/Image/Image.h>
#include <VyEngine/VK/Image/ImageView.h>
#include <VyEngine/VK/Image/Sampler.h>

namespace Vy
{
    // class VyTexture;

	// struct RenderPassAttachment 
    // {
	// 	Unique<VyTexture>&  ImageAttachment;
	// 	VkAttachmentLoadOp  LoadOp      = VK_ATTACHMENT_LOAD_OP_LOAD;
	// 	VkAttachmentStoreOp StoreOp     = VK_ATTACHMENT_STORE_OP_STORE;
	// 	VkImageLayout       FinalLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	// };

	// struct SubpassInfo 
    // {
	// 	TVector<U32> renderTargets{};
	// 	TVector<U32> subpassInputs{};
	// };

	// struct RenderPassInfo {
	// 	const TVector<RenderPassAttachment>& attachments;
	// 	const TVector<SubpassInfo>& subpassInfos;
	// };

	// struct Viewport {
	// 	float offsetX = 0.0f;
	// 	float offsetY = 0.0f;
	// 	float width = 0.0f;
	// 	float height = 0.0f;
	// 	float minDepth = 0.0f;
	// 	float maxDepth = 1.0f;
	// };

	// class RenderPass {
	// public:
	// 	RenderPass(const RenderPassInfo& createInfo);
	// 	~RenderPass();

	// 	void begin(Framebuffer& frameBuffer, VkCommandBuffer commandBuffer, const Viewport& viewport);
	// 	void end(VkCommandBuffer commandBuffer);

	// 	inline VkRenderPass getVulkanRenderPass() const { return m_RenderPass; }
		
	// 	void setClearColor(U32 attachmentIndex, Color color);

	// private:
	// 	VkRenderPass m_RenderPass = VK_NULL_HANDLE;
	// 	TVector<VkClearValue> m_ClearValues;

	// 	struct SubpassDescription {
	// 		VkAttachmentReference depthReference{};
	// 		TVector<VkAttachmentReference> colorReferences{};
	// 		TVector<VkAttachmentReference> resolveReferences{};
	// 		TVector<VkAttachmentReference> inputReferences{};
	// 		VkSubpassDescription data{};
	// 	};

	// 	U32 m_ColorAttachmentCount = 0;
	// 	bool m_HasDepthAttachment = false;
	// };


    // // class Framebuffer
    // // {
    // // public:
    // //     struct Attachment
    // //     {
    // //         VkFormat          Format;
    // //         VkImageUsageFlags Usage;
    // //         VkImageLayout     FinalLayout;
    // //     };

    // //     Framebuffer(VkExtent2D extent, U32 frameCount, bool bUseMipmaps = false);

    // //     ~Framebuffer();

    // //     Framebuffer(const Framebuffer&)            = delete;
    // //     Framebuffer& operator=(const Framebuffer&) = delete;

    // //     void resize(VkExtent2D newExtent);

    // //     VkRenderPass          renderPass() const { return m_RenderPass; }
    // //     VkDescriptorImageInfo descriptorImageInfo(int index) const;

    // //     void beginRenderPass(VkCommandBuffer cmdBuffer, int frameIndex);
    // //     void endRenderPass  (VkCommandBuffer cmdBuffer) const;
    // //     void generateMipmaps(VkCommandBuffer cmdBuffer, int frameIndex);

    // //     float aspectRatio() const 
    // //     { 
    // //         return 
    // //             static_cast<float>(m_Extent.width ) / 
    // //             static_cast<float>(m_Extent.height); 
    // //     }

    // //     VkImageView depthImageView   (int frameIndex) const { return m_DepthImageViews[frameIndex].handle(); }
    // //     VkImage     depthImage       (int frameIndex) const { return m_DepthImages[frameIndex].handle(); }
    // //     VkSampler   depthSampler()                    const { return m_DepthSampler.handle(); }

    // // private:
    // //     void createRenderPass();
    // //     void createImages();
    // //     void createFramebuffers();
    // //     void cleanup();

    // //     VkExtent2D m_Extent;

    // //     VkRenderPass m_RenderPass{VK_NULL_HANDLE};

    // //     // Color attachment
    // //     TVector<VyImage>        m_ColorImages;
    // //     TVector<VyImageView>    m_ColorImageViews;
    // //     TVector<VyImageView>    m_ColorAttachmentImageViews;

    // //     // Depth attachment
    // //     TVector<VyImage>              m_DepthImages;
    // //     TVector<VyImageView>          m_DepthImageViews;

    // //     TVector<VkFramebuffer>     m_Framebuffers;

    // //     VySampler                  m_ColorSampler;
    // //     VySampler                  m_DepthSampler;
    // // };


    // class VyFramebuffer
    // {
    // public:
    //     struct Attachment
    //     {
    //         VkFormat          Format;
    //         VkImageUsageFlags Usage;
    //         VkImageLayout     FinalLayout;
    //     };

    //     VyFramebuffer(VkExtent2D extent, U32 frameCount, bool bUseMipmaps = false);

    //     ~VyFramebuffer();

    //     VyFramebuffer(const VyFramebuffer&)            = delete;
    //     VyFramebuffer& operator=(const VyFramebuffer&) = delete;

    //     void resize(VkExtent2D newExtent);

    //     VkRenderPass          renderPass() const { return m_RenderPass; }
    //     VkDescriptorImageInfo descriptorImageInfo(int index) const;

    //     void beginRenderPass(VkCommandBuffer cmdBuffer, int frameIndex);
    //     void endRenderPass  (VkCommandBuffer cmdBuffer) const;
    //     void generateMipmaps(VkCommandBuffer cmdBuffer, int frameIndex);

    //     float aspectRatio() const 
    //     { 
    //         return 
    //             static_cast<float>(m_Extent.width ) / 
    //             static_cast<float>(m_Extent.height); 
    //     }

    //     // Accessors for HZB
    //     VkImageView depthMipImageView(int frameIndex, int mipLevel) const { return m_DepthMipImageViews[frameIndex][mipLevel].handle(); }
    //     VkImageView depthImageView   (int frameIndex)               const { return m_DepthImageViews[frameIndex].handle(); }
    //     VkImage     depthImage       (int frameIndex)               const { return m_DepthImages[frameIndex].handle(); }
    //     VkSampler   depthSampler()                                  const { return m_DepthSampler.handle(); }

    //     // Accessors for HZB Texture (R32_SFLOAT)
    //     VkImageView hzbMipImageView(int frameIndex, int mipLevel) const { return m_HZBMipImageViews[frameIndex][mipLevel].handle(); }
    //     VkImageView hzbImageView   (int frameIndex)               const { return m_HZBImageViews[frameIndex].handle(); }
    //     VkImage     hzbImage       (int frameIndex)               const { return m_HZBImages[frameIndex].handle(); }
    //     VkSampler   hzbSampler()                                  const { return m_HZBSampler.handle(); }

    // private:
    //     void createRenderPass();
    //     void createImages();
    //     void createFramebuffers();
    //     void cleanup();

    //     VkExtent2D m_Extent;
    //     U32        m_FrameCount;
    //     bool       m_UseMipmaps;
    //     U32        m_MipLevels{1};

    //     VkRenderPass m_RenderPass{VK_NULL_HANDLE};

    //     // Color attachment
    //     TVector<VyImage>        m_ColorImages;
    //     TVector<VyImageView>    m_ColorImageViews;
    //     TVector<VyImageView>    m_ColorAttachmentImageViews;

    //     // Depth attachment
    //     TVector<VyImage>              m_DepthImages;
    //     TVector<VyImageView>          m_DepthImageViews;
    //     // Per-mip views for depth (for HZB generation)
    //     // Outer vector: frame index, Inner vector: mip level
    //     TVector<TVector<VyImageView>> m_DepthMipImageViews;

    //     // HZB attachment (R32_SFLOAT)
    //     TVector<VyImage>              m_HZBImages;
    //     TVector<VyImageView>          m_HZBImageViews;
    //     TVector<TVector<VyImageView>> m_HZBMipImageViews;

    //     TVector<VkFramebuffer>     m_Framebuffers;
    //     VySampler                  m_Sampler;
    //     VySampler                  m_DepthSampler;
    //     VySampler                  m_HZBSampler;
    // };








    // class VyTexture2D {
    // public:
    //     class Builder {
    //     public:
    //         explicit Builder() {}

    //         Builder& setSize(U32 width, U32 height) 
    //         {
    //             m_Width  = width;
    //             m_Height = height;
    //             return *this;
    //         }

    //         Builder& setFormat(VkFormat format) 
    //         {
    //             m_Format = format;
    //             return *this;
    //         }

    //         Builder& setUsage(VkImageUsageFlags usage) 
    //         {
    //             m_Usage = usage;
    //             return *this;
    //         }

    //         Builder& setSamples(VkSampleCountFlagBits samples) 
    //         {
    //             m_Samples = samples;
    //             return *this;
    //         }

    //         Builder& setFilepath(const TString& filepath) 
    //         {
    //             m_Filepath = filepath;
    //             return *this;
    //         }

    //         Builder& asCubemap() 
    //         {
    //             m_bCubemap = true;
    //             return *this;
    //         }

    //         Builder& asHDR()
    //         {
    //             m_bHDR = true;
    //             return *this;
    //         }

    //         [[nodiscard]] Shared<VyTexture2D> build() const 
    //         {
    //             if (m_bCubemap && m_bHDR) 
    //             {
    //                 throw std::runtime_error("Cannot build a texture as both HDR and Cubemap. Choose only one.");
    //             }

    //             auto pTexture = MakeShared<VyTexture2D>();

    //             if (!m_Filepath.empty()) 
    //             {
    //                 pTexture->createTexture(m_Filepath, m_Format);
    //             } 
    //             else {
    //                 if (m_bCubemap) 
    //                 {
    //                     pTexture->createCubeMap(m_Width, m_Height, m_Format, m_Usage);
    //                 } 
    //                 else if (m_bHDR) 
    //                 {
    //                     if (!m_Filepath.empty())
    //                     {
    //                         pTexture->createHDRTexture(m_Filepath);
    //                     } 
    //                     else {
    //                         throw std::runtime_error("HDR texture requires a filepath.");
    //                     }
    //                 } 
    //                 else {
    //                     pTexture->createTexture(m_Width, m_Height, m_Format, m_Usage, m_Samples);
    //                 }
    //             }

    //             return pTexture;
    //         }

    //     private:
    //         bool                  m_bCubemap = false;
    //         bool                  m_bHDR     = false;
    //         U32                   m_Width    = 512;
    //         U32                   m_Height   = 512;
    //         VkFormat              m_Format   = VK_FORMAT_R8G8B8A8_UNORM;
    //         VkImageUsageFlags     m_Usage    = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    //         VkSampleCountFlagBits m_Samples  = VK_SAMPLE_COUNT_1_BIT;
    //         TString               m_Filepath;
    //     };

    //     enum class ETextureType 
    //     {
    //         Albedo,
    //         Normal,
    //         Emissive,
    //         AO,
    //         MetallicRoughness,
    //         Cubemap
    //     };

    //     public:

    //     [[nodiscard]] static Shared<VyTexture2D> create() 
    //     {
    //         return MakeShared<VyTexture2D>();
    //     }

    //     void GenerateMipmap(VkImage image, VkFormat format, int32_t width, int32_t height, U32 mipLevels);

    //     void createHDRTexture(const TString& filepath, VkFormat format = VK_FORMAT_R32G32B32A32_SFLOAT);
    //     void createCubeMap(U32 width, U32 height, VkFormat format, VkImageUsageFlags usage);
    //     // void createMipMappedCubemap(U32 width, U32 height, VkFormat format, VkImageUsageFlags usage);
    //     void createTexture(const TString& filepath, VkFormat format = VK_FORMAT_R8G8B8A8_UNORM);
    //     void createTexture(U32 width, U32 height, VkFormat format = VK_FORMAT_R8G8B8A8_UNORM,
    //                        VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
    //                        VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT);
    //     void useFallbackTextures(ETextureType type);

    //     // Inline Getters
    //     [[nodiscard]] VkImage         imageHandle()   const { return m_Image.handle(); }
    //     [[nodiscard]] const VyImage&  image()         const { return m_Image; }
    //     [[nodiscard]] VkImageView     view()          const { return m_View .handle(); }
    //     [[nodiscard]] VkSampler       sampler()       const { return m_Sampler.handle(); }
    //     [[nodiscard]] VkDescriptorSet descriptorSet() const { return m_DescriptorSet; }
    //     [[nodiscard]] U32             mipLevels()     const { return m_MipLevels; }
    //     [[nodiscard]] VkFormat        format()        const { return m_Format; }

    //     [[nodiscard]] bool hasImage()   const { return m_Image.valid(); }
    //     [[nodiscard]] bool hasView()    const { return m_View.valid(); }
    //     [[nodiscard]] bool hasSampler() const { return m_Sampler.valid(); }

    //     [[nodiscard]] VkDescriptorImageInfo descriptorImageInfo() const;

    //     void destroyAll();
    //     void destroy();

    //     VyTexture2D();
    //     ~VyTexture2D();

    //     VyTexture2D(const VyTexture2D&)                = delete;
    //     VyTexture2D& operator=(const VyTexture2D&)     = delete;
    //     VyTexture2D(VyTexture2D&&) noexcept            = default;
    //     VyTexture2D& operator=(VyTexture2D&&) noexcept = delete;

    // private:
    //     U32      m_MipLevels = 1;
    //     VkFormat m_Format    = VK_FORMAT_R8G8B8A8_UNORM;

    //     VyImage        m_Image;
    //     VyImageView    m_View;
    //     VySampler      m_Sampler;

    //     Unique<VyDescriptorPool>      m_DescriptorPool;
    //     Unique<VyDescriptorSetLayout> m_DescriptorSetLayout;
    //     VkDescriptorSet               m_DescriptorSet = VK_NULL_HANDLE;

    //     // HDR
    //     void createHDRTextureImage(const TString& filepath);
    //     void createHDRTextureImageView(VkFormat format);
    //     void createHDRTextureSampler();

    //     // Cubemap
    //     void createCubemapImage(U32 width, U32 height, VkFormat format, VkImageUsageFlags usage);
    //     void createCubemapImageView(VkFormat format);
    //     void createCubemapSampler();
    //     void createSolidColorCubemap(Vec4 color);

    //     // LDR
    //     void createTextureImage(const TString& filepath);
    //     void createTextureImageView(VkFormat format);
    //     void createTextureSampler();
    //     // void copyBufferToImage(VkBuffer buffer, VkImage image, U32 width, U32 height) const;
    //     void writeToDescriptorSet();
    //     void createSolidColorTexture(Vec4 color);

    //     // void transitionImageLayout(VkImage image, VkFormat format,
    //     //                            VkImageLayout oldLayout, VkImageLayout newLayout,
    //     //                            U32 layers = 1);
    // };


    // class RenderTarget 
    // {
    // public:
    //     Shared<VyTexture2D> ColorTexture{};  // Shared pointer to color texture
    //     Shared<VyTexture2D> DepthTexture{};  // Shared pointer to depth texture

    //     enum class AttachmentType 
    //     {
    //         COLOR,
    //         DEPTH,
    //         BOTH
    //     };

    //     enum class TargetType 
    //     {
    //         TEXTURE_2D,
    //         CUBEMAP
    //     };

    //     RenderTarget(
    //         U32               width, 
    //         U32               height,
    //         VkFormat          colorFormat, 
    //         VkImageUsageFlags colorUsage,
    //         VkFormat          depthFormat,
    //         VkImageUsageFlags depthUsage,
    //         AttachmentType   attachmentType = AttachmentType::BOTH,
    //         TargetType       targetType     = TargetType::TEXTURE_2D
    //     ) : 
    //         Target( targetType ) 
    //     {
    //         if (attachmentType == AttachmentType::COLOR || 
    //             attachmentType == AttachmentType::BOTH) 
    //         {
    //             ColorTexture = createColorTexture(targetType, width, height, colorFormat, colorUsage);
    //         }

    //         if (attachmentType == AttachmentType::DEPTH || 
    //             attachmentType == AttachmentType::BOTH) 
    //         {
    //             DepthTexture = createDepthTexture(width, height, depthFormat, depthUsage);
    //         }
    //     }

    //     static Shared<VyTexture2D> createColorTexture(
    //         TargetType           target,
    //         U32                   width, 
    //         U32                   height,
    //         VkFormat              format, 
    //         VkImageUsageFlags     usage, 
    //         VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT)
    //     {
    //         Shared<VyTexture2D> pTexture;
    //         if (target == TargetType::CUBEMAP) 
    //         {
    //             pTexture = VyTexture2D::Builder()
    //                 .setSize(width, height)
    //                 .setFormat(format)
    //                 .setUsage(usage | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
    //                 .asCubemap()
    //                 .build();
    //         } 
    //         else {
    //             pTexture = VyTexture2D::Builder()
    //                 .setSize(width, height)
    //                 .setFormat(format)
    //                 .setUsage(usage)
    //                 .setSamples(samples)
    //                 .build();
    //         }
    //         return pTexture;
    //     }

    //     static Shared<VyTexture2D> createDepthTexture(U32 width, U32 height, VkFormat format, VkImageUsageFlags usage)
    //     {
    //         Shared<VyTexture2D> pTexture;

    //         pTexture = VyTexture2D::Builder()
    //             .setSize(width, height)
    //             .setFormat(format)
    //             .setUsage(usage)
    //             .build();

    //         return pTexture;
    //     }

    // private:
    //     TargetType Target;
    // };


    // class Framebuffer {
    // public:
    //     enum class Target 
    //     {
    //         Offscreen,
    //         Swapchain
    //     };

    //     struct FramebufferDetails 
    //     {
    //         U32 Width;
    //         U32 Height;
    //         Framebuffer::Target Target{ Framebuffer::Target::Offscreen };
    //         U32 Layers{1};
    //         TVector<VkImageView> Attachments;
    //         VkRenderPass RenderPass;
    //     };

    //     Framebuffer(FramebufferDetails details);
    //     ~Framebuffer();

    //     // AQUILA_NONCOPYABLE(Framebuffer);
    //     // AQUILA_NONMOVEABLE(Framebuffer);

    //     static Shared<Framebuffer> create(FramebufferDetails details) 
    //     {
    //         return MakeShared<Framebuffer>(details);
    //     }

    //     void resize(const VkExtent2D& newExtent);

    //     void destroy() 
    //     {
    //         if (m_Framebuffer != VK_NULL_HANDLE) 
    //         {
    //             vkDestroyFramebuffer(VyContext::device(), m_Framebuffer, nullptr);
    //             m_Framebuffer = VK_NULL_HANDLE;
    //         }
    //     }

    //     [[nodiscard]] VkFramebuffer     handle() const { return m_Framebuffer; }
    //     [[nodiscard]] const VkExtent2D& extent() const { return m_Extent; }
    //     [[nodiscard]] const Target      target() const { return m_Target; }

    // private:
    //     VkFramebuffer m_Framebuffer;
    //     VkExtent2D m_Extent;

    //     VkRenderPass m_RenderPass;
    //     TVector<VkImageView> m_Attachments;

    //     Target m_Target;
    // };


    // class Renderpass 
    // {
    // public:
    //     Renderpass(const VkExtent2D& extent, const Shared<VyDescriptorSetLayout>& descriptorSetLayout)
    //     : m_Extent(extent),
    //     m_DescriptorSetLayout(descriptorSetLayout)
    //     {
    //         // Engine::DescriptorAllocator::Allocate(descriptorSetLayout->GetDescriptorSetLayout(),  m_DescriptorSet);
    //     }

    //     virtual ~Renderpass() = 0;

    //     [[nodiscard]] Shared<VyDescriptorSetLayout>& descriptorSetLayout() { return m_DescriptorSetLayout; }

    //     [[nodiscard]] VkImageView             targetColorImage()     const { return m_ColorAttachment->view(); }
    //     [[nodiscard]] VkImageView             targetDepthImage()     const { return m_DepthAttachment->view(); }
    //     [[nodiscard]] VkRenderPass            renderPass()                 const { return m_RenderPass; }
    //     [[nodiscard]] VkDescriptorSet&        descriptorSet()                    { return m_DescriptorSet; }
    //     [[nodiscard]] VkDescriptorImageInfo   descriptorImageInfo(VkImageLayout)     const;
    //     [[nodiscard]] TArray<VkClearValue, 2> clearValues()              const { return m_ClearValues; }
    //     [[nodiscard]] VkExtent2D              extent()                     const { return m_Extent; }
    //     [[nodiscard]] VkImage                 finalImage()                 const { return m_ColorAttachment->imageHandle(); }
    //     [[nodiscard]] Shared<Framebuffer>   framebuffers(int index = 0)   const { return m_Framebuffers.at(index); }

    // protected:
    //     // create render target
    //     virtual bool createRenderTarget() = 0;
    //     // create render pass
    //     virtual bool createRenderPass() = 0;
    //     // create framebuffer
    //     virtual bool createFramebuffer() = 0;

    //     virtual void createClearValues() = 0;

    //     void writeToDescriptorSet();

    //     // Members
    //     Shared<VyTexture2D> m_ColorAttachment;
    //     Shared<VyTexture2D> m_DepthAttachment;

    //     // Descriptor sets to write images to. They will be sampled in the shader (shadow, post-processing, etc.)
    //     VkDescriptorSet               m_DescriptorSet{};
    //     Shared<VyDescriptorSetLayout> m_DescriptorSetLayout{};

    //     // Vulkan stuff
    //     VkRenderPass m_RenderPass{};
    //     VkExtent2D m_Extent{};
    //     std::array<VkClearValue, 2> m_ClearValues{};

    //     TVector<Shared<Framebuffer>> m_Framebuffers;
    // };




}