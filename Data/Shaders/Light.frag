#version 450

// ================================================================================================
// CONSTANTS

#define MAX_POINT_LIGHTS 10
#define MAX_SPOT_LIGHTS  10

const float M_PI = 3.1415926538;

// ================================================================================================
// INPUT

layout(location = 0) in vec2 fragOffset;

// ================================================================================================
// OUTPUT

layout(location = 0) out vec4 outColor;

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
// CONSTANT PUSH : LIGHT OBJECT

layout (push_constant) uniform Push 
{
	vec4  Position;
	vec4  Color;
	float Radius;

} uPush;

// ================================================================================================

void main()
{
	float dis = sqrt(dot(fragOffset, fragOffset));
	
    if (dis >= 1.0)
	{
		discard;
	}

	outColor = vec4(uPush.Color.xyz, 0.5 * (cos(dis * M_PI) + 1.0));
}

// ================================================================================================