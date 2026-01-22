#include <VyEngine/GFX/Systems/ModelRenderSystem.h>

#include <VyEngine/VK/Context.h>
#include <VyEngine/Globals.h>

#include <VyEngine/GFX/Resources/Texture/Texture.h>
#include <VyEngine/Scene/ECS/Components.h>

#include <iostream>
#include <VyLib/Common/AnsiColor.h>

namespace Vy
{
    struct MeshPushConstantData
    {
        Mat4 ModelMatrix { 1.0f };
        Mat4 NormalMatrix{ 1.0f };
    };


    VyModelRenderSystem::VyModelRenderSystem(VkRenderPass renderPass, TVector<VkDescriptorSetLayout> globalSetLayouts) 
    {
        createPipeline( renderPass, globalSetLayouts );
    }


    VyModelRenderSystem::~VyModelRenderSystem() 
    {
    }


    void VyModelRenderSystem::createPipeline(VkRenderPass renderPass, TVector<VkDescriptorSetLayout> globalSetLayouts) 
    {
        auto colorBlendState = VyPipeline::colorBlendAttachmentState();

        auto builder = VyPipeline::GraphicsBuilder{};
        {
            builder.setName( "model" );
            
            builder.addDescriptorSetLayouts( globalSetLayouts );
            
            builder.addPushConstantRange(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(MeshPushConstantData));
            
            builder.addShaderStage(VK_SHADER_STAGE_VERTEX_BIT,   "BlingPhong.vert.spv");
            builder.addShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT, "BlingPhong.frag.spv");
            
            // No alpha blending.
            // builder.addColorAttachment(colorBlendState);
            builder.addColorAttachment(VK_FORMAT_R16G16B16A16_SFLOAT, false);
            builder.setDepthAttachment(VK_FORMAT_D32_SFLOAT);

            // builder.setCullMode (VK_CULL_MODE_BACK_BIT);
            // builder.setFrontFace(VK_FRONT_FACE_COUNTER_CLOCKWISE);
            builder.setRenderPass( renderPass );
        }

        m_Pipeline = builder.buildPtr();
    }


    void VyModelRenderSystem::render(VyFrameInfo& frameInfo) 
    {
        // Bind pipeline
        m_Pipeline->bind( frameInfo.CommandBuffer );

        // Set: 0 - Global Descriptor Set (Current Frame UBO)
        m_Pipeline->bindDescriptorSet(frameInfo.CommandBuffer, 
            0, 
            frameInfo.GlobalSet
        );

        auto view = frameInfo.Scene->registry().view<ModelComponent, TransformComponent>();
        
        for (auto&& [ entity, modelComp, transform ] : view.each())
        {
            if (!modelComp.Model) continue;
            
            MeshPushConstantData push{};
            {
                push.ModelMatrix  = transform.modelMatrix(); 
                push.NormalMatrix = transform.normalMatrix();
            }

            // Push model constants data.
            m_Pipeline->pushConstants(frameInfo.CommandBuffer, 
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 
                &push, 
                sizeof(MeshPushConstantData), 
                0
            );

            modelComp.Model->bind( frameInfo.CommandBuffer );
            modelComp.Model->draw( frameInfo.CommandBuffer );
        }
    }
}