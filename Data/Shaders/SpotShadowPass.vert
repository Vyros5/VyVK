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
layout (location = 4) in vec4 aTangent;

// ================================================================================================
// OUTPUT

layout (location = 0) out vec4 fragPos;

// ================================================================================================
// DESCRIPTOR SET 0 : GLOBAL

struct PointLightData
{
	vec4 Position;     // position x,y,z
	vec4 Color;        // color r=x, g=y, b=z, a=intensity
};

struct SpotLightData
{
	vec4 Position;     // position x,y,z
	vec4 Color;        // color r=x, g=y, b=z, a=intensity
	vec4 Direction;    // direction x, y, z
	vec4 Cutoffs;      // Cutoffs x=innerCutoff y=outerCutoff
};

struct DirectionalLightData
{
	vec4 Direction;    // direction x, y, z, w=ambientStrength
	vec4 Color;        // color rgb, a=intensity
};

struct CameraData
{
    mat4 Projection;
    mat4 View;
    mat4 InverseView;
};

layout (set = 0, binding = 0) uniform GlobalUbo
{
	CameraData Camera;

	DirectionalLightData DirectionalLight;

	PointLightData PointLights[ MAX_POINT_LIGHTS ];
	SpotLightData  SpotLights [ MAX_SPOT_LIGHTS  ];

	int PointLightsCount;
	int SpotLightsCount;

} uUbo;

// ================================================================================================
// DESCRIPTOR SET 1 : SPOT LIGHT PROJECTION FOR SHADOW MAPS

layout(set = 1, binding = 0) uniform SpotShadowPassUBO
{
	mat4 LightProjection;

} uSpotShadowPassUBO;

// ================================================================================================
// CONSTANT PUSH : SPOT SHADOW

layout (push_constant) uniform Push 
{
	mat4 ModelMatrix;
	int  LightCount;

} uPush;

// ================================================================================================
// MAIN 

void main()
{
    fragPos = uPush.ModelMatrix * vec4(aPosition, 1.0f);
	
    gl_Position = uSpotShadowPassUBO.LightProjection * fragPos;
}

// ================================================================================================