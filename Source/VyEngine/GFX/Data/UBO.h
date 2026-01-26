#pragma once

#include <VyEngine/VK/Core/VKCore.h>

#define MAX_POINT_LIGHTS  10
#define MAX_SPOT_LIGHTS   10

namespace Vy 
{
    struct PointLightUBO
    {
        Vec4 Position{}; // xyz = position, w = unused
        Vec4 Color   {}; // rgb = color,    a = intensity
    };

    struct DirectionalLightUBO
    {
        Vec4 Direction{ -5.0f, 30.0f, -2.0f, 0.06f }; // xyz = direction, w = ambientStrength
        Vec4 Color    {  1.0f,  1.0f,  1.0f,  0.7f }; // rgb = color,     a = intensity
    };
    
    struct SpotLightUBO
    {
        Vec4  Position    {}; // xyz = position,   w = unused
        Vec4  Direction   {}; // xyz = direction,  w = unused
        Vec4  Color       {}; // rgb = color,      a = intensity
        Vec4  Cutoffs     {}; // CutOffs x=innerCutoff y=outerCutoff
    };

    struct CameraDataUBO
    {
        Mat4 Projection   { 1.0f };
        Mat4 View         { 1.0f };
        Mat4 InverseView  { 1.0f };
    };

    struct GlobalUbo
    {
        CameraDataUBO       CameraData{};

        // Vec4                LightAmbient{1.0f, 1.0f, 1.0f, 0.02f};
        
        DirectionalLightUBO DirectionalLight{};
        
        PointLightUBO       PointLights[ MAX_POINT_LIGHTS ];
        SpotLightUBO        SpotLights [ MAX_SPOT_LIGHTS ];

        alignas(4) int      PointLightsCount;
        alignas(4) int      SpotLightsCount;

    };
}