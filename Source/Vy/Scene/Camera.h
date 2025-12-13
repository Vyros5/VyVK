#pragma once

// #include <Vy/GFX/FrameInfo.h>
#include <VyLib/VyLib.h>

namespace Vy
{
    /**
     * EProjectionType - Type of camera projection
     */
    enum class EProjectionType 
    {
        PERSPECTIVE,
        ORTHOGRAPHIC
    };

    struct VyFrustum
    {
        Vec4 Planes[6]; // Left, Right, Bottom, Top, Near, Far
    };

    namespace 
    {
		constexpr U32 ORTHO_L = 0; // Left
		constexpr U32 ORTHO_R = 1; // Right
		constexpr U32 ORTHO_T = 2; // Top
		constexpr U32 ORTHO_B = 3; // Bottom
    }

    /**
     * @brief Camera class for handling perspective and orthographic projections.
     */
    class VyCamera 
    {
    public:
        /**
         * @brief Sets the camera projection to a perspective projection.
         * 
         * @param aspect The aspect ratio (width / height).
         */
        void setPerspective(F32 aspect);

        void setAspect(F32 aspect);

        void setFOV(F32 fovY);

		void setPerspectiveParams(
            F32 fovYDegrees, 
            F32 zNear, 
            F32 zFar
        );

        /**
         * @brief Sets the camera projection to an orthographic projection.
         */
        void setOrthographic();

		void setOrthographicParams(
            F32 left, 
            F32 right, 
            F32 top, 
            F32 bottom, 
            F32 zNear, 
            F32 zFar
        );

		void setIsPerspective(bool isPerspective) 
        { 
            m_IsPerspective = isPerspective; 
        }

        /**
         * @brief Sets the camera view matrix based on a direction vector.
         * 
         * @param position  The camera position in world coordinates.
         * @param direction The direction the camera is facing.
         * @param up        The up vector, defaults to ( 0, -1, 0).
         */
        void setViewDirection(
            Vec3 position, 
            Vec3 direction, 
            Vec3 up = Vec3{ 0.0f, -1.0f, 0.0f }
        );

        /**
         * @brief Sets the camera view matrix to look at a specific target.
         * 
         * @param position The camera position in world coordinates.
         * @param target   The target position to look at.
         * @param up       The up vector, defaults to ( 0, -1, 0).
         */
        void setViewTarget(
            Vec3 position, 
            Vec3 target, 
            Vec3 up = Vec3{ 0.0f, -1.0f, 0.0f }
        );

        /**
         * @brief Sets the camera view matrix using YXZ Euler angles.
         * 
         * @param position The camera position in world coordinates.
         * @param rotation Euler angles representing the camera's orientation.
         */
        void setView(Vec3 position, Vec3 rotation);

        // void setView(Vec3 position, Quat rotation);

        /**
         * @brief Retrieves the projection matrix.
         * 
         * @return The projection matrix.
         */
        const Mat4& projection() const { return m_ProjectionMatrix; }

        /**
         * @brief Retrieves the view matrix.
         * 
         * @return The view matrix.
         */
        const Mat4& view() const { return m_ViewMatrix; }

        /**
         * @brief Retrieves the inverse view matrix.
         * 
         * @return The inverse view matrix.
         */
        const Mat4& inverseView() const { return m_InverseViewMatrix; }

        /**
         * @brief Retrieves the camera position from the inverse view matrix.
         * 
         * @return The camera position in world coordinates.
         */
        const Vec3 position() const { return Vec3(m_InverseViewMatrix[3]); }

		const bool isPerspective() const { return m_IsPerspective; }

		F32 fovYDegrees() const { return m_FovYDegrees; }

		F32 nearPlane()   const { return m_zNear; }
		F32 farPlane()    const { return m_zFar;  }

        F32 aspect()      const { return m_Aspect; }

		F32 orthoLeft()   const { return m_OrthoParams[ ORTHO_L ]; }
		F32 orthoRight()  const { return m_OrthoParams[ ORTHO_R ]; }
		F32 orthoTop()    const { return m_OrthoParams[ ORTHO_T ]; }
		F32 orthoBottom() const { return m_OrthoParams[ ORTHO_B ]; }

        // Frustum culling support.
        void updateFrustum();
        bool isInFrustum(const Vec3& center, float radius) const;

        const VyFrustum& frustum() const { return m_Frustum; }

    private:
        /**
         * @brief Updates the camera's view matrix based on provided basis vectors.
         * 
         * @param u        The right vector of the camera.
         * @param v        The up vector of the camera.
         * @param w        The forward vector of the camera.
         * @param position The camera position in world coordinates.
         */
        void updateViewMatrix(Vec3 u, Vec3 v, Vec3 w, Vec3 position);

        // Camera Transform : World to Camera
        Mat4 m_ViewMatrix{ 1.0f };

        // Projection Transform : Camera to Clip
        Mat4 m_ProjectionMatrix{ 1.0f };

        // Inverse of Camera Transform : Camera to World
        Mat4 m_InverseViewMatrix{ 1.0f };

		F32 m_FovYDegrees{  50.0f };
		F32 m_zNear      {   0.1f };
		F32 m_zFar       { 100.0f };
        F32 m_Aspect     {   0.0f };

        // Frustum planes for culling.
        VyFrustum m_Frustum{};

		Vec4 m_OrthoParams{ 
            -1.0f, // Left
             1.0f, // Right
            -1.0f, // Top
             1.0f  // Bottom
        };

		bool m_IsPerspective{ true };
    };
}

// class EngineCamera 
// {
// public:
// 	void setOrthographicProjection(
//         F32 left, 
//         F32 right, 
//         F32 top, 
//         F32 bottom, 
//         F32 zNear, 
//         F32 zFar
//     );

// 	void setPerspectiveProjection(
//         F32 fovy, 
//         F32 aspect, 
//         F32 zNear, 
//         F32 zFar
//     );

// 	void setViewDirection(
//         Vec3 position, 
//         Vec3 direction, 
//         Vec3 up = Vec3(0.0f, 1.0f, 0.0f)
//     );
    
//     void setViewTarget(
//         Vec3 position, 
//         Vec3 target, 
//         Vec3 up = Vec3(0.0f, 1.0f, 0.0f)
//     );

// 	void setViewYXZ(Vec3 position, Vec3 rotation);
    
//     void setViewYXZ(Mat4 transformMatrix);

// 	const Mat4& view()              const { return m_ViewMatrix;             }
// 	const Mat4& inverseView()       const { return m_InvViewMatrix;          }
// 	const Mat4& projection()        const { return m_ProjectionMatrix;       }
// 	const Mat4& inverseProjection() const { return m_InvProjectionMatrix;    }
// 	const Vec3  position()          const { return Vec3(m_InvViewMatrix[3]); }

// private:
// 	Mat4 m_ProjectionMatrix   { 1.0f };
// 	Mat4 m_InvProjectionMatrix{ 1.0f };
//     Mat4 m_ViewMatrix         { 1.0f };
// 	Mat4 m_InvViewMatrix      { 1.0f };
// };









//     class VCamera 
//     {
//     public:

//         VCamera(const VCamera& other) = default;
//         VCamera(VCamera&& other)      = default;

//         VCamera& operator=(const VCamera& other) = default;
//         VCamera& operator=(VCamera&& other)      = default;

//         VY_NODISCARD const Mat4& projection() const { return m_Projection; }
//         VY_NODISCARD const Mat4& viewMatrix() const { return m_ViewMatrix; }

//         void setProjection(const Mat4& projection = Mat4(1.0)) 
//         { 
//             m_Projection = projection; 
//         }

//         VY_NODISCARD const Mat4& transform() const { return m_Transform; }
//         VY_NODISCARD       Mat4& transform()       { return m_Transform; }
        
//         void setTransform(const Mat4& transform) 
//         { 
//             m_Transform = transform; 
//         }

//         VY_NODISCARD const Vec3& targetForward() const { return m_ForwardVector; }
//         VY_NODISCARD F32       fieldOfView()   const { return m_FieldOfView; }
//         VY_NODISCARD F32       nearPlane()     const { return m_NearClip; }
//         VY_NODISCARD F32       farPlane()      const { return m_FarClip; }
//         VY_NODISCARD F32       aspectRatio()   const { return m_ViewportWidth / m_ViewportHeight; }

//         VY_NODISCARD auto viewport() const -> decltype(auto) 
//         { 
//             return std::make_pair(m_ViewportWidth, m_ViewportHeight); 
//         }


//         /**
//          * @brief Retrieve the view projection matrix.
//          * @return The result of the projection matrix multiplied by the view matrix.
//          * */
//         VY_NODISCARD Mat4 viewProjection() const { return this->projection() * m_ViewMatrix; }

//         VY_NODISCARD const Vec3& position() const { return m_Position; }
//         VY_NODISCARD const Vec3& rotation() const { return m_Rotation; }

//         void setPosition(const Vec3& position) 
//         {
//             m_Translation = position;
//             m_Transform = glm::translate(m_Transform, m_Translation);
//         }

//         void SetRotation(const Vec3& angles = Vec3(0.0f))
//         {
//             m_Rotation = angles;

//             m_Transform = glm::rotate(m_Transform, glm::radians(m_Rotation[0]), Math::UNIT_VECTOR_X);
//             m_Transform = glm::rotate(m_Transform, glm::radians(m_Rotation[1]), Math::UNIT_VECTOR_Y);
//             m_Transform = glm::rotate(m_Transform, glm::radians(m_Rotation[2]), Math::UNIT_VECTOR_Z);
//         }

//         VY_NODISCARD EProjectionType projectionType() const { return m_ProjectionType; }
//         VY_NODISCARD bool isOrthographic() const { return m_ProjectionType == EProjectionType::ORTHOGRAPHIC; }

//         void setProjectionType(const EProjectionType type) 
//         {
//             m_ProjectionType = type;

//             updateProjection();
//         }

//         ~VCamera() = default;

//     protected:

//         explicit VCamera(
//             const Mat4&     projection     = Mat4(1.0f), 
//             const Mat4&     transform      = Mat4(1.0f), 
//             EProjectionType projectionType = EProjectionType::PERSPECTIVE
//         ) :   
//             m_Projection    { projection     }, 
//             m_Transform     { transform      }, 
//             m_ProjectionType{ projectionType }
//         {
//             updateProjection();
//         }

//         void updateProjection() 
//         {
//             m_AspectRatio = m_ViewportWidth / m_ViewportHeight;

//             switch(m_ProjectionType) 
//             {
//                 case EProjectionType::ORTHOGRAPHIC:

//                     setProjection(glm::ortho(0.0f, m_ViewportWidth, 0.0f, m_ViewportHeight));
//                     break;

//                 case EProjectionType::PERSPECTIVE:

//                     setProjection(glm::perspective(glm::radians(m_FieldOfView), m_AspectRatio, m_NearClip, m_FarClip));
//                     break;
//             }
//         }

//     protected:

//         F32 m_ViewportWidth { 1920 };
//         F32 m_ViewportHeight{ 1080 };

//         // [Projection Data]
//         F32 m_NearClip   { 0.1f    };
//         F32 m_FarClip    { 1000.0f };
//         F32 m_FieldOfView{ 45.0f   };
//         F32 m_AspectRatio{ m_ViewportWidth / m_ViewportHeight };

//         // [Matrices]
//         Mat4 m_ViewMatrix      {};
//         Mat4 m_ProjectionMatrix{};

//         // [Vectors]
//         Vec3 m_Position      { -1.0f,  0.0f,  1.0f };
//         Vec3 m_RightVector   {  1.0f,  0.0f,  0.0f };
//         Vec3 m_CameraUpVector{  0.0f,  1.0f,  0.0f };
//         Vec3 m_ForwardVector {  1.0f,  0.0f, -1.0f };

//         // [Rotations]
//         F32 m_Roll { 0.0f }; // Rotation around the X axis.
//         F32 m_Pitch{ 0.0f }; // Rotation around the Y axis.
//         F32 m_Yaw  { 0.0f }; // Rotation around the Z axis.

//         // [Misc]
//         VyUUID m_CameraID{};

//         Mat4 m_Projection{};
//         Mat4 m_Transform {};

//         Vec3 m_Translation{};
//         Vec3 m_Rotation   {};

//         EProjectionType m_ProjectionType{ EProjectionType::PERSPECTIVE };
//     };
// }












//     /**
//      * Camera - Pure data class for camera state (ECS-ready)
//      *
//      * Design principles:
//      * - Separate data from behavior (no GLFW, no input handling)
//      * - Can be used as ECS component directly
//      * - CameraController (separate class) will manipulate Camera data
//      * - Utility functions for computing matrices and populating CameraData UBO
//      */
//     class Camera 
//     {
//     public:
//         // --- Construction ---
//         Camera() = default;

//         /**
//          * Create perspective camera
//          *
//          * @param position    Camera position in world space
//          * @param target      Look-at target point
//          * @param fovYDegrees Vertical field of view in degrees
//          * @param aspectRatio Width/height ratio
//          * @param nearPlane   Near clipping plane
//          * @param farPlane    Far clipping plane
//          */
//         static Camera createPerspective(
//             const Vec3& position,
//             const Vec3& target,
//             F32         fovYDegrees,
//             F32         aspectRatio,
//             F32         nearPlane = 0.1f,
//             F32         farPlane  = 1000.0f
//         );

//         /**
//          * Create orthographic camera
//          *
//          * @param position    Camera position in world space
//          * @param target      Look-at target point
//          * @param orthoWidth  Width of orthographic view volume
//          * @param aspectRatio Width/height ratio
//          * @param nearPlane   Near clipping plane
//          * @param farPlane    Far clipping plane
//          */
//         static Camera createOrthographic(
//             const Vec3& position,
//             const Vec3& target,
//             F32         orthoWidth,
//             F32         aspectRatio,
//             F32         nearPlane = 0.1f,
//             F32         farPlane  = 1000.0f
//         );

//         // --- Position and Orientation ---

//         void setPosition(const Vec3& pos) 
//         { 
//             m_Position = pos; 
//             updateViewMatrix(); 
//         }
        
//         constexpr const Vec3& position() const noexcept 
//         { 
//             return m_Position; 
//         }

//         void setRotation(const Quat& rot) 
//         { 
//             m_Rotation = rot; 
//             updateViewMatrix(); 
//         }
        
//         constexpr const Quat& rotation() const noexcept 
//         { 
//             return m_Rotation; 
//         }

//         /**
//          * Set camera orientation by specifying look-at target and up direction
//          *
//          * @param target World space point to look at
//          * @param up Up direction (default: +Y)
//          */
//         void lookAt(const Vec3& target, const Vec3& up = Vec3(0.0f, 1.0f, 0.0f));

//         /**
//          * Set camera rotation using Euler angles (in degrees)
//          *
//          * @param pitch Rotation around right axis (X)
//          * @param yaw Rotation around up axis (Y)
//          * @param roll Rotation around forward axis (Z)
//          */
//         void setEulerAngles(F32 pitch, F32 yaw, F32 roll);

//         /**
//          * Get Euler angles from current rotation (in degrees)
//          *
//          * @return vec3(pitch, yaw, roll)
//          */
//         Vec3 eulerAngles() const;

//         // --- Direction Vectors ---

//         Vec3 forward() const { return m_Rotation * Vec3( 0.0f,  0.0f, -1.0f); }
//         Vec3 right()   const { return m_Rotation * Vec3( 1.0f,  0.0f,  0.0f); }
//         Vec3 up()      const { return m_Rotation * Vec3( 0.0f,  1.0f,  0.0f); }

//         // --- Projection Parameters ---

//         void setProjectionType(EProjectionType type) { m_ProjectionType = type; updateProjectionMatrix(); }
//         constexpr EProjectionType projectionType() const noexcept { return m_ProjectionType; }

//         void setFovY(F32 fovYDegrees) { m_FovY = fovYDegrees; updateProjectionMatrix(); }
//         constexpr F32 fovY() const noexcept { return m_FovY; }

//         void setAspectRatio(F32 aspect) { m_AspectRatio = aspect; updateProjectionMatrix(); }
//         constexpr F32 aspectRatio() const noexcept { return m_AspectRatio; }

//         void setNearPlane(F32 nearPlane) { m_NearPlane = nearPlane; updateProjectionMatrix(); }
//         constexpr F32 nearPlane() const noexcept { return m_NearPlane; }

//         void setFarPlane(F32 farPlane) { m_FarPlane = farPlane; updateProjectionMatrix(); }
//         constexpr F32 farPlane() const noexcept { return m_FarPlane; }

//         void setOrthoWidth(F32 width) { m_OrthoWidth = width; updateProjectionMatrix(); }
//         constexpr F32 orthoWidth() const noexcept { return m_OrthoWidth; }

//         /**
//          * Update aspect ratio (typically called on window resize)
//          *
//          * @param width Viewport width in pixels
//          * @param height Viewport height in pixels
//          */
//         void updateAspectRatio(U32 width, U32 height) 
//         {
//             m_AspectRatio = static_cast<F32>(width) / static_cast<F32>(height);
            
//             updateProjectionMatrix();
//         }

//         // --- Matrices ---

//         constexpr const Mat4& viewMatrix()           const noexcept { return m_ViewMatrix; }
//         constexpr const Mat4& projectionMatrix()     const noexcept { return m_ProjectionMatrix; }
//         constexpr       Mat4  viewProjectionMatrix() const noexcept { return m_ProjectionMatrix * m_ViewMatrix; }

//         // --- CameraData Population ---

//         /**
//          * Populate CameraData struct for uploading to GPU
//          *
//          * @param screenWidth Screen width in pixels
//          * @param screenHeight Screen height in pixels
//          * @return CameraData ready to upload to UBO
//          */
//         CameraData toCameraData(U32 screenWidth, U32 screenHeight) const;

//     private:
//         // Position and orientation
//         Vec3 m_Position{0.0f, 0.0f, 0.0f };
//         Quat m_Rotation{1.0f, 0.0f, 0.0f, 0.0f}; // Identity quaternion

//         // Projection parameters
//         EProjectionType m_ProjectionType = EProjectionType::PERSPECTIVE;
//         F32             m_FovY           = 60.0f;              // Vertical FOV in degrees (perspective)
//         F32             m_AspectRatio    = 1; //16.0f / 9.0f;
//         F32             m_NearPlane      = 0.1f;
//         F32             m_FarPlane       = 1000.0f;
//         F32             m_OrthoWidth     = 10.0f;        // Width of orthographic view (orthographic)

//         // Cached matrices
//         Mat4 m_ViewMatrix      { 1.0f };
//         Mat4 m_ProjectionMatrix{ 1.0f };

//         // Internal update methods
//         void updateViewMatrix();
//         void updateProjectionMatrix();
//     };
// }