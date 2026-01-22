#pragma once

#include <VyEngine/Core//Window/Window.h>
// #include <VyEngine/Core/Input/Input.h>
// #include <VyEngine/Core/Event/Event.h>

#include <VyEngine/VK/Renderer/Renderer.h>
#include <VyEngine/GFX/Resources/Texture/Texture.h>

#include <VyEngine/VK/Descriptors/Descriptors.h>
#include <VyEngine/VK/Device/Device.h>

#include <VyEngine/Scene/Scene.h>
#include <VyEngine/Scene/ECS/Entity.h>

#include <VyEngine/Core/Input/Input.h>

// #include <VyEngine/Systems/Rendering/MasterRenderSystem.h>
#include <VyEngine/GFX/Systems/LightSystem.h>
#include <VyEngine/GFX/Systems/ModelRenderSystem.h>
#include <VyEngine/GFX/Systems/MaterialSystem.h>
#include <VyEngine/GFX/Systems/GridSystem.h>
#include <VyEngine/GFX/Systems/SkyboxRenderSystem.h>

#include <VyEngine/GFX/Systems/ReflectionRenderSystem.h>
#include <VyEngine/GFX/Systems/ShadowRenderSystem.h>
#include <VyEngine/GFX/Systems/SceneRenderSystem.h>
#include <VyEngine/GFX/Systems/PointLightSystem.h>

namespace Vy
{
		static constexpr float LIGHT_NEAR_PLANE = 0.1f;
		static constexpr float LIGHT_FAR_PLANE = 50.0f;

		static constexpr float CAMERA_NEAR_PLANE = 0.1f;
		static constexpr float CAMERA_FAR_PLANE = 50.0f;

    class VyEngine
    {
    public:
        VyEngine();

        ~VyEngine();

        VyEngine(const VyEngine&)            = delete;
        VyEngine& operator=(const VyEngine&) = delete;

        static VyEngine& get();

        void initialize();

        void run();

        void onEvent(VyEvent& event);

        VY_NODISCARD VyRenderer& renderer()         { return m_Renderer; }
        // VY_NODISCARD VyEntity    mainCamera() const { return m_Scene->mainCamera(); }
        // Shared<VyMaterialSystem>& materialSystem()  { return m_MaterialSystem; }
        
    private:
        void loadEntities();

        bool isRunning();

        void updateCamera(VyCamera& camera);

    private:
        VyWindow   m_Window  { VyWindowData{} };
        VyInput    m_Input   { m_Window };
        VyRenderer m_Renderer{ m_Window };

        Unique<VyTexture> m_Texture;

        Unique<VyModelRenderSystem> m_ModelSystem;
        Unique<VyLightSystem>       m_LightSystem;
        Unique<VyGridSystem>        m_GridSystem;
        Unique<VySkyboxRenderSystem> m_SkyboxSystem;
        Unique<VySkybox>            m_Skybox;
        // Shared<VyMaterialSystem>    m_MaterialSystem{};

        Unique<VySceneRenderSystem> m_SceneSystem;
        Unique<VyShadowRenderSystem> m_ShadowSystem;
        Unique<VyReflectionRenderSystem> m_ReflectionSystem;
        Unique<VyPointLightSystem> m_PointLightSystem; 

        Shared<VyScene> m_Scene;

        VyEntity m_LightEntity;

        // Singleton
		static VyEngine* s_Instance;
		static bool      s_bInstanceFlag;

        Unique<VyDescriptorPool>      m_GlobalPool{};
        Unique<VyDescriptorSetLayout> m_ModelSetLayout;

        TVector<VkDescriptorSet>      m_GlobalSets     { MAX_FRAMES_IN_FLIGHT };
        Shared<VyDescriptorSetLayout> m_GlobalSetLayout{};

        Unique<VyDescriptorPool>      m_MaterialPool{};
        Unique<VyDescriptorSetLayout> m_MaterialSetLayout{};

        // UBO Buffers
        TVector<Unique<VyBuffer>> m_UniformBuffers{ MAX_FRAMES_IN_FLIGHT };
        

        bool m_Running;
        U32 m_FrameCount{ 0 };
    };
}