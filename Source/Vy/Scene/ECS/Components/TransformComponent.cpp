#include <Vy/Scene/ECS/Components/TransformComponent.h>

#include <glm/gtx/matrix_decompose.hpp>

namespace Vy
{
    struct MatrixCalculator 
    {
        // https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix

        // Matrix corresponds to translate * Rz * Rx * Ry * scale transformation
        // Rotation convention = ZXY tait-bryan angles.
        static inline Mat4 ZXY(TransformComponent& transform) 
        {
            const float c3 = glm::cos(transform.Rotation.z);
            const float s3 = glm::sin(transform.Rotation.z);

            const float c2 = glm::cos(transform.Rotation.x);
            const float s2 = glm::sin(transform.Rotation.x);
            
            const float c1 = glm::cos(transform.Rotation.y);
            const float s1 = glm::sin(transform.Rotation.y);

            return Mat4(
                {
                    transform.Scale.x * (-c2 * s1 ),
                    transform.Scale.x * ( c1 * c2 ),
                    transform.Scale.x * ( s2      ),
                    0.0f,
                },
                {
                    transform.Scale.y * ( c1 * s3 + c3 * s1 * s2 ),
                    transform.Scale.y * ( s1 * s3 - c1 * c3 * s2 ),
                    transform.Scale.y * ( c2 * c3                ),
                    0.0f,
                },
                {
                    transform.Scale.z * ( c1 * c3 - s1 * s2 * s3 ),
                    transform.Scale.z * ( c3 * s1 + c1 * s2 * s3 ),
                    transform.Scale.z * (-c2 * s3                ),
                    0.0f,
                },
                { 
                    transform.Translation.x, 
                    transform.Translation.y, 
                    transform.Translation.z, 
                    1.0f 
                }
            );
        }
        
        // Matrix corresponds to translate * Ry * Rx * Rz * scale transformation
        // Rotation convention = YXZ tait-bryan angles.
        static inline Mat4 YXZ(TransformComponent& transform) 
        {
            const float c3 = glm::cos(transform.Rotation.z);
            const float s3 = glm::sin(transform.Rotation.z);

            const float c2 = glm::cos(transform.Rotation.x);
            const float s2 = glm::sin(transform.Rotation.x);
            
            const float c1 = glm::cos(transform.Rotation.y);
            const float s1 = glm::sin(transform.Rotation.y);

            return Mat4(
                {
                    transform.Scale.x * ( c1 * c3 + s1 * s2 * s3 ),
                    transform.Scale.x * ( c2 * s3                ),
                    transform.Scale.x * ( c1 * s2 * s3 - c3 * s1 ),
                    0.0f,
                },
                {
                    transform.Scale.y * ( c3 * s1 * s2 - c1 * s3 ),
                    transform.Scale.y * ( c2 * c3                ),
                    transform.Scale.y * ( c1 * c3 * s2 + s1 * s3 ),
                    0.0f,
                },
                {
                    transform.Scale.z * ( c2 * s1 ),
                    transform.Scale.z * (-s2      ),
                    transform.Scale.z * ( c1 * c2 ),
                    0.0f,
                },
                { 
                    transform.Translation.x, 
                    transform.Translation.y, 
                    transform.Translation.z, 
                    1.0f 
                }
            );
        }
    };


    Mat4 TransformComponent::matrix() 
    {
        // return MatrixCalculator::YXZ( *this );

        const float c3 = glm::cos(Rotation.z);
        const float s3 = glm::sin(Rotation.z);

        const float c2 = glm::cos(Rotation.x);
        const float s2 = glm::sin(Rotation.x);
        
        const float c1 = glm::cos(Rotation.y);
        const float s1 = glm::sin(Rotation.y);

        return Mat4{
            {
                Scale.x * ( c1 * c3 + s1 * s2 * s3 ),
                Scale.x * ( c2 * s3                ),
                Scale.x * ( c1 * s2 * s3 - c3 * s1 ),
                0.0f,
            },
            {
                Scale.y * ( c3 * s1 * s2 - c1 * s3 ),
                Scale.y * ( c2 * c3                ),
                Scale.y * ( c1 * c3 * s2 + s1 * s3 ),
                0.0f,
            },
            {
                Scale.z * ( c2 * s1 ),
                Scale.z * (-s2      ),
                Scale.z * ( c1 * c2 ),
                0.0f,
            },
            { 
                Translation.x, 
                Translation.y, 
                Translation.z, 
                1.0f 
            }
        };
    }


    Mat3 TransformComponent::normalMatrix() 
    {
        const float c3 = glm::cos(Rotation.z);
        const float s3 = glm::sin(Rotation.z);

        const float c2 = glm::cos(Rotation.x);
        const float s2 = glm::sin(Rotation.x);
        
        const float c1 = glm::cos(Rotation.y);
        const float s1 = glm::sin(Rotation.y);

        const Vec3 invScale = 1.0f / Scale;

        return Mat3{
            {
                invScale.x * ( c1 * c3 + s1 * s2 * s3 ),
                invScale.x * ( c2 * s3                ),
                invScale.x * ( c1 * s2 * s3 - c3 * s1 ),
            },
            {
                invScale.y * ( c3 * s1 * s2 - c1 * s3 ),
                invScale.y * ( c2 * c3                ),
                invScale.y * ( c1 * c3 * s2 + s1 * s3 ),
            },
            {
                invScale.z * ( c2 * s1 ),
                invScale.z * (-s2      ),
                invScale.z * ( c1 * c2 ),
            }, 
//             {
//                 0.0f, 
//                 0.0f, 
//                 0.0f, 
//                 0.0f
//             }
        };
    }
}



#if 0
    void TransformComponent::decompose(const Mat4& transform, Vec3* outTranslation, Vec3* outRotationEulerXYZ, Vec3* outScale) 
    {
        // From glm::decompose in matrix_decompose.inl

        // using namespace glm;
        using T = float;

        Mat4 LocalMatrix(transform);

        if (glm::epsilonEqual(LocalMatrix[3][3], static_cast<float>(0), glm::epsilon<T>()))
            return;

        // First, isolate perspective.  This is the messiest.
        if (
            glm::epsilonNotEqual(LocalMatrix[0][3], static_cast<T>(0), glm::epsilon<T>()) ||
            glm::epsilonNotEqual(LocalMatrix[1][3], static_cast<T>(0), glm::epsilon<T>()) ||
            glm::epsilonNotEqual(LocalMatrix[2][3], static_cast<T>(0), glm::epsilon<T>()))
        {
            // Clear the perspective partition
            LocalMatrix[0][3] = LocalMatrix[1][3] = LocalMatrix[2][3] = static_cast<T>(0);
            LocalMatrix[3][3] = static_cast<T>(1);
        }

        // Next take care of translation (easy).
        *outTranslation = Vec3(LocalMatrix[3]);
        LocalMatrix[3]  = Vec4(0, 0, 0, LocalMatrix[3].w);

        Vec3 Row[3], Pdum3;

        // Now get scale and shear.
        for (glm::length_t i = 0; i < 3; ++i)
        {
            for (glm::length_t j = 0; j < 3; ++j)
            {
                Row[i][j] = LocalMatrix[i][j];
            }
        }

        // Compute X scale factor and normalize first row.
        outScale->x = glm::length(Row[0]);
        Row[0]      = glm::detail::scale(Row[0], static_cast<T>(1));

        outScale->y = glm::length(Row[1]);
        Row[1]      = glm::detail::scale(Row[1], static_cast<T>(1));
        
        outScale->z = glm::length(Row[2]);
        Row[2]      = glm::detail::scale(Row[2], static_cast<T>(1));

        // At this point, the matrix (in rows[]) is orthonormal.
        // Check for a coordinate system flip.  If the determinant
        // is -1, then negate the matrix and the scaling factors.
#if 0
        Pdum3 = glm::cross(Row[1], Row[2]); // v3Cross(row[1], row[2], Pdum3);
        if (glm::dot(Row[0], Pdum3) < 0)
        {
            for (glm::length_t i = 0; i < 3; i++)
            {
                outScale[i] *= static_cast<T>(-1);
                Row[i]      *= static_cast<T>(-1);
            }
        }
#endif

#if 0
        outRotationEulerXYZ->y = glm::asin(-Row[0][2]);

        if (glm::cos(outRotationEulerXYZ->y) != 0) 
        {
            outRotationEulerXYZ->x = glm::atan2( Row[1][2], Row[2][2]);
            outRotationEulerXYZ->z = glm::atan2( Row[0][1], Row[0][0]);
        }
        else {
            outRotationEulerXYZ->x = glm::atan2(-Row[2][0], Row[1][1]);
            outRotationEulerXYZ->z = 0;
        }
#endif

        outRotationEulerXYZ->y = glm::asin(-Row[1][2]);

        if (glm::cos(outRotationEulerXYZ->y) != 0) 
        {
            outRotationEulerXYZ->x = glm::atan2( Row[0][2], Row[2][2]);
            outRotationEulerXYZ->z = glm::atan2( Row[1][0], Row[1][1]);
        }
        else {
            outRotationEulerXYZ->x = glm::atan2(-Row[2][1], Row[0][0]);
            outRotationEulerXYZ->z = 0;
        }

    }
#endif
