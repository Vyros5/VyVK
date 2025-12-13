#include <Vy/Math/Math.h>

namespace Vy::Math
{
	Vec3 forward(const Vec3& rotation)
    {
		const float sx = glm::sin(rotation.x);
		const float cx = glm::cos(rotation.x);
		const float sy = glm::sin(rotation.y);
		const float cy = glm::cos(rotation.y);
		const float cz = glm::cos(rotation.z);
		const float sz = glm::sin(rotation.z);

		return { 
            sx * sy * cz - cx * sz, 
            sx * sy * sz + cx * cz, 
            sx * cy 
        };
    }

	Vec3 forward(const Quat& rotation)
    {
		return glm::rotate(rotation, World::FORWARD);
        // return rotation * World::FORWARD;
    }


	Vec3 right(const Vec3& rotation)
    {
		const float sy = glm::sin(rotation.y);
		const float cy = glm::cos(rotation.y);
		const float cz = glm::cos(rotation.z);
		const float sz = glm::sin(rotation.z);

		return { 
             cy * cz, 
             cy * sz, 
            -sy 
        };
    }

	Vec3 right(const Quat& rotation)
    {
		return glm::rotate(rotation, World::RIGHT);
        // return rotation * World::RIGHT;
    }


	Vec3 up(const Vec3& rotation)
    {
		const float sx = glm::sin(rotation.x);
		const float cx = glm::cos(rotation.x);
		const float sy = glm::sin(rotation.y);
		const float cy = glm::cos(rotation.y);
		const float cz = glm::cos(rotation.z);
		const float sz = glm::sin(rotation.z);

		return { 
            cx * sy * cz + sx * sz, 
            cx * sy * sz - sx * cz, 
            cx * cy 
        };
    }

	Vec3 up(const Quat& rotation)
    {
		return glm::rotate(rotation, World::UP);
        // return rotation * World::UP;
    }


	Mat4 tranformationMatrix(const Vec3& location, const Vec3& rotation, const Vec3& scale)
    {
		const float c3 = glm::cos(rotation.z);
		const float s3 = glm::sin(rotation.z);
		const float c2 = glm::cos(rotation.x);
		const float s2 = glm::sin(rotation.x);
		const float c1 = glm::cos(rotation.y);
		const float s1 = glm::sin(rotation.y);

		return Mat4{
			{
				scale.x * (c1 * c3 + s1 * s2 * s3),
				scale.x * (c2 * s3),
				scale.x * (c1 * s2 * s3 - c3 * s1),
				0.0f,
			},
			{
				scale.y * (c3 * s1 * s2 - c1 * s3),
				scale.y * (c2 * c3),
				scale.y * (c1 * c3 * s2 + s1 * s3),
				0.0f,
			},
			{
				scale.z * (c2 * s1),
				scale.z * (-s2),
				scale.z * (c1 * c2),
				0.0f,
			},
			{
				location.x, 
				location.y, 
				location.z, 
				1.0f
			} 
        };
    }

	Mat4 tranformationMatrix(const Vec3& location, const Quat& rotation, const Vec3& scale)
    {
		Mat3 rotMat = glm::mat3_cast(rotation);
		
		rotMat[0] *= scale.x;
		rotMat[1] *= scale.y;
		rotMat[2] *= scale.z;

		return Mat4{
			Vec4{ rotMat[0], 0.0f },
			Vec4{ rotMat[1], 0.0f },
			Vec4{ rotMat[2], 0.0f },
			Vec4{ location,  1.0f }
		};
    }


	/// @brief Composes a matrix to transform normals into world space
	Mat3 normalMatrix(const Vec3& rotation, const Vec3& scale)
    {
		const float c3 = glm::cos(rotation.z);
		const float s3 = glm::sin(rotation.z);
		const float c2 = glm::cos(rotation.x);
		const float s2 = glm::sin(rotation.x);
		const float c1 = glm::cos(rotation.y);
		const float s1 = glm::sin(rotation.y);

		const Vec3 invScale = 1.0f / scale;

		return Mat3{
			{
				invScale.x * (c1 * c3 + s1 * s2 * s3),
				invScale.x * (c2 * s3),
				invScale.x * (c1 * s2 * s3 - c3 * s1),
			},
			{
				invScale.y * (c3 * s1 * s2 - c1 * s3),
				invScale.y * (c2 * c3),
				invScale.y * (c1 * c3 * s2 + s1 * s3),
			},
			{
				invScale.z * (c2 * s1),
				invScale.z * (-s2),
				invScale.z * (c1 * c2),
			} 
        };
    }

	Mat3 normalMatrix(const Quat& rotation, const Vec3& scale)
    {
		Mat3 rotMat = glm::mat3_cast(rotation);

		rotMat[0] *= 1.0f / scale.x;
		rotMat[1] *= 1.0f / scale.y;
		rotMat[2] *= 1.0f / scale.z;

		return rotMat;
    }


	/// @brief Decomposes a transformation matrix into location, rotation and scale
	void decomposeTRS(const Mat4& matrix, Vec3& translation, Vec3& rotation, Vec3& scale)
    {
		// Extract translation from the last column.
		translation = Vec3(matrix[3]);

		// Extract scale from the basis vectors. (First 3 columns)
		scale.x = glm::length(Vec3(matrix[0]));
		scale.y = glm::length(Vec3(matrix[1]));
		scale.z = glm::length(Vec3(matrix[2]));

		Mat3 rotationMat = Mat3(matrix);

		rotationMat[0] /= scale.x;
		rotationMat[1] /= scale.y;
		rotationMat[2] /= scale.z;

		rotation = glm::eulerAngles(glm::quat_cast(rotationMat));
    }


	void decomposeTransform(const Mat4& matrix, Vec3& translation, Vec3& rotation, Vec3& scale)
    {
		// Extract translation from the last column.
		translation = Vec3(matrix[3]);

		// Extract scale from the basis vectors. (First 3 columns)
		scale.x = glm::length(Vec3(matrix[0]));
		scale.y = glm::length(Vec3(matrix[1]));
		scale.z = glm::length(Vec3(matrix[2]));

		// Remove scale from the matrix to isolate the rotation matrix.
		Mat4 rotationMat = matrix;

		if (scale.x != 0)
			rotationMat[0] /= scale.x;
		if (scale.y != 0)
			rotationMat[1] /= scale.y;
		if (scale.z != 0)
			rotationMat[2] /= scale.z;

		// Extract rotation (in radians) from the rotation matrix.
		rotation.x = glm::atan2(rotationMat[2][1], rotationMat[2][2]);                                                                        // Pitch (x Axis)
		rotation.y = glm::atan2(rotationMat[2][0], glm::sqrt(rotationMat[2][1] * rotationMat[2][1] + rotationMat[2][2] * rotationMat[2][2])); // Yaw   (y Axis)
		rotation.z = glm::atan2(rotationMat[1][0], rotationMat[0][0]);                                                                        // Roll  (z Axis)
    }


	void decomposeTRS(const Mat4& matrix, Vec3& translation, Quat& rotation, Vec3& scale)
    {
		translation = Vec3(matrix[3]);

		scale.x = glm::length(Vec3(matrix[0]));
		scale.y = glm::length(Vec3(matrix[1]));
		scale.z = glm::length(Vec3(matrix[2]));

		Mat3 rotationMat = Mat3(matrix);
		
		rotationMat[0] /= scale.x;
		rotationMat[1] /= scale.y;
		rotationMat[2] /= scale.z;
        
		rotation = glm::quat_cast(rotationMat);
    }


	/// @brief Returns the percentage of a value between a min and max value
	float percentage(float value, float min, float max)
    {
        return (value - min) / (max - min);
    }


	/// @brief Checks if the target is in the field of view of the view direction
	bool inFOV(const Vec3& viewDirection, const Vec3& targetDirection, float fov)
    {
        return glm::angle(viewDirection, targetDirection) < 0.5f * fov;
    }

}