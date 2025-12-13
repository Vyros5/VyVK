#pragma once

#include <Vy/Core/Window.h>
#include <Vy/Core/Input/Input.h>
#include <Vy/Core/Event/Event.h>

#include <Vy/GFX/Renderer.h>

#include <Vy/GFX/Backend/Descriptors.h>
#include <Vy/GFX/Backend/Device.h>

#include <Vy/Scene/Scene.h>
#include <Vy/Scene/ECS/EntityHandle.h>

#include <Vy/Systems/Rendering/MasterRenderSystem.h>
// #include <Vy/Scene/CameraController.h>
// #include <Vy/Asset/AssetManager.h>

namespace Vy
{
	// struct VyEngineInfo
	// {
	// 	String Name              = "VyEngine";
	// 	String Version           = "";//ENGINE_VERSION;
	// 	String WindowName        = "VyEngine";
	// 	U32    WindowWidth       = 1600;
    //     U32    WindowHeight      = 900;
	// 	Path   ProjectsDirectory = "Projects";
	// };

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

        // VY_NODISCARD static VyAssetManager& assetManager() { return get().m_AssetsManager; }
        // Shared<VyDescriptorAllocator> descriptorAllocator() const { return m_DescriptorAllocator; } 
        // VY_NODISCARD static Shared<VyDescriptorSetLayout> materialSetLayout() { return get().m_MaterialSetLayout; };
        
        VY_NODISCARD Shared<VyDescriptorPool>& globalPool()       { return m_GlobalPool; }
        VY_NODISCARD VyRenderer&               renderer()         { return m_Renderer; }
        VY_NODISCARD VyEntity                  mainCamera() const { return m_Scene->mainCamera(); }

    private:
        void loadEntities();

        void createDescriptorPools();
		void createUBOBuffers();
        void createDescriptors();

        // void createDefaultResources();
        // void registerResources();

        void updateCamera(VyCamera& camera);

    private:
        VyWindow   m_Window  { VyWindowData{} };
        VyInput    m_Input   { m_Window };
        VyRenderer m_Renderer{ m_Window };

        Unique<VyMasterRenderSystem> m_RenderSystem;

        // VyAssetManager m_AssetsManager{};

        Shared<VyScene> m_Scene;

        // Descriptor Resources
        // Shared<VyDescriptorAllocator> m_DescriptorAllocator{};
        Shared<VyDescriptorPool>      m_GlobalPool{};
        Shared<VyDescriptorSetLayout> m_GlobalSetLayout{};
        TVector<VkDescriptorSet>      m_GlobalSets     { MAX_FRAMES_IN_FLIGHT };
        TVector<Unique<VyBuffer>>     m_UBOBuffers     { MAX_FRAMES_IN_FLIGHT };

        Shared<VyMaterialSystem>      m_MaterialSystem{};
        Shared<VyDescriptorPool>      m_MaterialPool{};
        Shared<VyDescriptorSetLayout> m_MaterialSetLayout{};
        TVector<VkDescriptorSet>      m_MaterialSets;

        // Singleton
		static VyEngine* s_Instance;
		static bool      s_bInstanceFlag;

        // VyEngineInfo m_EngineInfo;

        bool m_Running;
    };


}