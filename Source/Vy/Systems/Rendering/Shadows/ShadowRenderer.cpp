#include <Vy/Systems/Rendering/Shadows/ShadowRenderer.h>

#include <Vy/GFX/Context.h>

namespace Vy
{
    struct ShadowPushConstant 
    {
        Mat4 ModelMatrix;
    };

    ShadowRenderer::ShadowRenderer(VkDescriptorSetLayout descriptorSetLayout)
    {
        createRenderPass();
        createResources();
        createPipeline(descriptorSetLayout);
    }

    ShadowRenderer::~ShadowRenderer() 
    {
        for (U32 i = 0; i < CASCADE_COUNT; i++) 
        {
            vkDestroyFramebuffer(VyContext::device(), m_CascadeFramebuffers[i], nullptr);
        }
        
        vkDestroyRenderPass(VyContext::device(), renderPass, nullptr);
    }


    void ShadowRenderer::render(const VyFrameInfo& frameInfo) 
    {
        for (U32 cascade = 0; cascade < CASCADE_COUNT; cascade++) 
        {
            VkClearValue clearValue = {1.0f, 0};

            VkRenderPassBeginInfo renderPassInfo{};
            {
                renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
                renderPassInfo.renderPass = renderPass;
                renderPassInfo.framebuffer = m_CascadeFramebuffers[cascade];
                renderPassInfo.renderArea.offset = {0, 0};
                renderPassInfo.renderArea.extent = {SHADOW_MAP_SIZE, SHADOW_MAP_SIZE};
                
                renderPassInfo.clearValueCount = 1;
                renderPassInfo.pClearValues = &clearValue;
            }

            vkCmdBeginRenderPass(frameInfo.CommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

            VKCmd::viewport(frameInfo.CommandBuffer, VkExtent2D{ SHADOW_MAP_SIZE, SHADOW_MAP_SIZE });
            VKCmd::scissor (frameInfo.CommandBuffer, VkExtent2D{ SHADOW_MAP_SIZE, SHADOW_MAP_SIZE });

            m_ShadowPipeline->bind(frameInfo.CommandBuffer);
            
            m_ShadowPipeline->bindDescriptorSet(frameInfo.CommandBuffer, 0, frameInfo.GlobalDescriptorSet);

            // 
            auto view = frameInfo.Scene->registry().view<ModelComponent, TransformComponent>();
            
            for (auto&& [ entity, model, transform ] : view.each())
            {
                // Bind main descriptor set for UBO/Textures if needed (shadow shader usually only needs vertex pos)
                // But pipeline was created with main descriptor layout, so we might need to bind it?
                // Yes, we need to bind it.
                // vkCmdBindDescriptorSets(frameInfo.CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &gameObject.descriptorSet, 0, nullptr);

                ShadowPushConstant push{};
                {
                    push.ModelMatrix = transform.matrix();
                }

                m_ShadowPipeline->pushConstants(frameInfo.CommandBuffer, VK_SHADER_STAGE_VERTEX_BIT, &push);

                model.Model->bind(frameInfo.CommandBuffer);
                model.Model->draw(frameInfo.CommandBuffer);
            }
            vkCmdEndRenderPass(frameInfo.CommandBuffer);
        }
        
        // Transition shadow array to shader read
        VkImageMemoryBarrier shadowBarrier{};
        {
            shadowBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            shadowBarrier.oldLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
            shadowBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            shadowBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            shadowBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            shadowBarrier.image = m_ShadowArrayImage.handle();
            shadowBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
            shadowBarrier.subresourceRange.baseMipLevel = 0;
            shadowBarrier.subresourceRange.levelCount = 1;
            shadowBarrier.subresourceRange.baseArrayLayer = 0;
            shadowBarrier.subresourceRange.layerCount = CASCADE_COUNT;
            shadowBarrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            shadowBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        }
        
        vkCmdPipelineBarrier(frameInfo.CommandBuffer,
            VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            0, 
            0, nullptr, 
            0, nullptr, 
            1, &shadowBarrier
        );
    }


    ShadowUBOData ShadowRenderer::computeShadowData(const VyCamera& camera, const Vec3& lightDir, float aspectRatio) 
    {
        ShadowUBOData data{};
        
        float nearClip    = 0.1f;
        float farClip     = 100.0f;
        float fov         = glm::radians(45.0f); 
        
        calculateCascadeSplits(nearClip, farClip, 0.5f);
        
        Mat4 camView    = camera.view();
        Mat4 invCamView = glm::inverse(camView);
        
        Vec3 lightDirNorm = -glm::normalize(lightDir);
        
        float cascadeEnds[5] = {
            nearClip,
            cascadeSplitDistances[0] * farClip,
            cascadeSplitDistances[1] * farClip,
            cascadeSplitDistances[2] * farClip,
            cascadeSplitDistances[3] * farClip
        };
        
        data.cascadeSplits = Vec4(
            cascadeEnds[1], 
            cascadeEnds[2], 
            cascadeEnds[3], 
            cascadeEnds[4]
        );
        
        for (int i = 0; i < 4; i++) 
        {
            float cascadeNear = cascadeEnds[i];
            float cascadeFar  = cascadeEnds[i + 1];
            
            float tanHalfFov = tan(fov * 0.5f);
            float nearHeight = cascadeNear * tanHalfFov;
            float nearWidth  = nearHeight * aspectRatio;
            float farHeight  = cascadeFar * tanHalfFov;
            float farWidth   = farHeight * aspectRatio;
            
            Vec3 frustumCornersVS[8] = {
                Vec3(-nearWidth, -nearHeight, -cascadeNear),
                Vec3( nearWidth, -nearHeight, -cascadeNear),
                Vec3( nearWidth,  nearHeight, -cascadeNear),
                Vec3(-nearWidth,  nearHeight, -cascadeNear),
                Vec3( -farWidth,  -farHeight,  -cascadeFar),
                Vec3(  farWidth,  -farHeight,  -cascadeFar),
                Vec3(  farWidth,   farHeight,  -cascadeFar),
                Vec3( -farWidth,   farHeight,  -cascadeFar)
            };
            
            Vec3 frustumCornersWS[8];
            Vec3 frustumCenter(0.0f);
            for (int j = 0; j < 8; j++) 
            {
                Vec4 cornerWS = invCamView * Vec4(frustumCornersVS[j], 1.0f);
                frustumCornersWS[j] = Vec3(cornerWS);
                frustumCenter += frustumCornersWS[j];
            }
            frustumCenter /= 8.0f;
            
            float radius = 0.0f;
            for (int j = 0; j < 8; j++) 
            {
                float dist = glm::length(frustumCornersWS[j] - frustumCenter);
                radius = std::max(radius, dist);
            }
            radius = std::ceil(radius * 16.0f) / 16.0f;
            
            Vec3 lightPos  = frustumCenter - lightDirNorm * radius;
            Mat4 lightView = glm::lookAt(lightPos, frustumCenter, Vec3(0.0f, 1.0f, 0.0f));
            
            float orthoSize = radius;
            float worldUnitsPerTexel = (orthoSize * 2.0f) / 2048.0f;
            
            Vec4 shadowOrigin = lightView * Vec4(frustumCenter, 1.0f);
            shadowOrigin.x = floor(shadowOrigin.x / worldUnitsPerTexel) * worldUnitsPerTexel;
            shadowOrigin.y = floor(shadowOrigin.y / worldUnitsPerTexel) * worldUnitsPerTexel;
            Vec4 snappedOriginWS = glm::inverse(lightView) * shadowOrigin;
            Vec3 snappedCenter = Vec3(snappedOriginWS);
            
            lightPos  = snappedCenter - lightDirNorm * radius;
            lightView = glm::lookAt(lightPos, snappedCenter, Vec3(0.0f, 1.0f, 0.0f));
            
            Mat4 lightProj = glm::ortho(-orthoSize, orthoSize, -orthoSize, orthoSize, 0.0f, radius * 2.0f);

            lightProj[1][1] *= -1;
            
            data.cascadeViewProj[i] = lightProj * lightView;
            
            if (i == 0) 
            {
                data.lightSpaceMatrix = data.cascadeViewProj[0];
            }
        }
        
        data.shadowParams = Vec4(2048.0f, 2.0f, 0.0005f, 0.1f);

        return data;
    }


    void ShadowRenderer::calculateCascadeSplits(float nearClip, float farClip, float lambda) 
    {
        const int NUM_CASCADES = 4;

        float clipRange = farClip - nearClip;
        float minZ      = nearClip;
        float maxZ      = nearClip + clipRange;
        float range     = maxZ - minZ;
        float ratio     = maxZ / minZ;
        
        for (int i = 0; i < NUM_CASCADES; i++) 
        {
            float p       = (i + 1) / static_cast<float>(NUM_CASCADES);
            float log     = minZ * std::pow(ratio, p);
            float uniform = minZ + range * p;
            float d       = lambda * (log - uniform) + uniform;

            cascadeSplitDistances[i] = (d - nearClip) / clipRange;
        }
    }

    void ShadowRenderer::createRenderPass() 
    {
        VkAttachmentDescription depthAttachment{};
        depthAttachment.format = VyContext::device().findDepthFormat();
        depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkAttachmentReference depthAttachmentRef{};
        depthAttachmentRef.attachment = 0;
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 0;
        subpass.pDepthStencilAttachment = &depthAttachmentRef;

        TArray<VkSubpassDependency, 2> dependencies;
        dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[0].dstSubpass = 0;
        dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        dependencies[0].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
        dependencies[0].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        dependencies[1].srcSubpass = 0;
        dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[1].srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        dependencies[1].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments = &depthAttachment;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = static_cast<U32>(dependencies.size());
        renderPassInfo.pDependencies = dependencies.data();

        if (vkCreateRenderPass(VyContext::device(), &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
            throw std::runtime_error("failed to create shadow render pass!");
        }
    }


    void ShadowRenderer::createResources() 
    {
        VkFormat depthFormat = VyContext::device().findDepthFormat();
        
        m_ShadowArrayImage = VyImage::Builder{}
            .imageType  (VK_IMAGE_TYPE_2D)
            .format     (depthFormat)
            .usage      (VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
            .extent     (VkExtent2D{ SHADOW_MAP_SIZE, SHADOW_MAP_SIZE })
            .arrayLayers(CASCADE_COUNT)
            .mipLevels  (1)
            .sampleCount(VK_SAMPLE_COUNT_1_BIT)
            .tiling     (VK_IMAGE_TILING_OPTIMAL)
            .sharingMode(VK_SHARING_MODE_EXCLUSIVE)
            .memoryUsage(VMA_MEMORY_USAGE_GPU_ONLY)
        .build();

        m_ShadowArrayImageView = VyImageView::Builder{}
            .viewType   (VK_IMAGE_VIEW_TYPE_2D)
            .format     (depthFormat)
            .aspectMask (VK_IMAGE_ASPECT_DEPTH_BIT)
            .arrayLayers(0, CASCADE_COUNT)
            .mipLevels  (0, 1)
        .build(m_ShadowArrayImage);
        
        for (U32 i = 0; i < CASCADE_COUNT; i++) 
        {
            m_CascadeViews[i] = VyImageView::Builder{}
                .viewType   (VK_IMAGE_VIEW_TYPE_2D)
                .format     (depthFormat)
                .aspectMask (VK_IMAGE_ASPECT_DEPTH_BIT)
                .arrayLayers(i, 1)
                .mipLevels  (0, 1)
            .build(m_ShadowArrayImage);
            
            VkFramebufferCreateInfo framebufferInfo{};
            {
                framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
                framebufferInfo.renderPass = renderPass;
                framebufferInfo.attachmentCount = 1;
                framebufferInfo.pAttachments = &m_CascadeViews[i].handleRef();
                framebufferInfo.width = SHADOW_MAP_SIZE;
                framebufferInfo.height = SHADOW_MAP_SIZE;
                framebufferInfo.layers = 1;
            }
            
            if (vkCreateFramebuffer(VyContext::device(), &framebufferInfo, nullptr, &m_CascadeFramebuffers[i]) != VK_SUCCESS) 
            {
                throw std::runtime_error("failed to create CSM cascade framebuffer!");
            }
        }
        
        m_ShadowSampler = VySampler::Builder{}
            .filters      (VK_FILTER_LINEAR)
            .mipmapMode   (VK_SAMPLER_MIPMAP_MODE_LINEAR)
            .addressMode  (VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER)
            // .enableCompare(VK_COMPARE_OP_LESS_OR_EQUAL)        // percentage-closer filtering (PCF) comparison
            .borderColor  (VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE) // Outside shadow map = no shadow
            .lodRange     (0.0f, 1.0f)
            .mipLodBias   (0.0f)
        .build();
    }

    
    void ShadowRenderer::createPipeline(VkDescriptorSetLayout descriptorSetLayout) 
    {
        m_ShadowPipeline = VyPipeline::GraphicsBuilder{}
            .addDescriptorSetLayout(descriptorSetLayout)
            .addPushConstantRange(VK_SHADER_STAGE_VERTEX_BIT, sizeof(ShadowPushConstant))
            .addShaderStage(VK_SHADER_STAGE_VERTEX_BIT,   "Shadows/Shadow.vert.spv")
            .addShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT, "Shadows/Shadow.frag.spv")

            // Cull front faces to reduce peter-panning
            .setCullMode(VK_CULL_MODE_FRONT_BIT)
            
            // Depth bias to prevent shadow acne
            .setDepthBias(/*ConstantFactor*/ 1.25f, /*Clamp*/ 0.0f, /*SlopeFactor*/ 1.75f)

            .setDepthTest(/*DepthTest*/ true, /*DepthWrite*/ true, VK_COMPARE_OP_LESS)

            // No color attachment - depth only
            .setDepthAttachment(VK_FORMAT_D32_SFLOAT)

            // Use the render pass
            .setRenderPass(renderPass)
        .buildUnique();


        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float)SHADOW_MAP_SIZE;
        viewport.height = (float)SHADOW_MAP_SIZE;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = {SHADOW_MAP_SIZE, SHADOW_MAP_SIZE};
    }
}