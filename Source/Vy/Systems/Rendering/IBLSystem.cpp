#include <Vy/Systems/Rendering/IBLSystem.h>

#include <Vy/GFX/Context.h>
#include <Vy/Globals.h>

namespace Vy
{
    IBLSystem::IBLSystem() 
    {
    }


    IBLSystem::~IBLSystem()
    {
        cleanup();
    }


    void IBLSystem::cleanup()
    {
        // Destroy Irradiance Resources
        if (m_IrradianceRenderPass)
        {
            vkDestroyRenderPass(VyContext::device(), m_IrradianceRenderPass, nullptr);
            m_IrradianceRenderPass = VK_NULL_HANDLE;
        }
        if (m_IrradianceDescPool)
        {
            vkDestroyDescriptorPool(VyContext::device(), m_IrradianceDescPool, nullptr);
            m_IrradianceDescPool = VK_NULL_HANDLE;
        }
        if (m_IrradianceDescSetLayout)
        {
            vkDestroyDescriptorSetLayout(VyContext::device(), m_IrradianceDescSetLayout, nullptr);
            m_IrradianceDescSetLayout = VK_NULL_HANDLE;
        }

        // Destroy Prefilter Resources
        if (m_PrefilterRenderPass)
        {
            vkDestroyRenderPass(VyContext::device(), m_PrefilterRenderPass, nullptr);
            m_PrefilterRenderPass = VK_NULL_HANDLE;
        }
        if (m_PrefilterDescPool)
        {
            vkDestroyDescriptorPool(VyContext::device(), m_PrefilterDescPool, nullptr);
            m_PrefilterDescPool = VK_NULL_HANDLE;
        }
        if (m_PrefilterDescSetLayout)
        {
            vkDestroyDescriptorSetLayout(VyContext::device(), m_PrefilterDescSetLayout, nullptr);
            m_PrefilterDescSetLayout = VK_NULL_HANDLE;
        }

        // Destroy BRDF Resources
        if (m_BrdfDescPool)
        {
            vkDestroyDescriptorPool(VyContext::device(), m_BrdfDescPool, nullptr);
            m_BrdfDescPool = VK_NULL_HANDLE;
        }
        if (m_BrdfDescSetLayout)
        {
            vkDestroyDescriptorSetLayout(VyContext::device(), m_BrdfDescSetLayout, nullptr);
            m_BrdfDescSetLayout = VK_NULL_HANDLE;
        }
    }


    VkDescriptorImageInfo IBLSystem::irradianceDescriptorImageInfo() const
    {
        return VkDescriptorImageInfo{
                .sampler     = m_IrradianceSampler  .handle(),
                .imageView   = m_IrradianceImageView.handle(),
                .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        };
    }


    VkDescriptorImageInfo IBLSystem::prefilteredDescriptorImageInfo() const
    {
        return VkDescriptorImageInfo{
                .sampler     = m_PrefilteredSampler  .handle(),
                .imageView   = m_PrefilteredImageView.handle(),
                .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        };
    }


    VkDescriptorImageInfo IBLSystem::brdfLUTDescriptorImageInfo() const
    {
        return VkDescriptorImageInfo{
                .sampler     = m_BrdfLUTSampler  .handle(),
                .imageView   = m_BrdfLUTImageView.handle(),
                .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        };
    }


    void IBLSystem::updateSettings(const Settings& settings)
    {
        m_Settings = settings;
    }


    void IBLSystem::requestRegeneration(const Settings& newSettings, Skybox& skybox)
    {
        m_NextSettings          = newSettings;
        m_NextSkybox            = &skybox;
        m_RegenerationRequested = true;
    }


    void IBLSystem::update()
    {
        if (m_RegenerationRequested && m_NextSkybox)
        {
            // Wait for device idle to ensure no resources are in use.
            vkDeviceWaitIdle(VyContext::device());

            // Update settings.
            m_Settings = m_NextSettings;

            // Regenerate
            generateFromSkybox(*m_NextSkybox);

            // Reset flag
            m_RegenerationRequested = false;
            m_NextSkybox            = nullptr;
        }
    }


    void IBLSystem::generateFromSkybox(Skybox& skybox)
    {
        if (m_Generated)
        {
            cleanup();
            // Reset handles to null just in case cleanup doesn't do it (it should if implemented correctly)
            // cleanup() calls vkDestroy... but doesn't necessarily set handles to null unless we do it.
            // Let's check cleanup().
            // It just calls vkDestroy. It doesn't set to null.
            // So we should set them to null or ensure create... handles it.
            // create... calls createImageHelper which calls create...
            // It's safer to set m_Generated = false and rely on create... overwriting the handles.
            // But if we don't set handles to null, cleanup() might try to destroy them again if called twice?
            // cleanup() checks `if (handle)`. If we don't null them, it will crash on second cleanup.
            // So cleanup() MUST set handles to null.
        }

        createIrradianceMap();
        createPrefilteredEnvMap();
        createBRDFLUT();

        createIrradianceResources();
        generateIrradianceMap(skybox);

        createPrefilterResources();
        generatePrefilteredEnvMap(skybox);

        createBRDFResources();
        generateBRDFLUT();

        m_Generated = true;

        // Wait for everything to finish
        vkDeviceWaitIdle(VyContext::device());
    }


    // Helper to transition image layout
    void transitionImageLayoutHelper(
        VkImage       image,
        VkFormat      format,
        VkImageLayout oldLayout,
        VkImageLayout newLayout,
        U32           mipLevels,
        U32           layerCount = 1)
    {
        VkCommandBuffer commandBuffer = VyContext::device().beginSingleTimeCommands();
        {
            VkPipelineStageFlags sourceStage;
            VkPipelineStageFlags destinationStage;

            VkImageMemoryBarrier barrier{ VKInit::imageMemoryBarrier() };
            {
                barrier.oldLayout                       = oldLayout;
                barrier.newLayout                       = newLayout;
                barrier.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
                barrier.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
                barrier.image                           = image;
                barrier.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
                barrier.subresourceRange.baseMipLevel   = 0;
                barrier.subresourceRange.levelCount     = mipLevels;
                barrier.subresourceRange.baseArrayLayer = 0;
                barrier.subresourceRange.layerCount     = layerCount;

                if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && 
                    newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
                {
                    barrier.srcAccessMask = 0;
                    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

                    sourceStage           = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
                    destinationStage      = VK_PIPELINE_STAGE_TRANSFER_BIT;
                }
                else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && 
                         newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
                {
                    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

                    sourceStage           = VK_PIPELINE_STAGE_TRANSFER_BIT;
                    destinationStage      = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                }
                else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && 
                         newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
                {
                    barrier.srcAccessMask = 0;
                    barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

                    sourceStage           = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
                    destinationStage      = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                }
                else if (oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && 
                         newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
                {
                    barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

                    sourceStage           = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                    destinationStage      = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                }
                else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && 
                         newLayout == VK_IMAGE_LAYOUT_GENERAL)
                {
                    barrier.srcAccessMask = 0;
                    barrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;

                    sourceStage           = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
                    destinationStage      = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
                }
                else if (oldLayout == VK_IMAGE_LAYOUT_GENERAL && 
                         newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
                {
                    barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
                    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

                    sourceStage           = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
                    destinationStage      = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                }
                else
                {
                    VY_THROW_INVALID_ARGUMENT("Unsupported layout transition!");
                }
            }

            vkCmdPipelineBarrier(commandBuffer, 
                sourceStage, destinationStage, 0, 
                0, nullptr, 
                0, nullptr, 
                1, &barrier
            );
        }
        VyContext::device().endSingleTimeCommands(commandBuffer);
    }

    // =====================================================================================================================

    void IBLSystem::createIrradianceMap()
    {
        m_IrradianceImage = VyImage::Builder{}
            .imageType  (VK_IMAGE_TYPE_2D)
            .format     (VK_FORMAT_R32G32B32A32_SFLOAT)
            .extent     (VkExtent2D{ static_cast<U32>(m_Settings.IrradianceSize), static_cast<U32>(m_Settings.IrradianceSize) })
            .mipLevels  (1)
            .tiling     (VK_IMAGE_TILING_OPTIMAL)
            .usage      (VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
            .memoryUsage(VMA_MEMORY_USAGE_AUTO)
            .arrayLayers(6)
            .createFlags(VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT)
        .build();

        m_IrradianceImageView = VyImageView::Builder{}
            .viewType   (VK_IMAGE_VIEW_TYPE_CUBE)
            .format     (VK_FORMAT_R32G32B32A32_SFLOAT)
            .aspectMask (VK_IMAGE_ASPECT_COLOR_BIT)
            .mipLevels  (0, 1)
            .arrayLayers(0, 6)
        .build(m_IrradianceImage);

        m_IrradianceSampler = VySampler::Builder{}
            .filters         (VK_FILTER_LINEAR)
            .mipmapMode      (VK_SAMPLER_MIPMAP_MODE_LINEAR)
            .addressMode     (VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE)
            .borderColor     (VK_BORDER_COLOR_INT_OPAQUE_BLACK)
            .enableAnisotropy(true)
            .lodRange        (0.0f, 1.0f)
            .mipLodBias      (0.0f)
        .build();

        // Transition to color attachment optimal
        transitionImageLayoutHelper(
            m_IrradianceImage,
            VK_FORMAT_R32G32B32A32_SFLOAT,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            1,
            6
        );
    }

    // =====================================================================================================================

    void IBLSystem::createPrefilteredEnvMap()
    {
        m_PrefilteredImage = VyImage::Builder{}
            .imageType  (VK_IMAGE_TYPE_2D)
            .format     (VK_FORMAT_R16G16B16A16_SFLOAT)
            .extent     (VkExtent2D{ static_cast<U32>(m_Settings.PrefilterSize), static_cast<U32>(m_Settings.PrefilterSize) })
            .mipLevels  (m_Settings.PrefilterMipLevels)
            .tiling     (VK_IMAGE_TILING_OPTIMAL)
            .usage      (VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
            .memoryUsage(VMA_MEMORY_USAGE_AUTO)
            .arrayLayers(6)
            .createFlags(VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT)
        .build();

        m_PrefilteredImageView = VyImageView::Builder{}
            .viewType   (VK_IMAGE_VIEW_TYPE_CUBE)
            .format     (VK_FORMAT_R16G16B16A16_SFLOAT)
            .aspectMask (VK_IMAGE_ASPECT_COLOR_BIT)
            .mipLevels  (0, m_Settings.PrefilterMipLevels)
            .arrayLayers(0, 6)
        .build(m_PrefilteredImage);

        m_PrefilteredSampler = VySampler::Builder{}
            .filters         (VK_FILTER_LINEAR)
            .mipmapMode      (VK_SAMPLER_MIPMAP_MODE_LINEAR)
            .addressMode     (VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE)
            .borderColor     (VK_BORDER_COLOR_INT_OPAQUE_BLACK)
            .enableAnisotropy(true)
            .lodRange        (0.0f, static_cast<float>(m_Settings.PrefilterMipLevels))
            .mipLodBias      (0.0f)
        .build();

        // Transition to color attachment optimal
        transitionImageLayoutHelper(
            m_PrefilteredImage,
            VK_FORMAT_R16G16B16A16_SFLOAT,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            m_Settings.PrefilterMipLevels,
            6
        );
    }

    // =====================================================================================================================

    void IBLSystem::createBRDFLUT()
    {
        m_BrdfLUTImage = VyImage::Builder{}
            .imageType  (VK_IMAGE_TYPE_2D)
            .format     (VK_FORMAT_R16G16_SFLOAT)
            .extent     (VkExtent2D{ static_cast<U32>(m_Settings.BrdfLUTSize), static_cast<U32>(m_Settings.BrdfLUTSize) })
            .mipLevels  (m_Settings.PrefilterMipLevels)
            .tiling     (VK_IMAGE_TILING_OPTIMAL)
            .usage      (VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
            .memoryUsage(VMA_MEMORY_USAGE_AUTO)
        .build();

        m_BrdfLUTImageView = VyImageView::Builder{}
            .viewType   (VK_IMAGE_VIEW_TYPE_2D)
            .format     (VK_FORMAT_R16G16_SFLOAT)
            .aspectMask (VK_IMAGE_ASPECT_COLOR_BIT)
            .mipLevels  (0, 1)
            // .arrayLayers(0)
        .build(m_BrdfLUTImage);

        m_BrdfLUTSampler = VySampler::Builder{}
            .filters         (VK_FILTER_LINEAR)
            .mipmapMode      (VK_SAMPLER_MIPMAP_MODE_LINEAR)
            .addressMode     (VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE)
            .borderColor     (VK_BORDER_COLOR_INT_OPAQUE_BLACK)
            .lodRange        (0.0f, 1.0f)
            .mipLodBias      (0.0f)
        .build();

        // Transition to general layout for compute shader storage
        transitionImageLayoutHelper(
            m_BrdfLUTImage, 
            VK_FORMAT_R16G16_SFLOAT, 
            VK_IMAGE_LAYOUT_UNDEFINED, 
            VK_IMAGE_LAYOUT_GENERAL,
            1
        );
    }

    // =====================================================================================================================

    void IBLSystem::createIrradianceResources()
    {
        // Render Pass
        VkAttachmentDescription attachment{};
        {
            attachment.format         = VK_FORMAT_R32G32B32A32_SFLOAT;
            attachment.samples        = VK_SAMPLE_COUNT_1_BIT;
            attachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
            attachment.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
            attachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachment.initialLayout  = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            attachment.finalLayout    = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
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

        VkRenderPassCreateInfo renderPassInfo{};
        {
            renderPassInfo.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
            renderPassInfo.attachmentCount = 1;
            renderPassInfo.pAttachments    = &attachment;
            renderPassInfo.subpassCount    = 1;
            renderPassInfo.pSubpasses      = &subpass;
        }

        if (vkCreateRenderPass(VyContext::device(), &renderPassInfo, nullptr, &m_IrradianceRenderPass) != VK_SUCCESS)
        {
            VY_THROW_RUNTIME_ERROR("failed to create irradiance render pass!");
        }

        // Descriptor Set Layout
        VkDescriptorSetLayoutBinding binding{};
        {
            binding.binding            = 0;
            binding.descriptorType     = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            binding.descriptorCount    = 1;
            binding.stageFlags         = VK_SHADER_STAGE_FRAGMENT_BIT;
            binding.pImmutableSamplers = nullptr;
        }

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        {
            layoutInfo.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            layoutInfo.bindingCount = 1;
            layoutInfo.pBindings    = &binding;
        }

        if (vkCreateDescriptorSetLayout(VyContext::device(), &layoutInfo, nullptr, &m_IrradianceDescSetLayout) != VK_SUCCESS)
        {
            VY_THROW_RUNTIME_ERROR("failed to create irradiance descriptor set layout!");
        }

        m_IrradiancePipeline = VyPipeline::GraphicsBuilder{}
            .addDescriptorSetLayout(m_IrradianceDescSetLayout)
            .addPushConstantRange(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(Mat4) + sizeof(int) + sizeof(float)) // ViewProj + FaceIndex + SampleDelta
            .addShaderStage(VK_SHADER_STAGE_VERTEX_BIT,   "IrradianceConvolution.vert.spv")
            .addShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT, "IrradianceConvolution.frag.spv")
            .setCullMode(VK_CULL_MODE_NONE) // No culling for full screen quad/cube
            .addColorAttachment(VK_FORMAT_R32G32B32A32_SFLOAT)
            // .setDepthAttachment(VK_FORMAT_D32_SFLOAT)
            // No vertex input needed (generated in shader)
            .setVertexBindingDescriptions  ({}) // Clear default vertex binding.
			.setVertexAttributeDescriptions({}) // Clear default vertex attributes.
            .setRenderPass(m_IrradianceRenderPass)
        .build();

        // Descriptor Pool
        VkDescriptorPoolSize poolSize{};
        poolSize.type            = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        poolSize.descriptorCount = 1;

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = 1;
        poolInfo.pPoolSizes    = &poolSize;
        poolInfo.maxSets       = 1;

        if (vkCreateDescriptorPool(VyContext::device(), &poolInfo, nullptr, &m_IrradianceDescPool) != VK_SUCCESS)
        {
            VY_THROW_RUNTIME_ERROR("failed to create irradiance descriptor pool!");
        }

        // Allocate Descriptor Set
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool     = m_IrradianceDescPool;
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts        = &m_IrradianceDescSetLayout;

        if (vkAllocateDescriptorSets(VyContext::device(), &allocInfo, &m_IrradianceDescSet) != VK_SUCCESS)
        {
            VY_THROW_RUNTIME_ERROR("failed to allocate irradiance descriptor set!");
        }
    }

    // =====================================================================================================================

    void IBLSystem::generateIrradianceMap(Skybox& skybox)
    {
        // Update descriptor set
        VkDescriptorImageInfo imageInfo = skybox.getDescriptorInfo();

        // VyDescriptorWriter{ }
        VkWriteDescriptorSet  descriptorWrite{};
        {
            descriptorWrite.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite.dstSet          = m_IrradianceDescSet;
            descriptorWrite.dstBinding      = 0;
            descriptorWrite.dstArrayElement = 0;
            descriptorWrite.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrite.descriptorCount = 1;
            descriptorWrite.pImageInfo      = &imageInfo;
        }
            
        vkUpdateDescriptorSets(VyContext::device(), 1, &descriptorWrite, 0, nullptr);

        Mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
        Mat4 captureViews[]    = {
            glm::lookAt(Vec3(0.0f, 0.0f, 0.0f), Vec3( 1.0f,  0.0f,  0.0f), Vec3( 0.0f, -1.0f,  0.0f)),
            glm::lookAt(Vec3(0.0f, 0.0f, 0.0f), Vec3(-1.0f,  0.0f,  0.0f), Vec3( 0.0f, -1.0f,  0.0f)),
            glm::lookAt(Vec3(0.0f, 0.0f, 0.0f), Vec3( 0.0f,  1.0f,  0.0f), Vec3( 0.0f,  0.0f,  1.0f)),   // Top
            glm::lookAt(Vec3(0.0f, 0.0f, 0.0f), Vec3( 0.0f, -1.0f,  0.0f), Vec3( 0.0f,  0.0f, -1.0f)), // Bottom
            glm::lookAt(Vec3(0.0f, 0.0f, 0.0f), Vec3( 0.0f,  0.0f,  1.0f), Vec3( 0.0f, -1.0f,  0.0f)),
            glm::lookAt(Vec3(0.0f, 0.0f, 0.0f), Vec3( 0.0f,  0.0f, -1.0f), Vec3( 0.0f, -1.0f,  0.0f))
        };

        TVector<VkFramebuffer> framebuffers;
        TVector<VkImageView>   imageViews;

        VkCommandBuffer commandBuffer = VyContext::device().beginSingleTimeCommands();
        {
            for (int i = 0; i < 6; ++i)
            {
                // Create view for this face
                VyImageView faceView = VyImageView::Builder{}
                    .viewType   (VK_IMAGE_VIEW_TYPE_2D)
                    .format     (VK_FORMAT_R32G32B32A32_SFLOAT)
                    .aspectMask (VK_IMAGE_ASPECT_COLOR_BIT)
                    .mipLevels  (0, 1)
                    .arrayLayers(i, 1)
                .build(m_IrradianceImage);

                imageViews.push_back(faceView.handle());

                // Create framebuffer
                VkFramebuffer           framebuffer;
                VkFramebufferCreateInfo framebufferInfo{ VKInit::framebufferCreateInfo() };
                {
                    framebufferInfo.renderPass      = m_IrradianceRenderPass;

                    framebufferInfo.attachmentCount = 1;
                    framebufferInfo.pAttachments    = &faceView.handleRef();
                    
                    framebufferInfo.width           = m_Settings.IrradianceSize;
                    framebufferInfo.height          = m_Settings.IrradianceSize;
                    framebufferInfo.layers          = 1;
                }

                vkCreateFramebuffer(VyContext::device(), &framebufferInfo, nullptr, &framebuffer);

                framebuffers.push_back(framebuffer);

                VkClearValue clearValue = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
                
                // Render
                VkRenderPassBeginInfo renderPassInfo{ VKInit::renderPassBeginInfo() };
                {
                    renderPassInfo.renderPass        = m_IrradianceRenderPass;
                    renderPassInfo.framebuffer       = framebuffer;
                    
                    renderPassInfo.renderArea.offset = {0, 0};
                    renderPassInfo.renderArea.extent = {static_cast<U32>(m_Settings.IrradianceSize), static_cast<U32>(m_Settings.IrradianceSize)};
                    
                    renderPassInfo.clearValueCount   = 1;
                    renderPassInfo.pClearValues      = &clearValue;
                }

                vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
                {
                    VkViewport viewport{};
                    {
                        viewport.x        = 0.0f;
                        viewport.y        = 0.0f;
                        viewport.width    = static_cast<float>(m_Settings.IrradianceSize);
                        viewport.height   = static_cast<float>(m_Settings.IrradianceSize);
                        viewport.minDepth = 0.0f;
                        viewport.maxDepth = 1.0f;
                    }
                    
                    VkRect2D scissor{};
                    {
                        scissor.offset = {0, 0};
                        scissor.extent = {static_cast<U32>(m_Settings.IrradianceSize), static_cast<U32>(m_Settings.IrradianceSize)};
                    }

                    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
                    vkCmdSetScissor (commandBuffer, 0, 1, &scissor);

                    m_IrradiancePipeline.bind(commandBuffer);
                    m_IrradiancePipeline.bindDescriptorSet(commandBuffer, 0, m_IrradianceDescSet);

                    struct PushBlock
                    {
                        Mat4  MVP;
                        int   FaceIndex;
                        float SampleDelta;

                    } pushBlock;

                    // Fill push constant data.
                    {
                        pushBlock.MVP         = captureProjection * captureViews[i];
                        pushBlock.FaceIndex   = i;
                        pushBlock.SampleDelta = m_Settings.IrradianceSampleDelta;
                    }

                    m_IrradiancePipeline.pushConstants(commandBuffer, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, &pushBlock);

                    vkCmdDraw(commandBuffer, 36, 1, 0, 0);
                }
                vkCmdEndRenderPass(commandBuffer);
            }
        }
        VyContext::device().endSingleTimeCommands(commandBuffer);

        for (auto framebuffer : framebuffers)
        {
            vkDestroyFramebuffer(VyContext::device(), framebuffer, nullptr);
        }

        // Transition to shader read
        transitionImageLayoutHelper(
            m_IrradianceImage,
            VK_FORMAT_R32G32B32A32_SFLOAT,
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            1,
            6
        );
    }

    // =====================================================================================================================

    void IBLSystem::createPrefilterResources()
    {
        // Similar to Irradiance but different format and shader
        VkAttachmentDescription attachment{};
        {
            attachment.format         = VK_FORMAT_R16G16B16A16_SFLOAT;
            attachment.samples        = VK_SAMPLE_COUNT_1_BIT;
            attachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
            attachment.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
            attachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachment.initialLayout  = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            attachment.finalLayout    = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
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

        VkRenderPassCreateInfo renderPassInfo{ VKInit::renderPassCreateInfo() };
        {
            renderPassInfo.attachmentCount = 1;
            renderPassInfo.pAttachments    = &attachment;

            renderPassInfo.subpassCount    = 1;
            renderPassInfo.pSubpasses      = &subpass;
        }

        if (vkCreateRenderPass(VyContext::device(), &renderPassInfo, nullptr, &m_PrefilterRenderPass) != VK_SUCCESS)
        {
            VY_THROW_RUNTIME_ERROR("failed to create prefilter render pass!");
        }

        // Descriptor Set Layout
        VkDescriptorSetLayoutBinding binding{};
        {
            binding.binding            = 0;
            binding.descriptorType     = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            binding.descriptorCount    = 1;
            binding.stageFlags         = VK_SHADER_STAGE_FRAGMENT_BIT;
            binding.pImmutableSamplers = nullptr;
        }

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        {
            layoutInfo.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            layoutInfo.bindingCount = 1;
            layoutInfo.pBindings    = &binding;
        }

        if (vkCreateDescriptorSetLayout(VyContext::device(), &layoutInfo, nullptr, &m_PrefilterDescSetLayout) != VK_SUCCESS)
        {
            VY_THROW_RUNTIME_ERROR("failed to create prefilter descriptor set layout!");
        }

        m_PrefilterPipeline = VyPipeline::GraphicsBuilder{}
            .addDescriptorSetLayout(m_PrefilterDescSetLayout)
            .addPushConstantRange(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(Mat4) + sizeof(int) + sizeof(float) + sizeof(U32)) // ViewProj + FaceIndex + Roughness + SampleCount
            .addShaderStage(VK_SHADER_STAGE_VERTEX_BIT,   "PrefilterEnvmap.vert.spv")
            .addShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT, "PrefilterEnvmap.frag.spv")
            .setCullMode(VK_CULL_MODE_NONE)
            .addColorAttachment(VK_FORMAT_R16G16B16A16_SFLOAT)
            .setDepthAttachment(VK_FORMAT_D32_SFLOAT)
            // No vertex input needed (generated in shader)
            .setVertexBindingDescriptions  ({}) // Clear default vertex binding.
			.setVertexAttributeDescriptions({}) // Clear default vertex attributes.
            .setRenderPass(m_PrefilterRenderPass)
        .build();

        // Descriptor Pool
        VkDescriptorPoolSize poolSize{};
        {
            poolSize.type            = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            poolSize.descriptorCount = 1;
        }

        VkDescriptorPoolCreateInfo poolInfo{};
        {
            poolInfo.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            poolInfo.poolSizeCount = 1;
            poolInfo.pPoolSizes    = &poolSize;
            poolInfo.maxSets       = 1;
        }

        if (vkCreateDescriptorPool(VyContext::device(), &poolInfo, nullptr, &m_PrefilterDescPool) != VK_SUCCESS)
        {
            VY_THROW_RUNTIME_ERROR("failed to create prefilter descriptor pool!");
        }

        // Allocate Descriptor Set
        VkDescriptorSetAllocateInfo allocInfo{};
        {
            allocInfo.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            allocInfo.descriptorPool     = m_PrefilterDescPool;
            allocInfo.descriptorSetCount = 1;
            allocInfo.pSetLayouts        = &m_PrefilterDescSetLayout;
        }

        if (vkAllocateDescriptorSets(VyContext::device(), &allocInfo, &m_PrefilterDescSet) != VK_SUCCESS)
        {
            VY_THROW_RUNTIME_ERROR("failed to allocate prefilter descriptor set!");
        }
    }

    // =====================================================================================================================

    void IBLSystem::generatePrefilteredEnvMap(Skybox& skybox)
    {
        // Update descriptor set
        VkDescriptorImageInfo imageInfo = skybox.getDescriptorInfo();

        VkWriteDescriptorSet  descriptorWrite{};
        {
            descriptorWrite.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite.dstSet          = m_PrefilterDescSet;
            descriptorWrite.dstBinding      = 0;
            descriptorWrite.dstArrayElement = 0;
            descriptorWrite.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrite.descriptorCount = 1;
            descriptorWrite.pImageInfo      = &imageInfo;
        }

        vkUpdateDescriptorSets(VyContext::device(), 1, &descriptorWrite, 0, nullptr);

        Mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
        Mat4 captureViews[]    = {
            glm::lookAt(Vec3(0.0f, 0.0f, 0.0f), Vec3( 1.0f,  0.0f,  0.0f), Vec3( 0.0f, -1.0f,  0.0f)),
            glm::lookAt(Vec3(0.0f, 0.0f, 0.0f), Vec3(-1.0f,  0.0f,  0.0f), Vec3( 0.0f, -1.0f,  0.0f)),
            glm::lookAt(Vec3(0.0f, 0.0f, 0.0f), Vec3( 0.0f,  1.0f,  0.0f), Vec3( 0.0f,  0.0f,  1.0f)),
            glm::lookAt(Vec3(0.0f, 0.0f, 0.0f), Vec3( 0.0f, -1.0f,  0.0f), Vec3( 0.0f,  0.0f, -1.0f)),
            glm::lookAt(Vec3(0.0f, 0.0f, 0.0f), Vec3( 0.0f,  0.0f,  1.0f), Vec3( 0.0f, -1.0f,  0.0f)),
            glm::lookAt(Vec3(0.0f, 0.0f, 0.0f), Vec3( 0.0f,  0.0f, -1.0f), Vec3( 0.0f, -1.0f,  0.0f))
        };

        TVector<VkFramebuffer> framebuffers;
        TVector<VkImageView>   imageViews;

        VkCommandBuffer commandBuffer = VyContext::device().beginSingleTimeCommands();
        {
            for (int mip = 0; mip < m_Settings.PrefilterMipLevels; ++mip)
            {
                U32 mipWidth  = m_Settings.PrefilterSize * std::pow(0.5, mip);
                U32 mipHeight = m_Settings.PrefilterSize * std::pow(0.5, mip);
                
                float roughness = (float)mip / (float)(m_Settings.PrefilterMipLevels - 1);

                for (int i = 0; i < 6; ++i)
                {
                    VyImageView faceView = VyImageView::Builder{}
                        .viewType   (VK_IMAGE_VIEW_TYPE_2D)
                        .format     (VK_FORMAT_R16G16B16A16_SFLOAT)
                        .aspectMask (VK_IMAGE_ASPECT_COLOR_BIT)
                        .mipLevels  (mip, 1)
                        .arrayLayers(i, 1)
                    .build(m_PrefilteredImage);

                    imageViews.push_back(faceView.handle());

                    VkFramebuffer           framebuffer;
                    VkFramebufferCreateInfo framebufferInfo{ VKInit::framebufferCreateInfo() };
                    {
                        framebufferInfo.renderPass      = m_PrefilterRenderPass;

                        framebufferInfo.attachmentCount = 1;
                        framebufferInfo.pAttachments    = &faceView.handleRef();
                        
                        framebufferInfo.width           = mipWidth;
                        framebufferInfo.height          = mipHeight;
                        framebufferInfo.layers          = 1;
                    }

                    vkCreateFramebuffer(VyContext::device(), &framebufferInfo, nullptr, &framebuffer);
                    
                    framebuffers.push_back(framebuffer);

                    VkClearValue clearValue = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
                    
                    VkRenderPassBeginInfo renderPassInfo{ VKInit::renderPassBeginInfo() };
                    {
                        renderPassInfo.renderPass        = m_PrefilterRenderPass;
                        renderPassInfo.framebuffer       = framebuffer;

                        renderPassInfo.renderArea.offset = {0, 0};
                        renderPassInfo.renderArea.extent = {mipWidth, mipHeight};
                        
                        renderPassInfo.clearValueCount = 1;
                        renderPassInfo.pClearValues    = &clearValue;
                    }

                    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
                    {
                        VkViewport viewport{};
                        {
                            viewport.x        = 0.0f;
                            viewport.y        = 0.0f;
                            viewport.width    = static_cast<float>(mipWidth);
                            viewport.height   = static_cast<float>(mipHeight);
                            viewport.minDepth = 0.0f;
                            viewport.maxDepth = 1.0f;
                        }
                        
                        VkRect2D scissor{};
                        {
                            scissor.offset = {0, 0};
                            scissor.extent = {mipWidth, mipHeight};
                        }

                        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
                        vkCmdSetScissor (commandBuffer, 0, 1, &scissor);

                        m_PrefilterPipeline.bind(commandBuffer);
                        m_PrefilterPipeline.bindDescriptorSet(commandBuffer, 0, m_PrefilterDescSet);

                        struct PushBlock
                        {
                            Mat4  MVP;
                            int   FaceIndex;
                            float Roughness;
                            U32   SampleCount;

                        } pushBlock;

                        // Fill push constant data.
                        {
                            pushBlock.MVP         = captureProjection * captureViews[i];
                            pushBlock.FaceIndex   = i;
                            pushBlock.Roughness   = roughness;
                            pushBlock.SampleCount = m_Settings.PrefilterSampleCount;
                        }

                        m_PrefilterPipeline.pushConstants(commandBuffer, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, &pushBlock);

                        vkCmdDraw(commandBuffer, 36, 1, 0, 0);
                    }
                    vkCmdEndRenderPass(commandBuffer);
                }
            }
        }
        VyContext::device().endSingleTimeCommands(commandBuffer);

        for (auto framebuffer : framebuffers)
        {
            vkDestroyFramebuffer(VyContext::device(), framebuffer, nullptr);
        }

        transitionImageLayoutHelper(
            m_PrefilteredImage,
            VK_FORMAT_R16G16B16A16_SFLOAT,
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            m_Settings.PrefilterMipLevels,
            6
        );
    }

    // =====================================================================================================================

    void IBLSystem::createBRDFResources()
    {
        // Descriptor Set Layout
        VkDescriptorSetLayoutBinding binding{};
        {
            binding.binding            = 0;
            binding.descriptorType     = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
            binding.descriptorCount    = 1;
            binding.stageFlags         = VK_SHADER_STAGE_COMPUTE_BIT;
            binding.pImmutableSamplers = nullptr;
        }

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        {
            layoutInfo.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            layoutInfo.bindingCount = 1;
            layoutInfo.pBindings    = &binding;
        }

        if (vkCreateDescriptorSetLayout(VyContext::device(), &layoutInfo, nullptr, &m_BrdfDescSetLayout) != VK_SUCCESS)
        {
            VY_THROW_RUNTIME_ERROR("failed to create BRDF descriptor set layout!");
        }


        m_BrdfPipeline = VyPipeline::ComputeBuilder{}
            .addDescriptorSetLayout(m_BrdfDescSetLayout)
            .setShaderStage("BrdfLUT.comp.spv")
        .build();

        // Descriptor Pool
        VkDescriptorPoolSize poolSize{};
        {
            poolSize.type            = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
            poolSize.descriptorCount = 1;
        }

        VkDescriptorPoolCreateInfo poolInfo{};
        {
            poolInfo.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            poolInfo.poolSizeCount = 1;
            poolInfo.pPoolSizes    = &poolSize;
            poolInfo.maxSets       = 1;
        }

        if (vkCreateDescriptorPool(VyContext::device(), &poolInfo, nullptr, &m_BrdfDescPool) != VK_SUCCESS)
        {
            VY_THROW_RUNTIME_ERROR("failed to create BRDF descriptor pool!");
        }

        // Allocate Descriptor Set
        VkDescriptorSetAllocateInfo allocInfo{};
        {
            allocInfo.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            allocInfo.descriptorPool     = m_BrdfDescPool;
            allocInfo.descriptorSetCount = 1;
            allocInfo.pSetLayouts        = &m_BrdfDescSetLayout;
        }

        if (vkAllocateDescriptorSets(VyContext::device(), &allocInfo, &m_BrdfDescSet) != VK_SUCCESS)
        {
            VY_THROW_RUNTIME_ERROR("failed to allocate BRDF descriptor set!");
        }
    }

    // =====================================================================================================================

    void IBLSystem::generateBRDFLUT()
    {
        // Update descriptor set
        VkDescriptorImageInfo imageInfo{};
        {
            imageInfo.imageView   = m_BrdfLUTImageView.handle();
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
        }

        VkWriteDescriptorSet descriptorWrite{};
        {
            descriptorWrite.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite.dstSet          = m_BrdfDescSet;
            descriptorWrite.dstBinding      = 0;
            descriptorWrite.dstArrayElement = 0;
            descriptorWrite.descriptorType  = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
            descriptorWrite.descriptorCount = 1;
            descriptorWrite.pImageInfo      = &imageInfo;
        }

        vkUpdateDescriptorSets(VyContext::device(), 1, &descriptorWrite, 0, nullptr);

        VkCommandBuffer commandBuffer = VyContext::device().beginSingleTimeCommands();
        {
            m_BrdfPipeline.bind(commandBuffer);
            m_BrdfPipeline.bindDescriptorSet(commandBuffer, 0 , m_BrdfDescSet);

            vkCmdDispatch(commandBuffer, m_Settings.BrdfLUTSize / 16, m_Settings.BrdfLUTSize / 16, 1);
        }
        VyContext::device().endSingleTimeCommands(commandBuffer);

        transitionImageLayoutHelper(
            m_BrdfLUTImage, 
            VK_FORMAT_R16G16_SFLOAT, 
            VK_IMAGE_LAYOUT_GENERAL, 
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 
            1
        );
    }
}