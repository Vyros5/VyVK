#pragma once

#include <VyEngine/Scene/ECS/Entity.h>
#include <VyEngine/Scene/ECS/Components/CameraComponent.h>
#include <VyEngine/Scripting/ScriptManager.h>

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
		VyScene(const TStringView name);
		
		VyScene(const VyScene&)            = delete;
		VyScene(VyScene&&)                 = delete;
		
		VyScene& operator=(const VyScene&) = delete;
		VyScene& operator=(VyScene&&)      = delete;
		
		~VyScene();
        
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

		TString getName() const { return m_SceneName; }
        void setName(TString name) { m_SceneName = name; }

		/**
		 * @brief Create a new VyEntity in the scene.
		 * 
		 * @param name Optional name for the entity.
		 * @param uuid Optional UUID for the entity. If not provided, a new UUID is generated.
		 * 
		 * @return The newly created VyEntity.
		 */
		VyEntity createEntity(const TString& name = TString(), VyUUID uuid = VyUUID());


		// template <typename T>
		// Shared<T> createEntity(TString name) 
		// {
		// 	static_assert(std::is_base_of_v<VyEntity, T>, "T must be derived from VyEntity");

		// 	Shared<T> entity = MakeShared<T>( m_Registry.create(), this, name );
			
		// 	m_Entities.push_back( entity );
			
		// 	return entity;
		// }


        // /**
        //  * @brief Retrieves an entity by its UUID.
		//  * 
        //  * @param uuid The UUID of the entity.
		//  * 
        //  * @return The corresponding entity.
        //  */
        // VyEntity getEntity(VyUUID uuid);

		/**
		 * @brief Destroy a VyEntity from the scene.
		 * 
		 * @param entity The VyEntity to destroy.
		 */
		void destroyEntity(VyEntity entity);

		void destroyEntity(U32 index);

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
		VY_NODISCARD VyEntity mainCameraEntity() const;

		// VY_NODISCARD CameraComponent& mainCameraComponent();

		void setMainCamera(VyEntity cameraEntity);

		// VY_NODISCARD bool hasMainCamera() const;

		// PostProcessingComponent getPostProcessingComponent() const
		// {
		// 	return m_PostProcessingSettings;
		// }

        /**
         * @brief Retrieves the environment settings for the scene.
		 * 
         * @return A shared pointer to the environment settings.
		 */
        // Shared<VyEnvironment> getEnvironment() const 
		// { 
		// 	return m_Environment; 
		// }


		/**
		 * @brief Adds tracking for a script component to call its virtual functions.
		 * 
		 * @param script 
		 */
		void addScript(VyEntityScript* pScript) 
		{ 
			m_ScriptManager.addScript( pScript ); 
		}

		/**
		 * @brief Adds a System to the scene.
		 * 
		 * @tparam T System that is `SystemDerived`.
		 */
		// template <LogicSystemDerived T, typename... Args>
		// 	requires std::constructible_from<T, Args...>
		// void addLogicSystem(Args&&... args)
		// {
		// 	m_LogicSystem.emplace_back( std::make_unique<T>(std::forward<Args>(args)...) );
		// }

		void reset();

	private:

		void addBuiltinSystems();
		void createMainCamera();

	private:

		TString m_SceneName{ "unnamed-scene" };
		
		THashMap<VyUUID, EntityHandle> m_EntityMap;
		TVector<Shared<VyEntity>> m_Entities;
		/* VyScene Registry for storing all the entities belonging to this scene. */
		entt::registry m_Registry;

        VyScriptManager m_ScriptManager;
		// TVector<Unique<ILogicSystem>> m_LogicSystem;

		// Shared<VyEnvironment> m_Environment = MakeShared<VyEnvironment>();

		VyEntity m_MainCamera;
    };
}