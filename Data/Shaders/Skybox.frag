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

// layout(set = 0, binding = 1) uniform samplerCube samplerCubeMap;
layout(set = 1, binding = 0) uniform samplerCube skyboxSampler;

layout(push_constant, std430) uniform Push
{
    mat4 ModelMatrix;
    mat4 NormalMatrix;

} uPush;

// ================================================================================================

// Input
layout(location = 0) in  vec3  fragUVW;

// Output
layout(location = 0) out vec4  outColor;

// ================================================================================================

void main()
{
    // outColor = texture(samplerCubeMap, fragUVW);

    // Sample the cubemap using the interpolated texture coordinate (view direction).
    outColor = texture(skyboxSampler, fragUVW);
}