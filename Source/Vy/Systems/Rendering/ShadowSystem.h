#pragma once

#include <Vy/GFX/Resources/ShadowMap.h>
#include <Vy/GFX/Backend/Descriptors.h>
#include <Vy/GFX/Backend/Pipeline.h>
#include <Vy/GFX/FrameInfo.h>
#include <Vy/Systems/Rendering/IRenderSystem.h>

namespace Vy
{
    /**
     * @brief System for rendering shadow maps from light perspectives.
     *
     * Manages shadow map rendering for directional, spot, and point lights.
     * Uses 2D shadow maps for directional/spot lights and cube maps for point lights.
     */
    class VyShadowSystem
    {
    public:
        static constexpr int MAX_SHADOW_MAPS      = 4; // For directional + spotlights
        static constexpr int MAX_CUBE_SHADOW_MAPS = 4; // For point lights (cube maps)

        VyShadowSystem(U32 shadowMapSize = 2048);

        ~VyShadowSystem();

        VyShadowSystem(const VyShadowSystem&)            = delete;
        VyShadowSystem& operator=(const VyShadowSystem&) = delete;

        /**
         * @brief Render all shadow maps for the frame
         * @param frameInfo Current frame information
         * @param sceneRadius Approximate scene bounds for light frustum calculation
         */
        void renderShadowMaps(VyFrameInfo& frameInfo, float sceneRadius = 20.0f);

        /**
         * @brief Get the shadow map at specified index for sampling
         */
        VyShadowMap& getShadowMap(int index = 0) { return *m_ShadowMaps[index]; }

        /**
         * @brief Get the cube shadow map at specified index for point lights
         */
        VyCubeShadowMap& getCubeShadowMap(int index = 0) { return *m_CubeShadowMaps[index]; }

        /**
         * @brief Get the light space matrix at specified index
         */
        const Mat4& getLightSpaceMatrix(int index = 0) const { return m_LightSpaceMatrices[index]; }

        /**
         * @brief Get number of active shadow-casting directional/spot lights
         */
        int getShadowLightCount() const { return m_ShadowLightCount; }

        /**
         * @brief Get number of active shadow-casting point lights
         */
        int getCubeShadowLightCount() const { return m_CubeShadowLightCount; }

        /**
         * @brief Get point light position for shadow calculation in shader
         */
        const Vec3& getPointLightPosition(int index = 0) const { return m_PointLightPositions[index]; }

        /**
         * @brief Get point light range (far plane) for shadow calculation
         */
        float getPointLightRange(int index = 0) const { return m_PointLightRanges[index]; }

        /**
         * @brief Get descriptor info for shadow map sampling
         */
        VkDescriptorImageInfo shadowMapDescriptorInfo(int index = 0) const { return m_ShadowMaps[index]->descriptorImageInfo(); }

        /**
         * @brief Get descriptor info for cube shadow map sampling
         */
        VkDescriptorImageInfo cubeShadowMapDescriptorInfo(int index = 0) const { return m_CubeShadowMaps[index]->descriptorImageInfo(); }

    private:

        void createPipeline();
        void createCubeShadowPipeline();

        /**
         * @brief Calculate orthographic projection matrix for directional light
         */
        Mat4 calculateDirectionalLightMatrix(const Vec3& lightDirection, const Vec3& sceneCenter, float sceneRadius);

        /**
         * @brief Calculate perspective projection matrix for spotlight
         */
        Mat4 calculateSpotLightMatrix(const Vec3& position, const Vec3& direction, float outerCutoffDegrees, float range);

        /**
         * @brief Calculate perspective projection matrix for one face of a point light cube map
         */
        Mat4 calculatePointLightMatrix(const Vec3& position, int face, float range);

        /**
         * @brief Render scene to a 2D shadow map with given light space matrix
         */
        void renderToShadowMap(VyFrameInfo& frameInfo, VyShadowMap& shadowMap, const Mat4& lightSpaceMatrix);

        /**
         * @brief Render point light shadow maps (all 6 faces for each point light)
         */
        void renderPointLightShadowMaps(VyFrameInfo& frameInfo);

        /**
         * @brief Render all 6 faces of a cube shadow map for a single point light
         */
        void renderToCubeShadowMap(VyFrameInfo& frameInfo, VyCubeShadowMap& cubeShadowMap, const Vec3& position, float range);

        /**
         * @brief Render scene to a single face of a cube shadow map
         */
        void renderToCubeFace(
            VyFrameInfo&     frameInfo,
            VyCubeShadowMap& cubeShadowMap,
            int              face,
            const Mat4&      lightSpaceMatrix,
            const Vec3&      lightPos,
            float            farPlane
        );

    private:

        U32 m_ShadowMapSize;

        // 2D shadow maps for directional/spot lights
        TVector<Unique<VyShadowMap>> m_ShadowMaps;
        Unique<VyPipeline>           m_Pipeline;

        // Cube shadow maps for point lights
        TVector<Unique<VyCubeShadowMap>> m_CubeShadowMaps;
        Unique<VyPipeline>               m_CubePipeline;

        Mat4 m_LightSpaceMatrices[MAX_SHADOW_MAPS];
        int  m_ShadowLightCount = 0;

        Vec3  m_PointLightPositions[MAX_CUBE_SHADOW_MAPS];
        float m_PointLightRanges[MAX_CUBE_SHADOW_MAPS];
        int   m_CubeShadowLightCount = 0;
    };
}