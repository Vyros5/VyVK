#include <Vy/Systems/Rendering/RenderSystem.h>

#include <Vy/GFX/Context.h>
#include <Vy/Globals.h>

namespace Vy 
{
    VyRenderSystem::VyRenderSystem(
        VkRenderPass                   renderPass, 
        TVector<VkDescriptorSetLayout> descSetLayouts)
    {
        createPipeline(renderPass, descSetLayouts);
    }


    VyRenderSystem::~VyRenderSystem()
    {
    }


    void VyRenderSystem::createPipeline(VkRenderPass& renderPass, TVector<VkDescriptorSetLayout> descSetLayouts)
    {
        m_Pipeline = VyPipeline::GraphicsBuilder{}
            .addDescriptorSetLayouts(descSetLayouts)
            .addPushConstantRange   (VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(MaterialPushConstantData))
            .addShaderStage         (VK_SHADER_STAGE_VERTEX_BIT,   "Material.vert.spv")
            .addShaderStage         (VK_SHADER_STAGE_FRAGMENT_BIT, "Material.frag.spv")
            .addColorAttachment     (VK_FORMAT_R16G16B16A16_SFLOAT)
            .setDepthAttachment     (VK_FORMAT_D32_SFLOAT)
            .setRenderPass          (renderPass)
        .buildUnique();
    }


    void VyRenderSystem::render(const VyFrameInfo& frameInfo) 
    {
        // Bind pipeline.
        m_Pipeline->bind(frameInfo.CommandBuffer);
        
        // Bind Global descriptor set ( 0 ).
        m_Pipeline->bindDescriptorSet(frameInfo.CommandBuffer, 0, frameInfo.GlobalDescriptorSet);

        // 
        auto view = frameInfo.Scene->registry().view<ModelComponent, TransformComponent>();
        
        for (auto&& [ entity, model, transform ] : view.each())
        {
            MaterialPushConstantData push{};
            {
                push.ModelMatrix  = transform.matrix();
                push.NormalMatrix = transform.normalMatrix();
            }

            // Optional material.
            if (auto* material = frameInfo.Scene->registry().try_get<MaterialComponent>(entity))
            {
                if (material->Material)
                {
                    // Copy material data.
                    const auto& matData = material->Material->getData();
                    {
                        push.Albedo           = matData.Albedo;
                        push.Metallic         = matData.Metallic;
                        push.Roughness        = matData.Roughness;
                        push.AO               = matData.AO;
                        push.TextureOffset    = matData.TextureOffset;
                        push.TextureScale     = matData.TextureScale;
                        
                        push.EmissionColor    = matData.EmissionColor;
                        push.EmissionStrength = matData.EmissionStrength;
                    }
                    
                    // Bind material descriptor set ( 1 ). 
                    VkDescriptorSet materialDescriptorSet = material->Material->descriptorSet();
                    
                    m_Pipeline->bindDescriptorSet(frameInfo.CommandBuffer, 1, materialDescriptorSet);
                }
            }
            else {
                // Optional color fallback.
                if (auto* color = frameInfo.Scene->registry().try_get<ColorComponent>(entity))
                {
                    push.Albedo = color->Color;
                }

                // Default emission for non-material objects.
                push.EmissionColor    = Vec3(0.0f);
                push.EmissionStrength =      0.0f;
            }

            // Push material constants data.
            m_Pipeline->pushConstants(frameInfo.CommandBuffer, 
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 
                push
            );
            
            // Bind and draw model data.
            model.Model->bind(frameInfo.CommandBuffer);
            model.Model->draw(frameInfo.CommandBuffer);
        }
    }
}



namespace Vy
{
    struct MainPushConstantData
    {
        Mat4 ModelMatrix{ 1.0f };
        Mat4 NormalMatrix{ 1.0f };
    };

    struct PointShadowPassPushConstantData
    {
        Mat4 ModelMatrix{ 1.0f };
        int LightCount{};
        int FaceCount{};
    };

    struct SpotShadowPassPushConstantData
    {
        Mat4 ModelMatrix{ 1.0f };
        int LightCount{};
    };

    struct CascadedShadowPassPushConstantData
    {
        Mat4 ModelMatrix{ 1.0f };
        int CascadeIndex{};
    };

    // =====================================================================================================================

    SimpleRenderSystem::SimpleRenderSystem(VkRenderPass renderPass, TVector<VkDescriptorSetLayout> setLayouts, VyDescriptorPool& descriptorPool)
    {
        prepareShadowPassUBO();

        preparePointShadowCubeMaps();
        preparePointShadowPassRenderPass();
        preparePointShadowPassFramebuffers();

        prepareSpotShadowMaps();
        prepareSpotShadowPassRenderPass();
        prepareSpotShadowPassFramebuffers();

        //prepareShadowPassFramebuffer();
        prepareCascadeShadowPass();

        createPipelineLayout(setLayouts, descriptorPool);
        createMainPipeline(renderPass);
        createCascadedShadowPassPipeline();
        createPointShadowPassPipeline();
        createSpotShadowPassPipeline();
    }

    // =====================================================================================================================

    SimpleRenderSystem::~SimpleRenderSystem()
    {
        // vkDestroyPipelineLayout(VyContext::device(), m_MainPipelineLayout, nullptr);
        // vkDestroyPipelineLayout(VyContext::device(), m_ShadowPassPipelineLayout, nullptr);
        // vkDestroyPipelineLayout(VyContext::device(), m_PointShadowPassPipelineLayout, nullptr);

        // Depth attachment
        // vkDestroyImageView(VyContext::device(), m_ShadowPass.ShadowMapImage.View, nullptr);
        // vkDestroyImage(VyContext::device(), m_ShadowPass.ShadowMapImage.Image, nullptr);

        // vkDestroyImageView(VyContext::device(), m_PointShadowPass.PointShadowMapImage.View, nullptr);
        // vkDestroyImage(VyContext::device(), m_PointShadowPass.PointShadowMapImage.Image, nullptr);
        // vkFreeMemory(VyContext::device(), m_PointShadowPass.PointShadowMapImage.mem, nullptr);

        // vkDestroyImageView(VyContext::device(), m_SpotShadowPass.SpotShadowMapImage.View, nullptr);
        // vkDestroyImage(VyContext::device(), m_SpotShadowPass.SpotShadowMapImage.Image, nullptr);
        // vkFreeMemory(VyContext::device(), m_SpotShadowPass.SpotShadowMapImage.mem, nullptr);
    }

    // =====================================================================================================================

    void SimpleRenderSystem::renderShadowPass(VyFrameInfo frameInfo, GlobalUBO& globalUBO)
    {
        updateShadowPassBuffer(globalUBO);

        VkClearValue clearValues[2];
        {
            clearValues[0].depthStencil = { 1.0f, 0 };
        }

        VkRenderPassBeginInfo renderPassBeginInfo{ VKInit::renderPassBeginInfo() };
        {
            renderPassBeginInfo.renderPass               = m_ShadowPass.RenderPass;
            renderPassBeginInfo.framebuffer              = m_ShadowPass.Framebuffer;
            renderPassBeginInfo.renderArea.extent.width  = m_ShadowPass.Width;
            renderPassBeginInfo.renderArea.extent.height = m_ShadowPass.Height;
            renderPassBeginInfo.clearValueCount          = 1;
            renderPassBeginInfo.pClearValues             = clearValues;
        }

        vkCmdBeginRenderPass(frameInfo.CommandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        VKCmd::viewport(frameInfo.CommandBuffer, VkExtent2D{ m_ShadowPass.Width, m_ShadowPass.Height });
        VKCmd::scissor (frameInfo.CommandBuffer, VkExtent2D{ m_ShadowPass.Width, m_ShadowPass.Height });

        // Depth bias (and slope) are used to avoid shadowing artifacts
        // Constant depth bias factor (always applied)
        float depthBiasConstant = 1.25f;
        // Slope depth bias factor, applied depending on polygon's slope
        float depthBiasSlope = 1.75f;

        // Set depth bias (aka "Polygon offset")
        // Required to avoid shadow mapping artifacts
        vkCmdSetDepthBias(
            frameInfo.CommandBuffer,
            depthBiasConstant,
            0.0f,
            depthBiasSlope);

        m_ShadowPassPipeline->bind(frameInfo.CommandBuffer);

        TVector<VkDescriptorSet> globSet = { frameInfo.GlobalDescriptorSet /*, m_ShadowPassDescriptorSet*/ };

        m_ShadowPassPipeline->bindDescriptorSets(frameInfo.CommandBuffer, 0, globSet);
        
        // vkCmdBindDescriptorSets(frameInfo.CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_ShadowPassPipelineLayout, 0, globSet.size(), globSet.data(), 0, nullptr);

        renderGameObjects(frameInfo, m_ShadowPassPipeline->layout(), PushConstantType::MAIN, globSet.size(), false);

        vkCmdEndRenderPass(frameInfo.CommandBuffer);
    }

    // =====================================================================================================================

    void SimpleRenderSystem::renderCascadedShadowPass(VyFrameInfo frameInfo, GlobalUBO& globalUBO)
    {
        updateCascades(globalUBO);

        m_CascadedShadowPassBuffer->writeToBuffer(&m_CascadedShadowPass.UBO);
        m_CascadedShadowPassBuffer->flush();

        VkClearValue clearValues[1];
        {
            clearValues[0].depthStencil = { 1.0f, 0 };
        }

        VkRenderPassBeginInfo renderPassBeginInfo{ VKInit::renderPassBeginInfo() };
        {
            renderPassBeginInfo.renderPass               = m_CascadedShadowPass.RenderPass;
            renderPassBeginInfo.renderArea.offset.x      = 0;
            renderPassBeginInfo.renderArea.offset.y      = 0;
            renderPassBeginInfo.renderArea.extent.width  = m_CascadedShadowMapSize;
            renderPassBeginInfo.renderArea.extent.height = m_CascadedShadowMapSize;
            renderPassBeginInfo.clearValueCount          = 1;
            renderPassBeginInfo.pClearValues             = clearValues;
        }

        VKCmd::viewport(frameInfo.CommandBuffer, VkExtent2D{ m_CascadedShadowMapSize, m_CascadedShadowMapSize });
        VKCmd::scissor (frameInfo.CommandBuffer, VkExtent2D{ m_CascadedShadowMapSize, m_CascadedShadowMapSize });

        TVector<VkDescriptorSet> globSet = { frameInfo.GlobalDescriptorSet, m_CascadedShadowPassDescriptorSet };

        m_CascadedShadowPassPipeline->bindDescriptorSets(frameInfo.CommandBuffer, 0, globSet);

        // vkCmdBindDescriptorSets(frameInfo.CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_CascadedShadowPassPipelineLayout, 0, globSet.size(), globSet.data(), 0, nullptr);

        // One pass per cascade
        // The layer that this pass renders to is defined by the cascade's image view (selected via the cascade's descriptor set)
        for (U32 j = 0; j < CASCADE_SHADOW_MAP_COUNT; j++) 
        {
            renderPassBeginInfo.framebuffer = m_CascadedShadowPass.Cascades[j].Framebuffer;

            vkCmdBeginRenderPass(frameInfo.CommandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
            {
                m_CascadeIndex = j;
                
                m_CascadedShadowPassPipeline->bind(frameInfo.CommandBuffer);
                
                renderGameObjects(frameInfo, m_CascadedShadowPassPipeline->layout(), PushConstantType::CASCADEDSHADOW, globSet.size(), false);
            }
            vkCmdEndRenderPass(frameInfo.CommandBuffer);
        }
    }

    // =====================================================================================================================

    void SimpleRenderSystem::renderPointShadowPass(VyFrameInfo frameInfo, GlobalUBO& globalUBO)
    {
        VKCmd::viewport(frameInfo.CommandBuffer, VkExtent2D{ m_PointShadowPass.Width, m_PointShadowPass.Height });
        VKCmd::scissor (frameInfo.CommandBuffer, VkExtent2D{ m_PointShadowPass.Width, m_PointShadowPass.Height });

        for (U32 i = 0; i < globalUBO.NumPointLights; i++)
        {
            m_PointLightCount = i;
            
            for (U32 face = 0; face < 6; face++) 
            {
                m_FaceCount = face;

                updateCubeFace(face, frameInfo, globalUBO);
            }
        }
    }

    // =====================================================================================================================

    void SimpleRenderSystem::renderMainPass(VyFrameInfo frameInfo)
    {
        m_SpotShadowLightProjectionsBuffer->writeToBuffer(&m_SpotShadowLightProjectionsUBO);
        m_SpotShadowLightProjectionsBuffer->flush();

        m_MainPipeline->bind(frameInfo.CommandBuffer);

        TVector<VkDescriptorSet> globSet = { 
            frameInfo.GlobalDescriptorSet, 
            //m_ShadowPassDescriptorSet, m_ShadowMapDescriptorSet,
            m_CascadedShadowPassDescriptorSet, m_CascadedShadowMapDescriptorSet,
            m_PointShadowMapDescriptorSet,
            m_SpotShadowMapDescriptorSet
        };

        m_MainPipeline->bindDescriptorSets(frameInfo.CommandBuffer, 0, globSet);

        // vkCmdBindDescriptorSets(frameInfo.CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_MainPipelineLayout, 0, globSet.size(), globSet.data(), 0, nullptr);

        renderGameObjects(frameInfo, m_MainPipeline->layout(), PushConstantType::MAIN, globSet.size(), true);
    }

    // =====================================================================================================================

    void SimpleRenderSystem::renderGameObjects(
        VyFrameInfo      frameInfo, 
        VkPipelineLayout pipelineLayout, 
        PushConstantType type, 
        int              setCount, 
        bool             bRenderMaterial)
    {
        auto view = frameInfo.Scene->registry().view<ModelComponent, TransformComponent>();
        
        for (auto&& [ entity, model, transform ] : view.each())
        {
            if (type == PushConstantType::MAIN)
            {
                MainPushConstantData data{};
                {
                    data.ModelMatrix  = transform.matrix();
                    data.NormalMatrix = transform.normalMatrix();
                }

                vkCmdPushConstants(
                    frameInfo.CommandBuffer,
                    pipelineLayout,
                    VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0,
                    sizeof(data),
                    &data
                );
            }
            else if(type == PushConstantType::POINTSHADOW)
            {
                PointShadowPassPushConstantData data{};
                {
                    data.ModelMatrix = transform.matrix();
                    data.LightCount  = m_PointLightCount;
                    data.FaceCount   = m_FaceCount;
                }

                vkCmdPushConstants(
                    frameInfo.CommandBuffer,
                    pipelineLayout,
                    VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0,
                    sizeof(data),
                    &data
                );
            }
            else if (type == PushConstantType::SPOTSHADOW)
            {
                SpotShadowPassPushConstantData data{};
                {
                    data.ModelMatrix = transform.matrix();
                    data.LightCount  = m_SpotLightIndex;
                }

                vkCmdPushConstants(
                    frameInfo.CommandBuffer,
                    pipelineLayout,
                    VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0,
                    sizeof(data),
                    &data
                );
            }
            else if (type == PushConstantType::CASCADEDSHADOW)
            {
                CascadedShadowPassPushConstantData data{};
                {
                    data.ModelMatrix  = transform.matrix();
                    data.CascadeIndex = m_CascadeIndex;
                }

                vkCmdPushConstants(
                    frameInfo.CommandBuffer,
                    pipelineLayout,
                    VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0,
                    sizeof(data),
                    &data
                );
            }

            model.Model->bind(frameInfo.CommandBuffer);
            model.Model->draw(frameInfo.CommandBuffer /*, pipelineLayout, setCount, bRenderMaterial */);
        }
    }

    // =====================================================================================================================

    void SimpleRenderSystem::createPipelineLayout(TVector<VkDescriptorSetLayout> setLayouts, VyDescriptorPool& descriptorPool)
    {
        // m_MainSetLayouts              .push_back(setLayouts[0]);
        // m_ShadowPassSetLayouts        .push_back(setLayouts[0]);
        // m_PointShadowPassSetLayouts   .push_back(setLayouts[0]);
        // m_SpotShadowPassSetLayouts    .push_back(setLayouts[0]);
        // m_CascadedShadowPassSetLayouts.push_back(setLayouts[0]);
        
        // ----------------------------------------------------------------------------------------
        
        // VkPushConstantRange mainPushConstantRange{};
        // {
        //     mainPushConstantRange.stageFlags = { VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT };
        //     mainPushConstantRange.offset     = 0;
        //     mainPushConstantRange.size       = sizeof(MainPushConstantData);
        // }

        // VkPushConstantRange pointShadowPushConstantRange{};
        // {
        //     pointShadowPushConstantRange.stageFlags = { VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT };
        //     pointShadowPushConstantRange.offset     = 0;
        //     pointShadowPushConstantRange.size       = sizeof(PointShadowPassPushConstantData);
        // }

        // VkPushConstantRange spotShadowPushConstantRange{};
        // {
        //     spotShadowPushConstantRange.stageFlags = { VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT };
        //     spotShadowPushConstantRange.offset     = 0;
        //     spotShadowPushConstantRange.size       = sizeof(SpotShadowPassPushConstantData);
        // }

        // VkPushConstantRange cascadedShadowPushConstantRange{};
        // {
        //     cascadedShadowPushConstantRange.stageFlags = { VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT };
        //     cascadedShadowPushConstantRange.offset     = 0;
        //     cascadedShadowPushConstantRange.size       = sizeof(CascadedShadowPassPushConstantData);
        // }

        // ----------------------------------------------------------------------------------------
        
        // ShadowPass Pipeline Layout
        //auto shadowPassUBOLayout = VyDescriptorSetLayout::Builder{}
        //	.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
        //	.build();
        //auto bufferInfo = m_ShadowPassBuffer->descriptorInfo();
        //VyDescriptorWriter(*shadowPassUBOLayout, descriptorPool)
        //	.writeBuffer(0, &bufferInfo)
        //	.build(m_ShadowPassDescriptorSet);
        //m_ShadowPassSetLayouts.push_back(shadowPassUBOLayout->handle());
        ////m_ShadowPassSetLayouts.push_back(setLayouts[1]);

        //VkPipelineLayoutCreateInfo shadowPassPipelineLayoutInfo{};
        //shadowPassPipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        //shadowPassPipelineLayoutInfo.setLayoutCount = static_cast<U32>(m_ShadowPassSetLayouts.size());
        //shadowPassPipelineLayoutInfo.pSetLayouts = m_ShadowPassSetLayouts.data();
        //shadowPassPipelineLayoutInfo.pushConstantRangeCount = 1;
        //shadowPassPipelineLayoutInfo.pPushConstantRanges = &mainPushConstantRange;

        //if (vkCreatePipelineLayout(VyContext::device(), &shadowPassPipelineLayoutInfo, nullptr, &m_ShadowPassPipelineLayout) != VK_SUCCESS)
        //{
        //	VY_THROW_RUNTIME_ERROR("Failed to create SimpleRenderSystem:ShadowPassPipelineLayout");
        //}

        // ----------------------------------------------------------------------------------------
        
        // Cascaded Shadow Pass
        auto cascadedShadowPassUBOLayout = VyDescriptorSetLayout::Builder{}
            .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
        .buildUnique();

        auto bufferInfoCas = m_CascadedShadowPassBuffer->descriptorBufferInfo();

        VyDescriptorWriter{ *cascadedShadowPassUBOLayout, descriptorPool }
            .writeBuffer(0, &bufferInfoCas)
        .build(m_CascadedShadowPassDescriptorSet);

        m_CascadedShadowPassSetLayouts.push_back(cascadedShadowPassUBOLayout->handle());

        // VkPipelineLayoutCreateInfo cascadedShadowPassPipelineLayoutInfo{};
        // cascadedShadowPassPipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        // cascadedShadowPassPipelineLayoutInfo.setLayoutCount = static_cast<U32>(m_CascadedShadowPassSetLayouts.size());
        // cascadedShadowPassPipelineLayoutInfo.pSetLayouts = m_CascadedShadowPassSetLayouts.data();
        // cascadedShadowPassPipelineLayoutInfo.pushConstantRangeCount = 1;
        // cascadedShadowPassPipelineLayoutInfo.pPushConstantRanges = &mainPushConstantRange;

        // if (vkCreatePipelineLayout(VyContext::device(), &cascadedShadowPassPipelineLayoutInfo, nullptr, &m_CascadedShadowPassPipelineLayout) != VK_SUCCESS)
        // {
        //     VY_THROW_RUNTIME_ERROR("Failed to create SimpleRenderSystem:CascadedShadowPassPipelineLayout");
        // }

        // ----------------------------------------------------------------------------------------
        
        // Main Pipeline Layout
            // Shadow Map descriptorSet
        //auto shadowMapDescriptorSetLayout = VyDescriptorSetLayout::Builder{}
        //	.addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
        //	.buildUnique();
        //VkDescriptorImageInfo shadowMapDescriptor{};
        //shadowMapDescriptor.sampler = m_ShadowPass.shadowMapSampler;
        //shadowMapDescriptor.imageView = m_ShadowPass.ShadowMapImage.View;
        //shadowMapDescriptor.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
        //VyDescriptorWriter(*shadowMapDescriptorSetLayout, descriptorPool)
        //	.writeImage(0, &shadowMapDescriptor)
        //	.build(m_ShadowMapDescriptorSet);
        //m_MainSetLayouts.push_back(shadowPassUBOLayout->handle());
        //m_MainSetLayouts.push_back(shadowMapDescriptorSetLayout->handle());

        // ----------------------------------------------------------------------------------------

        // Cascaded Shadow Map descriptorSet
        auto cascadedShadowMapDescriptorSetLayout = VyDescriptorSetLayout::Builder{}
            .addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
        .buildUnique();

        VkDescriptorImageInfo cascadedshadowMapDescriptor{};
        {
            cascadedshadowMapDescriptor.sampler     = m_CascadedDepthMapObject.Sampler;
            cascadedshadowMapDescriptor.imageView   = m_CascadedDepthMapObject.View;
            cascadedshadowMapDescriptor.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
        }

        VyDescriptorWriter(*cascadedShadowMapDescriptorSetLayout, descriptorPool)
            .writeImage(0, &cascadedshadowMapDescriptor)
            .build(m_CascadedShadowMapDescriptorSet);

        m_MainSetLayouts.push_back(cascadedShadowPassUBOLayout->handle());
        m_MainSetLayouts.push_back(cascadedShadowMapDescriptorSetLayout->handle());

        // ----------------------------------------------------------------------------------------
        
        // Point Shadow Map descriptorSet
        auto pointShadowMapDescriptorSetLayout = VyDescriptorSetLayout::Builder{}
            .addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
        .buildUnique();

        VkDescriptorImageInfo pointShadowMapDescriptor{};
        {
            pointShadowMapDescriptor.sampler     = m_PointShadowCubeMaps.CubeMapSampler;
            pointShadowMapDescriptor.imageView   = m_PointShadowCubeMaps.CubeMapImage.View;
            pointShadowMapDescriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        }

        VyDescriptorWriter(*pointShadowMapDescriptorSetLayout, descriptorPool)
            .writeImage(0, &pointShadowMapDescriptor)
            .build(m_PointShadowMapDescriptorSet);

        m_MainSetLayouts.push_back(pointShadowMapDescriptorSetLayout->handle());

        // ----------------------------------------------------------------------------------------
        
        // Spot Shadow Map descriptorSet
        auto spotShadowMapDescriptorSetLayout = VyDescriptorSetLayout::Builder{}
            .addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
            .addBinding(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
        .buildUnique();

        VkDescriptorImageInfo spotShadowMapDescriptor{};
        {
            spotShadowMapDescriptor.sampler     = m_SpotShadowMaps.CubeMapSampler;
            spotShadowMapDescriptor.imageView   = m_SpotShadowMaps.CubeMapImage.View;
            spotShadowMapDescriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        }

        auto bufferInfoTwo = m_SpotShadowLightProjectionsBuffer->descriptorBufferInfo();

        VyDescriptorWriter(*spotShadowMapDescriptorSetLayout, descriptorPool)
            .writeImage (0, &spotShadowMapDescriptor)
            .writeBuffer(1, &bufferInfoTwo)
            .build(m_SpotShadowMapDescriptorSet);

        m_MainSetLayouts.push_back(spotShadowMapDescriptorSetLayout->handle());
        m_MainSetLayouts.push_back(setLayouts[1]);

        // ----------------------------------------------------------------------------------------
        
        // VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        // {
        //     pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        //     pipelineLayoutInfo.setLayoutCount = static_cast<U32>(m_MainSetLayouts.size());
        //     pipelineLayoutInfo.pSetLayouts = m_MainSetLayouts.data();
        //     pipelineLayoutInfo.pushConstantRangeCount = 1;
        //     pipelineLayoutInfo.pPushConstantRanges = &mainPushConstantRange;
        // }

        // if (vkCreatePipelineLayout(VyContext::device(), &pipelineLayoutInfo, nullptr, &m_MainPipelineLayout) != VK_SUCCESS)
        // {
        //     VY_THROW_RUNTIME_ERROR("Failed to create SimpleRenderSystem:MainPipelineLayout");
        // }


        // Point Shadow Pass Pipeline Layout
        auto pointShadowPassUBOLayout = VyDescriptorSetLayout::Builder{}
            .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
        .buildUnique();

        auto pointBufferInfo = m_PointShadowPassBuffer->descriptorBufferInfo();
        
        VyDescriptorWriter(*pointShadowPassUBOLayout, descriptorPool)
            .writeBuffer(0, &pointBufferInfo)
            .build(m_PointShadowPassDescriptorSet);

        m_PointShadowPassSetLayouts.push_back(pointShadowPassUBOLayout->handle());
        //m_PointShadowPassSetLayouts.push_back(setLayouts[1]);

        // VkPipelineLayoutCreateInfo pointShadowPassPipelineLayoutInfo{};
        // {
        //     pointShadowPassPipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        //     pointShadowPassPipelineLayoutInfo.setLayoutCount = static_cast<U32>(m_PointShadowPassSetLayouts.size());
        //     pointShadowPassPipelineLayoutInfo.pSetLayouts = m_PointShadowPassSetLayouts.data();
        //     pointShadowPassPipelineLayoutInfo.pushConstantRangeCount = 1;
        //     pointShadowPassPipelineLayoutInfo.pPushConstantRanges = &pointShadowPushConstantRange;
        // }

        // if (vkCreatePipelineLayout(VyContext::device(), &pointShadowPassPipelineLayoutInfo, nullptr, &m_PointShadowPassPipelineLayout) != VK_SUCCESS)
        // {
        //     VY_THROW_RUNTIME_ERROR("Failed to create SimpleRenderSystem:PointShadowPassPipelineLayout");
        // }

        // Spot Shadow Pass Pipeline Layout
        auto spotShadowPassUBOLayout = VyDescriptorSetLayout::Builder{}
            .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
        .buildUnique();

        auto spotBufferInfo = m_SpotShadowPassBuffer->descriptorBufferInfoForIndex(0); //check
        //auto spotBufferInfo = m_SpotShadowPassBuffer->descriptorBufferInfo(sizeof(ShadowPassUBO)); //check

        VyDescriptorWriter(*spotShadowPassUBOLayout, descriptorPool)
            .writeBuffer(0, &spotBufferInfo)
            .build(m_SpotShadowPassDescriptorSet);

        m_SpotShadowPassSetLayouts.push_back(spotShadowPassUBOLayout->handle());
        //m_SpotShadowPassSetLayouts.push_back(setLayouts[1]);

        // VkPipelineLayoutCreateInfo spotShadowPassPipelineLayoutInfo{};
        // {
        //     spotShadowPassPipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        //     spotShadowPassPipelineLayoutInfo.setLayoutCount = static_cast<U32>(m_SpotShadowPassSetLayouts.size());
        //     spotShadowPassPipelineLayoutInfo.pSetLayouts = m_SpotShadowPassSetLayouts.data();
        //     spotShadowPassPipelineLayoutInfo.pushConstantRangeCount = 1;
        //     spotShadowPassPipelineLayoutInfo.pPushConstantRanges = &spotShadowPushConstantRange;
        // }

        // if (vkCreatePipelineLayout(VyContext::device(), &spotShadowPassPipelineLayoutInfo, nullptr, &m_SpotShadowPassPipelineLayout) != VK_SUCCESS)
        // {
        //     VY_THROW_RUNTIME_ERROR("Failed to create SimpleRenderSystem:SpotShadowPassPipelineLayout");
        // }
    }


    void SimpleRenderSystem::createMainPipeline(VkRenderPass renderPass)
    {
        m_MainPipeline = VyPipeline::GraphicsBuilder{}
            .addDescriptorSetLayouts(m_MainSetLayouts)
            .addPushConstantRange(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(MainPushConstantData))
            .addShaderStage(VK_SHADER_STAGE_VERTEX_BIT,   "Basic.vert.spv")
            .addShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT, "Basic.frag.spv")
            // .addColorAttachment(VK_FORMAT_R16G16B16A16_SFLOAT, true)
            // .setDepthAttachment(VK_FORMAT_D32_SFLOAT)
            // .setCullMode(VK_CULL_MODE_BACK_BIT)
            // .setTopology(VK_PRIMITIVE_TOPOLOGY_LINE_LIST)
            // .setDepthTest(true, false)
            .setRasterizationSamples(VyContext::device().supportedSampleCount())
            .setRenderPass(renderPass)
			// .clearVertexDescriptions() // Clear default vertex bindings and attributes.
        .buildUnique();
    }


    void SimpleRenderSystem::createShadowPassPipeline()
    {
        // m_ShadowPassPipeline = VyPipeline::GraphicsBuilder{}
        //     .addDescriptorSetLayouts(m_ShadowPassSetLayouts)
        //     .addPushConstantRange(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(PointLightPushConstantData))
        //     .addShaderStage(VK_SHADER_STAGE_VERTEX_BIT,   "Lighting/PointLight.vert.spv")
        //     .addShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT, "Lighting/PointLight.frag.spv")
        //     // .addColorAttachment(VK_FORMAT_R16G16B16A16_SFLOAT, true)
        //     // .setDepthAttachment(VK_FORMAT_D32_SFLOAT)
        //     // .setTopology(VK_PRIMITIVE_TOPOLOGY_LINE_LIST)
        //     // .setDepthTest(true, false)
        //     .setRenderPass(renderPass)
		// 	.clearVertexDescriptions() // Clear default vertex bindings and attributes.
        // .buildUnique();
    }


    void SimpleRenderSystem::createCascadedShadowPassPipeline()
    {
        m_CascadedShadowPassPipeline = VyPipeline::GraphicsBuilder{}
            .addDescriptorSetLayouts(m_CascadedShadowPassSetLayouts)
            .addPushConstantRange(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(CascadedShadowPassPushConstantData))
            .addShaderStage(VK_SHADER_STAGE_VERTEX_BIT,   "CascadedShadowPass.vert.spv")
            .addShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT, "CascadedShadowPass.frag.spv")
            // .addColorAttachment(VK_FORMAT_R16G16B16A16_SFLOAT, true)
            // .setDepthAttachment(VK_FORMAT_D32_SFLOAT)
            .setCullMode(VK_CULL_MODE_BACK_BIT)
            // .setTopology(VK_PRIMITIVE_TOPOLOGY_LINE_LIST)
            // .setDepthTest(true, false)
            .setRenderPass(m_CascadedShadowPass.RenderPass)
			// .clearVertexDescriptions() // Clear default vertex bindings and attributes.
        .buildUnique();
    }


    void SimpleRenderSystem::createPointShadowPassPipeline()
    {
        m_PointShadowPassPipeline = VyPipeline::GraphicsBuilder{}
            .addDescriptorSetLayouts(m_PointShadowPassSetLayouts)
            .addPushConstantRange(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(PointShadowPassPushConstantData))
            .addShaderStage(VK_SHADER_STAGE_VERTEX_BIT,   "PointShadowPass.vert.spv")
            .addShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT, "PointShadowPass.frag.spv")
            // .addColorAttachment(VK_FORMAT_R16G16B16A16_SFLOAT, true)
            // .setDepthAttachment(VK_FORMAT_D32_SFLOAT)
            // .setCullMode(VK_CULL_MODE_BACK_BIT)
            // .setTopology(VK_PRIMITIVE_TOPOLOGY_LINE_LIST)
            // .setDepthTest(true, false)
            .setRenderPass(m_PointShadowPass.RenderPass)
			// .clearVertexDescriptions() // Clear default vertex bindings and attributes.
        .buildUnique();
    }


    void SimpleRenderSystem::createSpotShadowPassPipeline()
    {
        m_SpotShadowPassPipeline = VyPipeline::GraphicsBuilder{}
            .addDescriptorSetLayouts(m_SpotShadowPassSetLayouts)
            .addPushConstantRange(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(SpotShadowPassPushConstantData))
            .addShaderStage(VK_SHADER_STAGE_VERTEX_BIT,   "SpotShadowPass.vert.spv")
            .addShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT, "SpotShadowPass.frag.spv")
            // .addColorAttachment(VK_FORMAT_R16G16B16A16_SFLOAT, true)
            // .setDepthAttachment(VK_FORMAT_D32_SFLOAT)
            // .setCullMode(VK_CULL_MODE_BACK_BIT)
            // .setTopology(VK_PRIMITIVE_TOPOLOGY_LINE_LIST)
            // .setDepthTest(true, false)
            .setRenderPass(m_SpotShadowPass.RenderPass)
			// .clearVertexDescriptions() // Clear default vertex bindings and attributes.
        .buildUnique();
    }





    // =====================================================================================================================

    void SimpleRenderSystem::createPipeline(VkRenderPass renderPass)
    {
        // // Main Pipeline
        // assert(m_MainPipelineLayout != nullptr && "Cannot create SimpleRenderSystem:MainPipeline before MainPipelineLayout");

        // PipelineConfigInfo pipelineConfig{};
        // Pipeline::DefaultPipelineConfigInfo(pipelineConfig);
        // pipelineConfig.renderPass = renderPass;
        // pipelineConfig.pipelineLayout = m_MainPipelineLayout;
        // pipelineConfig.multisampleInfo.rasterizationSamples = m_Device.msaaSampleCountFlagBits();

        // m_MainPipeline = std::make_unique<VyPipeline>(m_Device, "Assets/Shaders/Basic.vert.spv", "Assets/Shaders/Basic.frag.spv", pipelineConfig);

        // // Point Shadow Pass Pipeline
        // assert(m_PointShadowPassPipelineLayout != nullptr && "Cannot create SimpleRenderSystem:PointShadowPassPipeline before PointShadowPassPipelineLayout");

        // pipelineConfig.renderPass = m_PointShadowPass.RenderPass;
        // pipelineConfig.pipelineLayout = m_PointShadowPassPipelineLayout;
        // pipelineConfig.multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        // m_PointShadowPassPipeline = std::make_unique<VyPipeline>(m_Device, "Assets/Shaders/PointShadowPass.vert.spv", "Assets/Shaders/PointShadowPass.frag.spv", pipelineConfig);

        // // Spot Shadow Pass Pipeline
        // assert(m_SpotShadowPassPipelineLayout != nullptr && "Cannot create SimpleRenderSystem:SpotShadowPassPipeline before SpotShadowPassPipelineLayout");

        // pipelineConfig.renderPass = m_SpotShadowPass.RenderPass;
        // pipelineConfig.pipelineLayout = m_SpotShadowPassPipelineLayout;
        // pipelineConfig.multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        // m_SpotShadowPassPipeline = std::make_unique<VyPipeline>(m_Device, "Assets/Shaders/SpotShadowPass.vert.spv", "Assets/Shaders/SpotShadowPass.frag.spv", pipelineConfig);

        // // Shadow Pass Pipeline
        // //assert(m_ShadowPassPipelineLayout != nullptr && "Cannot create SimpleRenderSystem:ShadowPassPipeline before ShadowPassPipelineLayout");

        // //pipelineConfig.colorBlendInfo.attachmentCount = 0;
        // //pipelineConfig.rasterizationInfo.depthBiasEnable = VK_TRUE;

        // //pipelineConfig.dynamicStateEnables.push_back(VK_DYNAMIC_STATE_DEPTH_BIAS);
        // //pipelineConfig.dynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        // //pipelineConfig.dynamicStateCreateInfo.pDynamicStates = pipelineConfig.dynamicStateEnables.data();
        // //pipelineConfig.dynamicStateCreateInfo.dynamicStateCount = static_cast<U32>(pipelineConfig.dynamicStateEnables.size());

        // //pipelineConfig.renderPass = m_ShadowPass.RenderPass;
        // //pipelineConfig.pipelineLayout = m_ShadowPassPipelineLayout;
        // //pipelineConfig.multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        // //pipelineConfig.rasterizationInfo.cullMode = VK_CULL_MODE_BACK_BIT;

        // //m_ShadowPassPipeline = std::make_unique<VyPipeline>(m_Device, "Assets/Shaders/ShadowPass.vert.spv", "Assets/Shaders/ShadowPass.frag.spv", pipelineConfig);

        // // Cascaded Shadow Pass Pipeline
        // assert(m_CascadedShadowPassPipelineLayout != nullptr && "Cannot create SimpleRenderSystem:CascadedShadowPassPipeline before CascadedShadowPassPipelineLayout");

        // pipelineConfig.colorBlendInfo.attachmentCount = 0;

        // pipelineConfig.renderPass = m_CascadedShadowPass.RenderPass;
        // pipelineConfig.pipelineLayout = m_CascadedShadowPassPipelineLayout;
        // pipelineConfig.multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        // pipelineConfig.rasterizationInfo.cullMode = VK_CULL_MODE_BACK_BIT;

        // m_CascadedShadowPassPipeline = std::make_unique<VyPipeline>(m_Device, "Assets/Shaders/CascadedShadowPass.vert.spv", "Assets/Shaders/CascadedShadowPass.frag.spv", pipelineConfig);
    }

    // =====================================================================================================================

    void SimpleRenderSystem::prepareShadowPassRenderpass()
    {
        VkAttachmentDescription attachmentDescription{};
        {
            attachmentDescription.format         = m_ShadowPassImageFormat;
            attachmentDescription.samples        = VK_SAMPLE_COUNT_1_BIT;
            attachmentDescription.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;  // Clear depth at beginning of the render pass
            attachmentDescription.storeOp        = VK_ATTACHMENT_STORE_OP_STORE; // We will read from depth, so it's important to store the depth attachment results
            attachmentDescription.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachmentDescription.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;                      // We don't care about initial layout of the attachment
            attachmentDescription.finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL; // Attachment will be transitioned to shader read at render pass end
        }

        VkAttachmentReference depthReference{};
        {
            depthReference.attachment = 0;
            depthReference.layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;// Attachment will be used as depth/stencil during render pass
        }

        VkSubpassDescription subpass{};
        {
            subpass.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
            subpass.colorAttachmentCount    = 0;				// No color attachments
            subpass.pDepthStencilAttachment = &depthReference;	// Reference to our depth attachment
        }

        // Use subpass dependencies for layout transitions
        TArray<VkSubpassDependency, 2> dependencies;
        {
            dependencies[0].srcSubpass      = VK_SUBPASS_EXTERNAL;
            dependencies[0].dstSubpass      = 0;
            dependencies[0].srcStageMask    = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            dependencies[0].dstStageMask    = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
            dependencies[0].srcAccessMask   = VK_ACCESS_SHADER_READ_BIT;
            dependencies[0].dstAccessMask   = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
            
            dependencies[1].srcSubpass      = 0;
            dependencies[1].dstSubpass      = VK_SUBPASS_EXTERNAL;
            dependencies[1].srcStageMask    = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
            dependencies[1].dstStageMask    = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            dependencies[1].srcAccessMask   = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            dependencies[1].dstAccessMask   = VK_ACCESS_SHADER_READ_BIT;
            dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
        }

        VkRenderPassCreateInfo renderPassCreateInfo{ VKInit::renderPassCreateInfo() };
        {
            renderPassCreateInfo.attachmentCount = 1;
            renderPassCreateInfo.pAttachments    = &attachmentDescription;
            renderPassCreateInfo.subpassCount    = 1;
            renderPassCreateInfo.pSubpasses      = &subpass;
            renderPassCreateInfo.dependencyCount = static_cast<U32>(dependencies.size());
            renderPassCreateInfo.pDependencies   = dependencies.data();
        }

        if (vkCreateRenderPass(VyContext::device(), &renderPassCreateInfo, nullptr, &m_ShadowPass.RenderPass) != VK_SUCCESS) 
        {
            VY_THROW_RUNTIME_ERROR("failed to create offscreen render pass!");
        }
    }

    // =====================================================================================================================

    void SimpleRenderSystem::prepareShadowPassFramebuffer()
    {
        m_ShadowPass.Width  = m_ShadowMapSize;
        m_ShadowPass.Height = m_ShadowMapSize;

        // For shadow mapping we only need a depth attachment
        m_ShadowPass.ShadowMapImage.Image = VyImage::Builder{}
            .imageType  (VK_IMAGE_TYPE_2D)
            .format     (m_ShadowPassImageFormat) // Depth stencil attachment
            .extent     (m_ShadowPass.Width, m_ShadowPass.Height)
            .mipLevels  (1)
            .arrayLayers(1)
            .sampleCount(VK_SAMPLE_COUNT_1_BIT)
            .tiling     (VK_IMAGE_TILING_OPTIMAL)
            // We will sample directly from the depth attachment for the shadow mapping
            .usage      (VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
            .memoryUsage(VMA_MEMORY_USAGE_AUTO)
        .build();

        m_ShadowPass.ShadowMapImage.View = VyImageView::Builder{}
            .viewType   (VK_IMAGE_VIEW_TYPE_2D)
            .format     (m_ShadowPassImageFormat)
            .aspectMask (VK_IMAGE_ASPECT_DEPTH_BIT)
            .mipLevels  (0, 1)
            .arrayLayers(0, 1)
        .build(m_ShadowPass.ShadowMapImage.Image);

        // Create sampler to sample from to depth attachment
        // Used to sample in the fragment shader for shadowed rendering
        //VkFilter shadowmap_filter = vks::tools::formatIsFilterable(physicalDevice, offscreenDepthFormat, VK_IMAGE_TILING_OPTIMAL) ? VK_FILTER_LINEAR : VK_FILTER_NEAREST;

        m_ShadowPass.ShadowMapSampler = VySampler::Builder{}
            .filters         (VK_FILTER_LINEAR)
            .mipmapMode      (VK_SAMPLER_MIPMAP_MODE_LINEAR)
            .addressMode     (VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE)
            .borderColor     (VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE)
            .lodRange        (0.0f, 1.0f)
            .mipLodBias      (0.0f)
        .build();

        prepareShadowPassRenderpass();

        // Create frame buffer
        VkFramebufferCreateInfo framebufferInfo{ VKInit::framebufferCreateInfo() };
        {
            framebufferInfo.renderPass      = m_ShadowPass.RenderPass;
            framebufferInfo.attachmentCount = 1;
            framebufferInfo.pAttachments    = &m_ShadowPass.ShadowMapImage.View.handleRef();
            framebufferInfo.width           = m_ShadowPass.Width;
            framebufferInfo.height          = m_ShadowPass.Height;
            framebufferInfo.layers          = 1;
        }

        if (vkCreateFramebuffer(VyContext::device(), &framebufferInfo, nullptr, &m_ShadowPass.Framebuffer) != VK_SUCCESS) 
        {
            VY_THROW_RUNTIME_ERROR("failed to create offscreen depth frame buffer!");
        }
    }

    // =====================================================================================================================

    void SimpleRenderSystem::prepareShadowPassUBO()
    {
        //Direction Light Shadow Pass
        //m_ShadowPassBuffer = MakeUnique<VyBuffer>( VyBuffer::uniformBuffer(sizeof(ShadowPassUBO)), false);
        //m_ShadowPassBuffer->map();

        // Cascaded Shadow Map
        m_CascadedShadowPassBuffer = MakeUnique<VyBuffer>( VyBuffer::uniformBuffer(sizeof(CascadedShadowPassUBO)), false);
        m_CascadedShadowPassBuffer->map();

        //Point Light Shadow Pass
        m_PointShadowPassBuffer = MakeUnique<VyBuffer>( VyBuffer::uniformBuffer(sizeof(PointShadowPassViewMatrixUBO)), false);
        m_PointShadowPassBuffer->map();

        float aspect = (float)m_PointShadowMapSize / (float)m_PointShadowMapSize;
        Mat4  proj   = glm::perspective(glm::radians(90.0f), aspect, 0.1f, 25.0f);

        for (int i = 0; i < 6; i++)
        {
            Mat4 view = Mat4(1.0f);
            Vec3 pos  = Vec3(0.0f);

            switch (i)
            {
            case 0: // POSITIVE_X
                view = glm::lookAt(pos, pos + Vec3(1.0, 0.0, 0.0), Vec3(0.0, -1.0, 0.0));
                break;
            case 1:	// NEGATIVE_X
                view = glm::lookAt(pos, pos + Vec3(-1.0, 0.0, 0.0), Vec3(0.0, -1.0, 0.0));
                break;
            case 2:	// POSITIVE_Y
                view = glm::lookAt(pos, pos + Vec3(0.0, 1.0, 0.0), Vec3(0.0, 0.0, 1.0));
                break;
            case 3:	// NEGATIVE_Y
                view = glm::lookAt(pos, pos + Vec3(0.0, -1.0, 0.0), Vec3(0.0, 0.0, -1.0));
                break;
            case 4:	// POSITIVE_Z
                view = glm::lookAt(pos, pos + Vec3(0.0, 0.0, 1.0), Vec3(0.0, -1.0, 0.0));
                break;
            case 5:	// NEGATIVE_Z
                view = glm::lookAt(pos, pos + Vec3(0.0, 0.0, -1.0), Vec3(0.0, -1.0, 0.0));
                break;
            }

            m_PointShadowPassUBO.FaceViewMatrices[i] = proj * view;
        }

        m_PointShadowPassBuffer->writeToBuffer(&m_PointShadowPassUBO);
        m_PointShadowPassBuffer->flush();

        //Spot Shadow Pass
        m_SpotShadowPassBuffer = MakeUnique<VyBuffer>( VyBuffer::uniformBuffer(sizeof(ShadowPassUBO), MAX_SPOT_LIGHTS), false);
        m_SpotShadowPassBuffer->map();

        m_SpotShadowLightProjectionsBuffer = MakeUnique<VyBuffer>( VyBuffer::uniformBuffer(sizeof(SpotShadowLightProjectionsUBO)), false);
        m_SpotShadowLightProjectionsBuffer->map();
    }

    // =====================================================================================================================

    void SimpleRenderSystem::updateShadowPassBuffer(GlobalUBO& globalUBO)
    {
        Mat4 orthgonalProjection = glm::ortho(-30.0f, 30.0f, -30.0f, 30.0f, 0.1f, 100.0f);
        Mat4 lightView = glm::lookAt(Vec3(-globalUBO.DirectionalLights[0].Direction), Vec3(0.0f, 0.0f, 0.0f), Vec3(0.0f, 1.0f, 0.0f));
        
        m_ShadowPassUBO.LightProjection = orthgonalProjection * lightView;

        m_ShadowPassBuffer->writeToBuffer(&m_ShadowPassUBO);
        m_ShadowPassBuffer->flush();
    }

    // =====================================================================================================================

    void SimpleRenderSystem::prepareCascadeShadowPass()
    {
        VkFormat depthFormat = VyContext::device().findDepthFormat();

        // Render Pass
        VkAttachmentDescription attachmentDescription{};
        {
            attachmentDescription.format         = depthFormat;
            attachmentDescription.samples        = VK_SAMPLE_COUNT_1_BIT;
            attachmentDescription.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
            attachmentDescription.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
            attachmentDescription.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachmentDescription.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
            attachmentDescription.finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
        }

        VkAttachmentReference depthReference{};
        {
            depthReference.attachment = 0;
            depthReference.layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        }

        VkSubpassDescription subpass{};
        {
            subpass.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
            subpass.colorAttachmentCount    = 0;
            subpass.pDepthStencilAttachment = &depthReference;
        }

        TArray<VkSubpassDependency, 2> dependencies;
        {
            dependencies[0].srcSubpass      = VK_SUBPASS_EXTERNAL;
            dependencies[0].dstSubpass      = 0;
            dependencies[0].srcStageMask    = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            dependencies[0].dstStageMask    = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
            dependencies[0].srcAccessMask   = VK_ACCESS_SHADER_READ_BIT;
            dependencies[0].dstAccessMask   = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
            
            dependencies[1].srcSubpass      = 0;
            dependencies[1].dstSubpass      = VK_SUBPASS_EXTERNAL;
            dependencies[1].srcStageMask    = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
            dependencies[1].dstStageMask    = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            dependencies[1].srcAccessMask   = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            dependencies[1].dstAccessMask   = VK_ACCESS_SHADER_READ_BIT;
            dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
        }

        VkRenderPassCreateInfo renderPassCreateInfo{ VKInit::renderPassCreateInfo() };
        {
            renderPassCreateInfo.attachmentCount = 1;
            renderPassCreateInfo.pAttachments    = &attachmentDescription;
            renderPassCreateInfo.subpassCount    = 1;
            renderPassCreateInfo.pSubpasses      = &subpass;
            renderPassCreateInfo.dependencyCount = static_cast<U32>(dependencies.size());
            renderPassCreateInfo.pDependencies   = dependencies.data();
        }

        vkCreateRenderPass(VyContext::device(), &renderPassCreateInfo, nullptr, &m_CascadedShadowPass.RenderPass);

        // Main Depth Map Imag and View
        m_CascadedDepthMapObject.Image = VyImage::Builder{}
            .imageType  (VK_IMAGE_TYPE_2D)
            .format     (depthFormat)
            .extent     (m_CascadedShadowMapSize, m_CascadedShadowMapSize)
            .mipLevels  (1)
            .arrayLayers(CASCADE_SHADOW_MAP_COUNT)
            .sampleCount(VK_SAMPLE_COUNT_1_BIT)
            .tiling     (VK_IMAGE_TILING_OPTIMAL)
            .usage      (VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
            .memoryUsage(VMA_MEMORY_USAGE_AUTO)
        .build();

        m_CascadedDepthMapObject.View = VyImageView::Builder{}
            .viewType   (VK_IMAGE_VIEW_TYPE_2D)
            .format     (depthFormat)
            .aspectMask (VK_IMAGE_ASPECT_DEPTH_BIT)
            .mipLevels  (0, 1)
            .arrayLayers(0, CASCADE_SHADOW_MAP_COUNT)
        .build(m_CascadedDepthMapObject.Image);

        // Framebuffer and image view per cascade
        for (U32 i = 0; i < CASCADE_SHADOW_MAP_COUNT; i++) 
        {
            // Image view for this cascade's layer (inside the depth map)
            // This view is used to render to that specific depth image layer
            m_CascadedShadowPass.Cascades[i].View = VyImageView::Builder{}
                .viewType   (VK_IMAGE_VIEW_TYPE_2D_ARRAY)
                .format     (depthFormat)
                .aspectMask (VK_IMAGE_ASPECT_DEPTH_BIT)
                .mipLevels  (0, 1)
                .arrayLayers(i, 1)
            .build(m_CascadedDepthMapObject.Image);

            // Framebuffer
            VkFramebufferCreateInfo framebufferInfo{ VKInit::framebufferCreateInfo() };
            {
                framebufferInfo.renderPass      = m_CascadedShadowPass.RenderPass;
                framebufferInfo.attachmentCount = 1;
                framebufferInfo.pAttachments    = &m_CascadedShadowPass.Cascades[i].View.handleRef();
                framebufferInfo.width           = m_CascadedShadowMapSize;
                framebufferInfo.height          = m_CascadedShadowMapSize;
                framebufferInfo.layers          = 1;
            }

            vkCreateFramebuffer(VyContext::device(), &framebufferInfo, nullptr, &m_CascadedShadowPass.Cascades[i].Framebuffer);
        }

        // Sampler for caseded depth map reading
        // Shared sampler for cascade depth reads
        m_CascadedDepthMapObject.Sampler = VySampler::Builder{}
            .filters         (VK_FILTER_LINEAR)
            .mipmapMode      (VK_SAMPLER_MIPMAP_MODE_LINEAR)
            .addressMode     (VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE)
            .borderColor     (VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE)
            .lodRange        (0.0f, 1.0f)
            .mipLodBias      (0.0f)
        .build();
    }

    // =====================================================================================================================

    void SimpleRenderSystem::updateCascades(GlobalUBO& ubo)
    {
        float cascadeSplits[CASCADE_SHADOW_MAP_COUNT];

        float nearClip  = 0.1f;
        float farClip   = 100.0f;
        float clipRange = farClip - nearClip;

        float minZ = nearClip;
        float maxZ = nearClip + clipRange;

        float range = maxZ - minZ;
        float ratio = maxZ / minZ;

        float cascadeSplitLambda = 0.000001f;

        // Calculate split depths based on view camera frustum
        // Based on method presented in https://developer.nvidia.com/gpugems/GPUGems3/gpugems3_ch10.html
        for (U32 i = 0; i < CASCADE_SHADOW_MAP_COUNT; i++) 
        {
            float p       = (i + 1) / static_cast<float>(CASCADE_SHADOW_MAP_COUNT);
            float log     = minZ * std::pow(ratio, p);
            float uniform = minZ + range * p;
            float d       = cascadeSplitLambda * (log - uniform) + uniform;

            cascadeSplits[i] = (d - nearClip) / clipRange;
        }

        // Calculate orthographic projection matrix for each cascade
        float lastSplitDist = 0.0;
        for (U32 i = 0; i < CASCADE_SHADOW_MAP_COUNT; i++) 
        {
            float splitDist = cascadeSplits[i];

            Vec3 frustumCorners[8] = {
                Vec3(-1.0f,  1.0f,  0.0f),
                Vec3( 1.0f,  1.0f,  0.0f),
                Vec3( 1.0f, -1.0f,  0.0f),
                Vec3(-1.0f, -1.0f,  0.0f),
                Vec3(-1.0f,  1.0f,  1.0f),
                Vec3( 1.0f,  1.0f,  1.0f),
                Vec3( 1.0f, -1.0f,  1.0f),
                Vec3(-1.0f, -1.0f,  1.0f),
            };

            // Project frustum corners into world space
            Mat4 invCam = glm::inverse(ubo.CameraData.Projection * ubo.CameraData.View);
            for (U32 j = 0; j < 8; j++) 
            {
                Vec4 invCorner = invCam * Vec4(frustumCorners[j], 1.0f);
            
                frustumCorners[j] = invCorner / invCorner.w;
            }

            for (U32 j = 0; j < 4; j++) 
            {
                Vec3 dist = frustumCorners[j + 4] - frustumCorners[j];
            
                frustumCorners[j + 4] = frustumCorners[j] + (dist * splitDist);
                frustumCorners[j]     = frustumCorners[j] + (dist * lastSplitDist);
            }

            // Get frustum center
            Vec3 frustumCenter = Vec3(0.0f);
            for (U32 j = 0; j < 8; j++) 
            {
                frustumCenter += frustumCorners[j];
            }
            frustumCenter /= 8.0f;

            float radius = 0.0f;
            for (U32 j = 0; j < 8; j++) 
            {
                float distance = glm::length(frustumCorners[j] - frustumCenter);
            
                radius = glm::max(radius, distance);
            }
            radius = std::ceil(radius * 16.0f) / 16.0f;

            Vec3 maxExtents = Vec3(radius);
            Vec3 minExtents = -maxExtents;

            Vec3 lightDir         = glm::normalize(ubo.DirectionalLights[0].Direction);
            Mat4 lightViewMatrix  = glm::lookAt(frustumCenter - lightDir * -minExtents.z, frustumCenter, Vec3(0.0f, 1.0f, 0.0f));
            Mat4 lightOrthoMatrix = glm::ortho(minExtents.x, maxExtents.x, minExtents.y, maxExtents.y, 0.0f, maxExtents.z - minExtents.z);

            // Store split distance and matrix in cascade
            m_CascadedShadowPass.UBO.SplitDepths[i]      = (nearClip + splitDist * clipRange) * -1.0f;
            m_CascadedShadowPass.UBO.ViewProjMatrices[i] = lightOrthoMatrix * lightViewMatrix;

            lastSplitDist = cascadeSplits[i];
        }
    }

    // =====================================================================================================================

    void SimpleRenderSystem::preparePointShadowCubeMaps()
    {
        m_PointShadowCubeMaps.Width  = m_PointShadowMapSize;
        m_PointShadowCubeMaps.Height = m_PointShadowMapSize;

        // Cube map image description
        m_PointShadowCubeMaps.CubeMapImage.Image = VyImage::Builder{}
            .imageType  (VK_IMAGE_TYPE_2D)
            .format     (m_PointShadowPassImageFormat)
            .extent     (m_PointShadowCubeMaps.Width, m_PointShadowCubeMaps.Height)
            .mipLevels  (1)
            .arrayLayers(6 * MAX_POINT_LIGHTS)
            .sampleCount(VK_SAMPLE_COUNT_1_BIT)
            .tiling     (VK_IMAGE_TILING_OPTIMAL)
            .usage      (VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
            .sharingMode(VK_SHARING_MODE_EXCLUSIVE)
            .imageLayout(VK_IMAGE_LAYOUT_UNDEFINED)
            .createFlags(VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT)
            .memoryUsage(VMA_MEMORY_USAGE_AUTO)
        .build();

        // Image barrier for optimal image (target)
        // VkImageSubresourceRange subresourceRange{};
        // {
        //     subresourceRange.aspectMask   = VK_IMAGE_ASPECT_COLOR_BIT;
        //     subresourceRange.baseMipLevel = 0;
        //     subresourceRange.levelCount   = 1;
        //     subresourceRange.layerCount   = 6 * MAX_POINT_LIGHTS;
        // }

        VyContext::device().transitionImageLayout(
            m_PointShadowCubeMaps.CubeMapImage.Image,
            m_PointShadowPassImageFormat,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            0,
            6 * MAX_POINT_LIGHTS,
            1
        );

        // Create sampler next
        m_PointShadowCubeMaps.CubeMapSampler = VySampler::Builder{}
            .filters         (VK_FILTER_LINEAR)
            .mipmapMode      (VK_SAMPLER_MIPMAP_MODE_LINEAR)
            .addressMode     (VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER)
            .borderColor     (VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE)
            // .enableCompare   (VK_COMPARE_OP_NEVER)
            .lodRange        (0.0f, 1.0f)
            .mipLodBias      (0.0f)
        .build();

        // Create image view
        auto viewBuilder = VyImageView::Builder{}
            .viewType   (VK_IMAGE_VIEW_TYPE_CUBE_ARRAY)
            .format     (m_PointShadowPassImageFormat)
            .components (VK_COMPONENT_SWIZZLE_R)
            .aspectMask (VK_IMAGE_ASPECT_COLOR_BIT)
            .mipLevels  (0, 1)
            .arrayLayers(0, 6 * MAX_POINT_LIGHTS);

        m_PointShadowCubeMaps.CubeMapImage.View = viewBuilder.build(m_PointShadowCubeMaps.CubeMapImage.Image);

        viewBuilder.viewType(VK_IMAGE_VIEW_TYPE_2D);
        viewBuilder.arrayLayers(0, 1);

        for (int i = 0; i < MAX_POINT_LIGHTS; i++)
        {
            for (U32 j = 0; j < 6; j++)
            {
                viewBuilder.baseLayer(j + (6 * i));

                m_PointShadowCubeMapImageViews[i][j] = viewBuilder.build(m_PointShadowCubeMaps.CubeMapImage.Image);
            }
        }
    }

    // =====================================================================================================================

    void SimpleRenderSystem::preparePointShadowPassRenderPass()
    {
        m_PointShadowPassDepthFormat = VyContext::device().findDepthFormat();

        VkAttachmentDescription attachmentDescriptions[2] = {};
        {
            attachmentDescriptions[0].format         = m_PointShadowPassImageFormat;
            attachmentDescriptions[0].samples        = VK_SAMPLE_COUNT_1_BIT;
            attachmentDescriptions[0].loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
            attachmentDescriptions[0].storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
            attachmentDescriptions[0].stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachmentDescriptions[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachmentDescriptions[0].initialLayout  = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            attachmentDescriptions[0].finalLayout    = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            
            // Depth attachment
            attachmentDescriptions[1].format         = m_PointShadowPassDepthFormat;
            attachmentDescriptions[1].samples        = VK_SAMPLE_COUNT_1_BIT;
            attachmentDescriptions[1].loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
            attachmentDescriptions[1].storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
            attachmentDescriptions[1].stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachmentDescriptions[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachmentDescriptions[1].initialLayout  = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            attachmentDescriptions[1].finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        }

        VkAttachmentReference colorReference{};
        {
            colorReference.attachment = 0;
            colorReference.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        }

        VkAttachmentReference depthReference{};
        {
            depthReference.attachment = 1;
            depthReference.layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        }

        VkSubpassDescription subpass{};
        {
            subpass.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
            subpass.colorAttachmentCount    = 1;
            subpass.pColorAttachments       = &colorReference;
            subpass.pDepthStencilAttachment = &depthReference;
        }

        VkRenderPassCreateInfo renderPassCreateInfo{ VKInit::renderPassCreateInfo() };
        {
            renderPassCreateInfo.attachmentCount = 2;
            renderPassCreateInfo.pAttachments    = attachmentDescriptions;
            renderPassCreateInfo.subpassCount    = 1;
            renderPassCreateInfo.pSubpasses      = &subpass;
        }

        vkCreateRenderPass(VyContext::device(), &renderPassCreateInfo, nullptr, &m_PointShadowPass.RenderPass);
    }

    // =====================================================================================================================

    void SimpleRenderSystem::preparePointShadowPassFramebuffers()
    {
        m_PointShadowPass.Width  = m_PointShadowMapSize;
        m_PointShadowPass.Height = m_PointShadowMapSize;

        // Depth stencil attachment
        m_PointShadowPass.PointShadowMapImage.Image = VyImage::Builder{}
            .imageType  (VK_IMAGE_TYPE_2D)
            .format     (m_PointShadowPassDepthFormat)
            .extent     (m_PointShadowPass.Width, m_PointShadowPass.Height)
            .mipLevels  (1)
            .arrayLayers(1)
            .sampleCount(VK_SAMPLE_COUNT_1_BIT)
            .tiling     (VK_IMAGE_TILING_OPTIMAL)
            .sharingMode(VK_SHARING_MODE_EXCLUSIVE)
            // Image of the framebuffer is blit source
            .imageLayout(VK_IMAGE_LAYOUT_UNDEFINED)
            .usage      (VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT)
            .memoryUsage(VMA_MEMORY_USAGE_AUTO)
        .build();

        auto viewBuilder = VyImageView::Builder{}
            .viewType   (VK_IMAGE_VIEW_TYPE_2D)
            .format     (m_PointShadowPassDepthFormat)
            .aspectMask (VK_IMAGE_ASPECT_DEPTH_BIT)
            .mipLevels  (0, 1)
            .arrayLayers(0, 1);

        if (m_PointShadowPassDepthFormat >= VK_FORMAT_D16_UNORM_S8_UINT) 
        {
            viewBuilder.aspectMask(VK_IMAGE_ASPECT_STENCIL_BIT, /*exclusive*/ false);
        }

        VyContext::device().transitionImageLayout(
            m_PointShadowPass.PointShadowMapImage.Image,
            m_PointShadowPassDepthFormat,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
        );

        // m_Device.TransitionImageLayout(
        //     layoutCmd,
        //     m_PointShadowPass.PointShadowMapImage.Image,
        //     VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT,
        //     VK_IMAGE_LAYOUT_UNDEFINED,
        //     VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        //     VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
        //     VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

        m_PointShadowPass.PointShadowMapImage.View = viewBuilder.build(m_PointShadowPass.PointShadowMapImage.Image);

        VkImageView attachments[2];
        attachments[1] = m_PointShadowPass.PointShadowMapImage.View;

        VkFramebufferCreateInfo framebufferInfo{ VKInit::framebufferCreateInfo() };
        {
            framebufferInfo.renderPass      = m_PointShadowPass.RenderPass;
            framebufferInfo.attachmentCount = 2;
            framebufferInfo.pAttachments    = attachments;
            framebufferInfo.width           = m_PointShadowPass.Width;
            framebufferInfo.height          = m_PointShadowPass.Height;
            framebufferInfo.layers          = 1;
        }

        for (int i = 0; i < MAX_POINT_LIGHTS; i++)
        {
            for (U32 j = 0; j < 6; j++)
            {
                attachments[0] = m_PointShadowCubeMapImageViews[i][j];

                if (vkCreateFramebuffer(VyContext::device(), &framebufferInfo, nullptr, &m_PointShadowPass.Framebuffers[i][j]))
                {
                    VY_THROW_RUNTIME_ERROR("failed to create Point Shadow depth stencil image viewy");
                }
            }
        }
    }

    // =====================================================================================================================

    void SimpleRenderSystem::updateCubeFace(U32 faceIndex, VyFrameInfo frameInfo, GlobalUBO& ubo)
    {
        VkClearValue clearValues[2];
        {
            clearValues[0].color        = { { 0.0f, 0.0f, 0.0f, 1.0f } };
            clearValues[1].depthStencil = { 1.0f, 0 };
        }

        VkRenderPassBeginInfo renderPassBeginInfo{ VKInit::renderPassBeginInfo() };
        {
            renderPassBeginInfo.renderPass               = m_PointShadowPass.RenderPass;
            renderPassBeginInfo.framebuffer              = m_PointShadowPass.Framebuffers[m_PointLightCount][faceIndex];
            renderPassBeginInfo.renderArea.extent.width  = m_PointShadowPass.Width;
            renderPassBeginInfo.renderArea.extent.height = m_PointShadowPass.Height;
            renderPassBeginInfo.clearValueCount          = 2;
            renderPassBeginInfo.pClearValues             = clearValues;
        }

        // Render scene from cube face's point of view
        vkCmdBeginRenderPass(frameInfo.CommandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
        {
            m_PointShadowPassPipeline->bind(frameInfo.CommandBuffer);
            
            TVector<VkDescriptorSet> globSet = { frameInfo.GlobalDescriptorSet, m_PointShadowPassDescriptorSet };

            m_PointShadowPassPipeline->bindDescriptorSets(frameInfo.CommandBuffer, 0, globSet);
            
            renderGameObjects(frameInfo, m_PointShadowPassPipeline->layout(), PushConstantType::POINTSHADOW, globSet.size(), false);
        }
        vkCmdEndRenderPass(frameInfo.CommandBuffer);
    }

    // =====================================================================================================================

    void SimpleRenderSystem::prepareSpotShadowMaps()
    {
        m_SpotShadowMaps.Width  = m_SpotShadowMapSize;
        m_SpotShadowMaps.Height = m_SpotShadowMapSize;

        // Cube map image description
        m_SpotShadowMaps.CubeMapImage.Image = VyImage::Builder{}
            .imageType  (VK_IMAGE_TYPE_2D)
            .format     (m_SpotShadowPassImageFormat)
            .extent     (m_SpotShadowMaps.Width, m_SpotShadowMaps.Height)
            .mipLevels  (1)
            .arrayLayers(MAX_SPOT_LIGHTS)
            .sampleCount(VK_SAMPLE_COUNT_1_BIT)
            .tiling     (VK_IMAGE_TILING_OPTIMAL)
            .usage      (VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
            .sharingMode(VK_SHARING_MODE_EXCLUSIVE)
            .imageLayout(VK_IMAGE_LAYOUT_UNDEFINED)
            // .createFlags(VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT)
            .memoryUsage(VMA_MEMORY_USAGE_AUTO)
        .build();


        // Image barrier for optimal image (target)
        // VkImageSubresourceRange subresourceRange = {};
        // subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        // subresourceRange.baseMipLevel = 0;
        // subresourceRange.levelCount = 1;
        // subresourceRange.layerCount = MAX_SPOT_LIGHTS;

        VyContext::device().transitionImageLayout(
            m_SpotShadowMaps.CubeMapImage.Image,
            m_SpotShadowPassImageFormat,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            0,
            MAX_SPOT_LIGHTS,
            1
        );

        // Create sampler next
        m_PointShadowCubeMaps.CubeMapSampler = VySampler::Builder{}
            .filters         (VK_FILTER_LINEAR)
            .mipmapMode      (VK_SAMPLER_MIPMAP_MODE_LINEAR)
            .addressMode     (VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE)
            .borderColor     (VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE)
            // .enableCompare   (VK_COMPARE_OP_NEVER)
            .enableAnisotropy(true)
            .lodRange        (0.0f, 1.0f)
            .mipLodBias      (0.0f)
        .build();

        // Create image view
        auto viewBuilder = VyImageView::Builder{}
            .viewType   (VK_IMAGE_VIEW_TYPE_2D_ARRAY)
            .format     (m_SpotShadowPassImageFormat)
            .components (VK_COMPONENT_SWIZZLE_R)
            .aspectMask (VK_IMAGE_ASPECT_COLOR_BIT)
            .mipLevels  (0, 1)
            .arrayLayers(0, MAX_SPOT_LIGHTS);

        m_SpotShadowMaps.CubeMapImage.View = viewBuilder.build(m_SpotShadowMaps.CubeMapImage.Image);

        viewBuilder.viewType(VK_IMAGE_VIEW_TYPE_2D);
        viewBuilder.arrayLayers(0, 1);

        for (int i = 0; i < MAX_SPOT_LIGHTS; i++)
        {
            viewBuilder.baseLayer(i);

            m_SpotShadowPass.ImageViews[i] = viewBuilder.build(m_SpotShadowMaps.CubeMapImage.Image);
        }
    }

    // =====================================================================================================================

    void SimpleRenderSystem::prepareSpotShadowPassRenderPass()
    {
        m_SpotShadowPassDepthFormat = VyContext::device().findDepthFormat();

        VkAttachmentDescription attachmentDescriptions[2] = {};
        {
            attachmentDescriptions[0].format = m_SpotShadowPassImageFormat;
            attachmentDescriptions[0].samples = VK_SAMPLE_COUNT_1_BIT;
            attachmentDescriptions[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            attachmentDescriptions[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            attachmentDescriptions[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachmentDescriptions[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachmentDescriptions[0].initialLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            attachmentDescriptions[0].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            
            // Depth attachment
            attachmentDescriptions[1].format = m_SpotShadowPassDepthFormat;
            attachmentDescriptions[1].samples = VK_SAMPLE_COUNT_1_BIT;
            attachmentDescriptions[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            attachmentDescriptions[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            attachmentDescriptions[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachmentDescriptions[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachmentDescriptions[1].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            attachmentDescriptions[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        }

        VkAttachmentReference colorReference{};
        {
            colorReference.attachment = 0;
            colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        }

        VkAttachmentReference depthReference{};
        {
            depthReference.attachment = 1;
            depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        }

        VkSubpassDescription subpass{};
        {
            subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
            subpass.colorAttachmentCount = 1;
            subpass.pColorAttachments = &colorReference;
            subpass.pDepthStencilAttachment = &depthReference;
        }

        VkRenderPassCreateInfo renderPassCreateInfo{ VKInit::renderPassCreateInfo() };
        {
            renderPassCreateInfo.attachmentCount = 2;
            renderPassCreateInfo.pAttachments    = attachmentDescriptions;
            renderPassCreateInfo.subpassCount    = 1;
            renderPassCreateInfo.pSubpasses      = &subpass;
        }

        vkCreateRenderPass(VyContext::device(), &renderPassCreateInfo, nullptr, &m_SpotShadowPass.RenderPass);
    }

    // =====================================================================================================================

    void SimpleRenderSystem::prepareSpotShadowPassFramebuffers()
    {

        m_SpotShadowPass.Width  = m_SpotShadowMapSize;
        m_SpotShadowPass.Height = m_SpotShadowMapSize;

        m_SpotShadowPass.SpotShadowMapImage.Image = VyImage::Builder{}
            .imageType  (VK_IMAGE_TYPE_2D)
            .format     (m_SpotShadowPassDepthFormat)
            .extent     (m_SpotShadowPass.Width, m_SpotShadowPass.Height)
            .mipLevels  (1)
            .arrayLayers(1)
            .sampleCount(VK_SAMPLE_COUNT_1_BIT)
            .tiling     (VK_IMAGE_TILING_OPTIMAL)
            .sharingMode(VK_SHARING_MODE_EXCLUSIVE)
            // Image of the framebuffer is blit source
            .imageLayout(VK_IMAGE_LAYOUT_UNDEFINED)
            .usage      (VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT)
            .memoryUsage(VMA_MEMORY_USAGE_AUTO)
        .build();

        // VkImageViewCreateInfo colorImageView{};
        // colorImageView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        // colorImageView.viewType = VK_IMAGE_VIEW_TYPE_2D;
        // colorImageView.format = m_SpotShadowPassImageFormat;
        // colorImageView.flags = 0;
        // colorImageView.subresourceRange = {};
        // colorImageView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        // colorImageView.subresourceRange.baseMipLevel = 0;
        // colorImageView.subresourceRange.levelCount = 1;
        // colorImageView.subresourceRange.baseArrayLayer = 0;
        // colorImageView.subresourceRange.layerCount = 1;

        m_SpotShadowPass.SpotShadowMapImage.View = VyImageView::Builder{}
            .viewType   (VK_IMAGE_VIEW_TYPE_2D)
            .format     (m_SpotShadowPassDepthFormat)
            .aspectMask (VK_IMAGE_ASPECT_DEPTH_BIT)
            .mipLevels  (0, 1)
            .arrayLayers(0, 1)
        .build(m_SpotShadowPass.SpotShadowMapImage.Image);

        auto viewBuilder = VyImageView::Builder{}
            .viewType   (VK_IMAGE_VIEW_TYPE_2D)
            .format     (m_SpotShadowPassDepthFormat)
            .aspectMask (VK_IMAGE_ASPECT_DEPTH_BIT)
            .mipLevels  (0, 1)
            .arrayLayers(0, 1);

        if (m_PointShadowPassDepthFormat >= VK_FORMAT_D16_UNORM_S8_UINT) 
        {
            viewBuilder.aspectMask(VK_IMAGE_ASPECT_STENCIL_BIT, /*exclusive*/ false);
        }

        VyContext::device().transitionImageLayout(
            m_SpotShadowPass.SpotShadowMapImage.Image,
            m_SpotShadowPassDepthFormat,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
        );

        // m_Device.TransitionImageLayout(
        //     layoutCmd,
        //     m_SpotShadowPass.SpotShadowMapImage.Image,
        //     VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT,
        //     VK_IMAGE_LAYOUT_UNDEFINED,
        //     VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        //     VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
        //     VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

        m_SpotShadowPass.SpotShadowMapImage.View = viewBuilder.build(m_SpotShadowPass.SpotShadowMapImage.Image);

        VkImageView attachments[2];
        attachments[1] = m_SpotShadowPass.SpotShadowMapImage.View;

        VkFramebufferCreateInfo framebufferInfo{ VKInit::framebufferCreateInfo() };
        {
            framebufferInfo.renderPass      = m_SpotShadowPass.RenderPass;
            framebufferInfo.attachmentCount = 2;
            framebufferInfo.pAttachments    = attachments;
            framebufferInfo.width           = m_SpotShadowPass.Width;
            framebufferInfo.height          = m_SpotShadowPass.Height;
            framebufferInfo.layers          = 1;
        }

        for (int i = 0; i < MAX_SPOT_LIGHTS; i++)
        {
            attachments[0] = m_SpotShadowPass.ImageViews[i];

            if (vkCreateFramebuffer(VyContext::device(), &framebufferInfo, nullptr, &m_SpotShadowPass.Framebuffers[i]))
            {
                VY_THROW_RUNTIME_ERROR("failed to create Point Shadow depth stencil image viewy");
            }
        }
    }

    // =====================================================================================================================

    void SimpleRenderSystem::updateSpotShadowMaps(U32 lightIndex, VyFrameInfo frameInfo, GlobalUBO& ubo)
    {
        ShadowPassUBO sUbo;
        SpotLightUBO  light = ubo.SpotLights[lightIndex];
        Mat4 proj = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 1000.0f);
        Mat4 view = glm::lookAt(
            Vec3(light.Position.x, light.Position.y, light.Position.z), 
            Vec3(light.Position.x, light.Position.y, light.Position.z) + Vec3(light.Direction.x, light.Direction.y, light.Direction.z), 
            Vec3(0.0f, 0.0f, 1.0f)
        );
        
        sUbo.LightProjection = proj * view;
        m_SpotShadowLightProjectionsUBO.LightProjections[lightIndex] = sUbo.LightProjection;

        m_SpotShadowPassBuffer->writeToIndex(&sUbo, lightIndex);
        m_SpotShadowPassBuffer->flushIndex(lightIndex);

        VkClearValue clearValues[2];
        {
            clearValues[0].color        = { { 0.0f, 0.0f, 0.0f, 1.0f } };
            clearValues[1].depthStencil = { 1.0f, 0 };
        }

        VkRenderPassBeginInfo renderPassBeginInfo{ VKInit::renderPassBeginInfo() };
        {
            renderPassBeginInfo.renderPass               = m_SpotShadowPass.RenderPass;
            renderPassBeginInfo.framebuffer              = m_SpotShadowPass.Framebuffers[lightIndex];
            renderPassBeginInfo.renderArea.extent.width  = m_SpotShadowPass.Width;
            renderPassBeginInfo.renderArea.extent.height = m_SpotShadowPass.Height;
            renderPassBeginInfo.clearValueCount          = 2;
            renderPassBeginInfo.pClearValues             = clearValues;
        }

        // Render scene from cube face's point of view
        vkCmdBeginRenderPass(frameInfo.CommandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
        {
            m_SpotShadowPassPipeline->bind(frameInfo.CommandBuffer);
            
            TVector<VkDescriptorSet> globSet = { frameInfo.GlobalDescriptorSet, m_SpotShadowPassDescriptorSet };
            TVector<U32> dynamicOffset = { static_cast<U32>(lightIndex * m_SpotShadowPassBuffer->alignmentSize()) };

            m_SpotShadowPassPipeline->bindDescriptorSets(frameInfo.CommandBuffer, 0, globSet, dynamicOffset.size(), dynamicOffset.data());
            
            renderGameObjects(frameInfo, m_SpotShadowPassPipeline->layout(), PushConstantType::SPOTSHADOW, globSet.size(), false);
        }
        vkCmdEndRenderPass(frameInfo.CommandBuffer);
    }

    // =====================================================================================================================

    void SimpleRenderSystem::renderSpotShadowPass(VyFrameInfo frameInfo, GlobalUBO& ubo)
    {
        VKCmd::viewport(frameInfo.CommandBuffer, VkExtent2D{ m_SpotShadowPass.Width, m_SpotShadowPass.Height });
        VKCmd::scissor (frameInfo.CommandBuffer, VkExtent2D{ m_SpotShadowPass.Width, m_SpotShadowPass.Height });

        for (U32 i = 0; i < ubo.NumSpotLights; i++)
        {
            m_SpotLightIndex = i;

            updateSpotShadowMaps(i, frameInfo, ubo);
        }
    }
}