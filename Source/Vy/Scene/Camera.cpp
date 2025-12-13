#include <Vy/Scene/Camera.h>

namespace Vy
{
    /**
     * Column Major
     * ------------------
     * [col][row]
     * 
     * R = Right
     * U = Up
     * F = Forward
     * P = Position
     * ------------------
     * Rx Ux Fx Px
     * Ry Uy Uy Py
     * Rz Uz Fz Pz
     *  0  0  0  1
     * ------------------
     * 00 01 02 03 
     * 10 11 12 13 
     * 20 21 22 23 
     * 30 31 32 33 
     * ------------------
     */

    // ---------------------------------------------------------------------------------------------------------------------

    void VyCamera::setOrthographic() 
    {
        // Identity
        m_ProjectionMatrix = Mat4{ 1.0f };

        // Rx (2 / (R - L))
        m_ProjectionMatrix[0][0] = 2.0f / (m_OrthoParams[ ORTHO_R ] - m_OrthoParams[ ORTHO_L ]);
        
        // Uy (2 / (B - T))
        m_ProjectionMatrix[1][1] = 2.0f / (m_OrthoParams[ ORTHO_B ] - m_OrthoParams[ ORTHO_T ]);
        
        // Fz (1 / (zF - zN))
        m_ProjectionMatrix[2][2] = 1.0f / (m_zFar - m_zNear);
        
        // Bottom Col

        // -(R + L) / (R - L)
        m_ProjectionMatrix[3][0] = -(m_OrthoParams[ ORTHO_R ] + m_OrthoParams[ ORTHO_L ]) / (m_OrthoParams[ ORTHO_R ] - m_OrthoParams[ ORTHO_L ]);
        
        // -(B + T) / (B - T)
        m_ProjectionMatrix[3][1] = -(m_OrthoParams[ ORTHO_B ] + m_OrthoParams[ ORTHO_T ]) / (m_OrthoParams[ ORTHO_B ] - m_OrthoParams[ ORTHO_T ]);
        
        // -zN / (zF - zN)
        m_ProjectionMatrix[3][2] = -m_zNear / (m_zFar - m_zNear);


        // m_InverseProjectionMatrix = glm::inverse(m_ProjectionMatrix);
    }


    void VyCamera::setPerspective(F32 aspect) 
    {
        VY_ASSERT(glm::abs(aspect - std::numeric_limits<F32>::epsilon()) > 0.0f);

        m_Aspect = aspect;

        const F32 TanHalfFovy = tan(glm::radians(m_FovYDegrees) / 2.0f);

        // Zero out
        m_ProjectionMatrix = Mat4{ 0.0f };
        
        // X: Rx (1 / (A * THF))
        m_ProjectionMatrix[0][0] = 1.0f / (aspect * TanHalfFovy);

        // Y: Uy (-1 / (THF)) (note the minus to flip the Y axis)
        m_ProjectionMatrix[1][1] = 1.0f / (TanHalfFovy);

        // Z: Fz zF / (zF - zN)
        m_ProjectionMatrix[2][2] = m_zFar / (m_zFar - m_zNear);

        m_ProjectionMatrix[2][3] = 1.0f;
        
        // W: Fw -(zF * zN) / (zF - zN) (reversed-Z depth)
        m_ProjectionMatrix[3][2] = -(m_zFar * m_zNear) / (m_zFar - m_zNear);


        // m_InverseProjectionMatrix = glm::inverse(m_ProjectionMatrix);
    }

    // ---------------------------------------------------------------------------------------------------------------------

    void VyCamera::setPerspectiveParams(
        F32 fovYDegrees, 
        F32 zNear, 
        F32 zFar) 
    {
        m_FovYDegrees = fovYDegrees;
        m_zNear       = zNear;
        m_zFar        = zFar;
	}


    void VyCamera::setOrthographicParams(
        F32 left, 
        F32 right, 
        F32 top, 
        F32 bottom, 
        F32 zNear, 
        F32 zFar) 
    {
        m_OrthoParams[ ORTHO_L ] = left;
        m_OrthoParams[ ORTHO_R ] = right;
        m_OrthoParams[ ORTHO_T ] = top;
        m_OrthoParams[ ORTHO_B ] = bottom;
        m_zNear                  = zNear;
        m_zFar                   = zFar;
	}

    // ---------------------------------------------------------------------------------------------------------------------

    void VyCamera::setAspect(F32 aspect)
    {
        m_Aspect = aspect;
    }


    void VyCamera::setFOV(F32 fovY)
    {
        m_FovYDegrees = fovY;
    } 


    void VyCamera::setViewDirection(
        Vec3 position, 
        Vec3 direction, 
        Vec3 up) 
    {
        VY_ASSERT((glm::dot(direction, direction) > std::numeric_limits<F32>::epsilon()), "Direction cannot be zero");

		// Construct an orthonormal basis.
		// Three vectors of unit length are all orthogonal (90 degrees) to each other.
        const Vec3 w{ glm::normalize(direction)         }; // Front
        const Vec3 u{ glm::normalize(glm::cross(w, up)) }; // Right
        const Vec3 v{ glm::cross(w, u)                  }; // Up

        updateViewMatrix(u, v, w, position);
    }


    void VyCamera::setViewTarget(
        Vec3 position, 
        Vec3 target, 
        Vec3 up) 
    {
        VY_ASSERT((target - position).length() != 0.0f, "Direction cannot be zero");

        setViewDirection(position, target - position, up);
    }


    void VyCamera::setView(Vec3 position, Vec3 rotation) 
    {
        const F32 c3 = glm::cos(rotation.z);
        const F32 s3 = glm::sin(rotation.z);

        const F32 c2 = glm::cos(rotation.x);
        const F32 s2 = glm::sin(rotation.x);
        
        const F32 c1 = glm::cos(rotation.y);
        const F32 s1 = glm::sin(rotation.y);

        const Vec3 u{ ( c1 * c3 + s1 * s2 * s3 ), ( c2 * s3 ), ( c1 * s2 * s3 - c3 * s1 ) };
        const Vec3 v{ ( c3 * s1 * s2 - c1 * s3 ), ( c2 * c3 ), ( c1 * c3 * s2 + s1 * s3 ) };
        const Vec3 w{ ( c2 * s1                ), (-s2      ), ( c1 * c2                ) };
        
        updateViewMatrix(u, v, w, position);
    }

    // ---------------------------------------------------------------------------------------------------------------------
    
    void VyCamera::updateViewMatrix(Vec3 u, Vec3 v, Vec3 w, Vec3 position) 
    {
        m_ViewMatrix = Mat4{ 1.0f };
        {
            // Row 1
            m_ViewMatrix[0][0] = u.x;
            m_ViewMatrix[1][0] = u.y;
            m_ViewMatrix[2][0] = u.z;
            
            // Row 2
            m_ViewMatrix[0][1] = v.x;
            m_ViewMatrix[1][1] = v.y;
            m_ViewMatrix[2][1] = v.z;
            
            // Row 3
            m_ViewMatrix[0][2] = w.x;
            m_ViewMatrix[1][2] = w.y;
            m_ViewMatrix[2][2] = w.z;
            
            // Translation matrix components. (Col 4)
            m_ViewMatrix[3][0] = -glm::dot(u, position); // Row 1
            m_ViewMatrix[3][1] = -glm::dot(v, position); // Row 2
            m_ViewMatrix[3][2] = -glm::dot(w, position); // Row 3
        }

        m_InverseViewMatrix = Mat4{ 1.0f };
        {
            // Col 1
            m_InverseViewMatrix[0][0] = u.x;
            m_InverseViewMatrix[0][1] = u.y;
            m_InverseViewMatrix[0][2] = u.z;
            
            // Col 2
            m_InverseViewMatrix[1][0] = v.x;
            m_InverseViewMatrix[1][1] = v.y;
            m_InverseViewMatrix[1][2] = v.z;
            
            // Col 3
            m_InverseViewMatrix[2][0] = w.x;
            m_InverseViewMatrix[2][1] = w.y;
            m_InverseViewMatrix[2][2] = w.z;
            
            // Col 4
            m_InverseViewMatrix[3][0] = position.x; // Row 1
            m_InverseViewMatrix[3][1] = position.y; // Row 2
            m_InverseViewMatrix[3][2] = position.z; // Row 3
        }
    }

    // ---------------------------------------------------------------------------------------------------------------------

    void VyCamera::updateFrustum()
    {
        // Extract frustum planes from view-projection matrix.
        Mat4 vp = m_ProjectionMatrix * m_ViewMatrix;

        // Left plane
        m_Frustum.Planes[0] = Vec4(
            vp[0][3] + vp[0][0], 
            vp[1][3] + vp[1][0], 
            vp[2][3] + vp[2][0], 
            vp[3][3] + vp[3][0]
        );

        // Right plane
        m_Frustum.Planes[1] = Vec4(
            vp[0][3] - vp[0][0], 
            vp[1][3] - vp[1][0], 
            vp[2][3] - vp[2][0], 
            vp[3][3] - vp[3][0]
        );
        
        // Bottom plane
        m_Frustum.Planes[2] = Vec4(
            vp[0][3] + vp[0][1], 
            vp[1][3] + vp[1][1], 
            vp[2][3] + vp[2][1], 
            vp[3][3] + vp[3][1]
        );
        
        // Top plane
        m_Frustum.Planes[3] = Vec4(
            vp[0][3] - vp[0][1], 
            vp[1][3] - vp[1][1], 
            vp[2][3] - vp[2][1], 
            vp[3][3] - vp[3][1]
        );
        
        // Near plane
        m_Frustum.Planes[4] = Vec4(
            vp[0][3] + vp[0][2], 
            vp[1][3] + vp[1][2], 
            vp[2][3] + vp[2][2], 
            vp[3][3] + vp[3][2]
        );
        
        // Far plane
        m_Frustum.Planes[5] = Vec4(
            vp[0][3] - vp[0][2], 
            vp[1][3] - vp[1][2], 
            vp[2][3] - vp[2][2], 
            vp[3][3] - vp[3][2]
        );

        // Normalize planes
        for (int i = 0; i < 6; i++)
        {
            float length = glm::length(Vec3(m_Frustum.Planes[i]));

            m_Frustum.Planes[i] /= length;
        }
    }


    bool VyCamera::isInFrustum(const Vec3& center, float radius) const
    {
        // Test sphere against all 6 frustum planes
        for (int i = 0; i < 6; i++)
        {
            float distance = glm::dot(Vec3(m_Frustum.Planes[i]), center) + m_Frustum.Planes[i].w;

            if (distance < -radius)
            {
                return false; // Sphere is completely outside this plane
            }
        }

        return true; // Sphere intersects or is inside frustum
    }



    // void VyCamera::setView(Vec3 position, Quat rotation)
    // {
	// 	Mat4 transformMat{ 1.0f };
	// 	m_InverseViewMatrix = glm::translate(transformMat, position) * glm::toMat4(rotation); // glm::mat4_cast(rotation);
	// 	m_ViewMatrix        = glm::inverse(m_InverseViewMatrix);

    //     // auto t = glm::translate(Mat4(1.0f), position);
    //     // auto r = glm::mat4_cast(rotation);

    //     // auto camera = t * r;

    //     // m_ViewMatrix = glm::affineInverse(camera);
    // }


    // void VyCamera::drawCameraUi() 
    // {
    //     ImGui::Checkbox("Perspective View", &m_IsPerspective);
    //     if (m_IsPerspective) 
    //     {
    //         ImGui::SliderFloat("Vertical FOV (degrees)", &m_FovYDegrees, 1.0f, 120.0f);
	// 	}
	// 	else 
    //     {
	// 		ImGui::DragFloat4("Ortho Params (left, right, top, bottom)", glm::value_ptr(m_OrthoParams), 0.1f);
	// 	}

	// 	ImGui::SliderFloat("Near Plane", &m_zNear, 0.01f, m_zFar - 0.01f);
	// 	ImGui::SliderFloat("Far Plane",  &m_zFar, m_zNear + 0.01f, 1000.0f);
    // }
}


// namespace Vy
// {
//     /**
//      * Column Major
//      * ------------------
//      * [col][row]
//      * 
//      * R = Right
//      * U = Up
//      * F = Forward
//      * P = Position
//      * ------------------
//      * Rx Ux Fx Px
//      * Ry Uy Uy Py
//      * Rz Uz Fz Pz
//      *  0  0  0  1
//      * ------------------
//      * 00 01 02 03 
//      * 10 11 12 13 
//      * 20 21 22 23 
//      * 30 31 32 33 
//      * ------------------
//      */


// 	void EngineCamera::setOrthographicProjection(
// 		F32 left, 
//         F32 right, 
//         F32 top, 
//         F32 bottom, 
//         F32 zNear, 
//         F32 zFar) 
//     {
// 		m_ProjectionMatrix = Mat4{ 1.0f };
//         {
//             m_ProjectionMatrix[0][0] = 2.0f / (right - left);
//             m_ProjectionMatrix[1][1] = 2.0f / (bottom - top);
//             m_ProjectionMatrix[2][2] = 1.0f / (zFar - zNear);
//             m_ProjectionMatrix[3][0] = -(right + left) / (right - left);
//             m_ProjectionMatrix[3][1] = -(bottom + top) / (bottom - top);
//             m_ProjectionMatrix[3][2] = -zNear / (zFar - zNear);
//         }

// 		m_InvProjectionMatrix = glm::inverse(m_ProjectionMatrix);
// 	}


// 	void EngineCamera::setPerspectiveProjection(
//         F32 fovy, 
//         F32 aspect, 
//         F32 zNear /* Near clipping plane */, 
//         F32 zFar  /* Far clipping plane  */) 
//     {
// 		VY_ASSERT(glm::abs(aspect - std::numeric_limits<F32>::epsilon()) > 0.0f);

// 		const F32 TanHalfFovy = tan(fovy / 2.0f);

// 		m_ProjectionMatrix = Mat4{ 0.0f };
//         {
//             m_ProjectionMatrix[0][0] = 1.0f / (aspect * TanHalfFovy);
//             m_ProjectionMatrix[1][1] = 1.0f / (TanHalfFovy);
//             m_ProjectionMatrix[2][2] = zFar / (zFar - zNear);
//             m_ProjectionMatrix[2][3] = 1.0f;
//             m_ProjectionMatrix[3][2] = -(zFar * zNear) / (zFar - zNear);
//         }

// 		m_InvProjectionMatrix = glm::inverse(m_ProjectionMatrix);
// 	}


// 	/**
//      * Set camera view based on the camera's position and direction
//      */
// 	void EngineCamera::setViewDirection(
//         Vec3 position, 
//         Vec3 direction, 
//         Vec3 up) 
//     {
// 		const Vec3 w{ glm::normalize(direction)         };
// 		const Vec3 u{ glm::normalize(glm::cross(w, up)) };
// 		const Vec3 v{ glm::cross(w, u)                  };

// 		m_ViewMatrix = Mat4{ 1.0f };
//         {
//             m_ViewMatrix[0][0] = u.x;
//             m_ViewMatrix[1][0] = u.y;
//             m_ViewMatrix[2][0] = u.z;

//             m_ViewMatrix[0][1] = v.x;
//             m_ViewMatrix[1][1] = v.y;
//             m_ViewMatrix[2][1] = v.z;

//             m_ViewMatrix[0][2] = w.x;
//             m_ViewMatrix[1][2] = w.y;
//             m_ViewMatrix[2][2] = w.z;

//             m_ViewMatrix[3][0] = -glm::dot(u, position);
//             m_ViewMatrix[3][1] = -glm::dot(v, position);
//             m_ViewMatrix[3][2] = -glm::dot(w, position);
//         }

// 		m_InvViewMatrix = Mat4{ 1.0f };
//         {
//             m_InvViewMatrix[0][0] = u.x;
//             m_InvViewMatrix[0][1] = u.y;
//             m_InvViewMatrix[0][2] = u.z;
            
//             m_InvViewMatrix[1][0] = v.x;
//             m_InvViewMatrix[1][1] = v.y;
//             m_InvViewMatrix[1][2] = v.z;
            
//             m_InvViewMatrix[2][0] = w.x;
//             m_InvViewMatrix[2][1] = w.y;
//             m_InvViewMatrix[2][2] = w.z;
            
//             m_InvViewMatrix[3][0] = position.x;
//             m_InvViewMatrix[3][1] = position.y;
//             m_InvViewMatrix[3][2] = position.z;
//         }
// 	}
    

// 	/**
//      * Set camera view based on the camera's position and target
//      */
// 	void EngineCamera::setViewTarget(
//         Vec3 position, 
//         Vec3 target, 
//         Vec3 up) 
//     {
// 		VY_ASSERT((target - position) != Vec3(0), "Direction cannot be 0");
	
//         setViewDirection(position, target - position, up);
// 	}


// 	/**
//      * Set camera view based on the camera's position and rotation
//      */
// 	void EngineCamera::setViewYXZ(
//         Vec3 position, 
//         Vec3 rotation) 
//     {
//         // Z
//         const F32 c3 = glm::cos(rotation.z);
//         const F32 s3 = glm::sin(rotation.z);

//         // X
//         const F32 c2 = glm::cos(rotation.x);
//         const F32 s2 = glm::sin(rotation.x);
        
//         // Y
//         const F32 c1 = glm::cos(rotation.y);
//         const F32 s1 = glm::sin(rotation.y);

//         const Vec3 u{ ( c1 * c3 + s1 * s2 * s3 ), ( c2 * s3 ), ( c1 * s2 * s3 - c3 * s1 ) };
//         const Vec3 v{ ( c3 * s1 * s2 - c1 * s3 ), ( c2 * c3 ), ( c1 * c3 * s2 + s1 * s3 ) };
//         const Vec3 w{ ( c2 * s1                ), (-s2      ), ( c1 * c2                ) };

//         m_ViewMatrix = Mat4{ 1.0f };
//         {
//             // Row 1
//             m_ViewMatrix[0][0] = u.x;
//             m_ViewMatrix[1][0] = u.y;
//             m_ViewMatrix[2][0] = u.z;
            
//             // Row 2
//             m_ViewMatrix[0][1] = v.x;
//             m_ViewMatrix[1][1] = v.y;
//             m_ViewMatrix[2][1] = v.z;
            
//             // Row 3
//             m_ViewMatrix[0][2] = w.x;
//             m_ViewMatrix[1][2] = w.y;
//             m_ViewMatrix[2][2] = w.z;

//             m_ViewMatrix[3][0] = -glm::dot(u, position);
//             m_ViewMatrix[3][1] = -glm::dot(v, position);
//             m_ViewMatrix[3][2] = -glm::dot(w, position);
//         }

// 		m_InvViewMatrix = Mat4{ 1.0f };
//         {
//             m_InvViewMatrix[0][0] = u.x;
//             m_InvViewMatrix[0][1] = u.y;
//             m_InvViewMatrix[0][2] = u.z;

//             m_InvViewMatrix[1][0] = v.x;
//             m_InvViewMatrix[1][1] = v.y;
//             m_InvViewMatrix[1][2] = v.z;
            
//             m_InvViewMatrix[2][0] = w.x;
//             m_InvViewMatrix[2][1] = w.y;
//             m_InvViewMatrix[2][2] = w.z;
            
//             m_InvViewMatrix[3][0] = position.x;
//             m_InvViewMatrix[3][1] = position.y;
//             m_InvViewMatrix[3][2] = position.z;
//         }
// 	}

    
//     void EngineCamera::setViewYXZ(
//         Mat4 transformMatrix) 
//     {
// 		m_InvViewMatrix = transformMatrix;

// 		m_ViewMatrix = Mat4{ 1.0f };

//         // 1 
// 		const Vec3 u =  { 
// 			m_InvViewMatrix[0][0], 
// 			m_InvViewMatrix[0][1], 
// 			m_InvViewMatrix[0][2]  
// 		};

//         // 2 
// 		const Vec3 v = { 
// 			m_InvViewMatrix[1][0],
// 			m_InvViewMatrix[1][1],
// 			m_InvViewMatrix[1][2] 
// 		};

//         // 3 
// 		const Vec3 w = { 
// 			m_InvViewMatrix[2][0],
// 			m_InvViewMatrix[2][1],
// 			m_InvViewMatrix[2][2] 
// 		};

// 		m_ViewMatrix[0][0] = m_InvViewMatrix[0][0];
// 		m_ViewMatrix[1][0] = m_InvViewMatrix[0][1];
// 		m_ViewMatrix[2][0] = m_InvViewMatrix[0][2];
		
//         m_ViewMatrix[0][1] = m_InvViewMatrix[1][0];
// 		m_ViewMatrix[1][1] = m_InvViewMatrix[1][1];
// 		m_ViewMatrix[2][1] = m_InvViewMatrix[1][2];
		
//         m_ViewMatrix[0][2] = m_InvViewMatrix[2][0];
// 		m_ViewMatrix[1][2] = m_InvViewMatrix[2][1];
// 		m_ViewMatrix[2][2] = m_InvViewMatrix[2][2];

// 		m_ViewMatrix[3][0] = -glm::dot(u, { m_InvViewMatrix[3].x, m_InvViewMatrix[3].y, m_InvViewMatrix[3].z });
// 		m_ViewMatrix[3][1] = -glm::dot(v, { m_InvViewMatrix[3].x, m_InvViewMatrix[3].y, m_InvViewMatrix[3].z });
// 		m_ViewMatrix[3][2] = -glm::dot(w, { m_InvViewMatrix[3].x, m_InvViewMatrix[3].y, m_InvViewMatrix[3].z });
// 	}
// }





// namespace Vy
// {
//     // --- Static Factory Methods ---

//     Camera Camera::createPerspective(
//         const Vec3& position,
//         const Vec3& target,
//         F32         fovYDegrees,
//         F32         aspectRatio,
//         F32         nearPlane,
//         F32         farPlane) 
//     {
//         Camera camera;
//         {
//             camera.m_ProjectionType = EProjectionType::PERSPECTIVE;
//             camera.m_Position       = position;
//             camera.m_FovY           = fovYDegrees;
//             camera.m_AspectRatio    = aspectRatio;
//             camera.m_NearPlane      = nearPlane;
//             camera.m_FarPlane       = farPlane;
//         }

//         camera.lookAt(target);
//         camera.updateProjectionMatrix();

//         return camera;
//     }


//     Camera Camera::createOrthographic(
//         const Vec3& position,
//         const Vec3& target,
//         F32         orthoWidth,
//         F32         aspectRatio,
//         F32         nearPlane,
//         F32         farPlane) 
//     {
//         Camera camera;
//         {
//             camera.m_ProjectionType = EProjectionType::ORTHOGRAPHIC;
//             camera.m_Position       = position;
//             camera.m_OrthoWidth     = orthoWidth;
//             camera.m_AspectRatio    = aspectRatio;
//             camera.m_NearPlane      = nearPlane;
//             camera.m_FarPlane       = farPlane;
//         }

//         camera.lookAt(target);
//         camera.updateProjectionMatrix();

//         return camera;
//     }

//     // --- Orientation ---

//     void Camera::lookAt(const Vec3& target, const Vec3& up) 
//     {
//         // Compute forward direction
//         Vec3 forward = glm::normalize(target - m_Position);

//         // Compute right and up vectors
//         Vec3 right    = glm::normalize(glm::cross(forward, up));
//         Vec3 actualUp = glm::cross(right, forward);

//         // Build rotation matrix and convert to quaternion
//         Mat3 rotMatrix;
//         {
//             rotMatrix[0] =  right;
//             rotMatrix[1] =  actualUp;
//             rotMatrix[2] = -forward;  // Vulkan/OpenGL convention: camera looks down -Z
//         }

//         m_Rotation = glm::quat_cast(rotMatrix);

//         updateViewMatrix();
//     }

    
//     void Camera::setEulerAngles(F32 pitch, F32 yaw, F32 roll) 
//     {
//         // Convert degrees to radians
//         F32 pitchRad = glm::radians(pitch);
//         F32 yawRad   = glm::radians(yaw);
//         F32 rollRad  = glm::radians(roll);

//         // Build quaternion from Euler angles using explicit axis-angle rotations
//         // This avoids gimbal lock and ensures proper rotation order
//         // For FPS camera: Yaw (around Y) * Pitch (around X) * Roll (around Z)
//         Quat qYaw   = glm::angleAxis(yawRad,   Vec3(0, 1, 0)); // Rotation around world Y-axis
//         Quat qPitch = glm::angleAxis(pitchRad, Vec3(1, 0, 0)); // Rotation around local X-axis
//         Quat qRoll  = glm::angleAxis(rollRad,  Vec3(0, 0, 1)); // Rotation around local Z-axis

//         m_Rotation = qYaw * qPitch * qRoll;

//         updateViewMatrix();
//     }

    
//     Vec3 Camera::eulerAngles() const 
//     {
//         // Extract Euler angles from forward direction to avoid gimbal lock ambiguity
//         // This gives more stable results for FPS-style cameras

//         Vec3 fwd = forward();

//         // Yaw: rotation around Y-axis (horizontal)
//         F32 yaw = glm::degrees(atan2(fwd.x, -fwd.z));

//         // Pitch: rotation around X-axis (vertical)
//         F32 pitch = glm::degrees(asin(-fwd.y));

//         // Roll: rotation around Z-axis (for FPS cameras, this should stay near 0)
//         // Extract from up vector to determine roll
//         Vec3 upVec = up();
//         Vec3 right = glm::cross(fwd, Vec3(0, 1, 0));
        
//         if (glm::length(right) > 0.001f) 
//         {
//             right = glm::normalize(right);

//             Vec3 expectedUp = glm::cross(right, fwd);
//             F32  roll       = glm::degrees(atan2(glm::dot(upVec, right), glm::dot(upVec, expectedUp)));
            
//             return Vec3(pitch, yaw, roll);
//         }

//         return Vec3(pitch, yaw, 0.0f);
//     }

//     // --- CameraData Population ---

//     CameraData Camera::toCameraData(U32 screenWidth, U32 screenHeight) const 
//     {
//         CameraData data;
//         {
//             // Matrices
//             data.View           = m_ViewMatrix;
//             data.Projection     = m_ProjectionMatrix;
//             data.ViewProjection = m_ProjectionMatrix * m_ViewMatrix;
//             data.InvView        = glm::inverse(m_ViewMatrix);
//             data.InvProjection  = glm::inverse(m_ProjectionMatrix);
            
//             // Position and direction
//             data.Position  = Vec4(m_Position, 1.0f);
//             data.Direction = Vec4(forward(), 0.0f);
            
//             // Parameters
//             data.NearFar     = Vec2(m_NearPlane, m_FarPlane);
//             data.ScreenSize  = Vec2(static_cast<F32>(screenWidth), static_cast<F32>(screenHeight));
//             data.FOV         = m_FovY;
//             data.AspectRatio = m_AspectRatio;
//         }

//         return data;
//     }

//     // --- Internal Update Methods ---

//     void Camera::updateViewMatrix() 
//     {
//         // Build view matrix from position and rotation.
//         // m_Rotation represents the camera orientation from local->world space.
//         // View matrix is the inverse of the camera's world transform:
//         //   view = R^T * T^{-1}
//         Mat4 invRotation    = glm::mat4_cast(glm::conjugate(m_Rotation));
//         Mat4 invTranslation = glm::translate(Mat4(1.0f), -m_Position);
        
//         m_ViewMatrix = invRotation * invTranslation;
//     }

    
//     void Camera::updateProjectionMatrix() 
//     {
//         if (m_ProjectionType == EProjectionType::PERSPECTIVE) 
//         {
//             // Perspective projection
//             m_ProjectionMatrix = glm::perspective(
//                 glm::radians(m_FovY),
//                 m_AspectRatio,
//                 m_NearPlane,
//                 m_FarPlane
//             );
//         } 
//         else 
//         {
//             // Orthographic projection
//             F32 orthoHeight = m_OrthoWidth / m_AspectRatio;

//             m_ProjectionMatrix = glm::ortho(
//                 -m_OrthoWidth * 0.5f, m_OrthoWidth * 0.5f,
//                 -orthoHeight  * 0.5f, orthoHeight  * 0.5f,
//                 m_NearPlane,
//                 m_FarPlane
//             );
//         }

//         // GLM was designed for OpenGL, where Y coordinate is flipped
//         // For Vulkan, we need to flip the Y axis in the projection matrix
//         m_ProjectionMatrix[1][1] *= -1.0f;
//     }
// }




