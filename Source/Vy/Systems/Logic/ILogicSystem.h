#pragma once

#include <Vy/Scene/ECS/Entity.h>

#include <concepts>

namespace Vy
{
	class ILogicSystem
	{
	public:
		ILogicSystem()          = default;
		virtual ~ILogicSystem() = default;
		
        virtual void update(entt::registry& registry, float deltaTime) {}
	};
	

	template <typename T>
	concept LogicSystemDerived = std::derived_from<T, ILogicSystem>;
}