#version 450

// ================================================================================================
// Output

layout(location = 0) out vec3 fragColor;

// ================================================================================================
// Structs

struct SpotLight 
{
    vec4  Position;    // xyz = position,  w = unused
    vec4  Direction;   // xyz = direction, w = unused
    vec4  Color;       // rgb = color,     a = intensity
    float OuterCutoff; // cos of outer angle
    float constantAtten;
    float linearAtten;
    float quadraticAtten;
};

// ================================================================================================
// Uniforms

layout(set = 0, binding = 0) uniform GlobalUBO 
{
    mat4      Projection;
    mat4      View;
    vec4      AmbientLightColor;
    vec4      CameraPosition;
    vec4      PointLights[16 * 2];       // Placeholder
    vec4      DirectionalLights[16 * 2]; // Placeholder
    SpotLight SpotLights[16];
    // ... rest of UBO

} uUbo;

layout(push_constant) uniform Push
{
    mat4  ModelMatrix;
    vec4  Color;
    float ConeAngle; // Outer cutoff angle in radians

} uPush;

const float PI       = 3.14159265359;
const int   SEGMENTS = 32;


vec3 getConeVertex(int index, float angle)
{
    int triangleIndex = index / 3;
    int vertexIndex   = index % 3;

    // Apex
    if (vertexIndex == 0)
    {
        return vec3(0.0, 0.0, 0.0);
    }

    // Cone base circle at z = 2.0 (positive Z)
    float length = 2.0;
    float radius = tan(angle) * length;

    // For vertexIndex 1, use triangleIndex
    // For vertexIndex 2, use triangleIndex + 1
    int circleIndex = triangleIndex + (vertexIndex - 1);

    float theta = float(circleIndex) * 2.0 * PI / float(SEGMENTS);
    return vec3(radius * cos(theta), radius * sin(theta), length);
}

// ================================================================================================

void main()
{
    vec3 vertexPos = getConeVertex( gl_VertexIndex, uPush.ConeAngle );
    vec4 worldPos  = uPush.ModelMatrix * vec4(vertexPos, 1.0);
    gl_Position    = uUbo.Projection * uUbo.View * worldPos;
    fragColor      = uPush.Color.rgb;
}