#pragma once

#include <VyLib/VyLib.h>
#include <Vy/Math/Math.h>

namespace Vy
{
	struct TransformComponent 
	{
		Vec3 Translation{ 0.0f, 0.0f, 0.0f };
		Vec3 Scale      { 1.0f, 1.0f, 1.0f };
		Vec3 Rotation   { 0.0f, 0.0f, 0.0f };

		/**
		 * @brief Transforms the entity's position, scale, and rotation into a 4x4 matrix
		 *
		 * Matrix corrsponds to Translate * Ry * Rx * Rz * Scale
		 * Rotations correspond to Tait-Bryan angles of Y(1), X(2), Z(3)
		 *
		 * To view the rotation as extrinsic, just read the operations from right to left
		 * Otherwise, to view the rotation as intrinsic, read the operations from left to right
		 *
		 * - Extrinsic: Z(world) -> X(world) -> Y(world)
		 *
		 * - Intrinsic: Y(local) -> X(local) -> Z(local)
		 *
		 * @note https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
		 *
		 * @return Mat4
		 */
		Mat4 matrix();

		Mat3 normalMatrix();

		inline Vec3 forward() const
			{
			return Vec3{
				 glm::sin(Rotation.y) * glm::cos(Rotation.x),
				-glm::sin(Rotation.x),
				 glm::cos(Rotation.y) * glm::cos(Rotation.x),
			};
		}

		inline Vec3 right() const
			{
			return Vec3{
					 glm::cos(Rotation.y),
					 0.0f,
					-glm::sin(Rotation.y),
			};
		}

		// Set rotation to look at a target point (in world space)
		inline void lookAt(const Vec3& target)
		{
			Vec3 direction = glm::normalize(target - Translation);

			// Calculate yaw (rotation around Y axis)
			Rotation.y = std::atan2(direction.x, direction.z);

			// Calculate pitch (rotation around X axis)
			Rotation.x = -std::asin(direction.y);

			// Keep roll at 0
			Rotation.z = 0.0f;
		}


		TransformComponent()                          = default;
		TransformComponent(const TransformComponent&) = default;

		TransformComponent(
			const Vec3& translation
		) : 
			Translation{ translation } 
		{
		}

		TransformComponent(
			const Vec3& translation, 
			const Vec3& scale
		) : 
			Translation{ translation }, 
			Scale      { scale       } 
		{
		}

		TransformComponent(
			const Vec3& translation, 
			const Vec3& scale, 
			const Vec3& rotation
		) : 
			Translation{ translation }, 
			Scale      { scale       }, 
			Rotation   { rotation    }
		{
		}

		// Conversion operator calling the matrix function
		operator Mat4() 
		{ 
			return matrix(); 
		}

		friend class MatrixCalculator;
	};
}