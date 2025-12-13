#pragma once

// #include <Vy/VyCore.h>

// Required
#include <Vy/Scene/ECS/Components/IDComponent.h>
#include <Vy/Scene/ECS/Components/TagComponent.h>
#include <Vy/Scene/ECS/Components/TransformComponent.h>
#include <Vy/Scene/ECS/Components/RelationshipComponent.h>

// Optional
#include <Vy/Scene/ECS/Components/CameraComponent.h>
#include <Vy/Scene/ECS/Components/ModelComponent.h>
// #include <Vy/Scene/ECS/Components/MeshComponent.h>
#include <Vy/Scene/ECS/Components/LightComponent.h>
#include <Vy/Scene/ECS/Components/MaterialComponent.h>
#include <Vy/Scene/ECS/Components/ColorComponent.h>

#include <Vy/Scene/ECS/Components/PostProcessingComponent.h>

namespace Vy
{
	template<typename T, typename... Ts>
	concept IsAnyOf = (std::is_same_v<T, Ts> || ...);

	/**
	 * @brief Defines all required components for an entity (Each entity MUST have these components)
	 */
	template<typename T>
	concept RequiredComponent = IsAnyOf<
        T, 
		IDComponent,
        TagComponent, 
        TransformComponent,
		// GlobalTransformComponent,
		ParentComponent,
		// SiblingsComponent,
		ChildrenComponent
    >;


	/**
	 * @brief Defines all optional components for an entity (Each entity MAY have these components).
	 */
	template<typename T>
	concept OptionalComponent = !RequiredComponent<T>;
}