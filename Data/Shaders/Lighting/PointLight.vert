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

layout(push_constant) uniform Push 
{
    vec4  Position;
    vec4  Color;
    float Radius;

} uPush;

// ================================================================================================

// Output
layout (location = 0) out vec2 fragOffset;

// ================================================================================================
// Constants

const vec2 OFFSETS[6] = vec2[](
    vec2(-1.0, -1.0),
    vec2(-1.0,  1.0),
    vec2( 1.0, -1.0),
    vec2( 1.0, -1.0),
    vec2(-1.0,  1.0),
    vec2( 1.0,  1.0)
);

// ================================================================================================

void main() 
{
    fragOffset = OFFSETS[gl_VertexIndex];

    vec4 lightCameraSpace    = uUbo.View * uPush.Position;
    vec4 positionCameraSpace = lightCameraSpace + vec4(fragOffset, 0.0, 0.0) * uPush.Radius;

    gl_Position = uUbo.Projection * positionCameraSpace;
}