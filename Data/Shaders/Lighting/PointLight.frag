#version 450

layout (location = 0) in vec2 fragOffset;

layout (location = 0) out vec4 outColor;

struct PointLight
{
    vec4 Position; // w is ignored
    vec4 Color;    // w component can be used for intensity
};

layout (set = 0, binding = 0) uniform GlobalUBO 
{
    mat4             Projection;
    mat4             View;
    mat4             InverseView;
    vec4             AmbientLightColor; // rgb = color, a = intensity
    PointLight       PointLights      [ 16 ];
    int              NumPointLights;

} uUbo;

layout (push_constant) uniform Push
{
    vec4  LightPosition;
    vec4  Color;
    float Radius;

} uPush;

const float M_PI = 3.1415926538;

void main()
{
    float dist = sqrt( dot( fragOffset, fragOffset ) );

    if (dist >= 1.0) 
    {
        discard;
    }

    float cosDis = 0.5 * (cos( dist * M_PI ) + 1.0);  // ranges from 1 -> 0

    outColor = vec4(uPush.Color.xyz + 0.5 * cosDis, cosDis);
}