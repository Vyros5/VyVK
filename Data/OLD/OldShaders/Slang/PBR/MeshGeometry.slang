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

struct MSOut
{
    float4 Position : SV_Position;
    float3 WorldPosition;
    float3 WorldNormal;
    float2 UV;
};

// ================================================================================================

static const uint NUM_THREADS    = 32;
static const uint MAX_VERTICES   = 64;
static const uint MAX_PRIMITIVES = 126;

// ================================================================================================

[vk::push_constant] ConstantBuffer<Geometry::Push>   push;
[vk::binding(0, 0)] ConstantBuffer<Geometry::Global> global;

[vk::binding(0, 1)] ConstantBuffer<Material> material; // Binding Point: 0, Descriptor Set: 1
[vk::binding(1, 1)] Sampler2D albedoMap;
[vk::binding(2, 1)] Sampler2D normalMap;
[vk::binding(3, 1)] Sampler2D metalRoughnessMap;
[vk::binding(4, 1)] Sampler2D aoMap;
[vk::binding(5, 1)] Sampler2D emissiveMap;

[vk::binding(0, 2)] StructuredBuffer<Geometry::Meshlet> meshlets;
[vk::binding(0, 3)] StructuredBuffer<uint> vertexIndices;
[vk::binding(1, 3)] StructuredBuffer<uint8_t, ScalarDataLayout> primitives;
[vk::binding(2, 3)] StructuredBuffer<Geometry::Vertex, ScalarDataLayout> vertices;

// ================================================================================================

[shader("mesh")]
[numthreads(NUM_THREADS, 1, 1)]
[outputtopology("triangle")]
func meshMain(
    uint3 groupID  : SV_GroupID,
    uint3 threadID : SV_GroupThreadID,
    out vertices MSOut meshVertices[MAX_VERTICES],
    out indices  uint3 meshPrimitives[MAX_PRIMITIVES])
{
    var meshlet = meshlets[groupID.x];
    SetMeshOutputCounts(meshlet.VertexCount, meshlet.PrimitiveCount);

    float4x4 viewProjection = mul(global.Projection, global.View);

    // Emit vertices
    for (uint i = threadID.x; i < uint(meshlet.VertexCount); i += NUM_THREADS)
    {
        uint index  = vertexIndices[meshlet.VertexOffset + i];
        let  vertex = vertices[index];

        float4 worldPos               = mul(push.Model, float4(vertex.Position, 1.0));
        meshVertices[i].Position      = mul(viewProjection, worldPos);
        meshVertices[i].WorldPosition = worldPos.xyz;
        meshVertices[i].WorldNormal   = normalize(mul(float3x3(push.Normal), vertex.Normal));
        meshVertices[i].UV            = vertex.UV;
    }

    // Emit primitives
    for (uint i = threadID.x; i < uint(meshlet.PrimitiveCount); i += NUM_THREADS)
    {
        uint offset       = meshlet.PrimitiveOffset + i * 3;
        meshPrimitives[i] = uint3(primitives[offset + 0], primitives[offset + 1], primitives[offset + 2]);
    }
}

// ================================================================================================

[shader("fragment")]
func fragmentMain(MSOut input, out Geometry::GBuffer output) 
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
