#include <VyEngine/GFX/Systems/RenderSystem.h>

#include <VyEngine/VK/Context.h>
#include <VyEngine/Globals.h>

#include <VyEngine/GFX/Resources/Texture/Texture.h>
#include <VyEngine/Scene/ECS/Components.h>

#include <iostream>
#include <VyLib/Common/AnsiColor.h>

namespace Vy
{
    struct MainPushConstantData
    {
        Mat4 ModelMatrix { 1.0f };
        Mat4 NormalMatrix{ 1.0f };

        // UVec4 Flags{ 0 };
        U32 Flags{ 0 };
        // U32 _pad[3];
    };


    VyRenderSystem::VyRenderSystem(
        VkRenderPass                   renderPass, 
        TVector<VkDescriptorSetLayout> setLayouts)
    {
        createPipeline( renderPass, setLayouts );
    }


    VyRenderSystem::~VyRenderSystem()
    {
    }


    void VyRenderSystem::renderMainPass(VyFrameInfo frameInfo)
    {
        m_MainPipeline->bind( frameInfo.CommandBuffer );

        TVector<VkDescriptorSet> globSet = { 
            frameInfo.GlobalSet 
        };

        // Set: 0 
        m_MainPipeline->bindDescriptorSets(frameInfo.CommandBuffer, 
            0, 
            globSet
        );

        renderObjects( frameInfo, PushConstantType::MAIN, globSet.size(), true );
    }


    void VyRenderSystem::renderObjects(VyFrameInfo frameInfo, PushConstantType type, int setCount, bool bRenderMaterial)
    {
        auto view = frameInfo.Scene->registry().view<ModelComponent, TransformComponent>();
        
        for (auto&& [ entity, modelComp, transform ] : view.each())
        {
            if (type == VyRenderSystem::MAIN)
            {
                if (!modelComp.Model) return;

                MainPushConstantData data{};
                {
                    data.ModelMatrix  = transform.modelMatrix();
                    data.NormalMatrix = transform.normalMatrix();
                }

                for (const auto& primitive : modelComp.Model->primitives())
                {
                    // const VyPBRMaterial* pMaterial = nullptr;
                    // if (auto* mat = frameInfo.Scene->registry().try_get<VyPBRMaterial>(entity))
                    // {
                    //     pMaterial = mat;
                    // }

                    const auto& material = primitive.Material;

                    U32 textureFlags = 0;

                    if (material.hasAlbedoMap())
                    {
                        textureFlags |= 1 << 0;
                    }

                    // if (material.hasMetallicMap())
                    // {
                    //     textureFlags |= VyGLTFModel::VY_HAS_METALLIC_MAP;
                    // }

                    // if (material.hasRoughnessMap())
                    // {
                    //     textureFlags |= VyGLTFModel::VY_HAS_ROUGHNESS_MAP;
                    // }

                    if (material.hasMetallicRoughnessMap())
                    {
                        textureFlags |= VyGLTFModel::VY_HAS_METALLIC_ROUGHNESS_MAP;
                    }

                    if (material.hasAOMap())
                    {
                        textureFlags |= VyGLTFModel::VY_HAS_AO_MAP;
                    }

                    if (material.hasEmissiveMap())
                    {
                        textureFlags |= VyGLTFModel::VY_HAS_EMISSIVE_MAP;
                    }

                    if (material.hasNormalMap())
                    {
                        textureFlags |= VyGLTFModel::VY_HAS_NORMAL_MAP;
                    }

                    if (material.UseMetallicRoughnessTexture)
                    {
                        textureFlags |= VyGLTFModel::VY_USE_MR_PACKED;
                    }

                    if (material.UseOcclusionRoughnessMetallicTexture)
                    {
                        textureFlags |= VyGLTFModel::VY_USE_ARM_PACKED;
                    }

                    data.Flags = textureFlags;

                    // VY_INFO("{}", data.Flags);
                }

                m_MainPipeline->pushConstants(frameInfo.CommandBuffer, 
                    VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 
                    &data, sizeof(data), 0
                );

                modelComp.Model->bind(frameInfo.CommandBuffer);
                modelComp.Model->draw(frameInfo.CommandBuffer, m_MainPipeline->layout(), setCount, bRenderMaterial);
            }
        }
    }


    void VyRenderSystem::createPipeline(VkRenderPass renderPass, TVector<VkDescriptorSetLayout> setLayouts)
    {
        TVector<VkDescriptorSetLayout> mainSetLayouts;

        mainSetLayouts.push_back( setLayouts[ 0 ] );
        mainSetLayouts.push_back( setLayouts[ 1 ] );

        // ----------------------------------------------------------------------------------------
        
        // Main Pipeline
        {
            auto builder = VyPipeline::GraphicsBuilder{};

            builder.setName( "main" );
            
            builder.addDescriptorSetLayouts( mainSetLayouts );
            
            builder.addPushConstantRange(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(MainPushConstantData));
            
            builder.addShaderStage(VK_SHADER_STAGE_VERTEX_BIT,   "Basic2.vert.spv");
            builder.addShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT, "Basic2.frag.spv");
            
            // Set multisampled. (SAMPLE_COUNT_8_BIT)
            builder.setRasterizationSamples( VyContext::device().msaaSampleCountFlagBits() );

            // Alpha-Blending.
            builder.addColorAttachment(VK_FORMAT_R16G16B16A16_SFLOAT, true);
            builder.setDepthAttachment(VK_FORMAT_D32_SFLOAT);

            builder.setDepthTestEnable(true);
            // builder.setDepthWriteEnable(false);
            // builder.setDepthCompareOp  (VK_COMPARE_OP_LESS_OR_EQUAL);

            builder.setCullMode(VK_CULL_MODE_BACK_BIT);
            builder.setFrontFace(VK_FRONT_FACE_COUNTER_CLOCKWISE);

            builder.setRenderPass( renderPass );
            
            m_MainPipeline = builder.buildPtr();
        }
    }
}