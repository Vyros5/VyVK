#include <Vy/GFX/Backend/Resources/Framebuffer.h>

#include <Vy/GFX/Context.h>
#include <iostream>

namespace Vy
{
    VyFramebuffer2::VyFramebuffer2(VkExtent2D extent, U32 frameCount, bool useMipmaps) : 
        m_Extent    { extent     }, 
        m_FrameCount{ frameCount }, 
        m_UseMipmaps{ useMipmaps }
    {
        createRenderPass();
        createImages();
        createFramebuffers();
    }


    VyFramebuffer2::~VyFramebuffer2()
    {
        cleanup();
        vkDestroyRenderPass(VyContext::device(), m_RenderPass, nullptr);
    }


    void VyFramebuffer2::cleanup()
    {
        for (auto framebuffer : m_Framebuffers)
        {
            vkDestroyFramebuffer(VyContext::device(), framebuffer, nullptr);
        }

        m_DepthMipImageViews.clear();

    }


    void VyFramebuffer2::resize(VkExtent2D newExtent)
    {
        m_Extent = newExtent;

        cleanup();
        createImages();
        createFramebuffers();
    }


    void VyFramebuffer2::createRenderPass()
    {
        VkAttachmentDescription colorAttachment{};
        {
            colorAttachment.format         = VK_FORMAT_R16G16B16A16_SFLOAT;
            colorAttachment.samples        = VK_SAMPLE_COUNT_1_BIT;
            colorAttachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
            colorAttachment.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
            colorAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            colorAttachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
            
            if (m_UseMipmaps)
            {
                colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            }
            else
            {
                colorAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            }
        }

        VkAttachmentReference colorAttachmentRef{};
        {
            colorAttachmentRef.attachment = 0;
            colorAttachmentRef.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        }

        VkAttachmentDescription depthAttachment{};
        {
            depthAttachment.format         = VyContext::device().findSupportedFormat(
                {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
                VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
            );
            depthAttachment.samples        = VK_SAMPLE_COUNT_1_BIT;
            depthAttachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
            depthAttachment.storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            depthAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            depthAttachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
            depthAttachment.finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        }

        VkAttachmentReference depthAttachmentRef{};
        {
            depthAttachmentRef.attachment = 1;
            depthAttachmentRef.layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        }

        VkSubpassDescription subpass{};
        {
            subpass.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
            subpass.colorAttachmentCount    = 1;
            subpass.pColorAttachments       = &colorAttachmentRef;
            subpass.pDepthStencilAttachment = &depthAttachmentRef;
        }

        TArray<VkSubpassDependency, 2> dependencies;
        {
            dependencies[0].srcSubpass      = VK_SUBPASS_EXTERNAL;
            dependencies[0].dstSubpass      = 0;
            dependencies[0].srcStageMask    = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            dependencies[0].dstStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            dependencies[0].srcAccessMask   = VK_ACCESS_SHADER_READ_BIT;
            dependencies[0].dstAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
            
            dependencies[1].srcSubpass      = 0;
            dependencies[1].dstSubpass      = VK_SUBPASS_EXTERNAL;
            dependencies[1].srcStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            dependencies[1].dstStageMask    = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            dependencies[1].srcAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            dependencies[1].dstAccessMask   = VK_ACCESS_SHADER_READ_BIT;
            dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
        }

        TArray<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};

        VkRenderPassCreateInfo renderPassInfo{ VKInit::renderPassCreateInfo() };
        {
            renderPassInfo.attachmentCount = static_cast<U32>(attachments.size());
            renderPassInfo.pAttachments    = attachments.data();

            renderPassInfo.subpassCount    = 1;
            renderPassInfo.pSubpasses      = &subpass;
            
            renderPassInfo.dependencyCount = static_cast<U32>(dependencies.size());
            renderPassInfo.pDependencies   = dependencies.data();
        }

        if (vkCreateRenderPass(VyContext::device(), &renderPassInfo, nullptr, &m_RenderPass) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create frame buffer render pass!");
        }
    }


    void VyFramebuffer2::createImages()
    {
        if (m_UseMipmaps)
        {
            m_MipLevels = static_cast<U32>(std::floor(std::log2(std::max(m_Extent.width, m_Extent.height)))) + 1;
        }
        else
        {
            m_MipLevels = 1;
        }

        m_ColorImages              .resize(m_FrameCount);
        m_ColorImageViews          .resize(m_FrameCount);
        m_ColorAttachmentImageViews.resize(m_FrameCount);
        m_DepthImages              .resize(m_FrameCount);
        m_DepthImageViews          .resize(m_FrameCount);

        VkFormat colorFormat = VK_FORMAT_R16G16B16A16_SFLOAT;
        VkFormat depthFormat = VyContext::device().findSupportedFormat(
            {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
            VK_IMAGE_TILING_OPTIMAL,
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT
        );

        for (U32 i = 0; i < m_FrameCount; i++)
        {
            // Create Color Image
            m_ColorImages[i] = VyImage::Builder{}
                .imageType  (VK_IMAGE_TYPE_2D)
                .format     (colorFormat)
                .extent     (m_Extent)
                .mipLevels  (m_MipLevels, m_UseMipmaps)
                .arrayLayers(1)
                .tiling     (VK_IMAGE_TILING_OPTIMAL)
                .usage      (VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
                .memoryUsage(VMA_MEMORY_USAGE_AUTO)
                .sampleCount(VK_SAMPLE_COUNT_1_BIT)
                .sharingMode(VK_SHARING_MODE_EXCLUSIVE)
            .build();
        
            m_ColorImageViews[i] = VyImageView::Builder{}
                .viewType   (VK_IMAGE_VIEW_TYPE_2D)
                .format     (colorFormat)
                .aspectMask (VK_IMAGE_ASPECT_COLOR_BIT)
                .mipLevels  (0, m_MipLevels)
                .arrayLayers(0, 1)
            .build(m_ColorImages[i]);

            // Create Color Attachment Image View (Mip Level 0 only)
            m_ColorAttachmentImageViews[i] = VyImageView::Builder{}
                .viewType   (VK_IMAGE_VIEW_TYPE_2D)
                .format     (colorFormat)
                .aspectMask (VK_IMAGE_ASPECT_COLOR_BIT)
                .mipLevels  (0, 1)
                .arrayLayers(0, 1)
            .build(m_ColorImages[i]);


            // Create Depth Image
            m_DepthImages[i] = VyImage::Builder{}
                .imageType  (VK_IMAGE_TYPE_2D)
                .format     (depthFormat)
                .extent     (m_Extent)
                .mipLevels  (m_MipLevels, m_UseMipmaps)
                .arrayLayers(1)
                .tiling     (VK_IMAGE_TILING_OPTIMAL)
                .usage      (VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT)
                .memoryUsage(VMA_MEMORY_USAGE_AUTO)
                .sampleCount(VK_SAMPLE_COUNT_1_BIT)
                .sharingMode(VK_SHARING_MODE_EXCLUSIVE)
            .build();

            m_DepthImageViews[i] = VyImageView::Builder{}
                .viewType   (VK_IMAGE_VIEW_TYPE_2D)
                .format     (depthFormat)
                .aspectMask (VK_IMAGE_ASPECT_DEPTH_BIT)
                .mipLevels  (0, 1) // Framebuffer needs single mip level
                .arrayLayers(0, 1)
            .build(m_DepthImages[i]);


            // Create per-mip views for HZB
            m_DepthMipImageViews   .resize(m_FrameCount);
            m_DepthMipImageViews[i].resize(m_MipLevels);

            for (U32 mip = 0; mip < m_MipLevels; mip++)
            {
                m_DepthMipImageViews[i][mip] = VyImageView::Builder{}
                    .viewType   (VK_IMAGE_VIEW_TYPE_2D)
                    .format     (depthFormat)
                    .aspectMask (VK_IMAGE_ASPECT_DEPTH_BIT)
                    .mipLevels  (mip, 1)
                    .arrayLayers(0, 1)
                .build(m_DepthImages[i]);

                VY_INFO_TAG("VyFramebuffer2", "Created Depth Mip View: Frame {0}, Mip {1}", i, mip);
                // std::cout << "Created Depth Mip View: Frame " << i << ", Mip " << mip << ", Handle " << m_DepthMipImageViews[i][mip] << std::endl;
            }
        }

        // Create Sampler
        m_Sampler = VySampler::Builder{}
            .filters         (VK_FILTER_LINEAR)
            .mipmapMode      (VK_SAMPLER_MIPMAP_MODE_LINEAR)
            .addressMode     (VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE)
            .borderColor     (VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE)
            .enableAnisotropy(true)
            .lodRange        (0.0f, static_cast<float>(m_MipLevels))
            .mipLodBias      (0.0f)
        .build();

        // Create Depth Sampler (Reduction Mode Min/Max if supported, otherwise Linear)
        // For HZB we ideally want a reduction sampler, but for now we'll use a separate sampler for depth
        // Use NEAREST for depth to avoid interpolation issues during HZB generation if we do it manually
        // But for sampling in shader, we might want LINEAR if we do PCF or similar.
        // For Occlusion Culling, we want conservative depth.
        m_DepthSampler = VySampler::Builder{}
            .filters         (VK_FILTER_NEAREST)
            .mipmapMode      (VK_SAMPLER_MIPMAP_MODE_NEAREST)
            .addressMode     (VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE)
            .borderColor     (VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE)
            .enableAnisotropy(true)
            .lodRange        (0.0f, static_cast<float>(m_MipLevels))
            .mipLodBias      (0.0f)
        .build();
    }


    void VyFramebuffer2::createFramebuffers()
    {
        m_Framebuffers.resize(m_FrameCount);

        for (size_t i = 0; i < m_FrameCount; i++)
        {
            TArray<VkImageView, 2> attachments = { m_ColorAttachmentImageViews[i].handle(), m_DepthImageViews[i].handle() };

            VkFramebufferCreateInfo framebufferInfo{ VKInit::framebufferCreateInfo() };
            {
                framebufferInfo.renderPass      = m_RenderPass;

                framebufferInfo.attachmentCount = static_cast<U32>(attachments.size());
                framebufferInfo.pAttachments    = attachments.data();
                
                framebufferInfo.width           = m_Extent.width;
                framebufferInfo.height          = m_Extent.height;
                framebufferInfo.layers          = 1;
            }

            if (vkCreateFramebuffer(VyContext::device(), &framebufferInfo, nullptr, &m_Framebuffers[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("Failed to create framebuffer!");
            }
        }
    }


    VkDescriptorImageInfo VyFramebuffer2::descriptorImageInfo(int index) const
    {
        return VkDescriptorImageInfo{
            .sampler     = m_Sampler.handle(),
            .imageView   = m_ColorImageViews[index].handle(),
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        };
    }


    void VyFramebuffer2::beginRenderPass(VkCommandBuffer cmdBuffer, int frameIndex)
    {
        TArray<VkClearValue, 2> clearValues{};
        {
            clearValues[0].color        = {{ 0.01f, 0.01f, 0.01f, 1.0f }};
            clearValues[1].depthStencil = { 1.0f, 0 };
        }

        VkRenderPassBeginInfo renderPassInfo{ VKInit::renderPassBeginInfo() };
        {
            renderPassInfo.renderPass        = m_RenderPass;
            renderPassInfo.framebuffer       = m_Framebuffers[frameIndex];

            renderPassInfo.renderArea.offset = {0, 0};
            renderPassInfo.renderArea.extent = m_Extent;
            
            renderPassInfo.clearValueCount   = static_cast<U32>(clearValues.size());
            renderPassInfo.pClearValues      = clearValues.data();
        }

        vkCmdBeginRenderPass(cmdBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    }


    void VyFramebuffer2::endRenderPass(VkCommandBuffer cmdBuffer) const
    {
        vkCmdEndRenderPass(cmdBuffer);
    }


    void VyFramebuffer2::generateMipmaps(VkCommandBuffer cmdBuffer, int frameIndex)
    {
        if (!m_UseMipmaps) return;

        VkImage image     = m_ColorImages[frameIndex];
        I32     mipWidth  = m_Extent.width;
        I32     mipHeight = m_Extent.height;

        VkImageMemoryBarrier barrier{};
        barrier.sType                           = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.image                           = image;
        barrier.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
        barrier.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount     = 1;
        barrier.subresourceRange.levelCount     = 1;

        // Transition Mip 0 to TRANSFER_SRC
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.oldLayout                     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        barrier.newLayout                     = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.srcAccessMask                 = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        barrier.dstAccessMask                 = VK_ACCESS_TRANSFER_READ_BIT;

        vkCmdPipelineBarrier(cmdBuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

        for (U32 i = 1; i < m_MipLevels; i++)
        {
            // Transition Mip i to TRANSFER_DST
            barrier.subresourceRange.baseMipLevel = i;
            barrier.oldLayout                     = VK_IMAGE_LAYOUT_UNDEFINED;
            barrier.newLayout                     = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barrier.srcAccessMask                 = 0;
            barrier.dstAccessMask                 = VK_ACCESS_TRANSFER_WRITE_BIT;

            vkCmdPipelineBarrier(cmdBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

            VkImageBlit blit{};
            blit.srcOffsets[0]                 = {0, 0, 0};
            blit.srcOffsets[1]                 = {mipWidth, mipHeight, 1};
            blit.srcSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
            blit.srcSubresource.mipLevel       = i - 1;
            blit.srcSubresource.baseArrayLayer = 0;
            blit.srcSubresource.layerCount     = 1;
            blit.dstOffsets[0]                 = {0, 0, 0};
            blit.dstOffsets[1]                 = {mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1};
            blit.dstSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
            blit.dstSubresource.mipLevel       = i;
            blit.dstSubresource.baseArrayLayer = 0;
            blit.dstSubresource.layerCount     = 1;

            vkCmdBlitImage(cmdBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_LINEAR);

            // Transition Mip i-1 to SHADER_READ_ONLY
            barrier.subresourceRange.baseMipLevel = i - 1;
            barrier.oldLayout                     = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            barrier.newLayout                     = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            barrier.srcAccessMask                 = VK_ACCESS_TRANSFER_READ_BIT;
            barrier.dstAccessMask                 = VK_ACCESS_SHADER_READ_BIT;

            vkCmdPipelineBarrier(cmdBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

            // Transition Mip i to TRANSFER_SRC (for next loop)
            if (i < m_MipLevels - 1)
            {
                barrier.subresourceRange.baseMipLevel = i;
                barrier.oldLayout                     = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                barrier.newLayout                     = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
                barrier.srcAccessMask                 = VK_ACCESS_TRANSFER_WRITE_BIT;
                barrier.dstAccessMask                 = VK_ACCESS_TRANSFER_READ_BIT;

                vkCmdPipelineBarrier(cmdBuffer, 
                    VK_PIPELINE_STAGE_TRANSFER_BIT, 
                    VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 
                    0, nullptr, 
                    0, nullptr, 
                    1, &barrier
                );
            }

            if (mipWidth > 1) mipWidth /= 2;
            if (mipHeight > 1) mipHeight /= 2;
        }

        // Transition Last Mip to SHADER_READ_ONLY
        barrier.subresourceRange.baseMipLevel = m_MipLevels - 1;
        barrier.oldLayout                     = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout                     = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask                 = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask                 = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(cmdBuffer, 
            VK_PIPELINE_STAGE_TRANSFER_BIT, 
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 
            0, nullptr, 
            0, nullptr, 
            1, &barrier
        );
    }













    VyRenderPass::VyRenderPass(const VyRenderPassDesc& desc)
	{
		VkRenderPassCreateInfo renderPassInfo{ VKInit::renderPassCreateInfo() };
        {
            renderPassInfo.attachmentCount = static_cast<U32>(desc.AttachmentDescriptions.size());
            renderPassInfo.pAttachments    = desc.AttachmentDescriptions.data();

            renderPassInfo.subpassCount    = 1;
            renderPassInfo.pSubpasses      = &desc.SubpassDescription;
            
            renderPassInfo.dependencyCount = 1;
            renderPassInfo.pDependencies   = &desc.SubpassDependency;
        }

		if (vkCreateRenderPass(VyContext::device(), &renderPassInfo, nullptr, &m_RenderPass) != VK_SUCCESS) 
        {
			VY_THROW_RUNTIME_ERROR("Failed to create render pass");
		}
	}


	VyRenderPass::~VyRenderPass()
	{
		destroy();
	}


    VyRenderPass::VyRenderPass(VyRenderPass&& other) noexcept : 
        m_RenderPass(other.m_RenderPass)
    {
        other.m_RenderPass = VK_NULL_HANDLE;
    }


    VyRenderPass& VyRenderPass::operator=(VyRenderPass&& other) noexcept
    {
        if (this == &other)
        {
            return *this;
        }

        m_RenderPass = other.m_RenderPass;

        other.m_RenderPass = VK_NULL_HANDLE;

        return *this;
    }


    void VyRenderPass::destroy()
    {
        if (m_RenderPass != VK_NULL_HANDLE)
        {
            vkDestroyRenderPass(VyContext::device(), m_RenderPass, nullptr);

            m_RenderPass = VK_NULL_HANDLE;
        }
    }






    VyFramebuffer::VyFramebuffer(const VyRenderPass& renderPass, const VyFramebufferDesc& desc) : 
        m_Extent({ desc.Width, desc.Height })
    {
        if (desc.Attachments.empty()) 
        {
            VY_THROW_RUNTIME_ERROR("VyFramebuffer must have at least one attachment");
        }

        VkFramebufferCreateInfo framebufferInfo{ VKInit::framebufferCreateInfo() };
        {
            framebufferInfo.flags           = desc.Flags;
            
            framebufferInfo.renderPass      = renderPass.handle();
            
            framebufferInfo.attachmentCount = static_cast<U32>(desc.Attachments.size());
            framebufferInfo.pAttachments    = desc.Attachments.data();

            framebufferInfo.width           = m_Extent.width;
            framebufferInfo.height          = m_Extent.height;
            framebufferInfo.layers          = desc.Layers;
        }

        if (vkCreateFramebuffer(VyContext::device(), &framebufferInfo, nullptr, &m_Framebuffer) != VK_SUCCESS) 
        {
            VY_THROW_RUNTIME_ERROR("Failed to create framebuffer");
        }
    }


    VyFramebuffer::~VyFramebuffer() 
    {
        destroy();
    }


	VyFramebuffer::VyFramebuffer(VyFramebuffer&& other) noexcept
	{
		m_Framebuffer = other.m_Framebuffer;
		m_Extent      = other.m_Extent;

		other.m_Framebuffer = VK_NULL_HANDLE;
	}


	VyFramebuffer& VyFramebuffer::operator=(VyFramebuffer&& other) noexcept
	{
		m_Framebuffer = other.m_Framebuffer;
		m_Extent      = other.m_Extent;

		other.m_Framebuffer = VK_NULL_HANDLE;
		
        return *this;
	}


    void VyFramebuffer::destroy()
    {
        if (m_Framebuffer != VK_NULL_HANDLE) 
        {
            vkDestroyFramebuffer(VyContext::device(), m_Framebuffer, nullptr);

            m_Framebuffer = VK_NULL_HANDLE;
        }
    }
}