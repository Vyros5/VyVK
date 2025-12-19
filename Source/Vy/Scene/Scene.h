#pragma once

#include <Vy/Scene/ECS/Entity.h>

#include <Vy/Scripting/ScriptManager.h>
#include <Vy/Systems/Logic/ILogicSystem.h>
#include <Vy/Scene/Environment.h>

#include <Vy/Scene/ECS/Components/PostProcessingComponent.h>

namespace Vy
{
	/**
	 * @brief VyScene containing all entities and systems.
	 */
	class VyScene : public std::enable_shared_from_this<VyScene>
    {
        friend class VyEntity;
        friend class VySceneSerializer;

	public:
		VyScene() = default;
		
		VyScene(const VyScene&)            = delete;
		VyScene(VyScene&&)                 = delete;
		
		VyScene& operator=(const VyScene&) = delete;
		VyScene& operator=(VyScene&&)      = delete;
		
		~VyScene() = default;

		VY_NODISCARD VyEntity mainCamera() const
		{ 
			return m_MainCamera; 
		}

		// VY_NODISCARD VyEntity sunDirectionalLight() const
		// { 
		// 	return m_Sun; 
		// }

        /**
         * @brief Called when the scene starts.
         * 
         * Initializes scripts attached to entities.
         */
        // void onStart();
        
        /**
         * @brief Called every frame to update the scene.
         * @param deltaTime Time elapsed since the last update.
         */
        void update(float deltaTime);


		/**
		 * @brief Gets the entity registry of the current scene.
		 */
		VY_NODISCARD 
        entt::registry& registry() 
        { 
            return m_Registry; 
        }

		String getName() const { return m_SceneName; }
        void setName(String name) { m_SceneName = name; }

		/**
		 * @brief Create a new VyEntity in the scene.
		 * 
		 * @param name Optional name for the entity.
		 * @param uuid Optional UUID for the entity. If not provided, a new UUID is generated.
		 * 
		 * @return The newly created VyEntity.
		 */
		VyEntity createEntity(const String& name = String(), VyUUID uuid = VyUUID());

        /**
         * @brief Retrieves an entity by its UUID.
		 * 
         * @param uuid The UUID of the entity.
		 * 
         * @return The corresponding entity.
         */
        VyEntity getEntity(VyUUID uuid);

		/**
		 * @brief Destroy a VyEntity from the scene.
		 * 
		 * @param entity The VyEntity to destroy.
		 */
		void destroyEntity(VyEntity entity);

        /**
         * @brief Retrieves all entities that have the specified components.
         * 
         * @tparam T Component types to filter entities.
         * 
         * @return A view of the entities with the specified components.
         */
        template <typename ...T>
        auto getEntitiesWith() 
        {
            return m_Registry.view<T...>();
        }

        /**
         * @brief Gets the entity designated as the main camera.
         * 
         * @return The main camera entity or an empty entity if none exist.
         */
        // VyEntity getMainCameraEntity();

		PostProcessingComponent getPostProcessingComponent() const
		{
			return m_PostProcessingSettings;
		}

        /**
         * @brief Retrieves the environment settings for the scene.
		 * 
         * @return A shared pointer to the environment settings.
		 */
        Shared<VyEnvironment> getEnvironment() const { return m_Environment; }


		/**
		 * @brief Adds tracking for a script component to call its virtual functions.
		 * 
		 * @param script 
		 */
		void addScript(VyEntityScript* script) 
		{ 
			m_ScriptManager.addScript(script); 
		}

		/**
		 * @brief Adds a System to the scene.
		 * 
		 * @tparam T System that is `SystemDerived`.
		 */
		template <LogicSystemDerived T, typename... Args>
			requires std::constructible_from<T, Args...>
		void addLogicSystem(Args&&... args)
		{
			m_LogicSystem.emplace_back(std::make_unique<T>(std::forward<Args>(args)...));
		}


		void reset();

	private:

		void addBuiltinSystems();
		void createMainCamera();

	private:

		String m_SceneName{"Unnamed-Scene"};
		
		THashMap<VyUUID, EntityHandle> m_EntityMap;

		/* VyScene Registry for storing all the entities belonging to this scene. */
		entt::registry m_Registry;

        VyScriptManager               m_ScriptManager;
		TVector<Unique<ILogicSystem>> m_LogicSystem;

		Shared<VyEnvironment> m_Environment = MakeShared<VyEnvironment>();

		VyEntity m_MainCamera;

		PostProcessingComponent m_PostProcessingSettings{};
		// VyEntity m_Sun;

		// VyEntity m_AmbientLight;
		// VyEntity m_DirectionalLight;
		// VyEntity m_Skybox;
    };
}