#pragma once

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
        const Mat4& projection() const 
        { 
            return m_ProjectionMatrix; 
        }

        /**
         * @brief Retrieves the view matrix.
         * 
         * @return The view matrix.
         */
        const Mat4& view() const 
        { 
            return m_ViewMatrix; 
        }

        /**
         * @brief Retrieves the inverse view matrix.
         * 
         * @return The inverse view matrix.
         */
        const Mat4& inverseView() const 
        { 
            return m_InverseViewMatrix; 
        }

        /**
         * @brief Retrieves the camera position from the inverse view matrix.
         * 
         * @return The camera position in world coordinates.
         */
        const Vec3 position() const 
        { 
            return Vec3( m_InverseViewMatrix[ 3 ] ); 
        }

		const bool isPerspective() const { return m_IsPerspective; }

		F32 fovYDegrees() const { return m_FovYDegrees; }

		F32 nearZ()   const { return m_zNear; }
		F32 farZ()    const { return m_zFar;  }

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

		F32 m_FovYDegrees{  80.0f };
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