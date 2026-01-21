#include <VyEngine/Scene/Camera.h>

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
                // Sphere is completely outside this plane
                return false; 
            }
        }

        // Sphere intersects or is inside frustum
        return true; 
    }
}