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


	// struct TransformComponent 
	// {
	// 	void setTranslation(Vec3 pos) { Dirty = true; Position = Vec3(pos.x, pos.z, pos.y); }
	// 	void setRotation(Vec3 rot)    { Dirty = true; Rotation = Vec3(rot.x, rot.z, rot.y); }
	// 	void setScale(Vec3 scl)       { Dirty = true; Scale    = Vec3(scl.x, scl.z, scl.y); }

	// 	void setTranslationX(float val) { Dirty = true; Position.x = val; }
	// 	void setTranslationY(float val) { Dirty = true; Position.z = val; }
	// 	void setTranslationZ(float val) { Dirty = true; Position.y = val; }

	// 	void setRotationX(float val) { Dirty = true; Rotation.x = val; }
	// 	void setRotationY(float val) { Dirty = true; Rotation.z = val; }
	// 	void setRotationZ(float val) { Dirty = true; Rotation.y = val; }
		
	// 	void setScaleX(float val) { Dirty = true; Scale.x = val; }
	// 	void setScaleY(float val) { Dirty = true; Scale.z = val; }
	// 	void setScaleZ(float val) { Dirty = true; Scale.y = val; }

	// 	Vec3 postion()  { return Vec3(Position.x, Position.z, Position.y); }
	// 	Vec3 rotation() { return Vec3(Rotation.x, Rotation.z, Rotation.y); }
	// 	Vec3 scale()    { return Vec3(Scale.x,    Scale.z,    Scale.y);    }

	// 	Mat4 matrix()       const { return TransformMatrix; }
	// 	Mat3 normalMatrix() const { return NormalMatrix;    }

	// 	static void decompose(const Mat4& transform, Vec3* outTranslation, Vec3* outRotationEulerXYZ, Vec3* outScale);

	// 	// TransformComponent() = default;
	// 	// TransformComponent(const TransformComponent&) = default;

	// 	Mat4 TransformMatrix{ 1.0f };
	// 	Mat3 NormalMatrix   { 1.0f };

	// 	Mat4 calculateMatrix();
	// 	Mat3 calculateNormalMatrix();

	// 	void setGlobalTranslationDirect(Vec3 pos) 
	// 	{
	// 		Position        = Vec3(pos.x, pos.z, pos.y); 

	// 		TransformMatrix = calculateMatrix(); 
	// 		NormalMatrix    = calculateNormalMatrix();
	// 	}

	// 	void setGlobalRotationDirect(Vec3 rot) 
	// 	{
	// 		Rotation = Vec3(rot.x, rot.z, rot.y);
	// 	}

	// 	void setGlobalScaleDirect(Vec3 scl) 
	// 	{
	// 		Scale = Vec3(scl.x, scl.z, scl.y);
	// 	}

	// 	void recalculateMatrix() 
	// 	{
	// 		TransformMatrix = calculateMatrix();
	// 		NormalMatrix    = calculateNormalMatrix();
	// 	}

	// 	bool Dirty = true;
	// // private:
	// 	Vec3 Position{ 0.0f, 0.0f, 0.0f };
	// 	Vec3 Rotation{ 0.0f, 0.0f, 0.0f };
	// 	Vec3 Scale   { 1.0f, 1.0f, 1.0f };

	// 	friend struct MatrixCalculator;
	// };










    // /**
    //  * @brief Stores the local transformation of an Entity.
    //  * 
    //  * @note Required Component
    //  */
	// struct TransformComponent 
    // {
	// 	Vec3 Position{ 0.0f, 0.0f, 0.0f       };
	// 	Quat Rotation{ 1.0f, 0.0f, 0.0f, 0.0f };  // Identity quaternion
	// 	Vec3 Scale   { 1.0f, 1.0f, 1.0f       };

	// 	/**
	// 	 * @brief Transforms the entity's position, scale, and rotation into a 4x4 matrix
	// 	 *
	// 	 * Matrix corrsponds to Translate * Ry * Rx * Rz * Scale
	// 	 * Rotations correspond to Tait-Bryan angles of Y(1), X(2), Z(3)
	// 	 *
	// 	 * To view the rotation as extrinsic, just read the operations from right to left
	// 	 * Otherwise, to view the rotation as intrinsic, read the operations from left to right
	// 	 *
	// 	 * - Extrinsic: Z(world) -> X(world) -> Y(world)
	// 	 *
	// 	 * - Intrinsic: Y(local) -> X(local) -> Z(local)
	// 	 *
	// 	 * @note https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
	// 	 *
	// 	 * @return Mat4
	// 	 */
	// 	Mat4 matrix() const 
	// 	{ 
	// 		return Mat4 {
	// 			{
	// 				Scale.x * (1 - 2 * (Rotation.y * Rotation.y + Rotation.z * Rotation.z)),
	// 				Scale.x * (2 *     (Rotation.x * Rotation.y + Rotation.w * Rotation.z)),
	// 				Scale.x * (2 *     (Rotation.x * Rotation.z - Rotation.w * Rotation.y)),
	// 				0.0f
	// 			},
	// 			{
	// 				Scale.y * (2 *     (Rotation.x * Rotation.y - Rotation.w * Rotation.z)),
	// 				Scale.y * (1 - 2 * (Rotation.x * Rotation.x + Rotation.z * Rotation.z)),
	// 				Scale.y * (2 *     (Rotation.y * Rotation.z + Rotation.w * Rotation.x)),
	// 				0.0f
	// 			},
	// 			{
	// 				Scale.z * (2 *     (Rotation.x * Rotation.z + Rotation.w * Rotation.y)),
	// 				Scale.z * (2 *     (Rotation.y * Rotation.z - Rotation.w * Rotation.x)),
	// 				Scale.z * (1 - 2 * (Rotation.y * Rotation.y + Rotation.x * Rotation.x)),
	// 				0.0f,
	// 			},
	// 			{ 
	// 				Position.x, 
	// 				Position.y, 
	// 				Position.z, 
	// 				1.0f 
	// 			}
	// 		};

	// 		// return Math::tranformationMatrix(Position, Rotation, Scale); 
			
	// 		// const float c3 = glm::cos(Rotation.z);
	// 		// const float s3 = glm::sin(Rotation.z);
	// 		// const float c2 = glm::cos(Rotation.x);
	// 		// const float s2 = glm::sin(Rotation.x);
	// 		// const float c1 = glm::cos(Rotation.y);
	// 		// const float s1 = glm::sin(Rotation.y);

	// 		// return Mat4{
	// 		// 	{
	// 		// 		Scale.x * (c1 * c3 + s1 * s2 * s3),
	// 		// 		Scale.x * (c2 * s3),
	// 		// 		Scale.x * (c1 * s2 * s3 - c3 * s1),
	// 		// 		0.0f,
	// 		// 	},
	// 		// 	{
	// 		// 		Scale.y * (c3 * s1 * s2 - c1 * s3),
	// 		// 		Scale.y * (c2 * c3),
	// 		// 		Scale.y * (c1 * c3 * s2 + s1 * s3),
	// 		// 		0.0f,
	// 		// 	},
	// 		// 	{
	// 		// 		Scale.z * (c2 * s1),
	// 		// 		Scale.z * (-s2),
	// 		// 		Scale.z * (c1 * c2),
	// 		// 		0.0f,
	// 		// 	},
	// 		// 	{
	// 		// 		Position.x, 
	// 		// 		Position.y, 
	// 		// 		Position.z, 
	// 		// 		1.0f
	// 		// 	}
	// 		// };
	// 	}
		
	// 	Mat4 normalMatrix() const
	// 	{
	// 		const Vec3 inverseScale = 1.0f / Scale;

	// 		return Mat4 {
	// 			{
	// 				inverseScale.x * (1 - 2 * (Rotation.y * Rotation.y + Rotation.z * Rotation.z)),
	// 				inverseScale.x * (2 *     (Rotation.x * Rotation.y + Rotation.w * Rotation.z)),
	// 				inverseScale.x * (2 *     (Rotation.x * Rotation.z - Rotation.w * Rotation.y)),
	// 				0.0f,
	// 			},
	// 			{
	// 				inverseScale.y * (2 *     (Rotation.x * Rotation.y - Rotation.w * Rotation.z)),
	// 				inverseScale.y * (1 - 2 * (Rotation.x * Rotation.x + Rotation.z * Rotation.z)),
	// 				inverseScale.y * (2 *     (Rotation.y * Rotation.z + Rotation.w * Rotation.x)),
	// 				0.0f,
	// 			},
	// 			{
	// 				inverseScale.z * (2 *     (Rotation.x * Rotation.z + Rotation.w * Rotation.y)),
	// 				inverseScale.z * (2 *     (Rotation.y * Rotation.z - Rotation.w * Rotation.x)),
	// 				inverseScale.z * (1 - 2 * (Rotation.x * Rotation.x + Rotation.y * Rotation.y)),
	// 				0.0f,
	// 			},
	// 			{ 
	// 				0.0f, 
	// 				0.0f, 
	// 				0.0f, 
	// 				0.0f 
	// 			}
	// 		};
	// 	}


	// 	// Mat3 normalMatrix()  
	// 	// { 
	// 		// return Math::normalMatrix(Rotation, Scale); 

	// 		// const float c3 = glm::cos(Rotation.z);
	// 		// const float s3 = glm::sin(Rotation.z);
	// 		// const float c2 = glm::cos(Rotation.x);
	// 		// const float s2 = glm::sin(Rotation.x);
	// 		// const float c1 = glm::cos(Rotation.y);
	// 		// const float s1 = glm::sin(Rotation.y);

	// 		// const Vec3 invScale = 1.0f / Scale;
			
	// 		// return Mat3{
	// 		// 	{
	// 		// 		invScale.x * (c1 * c3 + s1 * s2 * s3),
	// 		// 		invScale.x * (c2 * s3),
	// 		// 		invScale.x * (c1 * s2 * s3 - c3 * s1),
	// 		// 	},
	// 		// 	{
	// 		// 		invScale.y * (c3 * s1 * s2 - c1 * s3),
	// 		// 		invScale.y * (c2 * c3),
	// 		// 		invScale.y * (c1 * c3 * s2 + s1 * s3),
	// 		// 	},
	// 		// 	{
	// 		// 		invScale.z * (c2 * s1),
	// 		// 		invScale.z * (-s2),
	// 		// 		invScale.z * (c1 * c2),
	// 		// 	}
	// 		// };
	// 	// }

	// 	Vec3 forward() const { return Math::forward(Rotation); }
	// 	Vec3 right()   const { return Math::right(Rotation);   }
	// 	Vec3 up()      const { return Math::up(Rotation);      }
	// };


	// /**
	//  * LocalToWorldComponent - Computed transform matrix
	//  *
	//  * Automatically computed by TransformSystem from TransformComponent.
	//  * Used by rendering systems to build push constants.
	//  *
	//  * Separation: TransformComponent has raw data (pos, rot, scale),
	//  * LocalToWorldComponent has computed matrix (for rendering).
	//  */
	// struct LocalToWorldComponent
	// {
	// 	Mat4 Matrix      { 1.0f }; // Local-to-world transform
	// 	Mat4 NormalMatrix{ 1.0f }; // For transforming normals (inverse transpose)

	// 	constexpr LocalToWorldComponent() = default;
		
	// 	explicit constexpr LocalToWorldComponent(const Mat4& m, const Mat4& n = Mat4(1.0f)) noexcept : 
	// 		Matrix(m), 
	// 		NormalMatrix(n) 
	// 	{
	// 	}
	// };


    // // /**
    // //  * @brief Stores the local transformation of an Entity in world space (including parent transforms).
    // //  * 
    // //  * @note Required Component
    // //  */
	// // struct GlobalTransformComponent 
    // // {
	// // 	Vec3 Position{ 0.0f, 0.0f, 0.0f };
	// // 	Quat Rotation{ 1.0f, 0.0f, 0.0f, 0.0f };  // Identity quaternion
	// // 	Vec3 Scale   { 1.0f, 1.0f, 1.0f };

	// // 	Mat4 matrix()  const { return Math::tranformationMatrix(Position, Rotation, Scale); }

	// // 	Vec3 forward() const { return Math::forward(Rotation); }
	// // 	Vec3 right()   const { return Math::right(Rotation);   }
	// // 	Vec3 up()      const { return Math::up(Rotation);      }
	// // };


	// struct TransformComponent2
    // {
	// 	Vec3 Position{ 0.0f };
	// 	Vec3 Rotation{ 0.0f }; 
	// 	Vec3 Scale   { 1.0f };

	// 	/**
	// 	 * @brief Transforms the entity's position, scale, and rotation into a 4x4 matrix
	// 	 *
	// 	 * Matrix corrsponds to Translate * Ry * Rx * Rz * Scale
	// 	 * Rotations correspond to Tait-Bryan angles of Y(1), X(2), Z(3)
	// 	 *
	// 	 * To view the rotation as extrinsic, just read the operations from right to left
	// 	 * Otherwise, to view the rotation as intrinsic, read the operations from left to right
	// 	 *
	// 	 * - Extrinsic: Z(world) -> X(world) -> Y(world)
	// 	 *
	// 	 * - Intrinsic: Y(local) -> X(local) -> Z(local)
	// 	 *
	// 	 * @note https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
	// 	 *
	// 	 * @return Mat4
	// 	 */

	// 	/**
	// 	 * @brief 
	// 	 * 
	// 	 * Matrix corrsponds to Translate * Ry * Rx * Rz * Scale
	// 	 * 
	// 	 * Rotations correspond to Tait-Bryan angles of axis order: Y(1), X(2), Z(3)
	// 	 * 
	// 	 * R = Y X Z
	// 	 * 
	// 	 * To interpret as an extrinsic rotation: read from right to left
	// 	 *  - Extrinsic: Z(world) -> X(world) -> Y(world)
	// 	 * 
	// 	 * To interpret as an intrinsic rotation: read from left to right
	// 	 * - Intrinsic: Y(local) -> X(local) -> Z(local)
	// 	 * 
	// 	 * @return Mat4 
	// 	 */
	// 	Mat4 matrix()  const 
	// 	{ 
	// 		// return Math::tranformationMatrix(Position, Rotation, Scale); 
	// 		const float c3 = glm::cos(Rotation.z);
	// 		const float s3 = glm::sin(Rotation.z);

	// 		const float c2 = glm::cos(Rotation.x);
	// 		const float s2 = glm::sin(Rotation.x);
			
	// 		const float c1 = glm::cos(Rotation.y);
	// 		const float s1 = glm::sin(Rotation.y);

	// 		return Mat4{
	// 			{ // Col 1
	// 				Scale.x * (c1 * c3 + s1 * s2 * s3), // a
	// 				Scale.x * (c2 * s3),                // d
	// 				Scale.x * (c1 * s2 * s3 - c3 * s1), // g
	// 				0.0f,                               // 0
	// 			},
	// 			{ // Col 2
	// 				Scale.y * (c3 * s1 * s2 - c1 * s3), // b
	// 				Scale.y * (c2 * c3),                // e
	// 				Scale.y * (c1 * c3 * s2 + s1 * s3), // h
	// 				0.0f,                               // 0
	// 			},
	// 			{ // Col 3
	// 				Scale.z * (c2 * s1),                // c
	// 				Scale.z * (-s2),                    // f
	// 				Scale.z * (c1 * c2),                // i
	// 				0.0f,                               // 0
	// 			},
	// 			{ // Col 4
	// 				Position.x,                         // tx
	// 				Position.y,                         // ty
	// 				Position.z,                         // tz
	// 				1.0f                                // 1
	// 			}
	// 		};
	// 	}
		
	// 	/**
	// 	 * @brief 
	// 	 * 
	// 	 * modelMatrix ^-1 ^T = (TRS)^-1^T. T = translation, R = rotation, S = scale
	// 	 * 
	// 	 * normals are unaffected by translations
	// 	 * 
	// 	 * 
	// 	 * @return Mat3 
	// 	 */
	// 	Mat3 normalMatrix()  
	// 	{ 
	// 		// return Math::normalMatrix(Rotation, Scale); 

	// 		const float c3 = glm::cos(Rotation.z);
	// 		const float s3 = glm::sin(Rotation.z);

	// 		const float c2 = glm::cos(Rotation.x);
	// 		const float s2 = glm::sin(Rotation.x);
			
	// 		const float c1 = glm::cos(Rotation.y);
	// 		const float s1 = glm::sin(Rotation.y);

	// 		const Vec3 invScale = 1.0f / Scale;
			
	// 		return Mat3{
	// 			{ 
	// 				invScale.x * (c1 * c3 + s1 * s2 * s3),
	// 				invScale.x * (c2 * s3),
	// 				invScale.x * (c1 * s2 * s3 - c3 * s1),
	// 			},
	// 			{
	// 				invScale.y * (c3 * s1 * s2 - c1 * s3),
	// 				invScale.y * (c2 * c3),
	// 				invScale.y * (c1 * c3 * s2 + s1 * s3),
	// 			},
	// 			{
	// 				invScale.z * (c2 * s1),
	// 				invScale.z * (-s2),
	// 				invScale.z * (c1 * c2),
	// 			}
	// 		};
	// 	}

	// 	// Vec3 forward() const { return Math::forward(Rotation); }
	// 	// Vec3 right()   const { return Math::right(Rotation);   }
	// 	// Vec3 up()      const { return Math::up(Rotation);      }
	// };
}