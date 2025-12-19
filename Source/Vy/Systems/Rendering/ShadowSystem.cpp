#include <Vy/Systems/Rendering/ShadowSystem.h>

#include <Vy/GFX/Context.h>
#include <Vy/Globals.h>

namespace Vy
{
    struct ShadowPushConstants
    {
        Mat4 ModelMatrix;
        Mat4 LightSpaceMatrix;
    };

    struct CubeShadowPushConstants
    {
        Mat4 ModelMatrix;
        Mat4 LightSpaceMatrix;
        Vec4 LightPosAndFarPlane; // xyz = light position, w = far plane
    };

    // =====================================================================================================================

    VyShadowSystem::VyShadowSystem(U32 shadowMapSize) : 
        m_ShadowMapSize{ shadowMapSize }
    {
        // Create multiple shadow maps for directional/spot lights.
        for (int i = 0; i < MAX_SHADOW_MAPS; i++)
        {
            m_ShadowMaps.push_back( MakeUnique<VyShadowMap>(shadowMapSize, shadowMapSize) );

            m_LightSpaceMatrices[i] = Mat4(1.0f);
        }

        // Create cube shadow maps for point lights.
        for (int i = 0; i < MAX_CUBE_SHADOW_MAPS; i++)
        {
            m_CubeShadowMaps.push_back( MakeUnique<VyCubeShadowMap>(shadowMapSize) );

            m_PointLightPositions[i] = Vec3(0.0f);
            m_PointLightRanges   [i] = 25.0f;
        }

        createPipeline();
        createCubeShadowPipeline();

        VY_INFO_TAG("VyShadowSystem", "Initialized with {0} 2D shadow maps and {1} cube shadow maps ({2}x{3})",
            MAX_SHADOW_MAPS, MAX_CUBE_SHADOW_MAPS, shadowMapSize, shadowMapSize
        );
    }


    VyShadowSystem::~VyShadowSystem()
    {
    }

    // =====================================================================================================================

    void VyShadowSystem::createPipeline()
    {
        // Only need position for shadow mapping.

        m_Pipeline = VyPipeline::GraphicsBuilder{}
            .addPushConstantRange(VK_SHADER_STAGE_VERTEX_BIT, sizeof(ShadowPushConstants))
            .addShaderStage(VK_SHADER_STAGE_VERTEX_BIT,   "Shadows/Shadow.vert.spv")
            .addShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT, "Shadows/Shadow.frag.spv")

            // Cull front faces to reduce peter-panning
            .setCullMode(VK_CULL_MODE_FRONT_BIT)
            
            // Depth bias to prevent shadow acne
            .setDepthBias(/*ConstantFactor*/ 1.25f, /*Clamp*/ 0.0f, /*SlopeFactor*/ 1.75f)

            // No color attachment - depth only
            .setDepthAttachment(VK_FORMAT_D32_SFLOAT)

            // Use the render pass from the first shadow map (all are identical)
            .setRenderPass(m_ShadowMaps[0]->renderPass())

        .buildUnique();
    }

    // =====================================================================================================================

    Mat4 VyShadowSystem::calculateDirectionalLightMatrix(
        const Vec3& lightDirection, 
        const Vec3& sceneCenter, 
        float       sceneRadius)
    {
        // lightDirection points FROM light TO scene (the direction light travels).
        Vec3 lightDir = glm::normalize(lightDirection);
        Vec3 lightPos = sceneCenter - lightDir * sceneRadius * 3.0f;

        // Handle edge case where light is directly above/below.
        Vec3 up = Vec3(0.0f, 1.0f, 0.0f);

        if (glm::abs(glm::dot(lightDir, up)) > 0.99f)
        {
            up = Vec3(0.0f, 0.0f, 1.0f);
        }

        Mat4 lightView = glm::lookAt(lightPos, sceneCenter, up);

        // Orthographic projection that encompasses the scene.
        float orthoSize = sceneRadius * 2.0f;
        float nearPlane = 0.1f;
        float farPlane  = sceneRadius * 6.0f;

        Mat4 lightProj = glm::ortho(-orthoSize, orthoSize, -orthoSize, orthoSize, nearPlane, farPlane);

        // Vulkan clip space correction (Y flip, Z [0,1]).
        lightProj[1][1] *= -1;

        return lightProj * lightView;
    }

    // =====================================================================================================================

    Mat4 VyShadowSystem::calculateSpotLightMatrix(
        const Vec3& position, 
        const Vec3& direction, 
        float       outerCutoffDegrees, 
        float       range)
    {
        Vec3 lightDir = glm::normalize(direction);

        // Handle edge case where light points directly up/down.
        Vec3 up = Vec3(0.0f, 1.0f, 0.0f);

        if (glm::abs(glm::dot(lightDir, up)) > 0.99f)
        {
            up = Vec3(0.0f, 0.0f, 1.0f);
        }

        Mat4 lightView = glm::lookAt(position, position + lightDir, up);

        // Perspective projection based on spotlight cone angle
        // outerCutoffDegrees is the outer cone angle in degrees
        // FOV should be 2 * angle to cover the full cone, add some margin.
        float fov       = glm::radians(outerCutoffDegrees * 2.0f + 5.0f); // Add 5 degree margin
        float nearPlane = 0.1f;
        float farPlane  = range > 0.0f ? range : 100.0f;

        Mat4 lightProj = glm::perspective(fov, 1.0f, nearPlane, farPlane);

        // Vulkan clip space correction (Y flip).
        lightProj[1][1] *= -1;

        return lightProj * lightView;
    }

    // =====================================================================================================================

    void VyShadowSystem::renderToShadowMap(VyFrameInfo& frameInfo, VyShadowMap& shadowMap, const Mat4& lightSpaceMatrix)
    {
        // Begin shadow render pass.
        shadowMap.beginRenderPass(frameInfo.CommandBuffer);
        {
            // Bind shadow pipeline.
            m_Pipeline->bind(frameInfo.CommandBuffer);

            // Render all objects to shadow map.
            auto view = frameInfo.Scene->registry().view<ModelComponent, TransformComponent>();
            
            for (auto&& [entity, modelComp, transform] : view.each())
            {
                if (!modelComp.Model) continue;

                ShadowPushConstants push{};
                {
                    push.ModelMatrix      = transform.matrix();
                    push.LightSpaceMatrix = lightSpaceMatrix;
                }

                m_Pipeline->pushConstants(frameInfo.CommandBuffer, 
                    VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 
                    &push
                );

                modelComp.Model->bind(frameInfo.CommandBuffer);
                modelComp.Model->draw(frameInfo.CommandBuffer);
            }
        }
        // End shadow render pass.
        shadowMap.endRenderPass(frameInfo.CommandBuffer);
    }

    // =====================================================================================================================

    void VyShadowSystem::renderShadowMaps(VyFrameInfo& frameInfo, float sceneRadius)
    {
        m_ShadowLightCount = 0;
        Vec3 sceneCenter   = Vec3(0.0f);

        // Render shadow map for first directional light.
        auto dirView = frameInfo.Scene->registry().view<DirectionalLightComponent, TransformComponent>();
        
        for (auto&& [entity, dirLight, transform] : dirView.each())
        {
            if (m_ShadowLightCount >= MAX_SHADOW_MAPS) break;

            Vec3 lightDir = transform.forward();

            m_LightSpaceMatrices[m_ShadowLightCount] = calculateDirectionalLightMatrix(lightDir, sceneCenter, sceneRadius);
            
            renderToShadowMap(frameInfo, *m_ShadowMaps[m_ShadowLightCount], m_LightSpaceMatrices[m_ShadowLightCount]);

            m_ShadowLightCount++;

            // Only one directional light shadow for now? The old code took dirLights[0].
            // I'll break after one.
            break;
        }

        // Render shadow maps for spotlights.
        auto spotView = frameInfo.Scene->registry().view<SpotLightComponent, TransformComponent>();
        
        for (auto&& [entity, spotLight, transform] : spotView.each())
        {
            if (m_ShadowLightCount >= MAX_SHADOW_MAPS) break;

            Vec3 position  = transform.Translation;
            Vec3 direction = transform.forward();

            float outerCutoffDegrees = spotLight.OuterCutoffAngle;
            float range              = 50.0f;

            m_LightSpaceMatrices[m_ShadowLightCount] = calculateSpotLightMatrix(position, direction, outerCutoffDegrees, range);
            
            renderToShadowMap(frameInfo, *m_ShadowMaps[m_ShadowLightCount], m_LightSpaceMatrices[m_ShadowLightCount]);
            
            m_ShadowLightCount++;
        }

        // Render cube shadow maps for point lights.
        renderPointLightShadowMaps(frameInfo);
    }

    // =====================================================================================================================

    void VyShadowSystem::createCubeShadowPipeline()
    {
        // Only need position for shadow mapping.

        m_CubePipeline = VyPipeline::GraphicsBuilder{}
            .addPushConstantRange(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(CubeShadowPushConstants))

            // Use specialized cube shadow shaders that write linear depth
            .addShaderStage(VK_SHADER_STAGE_VERTEX_BIT,   "Shadows/CubeShadow.vert.spv")
            .addShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT, "Shadows/CubeShadow.frag.spv")
            
            // No culling for point light shadows to ensure all geometry is captured
            .setCullMode(VK_CULL_MODE_NONE)
            
            // // Depth bias to prevent shadow acne
            .setDepthBias(/*ConstantFactor*/ 1.25f, /*Clamp*/ 0.0f, /*SlopeFactor*/ 1.75f)

            // No color attachment - depth only
            .setDepthAttachment(VK_FORMAT_D32_SFLOAT)

            // Use the render pass from the first cube shadow map
            .setRenderPass(m_CubeShadowMaps[0]->renderPass())

        .buildUnique();
    }

    // =====================================================================================================================

    void VyShadowSystem::renderPointLightShadowMaps(VyFrameInfo& frameInfo)
    {
        m_CubeShadowLightCount = 0;

        auto view = frameInfo.Scene->registry().view<PointLightComponent, TransformComponent>();
        
        for (auto&& [entity, pointLight, transform] : view.each())
        {
            if (m_CubeShadowLightCount >= MAX_CUBE_SHADOW_MAPS) break;

            Vec3  position = transform.Translation;
            float range    = 25.0f; // Default range

            // Store light data for UBO.
            m_PointLightPositions[m_CubeShadowLightCount] = position;
            m_PointLightRanges   [m_CubeShadowLightCount] = range;

            // Render to cube map faces.
            renderToCubeShadowMap(frameInfo, *m_CubeShadowMaps[m_CubeShadowLightCount], position, range);

            m_CubeShadowLightCount++;
        }
    }

    // =====================================================================================================================

    Mat4 VyShadowSystem::calculatePointLightMatrix(const Vec3& position, int face, float range)
    {
        float nearPlane  = 0.1f;
        Mat4  projection = glm::perspective(glm::radians(90.0f), 1.0f, nearPlane, range);

        // Vulkan Y flip
        projection[1][1] *= -1;

        Mat4 view = VyCubeShadowMap::faceViewMatrix(position, face);

        return projection * view;
    }

    // =====================================================================================================================

    void VyShadowSystem::renderToCubeShadowMap(VyFrameInfo& frameInfo, VyCubeShadowMap& cubeShadowMap, const Vec3& position, float range)
    {
        for (int face = 0; face < 6; face++)
        {
            Mat4 lightSpaceMatrix = calculatePointLightMatrix(position, face, range);
            
            renderToCubeFace(frameInfo, cubeShadowMap, face, lightSpaceMatrix, position, range);
        }
    }

    // =====================================================================================================================

    void VyShadowSystem::renderToCubeFace(
        VyFrameInfo&     frameInfo,
        VyCubeShadowMap& cubeShadowMap,
        int              face,
        const Mat4&      lightSpaceMatrix,
        const Vec3&      lightPos,
        float            farPlane)
    {
        // Begin render pass for this face.
        cubeShadowMap.beginRenderPass(frameInfo.CommandBuffer, face);
        {
            // Bind cube shadow pipeline.
            m_CubePipeline->bind(frameInfo.CommandBuffer);

            // Render all objects.
            auto view = frameInfo.Scene->registry().view<ModelComponent, TransformComponent>();
            
            for (auto&& [entity, modelComp, transform] : view.each())
            {
                if (!modelComp.Model) continue;

                CubeShadowPushConstants push{};
                {
                    push.ModelMatrix         = transform.matrix();
                    push.LightSpaceMatrix    = lightSpaceMatrix;
                    push.LightPosAndFarPlane = Vec4(lightPos, farPlane);
                }

                m_CubePipeline->pushConstants(frameInfo.CommandBuffer, 
                    VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 
                    &push
                );

                modelComp.Model->bind(frameInfo.CommandBuffer);
                modelComp.Model->draw(frameInfo.CommandBuffer);
            }
        }
        // End face render pass.
        cubeShadowMap.endRenderPass(frameInfo.CommandBuffer);
    }

    // =====================================================================================================================

}