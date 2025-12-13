// #include <Vy/Systems/Logic/TransformSystem.h>

// #include <Vy/Scene/Scene.h>
// #include <Vy/Scene/ECS/Entity.h>
// #include <Vy/Scene/ECS/Components.h>

// namespace Vy
// {
// 	void VyTransformSystem::update(entt::registry& registry, float deltaTime)
// 	{
// 		// // Process all entities with TransformComponent
		
// 		// Note: This lags 1 frame per parent behind the actual local transform (but this is fine for now).

// 		auto view = registry.view<TransformComponent>();

// 		for (auto&& [ entity, transform ] : view.each())
// 		{
// 			// Compute local-to-world matrix: T * R * S
// 			Mat4 translation = glm::translate(Mat4(1.0f), transform.Position);
// 			Mat4 rotation    = glm::mat4_cast(transform.Rotation);
// 			Mat4 scaleMatrix = glm::scale(Mat4(1.0f), transform.Scale);

// 			Mat4 localToWorld = translation * rotation * scaleMatrix;

// 			// Compute normal matrix (inverse transpose of upper-left 3x3)
// 			// For non-uniform scaling, normals need to be transformed differently
// 			Mat3 normalMat    = glm::transpose(glm::inverse(Mat3(localToWorld)));
// 			Mat4 normalMatrix = Mat4(normalMat);

// 			// Store result in LocalToWorld component (separate from input!)
// 			registry.emplace_or_replace<LocalToWorldComponent>(entity, localToWorld, normalMatrix);
// 		}
// 	}



// 	// void VyTransformSystem::update(entt::registry& registry, float deltaTime)
// 	// {
// 	// 	// // Process all entities with TransformComponent
		
// 	// 	// Note: This lags 1 frame per parent behind the actual local transform (but this is fine for now).

// 	// 	auto view = registry.view<TransformComponent, GlobalTransformComponent, ParentComponent>();

// 	// 	for (auto&& [ entity, transform, globalTransform, parent ] : view.each())
// 	// 	{
// 	// 		if (!parent.Parent || !parent.Parent.has<GlobalTransformComponent>())
// 	// 		{
// 	// 			globalTransform.Position = transform.Position;
// 	// 			globalTransform.Rotation = transform.Rotation;
// 	// 			globalTransform.Scale    = transform.Scale;

// 	// 			continue;
// 	// 		}

// 	// 		auto& parentGlobal = parent.Parent.get<GlobalTransformComponent>();
            
// 	// 		globalTransform.Position = parentGlobal.Position + transform.Position;
// 	// 		globalTransform.Rotation = parentGlobal.Rotation * transform.Rotation;
// 	// 		globalTransform.Scale    = parentGlobal.Scale    * transform.Scale;
// 	// 	}
// 	// }
// }