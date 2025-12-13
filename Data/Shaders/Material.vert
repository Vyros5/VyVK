#version 450

// ================================================================================================

struct PointLight 
{
    vec4 Position; // ignore w
    vec4 Color;    // w is intensity
};

struct SpotLight 
{
    vec4  Position;    // ignore w
    vec4  Direction;   // ignore w
    vec4  Color;       // w is intensity
    // float InnerCutoff; // cos of inner angle
    float OuterCutoff; // cos of outer angle
    float ConstantAtten;
    float LinearAtten;
    float QuadraticAtten;
};

struct DirectionalLight 
{
    vec4 Direction; // ignore w
    vec4 Color;     // w is intensity
};

// ================================================================================================
// Uniforms

layout(set = 0, binding = 0) uniform GlobalUBO 
{
    mat4             Projection;
    mat4             View;
    mat4             InverseView;

    vec4             AmbientLightColor; // w is intensity

    PointLight       PointLights[10];
    DirectionalLight DirectionalLights[10];
    SpotLight        SpotLights[10];
    mat4             LightSpaceMatrices[10];
    vec4             PointLightShadowData[4]; // xyz = position, w = far plane
    int              NumPointLights;
    int              NumSpotLights;
    int              NumDirectionalLights;
    int              ShadowLightCount;     // 2D shadow maps (directional + spot)
    int              CubeShadowLightCount; // Cube shadow maps (point lights)
    int              _pad1;
    int              _pad2;
    int              _pad3;
    vec4             FrustumPlanes[6];

} uUbo;

layout(push_constant) uniform Push 
{
    mat4 ModelMatrix;
    mat4 NormalMatrix;

    vec3  Albedo;
    float Metallic;
    float Roughness;
    float AO;
    vec2  TextureOffset;
    vec2  TextureScale;
    vec3  EmissionColor;
    float EmissionStrength;

} uPush;

// ================================================================================================

// Input
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
// layout(location = 2) in vec3 inTangent; // Unused atm
layout(location = 2) in vec3 inColor;
layout(location = 3) in vec2 inUV;

// Output
layout(location = 0) out vec3 fragPosWorld;
layout(location = 1) out vec3 fragNormalWorld;
layout(location = 2) out vec3 fragColor;
layout(location = 3) out vec2 fragUV;

// ================================================================================================

void main() 
{
    vec4 positionWorld = uPush.ModelMatrix * vec4(inPosition, 1.0);

    gl_Position = uUbo.Projection * uUbo.View * positionWorld;

    fragNormalWorld = normalize(mat3(uPush.NormalMatrix) * inNormal);
    fragPosWorld    = positionWorld.xyz;
    fragColor       = inColor;

    // Apply texture scaling and offset.
    fragUV          = inUV; //* uPush.TextureScale + uPush.TextureOffset;
    
}