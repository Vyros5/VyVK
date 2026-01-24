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
    vec4 Color;

} uPush;

// ================================================================================================
// Output

layout(location = 0) out vec3 fragColor;

// ================================================================================================

vec3 getArrowVertex(int index)
{
    // Main shaft (line) - pointing in positive Z
    if (index == 0)  return vec3(0.0, 0.0, 0.0);
    if (index == 1)  return vec3(0.0, 0.0, 2.0);
    if (index == 2)  return vec3(0.0, 0.0, 2.0);
    if (index == 3)  return vec3(0.0, 0.0, 2.0);

    // Arrow head - pointing forward (positive Z)
    if (index == 4)  return vec3(0.0, 0.0, 2.0);
    if (index == 5)  return vec3(-0.3, 0.0, 1.6);
    if (index == 6)  return vec3(0.0, 0.0, 2.0);
    if (index == 7)  return vec3(0.3, 0.0, 1.6);
    if (index == 8)  return vec3(0.0, 0.0, 2.0);
    if (index == 9)  return vec3(0.0, -0.3, 1.6);
    if (index == 10) return vec3(0.0, 0.0, 2.0);
    if (index == 11) return vec3(0.0, 0.3, 1.6);

    // Cross bars for visibility
    if (index == 12) return vec3(-0.3, 0.0, 1.6);
    if (index == 13) return vec3(0.3, 0.0, 1.6);
    if (index == 14) return vec3(0.0, -0.3, 1.6);
    if (index == 15) return vec3(0.0, 0.3, 1.6);

    // Additional cross at origin for reference
    if (index == 16) return vec3(-0.15, 0.0, 0.0);
    if (index == 17) return vec3(0.15, 0.0, 0.0);

    return vec3(0.0);
}

void main()
{
    vec3 vertexPos = getArrowVertex(gl_VertexIndex);

    vec4 worldPos  = uPush.ModelMatrix * vec4(vertexPos, 1.0);
    
    gl_Position    = uUbo.Camera.Projection * uUbo.Camera.View * worldPos;
    
    fragColor      = uPush.Color.rgb;
}