#include <Vy/Systems/Rendering/LightSystem.h>

#include <Vy/GFX/Context.h>

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
        createPointLightPipeline      (renderPass, globalSetLayout);
        createDirectionalLightPipeline(renderPass, globalSetLayout);
        createSpotLightPipeline       (renderPass, globalSetLayout);
    }


    VyLightSystem::~VyLightSystem()
    {
    }


    // void VyLightSystem::createPipeline(VkRenderPass& renderPass, TVector<VkDescriptorSetLayout> descSetLayouts)
    // {
    //     m_Pipeline = VyPipeline::GraphicsBuilder{}
    //         .addDescriptorSetLayouts(descSetLayouts)
    //         .addPushConstantRange(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(PointLightPushConstantData))
    //         .addShaderStage(VK_SHADER_STAGE_VERTEX_BIT,   "PointLight.vert.spv")
    //         .addShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT, "PointLight.frag.spv")
    //         .addColorAttachment(VK_FORMAT_R16G16B16A16_SFLOAT, true)
    //         .setDepthAttachment(VK_FORMAT_D32_SFLOAT)
    //         .setDepthTest(true, false)
    //         .setRenderPass(renderPass)
	// 		.clearVertexDescriptions() // Clear default vertex bindings and attributes.
    //     .buildUnique();
    // }


    void VyLightSystem::update(VyFrameInfo& frameInfo, GlobalUBO& ubo) 
    {
        ubo.NumPointLights       = 0;
        ubo.NumDirectionalLights = 0;
        ubo.NumSpotLights        = 0;

        auto rotateLight = glm::rotate(Mat4(1.0f), frameInfo.FrameTime * m_RotationSpeed, Vec3(0.0f, -1.0f, 0.0f)); // Axis of rotation

        auto& registry = frameInfo.Scene->registry();
        
        // ----------------------------------------------------------------------------------------

        // [ Process point lights ]
        auto pointView = registry.view<PointLightComponent, TransformComponent>();

        for (auto&& [ entity, pointLight, transform ] : pointView.each())
        {
            VY_ASSERT(ubo.NumPointLights < MAX_LIGHTS, "Exceeded maximum point light count!");

            // Update light Position. (Temporary)
            transform.Translation = Vec3(rotateLight * Vec4(transform.Translation, 1.0f));

            // Copy light to UBO.
            {
                ubo.PointLights[ubo.NumPointLights].Position = Vec4(transform.Translation, 1.0f);
                ubo.PointLights[ubo.NumPointLights].Color    = Vec4(pointLight.Color, pointLight.Intensity);
            }

            // Increment Point Lights
            ubo.NumPointLights++;
        }
        
        // ----------------------------------------------------------------------------------------

        // [ Process directional lights ]
        auto dirView = registry.view<DirectionalLightComponent, TransformComponent>();

        for (auto&& [ entity, dirLight, transform ] : dirView.each())
        {
            VY_ASSERT(ubo.NumDirectionalLights < MAX_LIGHTS, "Exceeded maximum directional light count!");

            // Update rotation to look at target if enabled.
            if (dirLight.UseTargetPoint)
            {
                transform.lookAt(dirLight.TargetPoint);
            }

            Vec3 direction = transform.forward();

            // Copy light to UBO.
            {
                ubo.DirectionalLights[ubo.NumDirectionalLights].Direction = Vec4(glm::normalize(direction), 0.0f);
                ubo.DirectionalLights[ubo.NumDirectionalLights].Color     = Vec4(dirLight.Color, dirLight.Intensity);
            }

            // Increment Directional Lights
            ubo.NumDirectionalLights++;
        }

        // ----------------------------------------------------------------------------------------

        // [ Process spot lights ]
        auto spotView = registry.view<SpotLightComponent, TransformComponent>();

        for (auto&& [ entity, spotLight, transform ] : spotView.each())
        {
            VY_ASSERT(ubo.NumSpotLights < MAX_LIGHTS, "Exceeded maximum spot light count!");

            // Update rotation to look at target if enabled.
            if (spotLight.UseTargetPoint)
            {
                transform.lookAt(spotLight.TargetPoint);
            }

            Vec3 direction = transform.forward();

            // Copy light to UBO.
            {
                ubo.SpotLights[ubo.NumSpotLights].Position       = Vec4(transform.Translation, 1.0f);
                ubo.SpotLights[ubo.NumSpotLights].Direction      = Vec4(glm::normalize(direction), glm::cos(glm::radians(spotLight.InnerCutoffAngle)));
                ubo.SpotLights[ubo.NumSpotLights].Color          = Vec4(spotLight.Color, spotLight.Intensity);
                ubo.SpotLights[ubo.NumSpotLights].OuterCutoff    = glm::cos(glm::radians(spotLight.OuterCutoffAngle));
                ubo.SpotLights[ubo.NumSpotLights].ConstantAtten  = spotLight.ConstantAttenuation;
                ubo.SpotLights[ubo.NumSpotLights].LinearAtten    = spotLight.LinearAttenuation;
                ubo.SpotLights[ubo.NumSpotLights].QuadraticAtten = spotLight.QuadraticAttenuation;
            }

            // Increment Spot Lights
            ubo.NumSpotLights++;
        }
    }


    void VyLightSystem::updateTargetLockedLight(EntityHandle entity, VyScene* scene)
    {
        auto& registry = scene->registry();

        // Update directional light target tracking
        if (registry.all_of<DirectionalLightComponent>(entity))
        {
            auto& dirLight = registry.get<DirectionalLightComponent>(entity);

            if (dirLight.UseTargetPoint)
            {
                registry.get<TransformComponent>(entity).lookAt(dirLight.TargetPoint);
            }
        }

        // Update spot light target tracking
        if (registry.all_of<SpotLightComponent>(entity))
        {
            auto& spotLight = registry.get<SpotLightComponent>(entity);

            if (spotLight.UseTargetPoint)
            {
                registry.get<TransformComponent>(entity).lookAt(spotLight.TargetPoint);
            }
        }
    }


    void VyLightSystem::render(const VyFrameInfo& frameInfo) 
    {
        auto& registry = frameInfo.Scene->registry();

        // ----------------------------------------------------------------------------------------

        // Render point lights.
        m_PointPipeline->bind(frameInfo.CommandBuffer);

        // Set: 0 - Global Descriptor Set
        m_PointPipeline->bindDescriptorSet(frameInfo.CommandBuffer, 0, frameInfo.GlobalDescriptorSet);

        auto pointView = registry.view<PointLightComponent, TransformComponent>();

        for (auto&& [ entity, pointLight, transform ] : pointView.each())
        {
            PointLightPushConstantData push{};
            {
                push.Position = Vec4( transform.Translation, 1.0f );
                push.Color    = Vec4( pointLight.Color, pointLight.Intensity );
                push.Radius   = transform.Scale.x; 
            }

            m_PointPipeline->pushConstants(frameInfo.CommandBuffer, 
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 
                push
            );
            
            // inefficient to draw a quad for each light, but okay for demo purposes.
            vkCmdDraw(frameInfo.CommandBuffer, 6, 1, 0, 0);
        }

        // ----------------------------------------------------------------------------------------

        // Render directional lights as arrows.
        m_DirectionalPipeline->bind(frameInfo.CommandBuffer);

        // Set: 0 - Global Descriptor Set
        m_DirectionalPipeline->bindDescriptorSet(frameInfo.CommandBuffer, 0, frameInfo.GlobalDescriptorSet);

        auto dirView = registry.view<DirectionalLightComponent, TransformComponent>();

        for (auto&& [ entity, dirLight, transform ] : dirView.each())
        {
            // Create a model matrix that orients the arrow in the light direction.
            Mat4 modelMatrix = Mat4(1.0f);
            modelMatrix      = glm::translate(modelMatrix, transform.Translation);

            // Apply rotation to orient arrow
            modelMatrix = glm::rotate(modelMatrix, transform.Rotation.y, Vec3(0.0f, 1.0f, 0.0f));
            modelMatrix = glm::rotate(modelMatrix, transform.Rotation.x, Vec3(1.0f, 0.0f, 0.0f));
            modelMatrix = glm::rotate(modelMatrix, transform.Rotation.z, Vec3(0.0f, 0.0f, 1.0f));

            DirectionalLightPushConstantData push{};
            {
                push.ModelMatrix = modelMatrix;
                push.Color       = Vec4(dirLight.Color, dirLight.Intensity);
            }

            m_DirectionalPipeline->pushConstants(frameInfo.CommandBuffer, 
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 
                push
            );

            // 18 vertices for arrow.
            vkCmdDraw(frameInfo.CommandBuffer, 18, 1, 0, 0);
        }

        // ----------------------------------------------------------------------------------------

        // Render spot lights as cones.
        m_SpotPipeline->bind(frameInfo.CommandBuffer);

        // Set: 0 - Global Descriptor Set
        m_SpotPipeline->bindDescriptorSet(frameInfo.CommandBuffer, 0, frameInfo.GlobalDescriptorSet);

        auto spotView = registry.view<SpotLightComponent, TransformComponent>();

        for (auto&& [ entity, spotLight, transform ] : spotView.each())
        {
            // Create a model matrix that positions and orients the cone.
            Mat4 modelMatrix = Mat4(1.0f);
            modelMatrix      = glm::translate(modelMatrix, transform.Translation);

            // Apply rotation to orient cone.
            modelMatrix = glm::rotate(modelMatrix, transform.Rotation.y, Vec3(0.0f, 1.0f, 0.0f));
            modelMatrix = glm::rotate(modelMatrix, transform.Rotation.x, Vec3(1.0f, 0.0f, 0.0f));
            modelMatrix = glm::rotate(modelMatrix, transform.Rotation.z, Vec3(0.0f, 0.0f, 1.0f));

            SpotLightPushConstantData push{};
            {
                push.ModelMatrix = modelMatrix;
                push.Color       = Vec4(spotLight.Color, spotLight.Intensity);
                push.ConeAngle   = glm::radians(spotLight.OuterCutoffAngle);
            }

            m_SpotPipeline->pushConstants(frameInfo.CommandBuffer, 
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 
                push
            );

            // Draw cone base circle (16 vertices) + 8 lines from apex = 24 vertices.
            vkCmdDraw(frameInfo.CommandBuffer, 25, 1, 0, 0);
        }
    }



    void VyLightSystem::createPointLightPipeline(VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout)
    {
        m_PointPipeline = VyPipeline::GraphicsBuilder{}
            .addDescriptorSetLayout(globalSetLayout)
            .addPushConstantRange(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(PointLightPushConstantData))
            .addShaderStage(VK_SHADER_STAGE_VERTEX_BIT,   "Lighting/PointLight.vert.spv")
            .addShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT, "Lighting/PointLight.frag.spv")
            // .addColorAttachment(VK_FORMAT_R16G16B16A16_SFLOAT, true)
            // .setDepthAttachment(VK_FORMAT_D32_SFLOAT)
            // .setTopology(VK_PRIMITIVE_TOPOLOGY_LINE_LIST)
            // .setDepthTest(true, false)
            .setRenderPass(renderPass)
			.clearVertexDescriptions() // Clear default vertex bindings and attributes.
        .buildUnique();
    }


    void VyLightSystem::createDirectionalLightPipeline(VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout)
    {
        m_DirectionalPipeline = VyPipeline::GraphicsBuilder{}
            .addDescriptorSetLayout(globalSetLayout)
            .addPushConstantRange(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(DirectionalLightPushConstantData))
            .addShaderStage(VK_SHADER_STAGE_VERTEX_BIT,   "Lighting/DirectionalLight.vert.spv")
            .addShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT, "Lighting/DirectionalLight.frag.spv")
            // .addColorAttachment(VK_FORMAT_R16G16B16A16_SFLOAT, true)
            .setDepthAttachment(VK_FORMAT_D32_SFLOAT)
            .setTopology(VK_PRIMITIVE_TOPOLOGY_LINE_LIST)
            // .setDepthTest(true, false)
            .setRenderPass(renderPass)
			.clearVertexDescriptions() // Clear default vertex bindings and attributes.
        .buildUnique();
    }


    void VyLightSystem::createSpotLightPipeline(VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout)
    {
        m_SpotPipeline = VyPipeline::GraphicsBuilder{}
            .addDescriptorSetLayout(globalSetLayout)
            .addPushConstantRange(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(SpotLightPushConstantData))
            .addShaderStage(VK_SHADER_STAGE_VERTEX_BIT,   "Lighting/SpotLight.vert.spv")
            .addShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT, "Lighting/SpotLight.frag.spv")
            .addColorAttachment(VK_FORMAT_R16G16B16A16_SFLOAT, true)
            .setDepthAttachment(VK_FORMAT_D32_SFLOAT)
            .setTopology(VK_PRIMITIVE_TOPOLOGY_LINE_LIST)
            // .setDepthTest(true, false)
            .setRenderPass(renderPass)
			.clearVertexDescriptions() // Clear default vertex bindings and attributes.
        .buildUnique();
    }
}





    // void VyLightSystem::render(const VyFrameInfo& frameInfo, U32 instanceCount)
    // {
    //     m_Pipeline->bind(frameInfo.CommandBuffer);

    //     m_Pipeline->bindDescriptorSet(frameInfo.CommandBuffer, 0, frameInfo.GlobalDescriptorSet);
    //     m_Pipeline->bindDescriptorSet(frameInfo.CommandBuffer, 1, frameInfo.LightDescriptorSet);

    //     vkCmdDraw(frameInfo.CommandBuffer, 6, instanceCount, 0, 0);
    // }


// // Calculate direction from rotation (forward is -Z in our coordinate system)
// Mat4 rotationMatrix = glm::rotate(Mat4(1.0f),     transform.Rotation.y, Vec3(0.0f, 1.0f, 0.0f));
// rotationMatrix      = glm::rotate(rotationMatrix, transform.Rotation.x, Vec3(1.0f, 0.0f, 0.0f));
// rotationMatrix      = glm::rotate(rotationMatrix, transform.Rotation.z, Vec3(0.0f, 0.0f, 1.0f));

// Vec3 direction = glm::normalize(Vec3(rotationMatrix * Vec4(0.0f, 0.0f, -1.0f, 0.0f)));


  // void VyLightSystem::render(const VyFrameInfo& frameInfo) 
    // {
    //     // Sort lights by distance to camera.
    //     // Transparent entities need to be drawn from back to front.
        
    //     //TODO: Should do this for every transparent object or use order independent transparency.

    //     TMap<float, EntityHandle> sortedLights;

    //     auto view = frameInfo.Scene->getEntitiesWith<PointLightComponent, TransformComponent>();

    //     for (auto&& [ entity, light, transform ] : view.each())
    //     {
    //         // Calculate distance.
    //         auto offset = frameInfo.Camera.Position() - transform.Translation;
    //         // auto offset = frameInfo.Scene->mainCamera().get<TransformComponent>().Translation - transform.Translation;

    //         // Dot product to get distance squared, less expensive than sqrt().
    //         float disSquared = glm::dot(offset, offset);

    //         sortedLights[ disSquared ] = entity;
    //     }

    //     m_Pipeline->bind(frameInfo.CommandBuffer);

    //     m_Pipeline->bindDescriptorSet(frameInfo.CommandBuffer, 0, frameInfo.GlobalDescriptorSet);

    //     // Iterate through sorted lights in reverse order.
    //     for (auto& [ _, entity ] : std::ranges::reverse_view(sortedLights))
    //     {
    //         const auto& [ light, transform ] = view.get<PointLightComponent, TransformComponent>(entity);

    //         PointLightPushConstantData push{};
    //         {
    //             push.Position = Vec4( transform.Translation,  1.0f );
    //             push.Color    = Vec4( light.Color, light.Intensity );
    //             push.Radius   = transform.Scale.x; 
    //         }

    //         m_Pipeline->pushConstants(frameInfo.CommandBuffer, 
    //             VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 
    //             push
    //         );
            
    //         vkCmdDraw(frameInfo.CommandBuffer, 6, 1, 0, 0);
    //     }
    // }




    // void VyLightSystem::update(VyFrameInfo& frameInfo, GlobalUBO& ubo) 
    // {
    //     // Point Lights
    //     {
    //         // static float time = 0.0f;
    //         // time += frameInfo.FrameTime;

    //         // auto rotateLight = glm::rotate(Mat4(1.0f), m_RotationSpeed * frameInfo.FrameTime, { 0.0f, -1.0f, 0.0f }); // Axis of rotation

    //         int pointLightIndex = 0;

    //         auto pointLightView = frameInfo.Scene->getEntitiesWith<PointLightComponent, TransformComponent>();
            
    //         for (auto&& [ entity, light, transform ] : pointLightView.each())
    //         {
    //             VY_ASSERT(pointLightIndex < MAX_LIGHTS, "Point lights exceed maximum specified limit");

    //             // Animate the point lights
    //             // if (transform.Translation.x > -3.0f && transform.Translation.x < 3.0f) 
    //             // {
    //                 auto rotateLight = glm::rotate(
    //                     Mat4(1.0f),
    //                     frameInfo.FrameTime * m_RotationSpeed,
    //                     Vec3{ 0.0f, -1.0f, 0.0f } // Axis of rotation
    //                 );

    //                 // Update light Position. (Temporary)
    //                 transform.Translation = Vec3(rotateLight * Vec4(transform.Translation, 1.0f));
    //             // }
    //             // else if (transform.Translation.x > 7.0f && transform.Translation.x < 13.0f) 
    //             // {
    //             //     transform.Translation.x = 10.0f + sin(time * 1.0f) * 2.0f;
    //             // } 
    //             // else if (transform.Translation.x > -13.0f && transform.Translation.x < -7.0f) 
    //             // {
    //             //     float pendulumAngle = sin(time * 0.6f) * (glm::pi<float>() / 2.0f);
    //             //     Vec3  helmetCenter  = Vec3(-10.0f, 0.5f, 0.0f);
    //             //     float radius        = 2.0f;

    //             //     transform.Translation.x = helmetCenter.x - radius * sin(pendulumAngle);
    //             //     transform.Translation.y = helmetCenter.y - 1.0f;
    //             //     transform.Translation.z = helmetCenter.z - radius * cos(pendulumAngle) - 0.3f;
    //             // }

    //             // Copy light to UBO.
    //             {
    //                 ubo.PointLights[ pointLightIndex ].Position = Vec4(transform.Translation, 1.0f);
    //                 ubo.PointLights[ pointLightIndex ].Color    = Vec4(light.Color, light.Intensity);
    //             }

    //             pointLightIndex++;
    //         }

    //         ubo.NumPointLights = pointLightIndex;
    //     }

    //     // Directional Lights
    //     {
    //         int dirLightIndex = 0;

    //         auto dirLightView = frameInfo.Scene->getEntitiesWith<DirectionalLightComponent, TransformComponent>();

    //         for (auto&& [ entity, light, transform ] : dirLightView.each())
    //         {
    //             VY_ASSERT(dirLightIndex < MAX_LIGHTS, "Directional lights exceed maximum specified limit");

    //             // Copy light to UBO.
    //             {
    //                 ubo.DirectionalLights[ dirLightIndex ].Direction = Vec4(light.Direction, 1.0f);
    //                 ubo.DirectionalLights[ dirLightIndex ].Color     = Vec4(light.Color, light.Intensity);
    //             }

    //             dirLightIndex++;
    //         }

    //         ubo.NumDirectionalLights = dirLightIndex;
    //     }
    // }