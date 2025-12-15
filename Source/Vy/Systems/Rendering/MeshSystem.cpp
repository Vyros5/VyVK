#include <Vy/Systems/Rendering/MeshSystem.h>

#include <Vy/Systems/Rendering/IBLSystem.h>
#include <Vy/Systems/Rendering/ShadowSystem.h>
#include <Vy/GFX/Resources/Texture.h>
#include <Vy/GFX/Resources/PBRMaterial.h>

#include <Vy/GFX/Context.h>

namespace Vy
{

    struct MeshPushConstantData
    {
        Mat4 ModelMatrix{1.0f};
        Mat4 NormalMatrix{1.0f};
        U32  MeshId{0};

        U64  MeshletBufferAddress;
        U64  MeshletVerticesAddress;
        U64  MeshletTrianglesAddress;
        U64  VertexBufferAddress;
        U32  MeshletOffset;
        U32  MeshletCount;
        Vec2 ScreenSize;
    };


    MeshRenderSystem::MeshRenderSystem(VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout, VkDescriptorSetLayout bindlessSetLayout)
    {
        createShadowDescriptorResources();
        createIBLDescriptorResources();
        createMaterialDescriptorResources();

        createPipeline(renderPass, globalSetLayout, bindlessSetLayout);
    }


    MeshRenderSystem::~MeshRenderSystem()
    {
        // vkDestroyPipelineLayout(VyContext::device(), pipelineLayout, nullptr);
        
        if (m_ShadowDescriptorPool != VK_NULL_HANDLE)
        {
            vkDestroyDescriptorPool(VyContext::device(), m_ShadowDescriptorPool, nullptr);
        }

        if (m_ShadowDescriptorSetLayout != VK_NULL_HANDLE)
        {
            vkDestroyDescriptorSetLayout(VyContext::device(), m_ShadowDescriptorSetLayout, nullptr);
        }

        if (m_IBLDescriptorPool != VK_NULL_HANDLE)
        {
            vkDestroyDescriptorPool(VyContext::device(), m_IBLDescriptorPool, nullptr);
        }

        if (m_IBLDescriptorSetLayout != VK_NULL_HANDLE)
        {
            vkDestroyDescriptorSetLayout(VyContext::device(), m_IBLDescriptorSetLayout, nullptr);
        }

        if (m_MaterialDescriptorPool != VK_NULL_HANDLE)
        {
            vkDestroyDescriptorPool(VyContext::device(), m_MaterialDescriptorPool, nullptr);
        }

        if (m_MaterialDescriptorSetLayout != VK_NULL_HANDLE)
        {
            vkDestroyDescriptorSetLayout(VyContext::device(), m_MaterialDescriptorSetLayout, nullptr);
        }
    }


    void MeshRenderSystem::createShadowDescriptorResources()
    {
        TArray<VkDescriptorSetLayoutBinding, 2> bindings{};
        {
            bindings[0].binding            = 0;
            bindings[0].descriptorType     = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            bindings[0].descriptorCount    = VyShadowSystem::MAX_SHADOW_MAPS;
            bindings[0].stageFlags         = VK_SHADER_STAGE_FRAGMENT_BIT;
            bindings[0].pImmutableSamplers = nullptr;
            
            bindings[1].binding            = 1;
            bindings[1].descriptorType     = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            bindings[1].descriptorCount    = VyShadowSystem::MAX_CUBE_SHADOW_MAPS;
            bindings[1].stageFlags         = VK_SHADER_STAGE_FRAGMENT_BIT;
            bindings[1].pImmutableSamplers = nullptr;
        }

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        {
            layoutInfo.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            layoutInfo.bindingCount = static_cast<U32>(bindings.size());
            layoutInfo.pBindings    = bindings.data();
        }

        if (vkCreateDescriptorSetLayout(VyContext::device(), &layoutInfo, nullptr, &m_ShadowDescriptorSetLayout) != VK_SUCCESS)
        {
            VY_THROW_RUNTIME_ERROR("Failed to create shadow descriptor set layout");
        }

        TArray<VkDescriptorPoolSize, 2> poolSizes{};
        {
            poolSizes[0].type            = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            poolSizes[0].descriptorCount = static_cast<U32>(MAX_FRAMES_IN_FLIGHT * VyShadowSystem::MAX_SHADOW_MAPS);
            poolSizes[1].type            = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            poolSizes[1].descriptorCount = static_cast<U32>(MAX_FRAMES_IN_FLIGHT * VyShadowSystem::MAX_CUBE_SHADOW_MAPS);
        }

        VkDescriptorPoolCreateInfo poolInfo{};
        {
            poolInfo.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            poolInfo.poolSizeCount = static_cast<U32>(poolSizes.size());
            poolInfo.pPoolSizes    = poolSizes.data();
            poolInfo.maxSets       = static_cast<U32>(MAX_FRAMES_IN_FLIGHT);
        }

        if (vkCreateDescriptorPool(VyContext::device(), &poolInfo, nullptr, &m_ShadowDescriptorPool) != VK_SUCCESS)
        {
            VY_THROW_RUNTIME_ERROR("Failed to create shadow descriptor pool");
        }

        TVector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, m_ShadowDescriptorSetLayout);

        VkDescriptorSetAllocateInfo allocInfo{};
        {
            allocInfo.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            allocInfo.descriptorPool     = m_ShadowDescriptorPool;
            allocInfo.descriptorSetCount = static_cast<U32>(MAX_FRAMES_IN_FLIGHT);
            allocInfo.pSetLayouts        = layouts.data();
        }

        m_ShadowDescriptorSets.resize(MAX_FRAMES_IN_FLIGHT);

        if (vkAllocateDescriptorSets(VyContext::device(), &allocInfo, m_ShadowDescriptorSets.data()) != VK_SUCCESS)
        {
            VY_THROW_RUNTIME_ERROR("Failed to allocate shadow descriptor sets");
        }
    }


    void MeshRenderSystem::createIBLDescriptorResources()
    {
        TArray<VkDescriptorSetLayoutBinding, 3> bindings{};
        {
            bindings[0].binding            = 0;
            bindings[0].descriptorType     = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            bindings[0].descriptorCount    = 1;
            bindings[0].stageFlags         = VK_SHADER_STAGE_FRAGMENT_BIT;
            bindings[0].pImmutableSamplers = nullptr;
            
            bindings[1].binding            = 1;
            bindings[1].descriptorType     = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            bindings[1].descriptorCount    = 1;
            bindings[1].stageFlags         = VK_SHADER_STAGE_FRAGMENT_BIT;
            bindings[1].pImmutableSamplers = nullptr;
            
            bindings[2].binding            = 2;
            bindings[2].descriptorType     = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            bindings[2].descriptorCount    = 1;
            bindings[2].stageFlags         = VK_SHADER_STAGE_FRAGMENT_BIT;
            bindings[2].pImmutableSamplers = nullptr;
        }

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        {
            layoutInfo.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            layoutInfo.bindingCount = static_cast<U32>(bindings.size());
            layoutInfo.pBindings    = bindings.data();
        }

        if (vkCreateDescriptorSetLayout(VyContext::device(), &layoutInfo, nullptr, &m_IBLDescriptorSetLayout) != VK_SUCCESS)
        {
            VY_THROW_RUNTIME_ERROR("Failed to create IBL descriptor set layout");
        }

        TArray<VkDescriptorPoolSize, 1> poolSizes{};
        {
            poolSizes[0].type            = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            poolSizes[0].descriptorCount = static_cast<U32>(MAX_FRAMES_IN_FLIGHT * 3);
        }

        VkDescriptorPoolCreateInfo poolInfo{};
        {
            poolInfo.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            poolInfo.poolSizeCount = static_cast<U32>(poolSizes.size());
            poolInfo.pPoolSizes    = poolSizes.data();
            poolInfo.maxSets       = static_cast<U32>(MAX_FRAMES_IN_FLIGHT);
        }

        if (vkCreateDescriptorPool(VyContext::device(), &poolInfo, nullptr, &m_IBLDescriptorPool) != VK_SUCCESS)
        {
            VY_THROW_RUNTIME_ERROR("Failed to create IBL descriptor pool");
        }

        TVector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, m_IBLDescriptorSetLayout);

        VkDescriptorSetAllocateInfo allocInfo{};
        {
            allocInfo.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            allocInfo.descriptorPool     = m_IBLDescriptorPool;
            allocInfo.descriptorSetCount = static_cast<U32>(MAX_FRAMES_IN_FLIGHT);
            allocInfo.pSetLayouts        = layouts.data();
        }
            
        m_IBLDescriptorSets.resize(MAX_FRAMES_IN_FLIGHT);

        if (vkAllocateDescriptorSets(VyContext::device(), &allocInfo, m_IBLDescriptorSets.data()) != VK_SUCCESS)
        {
            VY_THROW_RUNTIME_ERROR("Failed to allocate IBL descriptor sets");
        }
    }


    void MeshRenderSystem::createMaterialDescriptorResources()
    {
        VkDescriptorSetLayoutBinding binding{};
        {
            binding.binding            = 0;
            binding.descriptorType     = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
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

        if (vkCreateDescriptorSetLayout(VyContext::device(), &layoutInfo, nullptr, &m_MaterialDescriptorSetLayout) != VK_SUCCESS)
        {
            VY_THROW_RUNTIME_ERROR("Failed to create material descriptor set layout");
        }

        VkDeviceSize minAlignment = VyContext::device().properties().limits.minUniformBufferOffsetAlignment;
        VkDeviceSize atomSize     = sizeof(MaterialUniformData);

        if (minAlignment > 0)
        {
            atomSize = (atomSize + minAlignment - 1) & ~(minAlignment - 1);
        }

        m_MaterialBuffers.resize(MAX_FRAMES_IN_FLIGHT);

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            m_MaterialBuffers[i] = std::make_unique<VyBuffer>( 
                VyBuffer::uniformBuffer(atomSize, 10'000 /*Max objects assumption*/) 
            );

            m_MaterialBuffers[i]->map();
        }

        VkDescriptorPoolSize poolSize{};
        {
            poolSize.type            = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
            poolSize.descriptorCount = static_cast<U32>(MAX_FRAMES_IN_FLIGHT);
        }

        VkDescriptorPoolCreateInfo poolInfo{};
        {
            poolInfo.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            poolInfo.poolSizeCount = 1;
            poolInfo.pPoolSizes    = &poolSize;
            poolInfo.maxSets       = static_cast<U32>(MAX_FRAMES_IN_FLIGHT);
        }

        if (vkCreateDescriptorPool(VyContext::device(), &poolInfo, nullptr, &m_MaterialDescriptorPool) != VK_SUCCESS)
        {
            VY_THROW_RUNTIME_ERROR("Failed to create material descriptor pool");
        }

        TVector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, m_MaterialDescriptorSetLayout);

        VkDescriptorSetAllocateInfo allocInfo{};
        {
            allocInfo.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            allocInfo.descriptorPool     = m_MaterialDescriptorPool;
            allocInfo.descriptorSetCount = static_cast<U32>(MAX_FRAMES_IN_FLIGHT);
            allocInfo.pSetLayouts        = layouts.data();
        }

        m_MaterialDescriptorSets.resize(MAX_FRAMES_IN_FLIGHT);

        if (vkAllocateDescriptorSets(VyContext::device(), &allocInfo, m_MaterialDescriptorSets.data()) != VK_SUCCESS)
        {
            VY_THROW_RUNTIME_ERROR("Failed to allocate material descriptor sets");
        }

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            VkDescriptorBufferInfo bufferInfo = m_MaterialBuffers[i]->descriptorBufferInfoForIndex(0);

            VkWriteDescriptorSet descriptorWrite{};
            {
                descriptorWrite.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                descriptorWrite.dstSet          = m_MaterialDescriptorSets[i];
                descriptorWrite.dstBinding      = 0;
                descriptorWrite.dstArrayElement = 0;
                descriptorWrite.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
                descriptorWrite.descriptorCount = 1;
                descriptorWrite.pBufferInfo     = &bufferInfo;
            }

            vkUpdateDescriptorSets(VyContext::device(), 1, &descriptorWrite, 0, nullptr);
        }
    }


    void MeshRenderSystem::createPipeline(VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout, VkDescriptorSetLayout bindlessSetLayout)
    {
        TVector<VkDescriptorSetLayout> descriptorSetLayouts{
            globalSetLayout,
            bindlessSetLayout,
            m_ShadowDescriptorSetLayout,
            m_IBLDescriptorSetLayout,
            m_MaterialDescriptorSetLayout
        };

        // Create Pipeline
        m_Pipeline = VyPipeline::GraphicsBuilder{ VyPipeline::EFlags::MeshShader }
            .addDescriptorSetLayouts(descriptorSetLayouts)
            // .addFlag(VyPipeline::EFlags::MeshShader)
            .addPushConstantRange(VK_SHADER_STAGE_TASK_BIT_EXT | VK_SHADER_STAGE_MESH_BIT_EXT | VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(MeshPushConstantData))
            .addShaderStage(VK_SHADER_STAGE_TASK_BIT_EXT, "Mesh/Mesh.task.spv")
            .addShaderStage(VK_SHADER_STAGE_MESH_BIT_EXT, "Mesh/Mesh.mesh.spv")
            .addShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT, "PBR.frag.spv")
            .setRenderPass(renderPass)
			// .setVertexBindingDescriptions  ({}) // Clear default vertex binding.
			// .setVertexAttributeDescriptions({}) // Clear default vertex attributes.
        .buildUnique();

        // Create Transparent Pipeline
        m_TransparentPipeline = VyPipeline::GraphicsBuilder{ VyPipeline::EFlags::MeshShader }
            .addDescriptorSetLayouts(descriptorSetLayouts)
            // .addFlag(VyPipeline::EFlags::MeshShader)
            .addPushConstantRange(VK_SHADER_STAGE_TASK_BIT_EXT | VK_SHADER_STAGE_MESH_BIT_EXT | VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(MeshPushConstantData))
            .addShaderStage(VK_SHADER_STAGE_TASK_BIT_EXT, "Mesh/Mesh.task.spv")
            .addShaderStage(VK_SHADER_STAGE_MESH_BIT_EXT, "Mesh/Mesh.mesh.spv")
            .addShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT, "PBR.frag.spv")
            .addColorAttachment(VK_FORMAT_R16G16B16A16_SFLOAT, true) // Enable alpha-blending.
            // Disable depth write for transparent objects
            // .setDepthAttachment(VK_FORMAT_D32_SFLOAT)
            .setRenderPass(renderPass)
			// .setVertexBindingDescriptions  ({}) // Clear default vertex binding.
			// .setVertexAttributeDescriptions({}) // Clear default vertex attributes.
        .buildUnique();
    }


    void MeshRenderSystem::setShadowSystem(VyShadowSystem* shadowSystem)
    {
        m_CurrentShadowSystem = shadowSystem;
    }


    void MeshRenderSystem::setIBLSystem(IBLSystem* iblSystem)
    {
        m_CurrentIBLSystem = iblSystem;
    }


    void MeshRenderSystem::render(VyFrameInfo& frameInfo)
    {
        m_Pipeline->bind(frameInfo.CommandBuffer);

        // Set index: 0
        m_Pipeline->bindDescriptorSet(frameInfo.CommandBuffer, 0, frameInfo.GlobalDescriptorSet);

        // Set index: 1
        m_Pipeline->bindDescriptorSet(frameInfo.CommandBuffer, 1, frameInfo.GlobalTextureSet);

        // Bind Shadow Maps
        if (m_CurrentShadowSystem)
        {
            int shadowCount     = m_CurrentShadowSystem->shadowLightCount();
            int cubeShadowCount = m_CurrentShadowSystem->cubeShadowLightCount();

            TArray<VkDescriptorImageInfo, VyShadowSystem::MAX_SHADOW_MAPS> shadowInfos{};

            for (int i = 0; i < shadowCount && i < VyShadowSystem::MAX_SHADOW_MAPS; i++)
            {
                shadowInfos[i] = m_CurrentShadowSystem->shadowMapDescriptorInfo(i);
            }
            for (int i = shadowCount; i < VyShadowSystem::MAX_SHADOW_MAPS; i++)
            {
                shadowInfos[i] = shadowCount > 0 
                    ? m_CurrentShadowSystem->shadowMapDescriptorInfo(0) 
                    : m_CurrentShadowSystem->shadowMapDescriptorInfo(0);
            }

            TArray<VkDescriptorImageInfo, VyShadowSystem::MAX_CUBE_SHADOW_MAPS> cubeShadowInfos{};

            for (int i = 0; i < cubeShadowCount && i < VyShadowSystem::MAX_CUBE_SHADOW_MAPS; i++)
            {
                cubeShadowInfos[i] = m_CurrentShadowSystem->cubeShadowMapDescriptorInfo(i);
            }
            for (int i = cubeShadowCount; i < VyShadowSystem::MAX_CUBE_SHADOW_MAPS; i++)
            {
                cubeShadowInfos[i] = cubeShadowCount > 0 
                    ? m_CurrentShadowSystem->cubeShadowMapDescriptorInfo(0)
                    : m_CurrentShadowSystem->cubeShadowMapDescriptorInfo(0);
            }

            TArray<VkWriteDescriptorSet, 2> descriptorWrites{};
            {
                descriptorWrites[0].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                descriptorWrites[0].dstSet          = m_ShadowDescriptorSets[frameInfo.FrameIndex];
                descriptorWrites[0].dstBinding      = 0;
                descriptorWrites[0].dstArrayElement = 0;
                descriptorWrites[0].descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                descriptorWrites[0].descriptorCount = VyShadowSystem::MAX_SHADOW_MAPS;
                descriptorWrites[0].pImageInfo      = shadowInfos.data();
                
                descriptorWrites[1].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                descriptorWrites[1].dstSet          = m_ShadowDescriptorSets[frameInfo.FrameIndex];
                descriptorWrites[1].dstBinding      = 1;
                descriptorWrites[1].dstArrayElement = 0;
                descriptorWrites[1].descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                descriptorWrites[1].descriptorCount = VyShadowSystem::MAX_CUBE_SHADOW_MAPS;
                descriptorWrites[1].pImageInfo      = cubeShadowInfos.data();
            }

            vkUpdateDescriptorSets(VyContext::device(), static_cast<U32>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
            
            // Set index: 2
            m_Pipeline->bindDescriptorSet(frameInfo.CommandBuffer, 2, m_ShadowDescriptorSets[frameInfo.FrameIndex]);
        }

        // Bind IBL
        if (m_CurrentIBLSystem && m_CurrentIBLSystem->isGenerated())
        {
            VkDescriptorImageInfo irradianceInfo = m_CurrentIBLSystem->irradianceDescriptorImageInfo();
            VkDescriptorImageInfo prefilterInfo  = m_CurrentIBLSystem->prefilteredDescriptorImageInfo();
            VkDescriptorImageInfo brdfInfo       = m_CurrentIBLSystem->brdfLUTDescriptorImageInfo();

            TArray<VkWriteDescriptorSet, 3> descriptorWrites{};
            {
                descriptorWrites[0].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                descriptorWrites[0].dstSet          = m_IBLDescriptorSets[frameInfo.FrameIndex];
                descriptorWrites[0].dstBinding      = 0;
                descriptorWrites[0].dstArrayElement = 0;
                descriptorWrites[0].descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                descriptorWrites[0].descriptorCount = 1;
                descriptorWrites[0].pImageInfo      = &irradianceInfo;
                
                descriptorWrites[1].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                descriptorWrites[1].dstSet          = m_IBLDescriptorSets[frameInfo.FrameIndex];
                descriptorWrites[1].dstBinding      = 1;
                descriptorWrites[1].dstArrayElement = 0;
                descriptorWrites[1].descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                descriptorWrites[1].descriptorCount = 1;
                descriptorWrites[1].pImageInfo      = &prefilterInfo;
                
                descriptorWrites[2].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                descriptorWrites[2].dstSet          = m_IBLDescriptorSets[frameInfo.FrameIndex];
                descriptorWrites[2].dstBinding      = 2;
                descriptorWrites[2].dstArrayElement = 0;
                descriptorWrites[2].descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                descriptorWrites[2].descriptorCount = 1;
                descriptorWrites[2].pImageInfo      = &brdfInfo;
            }

            vkUpdateDescriptorSets(VyContext::device(), static_cast<U32>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
            
            // Set index: 3
            m_Pipeline->bindDescriptorSet(frameInfo.CommandBuffer, 3, m_IBLDescriptorSets[frameInfo.FrameIndex]);
        }

        // Material Buffer Preparation
        VkDeviceSize minAlignment = VyContext::device().properties().limits.minUniformBufferOffsetAlignment;
        VkDeviceSize atomSize     = sizeof(MaterialUniformData);

        if (minAlignment > 0)
        {
            atomSize = (atomSize + minAlignment - 1) & ~(minAlignment - 1);
        }

        char* mappedData         = (char*)m_MaterialBuffers[frameInfo.FrameIndex]->mappedData();
        U32   dynamicOffsetIndex = 0;

        auto view = frameInfo.Scene->registry().view<ModelComponent2, TransformComponent>();

        auto renderPass = [&](bool renderTransparent) 
        {
            for (auto&& [ entity, modelComp, transform ] : view.each())
            {
                if (!modelComp.model) continue;

                const auto& subMeshes = modelComp.model->subMeshes();
                const auto& materials = modelComp.model->materials();

                for (const auto& subMesh : subMeshes)
                {
                    if (subMesh.MeshletCount == 0) continue;

                    if (dynamicOffsetIndex >= 10000) break;

                    const VyPBRMaterial* pMaterial = nullptr;

                    if (auto* mat = frameInfo.Scene->registry().try_get<VyPBRMaterial>(entity))
                    {
                        pMaterial = mat;
                    }
                    else if (subMesh.MaterialId >= 0 && subMesh.MaterialId < materials.size())
                    {
                        pMaterial = &materials[subMesh.MaterialId].PBRMaterial;
                    }

                    bool isTransparent = false;

                    if (pMaterial)
                    {
                        if (pMaterial->AlphaMode == AlphaMode::Blend || pMaterial->Transmission > 0.0f)
                        {
                            isTransparent = true;
                        }
                    }

                    if (isTransparent != renderTransparent) continue;

                    MeshPushConstantData push{};
                    {
                        push.ModelMatrix             = transform.matrix();
                        push.NormalMatrix            = glm::transpose(glm::inverse(push.ModelMatrix));
                        push.MeshId                  = modelComp.model->meshId();
                        push.MeshletBufferAddress    = modelComp.model->meshletBufferAddress();
                        push.MeshletVerticesAddress  = modelComp.model->meshletVerticesAddress();
                        push.MeshletTrianglesAddress = modelComp.model->meshletTrianglesAddress();
                        push.VertexBufferAddress     = modelComp.model->vertexBufferAddress();
                        push.MeshletOffset           = subMesh.MeshletOffset;
                        push.MeshletCount            = subMesh.MeshletCount;
                        // push.ScreenSize              = Vec2(frameInfo.Extent.width, frameInfo.Extent.height);
                    }

                    MaterialUniformData matData{};
                    {
                        // matData.IsSelected = ((U32)entity == frameInfo.SelectedObjectId) ? 1.0f : 0.0f;
                    }

                    if (pMaterial)
                    {
                        const auto& material = *pMaterial;

                        U32 textureFlags = 0;

                        if (material.hasAlbedoMap())
                        {
                            textureFlags |= (1 << 0);
                            matData.AlbedoIndex = material.AlbedoMap->globalIndex();
                        }
                        if (material.hasNormalMap())
                        {
                            textureFlags |= (1 << 1);
                            matData.NormalIndex = material.NormalMap->globalIndex();
                        }
                        if (material.hasMetallicMap())
                        {
                            textureFlags |= (1 << 2);
                            matData.MetallicIndex = material.MetallicMap->globalIndex();
                        }
                        if (material.hasRoughnessMap())
                        {
                            textureFlags |= (1 << 3);
                            matData.RoughnessIndex = material.RoughnessMap->globalIndex();
                        }
                        if (material.hasAOMap())
                        {
                            textureFlags |= (1 << 4);
                            matData.AOIndex = material.AOMap->globalIndex();
                        }
                        if (material.hasEmissiveMap())
                        {
                            textureFlags |= (1 << 5);
                            matData.EmissiveIndex = material.EmissiveMap->globalIndex();
                        }
                        if (material.UseMetallicRoughnessTexture)
                        {
                            textureFlags |= (1 << 6);
                        }
                        if (material.UseOcclusionRoughnessMetallicTexture)
                        {
                            textureFlags |= (1 << 7);
                        }

                        matData.Albedo               = material.Albedo;
                        matData.Metallic             = material.Metallic;
                        matData.Roughness            = material.Roughness;
                        matData.AO                   = material.AO;
                        matData.EmissiveInfo         = Vec4(material.EmissiveColor, material.EmissiveStrength);
                        matData.Clearcoat            = material.Clearcoat;
                        matData.ClearcoatRoughness   = material.ClearcoatRoughness;
                        matData.Anisotropic          = material.Anisotropic;
                        matData.AnisotropicRotation  = material.AnisotropicRotation;
                        matData.Transmission         = material.Transmission;
                        matData.IOR                  = material.IOR;
                        matData.Iridescence          = material.Iridescence;
                        matData.IridescenceIOR       = material.IridescenceIOR;
                        matData.IridescenceThickness = material.IridescenceThickness;
                        matData.TextureFlags         = textureFlags;
                        matData.UVScale              = material.UVScale;
                        matData.AlphaCutoff          = material.AlphaCutoff;
                        matData.AlphaMode            = static_cast<U32>(material.AlphaMode);
                    }
                    else
                    {
                        matData.Albedo    = Vec4(1.0f);
                        matData.Metallic  = 0.0f;
                        matData.Roughness = 0.5f;
                        matData.AO        = 1.0f;
                    }

                    memcpy(mappedData + (dynamicOffsetIndex * atomSize), &matData, sizeof(MaterialUniformData));

                    U32 dynamicOffset = static_cast<U32>(dynamicOffsetIndex * atomSize);

                    // Set index: 4 (with Dynamic offsets)
                    m_Pipeline->bindDescriptorSet(frameInfo.CommandBuffer, 
                        4, 
                        m_MaterialDescriptorSets[frameInfo.FrameIndex],
                        1,
                        &dynamicOffset
                    );

                    dynamicOffsetIndex++;

                    m_Pipeline->pushConstants(frameInfo.CommandBuffer,
                        VK_SHADER_STAGE_TASK_BIT_EXT | VK_SHADER_STAGE_MESH_BIT_EXT | VK_SHADER_STAGE_FRAGMENT_BIT, 
                        push
                    );

                    // if (vkCmdDrawMeshTasksEXT)
                    // {
                        U32 groupCount = (subMesh.MeshletCount + 31) / 32;

                        vkCmdDrawMeshTasksEXT(frameInfo.CommandBuffer, groupCount, 1, 1);
                    // }
                } // End of submess iteration.

            } // End of entity iteration.

        }; // End of renderPass lambda.

        // Opaque Pass
        renderPass(false);

        // Transparent Pass
        m_TransparentPipeline->bind(frameInfo.CommandBuffer);

        renderPass(true);
    }
}