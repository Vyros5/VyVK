#version 450

layout(set = 0, binding = 0) uniform GlobalUbo 
{
    mat4 LightViewProj;

} uUbo;

layout(push_constant) uniform Push 
{
    mat4 ModelMatrix;

} uPush;

// ================================================================================================
// Input

// Only use position for shadow pass - other attributes are bound but unused.
layout(location = 0) in vec3 inPosition;
// layout(location = 1) in vec3 inNormal;
// layout(location = 2) in vec3 inTangent;
// layout(location = 3) in vec3 inColor;
// layout(location = 4) in vec2 inUV;

// ================================================================================================
// Output

// layout(location = 0) out vec4 fragPosLightSpace;

// ================================================================================================


void main()
{
    vec4 worldPosition = vec4(inPosition, 1.0);
    
    gl_Position = uUbo.LightViewProj * uPush.ModelMatrix * worldPosition;
}