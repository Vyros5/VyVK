#include <VyEngine/GFX/Systems/LightSystem.h>

#include <VyEngine/VK/Context.h>
#include <VyEngine/Globals.h>

#include <VyEngine/GFX/Resources/Texture/Texture.h>
#include <VyEngine/Scene/ECS/Components.h>

namespace Vy
{
    struct PointLightPushConstantData 
    {
        Vec4  Position{}; // Ignore W
        Vec4  Color   {}; // RGB: Color, A: Intensity
        float Radius  {};
    };

    struct DirectionalLightPushConstantData
    {
        Mat4 ModelMatrix{};
        Vec4 Color      {};
    };

    struct SpotLightPushConstantData
    {
        Mat4  ModelMatrix{};
        Vec4  Color      {};
        float ConeAngle  {};
    };


    VyLightSystem::VyLightSystem(
        VkRenderPass          renderPass, 
        VkDescriptorSetLayout globalSetLayout)
    {
        createPointLightPipeline      ( renderPass, globalSetLayout );
        // createDirectionalLightPipeline( renderPass, globalSetLayout );
        // createSpotLightPipeline       ( renderPass, globalSetLayout );
    }


    VyLightSystem::~VyLightSystem() 
    {
    }


    void VyLightSystem::createPointLightPipeline(VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout)
    {
        // auto alphaBlendState = VyPipeline::alphaBlendAttachmentState();

        auto builder = VyPipeline::GraphicsBuilder{};
        {
            builder.setName( "point_light" );
            
            builder.addDescriptorSetLayout( globalSetLayout );
            builder.addPushConstantRange(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(PointLightPushConstantData));
            
            builder.addShaderStage(VK_SHADER_STAGE_VERTEX_BIT,   "Lighting/PointLight.vert.spv");
            builder.addShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT, "Lighting/PointLight.frag.spv");
            
            // Enable Aplha Blending
            builder.addColorAttachment(VK_FORMAT_R16G16B16A16_SFLOAT, true);
            builder.setDepthAttachment(VK_FORMAT_D32_SFLOAT);
            builder.setDepthTest(true, false);
            // builder.setCullMode(VK_CULL_MODE_BACK_BIT);

            // Clear vertex bindings and attributes.
            builder.clearVertexDescriptions();
            
            builder.setRenderPass( renderPass );
        }

        m_PointPipeline = builder.buildPtr();
    }


    // void VyLightSystem::createDirectionalLightPipeline(VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout)
    // {
    //     auto builder = VyPipeline::GraphicsBuilder{};
    //     {
    //         builder.setName("directional_light");
            
    //         builder.addDescriptorSetLayout( globalSetLayout );
    //         builder.addPushConstantRange(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(DirectionalLightPushConstantData));
            
    //         builder.addShaderStage(VK_SHADER_STAGE_VERTEX_BIT,   "Lighting/DirectionalLight.vert.spv");
    //         builder.addShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT, "Lighting/DirectionalLight.frag.spv");
            
    //         builder.addColorAttachment(VK_FORMAT_R16G16B16A16_SFLOAT);
            
    //         builder.setTopology(VK_PRIMITIVE_TOPOLOGY_LINE_LIST);

    //         // Clear vertex bindings and attributes.
    //         builder.clearVertexDescriptions();
            
    //         builder.setRenderPass( renderPass );
    //     }

    //     m_DirectionalPipeline = builder.buildPtr();
    // }


    // void VyLightSystem::createSpotLightPipeline(VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout)
    // {
    //     auto builder = VyPipeline::GraphicsBuilder{};
    //     {
    //         builder.setName("spot_light");
            
    //         builder.addDescriptorSetLayout( globalSetLayout );
    //         builder.addPushConstantRange(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(SpotLightPushConstantData));
            
    //         builder.addShaderStage(VK_SHADER_STAGE_VERTEX_BIT,   "Lighting/SpotLight.vert.spv");
    //         builder.addShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT, "Lighting/SpotLight.frag.spv");
            
    //         // Enable alpha blending.
    //         builder.addColorAttachment(VK_FORMAT_R16G16B16A16_SFLOAT, true);
            
    //         builder.setTopology(VK_PRIMITIVE_TOPOLOGY_LINE_LIST);
    //         builder.setCullMode(VK_CULL_MODE_NONE);

    //         builder.setDepthWriteEnable(false);

    //         // Clear vertex bindings and attributes.
    //         builder.clearVertexDescriptions();
            
    //         builder.setRenderPass( renderPass );
    //     }

    //     m_SpotPipeline = builder.buildPtr();
    // }


    void VyLightSystem::update(VyFrameInfo& frameInfo, GlobalUbo& ubo)
    {
        auto& registry = frameInfo.Scene->registry();

        m_SortedLights.clear();

        Vec3 camPos = frameInfo.Camera.position();
        
        int lightIndex = 0;
        auto pointView = registry.view<PointLightComponent, TransformComponent>();
        
        for (auto&& [ entity, pointLight, transform ] : pointView.each())
        {
            VY_ASSERT(lightIndex < MAX_POINT_LIGHTS, "Exceeded maximum point light index!");

            // Calculate distance
            Vec3 offset = camPos - transform.Translation;
            
            // dot product to get distance squared, less expensive than sqrt.
            float disSquared = glm::dot( offset, offset );
            
            m_SortedLights.insert({ disSquared, entity });

            lightIndex++;
        }

        // Clear light counts.
        ubo.PointLightsCount = 0;

        // ----------------------------------------------------------------------------------------
        // [ Process Point Lights ]
        
        auto rotateLight = glm::rotate(Mat4(1.0f), frameInfo.FrameTime /** m_RotationSpeed*/, Vec3(0.0f, -1.0f, 0.0f)); // Axis of rotation
        // auto pointView   = registry.view<PointLightComponent, TransformComponent>();
        
        TMap<float, EntityHandle>::reverse_iterator it;
        
        lightIndex = 0;
        for (it = m_SortedLights.rbegin(); it != m_SortedLights.rend(); it++)
        {
            auto entity = it->second;
            const auto& [ pointLight, transform ] = pointView.get<PointLightComponent, TransformComponent>(entity);

            // Update light Position. 
            transform.Translation = Vec3( rotateLight * Vec4( transform.Translation, 1.0f ) );

            // Copy light to UBO.
            {
                ubo.PointLights[ ubo.PointLightsCount ].Position = Vec4( transform.Translation, 1.0f );
                ubo.PointLights[ ubo.PointLightsCount ].Color    = Vec4( pointLight.Color, pointLight.Intensity );
            }

            lightIndex++;
        }

        ubo.PointLightsCount = lightIndex;

        // for (auto&& [ entity, pointLight, transform ] : pointView.each())
        // {
        //     VY_ASSERT(ubo.LightCount < MAX_POINT_LIGHTS, "Exceeded maximum point light count!");

        //     // Update light Position. 
        //     transform.Translation = Vec3( rotateLight * Vec4( transform.Translation, 1.0f ) );

        //     // Copy light to UBO.
        //     {
        //         ubo.PointLights[ ubo.LightCount ].Position = Vec4( transform.Translation, 1.0f );
        //         ubo.PointLights[ ubo.LightCount ].Color    = Vec4( pointLight.Color, pointLight.Intensity );
        //     }

        //     // Increment Point Lights
        //     ubo.LightCount++;
        // }
        
        // ----------------------------------------------------------------------------------------
        // [ Process Directional Lights ]

        // auto dirView = registry.view<DirectionalLightComponent, TransformComponent>();

        // for (auto&& [ entity, dirLight, transform ] : dirView.each())
        // {
        //     VY_ASSERT(ubo.DirectionalLightCount < MAX_DIRECT_LIGHTS, "Exceeded maximum directional light count!");

        //     // Update rotation to look at target if enabled.
        //     if (dirLight.UseTargetPoint)
        //     {
        //         transform.lookAt( dirLight.TargetPoint );
        //     }

        //     Vec3 direction = transform.forward();

        //     // Copy light to UBO.
        //     {
        //         ubo.DirectionalLights[ ubo.DirectionalLightCount ].Direction = Vec4( glm::normalize( direction ), 0.0f );
        //         ubo.DirectionalLights[ ubo.DirectionalLightCount ].Color     = Vec4( dirLight.Color, dirLight.Intensity );
        //     }

        //     // Increment Directional Lights
        //     ubo.DirectionalLightCount++;
        // }

        // // ----------------------------------------------------------------------------------------
        // // [ Process Spot Lights ]

        // auto spotView = registry.view<SpotLightComponent, TransformComponent>();

        // for (auto&& [ entity, spotLight, transform ] : spotView.each())
        // {
        //     VY_ASSERT(ubo.SpotLightCount < MAX_SPOT_LIGHTS, "Exceeded maximum spot light count!");

        //     // Update rotation to look at target if enabled.
        //     if (spotLight.UseTargetPoint)
        //     {
        //         transform.lookAt( spotLight.TargetPoint );
        //     }

        //     Vec3 rawDir    = transform.forward();
        //     Vec4 direction = Vec4( glm::normalize( rawDir ), glm::cos( glm::radians( spotLight.InnerCutoffAngle ) ) );

        //     // Copy light to UBO.
        //     {
        //         ubo.SpotLights[ ubo.SpotLightCount ].Position       = Vec4( transform.Translation, 1.0f );
        //         ubo.SpotLights[ ubo.SpotLightCount ].Direction      = direction;
        //         ubo.SpotLights[ ubo.SpotLightCount ].Color          = Vec4( spotLight.Color, spotLight.Intensity );
        //         ubo.SpotLights[ ubo.SpotLightCount ].OuterCutoff    = glm::cos( glm::radians( spotLight.OuterCutoffAngle ) );
        //         ubo.SpotLights[ ubo.SpotLightCount ].ConstantAtten  = spotLight.ConstantAttenuation;
        //         ubo.SpotLights[ ubo.SpotLightCount ].LinearAtten    = spotLight.LinearAttenuation;
        //         ubo.SpotLights[ ubo.SpotLightCount ].QuadraticAtten = spotLight.QuadraticAttenuation;
        //     }

        //     // Increment Spot Lights
        //     ubo.SpotLightCount++;
        // }
    }


    void VyLightSystem::render(VyFrameInfo& frameInfo) 
    {
        auto& registry = frameInfo.Scene->registry();

        // ----------------------------------------------------------------------------------------

        
        
        // TMap<float, EntityHandle> sorted;

        // auto pointView = registry.view<PointLightComponent, TransformComponent>();
        
        // for (auto&& [ entity, pointLight, transform ] : pointView.each())
        // {
        //     Vec3 camPos = frameInfo.Camera.position();

        //     //calculate distance
        //     Vec3 offset = camPos - transform.Translation;
            
        //     // dot product to get distance squared, less expensive than sqrt.
        //     float disSquared = glm::dot( offset, offset );
            
        //     sorted[ disSquared ] = entity;
        // }

		// [ Alpha Sorting ]
		// Transparent entities need to be drawn from back to front.
		// Vec3 camPos = frameInfo.Camera.position();

		// registry.sort<TransformComponent>(
        //     [&](const TransformComponent& lhs, const TransformComponent& rhs) 
        //     {
        //         Vec3  lhsOffset     = camPos - lhs.Translation;
        //         float lhsDisSquared = glm::dot( lhsOffset, lhsOffset );

        //         Vec3  rhsOffset     = camPos - rhs.Translation;
        //         float rhsDisSquared = glm::dot( rhsOffset, rhsOffset );

        //         // Need to sort so that closest entities are last.
        //         return lhsDisSquared > rhsDisSquared;
		// 	}
        // );

        // ----------------------------------------------------------------------------------------

        // Render point lights.
        m_PointPipeline->bind( frameInfo.CommandBuffer );

        // Set: 0 - Global Set
        m_PointPipeline->bindDescriptorSet(frameInfo.CommandBuffer, 
            0, 
            frameInfo.GlobalSet
        );

		// Create push-constant data and send it to the device and draw.
		// Accessing in order of TransformComponent which was sorted above.
        // auto pointView = registry.view<PointLightComponent, TransformComponent>();
        
        // for (auto&& [ entity, pointLight, transform ] : pointView.each())
        // for (auto& [ _, entity ] : std::ranges::reverse_view(sorted))
        TMap<float, EntityHandle>::reverse_iterator it;

        for (it = m_SortedLights.rbegin(); it != m_SortedLights.rend(); it++)
        {
            auto entity = it->second;
            const auto& [ pointLight, transform ] = registry.get<PointLightComponent, TransformComponent>(entity);

            PointLightPushConstantData push{};
            {
                push.Position = Vec4( transform.Translation, 1.0f );
                push.Color    = Vec4( pointLight.Color, pointLight.Intensity );
                push.Radius   = pointLight.Radius; // transform.Scale.x; // 
            }

            m_PointPipeline->pushConstants(frameInfo.CommandBuffer, 
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 
                &push, 
                sizeof(PointLightPushConstantData), 
                0
            );

            // Inefficient to draw a quad for each light, but okay for demo purposes.
            vkCmdDraw(frameInfo.CommandBuffer, 6, 1, 0, 0);
        }

        // ----------------------------------------------------------------------------------------

        // // Render directional lights as arrows.
        // m_DirectionalPipeline->bind(frameInfo.CommandBuffer);

        // // Set: 0 - Global Set
        // m_DirectionalPipeline->bindDescriptorSet(frameInfo.CommandBuffer, 
        //     0, 
        //     frameInfo.GlobalSet
        // );

        // auto dirView = registry.view<DirectionalLightComponent, TransformComponent>();

        // for (auto&& [ entity, dirLight, transform ] : dirView.each())
        // {
        //     // Create a model matrix that orients the arrow in the light direction.
        //     Mat4 modelMatrix = Mat4( 1.0f );
        //     modelMatrix      = glm::translate( modelMatrix, transform.Translation );

        //     // Apply rotation to orient arrow.
        //     modelMatrix = glm::rotate( modelMatrix, transform.Rotation.y, Vec3( 0.0f, 1.0f, 0.0f ) );
        //     modelMatrix = glm::rotate( modelMatrix, transform.Rotation.x, Vec3( 1.0f, 0.0f, 0.0f ) );
        //     modelMatrix = glm::rotate( modelMatrix, transform.Rotation.z, Vec3( 0.0f, 0.0f, 1.0f ) );

        //     DirectionalLightPushConstantData push{};
        //     {
        //         push.ModelMatrix = modelMatrix;
        //         push.Color       = Vec4( dirLight.Color, dirLight.Intensity );
        //     }

        //     m_DirectionalPipeline->pushConstants(frameInfo.CommandBuffer, 
        //         VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 
        //         &push
        //     );

        //     // 18 vertices for arrow.
        //     vkCmdDraw(frameInfo.CommandBuffer, 18, 1, 0, 0);
        // }

        // // ----------------------------------------------------------------------------------------

        // // Render spot lights as cones.
        // m_SpotPipeline->bind(frameInfo.CommandBuffer);

        // // Set: 0 - Global Set
        // m_SpotPipeline->bindDescriptorSet(frameInfo.CommandBuffer, 
        //     0, 
        //     frameInfo.GlobalSet
        // );

        // auto spotView = registry.view<SpotLightComponent, TransformComponent>();

        // for (auto&& [ entity, spotLight, transform ] : spotView.each())
        // {
        //     // Create a model matrix that positions and orients the cone.
        //     Mat4 modelMatrix = Mat4( 1.0f );
        //     modelMatrix      = glm::translate( modelMatrix, transform.Translation );

        //     // Apply rotation to orient cone.
        //     modelMatrix = glm::rotate( modelMatrix, transform.Rotation.y, Vec3( 0.0f, 1.0f, 0.0f ) );
        //     modelMatrix = glm::rotate( modelMatrix, transform.Rotation.x, Vec3( 1.0f, 0.0f, 0.0f ) );
        //     modelMatrix = glm::rotate( modelMatrix, transform.Rotation.z, Vec3( 0.0f, 0.0f, 1.0f ) );

        //     SpotLightPushConstantData push{};
        //     {
        //         push.ModelMatrix = modelMatrix;
        //         push.Color       = Vec4( spotLight.Color, spotLight.Intensity );
        //         push.ConeAngle   = glm::radians( spotLight.OuterCutoffAngle );
        //     }

        //     m_SpotPipeline->pushConstants(frameInfo.CommandBuffer, 
        //         VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 
        //         &push
        //     );

        //     // Draw cone: 32 segments * 3 vertices per triangle = 96 vertices.
        //     vkCmdDraw(frameInfo.CommandBuffer, 96, 1, 0, 0);
        // }
    }
}