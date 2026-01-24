#pragma once

#include <VyEngine/VK/Core/VKCore.h>
// #include <VyEngine/Scene/ECS/Components/LightComponent.h>

// // #define MAX_LIGHTS        10
#define MAX_POINT_LIGHTS  16
#define MAX_DIRECT_LIGHTS 16
#define MAX_SPOT_LIGHTS   16

namespace Vy 
{
    struct PointLightUBO
    {
        Vec4 Position{}; // xyz = position, w = unused
        Vec4 Color   {}; // rgb = color,    a = intensity
    };

    struct DirectionalLightUBO
    {
        Vec4 Direction{}; // xyz = direction, w = unused (direction should be normalized)
        Vec4 Color    {}; // rgb = color,     a = intensity
    };
    
    struct SpotLightUBO
    {
        Vec4  Position    {}; // xyz = position,   w = unused
        Vec4  Direction   {}; // xyz = direction,  w = unused
        Vec4  Color       {}; // rgb = color,      a = intensity
        Vec4  Cutoffs     {}; // CutOffs x=innerCutoff y=outerCutoff
        // float OuterCutoff {}; // cos of outer angle
        // // float InnerCutoff {}; // cos of inner angle
        // float ConstantAtten {}; // Constant attenuation
        // float LinearAtten   {}; // Linear attenuation
        // float QuadraticAtten{}; // Quadratic attenuation
    };

    struct CameraDataUBO
    {
        Mat4 Projection   { 1.0f };
        Mat4 View         { 1.0f };
        Mat4 InverseView  { 1.0f };
    };

    // struct GlobalUBO 
    // {
    //     CameraDataUBO       CameraData          {};

    //     Vec4                AmbientLightColor   { 1.0f, 1.0f, 1.0f, 0.02f }; // rgb = color, a = intensity

    //     PointLightUBO       PointLights         [ MAX_POINT_LIGHTS  ];
    //     DirectionalLightUBO DirectionalLights   [ MAX_DIRECT_LIGHTS ];
    //     SpotLightUBO        SpotLights          [ MAX_SPOT_LIGHTS   ];
        
    //     Mat4                LightSpaceMatrix    {}; // For shadowMapping
    //     Vec4                ShadowLightDirection{}; // xyz = direction, w = light type
        
    //     // LightUBO            Lights              [ MAX_LIGHTS        ];
    //     // int                 NumLights           { 0 };
    //     int                 NumPointLights      { 0 };
    //     int                 NumDirectionalLights{ 0 };
    //     int                 NumSpotLights       { 0 };
    // };

    struct GlobalUbo
    {
        CameraDataUBO       CameraData{};

        Vec4                LightAmbient{1.0f, 1.0f, 1.0f, 0.02f};
        
        DirectionalLightUBO DirectionalLight{};
        
        PointLightUBO       PointLights[ MAX_POINT_LIGHTS ];
        SpotLightUBO        SpotLights [ MAX_SPOT_LIGHTS ];

        alignas(4) int      PointLightsCount;
        alignas(4) int      SpotLightsCount;

    };
}