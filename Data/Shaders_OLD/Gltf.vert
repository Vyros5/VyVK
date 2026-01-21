// Gltf.vert
#version 450
#extension GL_KHR_vulkan_glsl : enable

//— Vertex inputs
layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;
layout(location = 3) in vec4 inColor;
layout(location = 4) in vec4 inTangent; // .xyz = tangent, .w = bitangent sign

//— Scene UBO (set 0)
struct PointLight 
{
    vec4 position;
    vec4 color;
};

struct CameraData
{
    mat4 Projection;
    mat4 View;
    mat4 InverseView;
};

layout(set = 0, binding = 0) uniform GlobalUBO 
{
    CameraData       Camera;

    vec4             AmbientLightColor; // rgb = color, a = intensity
    vec4             LightDirection;
    PointLight       PointLights[ 10 ];
    int              NumPointLights;

} uUbo;


layout(set = 1, binding = 0) uniform Push 
{
    mat4 ModelMatrix;
    mat4 NormalMatrix; // inverse-transpose of model

} uPush;

//— Outputs to fragment
layout(location = 0) out vec3  fragNormal;
layout(location = 1) out vec4  fragColor;
layout(location = 2) out vec2  fragUV;
layout(location = 3) out vec3  fragViewVec;
layout(location = 4) out vec3  fragLightVec;
layout(location = 5) out vec4  fragTangent;
layout(location = 6) out vec4  fragLightColor;

void main() 
{
    // world-space position
    vec4 worldPos = uPush.ModelMatrix * vec4(inPos, 1.0);
    gl_Position   = uUbo.Camera.Projection * uUbo.Camera.View * worldPos;

    // normals & tangents in world-space
    fragNormal   = normalize(mat3(uPush.NormalMatrix) * inNormal);
    fragTangent  = normalize(uPush.NormalMatrix * inTangent); 
    // color & UV
    fragColor    = inColor;
    fragUV       = inUV;

    // view & light vectors
    vec3 camPos    = (uUbo.Camera.InverseView * vec4(0.0, 0.0, 0.0, 1.0)).xyz;
    vec3 lightPos  = uUbo.PointLights[0].position.xyz;

    fragViewVec    = camPos   - worldPos.xyz;
    fragLightVec   = lightPos - worldPos.xyz;
    fragLightColor = uUbo.PointLights[0].color;
}