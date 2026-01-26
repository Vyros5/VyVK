#version 450

// ================================================================================================
// CONSTANTS

#define MAX_POINT_LIGHTS 10
#define MAX_SPOT_LIGHTS  10

// ================================================================================================
// INPUT

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in vec2 aUV;
layout (location = 4) in vec3 aTangent;

// ================================================================================================
// OUTPUT

layout (location = 0) out vec3 fragColor;
layout (location = 1) out vec3 fragModelWS; // outEyePos
layout (location = 2) out vec3 fragNormalWS;
layout (location = 3) out vec2 fragUV;
layout (location = 4) out vec3 fragTangent;
layout (location = 5) out vec4 fragViewPos;

layout (location = 7) out vec3 fragModelPos; //outWorldPos

// ================================================================================================
// DESCRIPTOR SET 0 : GLOBAL

struct PointLightData
{
	vec4 Position;     // xyz = position,  w = unused
	vec4 Color;        // rgb = color,     a = intensity
};

struct SpotLightData
{
	vec4 Position;     // xyz = position,  w = unused
	vec4 Color;        // rgb = color,     a = intensity
	vec4 Direction;    // xyz = direction, w = unused
	vec4 Cutoffs;      // x = innerCutoff, y = outerCutoff, zw = unused
};

struct DirectionalLightData
{
	vec4 Direction;    // xyz = direction, w = ambientStrength
	vec4 Color;        // rgb = color,     a = intensity
};

struct CameraData
{
    mat4 Projection;
    mat4 View;
    mat4 InverseView;
};

layout (set = 0, binding = 0) uniform GlobalUbo
{
	CameraData           Camera;

	DirectionalLightData DirectionalLight;

	PointLightData       PointLights[ MAX_POINT_LIGHTS ];
	SpotLightData        SpotLights [ MAX_SPOT_LIGHTS  ];

	int                  PointLightsCount;
	int                  SpotLightsCount;

} uUbo;

// ================================================================================================
// CONSTANT PUSH : MAIN

layout (push_constant) uniform Push 
{
	mat4 ModelMatrix;
	mat4 NormalMatrix;

} uPush;

// ================================================================================================

void main()
{
	vec4 modelWS = uPush.ModelMatrix * vec4(aPosition, 1.0);
    
	gl_Position  = uUbo.Camera.Projection * uUbo.Camera.View * modelWS;

	fragNormalWS = mat3(uPush.NormalMatrix) * aNormal;
	fragModelWS  = modelWS.xyz; // outEyePos
	fragColor    = aColor;
	fragUV       = aUV;
	fragTangent  = mat3(uPush.NormalMatrix) * aTangent;
	fragViewPos  = uUbo.Camera.View * modelWS;
	fragModelPos = aPosition; // outWorldPos
}

// ================================================================================================