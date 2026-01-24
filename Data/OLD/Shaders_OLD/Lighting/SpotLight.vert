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
    mat4  ModelMatrix;
    vec4  Color;
    float ConeAngle; // Outer cutoff angle in radians

} uPush;

vec3 getConeVertex(int index, float angle)
{
    // Cone apex at origin
    if (index == 0) return vec3(0.0, 0.0, 0.0);

    // Cone base circle (16 segments) at z = 2.0 (positive Z)
    float radius      = tan(angle) * 2.0;
    int   circleIndex = index - 1;

    if (circleIndex < 16)
    {
        float theta = float(circleIndex) * 2.0 * 3.14159265359 / 16.0;
        
        return vec3(radius * cos(theta), radius * sin(theta), 2.0);
    }

    // Lines from apex to circle (every other point for clarity)
    int lineIndex = circleIndex - 16;

    if (lineIndex < 8)
    {
        if (lineIndex % 2 == 0)
            return vec3(0.0, 0.0, 0.0); // Apex
        else
        {
            int   circlePoint = (lineIndex / 2) * 2;
            float theta       = float(circlePoint) * 2.0 * 3.14159265359 / 16.0;
            return vec3(radius * cos(theta), radius * sin(theta), 2.0);
        }
    }

  return vec3(0.0);
}

// ================================================================================================
// Input

// ================================================================================================
// Output

layout(location = 0) out vec3 fragColor;

// ================================================================================================

void main()
{
    vec3 vertexPos = getConeVertex(gl_VertexIndex, uPush.ConeAngle);
    vec4 worldPos  = uPush.ModelMatrix * vec4(vertexPos, 1.0);
    gl_Position    = uUbo.Camera.Projection * uUbo.Camera.View * worldPos;
    fragColor      = uPush.Color.rgb;
}