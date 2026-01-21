// Shared input structures for VyEngine shaders.

// Scene data uniform buffer (set 0, binding 0)
struct CameraData
{
    float4x4 Projection;
    float4x4 View;
    float4x4 InverseView;
};

struct SceneData
{
    CameraData Camera;

    float4 ambientColor;
};

// Vertex structure matching CPU-side layout
struct Vertex
{
    float3 Position;
    float3 Normal;
    float2 UV;
    float3 Color;
};

// Point light structure (matches C++ GPUPointLight)
struct PointLight
{
    float4 Position; // xyz = position, w = radius
    float4 Color;    // xyz = color, w = intensity
};


// Light data buffer structure (matches C++ GPULightData)
struct LightData
{
    uint NumPointLights;
    uint NumSpotLights;
    uint _pad0;
    uint _pad1;
    PointLight PointLights[10];  // MAX_POINT_LIGHTS
};