#version 450

// ================================================================================================

struct PointLight 
{
    vec4 Position; // xyz = position, w = unused
    vec4 Color;    // rgb = color,    a = intensity
};

struct SpotLight 
{
    vec4  Position;    // xyz = position,  w = unused
    vec4  Direction;   // xyz = direction, w = unused
    vec4  Color;       // rgb = color,     a = intensity
    float InnerCutoff; // cos of inner angle
    float OuterCutoff; // cos of outer angle
    float _pad0;
    float _pad1;
};

struct DirectionalLight 
{
    vec4 Direction; // xyz = direction, w = unused
    vec4 Color;     // rgb = color,     a = intensity
};

struct CameraData
{
    mat4 Projection;
    mat4 View;
    mat4 InverseView;
};


const int MAX_POINT_LIGHTS  = 10;
const int MAX_DIRECT_LIGHTS = 10;
const int MAX_SPOT_LIGHTS   = 10;

// ================================================================================================
// Uniforms

layout(set = 0, binding = 0) uniform GlobalUBO 
{
    CameraData       Camera;

    vec4             AmbientLightColor; // rgb = color, a = intensity

    PointLight       PointLights      [ MAX_POINT_LIGHTS  ];
    DirectionalLight DirectionalLights[ MAX_DIRECT_LIGHTS ];
    SpotLight        SpotLights       [ MAX_SPOT_LIGHTS   ];
    int              NumPointLights;
    int              NumDirectionalLights;
    int              NumSpotLights;

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
// layout(location = 4) out vec4 fragPosLightSpace;

// ================================================================================================

void main() 
{
    vec4 positionWorld = uPush.ModelMatrix * vec4(inPosition, 1.0);

    gl_Position = uUbo.Camera.Projection * uUbo.Camera.View * positionWorld;

    fragNormalWorld = normalize(mat3(uPush.NormalMatrix) * inNormal);
    fragPosWorld    = positionWorld.xyz;
    fragColor       = inColor;

    // Apply texture scaling and offset.
    fragUV          = inUV; //* uPush.TextureScale + uPush.TextureOffset;

    // fragPosLightSpace = uPush.LightSpaceMatrix * positionWorld;
}