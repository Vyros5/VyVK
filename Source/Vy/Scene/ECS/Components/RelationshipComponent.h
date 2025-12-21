#pragma once

#include <Vy/Scene/ECS/Entity.h>

namespace Vy
{
	/**
	 * ParentComponent - Reference to parent entity
	 *
	 * When present, this entity's transform is relative to its parent.
	 * HierarchySystem computes world-space transform by combining with parent's LocalToWorld.
	 */
	struct ParentComponent
	{
		VyEntity Parent{};
		// EntityHandle Parent = kInvalidEntityHandle;

		constexpr ParentComponent() = default;

    	explicit  ParentComponent(VyEntity p) : Parent(p) {}
	};


	/**
	 * ChildrenComponent - List of child entities
	 *
	 * Automatically maintained by Scene::setParent() / Scene::removeParent().
	 * Used for efficient hierarchy traversal and cascade operations (e.g., delete with children).
	 */
	struct ChildrenComponent 
	{
		TVector<VyEntity> Children;

		constexpr ChildrenComponent() = default;
	};
}