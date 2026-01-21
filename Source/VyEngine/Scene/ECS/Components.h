#pragma once

// #include <VyEngine/VyCore.h>

// Required
#include <VyEngine/Scene/ECS/Components/IDComponent.h>
#include <VyEngine/Scene/ECS/Components/TagComponent.h>
#include <VyEngine/Scene/ECS/Components/TransformComponent.h>
#include <VyEngine/Scene/ECS/Components/RelationshipComponent.h>

// Optional
#include <VyEngine/Scene/ECS/Components/CameraComponent.h>
#include <VyEngine/Scene/ECS/Components/ModelComponent.h>
#include <VyEngine/Scene/ECS/Components/MaterialComponent.h>
#include <VyEngine/Scene/ECS/Components/LightComponent.h>
#include <VyEngine/Scene/ECS/Components/ColorComponent.h>
#include <VyEngine/Scene/ECS/Components/AnimationComponent.h>

namespace Vy
{
	template<typename T, typename... Ts>
	concept IsAnyOf = (std::is_same_v<T, Ts> || ...);

	/**
	 * @brief Defines all required components for an entity (Each entity MUST have these components)
	 */
	template<typename T>
	concept RequiredComponentConcept = IsAnyOf<
        T, 
		IDComponent,
        TagComponent, 
        TransformComponent,
		ParentComponent,
		ChildrenComponent
    >;


	/**
	 * @brief Defines all optional components for an entity (Each entity MAY have these components).
	 */
	template<typename T>
	concept OptionalComponentConcept = !RequiredComponentConcept<T>;
}