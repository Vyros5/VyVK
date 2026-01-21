#pragma once

#include <VyLib/VyLib.h>

#include <entt/entity/entity.hpp>

namespace Vy
{
	using EntityHandle  = entt::entity;
	using EntityVersion = entt::entt_traits<EntityHandle>::version_type;

	constexpr EntityHandle kInvalidEntityHandle = entt::null;
}