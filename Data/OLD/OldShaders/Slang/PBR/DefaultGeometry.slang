import "../Modules/Geometry";
import "../Modules/Constants";
import "../Modules/TBN";

// ================================================================================================

struct Material
{
    float3 Albedo;
    float3 Emissive;
    float  Metallic;
    float  Roughness;
    float  AO;
};

struct VSOut
{
    float4 Position : SV_Position;
    [vk::location(0)] float3 WorldPosition;
    [vk::location(1)] float3 WorldNormal;
    [vk::location(2)] float2 UV;
};

// ================================================================================================

[vk::push_constant] ConstantBuffer<Geometry::Push>   push;
[vk::binding(0, 0)] ConstantBuffer<Geometry::Global> global;

[vk::binding(0, 1)] ConstantBuffer<Material> material;
[vk::binding(1, 1)] Sampler2D albedoMap;
[vk::binding(2, 1)] Sampler2D normalMap;
[vk::binding(3, 1)] Sampler2D metalRoughnessMap;
[vk::binding(4, 1)] Sampler2D aoMap;
[vk::binding(5, 1)] Sampler2D emissiveMap;

// ================================================================================================

[shader("vertex")]
func vertexMain(in Geometry::VertexIn input, out VSOut output)
{
    float4 worldPosition = mul(push.Model, float4(input.Position, 1.0));

    output.Position      = mul(global.Projection, mul(global.View, worldPosition));
    output.WorldPosition = worldPosition.xyz;
    output.WorldNormal   = normalize(mul((float3x3)push.Normal, input.Normal));
    output.UV            = input.UV;
}

// ================================================================================================

[shader("fragment")]
func fragmentMain(in VSOut input, out Geometry::GBuffer output)
{
    float3 albedo           = albedoMap.Sample(input.UV).rgb * material.Albedo;
    float3 normal           = normalMap.Sample(input.UV).rgb * 2.0 - 1.0;
    float3 emissive         = emissiveMap.Sample(input.UV).rgb * material.Emissive;
    float2 metalicRoughness = metalRoughnessMap.Sample(input.UV).bg;
    float  metallic         = metalicRoughness.r * material.Metallic;
    float  roughness        = metalicRoughness.g * material.Roughness;
    float  ao               = aoMap.Sample(input.UV).r * material.AO;

    let    tbn = TBN::calcMatrix(input.WorldPosition, input.WorldNormal, input.UV);
    float3 N   = normalize(length(normal) < 0.1 ? input.WorldNormal : mul(normal, tbn));

    output.Position = float4(input.WorldPosition, 1.0);
    output.Normal   = float4(N, 0.0);
    output.Albedo   = float4(albedo, 1.0);
    output.ARM      = float4(ao, roughness, metallic, 0.0);
    output.Emissive = float4(emissive, 1.0);
}

// ================================================================================================
