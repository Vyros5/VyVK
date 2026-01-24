#version 450

// ================================================================================================
// CONSTANTS

#define MAX_POINT_LIGHTS 10
#define MAX_SPOT_LIGHTS 10

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
	vec4 Color;        // color r=x, g=y, b=z, a=intensity
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
	
    if (dis >= 1.0f)
	{
		discard;
	}

	outColor = vec4(uPush.Color.xyz, 0.5f * (cos(dis * M_PI) + 1.0f));
}

// ================================================================================================