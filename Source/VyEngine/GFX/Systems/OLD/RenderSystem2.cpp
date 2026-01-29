// #include <VyEngine/GFX/Systems/OLD/RenderSystem2.h>

// #include <VyEngine/VK/Context.h>
// #include <VyEngine/Globals.h>

// #include <VyEngine/GFX/Resources/Texture/Texture.h>
// #include <VyEngine/Scene/ECS/Components.h>

// #include <iostream>
// #include <VyLib/Common/AnsiColor.h>

// namespace Vy
// {
//     struct MainPushConstantData
//     {
//         Mat4 modelMatrix{ 1.0f };
//         Mat4 normalMatrix{ 1.0f };
//     };

//     struct PointShadowPassPushConstantData
//     {
//         Mat4 modelMatrix{ 1.0f };
//         int lightCount{};
//         int faceCount{};
//     };

//     struct SpotShadowPassPushConstantData
//     {
//         Mat4 modelMatrix{ 1.0f };
//         int lightCount{};
//     };

//     struct CascadedShadowPassPushConstantData
//     {
//         Mat4 modelMatrix{ 1.0f };
//         int cascadeIndex{};
//     };


//     VyRenderSystem2::VyRenderSystem2(VkRenderPass renderPass, TVector<VkDescriptorSetLayout> setLayouts, VyDescriptorPool& descriptorPool)
//     {
//         // prepareShadowPassUBO();

//         // preparePointShadowCubeMaps();
//         // preparePointShadowPassRenderPass();
//         // preparePointShadowPassFramebuffers();

//         // prepareSpotShadowMaps();
//         // prepareSpotShadowPassRenderPass();
//         // prepareSpotShadowPassFramebuffers();

//         // prepareCascadeShadowPass();

//         createPipeline(renderPass, setLayouts, descriptorPool);
//     }

//     VyRenderSystem2::~VyRenderSystem2()
//     {
//         // Cleanup framebuffers.
//         // for (U32 j = 0; j < CASCADE_SHADOW_MAP_COUNT; j++) 
//         // {
//         //     vkDestroyFramebuffer(VyContext::device(), m_CascadedShadowPass.Cascades[j].Framebuffer, nullptr);
//         // }

//         // for (int i = 0; i < MAX_POINT_LIGHTS; i++)
//         // {
//         //     for (U32 j = 0; j < 6; j++)
//         //     {
//         //         vkDestroyFramebuffer(VyContext::device(), m_PointShadowPass.Framebuffers[ i ][ j ], nullptr);
//         //     }
//         // }
        
//         // for (int i = 0; i < MAX_SPOT_LIGHTS; i++)
//         // {
//         //     vkDestroyFramebuffer(VyContext::device(), m_SpotShadowPass.Framebuffers[ i ], nullptr);
//         // }

//         // // Cleanup renderpasses.
//         // vkDestroyRenderPass(VyContext::device(), m_ShadowPass.RenderPass, nullptr);
//         // vkDestroyRenderPass(VyContext::device(), m_SpotShadowPass.RenderPass, nullptr);
//         // vkDestroyRenderPass(VyContext::device(), m_PointShadowPass.RenderPass, nullptr);
//         // vkDestroyRenderPass(VyContext::device(), m_CascadedShadowPass.RenderPass, nullptr);

//     }


//     void VyRenderSystem2::renderCascadedShadowPass(VyFrameInfo frameInfo, GlobalUbo& globalUBO)
//     {
//         updateCascades(globalUBO);

//         m_CascadedShadowPassBuffer->write(&m_CascadedShadowPass.UBO, sizeof(CascadedShadowPassUBO), 0);

//         VkClearValue clearValues[1];
//         {
//             clearValues[0].depthStencil = { 1.0f, 0 };
//         }

//         VkExtent2D extent{ m_CascadedShadowMapSize, m_CascadedShadowMapSize };

//         VkRenderPassBeginInfo passBeginInfo{ VKInit::renderPassBeginInfo() };
//         {
//             passBeginInfo.renderPass        = m_CascadedShadowPass.RenderPass;
//             passBeginInfo.renderArea.offset = { 0, 0 };
//             passBeginInfo.renderArea.extent = extent;
//             passBeginInfo.clearValueCount   = 1;
//             passBeginInfo.pClearValues      = clearValues;
//         }

//         VKCmd::viewport( frameInfo.CommandBuffer, extent );
//         VKCmd::scissor ( frameInfo.CommandBuffer, extent );

//         TVector<VkDescriptorSet> globSet = { 
//             frameInfo.GlobalSet, 
//             m_CascadedShadowPassDescriptorSet 
//         };
        
//         m_CascadedShadowPassPipeline->bindDescriptorSets(frameInfo.CommandBuffer,
//             0, 
//             globSet
//         );

//         // One pass per cascade
//         // The layer that this pass renders to is defined by the cascade's image view (selected via the cascade's Descriptor set)
//         for (U32 j = 0; j < CASCADE_SHADOW_MAP_COUNT; j++) 
//         {
//             passBeginInfo.framebuffer = m_CascadedShadowPass.Cascades[ j ].Framebuffer;

//             vkCmdBeginRenderPass(frameInfo.CommandBuffer, &passBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
//             {
//                 m_CascadeIndex = j;
                
//                 m_CascadedShadowPassPipeline->bind(frameInfo.CommandBuffer);
                
//                 renderObjects(frameInfo, PushConstantType::CASCADEDSHADOW, globSet.size(), false);
//             }
//             vkCmdEndRenderPass(frameInfo.CommandBuffer);
//         }
//     }


//     void VyRenderSystem2::renderPointShadowPass(VyFrameInfo frameInfo, GlobalUbo& globalUBO)
//     {
//         // PROFILE_FUNCTION();
//         VKCmd::viewport( frameInfo.CommandBuffer, m_PointShadowPass.Extent );
//         VKCmd::scissor ( frameInfo.CommandBuffer, m_PointShadowPass.Extent );

//         for (U32 i = 0; i < globalUBO.PointLightsCount; i++)
//         {
//             m_PointLightCount = i;
            
//             for (U32 face = 0; face < 6; face++) 
//             {
//                 m_FaceCount = face;

//                 updateCubeFace(face, frameInfo, globalUBO);
//             }
//         }
//     }


//     void VyRenderSystem2::renderMainPass(VyFrameInfo frameInfo)
//     {
//         // PROFILE_FUNCTION();
//         // m_SpotShadowLightProjectionsBuffer->write(&m_SpotShadowLightProjectionsUBO, sizeof(SpotShadowLightProjectionsUBO), 0);

//         m_MainPipeline->bind(frameInfo.CommandBuffer);

//         TVector<VkDescriptorSet> globSet = { 
//             frameInfo.GlobalSet //, 
//             // m_CascadedShadowPassDescriptorSet, 
//             // m_CascadedShadowMapDescSet,
//             // m_PointShadowMapDescSet,
//             // m_SpotShadowMapDescSet
//         };

//         m_MainPipeline->bindDescriptorSets(frameInfo.CommandBuffer, 
//             0, 
//             globSet
//         );

//         renderObjects(frameInfo, PushConstantType::MAIN, globSet.size(), true);
//     }


//     void VyRenderSystem2::renderObjects(VyFrameInfo frameInfo, PushConstantType type, int setCount, bool bRenderMaterial)
//     {
//         auto view = frameInfo.Scene->registry().view<ModelComponent, TransformComponent>();
        
//         for (auto&& [ entity, modelComp, transform ] : view.each())
//         {
//             if (type == VyRenderSystem2::MAIN)
//             {
//                 MainPushConstantData data{};
//                 {
//                     data.modelMatrix  = transform.modelMatrix();
//                     data.normalMatrix = transform.normalMatrix();
//                 }

//                 m_MainPipeline->pushConstants(frameInfo.CommandBuffer, 
//                     VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 
//                     &data, 
//                     sizeof(data), 0
//                 );

//                 modelComp.Model->bind(frameInfo.CommandBuffer);
//                 modelComp.Model->draw(frameInfo.CommandBuffer, m_MainPipeline->layout(), setCount, bRenderMaterial);
//             }
            
//             // else if(type == VyRenderSystem2::POINTSHADOW)
//             // {
//             //     PointShadowPassPushConstantData data{};
//             //     {
//             //         data.modelMatrix = transform.modelMatrix();
//             //         data.lightCount  = m_PointLightCount;
//             //         data.faceCount   = m_FaceCount;
//             //     }

//             //     m_PointShadowPassPipeline->pushConstants(frameInfo.CommandBuffer, 
//             //         VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 
//             //         &data, 
//             //         sizeof(data), 0
//             //     );

//             //     modelComp.Model->bind(frameInfo.CommandBuffer);
//             //     modelComp.Model->draw(frameInfo.CommandBuffer, m_PointShadowPassPipeline->layout(), setCount, bRenderMaterial);
//             // }

//             // else if (type == VyRenderSystem2::SPOTSHADOW)
//             // {
//             //     SpotShadowPassPushConstantData data{};
//             //     {
//             //         data.modelMatrix = transform.modelMatrix();
//             //         data.lightCount  = m_SpotLightIndex;
//             //     }
      
//             //     m_SpotShadowPassPipeline->pushConstants(frameInfo.CommandBuffer, 
//             //         VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 
//             //         &data, 
//             //         sizeof(data), 0
//             //     );
   
//             //     modelComp.Model->bind(frameInfo.CommandBuffer);
//             //     modelComp.Model->draw(frameInfo.CommandBuffer, m_SpotShadowPassPipeline->layout(), setCount, bRenderMaterial);
//             // }

//             // else if (type == VyRenderSystem2::CASCADEDSHADOW)
//             // {
//             //     CascadedShadowPassPushConstantData data{};
//             //     {
//             //         data.modelMatrix  = transform.modelMatrix();
//             //         data.cascadeIndex = m_CascadeIndex;
//             //     }

//             //     m_CascadedShadowPassPipeline->pushConstants(frameInfo.CommandBuffer, 
//             //         VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 
//             //         &data, 
//             //         sizeof(data), 0
//             //     );

//             //     modelComp.Model->bind(frameInfo.CommandBuffer);
//             //     modelComp.Model->draw(frameInfo.CommandBuffer, m_CascadedShadowPassPipeline->layout(), setCount, bRenderMaterial);
//             // }
//         }
//     }


//     void VyRenderSystem2::createPipeline(VkRenderPass renderPass, TVector<VkDescriptorSetLayout> setLayouts, VyDescriptorPool& descriptorPool)
//     {
//         TVector<VkDescriptorSetLayout> mainSetLayouts;
//         // TVector<VkDescriptorSetLayout> pointShadowPassSetLayouts;
//         // TVector<VkDescriptorSetLayout> spotShadowPassSetLayouts;
//         // TVector<VkDescriptorSetLayout> cascadedShadowPassSetLayouts;

//         mainSetLayouts              .push_back( setLayouts[ 0 ] );
//         // pointShadowPassSetLayouts   .push_back( setLayouts[ 0 ] );
//         // spotShadowPassSetLayouts    .push_back( setLayouts[ 0 ] );
//         // cascadedShadowPassSetLayouts.push_back( setLayouts[ 0 ] );

//         // ----------------------------------------------------------------------------------------

//         // Cascaded Shadow Pass
//         // auto cascadedShadowPassUBOLayout = VyDescriptorSetLayout::Builder{}
//         //     .setName    ("cascaded_shadow_pass_ubo")
//         //     .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
//         //     .buildPtr();

//         // // Cascaded Shadow Map descriptorSet
//         // auto cascadedShadowMapSetLayout = VyDescriptorSetLayout::Builder{}
//         //     .setName    ("cascaded_shadow_map")
//         //     .addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
//         //     .buildPtr();

//         // // Spot Shadow Map descriptorSet
//         // auto spotShadowMapDescriptorSetLayout = VyDescriptorSetLayout::Builder{}
//         //     .setName    ("spot_shadow_map")
//         //     .addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
//         //     .addBinding(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
//         //     .buildPtr();

//         // // Point Shadow Map descriptorSet
//         // auto pointShadowMapSetLayout = VyDescriptorSetLayout::Builder{}
//         //     .setName    ("point_shadow_map")
//         //     .addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
//         //     .buildPtr();

//         // // Point Shadow Pass Pipeline Layout
//         // auto pointShadowPassUBOLayout = VyDescriptorSetLayout::Builder{}
//         //     .setName    ("point_shadow_pass_ubo")
//         //     .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
//         //     .buildPtr();

//         // // Spot Shadow Map descriptorSet
//         // auto spotShadowMapDescriptorSetLayout = VyDescriptorSetLayout::Builder{}
//         //     .addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
//         //     .addBinding(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
//         //     .buildPtr();

//         // Spot Shadow Pass Pipeline Layout
//         // auto spotShadowPassUBOLayout = VyDescriptorSetLayout::Builder{}
//         //     .setName    ("spot_shadow_pass_ubo")
//         //     .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
//         //     .buildPtr();

//         // ----------------------------------------------------------------------------------------

//         // auto bufferInfoCas = m_CascadedShadowPassBuffer->descriptorBufferInfo();

//         // VyDescriptorWriter(*cascadedShadowPassUBOLayout, descriptorPool)
//         //     .writeBuffer(0, &bufferInfoCas)
//         //     .build(m_CascadedShadowPassDescriptorSet);

//         // cascadedShadowPassSetLayouts.push_back( cascadedShadowPassUBOLayout->handle() );

//         // ----------------------------------------------------------------------------------------

//         // VkDescriptorImageInfo cascadedshadowMapDescriptor{};
//         // {
//         //     cascadedshadowMapDescriptor.sampler     = m_CascadedDepthMap.Sampler.handle();
//         //     cascadedshadowMapDescriptor.imageView   = m_CascadedDepthMap.View.handle();
//         //     cascadedshadowMapDescriptor.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
//         // }

//         // VyDescriptorWriter(*cascadedShadowMapSetLayout, descriptorPool)
//         //     .writeImage(0, &cascadedshadowMapDescriptor)
//         //     .build( m_CascadedShadowMapDescSet );

//         // mainSetLayouts.push_back( cascadedShadowPassUBOLayout->handle() );
//         // mainSetLayouts.push_back( cascadedShadowMapSetLayout->handle()  );

//         // ----------------------------------------------------------------------------------------

//         // VkDescriptorImageInfo pointShadowMapDescriptor{};
//         // {
//         //     pointShadowMapDescriptor.sampler     = m_PointShadowCubeMaps.Sampler.handle();
//         //     pointShadowMapDescriptor.imageView   = m_PointShadowCubeMaps.Attachment.View.handle();
//         //     pointShadowMapDescriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
//         // }

//         // VyDescriptorWriter(*pointShadowMapSetLayout, descriptorPool)
//         //     .writeImage(0, &pointShadowMapDescriptor)
//         //     .build( m_PointShadowMapDescSet );

//         // mainSetLayouts.push_back( pointShadowMapSetLayout->handle() );

//         // ----------------------------------------------------------------------------------------

//         // VkDescriptorImageInfo spotShadowMapDescriptor{};
//         // {
//         //     spotShadowMapDescriptor.sampler = m_SpotShadowMaps.Sampler.handle();
//         //     spotShadowMapDescriptor.imageView = m_SpotShadowMaps.Attachment.View.handle();
//         //     spotShadowMapDescriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
//         // }

//         // auto bufferInfoTwo = m_SpotShadowLightProjectionsBuffer->descriptorBufferInfo();

//         // VyDescriptorWriter(*spotShadowMapDescriptorSetLayout, descriptorPool)
//         //     .writeImage(0, &spotShadowMapDescriptor)
//         //     .writeBuffer(1, &bufferInfoTwo)
//         //     .build(m_SpotShadowMapDescSet);

//         // mainSetLayouts.push_back( spotShadowMapDescriptorSetLayout->handle() );
//         mainSetLayouts.push_back( setLayouts[1] );

//         // ----------------------------------------------------------------------------------------

//         // auto pointBufferInfo = m_PointShadowPassBuffer->descriptorBufferInfo();

//         // VyDescriptorWriter(*pointShadowPassUBOLayout, descriptorPool)
//         //     .writeBuffer(0, &pointBufferInfo)
//         //     .build(m_PointShadowPassDescriptorSet);
        
//         // pointShadowPassSetLayouts.push_back(pointShadowPassUBOLayout->handle());

//         // ----------------------------------------------------------------------------------------

//         // auto spotBufferInfo = m_SpotShadowPassBuffer->descriptorBufferInfoForIndex(0); //check

//         // VyDescriptorWriter(*spotShadowPassUBOLayout, descriptorPool)
//         //     .writeBuffer(0, &spotBufferInfo)
//         //     .build(m_SpotShadowPassDescriptorSet);

//         // spotShadowPassSetLayouts.push_back(spotShadowPassUBOLayout->handle());

//         // ----------------------------------------------------------------------------------------
        
//         // Main Pipeline
//         {
//             auto builder = VyPipeline::GraphicsBuilder{};

//             builder.setName( "main" );
            
//             builder.addDescriptorSetLayouts( mainSetLayouts );
            
//             builder.addPushConstantRange(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(MainPushConstantData));
            
//             builder.addShaderStage(VK_SHADER_STAGE_VERTEX_BIT,   "Basic2.vert.spv");
//             builder.addShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT, "Basic2.frag.spv");
            
//             // Set multisampled.
//             builder.setRasterizationSamples( VyContext::device().msaaSampleCountFlagBits() );

//             builder.addColorAttachment(VK_FORMAT_R16G16B16A16_SFLOAT, false);
//             // builder.setDepthAttachment(VK_FORMAT_D32_SFLOAT);

//             builder.setCullMode(VK_CULL_MODE_FRONT_BIT);
//             // builder.setFrontFace(VK_FRONT_FACE_COUNTER_CLOCKWISE);
//             builder.setRenderPass( renderPass );
            
//             m_MainPipeline = builder.buildPtr();
//         }

//         // Point Shadow Pass Pipeline
//         // {
//         //     auto builder = VyPipeline::GraphicsBuilder{};

//         //     builder.setName( "point_shadow_pass" );
            
//         //     builder.addDescriptorSetLayouts( pointShadowPassSetLayouts );
            
//         //     builder.addPushConstantRange(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(PointShadowPassPushConstantData));
            
//         //     builder.addShaderStage(VK_SHADER_STAGE_VERTEX_BIT,   "PointShadowPass.vert.spv");
//         //     builder.addShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT, "PointShadowPass.frag.spv");
            
//         //     // Set to default multisample.
//         //     builder.setRasterizationSamples( VK_SAMPLE_COUNT_1_BIT );

//         //     builder.addColorAttachment(VK_FORMAT_R16G16B16A16_SFLOAT, false);
//         //     // builder.setDepthAttachment(VK_FORMAT_D32_SFLOAT);

//         //     builder.setCullMode(VK_CULL_MODE_FRONT_BIT);
//         //     // builder.setFrontFace(VK_FRONT_FACE_COUNTER_CLOCKWISE);
//         //     builder.setRenderPass( m_PointShadowPass.RenderPass );
            
//         //     m_PointShadowPassPipeline = builder.buildPtr();
//         // }

//         // // Spot Shadow Pass Pipeline
//         // {
//         //     auto builder = VyPipeline::GraphicsBuilder{};

//         //     builder.setName( "spot_shadow_pass" );
            
//         //     builder.addDescriptorSetLayouts( spotShadowPassSetLayouts );
            
//         //     builder.addPushConstantRange(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(SpotShadowPassPushConstantData));
            
//         //     builder.addShaderStage(VK_SHADER_STAGE_VERTEX_BIT,   "SpotShadowPass.vert.spv");
//         //     builder.addShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT, "SpotShadowPass.frag.spv");
            
//         //     // Set to default multisample.
//         //     builder.setRasterizationSamples( VK_SAMPLE_COUNT_1_BIT );

//         //     builder.addColorAttachment(VK_FORMAT_R16G16B16A16_SFLOAT, false);
//         //     // builder.setDepthAttachment(VK_FORMAT_D32_SFLOAT);

//         //     builder.setCullMode(VK_CULL_MODE_FRONT_BIT);
//         //     // builder.setFrontFace(VK_FRONT_FACE_COUNTER_CLOCKWISE);
//         //     builder.setRenderPass( m_SpotShadowPass.RenderPass );
            
//         //     m_SpotShadowPassPipeline = builder.buildPtr();
//         // }

//         // // Cascaded Shadow Pass Pipeline
//         // {
//         //     auto builder = VyPipeline::GraphicsBuilder{};

//         //     builder.setName( "cascaded_shadow_pass" );
            
//         //     builder.addDescriptorSetLayouts( cascadedShadowPassSetLayouts );
            
//         //     builder.addPushConstantRange(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(MainPushConstantData));
//         //     //, sizeof(CascadedShadowPassPushConstantData));
            
//         //     builder.addShaderStage(VK_SHADER_STAGE_VERTEX_BIT,   "CascadedShadowPass.vert.spv");
//         //     builder.addShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT, "Empty.frag.spv");
            
//         //     // Set to default multisample.
//         //     builder.setRasterizationSamples( VK_SAMPLE_COUNT_1_BIT );

//         //     builder.clearColorAttachments();

//         //     builder.setCullMode(VK_CULL_MODE_BACK_BIT);
//         //     // builder.setFrontFace(VK_FRONT_FACE_COUNTER_CLOCKWISE);
//         //     builder.setRenderPass( m_CascadedShadowPass.RenderPass );
            
//         //     m_CascadedShadowPassPipeline = builder.buildPtr();
//         // }
//     }


//     void VyRenderSystem2::prepareShadowPassUBO()
//     {
//         // Cascaded Shadow Map
//         m_CascadedShadowPassBuffer = std::make_unique<VyBuffer>( 
//             VyBuffer::uniformBuffer( "cascaded_shadow", sizeof(CascadedShadowPassUBO), 1 ) 
//         );

//         //Point Light Shadow Pass
//         m_PointShadowPassBuffer = std::make_unique<VyBuffer>( 
//             VyBuffer::uniformBuffer( "point_shadow", sizeof(PointShadowPassViewMatrixUBO), 1 ) 
//         );

//         float aspect = (float)m_PointShadowMapSize / (float)m_PointShadowMapSize;
//         Mat4  proj   = glm::perspective(glm::radians(90.0f), aspect, 0.1f, 25.0f);

//         for (int i = 0; i < 6; i++)
//         {
//             Mat4 view = Mat4(1.0f);
//             Vec3 pos  = Vec3(0.0f);

//             switch (i)
//             {
//             case 0: // POSITIVE_X
//                 view = glm::lookAt(pos, pos + Vec3(1.0, 0.0, 0.0), Vec3(0.0, -1.0, 0.0));
//                 break;
//             case 1:	// NEGATIVE_X
//                 view = glm::lookAt(pos, pos + Vec3(-1.0, 0.0, 0.0), Vec3(0.0, -1.0, 0.0));
//                 break;
//             case 2:	// POSITIVE_Y
//                 view = glm::lookAt(pos, pos + Vec3(0.0, 1.0, 0.0), Vec3(0.0, 0.0, 1.0));
//                 break;
//             case 3:	// NEGATIVE_Y
//                 view = glm::lookAt(pos, pos + Vec3(0.0, -1.0, 0.0), Vec3(0.0, 0.0, -1.0));
//                 break;
//             case 4:	// POSITIVE_Z
//                 view = glm::lookAt(pos, pos + Vec3(0.0, 0.0, 1.0), Vec3(0.0, -1.0, 0.0));
//                 break;
//             case 5:	// NEGATIVE_Z
//                 view = glm::lookAt(pos, pos + Vec3(0.0, 0.0, -1.0), Vec3(0.0, -1.0, 0.0));
//                 break;
//             }
        
//             m_PointShadowPassUBO.FaceViewMatrix[ i ] = proj * view;
//         }

//         m_PointShadowPassBuffer->write( &m_PointShadowPassUBO, sizeof(PointShadowPassViewMatrixUBO), 0 );

//         //Spot Shadow Pass
//         m_SpotShadowPassBuffer = std::make_unique<VyBuffer>( 
//             VyBuffer::uniformBuffer( "spot_shadow", sizeof(ShadowPassUBO), MAX_SPOT_LIGHTS ) 
//         );

//         m_SpotShadowLightProjectionsBuffer = std::make_unique<VyBuffer>( 
//             VyBuffer::uniformBuffer( "spot_shadow_light_proj", sizeof(SpotShadowLightProjectionsUBO), 1 ) 
//         );
//     }


//     void VyRenderSystem2::updateShadowPassBuffer(GlobalUbo& globalUBO)
//     {
//         Mat4 orthgonalProjection = glm::ortho(-30.0f, 30.0f, -30.0f, 30.0f, 0.1f, 100.0f);
//         Mat4 lightView           = glm::lookAt(Vec3(-globalUBO.DirectionalLight.Direction), Vec3(0.0f, 0.0f, 0.0f), Vec3(0.0f, 1.0f, 0.0f));
        
//         m_ShadowPassUBO.LightProjection = orthgonalProjection * lightView;

//         m_ShadowPassBuffer->write( &m_ShadowPassUBO, sizeof(ShadowPassUBO), 0 );
//     }

// #pragma endregion


// #pragma region Cascades

//     void VyRenderSystem2::prepareCascadeShadowPass()
//     {
//         VkFormat depthFormat = VyContext::device().findDepthFormat();

//         TVector<VkAttachmentDescription> attachments{};
//         {
//             // [ DEPTH - 0 ] 
//             auto attachment0{ VKInit::attachmentDescription() };
//             {
//                 attachment0.format         = depthFormat;
//                 attachment0.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
//                 attachment0.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
//                 attachment0.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
//                 attachment0.finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
//             }

//             attachments.push_back( attachment0 );
//         }

//         VkAttachmentReference depthReference;
//         {
//             depthReference = { 0, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };
//         }

//         TArray<VkSubpassDescription, 1> subpassDescriptions{};
//         {
//             subpassDescriptions[0].pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
//             subpassDescriptions[0].colorAttachmentCount    = 0;
//             subpassDescriptions[0].pColorAttachments       = nullptr;
//             subpassDescriptions[0].pDepthStencilAttachment = &depthReference;
//         }

//         // Subpass dependencies for layout transitions
//         TVector<VkSubpassDependency> dependencies{};
//         {
//             VkSubpassDependency dependency1{};
//             {
//                 dependency1.srcSubpass      = VK_SUBPASS_EXTERNAL;
//                 dependency1.dstSubpass      = 0;

//                 dependency1.srcStageMask    = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
//                 dependency1.dstStageMask    = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT; // VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | 
                
//                 dependency1.srcAccessMask   = VK_ACCESS_SHADER_READ_BIT;
//                 dependency1.dstAccessMask   = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                
//                 dependency1.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
//             }

//             dependencies.emplace_back( dependency1 );

//             VkSubpassDependency dependency2{};
//             {
//                 dependency2.srcSubpass      = 0;
//                 dependency2.dstSubpass      = VK_SUBPASS_EXTERNAL;

//                 dependency2.srcStageMask    = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
//                 dependency2.dstStageMask    = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                
//                 dependency2.srcAccessMask   = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
//                 dependency2.dstAccessMask   = VK_ACCESS_SHADER_READ_BIT;
                
//                 dependency2.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
//             }

//             dependencies.emplace_back( dependency2 );
//         }

//         VkRenderPassCreateInfo renderPassInfo{ VKInit::renderPassCreateInfo() };
//         {
//             renderPassInfo.attachmentCount = static_cast<U32>(attachments.size());
//             renderPassInfo.pAttachments    = attachments.data();

//             renderPassInfo.subpassCount    = subpassDescriptions.size();
//             renderPassInfo.pSubpasses      = subpassDescriptions.data();
            
//             renderPassInfo.dependencyCount = dependencies.size();
//             renderPassInfo.pDependencies   = dependencies.data();
//         }

//         VK_CHECK_SUCCESS(vkCreateRenderPass(VyContext::device(), &renderPassInfo, nullptr, &m_CascadedShadowPass.RenderPass), 
//             "Failed to create cascaded shadow render pass!"
//         );

//         VKDbg::setObjectName(m_CascadedShadowPass.RenderPass, "cascaded_shadow_renderpass");

//         VkExtent2D extent = { m_CascadedShadowMapSize, m_CascadedShadowMapSize };

//         // Main Depth Map Imag and View
//         auto dImgBuilder = VyImage::Builder{};
//         dImgBuilder.setName       ("cascade_shadow");
//         dImgBuilder.setImageType  (VK_IMAGE_TYPE_2D);
//         dImgBuilder.setFormat     (depthFormat);
//         dImgBuilder.setExtent     (extent);
//         dImgBuilder.setLevels     (1);
//         dImgBuilder.setLayers     ( CASCADE_SHADOW_MAP_COUNT );
//         dImgBuilder.setSamples    (VK_SAMPLE_COUNT_1_BIT);
//         dImgBuilder.setTiling     (VK_IMAGE_TILING_OPTIMAL);
//         dImgBuilder.setSharing    (VK_SHARING_MODE_EXCLUSIVE);
//         dImgBuilder.setLayout     (VK_IMAGE_LAYOUT_UNDEFINED);
//         dImgBuilder.setUsage      (VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
//         dImgBuilder.setMemoryUsage(VMA_MEMORY_USAGE_AUTO);

//         m_CascadedDepthMap.Image = dImgBuilder.build();

//         // Create image view
//         auto dViewBuilder = VyImageView::Builder{};
//         {
//             dViewBuilder.setName      ("cascade_shadow");
//             dViewBuilder.setViewType  (VK_IMAGE_VIEW_TYPE_2D_ARRAY);
//             dViewBuilder.setFormat    (depthFormat);
//             dViewBuilder.setAspect    (VK_IMAGE_ASPECT_DEPTH_BIT);
//             dViewBuilder.setLevels    (0, 1);
//             dViewBuilder.setLayers    (0, CASCADE_SHADOW_MAP_COUNT);
//         }

//         m_CascadedDepthMap.View = dViewBuilder.build( m_CascadedDepthMap.Image );

//         // Framebuffer and image view per cascade
//         for (U32 i = 0; i < CASCADE_SHADOW_MAP_COUNT; i++) 
//         {
//             // Image view for this cascade's layer (inside the depth map)
//             // This view is used to render to that specific depth image layer
//             auto cascadeBuilder = VyImageView::Builder{};
//             {
//                 cascadeBuilder.setName      (std::format("shadow_cascade_{}", i));
//                 cascadeBuilder.setViewType  (VK_IMAGE_VIEW_TYPE_2D_ARRAY);
//                 cascadeBuilder.setFormat    (depthFormat);
//                 cascadeBuilder.setAspect    (VK_IMAGE_ASPECT_DEPTH_BIT);
//                 cascadeBuilder.setLevels    (0, 1);
//                 cascadeBuilder.setLayers    (i, 1);
//             }

//             m_CascadedShadowPass.Cascades[ i ].View = cascadeBuilder.build( m_CascadedDepthMap.Image );

//             // Framebuffer
//             VkFramebufferCreateInfo framebufferInfo{ VKInit::framebufferCreateInfo() };
//             {
//                 framebufferInfo.renderPass      = m_CascadedShadowPass.RenderPass;
//                 framebufferInfo.attachmentCount = 1;
//                 framebufferInfo.pAttachments    = &m_CascadedShadowPass.Cascades[ i ].View.handle();
//                 framebufferInfo.width           = m_CascadedShadowMapSize;
//                 framebufferInfo.height          = m_CascadedShadowMapSize;
//                 framebufferInfo.layers          = 1;
//             }

//             vkCreateFramebuffer(VyContext::device(), &framebufferInfo, nullptr, &m_CascadedShadowPass.Cascades[ i ].Framebuffer);
//         }

//         // Sampler for caseded depth map reading
//         // Shared sampler for cascade depth reads
//         m_CascadedDepthMap.Sampler = VySampler::Builder{}
//             .setName         ("cascade_shadow")
//             .setFilters      (VK_FILTER_LINEAR)
//             .setMipmapMode   (VK_SAMPLER_MIPMAP_MODE_LINEAR)
//             .setWrap         (VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE)
//             .setBorder       (VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE)
//             .setLodRange     (0.0f, 1.0f)
//             .setMipLodBias   (0.0f)
//             .build();
//     }


//     void VyRenderSystem2::updateCascades(GlobalUbo& globalUBO)
//     {
//         float cascadeSplits[ CASCADE_SHADOW_MAP_COUNT ];

//         float nearClip  = 0.1f;
//         float farClip   = 100.0f;
//         float clipRange = farClip - nearClip;

//         float minZ = nearClip;
//         float maxZ = nearClip + clipRange;

//         float range = maxZ - minZ;
//         float ratio = maxZ / minZ;

//         float cascadeSplitLambda = 0.000001f;

//         // Calculate split depths based on view camera frustum
//         // Based on method presented in https://developer.nvidia.com/gpugems/GPUGems3/gpugems3_ch10.html
//         // https://github.com/NazaraEngine/NazaraEngine/blob/main/src/Nazara/Graphics/DirectionalLightShadowData.cpp
//         for (U32 i = 0; i < CASCADE_SHADOW_MAP_COUNT; i++) 
//         {
//             float p       = float(i + 1) / static_cast<float>(CASCADE_SHADOW_MAP_COUNT);
//             float log     = minZ * std::pow(ratio, p);
//             float uniform = minZ + range * p;
//             float d       = cascadeSplitLambda * (log - uniform) + uniform;

//             cascadeSplits[ i ] = (d - nearClip) / clipRange;
//         }

//         // Calculate orthographic projection matrix for each cascade
//         float lastSplitDist = 0.0;
//         for (U32 i = 0; i < CASCADE_SHADOW_MAP_COUNT; i++) 
//         {
//             float splitDist = cascadeSplits[ i ];

//             // Get the 8 points of the view frustum in world space
//             Vec3 frustumCorners[ 8 ] = {
//                 // Near Plane (Z = 0.0) 
//                 Vec3( -1.0f,  1.0f,  0.0f ), // TL Near
//                 Vec3(  1.0f,  1.0f,  0.0f ), // TR Near
//                 Vec3(  1.0f, -1.0f,  0.0f ), // BR Near
//                 Vec3( -1.0f, -1.0f,  0.0f ), // BL Near

//                 // Far Plane (Z = 1.0)
//                 Vec3( -1.0f,  1.0f,  1.0f ), // TL Far
//                 Vec3(  1.0f,  1.0f,  1.0f ), // TR Far
//                 Vec3(  1.0f, -1.0f,  1.0f ), // BR Far
//                 Vec3( -1.0f, -1.0f,  1.0f ), // BL Far
//             };

//             // Project frustum corners into world space
//             Mat4 invCam = glm::inverse( globalUBO.CameraData.Projection * globalUBO.CameraData.View );
            
//             for (U32 j = 0; j < 8; j++) 
//             {
//                 Vec4 invCorner = invCam * Vec4( frustumCorners[ j ], 1.0f );

//                 frustumCorners[ j ] = invCorner / invCorner.w;
//             }

//             for (U32 j = 0; j < 4; j++) 
//             {
//                 Vec3 dist = frustumCorners[j + 4] - frustumCorners[ j ];
            
//                 frustumCorners[ j + 4 ] = frustumCorners[ j ] + (dist * splitDist    );
//                 frustumCorners[ j     ] = frustumCorners[ j ] + (dist * lastSplitDist);
//             }

//             // Get frustum center
//             Vec3 frustumCenter = Vec3(0.0f);
//             for (U32 j = 0; j < 8; j++) 
//             {
//                 frustumCenter += frustumCorners[ j ];
//             }
            
//             frustumCenter /= 8.0f;

//             float radius = 0.0f;
//             for (U32 j = 0; j < 8; j++) 
//             {
//                 float distance = glm::length(frustumCorners[ j ] - frustumCenter);
            
//                 radius = glm::max(radius, distance);
//             }
//             radius = std::ceil(radius * 16.0f) / 16.0f;

//             Vec3 maxExtents = Vec3(radius);
//             Vec3 minExtents = -maxExtents;

//             Vec3 lightDir         = glm::normalize(globalUBO.DirectionalLight.Direction);
//             Mat4 lightViewMatrix  = glm::lookAt(frustumCenter - lightDir * -minExtents.z, frustumCenter, Vec3(0.0f, 1.0f, 0.0f));
//             Mat4 lightOrthoMatrix = glm::ortho(minExtents.x, maxExtents.x, minExtents.y, maxExtents.y, 0.0f, maxExtents.z - minExtents.z);

//             // Store split distance and matrix in cascade
//             m_CascadedShadowPass.UBO.SplitDepths [ i ] = (nearClip + splitDist * clipRange) * -1.0f;
//             m_CascadedShadowPass.UBO.ViewProjMats[ i ] = lightOrthoMatrix * lightViewMatrix;

//             lastSplitDist = cascadeSplits[ i ];
//         }
//     }

// #pragma endregion


// #pragma region PointShadow

//     void VyRenderSystem2::preparePointShadowCubeMaps()
//     {
//         m_PointShadowCubeMaps.Extent = { m_PointShadowMapSize, m_PointShadowMapSize };

//         // Cube map image description
//         auto cImgBuilder = VyImage::Builder{};
//         cImgBuilder.setName       ("point_shadow_cube");
//         cImgBuilder.setImageType  (VK_IMAGE_TYPE_2D);
//         cImgBuilder.setFormat     (m_PointShadowPassImageFormat);
//         cImgBuilder.setExtent     (m_PointShadowCubeMaps.Extent);
//         cImgBuilder.setLevels     (1);
//         cImgBuilder.setLayers     ( 6 * MAX_POINT_LIGHTS ); // ?? Correct ??
//         cImgBuilder.setSamples    (VK_SAMPLE_COUNT_1_BIT);
//         cImgBuilder.setTiling     (VK_IMAGE_TILING_OPTIMAL);
//         cImgBuilder.setSharing    (VK_SHARING_MODE_EXCLUSIVE);
//         cImgBuilder.setLayout     (VK_IMAGE_LAYOUT_UNDEFINED);
//         cImgBuilder.setUsage      (VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
//         cImgBuilder.setMemoryUsage(VMA_MEMORY_USAGE_AUTO);
//         cImgBuilder.setFlags      (VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT);

//         m_PointShadowCubeMaps.Attachment.Image = cImgBuilder.build();

//         // Image barrier for optimal image (target)
//         m_PointShadowCubeMaps.Attachment.Image.transitionLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

//         // Create sampler next
//         m_PointShadowCubeMaps.Sampler = VySampler::Builder{}
//             .setName         ("point_shadow_cube")
//             .setFilters      (VK_FILTER_LINEAR)
//             .setMipmapMode   (VK_SAMPLER_MIPMAP_MODE_LINEAR)
//             .setWrap         (VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER)
//             .setBorder       (VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE)
//             .setLodRange     (0.0f, 1.0f)
//             .setMipLodBias   (0.0f)
//             .build();


//         // Create image view
//         auto cViewBuilder = VyImageView::Builder{};
//         {
//             cViewBuilder.setName      ("point_shadow_cube");
//             cViewBuilder.setViewType  (VK_IMAGE_VIEW_TYPE_CUBE_ARRAY);
//             cViewBuilder.setFormat    (m_PointShadowPassImageFormat);
//             cViewBuilder.setComponents( VK_COMPONENT_SWIZZLE_R );
//             cViewBuilder.setAspect    (VK_IMAGE_ASPECT_COLOR_BIT);
//             cViewBuilder.setLevels    (0, 1);
//             cViewBuilder.setLayers    (0, 6 * MAX_POINT_LIGHTS);
//         }

//         m_PointShadowCubeMaps.Attachment.View = cViewBuilder.build( m_PointShadowCubeMaps.Attachment.Image );

//         cViewBuilder.setViewType(VK_IMAGE_VIEW_TYPE_2D);
//         cViewBuilder.setLayers  (0, 1);

//         for (U32 i = 0; i < MAX_POINT_LIGHTS; i++)
//         {
//             for (U32 j = 0; j < 6; j++)
//             {
//                 cViewBuilder.setName     (std::format("point_shadow_cube_light_{}_face_{}", i, j));
//                 cViewBuilder.setBaseLayer( j + (6 * i) );

//                 m_PointShadowCubeMapImageViews[ i ][ j ] = cViewBuilder.build( m_PointShadowCubeMaps.Attachment.Image );
//             }
//         }
//     }


//     void VyRenderSystem2::preparePointShadowPassRenderPass()
//     {
//         TVector<VkAttachmentDescription> attachments{};
//         {
//             // [ Color - 0 ] 
//             auto attachment0{ VKInit::attachmentDescription() };
//             {
//                 attachment0.format         = m_PointShadowPassImageFormat;
//                 attachment0.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
//                 attachment0.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
//                 attachment0.initialLayout  = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
//                 attachment0.finalLayout    = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
//             }

//             attachments.push_back( attachment0 );

//             m_PointShadowPassDepthFormat = VyContext::device().findDepthFormat();

//             // [ DEPTH - 1 ]
//             auto attachment1{ VKInit::attachmentDescription() };
//             {
//                 attachment1.format         = m_PointShadowPassDepthFormat;
//                 attachment1.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
//                 attachment1.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
//                 attachment1.initialLayout  = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
//                 attachment1.finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
//             }

//             attachments.push_back( attachment1 );
//         }

//         VkAttachmentReference colorReferences[ 1 ];
//         VkAttachmentReference depthReference;
//         {
//             colorReferences[0] = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
//             depthReference     = { 1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };
//         }

//         TArray<VkSubpassDescription, 1> subpassDescriptions{};
//         {
//             subpassDescriptions[0].pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
//             subpassDescriptions[0].colorAttachmentCount    = 1;
//             subpassDescriptions[0].pColorAttachments       = colorReferences;
//             subpassDescriptions[0].pDepthStencilAttachment = &depthReference;
//         }

//         VkRenderPassCreateInfo renderPassInfo{ VKInit::renderPassCreateInfo() };
//         {
//             renderPassInfo.attachmentCount = static_cast<U32>(attachments.size());
//             renderPassInfo.pAttachments    = attachments.data();

//             renderPassInfo.subpassCount    = subpassDescriptions.size();
//             renderPassInfo.pSubpasses      = subpassDescriptions.data();
            
//             renderPassInfo.dependencyCount = 0;
//             renderPassInfo.pDependencies   = nullptr;
//         }

//         VK_CHECK_SUCCESS(vkCreateRenderPass(VyContext::device(), &renderPassInfo, nullptr, &m_PointShadowPass.RenderPass), 
//             "Failed to create point shadow render pass!"
//         );

//         VKDbg::setObjectName(m_PointShadowPass.RenderPass, "point_shadow_renderpass");
//     }


//     void VyRenderSystem2::preparePointShadowPassFramebuffers()
//     {
//         m_PointShadowPass.Extent = { m_PointShadowMapSize, m_PointShadowMapSize };

//         // Depth stencil attachment
//         auto dImgBuilder = VyImage::Builder{};
//         dImgBuilder.setName       ("point_shadow_depth_attachment");
//         dImgBuilder.setImageType  (VK_IMAGE_TYPE_2D);
//         dImgBuilder.setFormat     (m_PointShadowPassDepthFormat);
//         dImgBuilder.setExtent     (m_PointShadowPass.Extent);
//         dImgBuilder.setLevels     (1);
//         dImgBuilder.setLayers     (1);
//         dImgBuilder.setSamples    (VK_SAMPLE_COUNT_1_BIT);
//         dImgBuilder.setTiling     (VK_IMAGE_TILING_OPTIMAL);
//         dImgBuilder.setSharing    (VK_SHARING_MODE_EXCLUSIVE);
//         dImgBuilder.setLayout     (VK_IMAGE_LAYOUT_UNDEFINED);
//         dImgBuilder.setUsage      (VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
//         dImgBuilder.setMemoryUsage(VMA_MEMORY_USAGE_AUTO);

//         auto dViewBuilder = VyImageView::Builder{};
//         {
//             dViewBuilder.setName    ("point_shadow_depth_attachment");
//             dViewBuilder.setViewType(VK_IMAGE_VIEW_TYPE_2D);
//             dViewBuilder.setFormat  (m_PointShadowPassDepthFormat);
//             dViewBuilder.setAspect  (VK_IMAGE_ASPECT_DEPTH_BIT);
//             dViewBuilder.setLevels  (0, 1);
//             dViewBuilder.setLayers  (0, 1);

//             if (m_PointShadowPassDepthFormat >= VK_FORMAT_D16_UNORM_S8_UINT)
//             {
//                 dViewBuilder.addAspect(VK_IMAGE_ASPECT_STENCIL_BIT);
//             }
//         }

//         m_PointShadowPass.Attachment.Image = dImgBuilder.build();
//         m_PointShadowPass.Attachment.Image.transitionLayout(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

//         m_PointShadowPass.Attachment.View  = dViewBuilder.build( m_PointShadowPass.Attachment.Image );

//         VkImageView attachments[2];
//         {
//             attachments[1] = m_PointShadowPass.Attachment.View.handle();
//         }

//         VkFramebufferCreateInfo framebufferInfo{ VKInit::framebufferCreateInfo() };
//         {
//             framebufferInfo.renderPass      = m_PointShadowPass.RenderPass;
//             framebufferInfo.attachmentCount = 2;
//             framebufferInfo.pAttachments    = attachments;
//             framebufferInfo.width           = m_PointShadowPass.Extent.width;
//             framebufferInfo.height          = m_PointShadowPass.Extent.height;
//             framebufferInfo.layers          = 1;
//         }

//         for (int i = 0; i < MAX_POINT_LIGHTS; i++)
//         {
//             for (U32 j = 0; j < 6; j++)
//             {
//                 attachments[0] = m_PointShadowCubeMapImageViews[ i ][ j ].handle();
                
//                 if (vkCreateFramebuffer(VyContext::device(), &framebufferInfo, nullptr, &m_PointShadowPass.Framebuffers[ i ][ j ]))
//                 {
//                     throw std::runtime_error("failed to create Point Shadow depth stencil image viewy");
//                 }
//             }
//         }
//     }


//     void VyRenderSystem2::updateCubeFace(U32 faceIndex, VyFrameInfo frameInfo, GlobalUbo& globalUBO)
//     {
//         VkClearValue clearValues[2];
//         {
//             clearValues[0].color        = { { 0.0f, 0.0f, 0.0f, 1.0f } };
//             clearValues[1].depthStencil = { 1.0f, 0 };
//         }

//         VkRenderPassBeginInfo passBeginInfo{ VKInit::renderPassBeginInfo() };
//         {
//             passBeginInfo.renderPass        = m_PointShadowPass.RenderPass;
//             passBeginInfo.framebuffer       = m_PointShadowPass.Framebuffers[ m_PointLightCount ][ faceIndex ];
//             passBeginInfo.renderArea.offset = { 0, 0 };
//             passBeginInfo.renderArea.extent = m_PointShadowPass.Extent;
//             passBeginInfo.clearValueCount   = 2;
//             passBeginInfo.pClearValues      = clearValues;
//         }

//         // Render scene from cube face's point of view
//         vkCmdBeginRenderPass(frameInfo.CommandBuffer, &passBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

//         m_PointShadowPassPipeline->bind(frameInfo.CommandBuffer);

//         TVector<VkDescriptorSet> globSet = { 
//             frameInfo.GlobalSet, 
//             m_PointShadowPassDescriptorSet 
//         };

//         m_PointShadowPassPipeline->bindDescriptorSets(frameInfo.CommandBuffer, 
//             0, 
//             globSet
//         );

//         renderObjects(frameInfo, PushConstantType::POINTSHADOW, globSet.size(), false);

//         vkCmdEndRenderPass(frameInfo.CommandBuffer);
//     }

// #pragma endregion


// #pragma region SpotShadow

//     void VyRenderSystem2::prepareSpotShadowMaps()
//     {
//         m_SpotShadowMaps.Extent = { m_SpotShadowMapSize, m_SpotShadowMapSize };

//         // Cube map image description
//         auto cImgBuilder = VyImage::Builder{};
//         cImgBuilder.setName       ("spot_shadow");
//         cImgBuilder.setImageType  (VK_IMAGE_TYPE_2D);
//         cImgBuilder.setFormat     (m_SpotShadowPassImageFormat);
//         cImgBuilder.setExtent     (m_SpotShadowMaps.Extent);
//         cImgBuilder.setLevels     (1);
//         cImgBuilder.setLayers     ( MAX_SPOT_LIGHTS ); // ?? Correct ??
//         cImgBuilder.setSamples    (VK_SAMPLE_COUNT_1_BIT);
//         cImgBuilder.setTiling     (VK_IMAGE_TILING_OPTIMAL);
//         cImgBuilder.setSharing    (VK_SHARING_MODE_EXCLUSIVE);
//         cImgBuilder.setLayout     (VK_IMAGE_LAYOUT_UNDEFINED);
//         cImgBuilder.setUsage      (VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
//         cImgBuilder.setMemoryUsage(VMA_MEMORY_USAGE_AUTO);

//         m_SpotShadowMaps.Attachment.Image = cImgBuilder.build();

//         // Image barrier for optimal image (target)
//         m_SpotShadowMaps.Attachment.Image.transitionLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

//         // Create sampler next
//         m_SpotShadowMaps.Sampler = VySampler::Builder{}
//             .setName         ("spot_shadow")
//             .setFilters      (VK_FILTER_LINEAR)
//             .setMipmapMode   (VK_SAMPLER_MIPMAP_MODE_LINEAR)
//             .setWrap         (VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE)
//             .setBorder       (VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE)
//             .enableAnisotropy(true)
//             .setLodRange     (0.0f, 1.0f)
//             .setMipLodBias   (0.0f)
//             .build();

//         // Create image view
//         auto cViewBuilder = VyImageView::Builder{};
//         {
//             cViewBuilder.setName      ("spot_shadow");
//             cViewBuilder.setViewType  (VK_IMAGE_VIEW_TYPE_2D_ARRAY);
//             cViewBuilder.setFormat    (m_SpotShadowPassImageFormat);
//             cViewBuilder.setAspect    (VK_IMAGE_ASPECT_COLOR_BIT);
//             cViewBuilder.setLevels    (0, 1);
//             cViewBuilder.setLayers    (0, MAX_SPOT_LIGHTS);
//             cViewBuilder.setComponents( VK_COMPONENT_SWIZZLE_R );
//         }

//         m_SpotShadowMaps.Attachment.View = cViewBuilder.build( m_SpotShadowMaps.Attachment.Image );

//         cViewBuilder.setViewType(VK_IMAGE_VIEW_TYPE_2D);
//         cViewBuilder.setLayers  (0, 1);

//         for (U32 i = 0; i < MAX_SPOT_LIGHTS; i++)
//         {
//             cViewBuilder.setBaseLayer( i );

//             m_SpotShadowPass.ImageViews[ i ] = cViewBuilder.build( m_SpotShadowMaps.Attachment.Image ).handle();
//         }
//     }


//     void VyRenderSystem2::prepareSpotShadowPassRenderPass()
//     {
//         TVector<VkAttachmentDescription> attachments{};
//         {
//             // [ Color - 0 ] 
//             auto attachment0{ VKInit::attachmentDescription() };
//             {
//                 attachment0.format         = m_SpotShadowPassImageFormat;
//                 attachment0.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
//                 attachment0.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
//                 attachment0.initialLayout  = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
//                 attachment0.finalLayout    = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
//             }

//             attachments.push_back( attachment0 );

//             m_SpotShadowPassDepthFormat = VyContext::device().findDepthFormat();

//             // [ DEPTH - 1 ]
//             auto attachment1{ VKInit::attachmentDescription() };
//             {
//                 attachment1.format         = m_SpotShadowPassDepthFormat;
//                 attachment1.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
//                 attachment1.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
//                 attachment1.initialLayout  = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
//                 attachment1.finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
//             }

//             attachments.push_back( attachment1 );
//         }

//         VkAttachmentReference colorReferences[ 1 ];
//         VkAttachmentReference depthReference;
//         {
//             colorReferences[0] = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
//             depthReference     = { 1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };
//         }

//         TArray<VkSubpassDescription, 1> subpassDescriptions{};
//         {
//             subpassDescriptions[0].pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
//             subpassDescriptions[0].colorAttachmentCount    = 1;
//             subpassDescriptions[0].pColorAttachments       = colorReferences;
//             subpassDescriptions[0].pDepthStencilAttachment = &depthReference;
//         }

//         VkRenderPassCreateInfo renderPassInfo{ VKInit::renderPassCreateInfo() };
//         {
//             renderPassInfo.attachmentCount = static_cast<U32>(attachments.size());
//             renderPassInfo.pAttachments    = attachments.data();

//             renderPassInfo.subpassCount    = subpassDescriptions.size();
//             renderPassInfo.pSubpasses      = subpassDescriptions.data();
            
//             renderPassInfo.dependencyCount = 0;
//             renderPassInfo.pDependencies   = nullptr;
//         }

//         VK_CHECK_SUCCESS(vkCreateRenderPass(VyContext::device(), &renderPassInfo, nullptr, &m_SpotShadowPass.RenderPass), 
//             "Failed to create spot shadow render pass!"
//         );

//         VKDbg::setObjectName(m_SpotShadowPass.RenderPass, "spot_shadow_renderpass");
//     }


//     void VyRenderSystem2::prepareSpotShadowPassFramebuffers()
//     {

//         m_SpotShadowPass.Extent = { m_SpotShadowMapSize, m_SpotShadowMapSize };

//         // Depth stencil attachment
//         auto dImgBuilder = VyImage::Builder{};
//         dImgBuilder.setName       ("depth_attachment");
//         dImgBuilder.setImageType  (VK_IMAGE_TYPE_2D);
//         dImgBuilder.setFormat     (m_SpotShadowPassDepthFormat);
//         dImgBuilder.setExtent     (m_SpotShadowPass.Extent);
//         dImgBuilder.setLevels     (1);
//         dImgBuilder.setLayers     (1);
//         dImgBuilder.setSamples    (VK_SAMPLE_COUNT_1_BIT);
//         dImgBuilder.setTiling     (VK_IMAGE_TILING_OPTIMAL);
//         dImgBuilder.setSharing    (VK_SHARING_MODE_EXCLUSIVE);
//         // Image of the framebuffer is blit source
//         dImgBuilder.setLayout     (VK_IMAGE_LAYOUT_UNDEFINED);
//         dImgBuilder.setUsage      (VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
//         dImgBuilder.setMemoryUsage(VMA_MEMORY_USAGE_AUTO);

//         auto dViewBuilder = VyImageView::Builder{};
//         {
//             dViewBuilder.setName    ("depth_attachment");
//             dViewBuilder.setViewType(VK_IMAGE_VIEW_TYPE_2D);
//             dViewBuilder.setFormat  (m_SpotShadowPassDepthFormat);
//             dViewBuilder.setAspect  (VK_IMAGE_ASPECT_DEPTH_BIT);
//             dViewBuilder.setLevels  (0, 1);
//             dViewBuilder.setLayers  (0, 1);

//             if (m_SpotShadowPassDepthFormat >= VK_FORMAT_D16_UNORM_S8_UINT)
//             {
//                 dViewBuilder.addAspect(VK_IMAGE_ASPECT_STENCIL_BIT);
//             }
//         }

//         m_SpotShadowPass.Attachment.Image = dImgBuilder.build();
//         m_SpotShadowPass.Attachment.Image.transitionLayout(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
        
//         m_SpotShadowPass.Attachment.View  = dViewBuilder.build( m_SpotShadowPass.Attachment.Image );

//         VkImageView attachments[2];
//         {
//             attachments[1] = m_SpotShadowPass.Attachment.View.handle();
//         }

//         VkFramebufferCreateInfo framebufferInfo{ VKInit::framebufferCreateInfo() };
//         {
//             framebufferInfo.renderPass      = m_SpotShadowPass.RenderPass;
//             framebufferInfo.attachmentCount = 2;
//             framebufferInfo.pAttachments    = attachments;
//             framebufferInfo.width           = m_SpotShadowPass.Extent.width;
//             framebufferInfo.height          = m_SpotShadowPass.Extent.height;
//             framebufferInfo.layers          = 1;
//         }

//         for (int i = 0; i < MAX_SPOT_LIGHTS; i++)
//         {
//             attachments[0] = m_SpotShadowPass.ImageViews[ i ];

//             if (vkCreateFramebuffer(VyContext::device(), &framebufferInfo, nullptr, &m_SpotShadowPass.Framebuffers[ i ]))
//             {
//                 throw std::runtime_error("failed to create Point Shadow depth stencil image viewy");
//             }
//         }
//     }


//     void VyRenderSystem2::updateSpotShadowMaps(U32 lightIndex, VyFrameInfo frameInfo, GlobalUbo& globalUBO)
//     {
//         ShadowPassUBO sUbo;
//         SpotLightUBO  light = globalUBO.SpotLights[ lightIndex ];

//         Mat4 proj = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 1000.0f);

//         Mat4 view = glm::lookAt(
//             Vec3(light.Position.x, light.Position.y, light.Position.z), 
//             Vec3(light.Position.x, light.Position.y, light.Position.z) + Vec3(light.Direction.x, light.Direction.y, light.Direction.z), 
//             Vec3(0.0f, 0.0f, 1.0f) // Up
//         );

//         sUbo.LightProjection = proj * view;
//         m_SpotShadowLightProjectionsUBO.LightProjections[lightIndex] = sUbo.LightProjection;

//         m_SpotShadowPassBuffer->writeToIndex(&sUbo, lightIndex);
//         m_SpotShadowPassBuffer->flushIndex(lightIndex);

//         VkClearValue clearValues[2];
//         {
//             clearValues[0].color        = {{ 0.0f, 0.0f, 0.0f, 1.0f }};
//             clearValues[1].depthStencil = { 1.0f, 0 };
//         }

//         VkRenderPassBeginInfo passBeginInfo{ VKInit::renderPassBeginInfo() };
//         {
//             passBeginInfo.renderPass        = m_SpotShadowPass.RenderPass;
//             passBeginInfo.framebuffer       = m_SpotShadowPass.Framebuffers[lightIndex];
//             passBeginInfo.renderArea.extent = m_SpotShadowPass.Extent;
//             passBeginInfo.clearValueCount   = 2;
//             passBeginInfo.pClearValues      = clearValues;
//         }

//         // Render scene from cube face's point of view
//         vkCmdBeginRenderPass(frameInfo.CommandBuffer, &passBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

//         m_SpotShadowPassPipeline->bind(frameInfo.CommandBuffer);

//         TVector<VkDescriptorSet> globSet = { 
//             frameInfo.GlobalSet, 
//             m_SpotShadowPassDescriptorSet 
//         };
        
//         TVector<U32> dynamicOffset = { 
//             static_cast<U32>(lightIndex * m_SpotShadowPassBuffer->alignmentSize()) 
//         };
        
//         m_SpotShadowPassPipeline->bindDescriptorSets(frameInfo.CommandBuffer, 
//             0, 
//             globSet, 
//             dynamicOffset.size(), dynamicOffset.data()
//         );

//         renderObjects(frameInfo, PushConstantType::SPOTSHADOW, globSet.size(), false);

//         vkCmdEndRenderPass(frameInfo.CommandBuffer);
//     }


//     void VyRenderSystem2::renderSpotShadowPass(VyFrameInfo frameInfo, GlobalUbo& globalUBO)
//     {
//         // PROFILE_FUNCTION();
//         VKCmd::viewport( frameInfo.CommandBuffer, m_SpotShadowPass.Extent );
//         VKCmd::scissor ( frameInfo.CommandBuffer, m_SpotShadowPass.Extent );

//         for (U32 i = 0; i < globalUBO.SpotLightsCount; i++)
//         {
//             m_SpotLightIndex = i;

//             updateSpotShadowMaps(i, frameInfo, globalUBO);
//         }
//     }

// #pragma endregion

// }