#include <VyEngine/GFX/Systems/PointLightSystem.h>

#include <VyEngine/VK/Context.h>
#include <VyEngine/Globals.h>

#include <VyEngine/GFX/Resources/Texture/Texture.h>
#include <VyEngine/Scene/ECS/Components.h>

#include <iostream>
#include <VyLib/Common/AnsiColor.h>

namespace Vy
{
    VyPointLightSystem::VyPointLightSystem(
        VkRenderPass          renderPass, 
        VkDescriptorSetLayout gBufferSetLayout, 
        VkDescriptorSetLayout compositionSetLayout)
    {
        createPointLightPipeline(renderPass, gBufferSetLayout, compositionSetLayout);
    }

    VyPointLightSystem::~VyPointLightSystem()
    {

    }

    void VyPointLightSystem::createPointLightPipeline(
        VkRenderPass          renderPass, 
        VkDescriptorSetLayout gBufferSetLayout, 
        VkDescriptorSetLayout compositionSetLayout)
    {
        TVector<VkDescriptorSetLayout> layouts = { gBufferSetLayout, compositionSetLayout };

        auto builder = VyPipeline::GraphicsBuilder{};
        {
            builder.setName( "point_light" );
            
            builder.addDescriptorSetLayouts( layouts );
            
            builder.addShaderStage(VK_SHADER_STAGE_VERTEX_BIT,   "PointLight.vert.spv");
            builder.addShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT, "PointLight.frag.spv");
            
            // Enable Aplha Blending
            // builder.addColorAttachment(VK_FORMAT_R16G16B16A16_SFLOAT, true);
            // builder.setDepthAttachment(VK_FORMAT_D32_SFLOAT);
            // builder.setDepthTest(true, false);
            // builder.setCullMode(VK_CULL_MODE_BACK_BIT);

            // Clear vertex bindings and attributes.
            builder.clearVertexDescriptions();
            
            builder.setRenderPass( renderPass );

            builder.setSubpass( 1 );
        }

        m_PointPipeline = builder.buildPtr();
    }

	void VyPointLightSystem::render(VyFrameInfo& frameInfo) 
    {
		m_PointPipeline->bind(frameInfo.CommandBuffer);
        
        // Set: 0 - GBuffer Set
        m_PointPipeline->bindDescriptorSet(frameInfo.CommandBuffer, 
            0, 
            frameInfo.GBufferSet
        );

        // Set: 1 - Composition Set
        m_PointPipeline->bindDescriptorSet(frameInfo.CommandBuffer, 
            1, 
            frameInfo.CompositionSet
        );

        // Draw quad.
		vkCmdDraw(frameInfo.CommandBuffer, 6, 1, 0, 0);
	}
}