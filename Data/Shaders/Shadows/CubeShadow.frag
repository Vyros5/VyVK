#version 450

// ================================================================================================
// Uniforms

layout(push_constant) uniform Push 
{
    mat4 ModelMatrix;
    mat4 LightSpaceMatrix;
    vec4 LightPosAndFarPlane; // xyz = light position, w = far plane

} uPush;

// ================================================================================================
// Input

layout(location = 0) in vec3 fragPosWorld;

// ================================================================================================

void main() 
{
    // Calculate distance from fragment to light
    vec3  lightPos           = uPush.LightPosAndFarPlane.xyz;
    float farPlane           = uPush.LightPosAndFarPlane.w;
    float distance           = length(fragPosWorld - lightPos);
    float normalizedDistance = distance / farPlane;

    // Write normalized distance as depth
    gl_FragDepth = clamp(normalizedDistance, 0.0, 1.0);
}