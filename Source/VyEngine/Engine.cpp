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
        loadEntities();

        m_Skybox = VySkybox::loadFromFolder(TString(CUBEMAP_DIR) + "Yokohama", "jpg");

        m_GlobalPool = VyDescriptorPool::Builder{}
            .setName    ("global")
            .setMaxSets (MAX_FRAMES_IN_FLIGHT)
            .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, MAX_FRAMES_IN_FLIGHT)
            .buildPtr();

        // Create the global uniform buffers (One per frame). 
        for (int i = 0; i < m_UniformBuffers.size(); i++) 
        {
            m_UniformBuffers[ i ] = MakeUnique<VyBuffer>( VyBuffer::uniformBuffer("global", sizeof(GlobalUbo)) );
        }

        m_GlobalSetLayout = VyDescriptorSetLayout::Builder{}
            .setName   ("global")
            .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
            .buildPtr();

        for (U32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) 
        {
            VkDescriptorBufferInfo bufferInfo = m_UniformBuffers[ i ]->descriptorBufferInfo();

            VyDescriptorWriter( *m_GlobalSetLayout, *m_GlobalPool )
                .writeBuffer( 0, &bufferInfo )
                .build( m_GlobalSets[ i ] );
        }

        TVector<VkDescriptorSetLayout> setLayouts = {
            m_GlobalSetLayout->handle(),
        };

        // [ Initialize Rendering System ]
        m_ModelSystem = MakeUnique<VyModelRenderSystem>( 
            m_Renderer.swapchainRenderPass(), 
            setLayouts 
        );
        
        m_LightSystem = MakeUnique<VyLightSystem>( 
            m_Renderer.swapchainRenderPass(), 
            m_GlobalSetLayout->handle()
        );

        m_GridSystem = MakeUnique<VyGridSystem>( 
            m_Renderer.swapchainRenderPass(), 
            m_GlobalSetLayout->handle()
        );

        m_SkyboxSystem = MakeUnique<VySkyboxRenderSystem>(
            m_Renderer.swapchainRenderPass(),
            m_GlobalSetLayout->handle()
        );

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

        // [ Initialize FrameRate Controller (60 FPS) ]
        FrameRateController frameRateController{ 60u };

        VkExtent2D previousExtent = m_Renderer.swapchainExtent();

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

            // [ Frame ]
            if (auto cmdBuffer = m_Renderer.beginFrame()) 
            {
                // Check if window was resized and recreate post-processing resources.
                {
                    VkExtent2D currentExtent = m_Renderer.swapchainExtent();
                    
                    if (currentExtent.width  != previousExtent.width || 
                        currentExtent.height != previousExtent.height) 
                    {
                        // m_RenderSystem->recreate(currentExtent);
                        
                        previousExtent = currentExtent;
                    }
                }

                // Update Frame Info.
                int frameIndex = m_Renderer.frameIndex();

                VyFrameInfo frameInfo{
                    .FrameIndex          = frameIndex,                    // Index of the current frame.
                    .FrameTime           = deltaTime,                     // Time between frames.
                    .CommandBuffer       = cmdBuffer,                     // Main command buffer.
                    .GlobalDescriptorSet = m_GlobalSets[ frameIndex ],    // Global descriptor set for the current frame.
                    .Scene               = m_Scene,                       // Active scene.
                    .Camera              = camera                         // Active camera to update the UBOs.
                };

                // [ Update ]
                {
                    GlobalUbo ubo{};

                    // [ Update UBO Data ]
                    {
                        ubo.Projection  = frameInfo.Camera.projection();
                        ubo.View        = frameInfo.Camera.view();
                        ubo.InverseView = frameInfo.Camera.inverseView();
                    }

                    // Update light values into UBO.
                    m_LightSystem->update( frameInfo, ubo );

                    // Write global uniform buffers.
                    m_UniformBuffers[ frameInfo.FrameIndex ]->write( &ubo, sizeof(GlobalUbo), 0 );
                }

                // [ Render ]
                {
                    m_Renderer.beginSwapchainRenderPass( cmdBuffer );
                    {
                        m_SkyboxSystem->render( frameInfo, m_Skybox.get() );
                        
                        m_ModelSystem ->render( frameInfo );

                        m_LightSystem ->render( frameInfo );

                        m_GridSystem  ->render( frameInfo );
                    }
                    m_Renderer.endCurrentRenderPass( cmdBuffer );
                }

                m_Renderer.endFrame();

            } // [ Frame End ]

        } // [ Main Loop End ]

        VyContext::waitIdle();
    }


    void VyEngine::loadEntities()
    {
        Shared<VyModel> pModel;
        
        pModel = VyModel::loadFromFile( MODELS_DIR "smooth_vase.obj" );

        auto vase = m_Scene->createEntity( "vase" );
        {
            vase.add<ModelComponent>( pModel );
            vase.get<TransformComponent>().Translation = Vec3(0.0f, -0.01f, 0.0f);
        }

        pModel = VyModel::loadFromFile( MODELS_DIR "plane.obj" );

        auto plane = m_Scene->createEntity( "plane" );
        {
            plane.add<ModelComponent>( pModel );
            plane.get<TransformComponent>().Translation = Vec3(0.0f, -0.01f, 0.0f);
        }

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
                pointLight.add<PointLightComponent>( lightColors[ i ], 0.5f );
                pointLight.get<TransformComponent>() = TransformComponent{
                    rotateLight * Vec4(-1.0f, -1.0f, -1.0f, 1.0f)//,
                    // { 0.2f, 0.2f, 0.2f }
                };
            }
        }
    }
}