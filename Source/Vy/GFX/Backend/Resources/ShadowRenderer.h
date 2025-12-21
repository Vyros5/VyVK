#pragma once

#include <Vy/GFX/Backend/Image/Image.h>
#include <Vy/GFX/Backend/Image/ImageView.h>
#include <Vy/GFX/Backend/Image/Sampler.h>

#include <Vy/Core/Window.h>

#define SHADOW_HEIGHT 2048
#define SHADOW_WIDTH 2048

namespace Vy
{
    class VyShadowRenderer
    {
    public:
        VyShadowRenderer(
            VyWindow& window, 
            size_t    imageCount, 
            VkFormat  depthFormat
        );

        ~VyShadowRenderer();

        VkImage     currentImage()     const { return m_DepthImages[m_ImageIndex].handle();}
        VkImageView currentImageView() const { return m_DepthImageViews[m_ImageIndex].handle(); }
        VkSampler   imageSampler()           { return m_ImageSampler.handle(); }

        void startOffscreenRenderPass(VkCommandBuffer cmdBuffer);
        void endOffscreenRenderPass(VkCommandBuffer cmdBuffer);

        void createNextImage();

    private:

        void init(size_t imageCount);

        void createImageSampler();

        void createDepthResources(size_t imageCount);
 
        VyWindow& m_Window;

        TVector<VyImage>     m_DepthImages;
        TVector<VyImageView> m_DepthImageViews;

        VkFormat m_DepthFormat = VK_FORMAT_UNDEFINED;
        
        VySampler m_ImageSampler;

        U32 m_ImageIndex{ 0 };
    };
}