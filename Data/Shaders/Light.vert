#version 450

// ================================================================================================
// CONSTANTS

#define MAX_POINT_LIGHTS 10
#define MAX_SPOT_LIGHTS 10

const vec2 OFFSETS[6] = vec2[](
    vec2(-1.0, -1.0),
    vec2(-1.0,  1.0),
    vec2( 1.0, -1.0),
    vec2( 1.0, -1.0),
    vec2(-1.0,  1.0),
    vec2( 1.0,  1.0)
);

// ================================================================================================
// OUTPUT

layout(location = 0) out vec2 fragOffset;

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

// WS = World Space

void main()
{
	fragOffset = OFFSETS[ gl_VertexIndex ];

	vec3 cameraRightWS = { uUbo.Camera.View[0][0], uUbo.Camera.View[1][0], uUbo.Camera.View[2][0] };
	vec3 cameraUpWS    = { uUbo.Camera.View[0][1], uUbo.Camera.View[1][1], uUbo.Camera.View[2][1] };

	vec3 lightWS = uPush.Position.xyz
        + uPush.Radius * fragOffset.x * cameraRightWS
        + uPush.Radius * fragOffset.y * cameraUpWS;

	gl_Position = uUbo.Camera.Projection * uUbo.Camera.View * vec4(lightWS, 1.0f);
}

// ================================================================================================