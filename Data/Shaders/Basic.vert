#version 450

// ================================================================================================
// CONSTANTS

#define MAX_POINT_LIGHTS 10
#define MAX_SPOT_LIGHTS  10

#define CASCADE_SHADOW_MAP_COUNT 4

const mat4 BIAS = mat4( 
    0.5, 0.0, 0.0, 0.0,
    0.0, 0.5, 0.0, 0.0,
    0.0, 0.0, 1.0, 0.0,
    0.5, 0.5, 0.0, 1.0 
);

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
layout (location = 8) out vec3 fragLightVec; //outLightVec

layout (location = 9) out vec4 fragSpotLightWS[ MAX_SPOT_LIGHTS ];

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
// DESCRIPTOR SET 4,1 : SPOT LIGHT PROJECTION FOR SHADOW MAP

layout (set = 4, binding = 1) uniform SpotShadowLightProjectionUBO
{
	mat4 LightProjection[MAX_SPOT_LIGHTS];

} uSpotShadowLightProjectionUBO;

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
	vec4 modelWS = uPush.ModelMatrix * vec4(aPosition, 1.0f);
    
	gl_Position = uUbo.Camera.Projection * uUbo.Camera.View * modelWS;

	fragNormalWS = mat3(uPush.NormalMatrix) * aNormal;
	fragModelWS  = modelWS.xyz; // outEyePos
	fragColor    = aColor;
	fragUV       = aUV;
	fragTangent  = mat3(uPush.NormalMatrix) * aTangent;

	fragViewPos = uUbo.Camera.View * modelWS;

	for(int i = 0; i < MAX_SPOT_LIGHTS; i++)
	{
		fragSpotLightWS[i] = BIAS * uSpotShadowLightProjectionUBO.LightProjection[ i ] * modelWS;
	}

	fragModelPos = aPosition; // outWorldPos
	fragLightVec = normalize(uUbo.PointLights[0].Position.xyz - aPosition.xyz); //outLightVec
}

// ================================================================================================