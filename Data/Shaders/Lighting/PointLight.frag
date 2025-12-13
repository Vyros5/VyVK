#version 450

// ================================================================================================

struct PointLight
{
    vec4 Position;  // ignore w
    vec4 Color;     // w is intensity
};

// ================================================================================================
// Uniforms

layout(set = 0, binding = 0) uniform GlobalUBO 
{
    mat4       Projection;
    mat4       View;
    mat4       InverseView;
    
    vec4       AmbientLightColor; // w is intensity
    PointLight PointLights[10];
    int        NumPointLights;

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

void main()
{
    float dist = length(fragOffset);
    
    if (dist >= 1.0) 
    {
        discard;
    }

    outColor = uPush.Color;
}

// void main() 
// {
//     float offsetDistance = dot(fragOffset, fragOffset);

//     if (offsetDistance >= 1.0) 
//     {
//         discard;
//     }

//     float cosDistance = 0.5 * (cos(offsetDistance * M_PI) + 1.0); // ranges from 1 -> 0

//     outColor = vec4(uPush.Color.xyz + 0.5 * cosDistance, cosDistance);
// }
