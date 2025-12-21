#pragma once

#include <Vy/Systems/Rendering/IRenderSystem.h>

#include <Vy/GFX/Renderer.h>
#include <Vy/GFX/Backend/Descriptors.h>

#include <Vy/Systems/Rendering/RenderSystem.h>
#include <Vy/Systems/Rendering/GridSystem.h>
#include <Vy/Systems/Rendering/LightSystem.h>
#include <Vy/Systems/Rendering/SkyboxSystem.h>
#include <Vy/Systems/Rendering/PostProcessSystem.h>
// #include <Vy/Systems/Rendering/ShadowSystem.h>
#include <Vy/Systems/Rendering/ShadowMapSystem.h>

#include <Vy/Systems/Buffer/MaterialSystem.h>


namespace Vy
{
    class VyMasterRenderSystem
    {
    public:
        VyMasterRenderSystem(
            VyRenderer&              renderer,
            Shared<VyMaterialSystem> materialSystem,
            Shared<VyEnvironment>    environment
        );

        ~VyMasterRenderSystem();

        // void initialize();
        
        void shutdown();

        void createRenderSystems();

        void updateUniformBuffers(VyFrameInfo& frameInfo, GlobalUBO& ubo);

        void render(VyFrameInfo& frameInfo);

        void recreate(VkExtent2D newExtent)
        {
            m_PostProcessSystem->recreate(newExtent);
        }

        void createDescriptorPools();
        void createDescriptors();
		void createUniformBuffers();

        VkDescriptorSet globalSet(int frameIndex)
        {
            return m_GlobalSets[ frameIndex ];
        }

    private:

        VyRenderer&                 m_Renderer;

        Unique<VyRenderSystem>      m_RenderSystem;
        Unique<VyLightSystem>       m_LightSystem;
        Unique<VyGridSystem>        m_GridSystem;
        Unique<VySkyboxSystem>      m_SkyboxSystem;
        Unique<VyPostProcessSystem> m_PostProcessSystem;
        // Unique<VyShadowSystem>      m_ShadowSystem;
        // Unique<VyShadowMapSystem>      m_ShadowMapSystem;

        Shared<VyMaterialSystem>     m_MaterialSystem;

        // UBO Buffers
        TVector<Unique<VyBuffer>>     m_UBOBuffers{ MAX_FRAMES_IN_FLIGHT };
        
        // Descriptor Pools
        Shared<VyDescriptorPool>      m_MaterialPool{};

        // Descriptor Sets
        TVector<VkDescriptorSet>      m_GlobalSets  { MAX_FRAMES_IN_FLIGHT };
        TVector<VkDescriptorSet>      m_MaterialSets;

        // Descriptor Set Layouts
        Shared<VyDescriptorSetLayout> m_GlobalSetLayout  {};
        Shared<VyDescriptorSetLayout> m_MaterialSetLayout{};

        Shared<VyEnvironment> m_Environment;

        bool m_IsRunning = false;
    };
}