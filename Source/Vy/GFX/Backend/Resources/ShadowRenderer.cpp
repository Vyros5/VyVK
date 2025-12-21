#include <Vy/GFX/Backend/Resources/ShadowRenderer.h>

#include <Vy/GFX/Context.h>

namespace Vy
{
    VyShadowRenderer::VyShadowRenderer(
        VyWindow& window, 
        size_t    imageCount, 
        VkFormat  depthFormat
    ) :
        m_Window     { window      },
        m_DepthFormat{ depthFormat }
    {
        init(imageCount);
    }


    VyShadowRenderer::~VyShadowRenderer()
    {
        m_DepthImages    .clear();
        m_DepthImageViews.clear();
    }


    void VyShadowRenderer::init(size_t imageCount)
    {
        createImageSampler();
        createDepthResources(imageCount);
    }


    void VyShadowRenderer::createNextImage()
    {
        if (m_ImageIndex >= m_DepthImages.size() - 1)
        {
            m_ImageIndex = 0;
            return;
        }

        m_ImageIndex++;
    }


    void transitionImageLayout(
        VkCommandBuffer cmdBuffer,
        VkImage         image,
        VkFormat        format,
        VkImageLayout   oldLayout,
        VkImageLayout   newLayout)
    {
        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;

        VkPipelineStageFlags sourceStage;
        VkPipelineStageFlags destinationStage;

        if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
        {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        }
        else
        {
            throw std::invalid_argument("Unsupported layout transition!");
        }

        vkCmdPipelineBarrier(
            cmdBuffer,
            sourceStage, destinationStage,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier
        );
    }

    void VyShadowRenderer::createImageSampler()
    {
        m_ImageSampler = VySampler::Builder{}
            .filters         (VK_FILTER_LINEAR)
            .mipmapMode      (VK_SAMPLER_MIPMAP_MODE_NEAREST)
            .addressMode     (VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER)
            .borderColor     (VK_BORDER_COLOR_INT_OPAQUE_BLACK)
            .enableAnisotropy(false)
            .enableCompare   (VK_COMPARE_OP_LESS_OR_EQUAL)
            .lodRange        (0.0f, 0.0f)
            .mipLodBias      (0.0f)
        .build();
    }


    void VyShadowRenderer::createDepthResources(size_t imageCount)
    {
        m_DepthImages    .resize( imageCount );
        m_DepthImageViews.resize( imageCount );

        auto extent = VkExtent2D{ static_cast<U32>(SHADOW_WIDTH), static_cast<U32>(SHADOW_HEIGHT) };

        for (size_t i = 0; i < m_DepthImages.size(); i++)
        {
            m_DepthImages[i] = VyImage::Builder{}
                .imageType  (VK_IMAGE_TYPE_2D)
                .format     (m_DepthFormat)
                .extent     (extent)
                .mipLevels  (1)
                .arrayLayers(1)
                .tiling     (VK_IMAGE_TILING_OPTIMAL)
                .imageLayout(VK_IMAGE_LAYOUT_UNDEFINED)
                .sampleCount(VK_SAMPLE_COUNT_1_BIT)
                .sharingMode(VK_SHARING_MODE_EXCLUSIVE)
                .usage      (VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
                .memoryUsage(VMA_MEMORY_USAGE_GPU_ONLY)
            .build();

            m_DepthImageViews[i] = VyImageView::Builder{}
                .viewType   (VK_IMAGE_VIEW_TYPE_2D)
                .format     (m_DepthFormat)
                .aspectMask (VK_IMAGE_ASPECT_DEPTH_BIT)
                .mipLevels  (0, 1)
                .arrayLayers(0, 1)
            .build(m_DepthImages[i]);

            VkCommandBuffer cmdBuffer = VyContext::device().beginSingleTimeCommands();
            {
                transitionImageLayout(
                    cmdBuffer,
                    m_DepthImages[i],
                    m_DepthFormat,
                    VK_IMAGE_LAYOUT_UNDEFINED,
                    VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
                );
            }
            VyContext::device().endSingleTimeCommands(cmdBuffer);
        }
    }


    void VyShadowRenderer::startOffscreenRenderPass(VkCommandBuffer cmdBuffer)
    {
        auto extent = VkExtent2D{ static_cast<U32>(SHADOW_WIDTH), static_cast<U32>(SHADOW_HEIGHT) };

        VkRenderingAttachmentInfoKHR depthAttachment{ VKInit::renderingAttachmentInfoKHR() };
        {
            VkClearValue depthClear = {{ 1.0f, 0 }};

            depthAttachment.imageView   = m_DepthImageViews[m_ImageIndex];
            depthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            depthAttachment.loadOp      = VK_ATTACHMENT_LOAD_OP_CLEAR;
            depthAttachment.storeOp     = VK_ATTACHMENT_STORE_OP_STORE;
            depthAttachment.clearValue  = depthClear;
        }

        VkRenderingInfoKHR renderingInfo{ VKInit::renderingInfoKHR() };
        {
            renderingInfo.renderArea.offset = {0, 0};
            renderingInfo.renderArea.extent = extent;

            renderingInfo.layerCount           = 1;
            renderingInfo.colorAttachmentCount = 0;
            renderingInfo.pColorAttachments    = nullptr;
            renderingInfo.pDepthAttachment     = &depthAttachment;
        }

        vkCmdBeginRenderingKHR(cmdBuffer, &renderingInfo);

        VKCmd::viewport(cmdBuffer, extent);
        VKCmd::scissor (cmdBuffer, extent);
    }


    void VyShadowRenderer::endOffscreenRenderPass(VkCommandBuffer cmdBuffer)
    {
        vkCmdEndRenderingKHR(cmdBuffer);
    }
}