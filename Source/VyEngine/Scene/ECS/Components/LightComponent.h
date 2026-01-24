#pragma once

#include <VyLib/VyLib.h>

namespace Vy
{
    struct LightComponent
    {
        bool IsPoint = true;
        Vec3 Color;

        float Intensity;
        float Radius;

        Vec3  Direction;
        float Cutoff;
        float OuterCutoff;

        static LightComponent pointLight(Vec3 color, float intensity, float radius)
        {
            LightComponent obj{};
            {
                obj.IsPoint   = true;
                obj.Color     = color;
                obj.Intensity = intensity;
                obj.Radius    = radius;
            }

            return obj;
        }

        static LightComponent spotLight(Vec3 color, float intensity, float radius, Vec3 spotDirection, float cutOff, float outerCutOff)
        {
            LightComponent obj{};
            {
                obj.IsPoint     = false;
                obj.Color       = color;
                obj.Intensity   = intensity;
                obj.Radius      = radius;
                obj.Direction   = spotDirection;
                obj.Cutoff      = cutOff;
                obj.OuterCutoff = outerCutOff;
            }

            return obj;
        }
    };

    /**
     * @brief Ambient Light.
     * 
     * @note Optional Component
     */
	struct AmbientLightComponent
	{
		Vec3  Color    { 1.0f, 1.0f, 1.0f };
		float Intensity{ 1.0f };
	};


    /**
     * @brief Directional Light. (like sun)
     * 
     * @note Optional Component
     */
	struct DirectionalLightComponent
	{
		Vec3  Color         { 1.0f, 1.0f, 1.0f };
		float Intensity     { 1.0f };

        bool  UseTargetPoint{ false };
        Vec3  TargetPoint   { 0.0f, 0.0f, 0.0f };
	};


    /**
     * @brief Omnidirectional point light.
     * 
     * @note Optional Component
     */
	struct PointLightComponent
	{
		Vec3  Color    { 1.0f, 1.0f, 1.0f };
		float Intensity{ 1.0f };
        float Radius   { 0.2f };
	};


    struct SpotLightComponent
    {
		Vec3  Color    { 1.0f, 1.0f, 1.0f };
		float Intensity{ 1.0f };
        
        float InnerCutoffAngle    { 12.5f }; // Inner cone angle in degrees
        float OuterCutoffAngle    { 17.5f }; // Outer cone angle in degrees
        float ConstantAttenuation { 1.0f  };
        float LinearAttenuation   { 0.09f };
        float QuadraticAttenuation{ 0.032f };
        bool  UseTargetPoint      { false };
        Vec3  TargetPoint         { 0.0f, 0.0f, 0.0f };
    };
}