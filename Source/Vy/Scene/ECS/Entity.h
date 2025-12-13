#pragma once

#include <Vy/Scene/ECS/EntityHandle.h>

#include <entt/entt.hpp>

#include <Vy/Scene/ECS/Components/IDComponent.h>

namespace Vy
{
	class VyScene;

	class VyEntityScript;

	template<typename T>
	concept IsScript = std::is_base_of_v<VyEntityScript, T>;

    /**
     * @brief An entity represents any object in a scene.
     * 
     * @note This class is meant to be passed by value since its just an ID.
     */
    class VyEntity
    {
        friend class VyScene;
        friend class SceneSerializer;

	public:
		VyEntity() = default;

		VyEntity(EntityHandle handle, VyScene* scene);

		VyEntity(const VyEntity&)            = default;
		VyEntity(VyEntity&&)                 = default;

		VyEntity& operator=(const VyEntity&) = default;
		VyEntity& operator=(VyEntity&&)      = default;

		bool operator==(const VyEntity& other) const;
		bool operator!=(const VyEntity& other) const;

        ~VyEntity();
        
        // ----------------------------------------------------------------------------------------

		VY_NODISCARD bool isValid()
		{
			return m_Scene                       && 
                m_Handle != kInvalidEntityHandle && 
                registry().valid(m_Handle);
		}

		VY_NODISCARD bool isValid() const
		{
			return m_Scene                       && 
                m_Handle != kInvalidEntityHandle && 
                registry().valid(m_Handle);
		}

		operator bool()         const { return isValid();  }
		operator EntityHandle() const { return m_Handle;   }
		operator U32()          const { return static_cast<U32>(m_Handle); }

		VY_NODISCARD entt::registry& registry() const;

        // ----------------------------------------------------------------------------------------

        /**
         * @brief Checks if the entity has all of the specified components.
         * 
         * @tparam T Type of component(s) to check.
         * 
         * @note This will assert if the VyEntity is valid.
         */
        template <typename... T>
        VY_NODISCARD bool has()
		{
            VY_ASSERT(isValid());

			return registry().all_of<T...>(m_Handle);
		}


        /**
         * @brief Checks if the entity has all of the specified components.
         * 
         * @tparam T Type of component(s) to check.
         * 
         * @note This will assert if the VyEntity is valid.
         */
        template <typename... T>
        VY_NODISCARD bool has() const
		{
            VY_ASSERT(isValid());

			return registry().all_of<T...>(m_Handle);
		}


        /**
         * @brief Checks if the entity has any of the specified components.
         * 
         * @tparam T Type of component(s) to check.
         * 
         * @note This will assert if the VyEntity is valid.
         */
        template <typename... T>
        VY_NODISCARD bool hasAny()
		{
            VY_ASSERT(isValid());

			return registry().any_of<T...>(m_Handle);
		}


        /**
         * @brief Checks if the entity has any of the specified components.
         * 
         * @tparam T Type of component(s) to check.
         * 
         * @note This will assert if the VyEntity is valid.
         */
        template <typename... T>
        VY_NODISCARD bool hasAny() const
		{
            VY_ASSERT(isValid());

			return registry().any_of<T...>(m_Handle);
		}

        // ----------------------------------------------------------------------------------------

        /**
         * @brief Constructs a new component of type T with the passed in arguments,
         *        adds it to the VyEntity, and returns a reference to it.
         * 
         * @tparam T The new component.
         * @tparam Args Arguments to pass to the component.
         * 
         * @return A refrence to the new component.
         * 
         * @note This will assert if the component already exists.
         */
        template <typename T, typename... Args>
        T& add(Args&&... args)
        {
            VY_ASSERT(!has<T>(), "Cannot add component: VyEntity already has component");

            return registry().emplace<T>(m_Handle, std::forward<Args>(args)...);
        }


        /**
         * @brief Overload to add a script derived from VyEntityScript to the entity.
         * 
         * @tparam T    The new script.
         * @tparam Args Arguments to pass to the script.
         * 
         * @return A refrence to the new script.
         */
		template<IsScript T, typename... Args>
		T& add(Args&&... args)
		{
			VY_ASSERT(!has<T>(), "Cannot add script: VyEntity already has the script");

			auto& script = registry().emplace<T>(m_Handle, std::forward<Args>(args)...);
			addScript(&script);
			
            return script;
		}


        template<typename T, typename... Args>
        T& addOrReplace(Args&&... args)
        {
            T& component = registry().emplace_or_replace<T>(m_Handle, std::forward<Args>(args)...);

            return component;
        }


        // ----------------------------------------------------------------------------------------

        /**
         * @brief Acces to the component of type T.
         * 
         * @tparam T The requested component.
         * @return A refrence to the component.
         * 
         * @note This will assert if the component exists.
         */
        template <typename T>
        T& get() const
		{
            VY_ASSERT(has<T>(), "Cannot get component: VyEntity does not have component");

			return registry().get<T>(m_Handle);
		}


        /**
         * @brief Try to get a pointer to the component of type T.
         * 
         * @tparam T Type of component to retrieve.
         * @return nullptr if entity does not have the requested component type.
         * 
         * @note This will assert if the VyEntity is valid.
         */
        template <typename T>
        T* tryGet()
		{
            VY_ASSERT(isValid());

			return registry().try_get<T>(m_Handle);
		}


        /**
         * @brief Try to get a pointer to the component of type T.
         * 
         * @tparam T Type of component to retrieve.
         * @return nullptr if entity does not have the requested component type.
         * 
         * @note This will assert if the VyEntity is valid.
         */
        template <typename T>
        const T* tryGet() const
		{
            VY_ASSERT(isValid());

			return registry().try_get<T>(m_Handle);
		}

        // ----------------------------------------------------------------------------------------

        /**
         * @brief Removes a component of type T from the entity.
         * 
         * @tparam T The component to remove.
         * 
         * @note VyEntity MUST have the component and it MUST be an optional component.
         */
        template <typename T>
            requires OptionalComponent<T>
        void remove()
		{
            VY_STATIC_ASSERT((!std::is_same_v<T, IDComponent>), "Cannot remove component: Must not be an IDComponent");
            VY_ASSERT(has<T>(),                                 "Cannot remove component: VyEntity does not have the component");

			return registry().remove<T>(m_Handle);
		}


        /**
         * @brief Get the UUID of the entity
         * 
         * @return UUID of the entity
         */
        VyUUID getUUID() 
        {
            return get<IDComponent>().ID;
        }

        // ----------------------------------------------------------------------------------------
    private:
        void addScript(VyEntityScript* script);

    private:
		EntityHandle m_Handle{ kInvalidEntityHandle };
		VyScene*     m_Scene { nullptr };
    };
}