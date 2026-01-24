#version 450

// ================================================================================================
// Structs

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

// ================================================================================================
// Constants

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
    vec4  Position;
    vec4  Color;
    float Radius;

} uPush;

// ================================================================================================
// Inputs

layout (location = 0) in  vec2 fragOffset;

// ================================================================================================
// Outputs

layout (location = 0) out vec4 outColor;

// ================================================================================================

const float M_PI = 3.1415926538;

// ================================================================================================

// void main()
// {
//     float dist = length(fragOffset);
    
//     if (dist >= 1.0) 
//     {
//         discard;
//     }

//     outColor = uPush.Color;
// }

void main() 
{
    float offsetDistance = dot(fragOffset, fragOffset);

    if (offsetDistance >= 1.0) 
    {
        discard;
    }

    float cosDistance = 0.5 * (cos(offsetDistance * M_PI) + 1.0); // ranges from 1 -> 0

    outColor = vec4(uPush.Color.xyz + 0.5 * cosDistance, cosDistance);
}
