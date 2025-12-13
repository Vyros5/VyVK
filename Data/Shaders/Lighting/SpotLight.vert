#version 450

layout(location = 0) out vec3 fragColor;

struct SpotLight
{
    vec4  position;
    vec4  direction;
    vec4  Color;
    float outerCutoff;
    float constantAtten;
    float linearAtten;
    float quadraticAtten;
};

// ================================================================================================
// Uniforms

layout(set = 0, binding = 0) uniform GlobalUBO 
{
    mat4       Projection;
    mat4       View;
    mat4       InverseView;

    vec4       AmbientLightColor; // w is intensity
    vec4       PointLights[16 * 2];       // Placeholder
    vec4       DirectionalLights[16 * 2]; // Placeholder
    SpotLight  SpotLights[16];

    // ... rest of UBO

} uUbo;

// ================================================================================================

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

void main()
{
    vec3 vertexPos = getConeVertex(gl_VertexIndex, uPush.ConeAngle);
    vec4 worldPos  = uPush.ModelMatrix * vec4(vertexPos, 1.0);
    gl_Position    = uUbo.Projection * uUbo.View * worldPos;
    fragColor      = uPush.Color.rgb;
}