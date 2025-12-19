#pragma once

// #include <Vy/Systems/Logic/ILogicSystem.h>

#include <Vy/Scene/Scene.h>
#include <Vy/Scene/ECS/Components.h>
#include <Vy/GFX/FrameInfo.h>

namespace Vy
{
	class VyCameraSystem 
	{
	public:
		VyCameraSystem()  = default;
		~VyCameraSystem() = default;

		void update(const VyFrameInfo& frameInfo, float aspectRatio) const;

	private:
		void updateCamera(CameraComponent& cameraComp, const TransformComponent& transform, float aspectRatio) const;
	};
}
// namespace Vy
// {
// 	class VyCameraSystem : public ILogicSystem
// 	{
// 	public:
// 		VyCameraSystem()  = default;
// 		~VyCameraSystem() = default;

// 		void update(entt::registry& registry, float deltaTime) override;

// 	private:
// 		void calcViewMatrix(CameraComponent& camera, TransformComponent transform);
// 		void calcPerspectiveProjection(CameraComponent& camera);
		
// 		void calcCamera(CameraComponent2& camera, TransformComponent2 transform);
// 	};



// class CameraSystem : public ILogicSystem 
// {
// public:
//     CameraSystem() = default;

//     ~CameraSystem() override = default;

//     void update(entt::registry& registry, float deltaTime) override;

// private:
//     // Helper: Compute view matrix from transform
//     static Mat4 computeViewMatrix(const Vec3& position, const Quat& rotation);

//     // Helper: Compute perspective projection (Vulkan-style: Y-down, Z:0-1)
//     static Mat4 computePerspective(float fovYDegrees, float aspect, float nearPlane, float farPlane);

//     // Helper: Compute orthographic projection (Vulkan-style)
//     static Mat4 computeOrthographic(float width, float aspect, float nearPlane, float farPlane);

//     // Helper: Extract frustum planes from view-projection matrix
//     static void extractFrustumPlanes(const Mat4& viewProj, Vec4 planes[6]);
// };
// }