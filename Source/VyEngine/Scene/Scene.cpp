#include <VyEngine/Scene/Scene.h>

#include <VyEngine/Scene/ECS/Components.h>
#include <VyEngine/Scene/ECS/Entity.h>

// #include <VyEngine/Systems/Logic/CameraSystem.h>
// #include <VyEngine/Systems/Logic/TransformSystem.h>
// #include <VyEngine/Systems/Logic/HierarchySystem.h>
#include <VyEngine/Engine.h>
#include <VyEngine/Scripting/Scripts/CameraController.h>

namespace Vy
{
    VyScene::VyScene(const TStringView name) :
        m_SceneName{ name }
    {

    }
    

    VyScene::~VyScene()
    {
        m_Registry.clear();
    }


	void VyScene::update(float deltaTime)
	{
        // Update scripts and contollers.
        m_ScriptManager.update( deltaTime );
	}


    VyEntity VyScene::createEntity(const TString& name, VyUUID uuid)
    {
        // Create entity in the registry.
        EntityHandle handle = m_Registry.create();

        // Create VyEntity wrapper.
        VyEntity entity = { handle, this };

        // Add Required Components.
        entity.add<IDComponent>(uuid);
        entity.add<TagComponent>(name.empty() ? "unnamed-entity" : name);
        
        entity.add<TransformComponent>();

        m_EntityMap[ entity.entityUUID() ] = entity;

        return entity;
    }

    // VyEntity VyScene::mainCameraEntity()
    // {
    //     VY_ASSERT(m_MainCamera.get<CameraComponent>().IsMainCamera, "IsMainCamera should be true");
    //     // VY_ASSERT(m_MainCamera.isValid(),                           "Main Camera is invalid"     );

    //     return m_MainCamera;
    // }


    // VyEntity VyScene::getEntity(VyUUID uuid) 
    // {
    //     VY_ASSERT(m_EntityMap.contains(uuid), "Entity not found in Scene!");

    //     return { m_EntityMap.at(uuid), this };
    // }


    void VyScene::destroyEntity(VyEntity entity)
    {
        m_EntityMap.erase( entity.entityUUID() );
        
        m_Registry.destroy( entity );
    }


    void VyScene::destroyEntity(U32 index)
    {
        m_Registry.destroy( m_Entities[ index ]->handle() );
        m_Entities.erase( m_Entities.begin() + index );
    }
    
    
    void VyScene::reset()
    {
        // Add Systems to the scene.
		addBuiltinSystems();

        createMainCamera();
    }


    void VyScene::createMainCamera()
    {
        // Create the main camera and attach components / controller scripts.
		m_MainCamera = this->createEntity("main-camera");
		{
			m_MainCamera.add<CameraComponent>().IsMainCamera = true;
			m_MainCamera.add<CameraController>();
            m_MainCamera.get<TransformComponent>() = TransformComponent{
                /* Translation */ { 0.0f, -1.0f, -5.0f }, // (5 spaces back, and 1 up)
            };
		}

        VY_ASSERT(m_MainCamera.get<CameraComponent>().IsMainCamera, "IsMainCamera should be true");
        VY_ASSERT(m_MainCamera.isValid(),                           "Main Camera is invalid"     );
    }


    void VyScene::setMainCamera(VyEntity cameraEntity)
    {
        if (!cameraEntity.isValid() || !cameraEntity.has<CameraComponent>()) 
        {
            return;
        }

        // Mark camera as main camera via component flag.
        cameraEntity.get<CameraComponent>().IsMainCamera = true;

        // Unmark any other cameras.
        auto view = m_Registry.view<CameraComponent>();
        
        for (auto entity : view) 
        {
            if (VyEntity( entity, this ) != cameraEntity) 
            {
                view.get<CameraComponent>( entity ).IsMainCamera = false;
            }
        }
    }


    VyEntity VyScene::mainCameraEntity() const
    {
        return m_MainCamera;

        // Query for camera with IsMainCamera flag.
        // auto view = m_Registry.view<CameraComponent>();

        // for (auto entity : view) 
        // {
        //     if (view.get<CameraComponent>( entity ).IsMainCamera) 
        //     {
        //         return VyEntity( entity, const_cast<VyScene*>( this ) );
        //     }
        // }

        // return VyEntity::null();
    }


    // CameraComponent& VyScene::mainCameraComponent()
    // { 
    //     return m_MainCamera.get<CameraComponent>(); 
    // }


    // bool VyScene::hasMainCamera() const
    // {
    //     return m_MainCamera.isValid() && m_MainCamera.has<CameraComponent>();
    // }


    void VyScene::addBuiltinSystems()
    {
		// addLogicSystem<VyTransformSystem>();
        // addLogicSystem<HierarchySystem>();
		// addLogicSystem<CameraSystem>();
    }
}