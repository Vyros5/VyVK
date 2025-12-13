#pragma once

#include <Vy/Systems/Rendering/IRenderSystem.h>

#include <Vy/GFX/Renderer.h>
#include <Vy/GFX/Backend/Descriptors.h>

#include <Vy/Systems/Rendering/RenderSystem.h>
#include <Vy/Systems/Rendering/GridSystem.h>
#include <Vy/Systems/Rendering/LightSystem.h>
#include <Vy/Systems/Rendering/SkyboxSystem.h>
#include <Vy/Systems/Rendering/PostProcessSystem.h>
#include <Vy/Systems/Rendering/ShadowSystem.h>

#include <Vy/Systems/Buffer/MaterialSystem.h>


namespace Vy
{
    class VyMasterRenderSystem
    {
    public:
        VyMasterRenderSystem(
            VyRenderer&                   renderer,
            Shared<VyDescriptorSetLayout> globalSetLayout,
            Shared<VyDescriptorSetLayout> materialSetLayout,
            Shared<VyMaterialSystem>      materialSystem,
            Shared<VyDescriptorPool>      materialPool,
            Shared<VyEnvironment>         environment
        );

        ~VyMasterRenderSystem();

        // void initialize();
        
        void deinitialize();

        void createRenderSystems();

        void update(VyFrameInfo& frameInfo, GlobalUBO& ubo);

        void render(VyFrameInfo& frameInfo);

        void renderPostProcess(VyFrameInfo& frameInfo);

        void renderFinalComposite(VyFrameInfo& frameInfo);

        void recreate(VkExtent2D newExtent)
        {
            m_PostProcessSystem->recreate(newExtent);
        }
        
    private:
        VyRenderer&                 m_Renderer;

        Unique<VyRenderSystem>      m_RenderSystem;
        Unique<VyLightSystem>       m_LightSystem;
        Unique<VyGridSystem>        m_GridSystem;
        Unique<VySkyboxSystem>      m_SkyboxSystem;
        Unique<VyPostProcessSystem> m_PostProcessSystem;
        Unique<VyShadowSystem>      m_ShadowSystem;

        Shared<VyMaterialSystem> m_MaterialSystem;
        // Shared<VyLightManager>   m_LightManager;
        // VyLightManager   m_LightManager;

        Shared<VyDescriptorSetLayout> m_GlobalSetLayout{};
        Shared<VyDescriptorSetLayout> m_MaterialSetLayout{};
        Shared<VyDescriptorPool>      m_MaterialPool{};

        Shared<VyEnvironment> m_Environment;

        bool m_IsRunning = false;
    };
}