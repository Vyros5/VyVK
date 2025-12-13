#pragma once

#include <VyLib/VyLib.h>

namespace Vy
{
    /**
     * @brief Ambient Light.
     * 
     * @note Optional Component
     */
	struct AmbientLightComponent
	{
		Vec3  Color     = { 1.0f, 1.0f, 1.0f };
		float Intensity = 0.1f;
	};


    /**
     * @brief Directional Light. (like sun)
     * 
     * @note Optional Component
     */
	struct DirectionalLightComponent
	{
		Vec3  Color{ 1.0f, 1.0f, 1.0f };
		float Intensity{1.0f};
        bool  UseTargetPoint{false};
        Vec3  TargetPoint{ 0.0f, 0.0f, 0.0f };
	};


    /**
     * @brief Omnidirectional point light.
     * 
     * @note Optional Component
     */
	struct PointLightComponent
	{
		Vec3  Color{ 1.0f, 1.0f, 1.0f };
		float Intensity{1.0f};
        float Radius{0.1f};

        // PointLightComponent(const Vec3& color, float intensity) : 
        //     Color{ color }, 
        //     Intensity{ intensity } 
        // {
        // }
	};


    struct SpotLightComponent
    {
        Vec3  Color{ 1.0f, 1.0f, 1.0f };
        float Intensity{1.0f};
        float InnerCutoffAngle{12.5f}; // Inner cone angle in degrees
        float OuterCutoffAngle{17.5f}; // Outer cone angle in degrees
        float ConstantAttenuation{1.0f};
        float LinearAttenuation{0.09f};
        float QuadraticAttenuation{0.032f};
        bool  UseTargetPoint{false};
        Vec3  TargetPoint{0.0f, 0.0f, 0.0f};
    };
}