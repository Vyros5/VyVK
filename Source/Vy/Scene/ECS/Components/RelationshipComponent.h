#pragma once

#include <Vy/Scene/ECS/Entity.h>

namespace Vy
{
	/// @brief Stores the parent entity
	/// @note Use VyEntity::setParent to set the parent of an entity 

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

	// /// @brief Stores the next and previous sibling of the entity (for linked list)
	// /// @note Use VyEntity::addChild to add a child to an entity (siblings are automatically updated)
	// struct SiblingsComponent
	// {
	// 	VyEntity Next{};
	// 	VyEntity Prev{};
	// };

	// /// @brief Stores the children entities as the first and last child of a linked list
	// /// @note Use VyEntity::addChild to add a child to an entity
	// struct ChildrenComponent
	// {
	// 	struct Iterator
	// 	{
	// 		VyEntity Current{};

	// 		VyEntity operator*() const 
    //         { 
    //             return Current; 
    //         }
			
    //         Iterator& operator++() 
    //         { 
    //             Current = Current.get<SiblingsComponent>().Next; 
    //             return *this; 
    //         }
			
    //         bool operator==(const Iterator& other) const 
    //         { 
    //             return Current == other.Current; 
    //         }
	// 	};

	// 	struct ReverseIterator
	// 	{
	// 		VyEntity Current{};

	// 		VyEntity operator*() const 
    //         { 
    //             return Current; 
    //         }
			
    //         ReverseIterator& operator++() 
    //         { 
    //             Current = Current.get<SiblingsComponent>().Prev; 
    //             return *this; 
    //         }
			
    //         bool operator==(const ReverseIterator& other) const 
    //         { 
    //             return Current == other.Current; 
    //         };
	// 	};

	// 	size_t   Count = 0;
	// 	VyEntity First{};
	// 	VyEntity Last{};

	// 	ChildrenComponent::Iterator        begin()  const { return { First      }; }
	// 	ChildrenComponent::Iterator        end()    const { return { VyEntity{} }; }
	// 	ChildrenComponent::ReverseIterator rbegin() const { return { Last       }; }
	// 	ChildrenComponent::ReverseIterator rend()   const { return { VyEntity{} }; }
	// };
}