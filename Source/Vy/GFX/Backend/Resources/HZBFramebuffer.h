

#pragma once

#include <Vy/GFX/Backend/Image/Image.h>
#include <Vy/GFX/Backend/Image/ImageView.h>
#include <Vy/GFX/Backend/Image/Sampler.h>

#include <Vy/GFX/Context.h>

namespace Vy
{
    class VyHZBFramebuffer
    {
    public:
        struct Attachment
        {
            VkFormat          Format;
            VkImageUsageFlags Usage;
            VkImageLayout     FinalLayout;
        };

        VyHZBFramebuffer(VkExtent2D extent, U32 frameCount, bool useMipmaps = false);

        ~VyHZBFramebuffer();

        VyHZBFramebuffer(const VyHZBFramebuffer&)            = delete;
        VyHZBFramebuffer& operator=(const VyHZBFramebuffer&) = delete;

        void resize(VkExtent2D newExtent);

        VkRenderPass          renderPass() const { return m_RenderPass; }
        VkDescriptorImageInfo descriptorImageInfo(int index) const;

        void beginRenderPass(VkCommandBuffer cmdBuffer, int frameIndex);
        void endRenderPass(VkCommandBuffer cmdBuffer) const;

        void generateMipmaps(VkCommandBuffer cmdBuffer, int frameIndex);

        float aspectRatio() const 
        { 
            return static_cast<float>(m_Extent.width) / static_cast<float>(m_Extent.height); 
        }

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

        void destroy();

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
}