#pragma once

#include <Vy/GFX/Backend/Resources/Image.h>
#include <Vy/GFX/Backend/Resources/ImageView.h>
#include <Vy/GFX/Backend/Resources/Sampler.h>

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
		VyRenderPass(const VyRenderPassDesc& desc);

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