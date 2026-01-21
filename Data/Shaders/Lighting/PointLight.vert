#version 450

const vec2 OFFSETS[ 6 ] = vec2[]
(
    vec2(-1.0, -1.0), 
    vec2(-1.0,  1.0), 
    vec2( 1.0, -1.0), 
    vec2( 1.0, -1.0), 
    vec2(-1.0,  1.0), 
    vec2( 1.0,  1.0)
);

layout (location = 0) out vec2 fragOffset;

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

void main() 
{
    fragOffset = OFFSETS[ gl_VertexIndex ];

    vec3 cameraRightWorld = { uUbo.View[0][0], uUbo.View[1][0], uUbo.View[2][0] };
    vec3 cameraUpWorld    = { uUbo.View[0][1], uUbo.View[1][1], uUbo.View[2][1] };

    vec3 positionWorld = uPush.LightPosition.xyz
        + uPush.Radius * fragOffset.x * cameraRightWorld
        + uPush.Radius * fragOffset.y * cameraUpWorld;

    gl_Position = uUbo.Projection * uUbo.View * vec4(positionWorld, 1.0);
}

// void main()
// {
//     fragOffset                 = OFFSETS[ gl_VertexIndex ];
//     vec4 lightInCameraSpace    = uUbo.View * uPush.LightPosition;
//     vec4 positionInCameraSpace = lightInCameraSpace + uPush.Radius * vec4(fragOffset, 0.0, 0.0);
//     gl_Position                = uUbo.Projection * positionInCameraSpace;
// }