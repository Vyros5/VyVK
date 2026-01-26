#version 450

// ================================================================================================
// CONSTANTS

#define MAX_POINT_LIGHTS 10
#define MAX_SPOT_LIGHTS  10

// ================================================================================================
// Uniforms

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
// Output

layout (location = 1) out vec3 nearPoint;
layout (location = 2) out vec3 farPoint;
layout (location = 3) out mat4 fragView;
layout (location = 7) out mat4 fragProj;

// ================================================================================================

// Grid position in clip space.
vec3 GRID_PLANE[ 6 ] = vec3[] 
(
    vec3( 1,  1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
    vec3(-1, -1,  0), vec3( 1,  1,  0), vec3( 1, -1,  0)
);

// ================================================================================================

vec3 UnprojectPoint(float x, float y, float z, mat4 view, mat4 projection) 
{
    mat4 viewInv = inverse(view);
    mat4 projInv = inverse(projection);

    vec4 unprojectedPoint =  viewInv * projInv * vec4(x, y, z, 1.0);

    return unprojectedPoint.xyz / unprojectedPoint.w;
}

// ================================================================================================

void main() 
{
    vec3 point = GRID_PLANE[ gl_VertexIndex ].xyz;

    // unprojecting on the near plane.
    nearPoint = UnprojectPoint(point.x, point.y, 0.0, uUbo.Camera.View, uUbo.Camera.Projection).xyz;

    // unprojecting on the far plane.
    farPoint  = UnprojectPoint(point.x, point.y, 1.0, uUbo.Camera.View, uUbo.Camera.Projection).xyz;
    
    fragView  = uUbo.Camera.View;
    fragProj  = uUbo.Camera.Projection;

    // Using directly the clipped coordinates.
    gl_Position = vec4(point.xyz, 1.0);
}