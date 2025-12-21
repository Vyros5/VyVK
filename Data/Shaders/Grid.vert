#version 450

// ================================================================================================

struct PointLight 
{
    vec4 Position; // ignore w
    vec4 Color;    // w is intensity
};

struct CameraData
{
    mat4 Projection;
    mat4 View;
    mat4 InverseView;
};

// ================================================================================================
// Uniforms

layout(set = 0, binding = 0) uniform GlobalUBO 
{
    CameraData       Camera;

    // vec4             AmbientLightColor; // rgb = color, a = intensity

    // PointLight       PointLights      [ MAX_POINT_LIGHTS  ];
    // DirectionalLight DirectionalLights[ MAX_DIRECT_LIGHTS ];
    // SpotLight        SpotLights       [ MAX_SPOT_LIGHTS   ];
    // int              NumPointLights;
    // int              NumDirectionalLights;
    // int              NumSpotLights;

} uUbo;

// ================================================================================================

// Output
layout(location = 1) out vec3 nearPoint;
layout(location = 2) out vec3 farPoint;
layout(location = 3) out mat4 fragView;
layout(location = 7) out mat4 fragProj;

// ================================================================================================

// Grid position in clip space.
vec3 gridPlane[6] = vec3[] 
(
    vec3( 1,  1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
    vec3(-1, -1,  0), vec3( 1,  1,  0), vec3( 1, -1,  0)
);

vec3 unprojectPoint(float x, float y, float z, mat4 view, mat4 projection) 
{
    mat4 viewInv = inverse(view);
    mat4 projInv = inverse(projection);

    vec4 unprojectedPoint =  viewInv * projInv * vec4(x, y, z, 1.0);

    return unprojectedPoint.xyz / unprojectedPoint.w;
}

// ================================================================================================

void main() 
{
    vec3 point = gridPlane[gl_VertexIndex].xyz;

    // unprojecting on the near plane.
    nearPoint = unprojectPoint(point.x, point.y, 0.0, uUbo.Camera.View, uUbo.Camera.Projection).xyz;

    // unprojecting on the far plane.
    farPoint  = unprojectPoint(point.x, point.y, 1.0, uUbo.Camera.View, uUbo.Camera.Projection).xyz;
    
    fragView  = uUbo.Camera.View;
    fragProj  = uUbo.Camera.Projection;

    // Using directly the clipped coordinates.
    gl_Position = vec4(point.xyz, 1.0);
}