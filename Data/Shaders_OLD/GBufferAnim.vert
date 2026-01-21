#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive    : enable

#include "macros.hlsl"

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV0;
layout(location = 3) in vec3 inTangent;
layout(location = 4) in vec3 inColor;

layout(location = 0) out vec3 fragNormal;
layout(location = 1) out vec2 fragUV;
layout(location = 2) out vec3 fragTangent;
layout(location = 3) out vec3 fragPosition;
layout(location = 4) out vec3 fragColor;

struct PointLight
{
    vec4 position; // ignore w
    vec4 color;    // w is intensity
};

struct CameraData
{
    mat4 Projection;
    mat4 View;
    mat4 InverseView;
};

layout(set = 0, binding = 0) uniform UniformBufferObject
{
    CameraData Camera;

    vec4 ambientLightColor;
    vec4 viewPos;
    vec4 camereInfo;
    vec4 size;
    PointLight pointLights[10];
    int numLights;
    
} uUbo;

#ifdef CPU_ANIM
    layout(set = 2, binding = 0) uniform NodeMatrices 
    {
        mat4 NodeMatrice;

    } uNodeUbo;
#endif

layout(push_constant) uniform Push
{
    mat4 ModelMatrix;
    mat4 NormalMatrix;

} uPush;


void main()
{
#ifdef CPU_ANIM
    mat4 model = uPush.ModelMatrix * uNodeUbo.NodeMatrice;
#else
    mat4 model = uPush.ModelMatrix;
#endif
    vec4 position = (model * vec4(inPosition, 1.0));
    fragPosition  = position.xyz;
    mat3 mat      = mat3(model);
    mat           = transpose(inverse(mat3(mat)));

    fragUV        = inUV0;
    fragNormal    = normalize(mat * inNormal);
    fragTangent   = normalize(mat * inTangent);
    fragColor     = inColor;
    
    gl_Position   = uUbo.Camera.Projection * uUbo.Camera.View * position;
}