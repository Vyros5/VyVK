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

namespace Vy
{
    /**
     * Phases:
     * 
     *  1. Update Pass: CPU-Side Processing
     *      - Object selection
     *      - Input
     *      - LOD
     *      - Camera 
     * 
     *  2. Compute Pass: Pre-Render calculations
     *      - Animation
     * 
     *  3. Shadow Pass: UBO Updating
     *      - Light System update
     *      - Light Space
     *      - Update UBO
     * 
     *  4. Render (Offscreen) Pass: Draw Calls
     *      - Skybox System render
     *      - Mesh System render
     *      - Light System render
     * 
     * 5. Composition Pass: Postprocessing and UI
     *      - UI render
     * 
     */

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
        VY_NODISCARD VyEntity    mainCamera() const { return m_Scene->mainCamera(); }

    private:
        void loadEntities();

        bool isRunning();

        void updateCamera(VyCamera& camera);

    private:
        VyWindow   m_Window  { VyWindowData{} };
        VyInput    m_Input   { m_Window };
        VyRenderer m_Renderer{ m_Window };

        Unique<VyMasterRenderSystem> m_RenderSystem;
        Shared<VyMaterialSystem>     m_MaterialSystem{};

        Shared<VyScene> m_Scene;

        // Singleton
		static VyEngine* s_Instance;
		static bool      s_bInstanceFlag;

        bool m_Running;
    };


}