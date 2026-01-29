#include <VyEngine/GFX/Systems/PointLightSystem.h>

#include <VyEngine/VK/Context.h>
#include <VyEngine/Globals.h>

#include <VyEngine/GFX/Resources/Texture/Texture.h>
#include <VyEngine/Scene/ECS/Components.h>

#include <iostream>
#include <VyLib/Common/AnsiColor.h>

namespace Vy
{
    struct LightObjectPushConstant
    {
        Vec4  Position{}; // Ignore W
        Vec4  Color   {}; // RGB: Color, A: Intensity
        float Radius  {};
    };

    VyPointLightSystem::VyPointLightSystem(VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout)
    {
        createPipeline( renderPass, globalSetLayout );
    }


    VyPointLightSystem::~VyPointLightSystem()
    {
    }


    void VyPointLightSystem::update(VyFrameInfo& frameInfo, GlobalUbo& ubo)
    {
        auto& registry = frameInfo.Scene->registry();

        // ----------------------------------------------------------------------------------------
        // [ Alpha Sorting ]

        m_SortedLights.clear();

        Vec3 camPos = frameInfo.Camera.position();

        int pointLightIndex = 0;
        int spotLightIndex  = 0;

        auto view = registry.view<LightComponent, TransformComponent>();
        
        for (auto&& [ entity, lightComp, transform ] : view.each())
        {
           VY_ASSERT(pointLightIndex <= MAX_POINT_LIGHTS, "Point lights exceed maximum specified");
           VY_ASSERT(spotLightIndex  <= MAX_SPOT_LIGHTS,  "Spot lights exceed maximum specified");

            // Calculate distance
            Vec3 offset = camPos - transform.Translation;

            // Dot product to get distance squared, less expensive than sqrt.
            float disSquared = glm::dot( offset, offset );

            m_SortedLights.insert({ disSquared, entity });

            if (lightComp.IsPoint)
            {
                pointLightIndex++;
            }
            else {
                spotLightIndex++;
            }
        }

        // Clear light counts.
        ubo.PointLightsCount = 0;
        ubo.SpotLightsCount  = 0;

        // ----------------------------------------------------------------------------------------
        // [ Process Lights ]

        auto rotateLight    = glm::rotate(Mat4(1.0f), frameInfo.FrameTime, { 0.0f, -1.0f,  0.0f });
        auto rotateDirLight = glm::rotate(Mat4(1.0f), frameInfo.FrameTime, { 0.0f,  0.0f, -1.0f });

        TMap<float, EntityHandle>::reverse_iterator it;
        
        for (it = m_SortedLights.rbegin(); it != m_SortedLights.rend(); it++)
        {
            auto& entity = it->second;
            const auto& [ lightComp, transform ] = view.get<LightComponent, TransformComponent>(entity);

           VY_ASSERT(pointLightIndex <= MAX_POINT_LIGHTS, "Point lights exceed maximum specified");
           VY_ASSERT(spotLightIndex  <= MAX_SPOT_LIGHTS,  "Spot lights exceed maximum specified");

            // Copy light data to UBO.
            if (lightComp.IsPoint)
            {
                // Update point light position. 
                transform.Translation = Vec3( rotateLight * Vec4( transform.Translation, 1.0f ) );

                ubo.PointLights[ pointLightIndex ].Position = Vec4(transform.Translation, 1.0f);
                ubo.PointLights[ pointLightIndex ].Color    = Vec4(lightComp.Color, lightComp.Intensity);
            }
            else {
                ubo.SpotLights[ spotLightIndex ].Position  = Vec4(transform.Translation, 1.0f);
                ubo.SpotLights[ spotLightIndex ].Color     = Vec4(lightComp.Color, lightComp.Intensity);
                ubo.SpotLights[ spotLightIndex ].Direction = Vec4(lightComp.Direction, 1.0f);
                ubo.SpotLights[ spotLightIndex ].Cutoffs   = Vec4(lightComp.Cutoff, lightComp.OuterCutoff, 0.0f, 0.0f);
            }
        }

        ubo.PointLightsCount = pointLightIndex;
        ubo.SpotLightsCount  = spotLightIndex;
    }


    void VyPointLightSystem::render(VyFrameInfo& frameInfo, GlobalUbo& globalUBO)
    {
        // PROFILE_FUNCTION();

        auto& registry = frameInfo.Scene->registry();

        // ----------------------------------------------------------------------------------------

        m_Pipeline->bind(frameInfo.CommandBuffer);

        // Set: 0 - Global Set
        m_Pipeline->bindDescriptorSet(frameInfo.CommandBuffer, 
            0, 
            frameInfo.GlobalSet
        );

        TMap<float, EntityHandle>::reverse_iterator it;
        
        //iterate through sorted map in reverse order (Point and Spot Light Objects)
        for (it = m_SortedLights.rbegin(); it != m_SortedLights.rend(); it++)
        {
            auto& entity = it->second;
            const auto& [ lightComp, transform ] = registry.get<LightComponent, TransformComponent>(entity);

            LightObjectPushConstant push{};
            {
                push.Position = Vec4( transform.Translation, lightComp.IsPoint ? -1.0f : lightComp.Cutoff );
                push.Color    = Vec4( lightComp.Color, lightComp.Intensity );
                push.Radius   = lightComp.Radius;
            }


            m_Pipeline->pushConstants(frameInfo.CommandBuffer, 
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 
                &push, 
                sizeof(LightObjectPushConstant), 
                0
            );

            // Draw quad.
            vkCmdDraw(frameInfo.CommandBuffer, 6, 1, 0, 0);
        }

        // Directional Light Data
        LightObjectPushConstant push{};
        {
            push.Position = Vec4(
                -globalUBO.DirectionalLight.Direction.x, 
                -globalUBO.DirectionalLight.Direction.y, 
                -globalUBO.DirectionalLight.Direction.z, 
                 globalUBO.DirectionalLight.Direction.w
            );
            push.Color    =        globalUBO.DirectionalLight.Color;
            push.Radius   = 5.0f * globalUBO.DirectionalLight.Color.w;
        }

        m_Pipeline->pushConstants(frameInfo.CommandBuffer, 
            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 
            &push, 
            sizeof(LightObjectPushConstant), 
            0
        );

        // Draw quad.
        vkCmdDraw(frameInfo.CommandBuffer, 6, 1, 0, 0);
    }


    void VyPointLightSystem::createPipeline(VkRenderPass renderpass, VkDescriptorSetLayout globalSetLayout)
    {
        auto builder = VyPipeline::GraphicsBuilder{};
        {
            builder.setName( "point_light" );
            
            builder.addDescriptorSetLayout( globalSetLayout );
            builder.addPushConstantRange(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(LightObjectPushConstant));
            
            builder.addShaderStage(VK_SHADER_STAGE_VERTEX_BIT,   "Light.vert.spv");
            builder.addShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT, "Light.frag.spv");
            
            // Enable Aplha Blending
            builder.addColorAttachment(VK_FORMAT_R16G16B16A16_SFLOAT, true);
            builder.setDepthAttachment(VK_FORMAT_D32_SFLOAT);

            // Set multisampled.
            builder.setRasterizationSamples( VyContext::device().msaaSampleCountFlagBits() );

            builder.setDepthTest(true, false, VK_COMPARE_OP_LESS_OR_EQUAL);
            builder.setCullMode(VK_CULL_MODE_FRONT_BIT);

            // Clear vertex bindings and attributes.
            builder.clearVertexDescriptions();
            
            builder.setRenderPass( renderpass );
        }

        m_Pipeline = builder.buildPtr();
    }
}


// namespace Vy
// {
//     VyPointLightSystem::VyPointLightSystem(
//         VkRenderPass          renderPass, 
//         VkDescriptorSetLayout gBufferSetLayout, 
//         VkDescriptorSetLayout compositionSetLayout)
//     {
//         createPointLightPipeline(renderPass, gBufferSetLayout, compositionSetLayout);
//     }

//     VyPointLightSystem::~VyPointLightSystem()
//     {

//     }

//     void VyPointLightSystem::createPointLightPipeline(
//         VkRenderPass          renderPass, 
//         VkDescriptorSetLayout gBufferSetLayout, 
//         VkDescriptorSetLayout compositionSetLayout)
//     {
//         TVector<VkDescriptorSetLayout> layouts = { gBufferSetLayout, compositionSetLayout };

//         auto builder = VyPipeline::GraphicsBuilder{};
//         {
//             builder.setName( "point_light" );
            
//             builder.addDescriptorSetLayouts( layouts );
            
//             builder.addShaderStage(VK_SHADER_STAGE_VERTEX_BIT,   "PointLight.vert.spv");
//             builder.addShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT, "PointLight.frag.spv");
            
//             // Enable Aplha Blending
//             // builder.addColorAttachment(VK_FORMAT_R16G16B16A16_SFLOAT, true);
//             // builder.setDepthAttachment(VK_FORMAT_D32_SFLOAT);
//             // builder.setDepthTest(true, false);
//             // builder.setCullMode(VK_CULL_MODE_BACK_BIT);

//             // Clear vertex bindings and attributes.
//             builder.clearVertexDescriptions();
            
//             builder.setRenderPass( renderPass );

//             builder.setSubpass( 1 );
//         }

//         m_Pipeline = builder.buildPtr();
//     }

// 	void VyPointLightSystem::render(VyFrameInfo& frameInfo) 
//     {
// 		m_Pipeline->bind(frameInfo.CommandBuffer);
        
//         // Set: 0 - GBuffer Set
//         m_Pipeline->bindDescriptorSet(frameInfo.CommandBuffer, 
//             0, 
//             frameInfo.GBufferSet
//         );

//         // Set: 1 - Composition Set
//         m_Pipeline->bindDescriptorSet(frameInfo.CommandBuffer, 
//             1, 
//             frameInfo.CompositionSet
//         );

//         // Draw quad.
// 		vkCmdDraw(frameInfo.CommandBuffer, 6, 1, 0, 0);
// 	}
// }