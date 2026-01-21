#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inColor;
layout(location = 3) in vec2 inUV;
layout(location = 4) in vec4 inTangent;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 3) out vec3 fragPosWorld;
layout(location = 4) out vec3 fragNormalWorld;
layout(location = 5) out vec4 fragTangentWorld;

// ================================================================================================

struct LightUBO 
{
    vec4  Position;           // xyz = position, w = unused
    vec4  Color;              // rgb = color,    a = intensity
    int   LightType;
    float SpotInnerConeAngle; // Inner cone in degrees
    float SpotOuterConeAngle; // Outer cone in degrees
    float _padding;           // for alignment
};

struct CameraData
{
    mat4 Projection;
    mat4 View;
    mat4 InverseView;
};


const int MAX_LIGHTS  = 10;

// ================================================================================================
// Uniforms

layout(set = 0, binding = 0) uniform GlobalUBO 
{
    CameraData      Camera;

    vec4            AmbientLightColor; // rgb = color, a = intensity

    mat4            LightSpaceMatrix;
    vec4            ShadowLightDirection;

    LightUBO        Lights[ MAX_LIGHTS ];
    int             NumLights;

} uUbo;


layout(push_constant) uniform Push 
{
    mat4 ModelMatrix;
    mat4 NormalMatrix;
    vec2 uvScale;
    vec2 uvOffset;
    float uvRotation;
    int hasNormalTexture;
    int hasMetallicRoughnessTexture;
    int debugMode;
    float metallicFactor;
    float roughnessFactor;

} uPush;


vec2 transformUV(vec2 uv, vec2 scale, vec2 offset, float rotation) 
{
    // Apply scale and offset first
    vec2 transformed = uv * scale + offset;

    // Apply rotation around (0.5, 0.5) pivot
    if (rotation != 0.0) 
    {
        vec2 pivot = vec2(0.5, 0.5);
        transformed -= pivot;

        float cosR = cos(rotation);
        float sinR = sin(rotation);
        mat2 rotMatrix = mat2(cosR, -sinR, sinR, cosR);
        transformed = rotMatrix * transformed;

        transformed += pivot;
    }

    return transformed;
}

void main() 
{
    // Transform to World Space
    vec4 positionWorld = uPush.ModelMatrix * vec4(inPosition, 1.0);

    gl_Position = uUbo.Camera.Projection * uUbo.Camera.View * positionWorld;

    fragColor = inColor;
    // fragTexCoord = inUV;
    fragTexCoord     = transformUV(inUV, uPush.uvScale, uPush.uvOffset, uPush.uvRotation);
    fragNormalWorld  = normalize(mat3(uPush.NormalMatrix) * inNormal);
    fragPosWorld     = positionWorld.xyz;
    fragTangentWorld = vec4(normalize(mat3(uPush.NormalMatrix) * inTangent.xyz), inTangent.w);
}