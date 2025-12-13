#version 450

// ================================================================================================

struct PointLight
{
    vec4 Position;  // ignore w
    vec4 Color;     // w is intensity
};

// ================================================================================================
// Uniforms

layout(set = 0, binding = 0) uniform GlobalUBO 
{
    mat4       Projection;
    mat4       View;
    mat4       InverseView;

    vec4       AmbientLightColor; // w is intensity
    PointLight PointLights[10];
    int        NumPointLights;

} uUbo;

layout(push_constant, std430) uniform Push
{
    mat4 ModelMatrix;
    mat4 NormalMatrix;

} uPush;

// ================================================================================================

// Input
// layout(location = 0) in vec3  inPosition;

// Output
layout(location = 0) out vec3 fragUVW;

// ================================================================================================

// void main()
// {
//     fragUVW = inPosition;

//     mat4 viewWithoutTranslation = mat4(mat3(uUbo.View)); // Remove translation
    
//     vec4 clipPos = uUbo.Projection * viewWithoutTranslation * vec4(inPosition, 1.0);

//     gl_Position = clipPos.xyww; // Force depth to 1.0
// }

// Vertices for a unit cube
vec3 positions[36] = vec3[](

    vec3(-1.0f,  1.0f, -1.0f), 
    vec3(-1.0f, -1.0f, -1.0f),
    vec3( 1.0f, -1.0f, -1.0f),
    vec3( 1.0f, -1.0f, -1.0f),
    vec3( 1.0f,  1.0f, -1.0f),
    vec3(-1.0f,  1.0f, -1.0f),

    vec3(-1.0f, -1.0f,  1.0f), 
    vec3(-1.0f, -1.0f, -1.0f),
    vec3(-1.0f,  1.0f, -1.0f),
    vec3(-1.0f,  1.0f, -1.0f),
    vec3(-1.0f,  1.0f,  1.0f),
    vec3(-1.0f, -1.0f,  1.0f),

    vec3( 1.0f, -1.0f, -1.0f),
    vec3( 1.0f, -1.0f,  1.0f),
    vec3( 1.0f,  1.0f,  1.0f),
    vec3( 1.0f,  1.0f,  1.0f),
    vec3( 1.0f,  1.0f, -1.0f),
    vec3( 1.0f, -1.0f, -1.0f),

    vec3(-1.0f, -1.0f,  1.0f),
    vec3(-1.0f,  1.0f,  1.0f),
    vec3( 1.0f,  1.0f,  1.0f),
    vec3( 1.0f,  1.0f,  1.0f),
    vec3( 1.0f, -1.0f,  1.0f),
    vec3(-1.0f, -1.0f,  1.0f),

    vec3(-1.0f,  1.0f, -1.0f),
    vec3( 1.0f,  1.0f, -1.0f),
    vec3( 1.0f,  1.0f,  1.0f),
    vec3( 1.0f,  1.0f,  1.0f),
    vec3(-1.0f,  1.0f,  1.0f),
    vec3(-1.0f,  1.0f, -1.0f),

    vec3(-1.0f, -1.0f, -1.0f),
    vec3(-1.0f, -1.0f,  1.0f),
    vec3( 1.0f, -1.0f, -1.0f),
    vec3( 1.0f, -1.0f, -1.0f),
    vec3(-1.0f, -1.0f,  1.0f),
    vec3( 1.0f, -1.0f,  1.0f)
);

void main() 
{
    // Get the vertex position from the hardcoded array
    vec3 pos = positions[ gl_VertexIndex ];

    // Remove translation from the view matrix for the skybox
    // This makes the skybox appear infinitely far away and fixed
    mat4 rotView = mat4(mat3(uUbo.View));
    
    // Calculate the clip-space position.
    gl_Position = uUbo.Projection * rotView * vec4(pos, 1.0);

    // Set z-component to w for max depth (ensures it's drawn behind everything).
    gl_Position.z = gl_Position.w;

    // The output texture coordinate is simply the vertex position,
    // which will be used as the cubemap lookup vector in the fragment shader.
    fragUVW = pos;

    // Flip y-axis for vulkan.
    fragUVW.y *= -1;
}