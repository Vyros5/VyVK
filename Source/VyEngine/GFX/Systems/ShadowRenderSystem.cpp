#include <VyEngine/GFX/Systems/ShadowRenderSystem.h>

#include <VyEngine/VK/Context.h>
#include <VyEngine/Globals.h>

#include <VyEngine/GFX/Resources/Texture/Texture.h>
#include <VyEngine/Scene/ECS/Components.h>

#include <iostream>
#include <VyLib/Common/AnsiColor.h>

namespace Vy
{
struct ShadowPushConstantData 
{
		glm::mat4 ModelMatrix{ 1.f };
	};

	//Add here descriptor set
	VyShadowRenderSystem::VyShadowRenderSystem(VkRenderPass renderPass, VkDescriptorSetLayout shadowSetLayout)
    {
		createShadowPipeline(renderPass, shadowSetLayout);
	}

	
    VyShadowRenderSystem::~VyShadowRenderSystem() 
    {
	}


	void VyShadowRenderSystem::createShadowPipeline(VkRenderPass renderPass, VkDescriptorSetLayout shadowSetLayout) 
    {
        auto builder = VyPipeline::GraphicsBuilder{};
        {
            builder.setName( "shadow" );
            
            builder.addDescriptorSetLayout( shadowSetLayout );
            
            builder.addPushConstantRange(VK_SHADER_STAGE_VERTEX_BIT, sizeof(ShadowPushConstantData));
            
            builder.addShaderStage(VK_SHADER_STAGE_VERTEX_BIT,   "Shadow.vert.spv");
            builder.addShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT, "Shadow.frag.spv");
            
            // No alpha blending.
            // builder.addColorAttachment(colorBlendState);
            builder.addColorAttachment(VK_FORMAT_R16G16B16A16_SFLOAT, false);
            builder.setDepthAttachment(VK_FORMAT_D32_SFLOAT);

            // builder.setCullMode (VK_CULL_MODE_BACK_BIT);
            // builder.setFrontFace(VK_FRONT_FACE_COUNTER_CLOCKWISE);
            builder.setRenderPass( renderPass );

            builder.setSubpass( 0 );
        }
	
        m_ShadowPipeline = builder.buildPtr();
    }


	void VyShadowRenderSystem::render(VyFrameInfo& frameInfo) 
    {
		//Set depth bias in order to avoid artifacts
		vkCmdSetDepthBias(
			frameInfo.CommandBuffer,
			kDepthBiasConstant,
			0.0f,
			kDepthBiasSlope
        );

        m_ShadowPipeline->bind( frameInfo.CommandBuffer );

        // Set: 0 - Shadow Set
        m_ShadowPipeline->bindDescriptorSet(frameInfo.CommandBuffer, 
            0, 
            frameInfo.ShadowSet
        );

        auto view = frameInfo.Scene->registry().view<ModelComponent, TransformComponent>();
        
        for (auto&& [ entity, modelComp, transform ] : view.each())
        {
			ShadowPushConstantData push{};
            {
                push.ModelMatrix = transform.modelMatrix(); 
            }

            // Push model constants data.
            m_ShadowPipeline->pushConstants(frameInfo.CommandBuffer, 
                VK_SHADER_STAGE_VERTEX_BIT, 
                &push, 
                sizeof(ShadowPushConstantData), 
                0
            );

            modelComp.Model->bind( frameInfo.CommandBuffer );
            modelComp.Model->draw( frameInfo.CommandBuffer );
		}
	}
}