#pragma once

#include <Vy/Scene/Camera.h>
#include <Vy/Scene/ECS/EntityHandle.h>

namespace Vy
{
	struct CameraComponent
	{
		VyCamera Camera{};
		bool     IsMainCamera{ true };

		const Mat4& view()        const { return Camera.view();        }
		const Mat4& inverseView() const { return Camera.inverseView(); }
		const Mat4& projection()  const { return Camera.projection();  }
		const Vec3  position()    const { return Camera.position();    }

		CameraComponent(const CameraComponent&) = default;

		CameraComponent() : 
			IsMainCamera{ true }
		{ 
			VY_INFO_TAG("CameraComponent", "[ Main Camera Initialized ]");
		}
		
		CameraComponent(const VyCamera& camera) : 
			Camera{ camera }
		{
		}
	};


// 	struct CameraComponent
// 	{
// 		enum ProjectType 
// 		{
// 			Orthographic = 0,
// 			Perspective = 1
// 		};

// 		EngineCamera Camera{};
// 		/* *
// * Projection type (Perspective/Orthographic)
// */
// 		void setProjectionType(ProjectType pt) { ProjectionType = pt; setProjection(); };
// 		ProjectType getProjectionType() { return ProjectionType; };
// 		/* *
// * Vertical Field of view (degrees)
// */
// 		void setFieldOfView(float fov) { FOV = fov; Camera.setPerspectiveProjection(glm::radians(FOV), AspectRatio, NearClip, FarClip); }
// 		float fieldOfView() { return FOV; }
// 		/* *
// * Near clip distance (meters)
// */
// 		void setNearClip(float zNear) { NearClip = zNear; setProjection(); }
// 		float nearClip() { return NearClip; }
// 		/* *
// * Far clip distance (meters)
// */
// 		void setFarClip(float zFar) { FarClip = zFar; setProjection();}
// 		float farClip() { return FarClip; }
// 		/* *
// * Aspect ratio (width/height)
// */
// 		float AspectRatio = 1;//16 / 9;
// 		/* *
// * Set projection with the given settings
// */	
// 		operator EngineCamera&() 
// 		{
// 			return Camera;
// 		}

// 		const Mat4& view()               const { return Camera.view();              }
// 		const Mat4& inverseView()        const { return Camera.inverseView();       }
// 		const Mat4& projection()         const { return Camera.projection();        }
// 		const Mat4& inverseProjection()  const { return Camera.inverseProjection(); }
		
// 		CameraComponent() = default;

// 		CameraComponent(const CameraComponent&) = default;

// 		void setProjection() 
// 		{
// 			if (ProjectionType == ProjectType::Perspective)
// 			{
// 				Camera.setPerspectiveProjection(glm::radians(FOV), AspectRatio, NearClip, FarClip);
// 			}
// 			else  
// 			{
// 				Camera.setOrthographicProjection(-AspectRatio, AspectRatio, -1, 1, NearClip, FarClip);
// 			}
// 		}
// 		// std::vector<PostProcessingMaterialInstance> postProcessMaterials{};
// 	private:
// 		float FOV      = 70.0f;
// 		float NearClip = 0.01f;
// 		float FarClip  = 100.0f;
// 		ProjectType ProjectionType = ProjectType::Perspective;
		
// 		friend class LevelPropertiesPanel;
// 	};


	// /**
	//  * @brief Camera system with consistent RHS coordinate convention
	//  *
	//  * COORDINATE SYSTEM:
	//  * ==================
	//  * World Space:      Right-Handed, Y-up, -Z forward (standard 3D convention)
	//  * Camera/View:      Right-Handed, Y-up, -Z forward (MATCHES world space)
	//  * Clip/NDC:         Vulkan convention (Y-down, Z ∈ [0,1])
	//  *
	//  * TRANSFORMATION FLOW:
	//  * ====================
	//  *   World (RHS)  →  [View]  →  Camera (RHS)  →  [Projection*]  →  Clip (Vulkan NDC)
	//  *       ↓                          ↓                                      ↓
	//  *    -Z fwd                     -Z fwd                           Y-down & +Z forward
	//  *
	//  * * Projection matrix includes Y-flip: projection[1][1] *= -1.0f
	//  *
	//  * CAMERA AXES (match world):
	//  * ==========================
	//  * - Right:   +X (world and camera)
	//  * - Up:      +Y (world and camera)
	//  * - Forward: -Z (camera looks down -Z)
	//  */
	// struct CameraComponent5
	// {
	// 	Mat4 ProjectionMatrix    = Mat4 { 1.0f };
	// 	Mat4 ViewMatrix          = Mat4 { 1.0f };
	// 	Mat4 InvViewMatrix       = Mat4 { 1.0f };
	// 	Mat4 InvProjectionMatrix = Mat4 { 1.0f };

	// 	float LeftPlane          = 0.f;
	// 	float RightPlane         = 0.f;
	// 	float TopPlane           = 0.f;
	// 	float BottomPlane        = 0.f;
	// 	float NearPlane          = 0.f;
	// 	float FarPlane           = 0.f;
	// 	float AspectRatio        = 0.f;
	// 	float FovY               = 0.f;

	// 	bool IsPerspective       = false;
	// 	bool IsMainCamera        = false;

    //     const Mat4& view()              const noexcept { return ViewMatrix;       }
    //     const Mat4& projection()        const noexcept { return ProjectionMatrix; }
	// 	const Mat4& inverseView()       const noexcept { return InvViewMatrix;    }
	// 	const Mat4& inverseProjection() const noexcept { return InvProjectionMatrix; }
	// 	Mat4        inverseViewMat()                   { return glm::inverse(ViewMatrix); }

	// 	/**
	// 	 * @brief Set camera projection as orthographic
	// 	 * @param left plane of the projection volume
	// 	 * @param right plane of the projection volume
	// 	 * @param top top plane of the projection volume
	// 	 * @param bottom plane of the projection volume
	// 	 * @param n near plane
	// 	 * @param f far plane
	// 	 */
	// 	void setOrthographicProjection(
	// 		float left,
	// 		float right,
	// 		float top,
	// 		float bottom,
	// 		float n,
	// 		float f)
	// 	{
	// 		LeftPlane        = left;
	// 		RightPlane       = right;
	// 		TopPlane         = top;
	// 		BottomPlane      = bottom;
	// 		NearPlane        = n;
	// 		FarPlane         = f;
	// 		IsPerspective    = false;

	// 		ProjectionMatrix = Mat4 { 1.0f };
	// 		ProjectionMatrix = glm::orthoRH_ZO(left, right, bottom, top, n, f);

	// 		ProjectionMatrix[1][1] *= -1; // Flip Y

	// 		InvProjectionMatrix = glm::inverse(ProjectionMatrix);
	// 	};

	// 	/**
	// 	 * @brief Set camera's projection as perspective
	// 	 * @param fovy frustum angle
	// 	 * @param aspect ratio
	// 	 * @param n near plane
	// 	 * @param f far plane
	// 	 */
	// 	void setPerspectiveProjection(float fovy, float aspect, float n, float f)
	// 	{
	// 		VY_ASSERT(glm::abs(aspect - std::numeric_limits<float>::epsilon()) > 0.0f);

	// 		NearPlane        = n;
	// 		FarPlane         = f;
	// 		AspectRatio      = aspect;
	// 		FovY             = fovy;
	// 		IsPerspective    = true;
			
	// 		ProjectionMatrix = glm::perspectiveRH_ZO(fovy, aspect, n, f);

	// 		ProjectionMatrix[1][1] *= -1; // Flip Y

	// 		InvProjectionMatrix = glm::inverse(ProjectionMatrix);
	// 	};

	// 	void setAspectRatio(float aspect)
	// 	{
	// 		if (IsPerspective)
	// 		{
	// 			setPerspectiveProjection(FovY, aspect, NearPlane, FarPlane);
	// 		}
	// 	}

	// 	/**
	// 	 * @brief Set view as per camera's position and rotation
	// 	 * @param position position of the camera
	// 	 * @param total rotation to be applied on the camera
	// 	 */
	// 	void setView(Vec3 position, Quat rotation)
	// 	{
	// 		Mat3 rotationMat = glm::mat3_cast(rotation);
	// 		Vec3 right       = rotationMat[0];
	// 		Vec3 up          = rotationMat[1];
	// 		Vec3 forward     = rotationMat[2];

	// 		updateViewMatrix(position, right, up, forward);
	// 	}

	// 	/**
	// 	 * @brief update view and inverseview matrices for given position and camera's bases vectors
	// 	 * @param position of the camera
	// 	 * @param Camera's right direction
	// 	 * @param Camera's up direction
	// 	 * @param Camera's forward direction
	// 	 */
	// 	void updateViewMatrix(Vec3 position, Vec3 right, Vec3 up, Vec3 fwd)
	// 	{
	// 		// Rotation part
	// 		ViewMatrix    = Mat4(1.0f);
	// 		ViewMatrix[0] = Vec4(right.x, up.x, fwd.x, 0.0f);
	// 		ViewMatrix[1] = Vec4(right.y, up.y, fwd.y, 0.0f);
	// 		ViewMatrix[2] = Vec4(right.z, up.z, fwd.z, 0.0f);

	// 		// Translation part
	// 		ViewMatrix[3] = Vec4(
	// 			-glm::dot(right, position),
	// 			-glm::dot(up,    position),
	// 			-glm::dot(fwd,   position),
	// 			1.0f
	// 		);

	// 		// Inverse: just transpose rotation and use original position
	// 		InvViewMatrix    = Mat4(1.0f);
	// 		InvViewMatrix[0] = Vec4(right.x, right.y, right.z, 0.0f);
	// 		InvViewMatrix[1] = Vec4(   up.x,    up.y,    up.z, 0.0f);
	// 		InvViewMatrix[2] = Vec4(  fwd.x,   fwd.y,   fwd.z, 0.0f);
	// 		InvViewMatrix[3] = Vec4(position, 1.0f);
	// 	}
	// };
	




	// struct CameraComponent2
	// {
	// 	VyCamera Camera{};
	// 	bool     IsMainCamera = true;

	// 	const Mat4& view()        const { return Camera.view();        }
	// 	const Mat4& inverseView() const { return Camera.inverseView(); }
	// 	const Mat4& projection()  const { return Camera.projection();  }

	// 	CameraComponent2() : IsMainCamera(true) { VY_INFO_TAG("CameraComponent2", "[ Main Camera Initialized "); }
	// 	CameraComponent2(const VyCamera& camera) : Camera(camera) {}
	// 	CameraComponent2(const CameraComponent2&) = default;
	// };


	// struct CameraComponent
    // {
    //     const Mat4& view()        const { return ViewMatrix;        }
	// 	const Mat4& inverseView() const { return InverseViewMatrix; }
    //     const Mat4& projection()  const { return ProjectionMatrix;  }
		
	// 	float       aspect()      const { return Aspect;   }
	// 	float       farClip()     const { return FarClip;  }
	// 	float       nearClip()    const { return NearClip; }
	// 	float       fieldOfView() const { return FOV;      }

	// 	void setFarClip(float farClip)           { FarClip           = farClip;  }
	// 	void setNearClip(float nearClip)         { NearClip          = nearClip; }
	// 	void setFOV(float fov)                   { FOV               = fov;      }
	// 	void setAspect(float aspect)             { Aspect            = aspect;   }
    //     void setView(const Mat4& view)           { ViewMatrix        = view;     }
	// 	void setInverseView(const Mat4& invView) { InverseViewMatrix = invView;  }
    //     void setProjection(const Mat4& proj)     { ProjectionMatrix  = proj;     }

	// 	float FOV      = glm::radians(50.0f);
	// 	float Aspect   = 1.0f;
	// 	float NearClip = 0.1f;
	// 	float FarClip  = 100.0f;

	// 	Mat4 ViewMatrix        = Mat4{ 1.0f };
	// 	Mat4 InverseViewMatrix = Mat4{ 1.0f };
	// 	Mat4 ProjectionMatrix  = Mat4{ 1.0f };

	// 	bool IsMainCamera = true;
	// };





















	// /**
	//  * CameraComponent - Camera projection settings (Pure data)
	//  *
	//  * Stores raw camera parameters. CameraSystem computes view/projection matrices
	//  * and stores them in RenderCamera component (see render_components.hpp).
	//  *
	//  * TransformComponent provides position/orientation.
	//  * This component only stores projection settings.
	//  */
	// struct CameraComponent3
	// {
	// 	enum class EType { Perspective, Orthographic };

	// 	EType Type   = EType::Perspective;
	// 	bool  Active = true;  // Is this the active rendering camera?

	// 	// Perspective settings
	// 	float FovYDegrees = 45.0f;

	// 	// Orthographic settings
	// 	float OrthoWidth = 10.0f;

	// 	// Common settings
	// 	float AspectRatio = 16.0f / 9.0f;
	// 	float NearPlane   = 0.1f;
	// 	float FarPlane    = 1000.0f;

	// 	constexpr CameraComponent3() = default;

	// 	// Factory methods
	// 	static constexpr CameraComponent3 createPerspective(
	// 		float fovYDegrees = 45.0f,
	// 		float aspectRatio = 16.0f / 9.0f,
	// 		float nearPlane   = 0.1f,
	// 		float farPlane    = 1000.0f) 
	// 	{
	// 		CameraComponent3 comp;
	// 		{
	// 			comp.Type        = EType::Perspective;
	// 			comp.FovYDegrees = fovYDegrees;
	// 			comp.AspectRatio = aspectRatio;
	// 			comp.NearPlane   = nearPlane;
	// 			comp.FarPlane    = farPlane;
	// 		}

	// 		return comp;
	// 	}

	// 	static constexpr CameraComponent3 createOrthographic(
	// 		float orthoWidth  = 10.0f,
	// 		float aspectRatio = 16.0f / 9.0f,
	// 		float nearPlane   = 0.1f,
	// 		float farPlane    = 1000.0f
	// 	) {
	// 		CameraComponent3 comp;
	// 		{
	// 			comp.Type        = EType::Orthographic;
	// 			comp.OrthoWidth  = orthoWidth;
	// 			comp.AspectRatio = aspectRatio;
	// 			comp.NearPlane   = nearPlane;
	// 			comp.FarPlane    = farPlane;
	// 		}

	// 		return comp;
	// 	}
	// };




















	// /**
	//  * RenderCamera - Computed camera matrices and frustum
	//  *
	//  * Automatically computed by CameraSystem from CameraComponent + TransformComponent.
	//  * Used by rendering systems for view/projection and culling.
	//  *
	//  * Separation: CameraComponent has pure data (FOV, near, far),
	//  * RenderCamera has computed matrices (for rendering).
	//  */
	// struct RenderCameraComponent
	// {
	// 	Mat4 View          { 1.0f }; // View matrix (world → camera space)
	// 	Mat4 Projection    { 1.0f }; // Projection matrix (camera → clip space)
	// 	Mat4 ViewProjection{ 1.0f }; // Combined view-projection matrix

	// 	// Frustum planes (for culling) - optional, can be computed on-demand
	// 	// Format: vec4(A, B, C, D) where Ax + By + Cz + D = 0
	// 	Vec4 FrustumPlanes[6];

	// 	constexpr RenderCameraComponent() = default;
	// };

/**
 * CameraComponent - Camera projection settings (Pure data)
 *
 * Stores raw camera parameters. CameraSystem computes view/projection matrices
 * and stores them in RenderCamera component (see render_components.hpp).
 *
 * TransformComponent provides position/orientation.
 * This component only stores projection settings.
 */
// struct CameraComponent 
// {
//     enum class EType { Perspective, Orthographic };

//     EType Type = EType::Perspective;
//     bool  Active = true;  // Is this the active rendering camera?

//     // Perspective settings
//     float FovYDegrees = 45.0f;

//     // Orthographic settings
//     float OrthoWidth = 10.0f;

//     // Common settings
//     float AspectRatio = 16.0f / 9.0f;
//     float NearPlane = 0.1f;
//     float FarPlane = 1000.0f;

//     constexpr CameraComponent() = default;

//     // Factory methods
//     static constexpr CameraComponent createPerspective(
//         float fovYDegrees = 45.0f,
//         float aspectRatio = 16.0f / 9.0f,
//         float nearPlane = 0.1f,
//         float farPlane = 1000.0f
//     ) {
//         CameraComponent comp;
//         comp.Type = EType::Perspective;
//         comp.FovYDegrees = fovYDegrees;
//         comp.AspectRatio = aspectRatio;
//         comp.NearPlane = nearPlane;
//         comp.FarPlane = farPlane;
//         return comp;
//     }

//     static constexpr CameraComponent createOrthographic(
//         float orthoWidth = 10.0f,
//         float aspectRatio = 16.0f / 9.0f,
//         float nearPlane = 0.1f,
//         float farPlane = 1000.0f
//     ) {
//         CameraComponent comp;
//         comp.Type = EType::Orthographic;
//         comp.OrthoWidth = orthoWidth;
//         comp.AspectRatio = aspectRatio;
//         comp.NearPlane = nearPlane;
//         comp.FarPlane = farPlane;
//         return comp;
//     }
// };


// /**
//  * RenderCameraComponent - Computed camera matrices and frustum
//  *
//  * Automatically computed by CameraSystem from CameraComponent + TransformComponent.
//  * Used by rendering systems for view/projection and culling.
//  *
//  * Separation: CameraComponent has pure data (FOV, near, far),
//  * RenderCamera has computed matrices (for rendering).
//  */
// struct RenderCameraComponent 
// {
//     Mat4 View{1.0f};          // View matrix (world → camera space)
//     Mat4 Projection{1.0f};    // Projection matrix (camera → clip space)
//     Mat4 ViewProjection{1.0f}; // Combined view-projection matrix

//     // Frustum planes (for culling) - optional, can be computed on-demand
//     // Format: vec4(A, B, C, D) where Ax + By + Cz + D = 0
//     Vec4 FrustumPlanes[6];

//     constexpr RenderCameraComponent() = default;
// };

// /**
//  * LocalToWorldComponent - Computed transform matrix
//  *
//  * Automatically computed by TransformSystem from TransformComponent.
//  * Used by rendering systems to build push constants.
//  *
//  * Separation: TransformComponent has raw data (pos, rot, scale),
//  * LocalToWorld has computed matrix (for rendering).
//  */
// struct LocalToWorldComponent
// {
//     Mat4 Matrix{1.0f};       // Local-to-world transform
//     Mat4 NormalMatrix{1.0f}; // For transforming normals (inverse transpose)

//     constexpr LocalToWorldComponent() = default;

//     explicit constexpr LocalToWorldComponent(const Mat4& m, const Mat4& n = Mat4(1.0f)) noexcept
//         : Matrix(m), NormalMatrix(n) {}
// };

// 	/**
// 	 * CameraData - Per-view camera uniforms
// 	 *
// 	 * Update frequency: Per frame, per camera/view
// 	 * Shader binding: set = 0, binding = 1, std140
// 	 *
// 	 * Contains:
// 	 * - View and projection matrices (and inverses)
// 	 * - Precomputed ViewProjection matrix
// 	 * - Camera position and Direction
// 	 * - Near/far planes, FOV, aspect ratio
// 	 * - Screen resolution
// 	 *
// 	 * Design notes:
// 	 * - std140 layout (matrices are column-major, 16-byte aligned)
// 	 * - Provides inverse matrices for screenspace reconstruction
// 	 * - Explicit position/Direction for lighting calculations
// 	 * - All commonly needed camera data in one place
// 	 */
// 	struct CameraData 
// 	{
// 		// --- Matrices (64 bytes each) ---
// 		Mat4 View;                // world -> view space
// 		Mat4 Projection;          // view -> clip space (reversed-Z friendly)
// 		Mat4 ViewProjection;      // precomputed: projection * view
// 		Mat4 InverseView;             // view -> world (for world-space reconstruction)
// 		Mat4 InverseProjection;       // clip -> view (for screenspace -> viewspace)

// 		// --- Camera Vectors (32 bytes) ---
// 		Vec4 Position;            // xyz = world position, w = unused
// 		Vec4 Direction;           // xyz = forward Direction (normalized), w = unused

// 		// --- Camera Parameters (16 bytes) ---
// 		Vec2 NearFar;             // x = near plane, y = far plane
// 		Vec2 ScreenSize;          // x = width, y = height (in pixels)

// 		// --- Additional Parameters (16 bytes) ---
// 		float Fov;                 // vertical field of view (radians)
// 		float AspectRatio;         // width / height
// 		U32   _pad0;
// 		U32   _pad1;

// 		// Default constructor
// 		constexpr CameraData() noexcept
// 			: View(1.0f)
// 			, Projection(1.0f)
// 			, ViewProjection(1.0f)
// 			, InverseView(1.0f)
// 			, InverseProjection(1.0f)
// 			, Position(0.0f, 0.0f, 0.0f, 0.0f)
// 			, Direction(0.0f, 0.0f, -1.0f, 0.0f)
// 			, NearFar(0.1f, 1000.0f)
// 			, ScreenSize(1920.0f, 1080.0f)
// 			, Fov(glm::radians(60.0f))
// 			, AspectRatio(1920.0f / 1080.0f)
// 			, _pad0(0), _pad1(0)
// 		{}

// 		/**
// 		 * Update camera from position, target, and projection parameters
// 		 *
// 		 * @param pos Camera position in world space
// 		 * @param target Point to look at in world space
// 		 * @param up Up vector (usually (0, 1, 0))
// 		 * @param fovRadians Vertical field of view in radians
// 		 * @param aspect Aspect ratio (width / height)
// 		 * @param nearPlane Near clip plane
// 		 * @param farPlane Far clip plane
// 		 * @param screenWidth Screen width in pixels
// 		 * @param screenHeight Screen height in pixels
// 		 */
// 		void setLookAt(
// 			const Vec3& pos,
// 			const Vec3& target,
// 			const Vec3& up,
// 			float fovRadians,
// 			float aspect,
// 			float nearPlane,
// 			float farPlane,
// 			float screenWidth,
// 			float screenHeight
// 		) {
// 			// Store position and compute direction
// 			Position  = Vec4(pos, 0.0f);
// 			Direction = Vec4(glm::normalize(target - pos), 0.0f);

// 			// Build view matrix
// 			View        = glm::lookAt(pos, target, up);
// 			InverseView = glm::inverse(View);

// 			// Build projection matrix (Vulkan NDC: x,y in [-1,1], z in [0,1])
// 			Projection = glm::perspective(fovRadians, aspect, nearPlane, farPlane);

// 			// GLM was designed for OpenGL where clip space Z is [-1, 1] and Y is inverted
// 			// For Vulkan, we need to flip Y and use [0, 1] for Z (handled by GLM_FORCE_DEPTH_ZERO_TO_ONE)
// 			// But we still need to flip Y:
// 			Projection[1][1] *= -1.0f;

// 			InverseProjection = glm::inverse(Projection);

// 			// Precompute combined matrix
// 			ViewProjection = Projection * View;

// 			// Store parameters
// 			NearFar     = Vec2(nearPlane, farPlane);
// 			ScreenSize  = Vec2(screenWidth, screenHeight);
// 			Fov         = fovRadians;
// 			AspectRatio = aspect;
// 		}

// 		/**
// 		 * Update camera from view and projection matrices directly
// 		 * (Useful when integrating with external camera systems)
// 		 */
// 		void setMatrices(
// 			const Mat4& viewMatrix,
// 			const Mat4& projectionMatrix,
// 			float screenWidth,
// 			float screenHeight
// 		) {
// 			View              = viewMatrix;
// 			Projection        = projectionMatrix;
// 			ViewProjection    = Projection * View;
// 			InverseView       = glm::inverse(View);
// 			InverseProjection = glm::inverse(Projection);

// 			// Extract position from inverse view matrix
// 			Position = Vec4(InverseView[3].x, InverseView[3].y, InverseView[3].z, 0.0f);

// 			// Extract forward Direction from view matrix (negative Z axis)
// 			Direction = Vec4(-View[0][2], -View[1][2], -View[2][2], 0.0f);

// 			ScreenSize = Vec2(screenWidth, screenHeight);
// 		}

// 		/**
// 		 * Update only projection matrix (useful for resize)
// 		 */
// 		void updateProjection(
// 			float fovRadians,
// 			float aspect,
// 			float nearPlane,
// 			float farPlane,
// 			float screenWidth,
// 			float screenHeight
// 		) {
// 			Projection        = glm::perspective(fovRadians, aspect, nearPlane, farPlane);
// 			Projection[1][1] *= -1.0f; // Flip Y for Vulkan
// 			InverseProjection = glm::inverse(Projection);
// 			ViewProjection    = Projection * View;

// 			NearFar     = Vec2(nearPlane, farPlane);
// 			ScreenSize  = Vec2(screenWidth, screenHeight);
// 			Fov         = fovRadians;
// 			AspectRatio = aspect;
// 		}

// 		/**
// 		 * Get the right vector (X axis in camera space)
// 		 */
// 		constexpr Vec3 rightVector() const noexcept 
// 		{
// 			return Vec3(InverseView[0].x, InverseView[0].y, InverseView[0].z);
// 		}

// 		/**
// 		 * Get the up vector (Y axis in camera space)
// 		 */
// 		constexpr Vec3 upVector() const noexcept 
// 		{
// 			return Vec3(InverseView[1].x, InverseView[1].y, InverseView[1].z);
// 		}

// 		/**
// 		 * Get the forward vector (negative Z axis in camera space)
// 		 */
// 		constexpr Vec3 forwardVector() const noexcept 
// 		{
// 			return Vec3(Direction.x, Direction.y, Direction.z);
// 		}
// 	};

// 	// Size validation (std140 alignment)
// 	static_assert(sizeof(CameraData) % 16 == 0, "CameraData must be aligned to 16 bytes for std140");



// 	/**
// 	 * ProjectionType - Type of camera projection
// 	 */
// 	enum class ProjectionType 
// 	{
// 		Perspective,
// 		Orthographic
// 	};

// 	/**
// 	 * Camera - Pure data class for camera state (ECS-ready)
// 	 *
// 	 * Design principles:
// 	 * - Separate data from behavior (no GLFW, no input handling)
// 	 * - Can be used as ECS component directly
// 	 * - CameraController (separate class) will manipulate Camera data
// 	 * - Utility functions for computing matrices and populating CameraData UBO
// 	 */
// 	class VyCamera 
// 	{
// 	public:
// 		// --- Construction ---
// 		VyCamera() = default;

// 		/**
// 		 * Create perspective camera
// 		 *
// 		 * @param position Camera position in world space
// 		 * @param target Look-at target point
// 		 * @param fovYDegrees Vertical field of view in degrees
// 		 * @param aspectRatio Width/height ratio
// 		 * @param nearPlane Near clipping plane
// 		 * @param farPlane Far clipping plane
// 		 */
// 		static VyCamera createPerspective(
// 			const Vec3& position,
// 			const Vec3& target,
// 			float fovYDegrees,
// 			float aspectRatio,
// 			float nearPlane = 0.1f,
// 			float farPlane = 1000.0f
// 		);

// 		/**
// 		 * Create orthographic camera
// 		 *
// 		 * @param position Camera position in world space
// 		 * @param target Look-at target point
// 		 * @param orthoWidth Width of orthographic view volume
// 		 * @param aspectRatio Width/height ratio
// 		 * @param nearPlane Near clipping plane
// 		 * @param farPlane Far clipping plane
// 		 */
// 		static VyCamera createOrthographic(
// 			const Vec3& position,
// 			const Vec3& target,
// 			float orthoWidth,
// 			float aspectRatio,
// 			float nearPlane = 0.1f,
// 			float farPlane = 1000.0f
// 		);

// 		// --- Position and Orientation ---

// 		void setPosition(const Vec3& pos) { m_Position = pos; updateViewMatrix(); }
// 		constexpr const Vec3& position() const noexcept { return m_Position; }

// 		void setRotation(const Quat& rot) { m_Rotation = rot; updateViewMatrix(); }
// 		constexpr const Quat& rotation() const noexcept { return m_Rotation; }

// 		/**
// 		 * Set camera orientation by specifying look-at target and up direction
// 		 *
// 		 * @param target World space point to look at
// 		 * @param up Up direction (default: +Y)
// 		 */
// 		void lookAt(const Vec3& target, const Vec3& up = Vec3(0.0f, 1.0f, 0.0f));

// 		/**
// 		 * Set camera rotation using Euler angles (in degrees)
// 		 *
// 		 * @param pitch Rotation around right axis (X)
// 		 * @param yaw Rotation around up axis (Y)
// 		 * @param roll Rotation around forward axis (Z)
// 		 */
// 		void setEulerAngles(float pitch, float yaw, float roll);

// 		/**
// 		 * Get Euler angles from current rotation (in degrees)
// 		 *
// 		 * @return vec3(pitch, yaw, roll)
// 		 */
// 		Vec3 eulerAngles() const;

// 		// --- Direction Vectors ---

// 		Vec3 forward() const { return m_Rotation * Vec3( 0.0f,  0.0f, -1.0f ); }
// 		Vec3 right()   const { return m_Rotation * Vec3( 1.0f,  0.0f,  0.0f ); }
// 		Vec3 up()      const { return m_Rotation * Vec3( 0.0f,  1.0f,  0.0f ); }

// 		// --- Projection Parameters ---

// 		void setProjectionType(ProjectionType type) { m_ProjectionType = type; updateProjectionMatrix(); }
// 		constexpr ProjectionType projectionType() const noexcept { return m_ProjectionType; }

// 		void setFovY(float fovYDegrees) { m_FovY = fovYDegrees; updateProjectionMatrix(); }
// 		constexpr float fovY() const noexcept { return m_FovY; }

// 		void setAspectRatio(float aspect) { m_AspectRatio = aspect; updateProjectionMatrix(); }
// 		constexpr float aspectRatio() const noexcept { return m_AspectRatio; }

// 		void setNearPlane(float nearPlane) { m_NearPlane = nearPlane; updateProjectionMatrix(); }
// 		constexpr float nearPlane() const noexcept { return m_NearPlane; }

// 		void setFarPlane(float farPlane) { m_FarPlane = farPlane; updateProjectionMatrix(); }
// 		constexpr float farPlane() const noexcept { return m_FarPlane; }

// 		void setOrthoWidth(float width) { m_OrthoWidth = width; updateProjectionMatrix(); }
// 		constexpr float orthoWidth() const noexcept { return m_OrthoWidth; }

// 		/**
// 		 * Update aspect ratio (typically called on window resize)
// 		 *
// 		 * @param width Viewport width in pixels
// 		 * @param height Viewport height in pixels
// 		 */
// 		void updateAspectRatio(U32 width, U32 height) 
// 		{
// 			m_AspectRatio = static_cast<float>(width) / static_cast<float>(height);
// 			updateProjectionMatrix();
// 		}

// 		// --- Matrices ---

// 		constexpr const Mat4& viewMatrix() const noexcept { return m_ViewMatrix; }
// 		constexpr const Mat4& projectionMatrix() const noexcept { return m_ProjectionMatrix; }
// 		constexpr Mat4 viewProjectionMatrix() const noexcept { return m_ProjectionMatrix * m_ViewMatrix; }

// 		// --- CameraData Population ---

// 		/**
// 		 * Populate CameraData struct for uploading to GPU
// 		 *
// 		 * @param screenWidth Screen width in pixels
// 		 * @param screenHeight Screen height in pixels
// 		 * @return CameraData ready to upload to UBO
// 		 */
// 		CameraData toCameraData(U32 screenWidth, U32 screenHeight) const;

// 	private:
// 		// Position and orientation
// 		Vec3 m_Position{ 0.0f, 0.0f, 0.0f };
// 		Quat m_Rotation{ 1.0f, 0.0f, 0.0f, 0.0f};  // Identity quaternion

// 		// Projection parameters
// 		ProjectionType m_ProjectionType = ProjectionType::Perspective;
// 		float m_FovY        = 60.0f;        // Vertical FOV in degrees (perspective)
// 		float m_AspectRatio = 16.0f / 9.0f;
// 		float m_NearPlane   = 0.1f;
// 		float m_FarPlane    = 1000.0f;
// 		float m_OrthoWidth  = 10.0f;        // Width of orthographic view (orthographic)

// 		// Cached matrices
// 		Mat4 m_ViewMatrix{ 1.0f };
// 		Mat4 m_ProjectionMatrix{ 1.0f };

// 		// Internal update methods
// 		void updateViewMatrix();
// 		void updateProjectionMatrix();
// 	};








}