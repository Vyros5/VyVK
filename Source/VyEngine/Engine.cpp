#include <VyEngine/Engine.h>

#include <VyEngine/VK/Context.h>
#include <VyEngine/Scene/ECS/Components.h>
#include <VyEngine/Core/FrameRateController.h>

#include <iostream>
#include <VyLib/Common/AnsiColor.h>

#include <VyEngine/Core/Stats.h>

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


    void VyEngine::initDescriptors()
    {
        m_GlobalPool = VyDescriptorPool::Builder()
            .setName    ("global")
            .setMaxSets (1000)
            .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         1000)
            .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000)
            .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000)
            .buildPtr();

        // Create the global uniform buffers (One per frame). 
        for (int i = 0; i < m_UniformBuffers.size(); i++) 
        {
            m_UniformBuffers[ i ] = MakeUnique<VyBuffer>( VyBuffer::uniformBuffer("global", sizeof(GlobalUbo)) );
        }

        m_GlobalSetLayout = VyDescriptorSetLayout::Builder{}
            .setName   ("global")
            .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         VK_SHADER_STAGE_ALL_GRAPHICS)
            .addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
            .buildPtr();

        m_MaterialSetLayout = VyDescriptorSetLayout::Builder{}
            .setName   ("material")
            .addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT) // ALB
            .addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT) // NRM
            .addBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT) // MR
            .addBinding(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT) // AO
            .addBinding(4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT) // EMIS
            .buildPtr();

        m_TempGroundTexture = VyTexture::createFromFilepath(TEXTURE_DIR "Ground.png");

        VkDescriptorImageInfo imageInfo = m_TempGroundTexture->descriptorImageInfo();

        for (U32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) 
        {
            VkDescriptorBufferInfo bufferInfo = m_UniformBuffers[ i ]->descriptorBufferInfo();

            VyDescriptorWriter( *m_GlobalSetLayout, *m_GlobalPool )
                .writeBuffer( 0, &bufferInfo )
                .writeImage ( 1, &imageInfo  )
                .build( m_GlobalSets[ i ] );
        }

        m_SetLayouts.push_back( m_GlobalSetLayout  ->handle() );
        m_SetLayouts.push_back( m_MaterialSetLayout->handle() );
    }


    void VyEngine::initialize()
    {
        // Create and reset (initialize) the scene.
        m_Scene = MakeShared<VyScene>( "main-scene" );
        m_Scene->reset();

        // m_Skybox = VySkybox::loadFromFolder(TString(CUBEMAP_DIR) + "Yokohama", "jpg");

        initDescriptors();

        // Load builtin entities for the current scene. 
        loadEntities();

        // [ Initialize Rendering Systems ]
        m_RenderSystem     = MakeUnique<VyRenderSystem>    (m_Renderer.swapchainRenderPass(), m_SetLayouts);
        m_PointLightSystem = MakeUnique<VyPointLightSystem>(m_Renderer.swapchainRenderPass(), m_GlobalSetLayout->handle());
        m_GridSystem       = MakeUnique<VyGridSystem>      (m_Renderer.swapchainRenderPass(), m_GlobalSetLayout->handle());
        
        printEntities();
    }


    void VyEngine::printEntities()
    {
        auto view = m_Scene->registry().view<TagComponent>();

        std::stringstream ss;
        ss  << GRAY "Loaded Entities: (" << view.size() << ")" RESET << '\n';
        
        for (auto&& [ entity, tag ] : view.each())
        {
            ss  << CYAN "  - '" << tag.Tag.c_str() << "'" RESET << '\n';
        }

        VY_TRACE("{0}", ss.str());
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
        VyStats stats{};

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
                }

                // Update Frame Info.
                int frameIndex = m_Renderer.frameIndex();

                VyFrameInfo frameInfo{
                    .FrameIndex          = frameIndex,                 // Index of the current frame.
                    .FrameTime           = deltaTime,                  // Time between frames.
                    .CommandBuffer       = cmdBuffer,                  // Main command buffer.
                    .GlobalSet           = m_GlobalSets[ frameIndex ], // Global descriptor set for the current frame.
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
                }

                // [ Render ]
                {
                    // Note: Order matters - Solid objects first, then transparent.

                    m_Renderer.beginSwapchainRenderPass( cmdBuffer );
                    {
                        m_RenderSystem->renderMainPass( frameInfo );

                        m_PointLightSystem->render( frameInfo, ubo );

                        m_GridSystem->render( frameInfo );
                    }
                    m_Renderer.endRenderPass( cmdBuffer );
                }

                m_Renderer.endFrame();
                
            } // [ Frame End ]
            
            stats.nextFrame();

        } // [ Main Loop End ]

        VyContext::waitIdle();

        stats.print();
    }


    void VyEngine::loadEntities()
    {
        m_Models.push_back( VyGLTFModel::createFromFile( MODELS_DIR "Helmet/DamagedHelmet.gltf", *m_MaterialSetLayout, *m_GlobalPool) );
        // m_Models.push_back( VyGLTFModel::createFromFile( MODELS_DIR "BoomBox/BoomBox.gltf", *m_MaterialSetLayout, *m_GlobalPool) );
        m_Models.push_back( VyGLTFModel::createFromFile( MODELS_DIR "Cube/Cube.gltf", *m_MaterialSetLayout, *m_GlobalPool) );
        
        auto helmet = m_Scene->createEntity( "helmet" );
        {
            helmet.add<ModelComponent>( m_Models[ 0 ] );
            helmet.get<TransformComponent>() = TransformComponent{
                Vec3(0.0f, -1.0f, 0.0f),
                Vec3(1.0f, 1.0f, 1.0f),
                Vec3( -glm::pi<float>() / 2.0f, 0.0f, 0.0f )
            };
        }
        
        // auto boombox = m_Scene->createEntity( "boombox" );
        // {
        //     boombox.add<ModelComponent>( m_Models[ 0 ] );
        //     boombox.get<TransformComponent>() = TransformComponent{
        //         Vec3(0.0f, -1.0f, 0.0f),
        //         Vec3(20.0f, 20.0f, 20.0f),
        //         Vec3( glm::pi<float>() , 0.0f, 0.0f )
        //     };
        // }

        const float kGroundSize = 20.0f;

        auto ground = m_Scene->createEntity( "ground" );
        {
            ground.add<ModelComponent>( m_Models[ 1 ] );

            ground.get<TransformComponent>() = TransformComponent{
                Vec3(0.0f, 0.0f, 0.0f),
                Vec3(1.0f * kGroundSize, 0.05f, 1.0f * kGroundSize)
            };
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
                { 0.0f, 2.5f, 0.0f }
            );

            auto pointLight = m_Scene->createEntity("point_light_" + std::to_string( i ));
            {
                pointLight.add<LightComponent>( LightComponent::pointLight( lightColors[ i ], 0.5f, 0.2f ) );
                pointLight.get<TransformComponent>() = TransformComponent{
                    rotateLight * Vec4(-1.0f, -2.5f, -1.0f, 1.0f)
                };
            }
        }
    }
}