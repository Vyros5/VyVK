#pragma once


#include <VyLib/VyLib.h>

// #include <glm/gtx/matrix_decompose.inl>

namespace Vy::Math
{
    namespace World
    {
        constexpr Vec3 FORWARD  = {  0.0f,  0.0f, -1.0f }; // -z
        constexpr Vec3 RIGHT    = {  1.0f,  0.0f,  0.0f }; // +x
        constexpr Vec3 UP       = {  0.0f, -1.0f,  0.0f }; // +y
    }
	
    constexpr Vec3 UNIT_VECTOR_X{ 1.0f, 0.0f, 0.0f };
    constexpr Vec3 UNIT_VECTOR_Y{ 0.0f, 1.0f, 0.0f };
    constexpr Vec3 UNIT_VECTOR_Z{ 0.0f, 0.0f, 1.0f };

    constexpr Mat4 IDENTITY_MAT4{ Mat4( 1.0f ) };


	/// @brief Returns the forward direction of a rotation
	Vec3 forward(const Vec3& rotation);
	Vec3 forward(const Quat& rotation);

	/// @brief Returns the right direction of a rotation
	Vec3 right(const Vec3& rotation);
	Vec3 right(const Quat& rotation);

	/// @brief Returns the up direction of a rotation
	Vec3 up(const Vec3& rotation);
	Vec3 up(const Quat& rotation);

	/// @brief Composes a transformation matrix from location, rotation and scale
	Mat4 tranformationMatrix(const Vec3& location, const Vec3& rotation, const Vec3& scale);
	Mat4 tranformationMatrix(const Vec3& location, const Quat& rotation, const Vec3& scale);

	/// @brief Composes a matrix to transform normals into world space
	Mat3 normalMatrix(const Vec3& rotation, const Vec3& scale);
	Mat3 normalMatrix(const Quat& rotation, const Vec3& scale);

	/// @brief Decomposes a transformation matrix into location, rotation and scale
	void decomposeTRS(const Mat4& matrix, Vec3& translation, Vec3& rotation, Vec3& scale);
	void decomposeTransform(const Mat4& matrix, Vec3& translation, Vec3& rotation, Vec3& scale);

	void decomposeTRS(const Mat4& matrix, Vec3& translation, Quat& rotation, Vec3& scale);

	/// @brief Returns the percentage of a value between a min and max value
	float percentage(float value, float min, float max);

	/// @brief Checks if the target is in the field of view of the view direction
	bool inFOV(const Vec3& viewDirection, const Vec3& targetDirection, float fov);


//     VY_NODISCARD
//     inline bool DecomposeTransform(const Mat4& transform, Vec3& translation, Vec3& rotation, Vec3& scale) 
// 	{
//         // From glm::decompose in matrix_decompose.inl

//         using namespace glm;
//         using T = float;

//         Mat4 LocalMatrix( transform );

//         // Normalize the matrix.
//         if (glm::epsilonEqual(LocalMatrix[3][3], static_cast<float>(0), glm::epsilon<T>()))
//             return false;

//         // First, isolate perspective.  This is the messiest.
//         if (
//                 glm::epsilonNotEqual( LocalMatrix[0][3], static_cast<T>( 0 ), glm::epsilon<T>()) ||
//                 glm::epsilonNotEqual( LocalMatrix[1][3], static_cast<T>( 0 ), glm::epsilon<T>()) ||
//                 glm::epsilonNotEqual( LocalMatrix[2][3], static_cast<T>( 0 ), glm::epsilon<T>())) 
// 		{
//             // Clear the perspective partition
//             LocalMatrix[0][3] = LocalMatrix[1][3] = LocalMatrix[2][3] = static_cast<T>( 0 );
//             LocalMatrix[3][3] = static_cast<T>(1);
//         }

//         // Next take care of translation (easy).
//         translation = Vec3(LocalMatrix[3]);
//         LocalMatrix[3] = Vec4( 0, 0, 0, LocalMatrix[3].w );

//         Vec3 Row[3], Pdum3;

//         // Now get scale and shear.
//         for (glm::length_t i = 0; i < 3; ++i)
// 		{
// 			for ( glm::length_t j = 0; j < 3; ++j )
// 			{
// 				Row[i][j] = LocalMatrix[i][j];
// 			}
// 		}

//         // Compute X scale factor and normalize first row.
//         scale.x = glm::length( Row[0] );
//         Row[0]  = glm::detail::scale( Row[0], static_cast<T>( 1 ) );
//         scale.y = glm::length( Row[1] );
//         Row[1]  = glm::detail::scale( Row[1], static_cast<T>( 1 ) );
//         scale.z = glm::length( Row[2] );
//         Row[2]  = glm::detail::scale( Row[2], static_cast<T>( 1 ) );

//         // At this point, the matrix (in rows[]) is orthonormal.
//         // Check for a coordinate system flip.  If the determinant
//         // is -1, then negate the matrix and the scaling factors.
// #if 0
// 		Pdum3 = cross(Row[1], Row[2]); // v3Cross(row[1], row[2], Pdum3);
// 		if (dot(Row[0], Pdum3) < 0)
// 		{
// 			for (length_t i = 0; i < 3; i++)
// 			{
// 				scale[i] *= static_cast<T>(-1);
// 				Row[i] *= static_cast<T>(-1);
// 			}
// 		}
// #endif

//         rotation.y = asin( -Row[0][2] );
//         if ( cos( rotation.y ) != 0 ) {
//             rotation.x = atan2( Row[1][2], Row[2][2] );
//             rotation.z = atan2( Row[0][1], Row[0][0] );
//         } else {
//             rotation.x = atan2( -Row[2][0], Row[1][1] );
//             rotation.z = 0;
//         }

//         return true;
//     }


//     /**
//      * Computes the model matrix as in Translate * Ry * Rx * Rz * Scale (where R represents a
//      * rotation in the desired axis. Rotation convention uses Tait-Bryan angles with axis order
//      * Y(1), X(2), Z(3)
//      * */
//     VY_NODISCARD
//     inline auto RecomputeTransform( const Vec3& position, const Vec3& size, const Vec3& angles = Vec3( 0.0f ) ) -> glm::mat4 
// 	{
//         // Compute scale matrix
//         const glm::mat4 scale{ glm::scale( IDENTITY_MAT4, size ) };

//         // Compute rotation matrix
//         glm::mat4 rotation{ glm::rotate( IDENTITY_MAT4, ( float )glm::radians( angles.y ), UNIT_VECTOR_Y ) };
//         rotation = glm::rotate( rotation, ( float )glm::radians( angles.x ), UNIT_VECTOR_X );
//         rotation = glm::rotate( rotation, ( float )glm::radians( angles.z ), UNIT_VECTOR_Z );

//         return glm::translate( IDENTITY_MAT4, position ) * rotation * scale;
//     }
}