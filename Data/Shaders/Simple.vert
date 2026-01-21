#version 450

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inColor;
layout (location = 2) in vec3 inNormal;
layout (location = 3) in vec4 inTangent; // w = handidness
layout (location = 4) in vec2 inUV;

layout (location = 0) out vec3 fragPosition;
layout (location = 1) out vec3 fragColor;
layout (location = 2) out vec3 fragNormal;
layout (location = 3) out vec4 fragTangent;
layout (location = 4) out vec2 fragUV;

struct PointLight 
{
    vec4 Position;  // ignore w
    vec4 Color;     // w is intensity
};

layout (set = 0, binding = 0) uniform UBO
{
    mat4             Projection;
    mat4             View;
    mat4             InverseView;
    vec4             AmbientLightColor;
    PointLight       PointLights[ 16 ];
    int              LightCount;

} uUbo;

layout (push_constant) uniform Push 
{
    mat4 ModelMatrix; 
    mat4 NormalMatrix;

} uPush;


void main() 
{
    vec4 positionWorld = uPush.ModelMatrix * vec4(inPosition, 1.0);
    
    gl_Position = uUbo.Projection * uUbo.View * positionWorld;

    fragPosition = positionWorld.xyz;
    fragNormal   = normalize(mat3(uPush.NormalMatrix) * inNormal);
    fragTangent  = normalize(mat4(uPush.NormalMatrix) * inTangent);
    fragColor    = inColor;
    fragUV       = inUV;
} 