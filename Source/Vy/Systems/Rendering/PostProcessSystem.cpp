#include <Vy/Systems/Rendering/PostProcessSystem.h>

#include <Vy/GFX/Context.h>
#include <Vy/Globals.h>

namespace Vy 
{
    // =====================================================================================================================

    VyPostProcessSystem::VyPostProcessSystem(
        VkExtent2D extent
    ) :
        m_Extent(extent)
    {
        createHDRResources();
        createBloomResources();
        createRenderPasses();
        createFramebuffers();
        createDescriptorSetLayouts();
        createDescriptorSets();
        createPipelines();
    }


    VyPostProcessSystem::~VyPostProcessSystem() 
    {
        cleanup();
    }

// =========================================================================================================================
#pragma region [ Handling ]
// =========================================================================================================================

    void VyPostProcessSystem::cleanup() 
    {
        vkDeviceWaitIdle(VyContext::device());

        // Destroy pipelines
        if (m_PostProcessPipelineLayout != VK_NULL_HANDLE) 
        {
            // TODO: Being passes to pipeline and gets deleted there causing an error.
            // TEMP FIX: Storing the layout in 'tempLayoutCopy' before passing the copy to the pipeline.

            // vkDestroyPipelineLayout(VyContext::device(), m_PostProcessPipelineLayout, nullptr);
            // VyContext::destroy(m_PostProcessPipelineLayout); 
            // m_PostProcessPipelineLayout = VK_NULL_HANDLE;
        }

        m_PostProcessPipeline      .reset();
        m_BlurPipeline             .reset();
        m_BrightnessExtractPipeline.reset();

        // Destroy descriptor layouts and pool
        m_BrightnessExtractSetLayout.reset();
        m_BlurSetLayout             .reset();
        m_PostProcessSetLayout      .reset();
        m_DescriptorPool            .reset();

        // Destroy bloom framebuffers and images.
        for (int i = 0; i < 2; i++) 
        {
            for (size_t j = 0; j < MAX_FRAMES_IN_FLIGHT; j++) 
            {
                if (j < m_BloomFramebuffers[i].size()) 
                {
                    vkDestroyFramebuffer(VyContext::device(), m_BloomFramebuffers[i][j], nullptr);
                }
            }

            m_BloomFramebuffers[i].clear();
            m_BloomImageViews[i]  .clear();
            m_BloomImages[i]      .clear();
        }

        if (m_BloomRenderPass != VK_NULL_HANDLE) 
        {
            vkDestroyRenderPass(VyContext::device(), m_BloomRenderPass, nullptr);
            m_BloomRenderPass = VK_NULL_HANDLE;
        }

        // Destroy HDR framebuffers and images.
        for (size_t i = 0; i < m_HDRFramebuffers.size(); i++) 
        {
            vkDestroyFramebuffer(VyContext::device(), m_HDRFramebuffers[i], nullptr);
        }

        m_HDRFramebuffers   .clear();
        m_HDRDepthImageViews.clear();
        m_HDRDepthImages    .clear();
        m_HDRImageViews     .clear();
        m_HDRImages         .clear();

        if (m_HDRRenderPass != VK_NULL_HANDLE) 
        {
            vkDestroyRenderPass(VyContext::device(), m_HDRRenderPass, nullptr);
            m_HDRRenderPass = VK_NULL_HANDLE;
        }
    }

    // =====================================================================================================================

    void VyPostProcessSystem::recreate(VkExtent2D newExtent) 
    {
        m_Extent = newExtent;

        cleanup();
        createHDRResources();
        createBloomResources();
        createRenderPasses();
        createFramebuffers();
        createDescriptorSetLayouts();
        createDescriptorSets();
        createPipelines();
    }

#pragma endregion Handling

    
// =========================================================================================================================
#pragma region [ HDR Resources ]
// =========================================================================================================================

    void VyPostProcessSystem::createHDRResources() 
    {
        m_HDRImages         .resize(MAX_FRAMES_IN_FLIGHT);
        m_HDRImageViews     .resize(MAX_FRAMES_IN_FLIGHT);
        m_HDRDepthImages    .resize(MAX_FRAMES_IN_FLIGHT);
        m_HDRDepthImageViews.resize(MAX_FRAMES_IN_FLIGHT);

        VkFormat hdrFormat = VK_FORMAT_R16G16B16A16_SFLOAT;

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) 
        {
            // Create HDR color image
            m_HDRImages[i] = VyImage::Builder{}
                .extent     (m_Extent)
                .format     (hdrFormat)
                .tiling     (VK_IMAGE_TILING_OPTIMAL)
                .usage      (VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
                .memoryUsage(VMA_MEMORY_USAGE_AUTO)
                .build();

            // Create image view
            m_HDRImageViews[i] = VyImageView::Builder{}
                .format    (hdrFormat)
                .aspectMask(VK_IMAGE_ASPECT_COLOR_BIT)
                .build     (m_HDRImages[i]);


            VkFormat depthFormat = VyContext::device().findSupportedFormat(
                { 
                    VK_FORMAT_D32_SFLOAT, 
                    VK_FORMAT_D32_SFLOAT_S8_UINT, 
                    VK_FORMAT_D24_UNORM_S8_UINT
                },
                VK_IMAGE_TILING_OPTIMAL,
                VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
            );

            // Create depth image
            m_HDRDepthImages[i] = VyImage::Builder{}
                .extent     (m_Extent)
                .format     (depthFormat)
                .tiling     (VK_IMAGE_TILING_OPTIMAL)
                .usage      (VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
                .memoryUsage(VMA_MEMORY_USAGE_AUTO)
                .build();

            // Create depth image view
            m_HDRDepthImageViews[i] = VyImageView::Builder{}
                .format    (depthFormat)
                .aspectMask(VK_IMAGE_ASPECT_DEPTH_BIT)
                .build     (m_HDRDepthImages[i]);
        }

        // Create HDR sampler
        m_HDRSampler = VySampler::Builder{}
            .filters          (VK_FILTER_LINEAR)
            .mipmapMode       (VK_SAMPLER_MIPMAP_MODE_LINEAR)
            .addressMode      (VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE)
            .enableAnisotropy (false)
            .borderColor      (VK_BORDER_COLOR_INT_OPAQUE_BLACK)
            .build();
    }

#pragma endregion HDR Resources

    
// =========================================================================================================================
#pragma region [ Bloom Resources ]
// =========================================================================================================================

    void VyPostProcessSystem::createBloomResources() 
    {
        for (int i = 0; i < 2; i++) 
        {
            m_BloomImages[i]    .resize(MAX_FRAMES_IN_FLIGHT);
            m_BloomImageViews[i].resize(MAX_FRAMES_IN_FLIGHT);
        }

        VkFormat bloomFormat = VK_FORMAT_R16G16B16A16_SFLOAT;

        for (int pingPong = 0; pingPong < 2; pingPong++) 
        {
            for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) 
            {
                // Create bloom image
                m_BloomImages[pingPong][i] = VyImage::Builder{}
                    .extent     (VkExtent2D{ m_Extent.width / 2, m_Extent.height / 2 })
                    .format     (bloomFormat)
                    .tiling     (VK_IMAGE_TILING_OPTIMAL)
                    .usage      (VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
                    .memoryUsage(VMA_MEMORY_USAGE_AUTO)
                    .build();

                // Create bloom image view
                m_BloomImageViews[pingPong][i] = VyImageView::Builder{}
                    .format    (bloomFormat)
                    .aspectMask(VK_IMAGE_ASPECT_COLOR_BIT)
                    .build     (m_BloomImages[pingPong][i]);
            }
        }

        // Create bloom sampler
        m_BloomSampler = VySampler::Builder{}
            .filters          (VK_FILTER_LINEAR)
            .mipmapMode       (VK_SAMPLER_MIPMAP_MODE_LINEAR)
            .addressMode      (VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE)
            .enableAnisotropy (false)
            .borderColor      (VK_BORDER_COLOR_INT_OPAQUE_BLACK)
            .build();
    }

#pragma endregion Bloom Resources

    
// =========================================================================================================================
#pragma region [ Render Passes ]
// =========================================================================================================================

    void VyPostProcessSystem::createRenderPasses() 
    {
        // [ HDR Render Pass ]
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
                colorAttachment.finalLayout    = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            }

            VkAttachmentDescription depthAttachment{};
            {
                // Find and set image format to use for this depth buffer.
                depthAttachment.format = VyContext::device().findSupportedFormat(
                    { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
                    VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
                );
                // One sample per pixel (more samples used for multisampling).
                depthAttachment.samples        = VK_SAMPLE_COUNT_1_BIT;
                // Tells the depthbuffer attachment to clear each time it is loaded.
                depthAttachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
                // Contents within the depth buffer render area are not needed after rendering.
                depthAttachment.storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                // Previous contents within the stencil don't need to be preserved and will be undefined.
                depthAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                // Contents within the stencil render area are not needed after rendering.
                depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                // Inital image contents does not matter since we are clearing them on load.
                depthAttachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
                // Set so the layout after rendering allows read and write access as a depth/stencil attachment.
                depthAttachment.finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            }

            VkAttachmentReference colorAttachmentRef{};
            {
                colorAttachmentRef.attachment = 0;
                colorAttachmentRef.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
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

            VkSubpassDependency dependency{};
            {
                dependency.srcSubpass    = VK_SUBPASS_EXTERNAL;
                dependency.dstSubpass    = 0;
                
                dependency.srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
                dependency.srcAccessMask = 0;
                
                dependency.dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
                dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            }

            TArray<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };

            VkRenderPassCreateInfo renderPassInfo{ VKInit::renderPassCreateInfo() };
            {
                renderPassInfo.attachmentCount = static_cast<U32>(attachments.size());
                renderPassInfo.pAttachments    = attachments.data();
                
                renderPassInfo.subpassCount    = 1;
                renderPassInfo.pSubpasses      = &subpass;
                
                renderPassInfo.dependencyCount = 1;
                renderPassInfo.pDependencies   = &dependency;
            }

            if (vkCreateRenderPass(VyContext::device(), &renderPassInfo, nullptr, &m_HDRRenderPass) != VK_SUCCESS) 
            {
                VY_THROW_RUNTIME_ERROR("Failed to create HDR render pass!");
            }
        }

        // [ Bloom Render Pass ] (simple color attachment for bloom buffers)
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
                colorAttachment.finalLayout    = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            }

            VkAttachmentReference colorAttachmentRef{};
            {
                colorAttachmentRef.attachment = 0;
                colorAttachmentRef.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            }

            VkSubpassDescription subpass{};
            {
                subpass.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;

                subpass.colorAttachmentCount = 1;
                subpass.pColorAttachments    = &colorAttachmentRef;
            }

            VkSubpassDependency dependency{};
            {
                dependency.srcSubpass    = VK_SUBPASS_EXTERNAL;
                dependency.dstSubpass    = 0;

                dependency.srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                dependency.srcAccessMask = 0;
                
                dependency.dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            }

            VkRenderPassCreateInfo renderPassInfo{ VKInit::renderPassCreateInfo() };
            {
                renderPassInfo.attachmentCount = 1;
                renderPassInfo.pAttachments    = &colorAttachment;
                
                renderPassInfo.subpassCount    = 1;
                renderPassInfo.pSubpasses      = &subpass;

                renderPassInfo.dependencyCount = 1;
                renderPassInfo.pDependencies   = &dependency;
            }

            if (vkCreateRenderPass(VyContext::device(), &renderPassInfo, nullptr, &m_BloomRenderPass) != VK_SUCCESS) 
            {
                VY_THROW_RUNTIME_ERROR("Failed to create bloom render pass!");
            }
        }
    }

#pragma endregion Render Passes

    
// =========================================================================================================================
#pragma region [ Framebuffers ]
// =========================================================================================================================

    void VyPostProcessSystem::createFramebuffers() 
    {
        // [ HDR Framebuffers ]
        m_HDRFramebuffers.resize(MAX_FRAMES_IN_FLIGHT);

        for (size_t i = 0; i < m_HDRFramebuffers.size(); i++) 
        {
            TArray<VkImageView, 2> attachments = {
                m_HDRImageViews[i]     .handle(),
                m_HDRDepthImageViews[i].handle()
            };

            VkFramebufferCreateInfo framebufferInfo{ VKInit::framebufferCreateInfo() };
            {
                framebufferInfo.renderPass      = m_HDRRenderPass;

                framebufferInfo.attachmentCount = static_cast<U32>(attachments.size());
                framebufferInfo.pAttachments    = attachments.data();
                
                framebufferInfo.width           = m_Extent.width;
                framebufferInfo.height          = m_Extent.height;
                framebufferInfo.layers          = 1;
            }

            if (vkCreateFramebuffer(VyContext::device(), &framebufferInfo, nullptr, &m_HDRFramebuffers[i]) != VK_SUCCESS) 
            {
                VY_THROW_RUNTIME_ERROR("Failed to create HDR Framebuffer!");
            }
        }

        // [ Bloom Framebuffers ]
        for (int pingPong = 0; pingPong < 2; pingPong++) 
        {
            m_BloomFramebuffers[pingPong].resize(MAX_FRAMES_IN_FLIGHT);

            for (size_t i = 0; i < m_BloomFramebuffers[pingPong].size(); i++) 
            {
                VkFramebufferCreateInfo framebufferInfo{ VKInit::framebufferCreateInfo() };
                {
                    framebufferInfo.renderPass      = m_BloomRenderPass;

                    framebufferInfo.attachmentCount = 1;
                    framebufferInfo.pAttachments    = &m_BloomImageViews[pingPong][i].handleRef();
                    
                    framebufferInfo.width           = m_Extent.width  / 2;
                    framebufferInfo.height          = m_Extent.height / 2;
                    framebufferInfo.layers          = 1;
                }

                if (vkCreateFramebuffer(VyContext::device(), &framebufferInfo, nullptr, &m_BloomFramebuffers[pingPong][i]) != VK_SUCCESS) 
                {
                    VY_THROW_RUNTIME_ERROR("Failed to create Bloom Framebuffer!");
                }
            }
        }
    }

#pragma endregion Framebuffers

    
// =========================================================================================================================
#pragma region [ Descriptors ]
// =========================================================================================================================

    void VyPostProcessSystem::createDescriptorSetLayouts() 
    {
        // Brightness extract: single texture input
        m_BrightnessExtractSetLayout = VyDescriptorSetLayout::Builder{}
            .addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT) // hdrTexture
            .buildUnique();

        // Blur: single texture input
        m_BlurSetLayout = VyDescriptorSetLayout::Builder{}
            .addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT) // inputTexture
            .buildUnique();

        // Post process: scene texture + bloom texture
        m_PostProcessSetLayout = VyDescriptorSetLayout::Builder{}
            .addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT) // sceneTexture
            .addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT) // bloomTexture
            .buildUnique();
    }

    // =====================================================================================================================

    void VyPostProcessSystem::createDescriptorSets() 
    {
        // Descriptor pool
        m_DescriptorPool = VyDescriptorPool::Builder{}
            .setMaxSets(MAX_FRAMES_IN_FLIGHT * 6)
            .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_FRAMES_IN_FLIGHT * 8)
            .buildUnique();

        // [ Brightness Extract Descriptor Sets ]
        m_BrightnessExtractDescriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
        
        for (size_t i = 0; i < m_BrightnessExtractDescriptorSets.size(); i++) 
        {
            VkDescriptorImageInfo imageInfo{};
            {
                imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                imageInfo.imageView   = m_HDRImageViews[i].handle();
                imageInfo.sampler     = m_HDRSampler      .handle();
            }

            VyDescriptorWriter{ *m_BrightnessExtractSetLayout, *m_DescriptorPool }
                .writeImage(0, &imageInfo)
                .build(m_BrightnessExtractDescriptorSets[i]);
        }

        // [ Blur Descriptor Sets ]
        for (int pingPong = 0; pingPong < 2; pingPong++) 
        {
            m_BlurDescriptorSets[pingPong].resize(MAX_FRAMES_IN_FLIGHT);

            for (size_t i = 0; i < m_BlurDescriptorSets[pingPong].size(); i++) 
            {
                VkDescriptorImageInfo imageInfo{};
                {
                    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                    imageInfo.imageView   = m_BloomImageViews[pingPong][i].handle();
                    imageInfo.sampler     = m_BloomSampler                .handle();
                }

                VyDescriptorWriter{ *m_BlurSetLayout, *m_DescriptorPool }
                    .writeImage(0, &imageInfo)
                    .build(m_BlurDescriptorSets[pingPong][i]);
            }
        }

        // [ Post Process Descriptor Sets ]
        m_PostProcessDescriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
        
        for (size_t i = 0; i < m_PostProcessDescriptorSets.size(); i++) 
        {
            VkDescriptorImageInfo sceneImageInfo{};
            {
                sceneImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                sceneImageInfo.imageView   = m_HDRImageViews[i].handle();
                sceneImageInfo.sampler     = m_HDRSampler      .handle();
            }

            VkDescriptorImageInfo bloomImageInfo{};
            {
                bloomImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                bloomImageInfo.imageView   = m_BloomImageViews[0][i].handle(); // Use buffer 0
                bloomImageInfo.sampler     = m_BloomSampler         .handle();
            }

            VyDescriptorWriter{ *m_PostProcessSetLayout, *m_DescriptorPool }
                .writeImage(0, &sceneImageInfo)
                .writeImage(1, &bloomImageInfo)
                .build(m_PostProcessDescriptorSets[i]);
        }
    }

#pragma endregion Descriptors

    
// =========================================================================================================================
#pragma region [ Pipelines ]
// =========================================================================================================================

    void VyPostProcessSystem::createPipelines() 
    {
        // [ Brightness Extract Pipeline ]
        {
            m_BrightnessExtractPipeline = VyPipeline::GraphicsBuilder{}
                .addDescriptorSetLayout(m_BrightnessExtractSetLayout->handle())
                .addPushConstantRange(VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(float))
                .addShaderStage(VK_SHADER_STAGE_VERTEX_BIT,   "PostProcess/BrightnessExtract.vert.spv")
                .addShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT, "PostProcess/BrightnessExtract.frag.spv")
                .setDepthTest(false, false)
                .setCullMode(VK_CULL_MODE_NONE)
                .addColorAttachment(VK_FORMAT_R16G16B16A16_SFLOAT)
                .setDepthAttachment(VK_FORMAT_D32_SFLOAT)
                .setVertexBindingDescriptions  ({}) // Clear default vertex binding.
                .setVertexAttributeDescriptions({}) // Clear default vertex attributes.
                .setRenderPass(m_BloomRenderPass)
            .buildUnique();
        }

        // [ Blur Pipeline ]
        {
            m_BlurPipeline = VyPipeline::GraphicsBuilder{}
                .addDescriptorSetLayout(m_BlurSetLayout->handle())
                .addPushConstantRange(VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(Vec2))
                .addShaderStage(VK_SHADER_STAGE_VERTEX_BIT,   "PostProcess/Blur.vert.spv")
                .addShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT, "PostProcess/Blur.frag.spv")
                .setDepthTest(false, false)
                .setCullMode(VK_CULL_MODE_NONE)
                .addColorAttachment(VK_FORMAT_R16G16B16A16_SFLOAT)
                .setDepthAttachment(VK_FORMAT_D32_SFLOAT)
                .setVertexBindingDescriptions  ({}) // Clear default vertex binding.
                .setVertexAttributeDescriptions({}) // Clear default vertex attributes.
                .setRenderPass(m_BloomRenderPass)
            .buildUnique();
        }

        // [ Post Process Pipeline Layout ]
        {
            VkPushConstantRange pushConstantRange{};
            {
                pushConstantRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
                pushConstantRange.offset     = 0;
                pushConstantRange.size       = sizeof(PostProcessPushConstantData);
            }

            VkDescriptorSetLayout setLayout = m_PostProcessSetLayout->handle();

            VkPipelineLayoutCreateInfo pipelineLayoutInfo{ VKInit::pipelineLayoutCreateInfo() };
            {
                pipelineLayoutInfo.setLayoutCount         = 1;
                pipelineLayoutInfo.pSetLayouts            = &setLayout;

                pipelineLayoutInfo.pushConstantRangeCount = 1;
                pipelineLayoutInfo.pPushConstantRanges    = &pushConstantRange;
            }

            if (vkCreatePipelineLayout(VyContext::device(), &pipelineLayoutInfo, nullptr, &m_PostProcessPipelineLayout) != VK_SUCCESS) 
            {
                VY_THROW_RUNTIME_ERROR("Failed to create post process pipeline layout!");
            }

            // Will be created on first use with correct render pass.
            m_PostProcessPipeline = nullptr;
        }
    }

#pragma endregion Pipelines

    
// =========================================================================================================================
#pragma region [ Rendering ]
// =========================================================================================================================

    void VyPostProcessSystem::renderPostProcess(
        VkCommandBuffer                cmdBuffer,
        int                            frameIndex,
        const PostProcessingComponent& settings) 
    {
        VkClearValue clearColor = {{{ 0.0f, 0.0f, 0.0f, 1.0f }}};

        VkRenderPassBeginInfo renderPassInfo{ VKInit::renderPassBeginInfo()};
        {
            renderPassInfo.renderPass        = m_BloomRenderPass;
            renderPassInfo.framebuffer       = m_BloomFramebuffers[0][frameIndex]; // Ping-pong buffer 0

            renderPassInfo.renderArea.offset = { 0, 0 };
            renderPassInfo.renderArea.extent = { m_Extent.width / 2, m_Extent.height / 2 };
            
            renderPassInfo.clearValueCount   = 1;
            renderPassInfo.pClearValues      = &clearColor;
        }

        VkViewport viewport{};
        {
            viewport.x        = 0.0f;
            viewport.y        = 0.0f;
            viewport.width    = static_cast<float>(m_Extent.width  / 2);
            viewport.height   = static_cast<float>(m_Extent.height / 2);
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;
        }

        VkRect2D scissor{};
        {
            scissor.offset = { 0, 0 };
            scissor.extent = { m_Extent.width / 2, m_Extent.height / 2 };
        }

        // [ Extract bright pixels Pass ] (or clear to black if bloom is disabled)
        if (settings.BloomEnabled) 
        {
            vkCmdBeginRenderPass(cmdBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
            {
                vkCmdSetViewport(cmdBuffer, 0, 1, &viewport);
                vkCmdSetScissor (cmdBuffer, 0, 1, &scissor );

                m_BrightnessExtractPipeline->bind(cmdBuffer);

                m_BrightnessExtractPipeline->bindDescriptorSet(cmdBuffer, 0, m_BrightnessExtractDescriptorSets[frameIndex]);
                
                float threshold = settings.BloomThreshold;

                m_BrightnessExtractPipeline->pushConstants(cmdBuffer, VK_SHADER_STAGE_FRAGMENT_BIT, &threshold, sizeof(float));

                vkCmdDraw(cmdBuffer, 3, 1, 0, 0); // Full-screen triangle
            }
            vkCmdEndRenderPass(cmdBuffer);

            // [ Blur passes ]
            for (int i = 0; i < settings.BloomIterations; i++) 
            {
                // [ Horizontal Blur Pass ]
                {
                    renderPassInfo.framebuffer = m_BloomFramebuffers[1][frameIndex]; // Ping-pong buffer 1
                    
                    vkCmdBeginRenderPass(cmdBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
                    {
                        vkCmdSetViewport(cmdBuffer, 0, 1, &viewport);
                        vkCmdSetScissor (cmdBuffer, 0, 1, &scissor );

                        m_BlurPipeline->bind(cmdBuffer);

                        m_BlurPipeline->bindDescriptorSet(cmdBuffer, 0, m_BlurDescriptorSets[0][frameIndex]);

                        Vec2 direction(1.0f, 0.0f);

                        m_BlurPipeline->pushConstants(cmdBuffer, VK_SHADER_STAGE_FRAGMENT_BIT, &direction, sizeof(Vec2));

                        vkCmdDraw(cmdBuffer, 3, 1, 0, 0);
                    }
                    vkCmdEndRenderPass(cmdBuffer);
                }

                // [ Vertical Blur Pass ]
                {
                    renderPassInfo.framebuffer = m_BloomFramebuffers[0][frameIndex]; // Ping-pong buffer 0
                    
                    vkCmdBeginRenderPass(cmdBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
                    {
                        vkCmdSetViewport(cmdBuffer, 0, 1, &viewport);
                        vkCmdSetScissor (cmdBuffer, 0, 1, &scissor );

                        m_BlurPipeline->bind(cmdBuffer);

                        m_BlurPipeline->bindDescriptorSet(cmdBuffer, 0, m_BlurDescriptorSets[1][frameIndex]);

                        Vec2 direction(0.0f, 1.0f);

                        m_BlurPipeline->pushConstants(cmdBuffer, VK_SHADER_STAGE_FRAGMENT_BIT, &direction, sizeof(Vec2));

                        vkCmdDraw(cmdBuffer, 3, 1, 0, 0);
                    }
                    vkCmdEndRenderPass(cmdBuffer);
                }
            }
        } 
        else 
        {
            // Clear bloom buffer to black and transition to correct layout.
            vkCmdBeginRenderPass(cmdBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
            
            vkCmdEndRenderPass(cmdBuffer);
        }
        
    }

    // =====================================================================================================================

    void VyPostProcessSystem::renderFinalComposite(
        VkCommandBuffer                cmdBuffer,
        VkRenderPass                   swapchainRenderPass,
        int                            frameIndex,
        const PostProcessingComponent& settings
    ) {
        // Create post-process pipeline if needed.
        if (!m_PostProcessPipeline) 
        {
            auto tempLayoutCopy = m_PostProcessPipelineLayout;

            m_PostProcessPipeline = VyPipeline::GraphicsBuilder{}
                .addShaderStage(VK_SHADER_STAGE_VERTEX_BIT,   "PostProcess/PostProcess.vert.spv")
                .addShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT, "PostProcess/PostProcess.frag.spv")
                .setDepthTest(false, false)
                .setCullMode(VK_CULL_MODE_NONE) // Disable culling for fullscreen triangle.
                .addColorAttachment(VK_FORMAT_R16G16B16A16_SFLOAT)
                .setDepthAttachment(VK_FORMAT_D32_SFLOAT)
                .setVertexBindingDescriptions  ({}) // Clear default vertex binding.
                .setVertexAttributeDescriptions({}) // Clear default vertex attributes.
                .setRenderPass(swapchainRenderPass)
            .buildUnique(tempLayoutCopy);
        }

        // Bind pipeline and render full-screen quad.
        m_PostProcessPipeline->bind(cmdBuffer);

        m_PostProcessPipeline->bindDescriptorSet(cmdBuffer, 0, m_PostProcessDescriptorSets[frameIndex]);

        PostProcessPushConstantData push{};
        {
            push.BloomIntensity = settings.BloomIntensity;
            push.Exposure       = settings.Exposure;
            push.Gamma          = settings.Gamma;
            push.BloomEnabled   = settings.BloomEnabled ? 1 : 0;
            push.Contrast       = settings.Contrast;
            push.Saturation     = settings.Saturation;
            push.Vibrance       = settings.Vibrance;
        }

        m_PostProcessPipeline->pushConstants(cmdBuffer, VK_SHADER_STAGE_FRAGMENT_BIT, &push, sizeof(PostProcessPushConstantData));

        vkCmdDraw(cmdBuffer, 3, 1, 0, 0); // Full-screen triangle
    }

#pragma endregion Rendering
}




























// VkPushConstantRange pushConstantRange{};
// pushConstantRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
// pushConstantRange.offset = 0;
// pushConstantRange.size = sizeof(glm::vec2);

// VkDescriptorSetLayout setLayout = m_BlurSetLayout->getDescriptorSetLayout();
// VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
// pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
// pipelineLayoutInfo.setLayoutCount = 1;
// pipelineLayoutInfo.pSetLayouts = &setLayout;
// pipelineLayoutInfo.pushConstantRangeCount = 1;
// pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

// if (vkCreatePipelineLayout(VyContext::device(), &pipelineLayoutInfo, nullptr, &blurPipelineLayout) != VK_SUCCESS) {
//     VY_THROW_RUNTIME_ERROR("Failed to create blur pipeline layout!");
// }

// PipelineConfigInfo pipelineConfig{};
// VyPipeline::defaultPipelineConfigInfo(pipelineConfig);
// pipelineConfig.renderPass = m_BloomRenderPass;
// pipelineConfig.pipelineLayout = blurPipelineLayout;
// pipelineConfig.depthStencilInfo.depthTestEnable = VK_FALSE;
// pipelineConfig.depthStencilInfo.depthWriteEnable = VK_FALSE;
// pipelineConfig.rasterizationInfo.cullMode = VK_CULL_MODE_NONE;

// m_BlurPipeline = std::make_unique<VyPipeline>(
//     knoxicDevice,
//     "shaders/vk_blur.vert.spv",
//     "shaders/vk_blur.frag.spv",
//     pipelineConfig
// );



// VkPushConstantRange pushConstantRange{};
// pushConstantRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
// pushConstantRange.offset = 0;
// pushConstantRange.size = sizeof(float);

// VkDescriptorSetLayout setLayout = m_BrightnessExtractSetLayout->getDescriptorSetLayout();
// VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
// pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
// pipelineLayoutInfo.setLayoutCount = 1;
// pipelineLayoutInfo.pSetLayouts = &setLayout;
// pipelineLayoutInfo.pushConstantRangeCount = 1;
// pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

// if (vkCreatePipelineLayout(VyContext::device(), &pipelineLayoutInfo, nullptr, &brightnessExtractPipelineLayout) != VK_SUCCESS) {
//     VY_THROW_RUNTIME_ERROR("Failed to create brightness extract pipeline layout!");
// }

// PipelineConfigInfo pipelineConfig{};
// VyPipeline::defaultPipelineConfigInfo(pipelineConfig);
// pipelineConfig.renderPass = m_BloomRenderPass;
// pipelineConfig.pipelineLayout = brightnessExtractPipelineLayout;
// pipelineConfig.depthStencilInfo.depthTestEnable = VK_FALSE;
// pipelineConfig.depthStencilInfo.depthWriteEnable = VK_FALSE;
// pipelineConfig.rasterizationInfo.cullMode = VK_CULL_MODE_NONE;

// m_BrightnessExtractPipeline = std::make_unique<VyPipeline>(
//     knoxicDevice,
//     "shaders/vk_brightness_extract.vert.spv",
//     "shaders/vk_brightness_extract.frag.spv",
//     pipelineConfig
// );