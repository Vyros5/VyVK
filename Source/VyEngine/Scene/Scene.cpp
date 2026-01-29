#include <VyEngine/Scene/Scene.h>

#include <VyEngine/Scene/ECS/Components.h>
#include <VyEngine/Scene/ECS/Entity.h>

// #include <VyEngine/Systems/Logic/CameraSystem.h>
// #include <VyEngine/Systems/Logic/TransformSystem.h>
// #include <VyEngine/Systems/Logic/HierarchySystem.h>
#include <VyEngine/Engine.h>
#include <VyEngine/Scripting/Scripts/CameraController.h>

#include <VyEngine/VK/Swapchain/Swapchain.h>

#include <iostream>
#include <VyLib/Common/AnsiColor.h>

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
                CAMERA_POSITION
                // /* Translation */ { 0.0f, -1.0f, -5.0f }, // (5 spaces back, and 1 up)
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


    void VyScene::setEntityParent(EntityHandle child, EntityHandle parent)
    {
        if (!m_Registry.valid(child) && !m_Registry.valid(parent))
        {
            throw std::invalid_argument("Entity or parent is invalid");
        }

        auto& oldHierarchy = m_Registry.get<HierarchyComponent>(child);

        if (m_Registry.valid(oldHierarchy.Parent))
        {
            auto &oldParentHierarchy = m_Registry.get<HierarchyComponent>(oldHierarchy.Parent);
            auto &children           = oldParentHierarchy.Children;

            children.erase(std::remove(children.begin(), children.end(), child), children.end());

            oldHierarchy.Parent = entt::null;
        }

        oldHierarchy.Parent = parent;
        auto& newHierarchy  = m_Registry.get<HierarchyComponent>(parent);
        
        newHierarchy.Children.push_back(child);
    }

    
    const TVector<EntityHandle>& VyScene::getEntityChildren(EntityHandle entity)
    {
        if (m_Registry.all_of<HierarchyComponent>(entity))
        {
            return m_Registry.get<HierarchyComponent>(entity).Children;
        }

        static const TVector<EntityHandle> empty;
        return empty;
    }

    EntityHandle VyScene::getEntityParent(EntityHandle entity)
    {
        if (m_Registry.all_of<HierarchyComponent>(entity))
        {
            return m_Registry.get<HierarchyComponent>(entity).Parent;
        }
        return entt::null;
    }


    void VyScene::print()
    {
        auto view = this->registry().view<TagComponent>();

		std::stringstream ss;
        ss  << "\n--------------------------------------------------------------------------" << '\n'
			<< "[" << YELLOW "Scene Stats" RESET "] " << '\n'
            << " - Total Entities : " << view.size()  << '\n'
            << '\n'
            <<  " - Entities: " << '\n'
        ;

		for (auto&& [ entity, tag ] : view.each())
		{
			ss  << "    - " << "(" << GRAY << (U32)entity << RESET << ") " CYAN << tag.Tag.c_str() << RESET << '\n';
		}

		ss  << "--------------------------------------------------------------------------" << '\n';

        std::cout << ss.str() << std::endl;
    }
}