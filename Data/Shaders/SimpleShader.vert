#version 450

// ================================================================================================

struct PointLight 
{
    vec4 Position; // ignore w
    vec4 Color;    // w is intensity
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

layout(push_constant) uniform Push 
{
    mat4 ModelMatrix;
    mat4 NormalMatrix;

} uPush;

// ================================================================================================

// Input
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inTangent;
layout(location = 3) in vec3 inColor;
layout(location = 4) in vec2 inUV;

// Output
layout(location = 0) out vec3 fragPosWorld;
layout(location = 1) out vec3 fragNormalWorld;
layout(location = 2) out vec3 fragColor;
layout(location = 3) out vec2 fragUV;

// ================================================================================================

void main() 
{
    vec4 positionWorld = uPush.ModelMatrix * vec4(inPosition, 1.0); // position is a column vector

    gl_Position = uUbo.Projection * uUbo.View * positionWorld;
    
    fragNormalWorld = normalize(mat3(uPush.NormalMatrix) * inNormal);
    fragPosWorld    = positionWorld.xyz;
    fragColor       = inColor;
    fragUV          = inUV;
}
