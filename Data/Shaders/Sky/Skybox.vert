#version 450

// ================================================================================================
// CONSTANTS

#define MAX_POINT_LIGHTS 10
#define MAX_SPOT_LIGHTS  10

// ================================================================================================
// INPUT

// ================================================================================================
// OUTPUT

layout (location = 0) out vec3 fragUVW;

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

// Vertices for a unit cube
vec3 POSITIONS2[36] = vec3[](

    vec3(-1.0,  1.0, -1.0), 
    vec3(-1.0, -1.0, -1.0),
    vec3( 1.0, -1.0, -1.0),
    vec3( 1.0, -1.0, -1.0),
    vec3( 1.0,  1.0, -1.0),
    vec3(-1.0,  1.0, -1.0),

    vec3(-1.0, -1.0,  1.0), 
    vec3(-1.0, -1.0, -1.0),
    vec3(-1.0,  1.0, -1.0),
    vec3(-1.0,  1.0, -1.0),
    vec3(-1.0,  1.0,  1.0),
    vec3(-1.0, -1.0,  1.0),

    vec3( 1.0, -1.0, -1.0),
    vec3( 1.0, -1.0,  1.0),
    vec3( 1.0,  1.0,  1.0),
    vec3( 1.0,  1.0,  1.0),
    vec3( 1.0,  1.0, -1.0),
    vec3( 1.0, -1.0, -1.0),

    vec3(-1.0, -1.0,  1.0),
    vec3(-1.0,  1.0,  1.0),
    vec3( 1.0,  1.0,  1.0),
    vec3( 1.0,  1.0,  1.0),
    vec3( 1.0, -1.0,  1.0),
    vec3(-1.0, -1.0,  1.0),

    vec3(-1.0,  1.0, -1.0),
    vec3( 1.0,  1.0, -1.0),
    vec3( 1.0,  1.0,  1.0),
    vec3( 1.0,  1.0,  1.0),
    vec3(-1.0,  1.0,  1.0),
    vec3(-1.0,  1.0, -1.0),

    vec3(-1.0, -1.0, -1.0),
    vec3(-1.0, -1.0,  1.0),
    vec3( 1.0, -1.0, -1.0),
    vec3( 1.0, -1.0, -1.0),
    vec3(-1.0, -1.0,  1.0),
    vec3( 1.0, -1.0,  1.0)
);

// Cube vertices (no vertex buffer needed - generate from gl_VertexIndex)
vec3 POSITIONS[ 36 ] = vec3[]
(
    // Front face
    vec3(-1.0, -1.0,  1.0),
    vec3( 1.0, -1.0,  1.0),
    vec3( 1.0,  1.0,  1.0),
    vec3( 1.0,  1.0,  1.0),
    vec3(-1.0,  1.0,  1.0),
    vec3(-1.0, -1.0,  1.0),
    // Back face
    vec3( 1.0, -1.0, -1.0),
    vec3(-1.0, -1.0, -1.0),
    vec3(-1.0,  1.0, -1.0),
    vec3(-1.0,  1.0, -1.0),
    vec3( 1.0,  1.0, -1.0),
    vec3( 1.0, -1.0, -1.0),
    // Top face
    vec3(-1.0,  1.0,  1.0),
    vec3( 1.0,  1.0,  1.0),
    vec3( 1.0,  1.0, -1.0),
    vec3( 1.0,  1.0, -1.0),
    vec3(-1.0,  1.0, -1.0),
    vec3(-1.0,  1.0,  1.0),
    // Bottom face
    vec3(-1.0, -1.0, -1.0),
    vec3( 1.0, -1.0, -1.0),
    vec3( 1.0, -1.0,  1.0),
    vec3( 1.0, -1.0,  1.0),
    vec3(-1.0, -1.0,  1.0),
    vec3(-1.0, -1.0, -1.0),
    // Right face
    vec3( 1.0, -1.0,  1.0),
    vec3( 1.0, -1.0, -1.0),
    vec3( 1.0,  1.0, -1.0),
    vec3( 1.0,  1.0, -1.0),
    vec3( 1.0,  1.0,  1.0),
    vec3( 1.0, -1.0,  1.0),
    // Left face
    vec3(-1.0, -1.0, -1.0),
    vec3(-1.0, -1.0,  1.0),
    vec3(-1.0,  1.0,  1.0),
    vec3(-1.0,  1.0,  1.0),
    vec3(-1.0,  1.0, -1.0),
    vec3(-1.0, -1.0, -1.0)
);

// ================================================================================================
// MAIN

void main()
{
    // Get the vertex position from the hardcoded array
    vec3 pos = POSITIONS2[ gl_VertexIndex ];

    // Remove translation from the view matrix for the skybox
    // This makes the skybox appear infinitely far away and fixed
    mat4 rotView = mat4(mat3(uUbo.Camera.View));

    // Calculate the clip-space position.
    gl_Position = uUbo.Camera.Projection * rotView * vec4(pos, 1.0);

    // Set z-component to w for max depth (ensures it's drawn behind everything).
    gl_Position.z = gl_Position.w;

    // The output texture coordinate is simply the vertex position,
    // which will be used as the cubemap lookup vector in the fragment shader.
    fragUVW = pos;

    // Flip y-axis for vulkan.
    fragUVW.y *= -1;
}