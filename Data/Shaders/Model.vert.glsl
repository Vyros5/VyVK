#version 450
#pragma shader_stage(vertex)
// #extension GL_KHR_vulkan_glsl: enable

#include "../../Shared/Defines/GPUSharedDefines.h"

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec4 inTangent;
layout(location = 4) in vec2 inUV;
layout(location = 5) in vec2 inUV1;
// layout(location = 5) in ivec4 inJointIds;
// layout(location = 6) in vec4  inWeights;

layout(location = 0) out vec3 fragPosition;
layout(location = 1) out vec3 fragColor;
layout(location = 2) out vec3 fragNormal;
layout(location = 3) out vec4 fragTangent;
layout(location = 4) out vec2 fragUV;
layout(location = 5) out vec2 fragUV1;

struct PointLight 
{
    vec4 Position;  // ignore w
    vec4 Color;     // w is intensity
};

struct DirectionalLight 
{
    vec4 Direction;  // ignore w
    vec4 Color;      // w is intensity
};


layout(set = 0, binding = 0) uniform UBO
{
    mat4             Projection;
    mat4             View;
    mat4             InverseView;
    vec4             AmbientLightColor;
    PointLight       PointLights[MAX_LIGHTS];
    DirectionalLight DirectLight;
    int              LightCount;
    float            Gamma;
	float            Exposure;

} uUbo;

// layout(set = 0, binding = 0) uniform GlobalUbo 
// {
//     mat4 Projection;
//     mat4 View;
//     mat4 invView;
//     vec4 ambientLightColor;  // w is intensity
//     PointLight pointLights[MAX_LIGHTS];
//     DirectionalLight directionalLight;
//     int numLights;
//     float gamma;
// 	float exposure;
// } uUbo;

layout(push_constant) uniform Push 
{
    mat4 ModelMatrix;
    mat4 NormalMatrix;

} uPush;


void main() 
{
    vec4 positionWorld = uPush.ModelMatrix * vec4(inPosition, 1.0f);
    gl_Position  = uUbo.Projection * uUbo.View * positionWorld;
    fragPosition = positionWorld.xyz;

    fragNormal  = normalize(mat3(uPush.NormalMatrix) * inNormal);
    fragTangent = normalize(mat4(uPush.NormalMatrix) * inTangent);
    fragColor   = inColor;
    fragUV      = inUV;
}