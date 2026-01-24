#include <VyEngine/Engine.h>

#include <VyEngine/VK/Context.h>
#include <VyEngine/Scene/ECS/Components.h>
#include <VyEngine/Core/FrameRateController.h>

#include <iostream>
#include <VyLib/Common/AnsiColor.h>

Vy::Shared<Vy::VyTexture> g_DefaultTexture;
Vy::Shared<Vy::VyBuffer>  g_DummyBuffer;

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

        initialize();
    }

    
    VyEngine::~VyEngine() 
    {
        g_DummyBuffer   .reset();
        g_DefaultTexture.reset();

        m_FrameCount = 0;
    }


    bool VyEngine::isRunning()
    {
        return !m_Window.shouldClose() && m_Running;
    }


    void VyEngine::initialize()
    {
        // Create and reset (initialize) the scene.
        m_Scene = MakeShared<VyScene>( "main-scene" );
        m_Scene->reset();

        // Load builtin entities for the current scene. 
        // loadEntities();

        // m_Skybox = VySkybox::loadFromFolder(TString(CUBEMAP_DIR) + "Yokohama", "jpg");

        m_GlobalPool = VyDescriptorPool::Builder()
            .setName    ("global")
            .setMaxSets (1000)
            .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         1000)
            .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000)
            .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000)
            .buildPtr();


		// m_ShadowSystem = MakeUnique<VyShadowRenderSystem>(
        //     m_Renderer.shadowRenderPass(), 
        //     m_Renderer.shadowSetLayout() 
        // );
        // m_ReflectionSystem = MakeUnique<VyReflectionRenderSystem>(
        //     m_Renderer.mappingsRenderPass(), 
        //     m_Renderer.mappingsSetLayout(), 
        //     m_Renderer.uvReflectionRenderPass(), 
        //     m_Renderer.uvReflectionSetLayout()
        // );
        // m_SceneSystem = MakeUnique<VySceneRenderSystem>(
		// 	m_Renderer.lightingRenderPass(), 
		// 	m_Renderer.gBufferSetLayout(), 
		// 	m_Renderer.compositionSetLayout(),
		// 	m_Renderer.postProcessingRenderPass(),
		// 	m_Renderer.postProcessingSetLayout()
        // );

        // m_PointLightSystem = MakeUnique<VyPointLightSystem>(
		// 	m_Renderer.lightingRenderPass(), 
		// 	m_Renderer.gBufferSetLayout(), 
		// 	m_Renderer.compositionSetLayout()
        // );


        // m_GlobalPool = VyDescriptorPool::Builder{}
        //     .setName    ("global")
        //     .setMaxSets (MAX_FRAMES_IN_FLIGHT)
        //     .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, MAX_FRAMES_IN_FLIGHT)
        //     .buildPtr();

        // Create the global uniform buffers (One per frame). 
        for (int i = 0; i < m_UniformBuffers.size(); i++) 
        {
            m_UniformBuffers[ i ] = MakeUnique<VyBuffer>( VyBuffer::uniformBuffer("global", sizeof(GlobalUbo)) );
        }

        m_TempGroundTexture = VyTexture::createFromFilepath(TEXTURE_DIR "Ground.png");

        VkDescriptorImageInfo imageInfo = m_TempGroundTexture->descriptorImageInfo();

        m_GlobalSetLayout = VyDescriptorSetLayout::Builder{}
            .setName   ("global")
            .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         VK_SHADER_STAGE_ALL_GRAPHICS)
            .addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT) // ground texture
            .buildPtr();

        m_MaterialSetLayout = VyDescriptorSetLayout::Builder{}
            .setName   ("material")
            .addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
            .addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
            .addBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
            .buildPtr();


        for (U32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) 
        {
            VkDescriptorBufferInfo bufferInfo = m_UniformBuffers[ i ]->descriptorBufferInfo();

            VyDescriptorWriter( *m_GlobalSetLayout, *m_GlobalPool )
                .writeBuffer( 0, &bufferInfo )
                .writeImage ( 1, &imageInfo  )
                .build( m_GlobalSets[ i ] );
        }

        m_Models.push_back( VyGLTFModel::createFromFile( MODELS_DIR "Cube/Cube.gltf",   *m_MaterialSetLayout, *m_GlobalPool) );
        m_Models.push_back( VyGLTFModel::createFromFile( MODELS_DIR "Plane/Plane.gltf", *m_MaterialSetLayout, *m_GlobalPool) );
        // m_Models.push_back( VyGLTFModel::createFromFile( MODELS_DIR "Sponza/Sponza.gltf", *m_MaterialSetLayout, *m_GlobalPool) );

        m_SetLayouts.push_back(m_GlobalSetLayout->handle());
        m_SetLayouts.push_back(m_MaterialSetLayout->handle());

        m_RenderSystem     = MakeUnique<VyRenderSystem>    (m_Renderer.swapchainRenderPass(), m_SetLayouts, *m_GlobalPool);
        m_PointLightSystem = MakeUnique<VyPointLightSystem>(m_Renderer.swapchainRenderPass(), m_GlobalSetLayout->handle());

        loadEntities();


        // TVector<VkDescriptorSetLayout> setLayouts = {
        //     m_GlobalSetLayout->handle(),
        // };

        // [ Initialize Rendering Systems ]
        // m_ModelSystem = MakeUnique<VyModelRenderSystem>( 
        //     m_Renderer.swapchainRenderPass(), 
        //     setLayouts 
        // );
        
        // m_LightSystem = MakeUnique<VyLightSystem>( 
        //     m_Renderer.swapchainRenderPass(), 
        //     m_GlobalSetLayout->handle()
        // );

        // m_GridSystem = MakeUnique<VyGridSystem>( 
        //     m_Renderer.swapchainRenderPass(), 
        //     m_GlobalSetLayout->handle()
        // );

        // m_SkyboxSystem = MakeUnique<VySkyboxRenderSystem>(
        //     m_Renderer.swapchainRenderPass(),
        //     m_GlobalSetLayout->handle()
        // );

        auto view = m_Scene->registry().view<TagComponent>();

        std::stringstream ss;
        ss  << GRAY "Loaded Entities: " RESET << '\n';
        
        for (auto&& [ entity, tag ] : view.each())
        {
            ss  << CYAN "  - '" << tag.Tag.c_str() << "'" RESET << '\n';
        }

        VY_TRACE_TAG("VyEngine", "{0}", ss.str());
    }
    

    void VyEngine::updateCamera(VyCamera& camera)
	{
        if (VyEntity mainCameraEntity = m_Scene->mainCameraEntity()) 
        {
            const auto& cameraComponent = mainCameraEntity.get<CameraComponent>();
            const auto& transform       = mainCameraEntity.get<TransformComponent>();

            camera = cameraComponent.Camera;

            camera.setView( transform.Translation, transform.Rotation );

            if (camera.isPerspective()) 
            {
                // camera.setPerspectiveParams()
                camera.setPerspective( m_Renderer.aspectRatio() );
            }
            else {
				camera.setOrthographic();
            }
        }
        else
        {
            VY_ERROR_TAG("VyEngine", "No main camera!");
        }
	}


    void VyEngine::run()
    {
        // Active Camera Object.
        VyCamera camera{};

        // VyCamera light{};

		// ShadowUbo         shadowUbo{};
		// GBufferUbo        gBufferUbo{};
		// CompositionUbo    compositionUbo{};
		// MappingsUbo       mappingsUbo{};
		// UVReflectionUbo   uvReflectionUbo{};
		// PostProcessingUbo postProcessingUbo{};

        // m_LightEntity = m_Scene->createEntity("light-camera");
        // auto lightTransform = m_LightEntity.get<TransformComponent>();
		// lightTransform.Translation = LIGHT_POSITION;

		// float aspect = m_Renderer.shadowAspectRatio();
		// light.setPerspectiveParams(glm::radians(90.0f), LIGHT_NEAR_PLANE, LIGHT_FAR_PLANE);
        // light.setPerspective(aspect);

		// for (int faceIndex = 0; faceIndex < NUM_CUBE_FACES; faceIndex++) 
        // {
        //     lightTransform.resetRotation();

		// 	switch (faceIndex)
		// 	{
		// 	case 0: // POSITIVE_X
		// 		lightTransform.Rotation.y = glm::radians(90.0f);
		// 		break;

		// 	case 1:	// NEGATIVE_X
		// 		lightTransform.Rotation.y = glm::radians(-90.0f);
		// 		break;

		// 	case 2:	// POSITIVE_Y
		// 		lightTransform.Rotation.x = glm::radians(90.0f);
		// 		break;

		// 	case 3:	// NEGATIVE_Y
		// 		lightTransform.Rotation.x = glm::radians(-90.0f);
		// 		break;

		// 	case 4:	// POSITIVE_Z

		// 		break;

		// 	case 5:	// NEGATIVE_Z
		// 		lightTransform.Rotation.y = glm::radians(180.0f);
		// 		break;
		// 	}

		// 	light.setView(lightTransform.Translation, lightTransform.Rotation);

		// 	shadowUbo.projectionView[ faceIndex ] = light.projection() * light.view();
		// }


        // [ Initialize FrameRate Controller (60 FPS) ]
        FrameRateController frameRateController{ 60u };

        VkExtent2D previousExtent = m_Renderer.swapchainExtent();

        m_FrameCount = 0;

        // [ Main Loop ]
        while (isRunning())
        {
            // Poll Window Events.
            m_Window.pollEvents();

            float deltaTime = frameRateController.waitForNextFrame();

            // [ Pre-Frame Update ]
            {
                // Update scripts and controllers.
                m_Scene->update( deltaTime );
                
                updateCamera( camera );
            }

            Vec2 invResolution;

            // [ Frame ]
            if (auto cmdBuffer = m_Renderer.beginFrame()) 
            {
                // Check if window was resized and recreate resources.
                {
                    VkExtent2D currentExtent = m_Renderer.swapchainExtent();
                    
                    if (currentExtent.width  != previousExtent.width || 
                        currentExtent.height != previousExtent.height) 
                    {
                        // m_RenderSystem->recreate(currentExtent);
                        
                        previousExtent = currentExtent;
                    }

                    // invResolution = Vec2(
                    //     1.0f / currentExtent.width, 
                    //     1.0f / currentExtent.height
                    // );
                    
                    // uvReflectionUbo  .InvResolution = invResolution;
                    // postProcessingUbo.InvResolution = invResolution;
                }

                // Update Frame Info.
                int frameIndex = m_Renderer.frameIndex();

                VyFrameInfo frameInfo{
                    .FrameIndex          = frameIndex,                 // Index of the current frame.
                    .FrameTime           = deltaTime,                  // Time between frames.
                    .CommandBuffer       = cmdBuffer,                  // Main command buffer.
                    .GlobalSet           = m_GlobalSets[ frameIndex ], // Global descriptor set for the current frame.
                    // .ShadowSet           = m_Renderer.currentShadowSet(),
                    // .MappingsSet         = m_Renderer.currentMappingsSet(),
                    // .UVReflectionSet     = m_Renderer.currentUVReflectionSet(),
                    // .GBufferSet          = m_Renderer.currentGBufferSet(),
                    // .CompositionSet      = m_Renderer.currentCompositionSet(),
                    // .PostProcessingSet   = m_Renderer.currentPostProcessingSet(),
                    .Scene               = m_Scene,                    // Active scene.
                    .Camera              = camera                      // Active camera to update the UBOs.
                };

                GlobalUbo ubo{};

                // [ Update ]
                {
                    // [ Update UBO Data ]
                    {
                        ubo.CameraData.Projection  = frameInfo.Camera.projection();
                        ubo.CameraData.View        = frameInfo.Camera.view();
                        ubo.CameraData.InverseView = frameInfo.Camera.inverseView();
                    }

                    // Update light values into UBO.
                    m_PointLightSystem->update( frameInfo, ubo );

                    // Write global uniform buffers.
                    m_UniformBuffers[ frameInfo.FrameIndex ]->write( &ubo, sizeof(GlobalUbo), 0 );


                    m_RenderSystem->renderCascadedShadowPass(frameInfo, ubo);
                    m_RenderSystem->renderPointShadowPass   (frameInfo, ubo);
                    m_RenderSystem->renderSpotShadowPass    (frameInfo, ubo);
                }

                // [ Render ]
                {
                    m_Renderer.beginSwapchainRenderPass( cmdBuffer );
                    {
                        // order matters
                        // solid objects first, then transparent
                        m_RenderSystem->renderMainPass( frameInfo );
                        m_PointLightSystem->render( frameInfo, ubo );
                    }
                    m_Renderer.endRenderPass( cmdBuffer );
                }

                // [ Update ]
                // {
                //     m_Renderer.updateCurrentShadowUbo(&shadowUbo);
                    
                //     {
                //         gBufferUbo.projection = camera.projection();
                //         gBufferUbo.view       = camera.view();
                        
                //         m_Renderer.updateCurrentGBufferUbo(&gBufferUbo);
                //     }

                //     {
                //         mappingsUbo.projection = camera.projection();
                //         mappingsUbo.view       = camera.view();
                        
                //         m_Renderer.updateCurrentMappingsUbo(&mappingsUbo);
                //     }

                //     {
                //         uvReflectionUbo.viewPos    = camera.position();
                //         uvReflectionUbo.projection = camera.projection();
                //         uvReflectionUbo.view       = camera.view();
                        
                //         m_Renderer.updateCurrentUVReflectionUbo(&uvReflectionUbo);
                //     }

                //     {
                //         compositionUbo.viewPos = camera.position();
                        
                //         m_Renderer.updateCurrentCompositionUbo(&compositionUbo);
                //     }
                    
                //     m_Renderer.updateCurrentPostProcessingUbo(&postProcessingUbo);
                // }
                    
                // // [ Render ]
                // {
                //     // render shadows
                //     {
                //         m_Renderer.beginShadowRenderPass( cmdBuffer );
                //         {
                //             m_ShadowSystem->render( frameInfo );
                //         }
                //         m_Renderer.endRenderPass( cmdBuffer );
                //     }

                //     // render mappings
                //     {
                //         m_Renderer.beginMappingsRenderPass( cmdBuffer );
                //         {
                //             m_ReflectionSystem->renderMappings( frameInfo );
                //         }
                //         m_Renderer.endRenderPass( cmdBuffer );
                //     }

                //     // render reflection map
                //     {
                //         m_Renderer.beginUVReflectionRenderPass( cmdBuffer );
                //         {
                //             m_ReflectionSystem->renderUVReflectionMap( frameInfo );
                //         }
                //         m_Renderer.endRenderPass( cmdBuffer );
                //     }

                //     // render swapchain
                //     {
                //         m_Renderer.beginLightingRenderPass( cmdBuffer );
                //         {
                //             Mat4 invVP = glm::inverse( camera.projection() * camera.view() );
                            
                //             m_SceneSystem->render( frameInfo, invVP, invResolution );
                            
                //             m_PointLightSystem->render( frameInfo );
                //         }
                //         m_Renderer.endRenderPass( cmdBuffer );
                        
                //         m_Renderer.beginPostProcessingRenderPass( cmdBuffer );
                //         {
                //             m_SceneSystem->renderPostProcessing( frameInfo );
                //         }
                //         m_Renderer.endRenderPass( cmdBuffer );
                //     }
                // }

                // [ Render ]
                // {
                    // m_Renderer.beginSwapchainRenderPass( cmdBuffer );
                    // {
                    //     m_SkyboxSystem->render( frameInfo, m_Skybox.get() );
                        
                    //     m_ModelSystem ->render( frameInfo );

                    //     m_LightSystem ->render( frameInfo );

                    //     m_GridSystem  ->render( frameInfo );
                    // }
                    // m_Renderer.endCurrentRenderPass( cmdBuffer );
                // }

                m_Renderer.endFrame();
                
            } // [ Frame End ]
            
            m_FrameCount++;

        } // [ Main Loop End ]

        VyContext::waitIdle();

        VY_INFO_TAG("VyEngine", "Rendered {} frames.", m_FrameCount);
    }


    void VyEngine::loadEntities()
    {
        const float kGroundSize = 40.0f;

        auto ground = m_Scene->createEntity( "ground" );
        {
            ground.add<ModelComponent>( m_Models[ 0 ] );
            ground.get<TransformComponent>() = TransformComponent{
                Vec3(0.0f, 0.0f, 0.0f),
                Vec3(1.0f * kGroundSize, 0.05f, 1.0f * kGroundSize)
            };
        }

        // auto cube = m_Scene->createEntity( "cube" );
        // {
        //     cube.add<ModelComponent>( m_Models[ 0 ] );
        //     cube.get<TransformComponent>().Translation = Vec3(0.0f, 0.0f, 0.0f);
        // }

        // Shared<VyModel> pModel;

        // pModel = VyModel::loadFromFile( MODELS_DIR "smooth_vase.obj" );

        // auto vase = m_Scene->createEntity( "vase" );
        // {
        //     vase.add<ModelComponent>( pModel );
        //     vase.get<TransformComponent>().Translation = Vec3(0.0f, -0.01f, 0.0f);
        // }

        // pModel = VyModel::loadFromFile( MODELS_DIR "colored_cube.obj" );

        // auto cube = m_Scene->createEntity( "cube" );
        // {
        //     cube.add<ModelComponent>( pModel );
        //     cube.get<TransformComponent>() = TransformComponent{ 
        //         Vec3(-0.6f, -0.21f, 0.6f),
        //         Vec3(0.2f, 0.2f, 0.2f)
        //     };
        // }

        // pModel = VyModel::loadFromFile( MODELS_DIR "plane.obj" );

        // auto plane = m_Scene->createEntity( "plane" );
        // {
        //     plane.add<ModelComponent>( pModel );
        //     plane.get<TransformComponent>() = TransformComponent{ 
        //         Vec3(0.0f, -0.01f, 0.0f),
        //         Vec3(1.0f, 1.0f, 1.0f)
        //     };
        // }

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
            auto rotateLight = glm::rotate(Mat4( 1.0f ),
                (i * glm::two_pi<float>()) / lightColors.size(),
                { 0.0f, 2.0f, 0.0f }
            );

            auto pointLight = m_Scene->createEntity("point_light_" + std::to_string( i ));
            {
                pointLight.add<LightComponent>( LightComponent::pointLight( lightColors[ i ], 0.5f, 0.2f ) );
                pointLight.get<TransformComponent>() = TransformComponent{
                    rotateLight * Vec4(-1.0f, -1.0f, -1.0f, 1.0f)//,
                    // { 0.2f, 0.2f, 0.2f }
                };
            }
        }
    }
}