#include <Vy/GFX/Backend/Resources/OffscreenRenderer.h>

#include <Vy/GFX/Context.h>

namespace Vy
{
    VyOffscreenRenderer::VyOffscreenRenderer(
        VyWindow& window, 
        size_t    imageCount, 
        VkFormat  depthFormat, 
        VkFormat  colorFormat
    ) :
        m_Window     { window      },
        m_DepthFormat{ depthFormat },
        m_ColorFormat{ colorFormat }
    {
        init(imageCount);
    }


    VyOffscreenRenderer::~VyOffscreenRenderer()
    {
        m_Images         .clear();
        m_ImageViews     .clear();
        m_DepthImages    .clear();
        m_DepthImageViews.clear();
    }


    void VyOffscreenRenderer::init(size_t imageCount)
    {
        createImageSampler();
        createImages(imageCount);
        createImageViews();
        createDepthResources(imageCount);
    }


    void VyOffscreenRenderer::createNextImage()
    {
        if (m_ImageIndex >= m_Images.size() - 1)
        {
            VyContext::waitIdle();

            // m_Images    .clear();
            // m_ImageViews.clear();

            // for (size_t i = 0; i < m_Images.size(); ++i)
            // {
            //     if (m_ImageViews[i] != VK_NULL_HANDLE)
            //     {
            //         vkDestroyImageView(device.device(), m_ImageViews[i], nullptr);
            //         m_ImageViews[i] = VK_NULL_HANDLE;
            //     }

            //     if (m_Images[i] != VK_NULL_HANDLE)
            //     {
            //         vmaDestroyImage(device.getAllocator(), m_Images[i], imageAllocations[i]);
            //         m_Images[i] = VK_NULL_HANDLE;
            //         imageAllocations[i] = VK_NULL_HANDLE;
            //     }
            // }
            
            auto extent = m_Window.windowExtent();

            for (size_t i = 0; i < m_Images.size(); ++i)
            {
                m_Images[i] = VyImage::Builder{}
                    .imageType  (VK_IMAGE_TYPE_2D)
                    .format     (m_ColorFormat)
                    .extent     (extent)
                    .mipLevels  (1)
                    .arrayLayers(1)
                    .tiling     (VK_IMAGE_TILING_OPTIMAL)
                    .imageLayout(VK_IMAGE_LAYOUT_UNDEFINED)
                    .sampleCount(VK_SAMPLE_COUNT_1_BIT)
                    .sharingMode(VK_SHARING_MODE_EXCLUSIVE)
                    .usage      (VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
                    .memoryUsage(VMA_MEMORY_USAGE_GPU_ONLY)
                .build();

                m_ImageViews[i] = VyImageView::Builder{}
                    .viewType   (VK_IMAGE_VIEW_TYPE_2D)
                    .format     (m_ColorFormat)
                    .aspectMask (VK_IMAGE_ASPECT_COLOR_BIT)
                    .mipLevels  (0, 1)
                    .arrayLayers(0, 1)
                .build(m_Images[i]);
            }

            m_ImageIndex = 0;
            return;
        }

        m_ImageIndex++;
    }


    void VyOffscreenRenderer::createImageSampler()
    {
        m_ImageSampler = VySampler::Builder{}
            .filters         (VK_FILTER_LINEAR)
            .mipmapMode      (VK_SAMPLER_MIPMAP_MODE_LINEAR)
            .addressMode     (VK_SAMPLER_ADDRESS_MODE_REPEAT)
            .borderColor     (VK_BORDER_COLOR_INT_OPAQUE_BLACK)
            .enableAnisotropy(true)
            .lodRange        (0.0f, 0.0f)
            .mipLodBias      (0.0f)
        .build();
    }

    void VyOffscreenRenderer::createImages(size_t imageCount)
    {
        auto extent = m_Window.windowExtent();

        m_Images.resize( imageCount );

        for (size_t i = 0; i < m_Images.size(); ++i)
        {
            m_Images[i] = VyImage::Builder{}
                .imageType  (VK_IMAGE_TYPE_2D)
                .format     (m_ColorFormat)
                .extent     (extent)
                .mipLevels  (1)
                .arrayLayers(1)
                .tiling     (VK_IMAGE_TILING_OPTIMAL)
                .imageLayout(VK_IMAGE_LAYOUT_UNDEFINED)
                .sampleCount(VK_SAMPLE_COUNT_1_BIT)
                .sharingMode(VK_SHARING_MODE_EXCLUSIVE)
                .usage      (VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
                .memoryUsage(VMA_MEMORY_USAGE_GPU_ONLY)
            .build();
        }
    }

    void VyOffscreenRenderer::createImageViews()
    {
        m_ImageViews.resize( m_Images.size() );

        for (size_t i = 0; i < m_ImageViews.size(); i++)
        {
            m_ImageViews[i] = VyImageView::Builder{}
                .viewType   (VK_IMAGE_VIEW_TYPE_2D)
                .format     (m_ColorFormat)
                .aspectMask (VK_IMAGE_ASPECT_COLOR_BIT)
                .mipLevels  (0, 1)
                .arrayLayers(0, 1)
            .build(m_Images[i]);
        }
    }

    void VyOffscreenRenderer::createDepthResources(size_t imageCount)
    {
        m_DepthImages    .resize( imageCount );
        m_DepthImageViews.resize( imageCount );

        for (size_t i = 0; i < m_DepthImages.size(); i++)
        {
            m_DepthImages[i] = VyImage::Builder{}
                .imageType  (VK_IMAGE_TYPE_2D)
                .format     (m_DepthFormat)
                .extent     (VkExtent2D{ 2048, 2048 })
                .mipLevels  (1)
                .arrayLayers(1)
                .tiling     (VK_IMAGE_TILING_OPTIMAL)
                .imageLayout(VK_IMAGE_LAYOUT_UNDEFINED)
                .sampleCount(VK_SAMPLE_COUNT_1_BIT)
                .sharingMode(VK_SHARING_MODE_EXCLUSIVE)
                .usage      (VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
                .memoryUsage(VMA_MEMORY_USAGE_GPU_ONLY)
            .build();

            m_DepthImageViews[i] = VyImageView::Builder{}
                .viewType   (VK_IMAGE_VIEW_TYPE_2D)
                .format     (m_DepthFormat)
                .aspectMask (VK_IMAGE_ASPECT_DEPTH_BIT)
                .mipLevels  (0, 1)
                .arrayLayers(0, 1)
            .build(m_DepthImages[i]);
        }
    }

    void transitionImageLayout(VkCommandBuffer cmdBuffer, VkImage image)
    {
        VkImageMemoryBarrier barrier{ VKInit::imageMemoryBarrier() };
        {
            barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.image = image;
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            barrier.subresourceRange.baseMipLevel = 0;
            barrier.subresourceRange.levelCount = 1;
            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.layerCount = 1;
            
            barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT; 
            barrier.dstAccessMask = 0;
        }

        vkCmdPipelineBarrier(
            cmdBuffer,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, // Source stage
            VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,          // Destination stage
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier
        );
    }

    void VyOffscreenRenderer::startOffscreenRenderPass(VkCommandBuffer cmdBuffer)
    {
        auto extent = m_Window.windowExtent();

        VkRenderingAttachmentInfoKHR colorAttachment{ VKInit::renderingAttachmentInfoKHR() };
        {
            VkClearValue colorClear = {{{ 0.01f, 0.0f, 0.0f, 1.0f }}};

            colorAttachment.imageView   = m_ImageViews[m_ImageIndex];
            colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            colorAttachment.loadOp      = VK_ATTACHMENT_LOAD_OP_CLEAR;
            colorAttachment.storeOp     = VK_ATTACHMENT_STORE_OP_STORE;
            colorAttachment.clearValue  = colorClear;
        }

        VkRenderingAttachmentInfoKHR depthAttachment{ VKInit::renderingAttachmentInfoKHR() };
        {
            VkClearValue depthClear = {{ 1.0f, 0 }};

            depthAttachment.imageView   = m_DepthImageViews[m_ImageIndex];
            depthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            depthAttachment.loadOp      = VK_ATTACHMENT_LOAD_OP_CLEAR;
            depthAttachment.storeOp     = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            depthAttachment.clearValue  = depthClear;
        }

        VkRenderingInfoKHR renderingInfo{ VKInit::renderingInfoKHR() };
        {
            renderingInfo.renderArea.offset = {0, 0};
            renderingInfo.renderArea.extent = extent;

            renderingInfo.layerCount           = 1;
            renderingInfo.colorAttachmentCount = 1;
            renderingInfo.pColorAttachments    = &colorAttachment;
            renderingInfo.pDepthAttachment     = &depthAttachment;
        }

        vkCmdBeginRenderingKHR(cmdBuffer, &renderingInfo);

        VKCmd::viewport(cmdBuffer, extent);
        VKCmd::scissor (cmdBuffer, extent);
    }


    void VyOffscreenRenderer::endOffscreenRenderPass(VkCommandBuffer cmdBuffer)
    {
        vkCmdEndRenderingKHR(cmdBuffer);

        transitionImageLayout(cmdBuffer, currentImage());
    }
}