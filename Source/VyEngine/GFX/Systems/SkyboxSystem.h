#pragma once

#include <VyEngine/GFX/Resources/Texture/Skybox.h>
#include <VyEngine/GFX/Data/FrameInfo.h>

#include <VyEngine/VK/Pipeline/Pipeline.h>
#include <VyEngine/VK/Descriptors/Descriptors.h>

namespace Vy
{
    // struct SkyboxSettings
    // {
    //     bool  UseProcedural  { false };
    //     Vec4  SunDirection   { 0.0f, 1.0f, 0.0f, 1.0f }; // w = intensity
    //     Vec4  SunColor       { 1.0f, 1.0f, 1.0f, 1.0f };
    //     float Rayleigh       { 1.0f  };
    //     float MIE            { 0.02f };
    //     float MIEEccentricity{ 0.76f };
    // };

    // struct FogSettings
    // {
    //     float Density      { 0.005f };
    //     float Height       { 0.0f   };
    //     float HeightDensity{ 0.1f   };
    //     bool  UseSkyColor  { true   };
    //     Vec3  Color        { 0.5f, 0.6f, 0.7f };

    //     // God Rays
    //     bool  EnableGodRays { true  };
    //     float GodRayDensity { 1.0f  };
    //     float GodRayWeight  { 0.01f };
    //     float GodRayDecay   { 0.97f };
    //     float GodRayExposure{ 0.5f  };
    // };

    /**
     * @brief Render system for skybox/environment maps
     *
     * Renders a cubemap skybox as the background of the scene.
     * Should be rendered first (or last with depth write disabled).
     */
    class VySkyboxSystem
    {
    public:
        VySkyboxSystem(VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
        ~VySkyboxSystem();

        // Non-copyable
        VySkyboxSystem(const VySkyboxSystem&)            = delete;
        VySkyboxSystem& operator=(const VySkyboxSystem&) = delete;

        /**
         * @brief Render the skybox
         * @param frameInfo Current frame information (camera, etc.)
         * @param pSkybox The skybox cubemap to render (can be null if using procedural)
         * @param settings Skybox configuration
         */
        void render(VyFrameInfo& frameInfo, VySkybox* pSkybox); // , const SkyboxSettings& settings

    private:
        void createDescriptorSetLayout();
        void createPipeline(VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);

        Unique<VyPipeline>            m_Pipeline;
        Unique<VyDescriptorPool>      m_DescriptorPool;
        Unique<VyDescriptorSetLayout> m_DescriptorSetLayout;

        // Pre-allocated descriptor sets per frame
        TVector<VkDescriptorSet> m_DescriptorSets;
    };
}