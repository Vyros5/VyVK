#include <Vy/Engine.h>

#include <Vy/GFX/Context.h>

#include <Vy/Core/FrameRateController.h>
#include <Vy/Core/DataBuffer.h>

#include <Vy/Systems/Buffer/MaterialSystem.h>

namespace Vy
{
	VyEngine* VyEngine::s_Instance      = nullptr;
	bool      VyEngine::s_bInstanceFlag = false;

    
	VyEngine& VyEngine::get()
	{
		VY_ASSERT(s_bInstanceFlag, "VyEngine instance not created");

		return *s_Instance;
	}


    VyEngine::VyEngine()
    {
		s_Instance      = this;
		s_bInstanceFlag = true;
    }


    VyEngine::~VyEngine() 
    {
        m_RenderSystem->shutdown();
    }


    void VyEngine::initialize()
    {
        // Create and reset (initialize) the scene.
        m_Scene = MakeShared<VyScene>();
        m_Scene->reset();

        // Load builtin entities for the current scene. 
        loadEntities();

        m_MaterialSystem = MakeUnique<VyMaterialSystem>();

        // [ Initialize Rendering System ]
        m_RenderSystem = MakeUnique<VyMasterRenderSystem>(
            m_Renderer,
            m_MaterialSystem,
            m_Scene->getEnvironment()
        );

        m_Window.setEventCallback([this]<typename E>(E&& event) 
        {
            onEvent(std::forward<E>(event));
        });
    }


    void VyEngine::updateCamera(VyCamera& camera)
	{
        if (VyEntity mainCameraEntity = m_Scene->mainCamera()) 
        {
            const auto& cameraComponent = mainCameraEntity.get<CameraComponent>();
            const auto& transform       = mainCameraEntity.get<TransformComponent>();

            camera = cameraComponent.Camera;

            camera.setView(transform.Translation, transform.Rotation);

            if (camera.isPerspective()) 
            {
                camera.setPerspective(m_Renderer.aspectRatio());
            }
            else {
				camera.setOrthographic();
            }
        }
        else
        {
            VY_ERROR("No main camera");
        }
	}


    void VyEngine::run()
    {
        // Active Camera Object.
        VyCamera camera{};

        // [ Initialize FrameRate Controller (60 FPS) ]
        FrameRateController frameRateController{ 60u };

        VkExtent2D previousExtent = m_Renderer.swapchainExtent();

        // [ Main Loop ]
        while (!m_Window.shouldClose() || m_Running) 
        {
            // Poll Window Events
            m_Window.pollEvents();

            float deltaTime = frameRateController.waitForNextFrame();

            // [ Pre-Frame Update ]
            {
                // Update Scripts and Scene Systems.
                m_Scene->update(deltaTime);
                
                updateCamera(camera);
            }

            // [ Frame ]
            if (auto cmdBuffer = m_Renderer.beginFrame()) 
            {
                // Check if window was resized and recreate post-processing resources.
                {
                    VkExtent2D currentExtent = m_Renderer.swapchainExtent();
                    
                    if (currentExtent.width  != previousExtent.width || 
                        currentExtent.height != previousExtent.height) 
                    {
                        m_RenderSystem->recreate(currentExtent);
                        
                        previousExtent = currentExtent;
                    }
                }

                // Update Frame Info.
                int frameIndex = m_Renderer.frameIndex();

                VyFrameInfo frameInfo{
                    .FrameIndex          = frameIndex,                            // Index of the current frame.
                    .FrameTime           = deltaTime,                             // Time between frames.
                    .CommandBuffer       = cmdBuffer,                             // Main command buffer.
                    .GlobalDescriptorSet = m_RenderSystem->globalSet(frameIndex), // Global descriptor set for the current frame.
                    .Scene               = m_Scene,                               // Active scene.
                    .Camera              = camera                                 // Active camera to update the UBOs.
                };

                // [ Update ]
                {
                    GlobalUBO ubo{};

                    // Update UBOs and Render Systems.
                    m_RenderSystem->updateUniformBuffers(frameInfo, ubo);
                }

                // [ Render ]
                {
                    m_RenderSystem->render(frameInfo);
                }

                m_Renderer.endFrame();

            } // [ Frame End ]
        } // [ Main Loop End ]

        VyContext::waitIdle();
    }


    void VyEngine::onEvent(VyEvent& event)
    {
        VyEventDispatcher dispatcher(event);

        dispatcher.dispatch<WindowCloseEvent>([this](auto& event) 
        {
            m_Running = false;
        });
    }


    void VyEngine::loadEntities()
    {
        // ========================================================================================
        // Environment 

        TArray<String, 6> skyboxTextures;
        {
            skyboxTextures[CubeFace::RIGHT ] = CUBEMAP_DIR "BlueCloud/bluecloud_rt.jpg";
            skyboxTextures[CubeFace::LEFT  ] = CUBEMAP_DIR "BlueCloud/bluecloud_lf.jpg";
            skyboxTextures[CubeFace::TOP   ] = CUBEMAP_DIR "BlueCloud/bluecloud_up.jpg";
            skyboxTextures[CubeFace::BOTTOM] = CUBEMAP_DIR "BlueCloud/bluecloud_dn.jpg";
            skyboxTextures[CubeFace::BACK  ] = CUBEMAP_DIR "BlueCloud/bluecloud_bk.jpg";
            skyboxTextures[CubeFace::FRONT ] = CUBEMAP_DIR "BlueCloud/bluecloud_ft.jpg";
        }

        auto environment = m_Scene->getEnvironment();

        environment->setAmbientLight({ 1.0f, 1.0f, 1.0f, 0.1f });
        environment->setSkybox(skyboxTextures);

        // ========================================================================================
        // Materials

        MaterialComponent floorMat{ MakeShared<VyMaterial>() };
        // floorMat.loadAlbedoTexture("Textures/Floor/slate_floor_diff_2k.jpg");
        // floorMat.loadNormalTexture("Textures/Floor/slate_floor_nor_gl_2k.jpg");
        floorMat.setMetallic (0.7f);
        floorMat.setRoughness(0.3f);

        MaterialComponent wallMat{ MakeShared<VyMaterial>() };
        wallMat.loadAlbedoTexture("Textures/StoneSlate/castle_wall_slates_diff_4k.jpg");
        wallMat.loadNormalTexture("Textures/StoneSlate/castle_wall_slates_nor_gl_4k.jpg");
        wallMat.setMetallic (3.0f);
        wallMat.setRoughness(0.002f);

        MaterialComponent vaseMat{ MakeShared<VyMaterial>() };
        vaseMat.setMetallic (7.0f);
        vaseMat.setRoughness(0.002f);
        vaseMat.setColor    ({ 1.0f, 1.0f, 0.1f });

        // ========================================================================================
        // Models & Entities

        Shared<VyStaticMesh> 
        model = VyStaticMesh::create("plane.obj");

        auto floor = m_Scene->createEntity("Floor");
        {
            floor.add<ModelComponent>(model);
            floor.add<MaterialComponent>(floorMat);
            floor.get<TransformComponent>() = TransformComponent{
                /* Position */ { 0.0f, -0.01f, 0.0f },
                /* Scale    */ { 2.0f, 2.0f, 2.0f }
            };
        }
        
        auto roof = m_Scene->createEntity("Roof");
        {
            roof.add<ModelComponent>(model);
            roof.add<MaterialComponent>(floorMat);
            roof.get<TransformComponent>() = TransformComponent{
                /* Position */ { 0.0f, -4.0f, 0.0f },
                /* Scale    */ { 2.0f, 2.0f, 2.0f }//,
                // /* Rotation */ { glm::pi<float>() / 2, 0.0f, 0.0f }
            };
        }

        auto wallRight = m_Scene->createEntity("Wall-Right");
        {
            wallRight.add<ModelComponent>(model);
            wallRight.add<MaterialComponent>(wallMat);
            wallRight.get<TransformComponent>() = TransformComponent{
                /* Position */ { 2.0f, -2.0f, 0.0f },
                /* Scale    */ { 2.0f, 2.0f, 2.0f },
                /* Rotation */ { glm::pi<float>() / 2, 0.0f, -glm::pi<float>() / 2 }
            };
        }

        auto wallLeft = m_Scene->createEntity("Wall-Left");
        {
            wallLeft.add<ModelComponent>(model);
            wallLeft.add<MaterialComponent>(wallMat);
            wallLeft.get<TransformComponent>() = TransformComponent{
                /* Position */ { -2.0f, -2.0f, 0.0f },
                /* Scale    */ { 2.0f, 2.0f, 2.0f },
                /* Rotation */ { glm::pi<float>() / 2, 0.0f, glm::pi<float>() / 2 }
            };
        }

        auto wallBack = m_Scene->createEntity("Wall-Back");
        {
            wallBack.add<ModelComponent>(model);
            wallBack.add<MaterialComponent>(wallMat);
            wallBack.get<TransformComponent>() = TransformComponent{
                /* Position */ { 0.0f, -2.0f, 2.0f },
                /* Scale    */ { 2.0f, 2.0f, 2.0f },
                /* Rotation */ { glm::pi<float>() / 2, 0.0f, 0.0f }
            };
        }


        model = VyStaticMesh::create("smooth_vase.obj");
        
        auto vase = m_Scene->createEntity("Vase");
        {
            vase.add<ModelComponent>(model);
            vase.add<MaterialComponent>(vaseMat);
            vase.get<TransformComponent>() = TransformComponent{
                { 0.0f, -0.01f, 0.0f },
            };
        }

        // ========================================================================================
        
        TVector<Vec3> lightColors{
            { 1.0f, 0.1f, 0.1f }, // Red
            { 0.1f, 0.1f, 1.0f }, // Blue
            { 0.1f, 1.0f, 0.1f }, // Green
            { 1.0f, 1.0f, 0.1f }, // Yellow
            { 0.1f, 1.0f, 1.0f }, // Cyan
            { 1.0f, 1.0f, 1.0f }  // White
        };

        for (int i = 0; i < lightColors.size(); i++) 
        {
            auto rotateLight = glm::rotate(Mat4(1.0f),
                (i * glm::two_pi<float>()) / lightColors.size(),
                { 0.0f, 2.0f, 0.0f }
            );

            auto pointLight = m_Scene->createEntity("PointLight_" + std::to_string(i));
            {
                pointLight.add<PointLightComponent>(lightColors[i], 0.2f);
                pointLight.get<TransformComponent>() = TransformComponent{
                    Vec3(rotateLight * Vec4(-1.0f, -2.0f, -1.0f, 1.0f)),
                    { 0.2f, 0.2f, 0.2f }
                };
            }
        }
        
        // auto dirLight = m_Scene->createEntity("DirLight");
        // {
        //     dirLight.add<DirectionalLightComponent>().;
        //     vase.get<TransformComponent>() = TransformComponent{
        //         { 0.0f, -0.01f, 0.0f },
        //     };
        // }

        // ========================================================================================
        
        VY_INFO_TAG("VyEngine", "Entities loaded");
    }
}