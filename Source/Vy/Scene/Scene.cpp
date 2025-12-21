#include <Vy/Scene/Scene.h>

#include <Vy/Scene/ECS/Components.h>
#include <Vy/Scene/ECS/Entity.h>

#include <Vy/Systems/Logic/CameraSystem.h>
#include <Vy/Systems/Logic/TransformSystem.h>
#include <Vy/Systems/Logic/HierarchySystem.h>

#include <Vy/Scripting/Scripts/CameraController.h>
#include <Vy/Scripting/Scripts/KinematicMovementController.h>
#include <Vy/Engine.h>

namespace Vy
{
	void VyScene::update(float deltaTime)
	{
        m_ScriptManager.update(deltaTime);

        for (auto& system : m_LogicSystem)
        {
            system->update(m_Registry, deltaTime);
        }
        
	}


    VyEntity VyScene::createEntity(const String& name, VyUUID uuid)
    {
        VyEntity entity = { m_Registry.create(), this };

        // Add Required Components.
        entity.add<IDComponent>(uuid);
        entity.add<TagComponent>(name.empty() ? "Unnamed-Entity" : name);
        
        entity.add<TransformComponent>();

        entity.add<ParentComponent>();
        entity.add<ChildrenComponent>();
        
        m_EntityMap[ entity.getUUID() ] = entity;

        return entity;
    }


    VyEntity VyScene::getEntity(VyUUID uuid) 
    {
        VY_ASSERT(m_EntityMap.contains(uuid), "Entity not found in Scene!");

        return { m_EntityMap.at(uuid), this };
    }


    void VyScene::destroyEntity(VyEntity entity)
    {
        m_EntityMap.erase(entity.getUUID());
        
        m_Registry.destroy(entity);
    }

    
    void VyScene::reset()
    {
        // Add Systems to the scene.
		addBuiltinSystems();

        createMainCamera();

        PostProcessingComponent postProc{};
        {
            postProc.BloomEnabled    = true;
            postProc.BloomThreshold  = 0.8f;
            postProc.BloomIntensity  = 0.5f;
            postProc.BloomIterations = 5; // Note: Do not change, This number is hardcoded in the shader.

            postProc.Exposure        = 1.0f;
            postProc.Gamma           = 1.65f;
        }

        m_PostProcessingSettings = postProc;
    }


    void VyScene::createMainCamera()
    {
        // Create the main camera and attach components / controller scripts.
		m_MainCamera = createEntity("MainCamera");
		{
			m_MainCamera.add<CameraComponent>();
			m_MainCamera.add<CameraController>();
            m_MainCamera.get<TransformComponent>() = TransformComponent{
                /* Translation */ { 0.0f, -1.0f, -5.0f },
            };
		}

        VY_ASSERT(m_MainCamera.get<CameraComponent>().IsMainCamera, "IsMainCamera should be true");
        VY_ASSERT(m_MainCamera.isValid(),                           "Main Camera is invalid"     );
    }


    void VyScene::addBuiltinSystems()
    {
		// addLogicSystem<VyTransformSystem>();
        // addLogicSystem<HierarchySystem>();
		// addLogicSystem<CameraSystem>();
    }
}