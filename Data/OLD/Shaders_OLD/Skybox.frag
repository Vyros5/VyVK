#version 450

// ================================================================================================


// ================================================================================================
// Uniforms

layout(set = 1, binding = 0) uniform samplerCube skyboxSampler;


layout(push_constant, std430) uniform Push
{
    mat4 ModelMatrix;
    mat4 NormalMatrix;

} uPush;

// ================================================================================================
// Input

layout(location = 0) in  vec3  fragUVW;

// ================================================================================================
// Output

layout(location = 0) out vec4  outColor;

// ================================================================================================

void main()
{
    // Sample the cubemap using the interpolated texture coordinate (view direction).
    outColor = texture(skyboxSampler, fragUVW);
}