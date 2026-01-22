#version 450

// ================================================================================================
// Input

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inColor;
layout (location = 2) in vec3 inNormal;
layout (location = 3) in vec2 inUV;
// layout (location = 4) in vec3 inTangent;
// layout (location = 5) in vec3 inBitangent;

// ================================================================================================
// Output

layout (location = 0) out vec3 fragPosWorld;
layout (location = 1) out vec3 fragNormalWorld;
layout (location = 2) out vec3 fragColor;
layout (location = 3) out vec2 fragUV;

// ================================================================================================
// Structs

struct PointLight 
{
    vec4 Position; // xyz = position, w = unused
    vec4 Color;    // rgb = color,    a = intensity
};

// ================================================================================================
// Uniforms

layout (set = 0, binding = 0) uniform GlobalUBO 
{
    mat4             Projection;
    mat4             View;
    mat4             InverseView;
    vec4             AmbientLightColor; // rgb = color, a = intensity
    PointLight       PointLights      [ 16 ];
    int              NumPointLights;

} uUbo;


layout (push_constant) uniform Push 
{
    mat4 ModelMatrix;
    mat4 NormalMatrix;

} uPush;

// ================================================================================================

void main() 
{
    vec4 positionWorld = uPush.ModelMatrix * vec4(inPosition, 1.0);

    gl_Position = uUbo.Projection * (uUbo.View * positionWorld);

    fragNormalWorld = normalize( mat3(uPush.NormalMatrix) * inNormal );
    fragPosWorld    = positionWorld.xyz;
    fragColor       = inColor;
    fragUV          = inUV;
}