#version 450

// ================================================================================================
// Uniforms

layout(push_constant) uniform Push 
{
    mat4 ModelMatrix;
    mat4 LightSpaceMatrix;

} uPush;

// ================================================================================================
// Input

// Only use position for shadow pass - other attributes are bound but unused.
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inTangent;
layout(location = 3) in vec3 inColor;
layout(location = 4) in vec2 inUV;

// ================================================================================================
// Output

// None

// ================================================================================================

void main() 
{
    vec4 positionWorld = uPush.ModelMatrix * vec4(inPosition, 1.0);

    gl_Position = uPush.LightSpaceMatrix * positionWorld;
}