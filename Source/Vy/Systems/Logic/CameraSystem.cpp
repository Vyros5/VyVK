#include <Vy/Systems/Logic/CameraSystem.h>

#include <Vy/Math/Math.h>
// #include <Vy/Engine.h>

namespace Vy
{
    void VyCameraSystem::update(const VyFrameInfo& frameInfo, float aspectRatio) const
    {
        auto& registry = frameInfo.Scene->registry();

        if (VyEntity mainCameraEntity = frameInfo.Scene->mainCamera())
        {
            // Check if the entity has the required components.
            if (registry.valid(mainCameraEntity) && registry.all_of<CameraComponent, TransformComponent>(mainCameraEntity))
            {
                auto&       cameraComp = mainCameraEntity.get<CameraComponent>();
                const auto& transform  = mainCameraEntity.get<TransformComponent>();

                updateCamera(cameraComp, transform, aspectRatio);

                // Sync the frameInfo camera with the component camera
                // This ensures the renderer uses the updated camera matrices
                frameInfo.Camera = cameraComp.Camera;
            }
        }
    }


    void VyCameraSystem::updateCamera(CameraComponent& cameraComp, const TransformComponent& transform, float aspectRatio) const
    {
        // Update projection
        if (cameraComp.Camera.isPerspective())
        {
            cameraComp.Camera.setPerspective(aspectRatio);
        }
        else
        {
            cameraComp.Camera.setOrthographic();
        }

        // Update view
        cameraComp.Camera.setView(transform.Translation, transform.Rotation);

        // Update frustum
        cameraComp.Camera.updateFrustum();
    }
}




// namespace Vy
// {
// 	void VyCameraSystem::update(entt::registry& registry, float deltaTime)
// 	{
// 		auto mainCam = VyEngine::get().mainCamera();
// 		auto view    = registry.view<TransformComponent2, CameraComponent2>();

// 		for (auto&& [ entity, transformComp, cameraComp ] : view.each())
// 		{
// 			if (entity == mainCam)
// 			{
// 				calcCamera(cameraComp, transformComp);
// 			}
// 			else
// 			{
// 				VY_WARN_TAG("VyCameraSystem", "Failed to update main camera!");
// 			}
// 		}

// 		// auto view2 = registry.view<TransformComponent, CameraComponent>();

// 		// for (auto&& [ entity, transform, camera ] : view2.each())
// 		// {
// 		// 	calcViewMatrix(camera, transform);
// 		// 	calcPerspectiveProjection(camera);
// 		// }
// 	}

// // #if 0
// 	void VyCameraSystem::calcCamera(CameraComponent2& cameraComp, TransformComponent2 transformComp)
// 	{
// 		auto camera = cameraComp.Camera;

// 		camera.setView(transformComp.Position, transformComp.Rotation);

// 		if (camera.isPerspective()) 
// 		{
// 			auto aspect = VyEngine::get().renderer().aspectRatio();
// 			camera.setPerspective(aspect);

// 			// VY_INFO_TAG("VyCameraSystem", "Set perspective camera with aspect: {}", aspect);
// 		}
// 		else 
// 		{
// 			camera.setOrthographic();
// 		}
// 	}
// // #endif

// #if 1
//     void VyCameraSystem::calcViewMatrix(CameraComponent& camera, TransformComponent transform)
//     {
// 		// Calculate the view matrix based on the camera's transform
// 		// see: https://www.3dgep.com/understanding-the-view-matrix/

// 		// Vectors for the view coordinate system.
// 		const Vec3 w(glm::normalize(transform.forward()));				// Forward
// 		const Vec3 u(glm::normalize(glm::cross(w, Math::World::UP)));	// Right
// 		const Vec3 v(glm::cross(u, w));							     	// Up

// 		/**
// 		 * Column Major
// 		 * ------------------
// 		 * [col][row]
// 		 * 
// 		 * R = Right
// 		 * U = Up
// 		 * F = Forward
// 		 * P = Position
// 		 * ------------------
// 		 * Rx Ux Fx Px
// 		 * Ry Uy Uy Py
// 		 * Rz Uz Fz Pz
// 		 *  0  0  0  1
// 		 * ------------------
// 		 * 00 01 02 03 
// 		 * 10 11 12 13 
// 		 * 20 21 22 23 
// 		 * 30 31 32 33 
// 		 * ------------------
// 		 */

// 		Mat4 viewMatrix{ 1.0f };
// 		{
// 			// Right
// 			viewMatrix[0][0] =  u.x;
// 			viewMatrix[1][0] =  u.y;
// 			viewMatrix[2][0] =  u.z;

// 			// Up
// 			viewMatrix[0][1] =  v.x;
// 			viewMatrix[1][1] =  v.y;
// 			viewMatrix[2][1] =  v.z;

// 			// Forward
// 			viewMatrix[0][2] = -w.x;
// 			viewMatrix[1][2] = -w.y;
// 			viewMatrix[2][2] = -w.z;
			
// 			viewMatrix[3][0] = -glm::dot(u, transform.Position);
// 			viewMatrix[3][1] = -glm::dot(v, transform.Position);
// 			viewMatrix[3][2] =  glm::dot(w, transform.Position); 
// 		}

// 		camera.setView(viewMatrix);

// 		Mat4 inverseViewMat{ 1.0f };
// 		{
// 			// Right
// 			inverseViewMat[0][0] = u.x;
// 			inverseViewMat[0][1] = u.y;
// 			inverseViewMat[0][2] = u.z;

// 			// Up
// 			inverseViewMat[1][0] = v.x;
// 			inverseViewMat[1][1] = v.y;
// 			inverseViewMat[1][2] = v.z;
			
// 			// Forward
// 			inverseViewMat[2][0] = w.x;
// 			inverseViewMat[2][1] = w.y;
// 			inverseViewMat[2][2] = w.z;
			
// 			inverseViewMat[3][0] = transform.Position.x;
// 			inverseViewMat[3][1] = transform.Position.y;
// 			inverseViewMat[3][2] = transform.Position.z;
// 		}

// 		camera.setInverseView(inverseViewMat);
//     }


//     void VyCameraSystem::calcPerspectiveProjection(CameraComponent& camera)
//     {
// 		VY_ASSERT(abs(camera.aspect() - std::numeric_limits<float>::epsilon()) > 0.0f);

// 		auto farClip  = camera.farClip();
// 		auto nearClip = camera.nearClip();

// 		// Projection matrix for right handed system with depth range [0, 1].
// 		// Note: Value `[1][1]` is negated because Vulkan uses a flipped y-axis.
		
// 		// https://www.vincentparizet.com/blog/posts/vulkan_perspective_matrix/

// 		const float tanHalfFovy = tan(camera.fieldOfView() / 2.0f);

// 		Mat4 perspective{ 0.0f };
// 		{
// 			perspective[0][0] =  1.0f / (camera.aspect() * tanHalfFovy);
// 			perspective[1][1] = -1.0f / (tanHalfFovy);
// 			perspective[2][2] =  farClip / (nearClip - farClip);
// 			perspective[2][3] = -1.0f;
// 			perspective[3][2] = -(farClip * nearClip) / (farClip - nearClip);
// 		}

// 		// Mat4 perspective = glm::perspective(camera.fieldOfView(), camera.aspect(), nearClip, farClip);
// 		// perspective[1][1] *= -1.0f; // convert to Vulkan coordinates (from OpenGL)

// 		camera.setProjection(perspective);
//     }
// #endif












// 	// Mat4 perspective(float vertical_fov, float aspect_ratio, float nearClip, float farClip, Mat4 *inverse)
// 	// {
// 	// 	float fov_rad = vertical_fov * 2.0f * M_PI / 360.0f;
// 	// 	float focal_length = 1.0f / std::tan(fov_rad / 2.0f);

// 	// 	float x  =  focal_length / aspect_ratio;
// 	// 	float y  = -focal_length;
// 	// 	float A  = nearClip / (farClip - nearClip);
// 	// 	float B  = farClip * A;

// 	// 	Mat4 projection({
// 	// 		x,    0.0f,  0.0f, 0.0f,
// 	// 		0.0f,    y,  0.0f, 0.0f,
// 	// 		0.0f, 0.0f,     A,    B,
// 	// 		0.0f, 0.0f, -1.0f, 0.0f,
// 	// 	});

// 	// 	if (inverse)
// 	// 	{
// 	// 		*inverse = Mat4({
// 	// 			1/x,  0.0f, 0.0f,  0.0f,
// 	// 			0.0f,  1/y, 0.0f,  0.0f,
// 	// 			0.0f, 0.0f, 0.0f, -1.0f,
// 	// 			0.0f, 0.0f,  1/B,   A/B,
// 	// 		});
// 	// 	}

// 	// 	return projection;
// 	// }






// 	void CameraSystem::update(entt::registry& registry, float deltaTime)
// 	{
// 		auto view = registry.view<CameraComponent3, TransformComponent>();

// 		// Iterate all cameras (CameraComponent + TransformComponent)
// 		for (auto&& [ entity, camera, transform ] : view.each())
// 		{
// 			// Compute view matrix from transform
// 			Mat4 viewMatrix = computeViewMatrix(transform.Position, transform.Rotation);

// 			// Compute projection matrix based on camera type
// 			Mat4 projMatrix;

// 			if (camera.Type == CameraComponent3::EType::Perspective) 
// 			{
// 				projMatrix = computePerspective(
// 					camera.FovYDegrees,
// 					camera.AspectRatio,
// 					camera.NearPlane,
// 					camera.FarPlane
// 				);
// 			} 
// 			else 
// 			{
// 				projMatrix = computeOrthographic(
// 					camera.OrthoWidth,
// 					camera.AspectRatio,
// 					camera.NearPlane,
// 					camera.FarPlane
// 				);
// 			}

// 			// Combine view-projection
// 			Mat4 viewProj = projMatrix * viewMatrix;

// 			// Create/update RenderCamera component
// 			RenderCameraComponent renderCam;
// 			{
// 				renderCam.View           = viewMatrix;
// 				renderCam.Projection     = projMatrix;
// 				renderCam.ViewProjection = viewProj;
// 			}

// 			// Extract frustum planes for culling
// 			extractFrustumPlanes(viewProj, renderCam.FrustumPlanes);

// 			// Store in RenderCamera component
// 			registry.emplace_or_replace<RenderCameraComponent>(entity, renderCam);
// 		}
// 	}

// 	// ============================================================================
// 	// Helper Functions
// 	// ============================================================================

// 	Mat4 CameraSystem::computeViewMatrix(const Vec3& position, const Quat& rotation) 
// 	{
// 		// Camera view matrix: inverse of camera's world transform
// 		// World transform: T * R
// 		// View transform: R^-1 * T^-1

// 		// Compute rotation matrix from quaternion
// 		Mat4 rotationMatrix = glm::mat4_cast(rotation);

// 		// Compute translation
// 		Mat4 translationMatrix = glm::translate(Mat4(1.0f), position);

// 		// View matrix = inverse(translation * rotation)
// 		// = inverse(rotation) * inverse(translation)
// 		Mat4 worldToCamera = glm::inverse(translationMatrix * rotationMatrix);

// 		return worldToCamera;
// 	}


// 	Mat4 CameraSystem::computePerspective(float fovYDegrees, float aspect, float nearPlane, float farPlane) 
// 	{
// 		// Vulkan-style perspective projection:
// 		// - Y-axis points down in NDC (negative viewport height flips it)
// 		// - Z-axis range: [0, 1] (not [-1, 1] like OpenGL)
// 		// - Right-handed coordinate system

// 		// GLM's perspectiveFov expects radians
// 		float fovYRadians = glm::radians(fovYDegrees);

// 		// GLM provides glm::perspectiveLH_ZO (left-handed, Z:0-1) and glm::perspectiveRH_ZO (right-handed, Z:0-1)
// 		// We want right-handed with Z:[0,1] for Vulkan
// 		Mat4 proj = glm::perspectiveRH_ZO(fovYRadians, aspect, nearPlane, farPlane);

// 		// Vulkan's Y-axis is flipped compared to OpenGL
// 		// Flip Y-axis in projection to match Vulkan's coordinate system
// 		proj[1][1] *= -1.0f;

// 		return proj;
// 	}


// 	Mat4 CameraSystem::computeOrthographic(float width, float aspect, float nearPlane, float farPlane) 
// 	{
// 		// Orthographic projection for Vulkan
// 		float height = width / aspect;

// 		float left   = -width  * 0.5f;
// 		float right  =  width  * 0.5f;
// 		float bottom = -height * 0.5f;
// 		float top    =  height * 0.5f;

// 		// GLM provides orthoRH_ZO (right-handed, Z:0-1) for Vulkan
// 		Mat4 proj = glm::orthoRH_ZO(left, right, bottom, top, nearPlane, farPlane);

// 		// Flip Y-axis for Vulkan
// 		proj[1][1] *= -1.0f;

// 		return proj;
// 	}


// 	void CameraSystem::extractFrustumPlanes(const Mat4& viewProj, Vec4 planes[6]) 
// 	{
// 		// Extract frustum planes from view-projection matrix
// 		// Plane equation: Ax + By + Cz + D = 0
// 		// Stored as vec4(A, B, C, D)

// 		// Left plane
// 		planes[0] = Vec4(
// 			viewProj[0][3] + viewProj[0][0],
// 			viewProj[1][3] + viewProj[1][0],
// 			viewProj[2][3] + viewProj[2][0],
// 			viewProj[3][3] + viewProj[3][0]
// 		);

// 		// Right plane
// 		planes[1] = Vec4(
// 			viewProj[0][3] - viewProj[0][0],
// 			viewProj[1][3] - viewProj[1][0],
// 			viewProj[2][3] - viewProj[2][0],
// 			viewProj[3][3] - viewProj[3][0]
// 		);

// 		// Bottom plane
// 		planes[2] = Vec4(
// 			viewProj[0][3] + viewProj[0][1],
// 			viewProj[1][3] + viewProj[1][1],
// 			viewProj[2][3] + viewProj[2][1],
// 			viewProj[3][3] + viewProj[3][1]
// 		);

// 		// Top plane
// 		planes[3] = Vec4(
// 			viewProj[0][3] - viewProj[0][1],
// 			viewProj[1][3] - viewProj[1][1],
// 			viewProj[2][3] - viewProj[2][1],
// 			viewProj[3][3] - viewProj[3][1]
// 		);

// 		// Near plane
// 		planes[4] = Vec4(
// 			viewProj[0][2],
// 			viewProj[1][2],
// 			viewProj[2][2],
// 			viewProj[3][2]
// 		);

// 		// Far plane
// 		planes[5] = Vec4(
// 			viewProj[0][3] - viewProj[0][2],
// 			viewProj[1][3] - viewProj[1][2],
// 			viewProj[2][3] - viewProj[2][2],
// 			viewProj[3][3] - viewProj[3][2]
// 		);

// 		// Normalize planes
// 		for (int i = 0; i < 6; i++) 
// 		{
// 			float length = glm::length(Vec3(planes[i]));
			
// 			if (length > 0.0f) 
// 			{
// 				planes[i] /= length;
// 			}
// 		}
// 	}
// }