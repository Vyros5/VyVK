// #include <Vy/Systems/Rendering/MeshSystem.h>
// #include <Vy/Systems/Rendering/IBLSystem.h>
// #include <Vy/Systems/Rendering/ShadowSystem.h>
// #include <Vy/GFX/Resources/Texture.h>
// #include <Vy/GFX/Resources/PBRMaterial.h>

// #include <Vy/GFX/Context.h>

// namespace Vy
// {

//     struct MeshPushConstantData
//     {
//         Mat4 modelMatrix{1.0f};
//         Mat4 normalMatrix{1.0f};
//         U32  meshId{0};

//         U64  meshletBufferAddress;
//         U64  meshletVerticesAddress;
//         U64  meshletTrianglesAddress;
//         U64  vertexBufferAddress;
//         U32  meshletOffset;
//         U32  meshletCount;
//         Vec2 screenSize;
//     };

//     MeshRenderSystem::MeshRenderSystem(VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout, VkDescriptorSetLayout bindlessSetLayout)
//     {
//         createShadowDescriptorResources();
//         createIBLDescriptorResources();
//         createMaterialDescriptorResources();
//         createPipelineLayout(globalSetLayout, bindlessSetLayout);
//         createPipeline(renderPass);
//     }

//     MeshRenderSystem::~MeshRenderSystem()
//     {
//         vkDestroyPipelineLayout(VyContext::device(), pipelineLayout, nullptr);
//         if (m_ShadowDescriptorPool != VK_NULL_HANDLE)
//         {
//         vkDestroyDescriptorPool(VyContext::device(), m_ShadowDescriptorPool, nullptr);
//         }
//         if (m_ShadowDescriptorSetLayout != VK_NULL_HANDLE)
//         {
//         vkDestroyDescriptorSetLayout(VyContext::device(), m_ShadowDescriptorSetLayout, nullptr);
//         }
//         if (m_IBLDescriptorPool != VK_NULL_HANDLE)
//         {
//         vkDestroyDescriptorPool(VyContext::device(), m_IBLDescriptorPool, nullptr);
//         }
//         if (m_IBLDescriptorSetLayout != VK_NULL_HANDLE)
//         {
//         vkDestroyDescriptorSetLayout(VyContext::device(), m_IBLDescriptorSetLayout, nullptr);
//         }
//         if (m_MaterialDescriptorPool != VK_NULL_HANDLE)
//         {
//         vkDestroyDescriptorPool(VyContext::device(), m_MaterialDescriptorPool, nullptr);
//         }
//         if (m_MaterialDescriptorSetLayout != VK_NULL_HANDLE)
//         {
//         vkDestroyDescriptorSetLayout(VyContext::device(), m_MaterialDescriptorSetLayout, nullptr);
//         }
//     }

//     void MeshRenderSystem::createShadowDescriptorResources()
//     {
//         TArray<VkDescriptorSetLayoutBinding, 2> bindings{};
//         {
//             bindings[0].binding            = 0;
//             bindings[0].descriptorType     = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
//             bindings[0].descriptorCount    = VyShadowSystem::MAX_SHADOW_MAPS;
//             bindings[0].stageFlags         = VK_SHADER_STAGE_FRAGMENT_BIT;
//             bindings[0].pImmutableSamplers = nullptr;
            
//             bindings[1].binding            = 1;
//             bindings[1].descriptorType     = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
//             bindings[1].descriptorCount    = VyShadowSystem::MAX_CUBE_SHADOW_MAPS;
//             bindings[1].stageFlags         = VK_SHADER_STAGE_FRAGMENT_BIT;
//             bindings[1].pImmutableSamplers = nullptr;
//         }

//         VkDescriptorSetLayoutCreateInfo layoutInfo{};
//         {
//             layoutInfo.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
//             layoutInfo.bindingCount = static_cast<U32>(bindings.size());
//             layoutInfo.pBindings    = bindings.data();
//         }

//         if (vkCreateDescriptorSetLayout(VyContext::device(), &layoutInfo, nullptr, &m_ShadowDescriptorSetLayout) != VK_SUCCESS)
//         {
//             throw std::runtime_error("Failed to create shadow descriptor set layout");
//         }

//         TArray<VkDescriptorPoolSize, 2> poolSizes{};
//         {
//             poolSizes[0].type            = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
//             poolSizes[0].descriptorCount = static_cast<U32>(MAX_FRAMES_IN_FLIGHT * VyShadowSystem::MAX_SHADOW_MAPS);
//             poolSizes[1].type            = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
//             poolSizes[1].descriptorCount = static_cast<U32>(MAX_FRAMES_IN_FLIGHT * VyShadowSystem::MAX_CUBE_SHADOW_MAPS);
//         }

//         VkDescriptorPoolCreateInfo poolInfo{};
//         {
//             poolInfo.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
//             poolInfo.poolSizeCount = static_cast<U32>(poolSizes.size());
//             poolInfo.pPoolSizes    = poolSizes.data();
//             poolInfo.maxSets       = static_cast<U32>(MAX_FRAMES_IN_FLIGHT);
//         }

//         if (vkCreateDescriptorPool(VyContext::device(), &poolInfo, nullptr, &m_ShadowDescriptorPool) != VK_SUCCESS)
//         {
//             throw std::runtime_error("Failed to create shadow descriptor pool");
//         }

//         std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, m_ShadowDescriptorSetLayout);

//         VkDescriptorSetAllocateInfo allocInfo{};
//         {
//             allocInfo.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
//             allocInfo.descriptorPool     = m_ShadowDescriptorPool;
//             allocInfo.descriptorSetCount = static_cast<U32>(MAX_FRAMES_IN_FLIGHT);
//             allocInfo.pSetLayouts        = layouts.data();
//         }

//         m_ShadowDescriptorSets.resize(MAX_FRAMES_IN_FLIGHT);

//         if (vkAllocateDescriptorSets(VyContext::device(), &allocInfo, m_ShadowDescriptorSets.data()) != VK_SUCCESS)
//         {
//             throw std::runtime_error("Failed to allocate shadow descriptor sets");
//         }
//     }


//     void MeshRenderSystem::createIBLDescriptorResources()
//     {
//         TArray<VkDescriptorSetLayoutBinding, 3> bindings{};
//         {
//             bindings[0].binding            = 0;
//             bindings[0].descriptorType     = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
//             bindings[0].descriptorCount    = 1;
//             bindings[0].stageFlags         = VK_SHADER_STAGE_FRAGMENT_BIT;
//             bindings[0].pImmutableSamplers = nullptr;
            
//             bindings[1].binding            = 1;
//             bindings[1].descriptorType     = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
//             bindings[1].descriptorCount    = 1;
//             bindings[1].stageFlags         = VK_SHADER_STAGE_FRAGMENT_BIT;
//             bindings[1].pImmutableSamplers = nullptr;
            
//             bindings[2].binding            = 2;
//             bindings[2].descriptorType     = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
//             bindings[2].descriptorCount    = 1;
//             bindings[2].stageFlags         = VK_SHADER_STAGE_FRAGMENT_BIT;
//             bindings[2].pImmutableSamplers = nullptr;
//         }

//         VkDescriptorSetLayoutCreateInfo layoutInfo{};
//         {
//             layoutInfo.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
//             layoutInfo.bindingCount = static_cast<U32>(bindings.size());
//             layoutInfo.pBindings    = bindings.data();
//         }

//         if (vkCreateDescriptorSetLayout(VyContext::device(), &layoutInfo, nullptr, &m_IBLDescriptorSetLayout) != VK_SUCCESS)
//         {
//             throw std::runtime_error("Failed to create IBL descriptor set layout");
//         }

//         TArray<VkDescriptorPoolSize, 1> poolSizes{};
//         {
//             poolSizes[0].type            = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
//             poolSizes[0].descriptorCount = static_cast<U32>(MAX_FRAMES_IN_FLIGHT * 3);
//         }

//         VkDescriptorPoolCreateInfo poolInfo{};
//         {
//             poolInfo.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
//             poolInfo.poolSizeCount = static_cast<U32>(poolSizes.size());
//             poolInfo.pPoolSizes    = poolSizes.data();
//             poolInfo.maxSets       = static_cast<U32>(MAX_FRAMES_IN_FLIGHT);
//         }

//         if (vkCreateDescriptorPool(VyContext::device(), &poolInfo, nullptr, &m_IBLDescriptorPool) != VK_SUCCESS)
//         {
//             throw std::runtime_error("Failed to create IBL descriptor pool");
//         }

//         std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, m_IBLDescriptorSetLayout);

//         VkDescriptorSetAllocateInfo allocInfo{};
//         {
//             allocInfo.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
//             allocInfo.descriptorPool     = m_IBLDescriptorPool;
//             allocInfo.descriptorSetCount = static_cast<U32>(MAX_FRAMES_IN_FLIGHT);
//             allocInfo.pSetLayouts        = layouts.data();
//         }
            
//         m_IBLDescriptorSets.resize(MAX_FRAMES_IN_FLIGHT);

//         if (vkAllocateDescriptorSets(VyContext::device(), &allocInfo, m_IBLDescriptorSets.data()) != VK_SUCCESS)
//         {
//             throw std::runtime_error("Failed to allocate IBL descriptor sets");
//         }
//     }


//     void MeshRenderSystem::createMaterialDescriptorResources()
//     {
//         VkDescriptorSetLayoutBinding binding{};
//         {
//             binding.binding            = 0;
//             binding.descriptorType     = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
//             binding.descriptorCount    = 1;
//             binding.stageFlags         = VK_SHADER_STAGE_FRAGMENT_BIT;
//             binding.pImmutableSamplers = nullptr;
//         }

//         VkDescriptorSetLayoutCreateInfo layoutInfo{};
//         {
//             layoutInfo.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
//             layoutInfo.bindingCount = 1;
//             layoutInfo.pBindings    = &binding;
//         }

//         if (vkCreateDescriptorSetLayout(VyContext::device(), &layoutInfo, nullptr, &m_MaterialDescriptorSetLayout) != VK_SUCCESS)
//         {
//             throw std::runtime_error("Failed to create material descriptor set layout");
//         }

//         VkDeviceSize minAlignment = VyContext::device().properties().limits.minUniformBufferOffsetAlignment;
//         VkDeviceSize atomSize     = sizeof(MaterialUniformData);

//         if (minAlignment > 0)
//         {
//             atomSize = (atomSize + minAlignment - 1) & ~(minAlignment - 1);
//         }

//         m_MaterialBuffers.resize(MAX_FRAMES_IN_FLIGHT);

//         for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
//         {
//             m_MaterialBuffers[i] = std::make_unique<VyBuffer>( 
//                 VyBuffer::uniformBuffer(atomSize, 10'000 /*Max objects assumption*/) 
//             );

//             m_MaterialBuffers[i]->map();
//         }

//         VkDescriptorPoolSize poolSize{};
//         {
//             poolSize.type            = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
//             poolSize.descriptorCount = static_cast<U32>(MAX_FRAMES_IN_FLIGHT);
//         }

//         VkDescriptorPoolCreateInfo poolInfo{};
//         {
//             poolInfo.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
//             poolInfo.poolSizeCount = 1;
//             poolInfo.pPoolSizes    = &poolSize;
//             poolInfo.maxSets       = static_cast<U32>(MAX_FRAMES_IN_FLIGHT);
//         }

//         if (vkCreateDescriptorPool(VyContext::device(), &poolInfo, nullptr, &m_MaterialDescriptorPool) != VK_SUCCESS)
//         {
//             throw std::runtime_error("Failed to create material descriptor pool");
//         }

//         std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, m_MaterialDescriptorSetLayout);

//         VkDescriptorSetAllocateInfo allocInfo{};
//         {
//             allocInfo.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
//             allocInfo.descriptorPool     = m_MaterialDescriptorPool;
//             allocInfo.descriptorSetCount = static_cast<U32>(MAX_FRAMES_IN_FLIGHT);
//             allocInfo.pSetLayouts        = layouts.data();
//         }

//         m_MaterialDescriptorSets.resize(MAX_FRAMES_IN_FLIGHT);

//         if (vkAllocateDescriptorSets(VyContext::device(), &allocInfo, m_MaterialDescriptorSets.data()) != VK_SUCCESS)
//         {
//             throw std::runtime_error("Failed to allocate material descriptor sets");
//         }

//         for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
//         {
//             VkDescriptorBufferInfo bufferInfo = m_MaterialBuffers[i]->descriptorBufferInfoForIndex(0);

//             VkWriteDescriptorSet descriptorWrite{};
//             {
//                 descriptorWrite.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
//                 descriptorWrite.dstSet          = m_MaterialDescriptorSets[i];
//                 descriptorWrite.dstBinding      = 0;
//                 descriptorWrite.dstArrayElement = 0;
//                 descriptorWrite.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
//                 descriptorWrite.descriptorCount = 1;
//                 descriptorWrite.pBufferInfo     = &bufferInfo;
//             }

//             vkUpdateDescriptorSets(VyContext::device(), 1, &descriptorWrite, 0, nullptr);
//         }
//     }



//     void MeshRenderSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout, VkDescriptorSetLayout bindlessSetLayout)
//     {
//         VkPushConstantRange pushConstantRange{
//                 .stageFlags = VK_SHADER_STAGE_TASK_BIT_EXT | VK_SHADER_STAGE_MESH_BIT_EXT | VK_SHADER_STAGE_FRAGMENT_BIT,
//                 .offset     = 0,
//                 .size       = sizeof(MeshPushConstantData),
//         };

//         std::vector<VkDescriptorSetLayout> descriptorSetLayouts{
//             globalSetLayout,
//             bindlessSetLayout,
//             m_ShadowDescriptorSetLayout,
//             m_IBLDescriptorSetLayout,
//             m_MaterialDescriptorSetLayout
//         };

//         VkPipelineLayoutCreateInfo pipelineLayoutInfo{
//                 .sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
//                 .setLayoutCount         = static_cast<U32>(descriptorSetLayouts.size()),
//                 .pSetLayouts            = descriptorSetLayouts.data(),
//                 .pushConstantRangeCount = 1,
//                 .pPushConstantRanges    = &pushConstantRange,
//         };
//         if (vkCreatePipelineLayout(VyContext::device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
//         {
//             throw std::runtime_error("failed to create pipeline layout!");
//         }
//     }


//     void MeshRenderSystem::createPipeline(VkRenderPass renderPass)
//     {
//         assert(pipelineLayout != VK_NULL_HANDLE && "Pipeline layout must be created before pipeline.");

//         PipelineConfigInfo pipelineConfig{};
//         Pipeline::defaultMeshPipelineConfigInfo(pipelineConfig);

//         pipelineConfig.renderPass     = renderPass;
//         pipelineConfig.pipelineLayout = pipelineLayout;

//         m_Pipeline = std::make_unique<Pipeline>(device,
//                                             SHADER_PATH "/simple_mesh.task.spv",
//                                             SHADER_PATH "/simple_mesh.mesh.spv",
//                                             SHADER_PATH "/pbr_shader.frag.spv",
//                                             pipelineConfig);

//         // Create Transparent Pipeline
//         PipelineConfigInfo transparentConfig                       = pipelineConfig;
//         transparentConfig.colorBlendAttachment.blendEnable         = VK_TRUE;
//         transparentConfig.colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
//         transparentConfig.colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
//         transparentConfig.colorBlendAttachment.colorBlendOp        = VK_BLEND_OP_ADD;
//         transparentConfig.colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
//         transparentConfig.colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
//         transparentConfig.colorBlendAttachment.alphaBlendOp        = VK_BLEND_OP_ADD;

//         // Fix pointer to attachment
//         transparentConfig.colorBlendInfo.pAttachments = &transparentConfig.colorBlendAttachment;

//         // Disable depth write for transparent objects
//         transparentConfig.depthStencilInfo.depthWriteEnable = VK_FALSE;

//         m_TransparentPipeline = std::make_unique<Pipeline>(device,
//                                                         SHADER_PATH "/simple_mesh.task.spv",
//                                                         SHADER_PATH "/simple_mesh.mesh.spv",
//                                                         SHADER_PATH "/pbr_shader.frag.spv",
//                                                         transparentConfig);
//     }

//     void MeshRenderSystem::setShadowSystem(VyShadowSystem* shadowSystem)
//     {
//         m_CurrentShadowSystem = shadowSystem;
//     }

//     void MeshRenderSystem::setIBLSystem(IBLSystem* iblSystem)
//     {
//         m_CurrentIBLSystem = iblSystem;
//     }


//     void MeshRenderSystem::render(VyFrameInfo& frameInfo)
//     {
//         m_Pipeline->bind(frameInfo.CommandBuffer);

//         vkCmdBindDescriptorSets(frameInfo.CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &frameInfo.globalDescriptorSet, 0, nullptr);
//         vkCmdBindDescriptorSets(frameInfo.CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 1, 1, &frameInfo.globalTextureSet, 0, nullptr);

//         // Bind Shadow Maps
//         if (m_CurrentShadowSystem)
//         {
//         int shadowCount     = m_CurrentShadowSystem->getShadowLightCount();
//         int cubeShadowCount = m_CurrentShadowSystem->getCubeShadowLightCount();

//         TArray<VkDescriptorImageInfo, VyShadowSystem::MAX_SHADOW_MAPS> shadowInfos{};
//         for (int i = 0; i < shadowCount && i < VyShadowSystem::MAX_SHADOW_MAPS; i++)
//         {
//             shadowInfos[i] = m_CurrentShadowSystem->shadowMapDescriptorInfo(i);
//         }
//         for (int i = shadowCount; i < VyShadowSystem::MAX_SHADOW_MAPS; i++)
//         {
//             shadowInfos[i] = shadowCount > 0 ? m_CurrentShadowSystem->shadowMapDescriptorInfo(0) : m_CurrentShadowSystem->getShadowMapDescriptorInfo(0);
//         }

//         TArray<VkDescriptorImageInfo, VyShadowSystem::MAX_CUBE_SHADOW_MAPS> cubeShadowInfos{};
//         for (int i = 0; i < cubeShadowCount && i < VyShadowSystem::MAX_CUBE_SHADOW_MAPS; i++)
//         {
//             cubeShadowInfos[i] = m_CurrentShadowSystem->cubeShadowMapDescriptorInfo(i);
//         }
//         for (int i = cubeShadowCount; i < VyShadowSystem::MAX_CUBE_SHADOW_MAPS; i++)
//         {
//             cubeShadowInfos[i] = cubeShadowCount > 0 ? m_CurrentShadowSystem->cubeShadowMapDescriptorInfo(0)
//                                                     : m_CurrentShadowSystem->cubeShadowMapDescriptorInfo(0);
//         }

//         TArray<VkWriteDescriptorSet, 2> descriptorWrites{};

//         descriptorWrites[0].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
//         descriptorWrites[0].dstSet          = m_ShadowDescriptorSets[frameInfo.FrameIndex];
//         descriptorWrites[0].dstBinding      = 0;
//         descriptorWrites[0].dstArrayElement = 0;
//         descriptorWrites[0].descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
//         descriptorWrites[0].descriptorCount = VyShadowSystem::MAX_SHADOW_MAPS;
//         descriptorWrites[0].pImageInfo      = shadowInfos.data();

//         descriptorWrites[1].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
//         descriptorWrites[1].dstSet          = m_ShadowDescriptorSets[frameInfo.FrameIndex];
//         descriptorWrites[1].dstBinding      = 1;
//         descriptorWrites[1].dstArrayElement = 0;
//         descriptorWrites[1].descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
//         descriptorWrites[1].descriptorCount = VyShadowSystem::MAX_CUBE_SHADOW_MAPS;
//         descriptorWrites[1].pImageInfo      = cubeShadowInfos.data();

//         vkUpdateDescriptorSets(VyContext::device(), static_cast<U32>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
//         vkCmdBindDescriptorSets(frameInfo.CommandBuffer,
//                                 VK_PIPELINE_BIND_POINT_GRAPHICS,
//                                 pipelineLayout,
//                                 2,
//                                 1,
//                                 &m_ShadowDescriptorSets[frameInfo.FrameIndex],
//                                 0,
//                                 nullptr);
//         }

//         // Bind IBL
//         if (m_CurrentIBLSystem && m_CurrentIBLSystem->isGenerated())
//         {
//         VkDescriptorImageInfo irradianceInfo = m_CurrentIBLSystem->getIrradianceDescriptorInfo();
//         VkDescriptorImageInfo prefilterInfo  = m_CurrentIBLSystem->getPrefilteredDescriptorInfo();
//         VkDescriptorImageInfo brdfInfo       = m_CurrentIBLSystem->getBRDFLUTDescriptorInfo();

//         TArray<VkWriteDescriptorSet, 3> descriptorWrites{};

//         descriptorWrites[0].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
//         descriptorWrites[0].dstSet          = m_IBLDescriptorSets[frameInfo.FrameIndex];
//         descriptorWrites[0].dstBinding      = 0;
//         descriptorWrites[0].dstArrayElement = 0;
//         descriptorWrites[0].descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
//         descriptorWrites[0].descriptorCount = 1;
//         descriptorWrites[0].pImageInfo      = &irradianceInfo;

//         descriptorWrites[1].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
//         descriptorWrites[1].dstSet          = m_IBLDescriptorSets[frameInfo.FrameIndex];
//         descriptorWrites[1].dstBinding      = 1;
//         descriptorWrites[1].dstArrayElement = 0;
//         descriptorWrites[1].descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
//         descriptorWrites[1].descriptorCount = 1;
//         descriptorWrites[1].pImageInfo      = &prefilterInfo;

//         descriptorWrites[2].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
//         descriptorWrites[2].dstSet          = m_IBLDescriptorSets[frameInfo.FrameIndex];
//         descriptorWrites[2].dstBinding      = 2;
//         descriptorWrites[2].dstArrayElement = 0;
//         descriptorWrites[2].descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
//         descriptorWrites[2].descriptorCount = 1;
//         descriptorWrites[2].pImageInfo      = &brdfInfo;

//         vkUpdateDescriptorSets(VyContext::device(), static_cast<U32>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
//         vkCmdBindDescriptorSets(frameInfo.CommandBuffer,
//                                 VK_PIPELINE_BIND_POINT_GRAPHICS,
//                                 pipelineLayout,
//                                 3,
//                                 1,
//                                 &m_IBLDescriptorSets[frameInfo.FrameIndex],
//                                 0,
//                                 nullptr);
//         }

//         // Material Buffer Preparation
//         VkDeviceSize minAlignment = VyContext::device().properties().limits.minUniformBufferOffsetAlignment;
//         VkDeviceSize atomSize     = sizeof(MaterialUniformData);
//         if (minAlignment > 0)
//         {
//         atomSize = (atomSize + minAlignment - 1) & ~(minAlignment - 1);
//         }

//         char* mappedData         = (char*)m_MaterialBuffers[frameInfo.FrameIndex]->getMappedMemory();
//         U32   dynamicOffsetIndex = 0;

//         auto view = frameInfo.scene->getRegistry().view<ModelComponent, TransformComponent>();

//         auto renderPass = [&](bool renderTransparent) {
//         for (auto entity : view)
//         {
//             auto [modelComp, transform] = view.get<ModelComponent, TransformComponent>(entity);
//             if (!modelComp.model) continue;

//             const auto& subMeshes = modelComp.model->getSubMeshes();
//             const auto& materials = modelComp.model->getMaterials();

//             for (const auto& subMesh : subMeshes)
//             {
//             if (subMesh.meshletCount == 0) continue;

//             if (dynamicOffsetIndex >= 10000) break;

//             const PBRMaterial* pMaterial = nullptr;
//             if (auto* mat = frameInfo.scene->getRegistry().try_get<PBRMaterial>(entity))
//             {
//                 pMaterial = mat;
//             }
//             else if (subMesh.materialId >= 0 && subMesh.materialId < materials.size())
//             {
//                 pMaterial = &materials[subMesh.materialId].pbrMaterial;
//             }

//             bool isTransparent = false;
//             if (pMaterial)
//             {
//                 if (pMaterial->alphaMode == AlphaMode::Blend || pMaterial->transmission > 0.0f)
//                 {
//                 isTransparent = true;
//                 }
//             }

//             if (isTransparent != renderTransparent) continue;

//             MeshPushConstantData push{};
//             push.modelMatrix             = transform.modelTransform();
//             push.normalMatrix            = glm::transpose(glm::inverse(push.modelMatrix));
//             push.meshId                  = modelComp.model->getMeshId();
//             push.meshletBufferAddress    = modelComp.model->getMeshletBufferAddress();
//             push.meshletVerticesAddress  = modelComp.model->getMeshletVerticesAddress();
//             push.meshletTrianglesAddress = modelComp.model->getMeshletTrianglesAddress();
//             push.vertexBufferAddress     = modelComp.model->getVertexBufferAddress();
//             push.meshletOffset           = subMesh.meshletOffset;
//             push.meshletCount            = subMesh.meshletCount;
//             push.screenSize              = Vec2(frameInfo.extent.width, frameInfo.extent.height);

//             MaterialUniformData matData{};
//             matData.isSelected = ((U32)entity == frameInfo.selectedObjectId) ? 1.0f : 0.0f;

//             if (pMaterial)
//             {
//                 const auto& material = *pMaterial;

//                 U32 textureFlags = 0;
//                 if (material.hasAlbedoMap())
//                 {
//                 textureFlags |= (1 << 0);
//                 matData.albedoIndex = material.albedoMap->getGlobalIndex();
//                 }
//                 if (material.hasNormalMap())
//                 {
//                 textureFlags |= (1 << 1);
//                 matData.normalIndex = material.normalMap->getGlobalIndex();
//                 }
//                 if (material.hasMetallicMap())
//                 {
//                 textureFlags |= (1 << 2);
//                 matData.metallicIndex = material.metallicMap->getGlobalIndex();
//                 }
//                 if (material.hasRoughnessMap())
//                 {
//                 textureFlags |= (1 << 3);
//                 matData.roughnessIndex = material.roughnessMap->getGlobalIndex();
//                 }
//                 if (material.hasAOMap())
//                 {
//                 textureFlags |= (1 << 4);
//                 matData.aoIndex = material.aoMap->getGlobalIndex();
//                 }
//                 if (material.hasEmissiveMap())
//                 {
//                 textureFlags |= (1 << 5);
//                 matData.emissiveIndex = material.emissiveMap->getGlobalIndex();
//                 }
//                 if (material.useMetallicRoughnessTexture)
//                 {
//                 textureFlags |= (1 << 6);
//                 }
//                 if (material.useOcclusionRoughnessMetallicTexture)
//                 {
//                 textureFlags |= (1 << 7);
//                 }

//                 matData.albedo               = material.albedo;
//                 matData.metallic             = material.metallic;
//                 matData.roughness            = material.roughness;
//                 matData.ao                   = material.ao;
//                 matData.emissiveInfo         = Vec4(material.emissiveColor, material.emissiveStrength);
//                 matData.clearcoat            = material.clearcoat;
//                 matData.clearcoatRoughness   = material.clearcoatRoughness;
//                 matData.anisotropic          = material.anisotropic;
//                 matData.anisotropicRotation  = material.anisotropicRotation;
//                 matData.transmission         = material.transmission;
//                 matData.ior                  = material.ior;
//                 matData.iridescence          = material.iridescence;
//                 matData.iridescenceIOR       = material.iridescenceIOR;
//                 matData.iridescenceThickness = material.iridescenceThickness;
//                 matData.textureFlags         = textureFlags;
//                 matData.uvScale              = material.uvScale;
//                 matData.alphaCutoff          = material.alphaCutoff;
//                 matData.alphaMode            = static_cast<U32>(material.alphaMode);
//             }
//             else
//             {
//                 matData.albedo    = Vec4(1.0f);
//                 matData.metallic  = 0.0f;
//                 matData.roughness = 0.5f;
//                 matData.ao        = 1.0f;
//             }

//             memcpy(mappedData + (dynamicOffsetIndex * atomSize), &matData, sizeof(MaterialUniformData));

//             U32 dynamicOffset = static_cast<U32>(dynamicOffsetIndex * atomSize);
//             vkCmdBindDescriptorSets(frameInfo.CommandBuffer,
//                                     VK_PIPELINE_BIND_POINT_GRAPHICS,
//                                     pipelineLayout,
//                                     4,
//                                     1,
//                                     &m_MaterialDescriptorSets[frameInfo.FrameIndex],
//                                     1,
//                                     &dynamicOffset);

//             dynamicOffsetIndex++;

//             vkCmdPushConstants(frameInfo.CommandBuffer,
//                                 pipelineLayout,
//                                 VK_SHADER_STAGE_TASK_BIT_EXT | VK_SHADER_STAGE_MESH_BIT_EXT | VK_SHADER_STAGE_FRAGMENT_BIT,
//                                 0,
//                                 sizeof(MeshPushConstantData),
//                                 &push);

//             if (device.vkCmdDrawMeshTasksEXT)
//             {
//                 U32 groupCount = (subMesh.meshletCount + 31) / 32;
//                 device.vkCmdDrawMeshTasksEXT(frameInfo.CommandBuffer, groupCount, 1, 1);
//             }
//             }
//         }
//         };

//         // Opaque Pass
//         renderPass(false);

//         // Transparent Pass
//         m_TransparentPipeline->bind(frameInfo.CommandBuffer);
//         renderPass(true);
//     }
// }