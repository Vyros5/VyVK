#pragma once

#include <Vy/Scene/ECS/Entity.h>
#include <Vy/Scene/ECS/Components.h>

namespace Vy
{
	class VyEntityScript
	{
	public:

		/**
		 * @brief Constructor
		 * 
		 * @note When overriding, dont't use member functions (m_Entity is not initialized yet).
		 */
		VyEntityScript() = default;
        
		virtual ~VyEntityScript() = default;

		/**
		 * @brief Called once at the first frame just before update.
		 */
		virtual void begin() 
        {
        }

		/**
		 * @brief Called every frame with the delta of the last two frames in seconds.
		 * 
		 * @param deltaTime Time elapsed since the last update.
		 */
		virtual void update(float deltaTime) 
        {
        }
		
        /**
         * @brief Called once at the end of the last frame.
         */
		virtual void end() 
        {
        }

		/**
		 * @brief Returns true if the entity has all components of type T...
		 */
		template<typename... T>
		bool has() const
		{
			return m_Entity.has<T...>();
		}

		/**
		 * @brief Returns a reference of the component of type T.
		 * 
		 * @note Component of type T must exist!
		 */
		template<typename T>
		T& get() const
		{
			return m_Entity.get<T>();
		}


		/**
		 * @brief Adds a component of type T to the entity and returns a reference to it.
		 */
		template<typename T, typename... Args>
		T& add(Args&&... args)
		{
			return m_Entity.add<T>(std::forward<Args>(args)...);
		}

		/**
		 * @brief Returns the entity.
		 */
		Vy::VyEntity entity() const { return m_Entity; }

	private:
		Vy::VyEntity m_Entity;

		friend class Vy::VyEntity;
	};
}