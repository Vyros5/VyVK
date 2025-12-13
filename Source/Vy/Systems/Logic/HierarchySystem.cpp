// #include <Vy/Systems/Logic/HierarchySystem.h>

// #include <Vy/Math/Math.h>
// #include <Vy/Engine.h>

// namespace Vy
// {
//     void HierarchySystem::update(entt::registry& registry, float /*deltaTime*/) 
//     {
//         // Process all entities with a parent
//         // We iterate through children and update their world transforms based on their parent's LocalToWorld
//         auto view = registry.view<TransformComponent, ParentComponent>();

//         for (auto&& [ entity, transform, parent ] : view.each()) 
//         {
//             // Ensure parent is valid and has a world transform.
//             if (!registry.valid(parent.Parent)) 
//             {
//                 // Parent is invalid, skip this child.
//                 continue;
//             }

//             const auto* parentLocalToWorld = registry.try_get<LocalToWorldComponent>(parent.Parent);

//             if (!parentLocalToWorld) 
//             {
//                 // Parent has no world transform yet, skip.
//                 continue;  
//             }

//             // Get child's local transform.
//             const auto& childTransform = view.get<TransformComponent>(entity);

//             // Compute child's local matrix.
//             Mat4 childLocalMatrix = computeLocalMatrix(childTransform);

//             // Compute child's world matrix: Parent world * Child local.
//             Mat4 childWorldMatrix = parentLocalToWorld->Matrix * childLocalMatrix;

//             // Compute normal matrix for lighting (inverse transpose of upper-left 3x3).
//             Mat3 normalMatrix = glm::transpose(glm::inverse(Mat3(childWorldMatrix)));
//             Mat4 normalMatrix4(normalMatrix);  // Convert to mat4 for storage.

//             // Update or create LocalToWorld for child.
//             registry.emplace_or_replace<LocalToWorldComponent>(entity, childWorldMatrix, normalMatrix4);

//             // If this child has children, recursively update them
//             // This handles deep hierarchies (grandchildren, etc.).
//             const auto* childrenComp = registry.try_get<ChildrenComponent>(entity);

//             if (childrenComp) 
//             {
//                 for (EntityHandle grandchild : childrenComp->Children) 
//                 {
//                     updateChildRecursive(registry, grandchild, childWorldMatrix);
//                 }
//             }
//         }
//     }


//     void HierarchySystem::updateChildRecursive(entt::registry& registry, EntityHandle entity, const Mat4& parentWorldMatrix) 
//     {
//         // Get child's local transform.
//         const auto* childTransform = registry.try_get<TransformComponent>(entity);

//         if (!childTransform) 
//         {
//             // No transform, skip.
//             return;  
//         }

//         // Compute child's local matrix
//         Mat4 childLocalMatrix = computeLocalMatrix(*childTransform);

//         // Compute child's world matrix
//         Mat4 childWorldMatrix = parentWorldMatrix * childLocalMatrix;

//         // Compute normal matrix
//         Mat3 normalMatrix = glm::transpose(glm::inverse(Mat3(childWorldMatrix)));
//         Mat4 normalMatrix4(normalMatrix);

//         // Update LocalToWorld
//         registry.emplace_or_replace<LocalToWorldComponent>(entity, childWorldMatrix, normalMatrix4);

//         // Recurse to grandchildren
//         const auto* childrenComp = registry.try_get<ChildrenComponent>(entity);
        
//         if (childrenComp) 
//         {
//             for (EntityHandle grandchild : childrenComp->Children) 
//             {
//                 updateChildRecursive(registry, grandchild, childWorldMatrix);
//             }
//         }
//     }


//     Mat4 HierarchySystem::computeLocalMatrix(const TransformComponent& transform) 
//     {
//         // Compute T * R * S
//         Mat4 translation = glm::translate(Mat4(1.0f), transform.Position);
//         Mat4 rotation    = glm::mat4_cast(transform.Rotation);
//         Mat4 scale       = glm::scale(Mat4(1.0f), transform.Scale);

//         return translation * rotation * scale;
//     }
// }