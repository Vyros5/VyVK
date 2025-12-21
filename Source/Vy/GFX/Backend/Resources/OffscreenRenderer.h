#pragma once

#include <Vy/GFX/Backend/Image/Image.h>
#include <Vy/GFX/Backend/Image/ImageView.h>
#include <Vy/GFX/Backend/Image/Sampler.h>

#include <Vy/Core/Window.h>

namespace Vy
{
    class VyOffscreenRenderer
    {
    public:
        VyOffscreenRenderer(
            VyWindow& window, 
            size_t    imageCount, 
            VkFormat  depthFormat, 
            VkFormat  colorFormat
        );

        ~VyOffscreenRenderer();

        VkImage     currentImage()     const { return m_Images[m_ImageIndex].handle();}
        VkImageView currentImageView() const { return m_ImageViews[m_ImageIndex].handle(); }
        VkSampler   imageSampler()           { return m_ImageSampler.handle(); }

        void startOffscreenRenderPass(VkCommandBuffer cmdBuffer);
        void endOffscreenRenderPass(VkCommandBuffer cmdBuffer);

        void createNextImage();

    private:

        void init(size_t imageCount);

        void createImageSampler();

        void createImages(size_t imageCount);
        void createImageViews();
        void createDepthResources(size_t imageCount);
 
        VyWindow& m_Window;

        TVector<VyImage>     m_Images;
        TVector<VyImageView> m_ImageViews;

        TVector<VyImage>     m_DepthImages;
        TVector<VyImageView> m_DepthImageViews;

        VkFormat m_ColorFormat = VK_FORMAT_UNDEFINED;
        VkFormat m_DepthFormat = VK_FORMAT_UNDEFINED;
        
        VySampler m_ImageSampler;

        U32 m_ImageIndex{ 0 };
    };
}