import "Modules/Geometry";
import "Modules/Constants";
import "Modules/TBN";
import "Modules/Bindless";

static const uint NUM_THREADS    = 32;
static const uint MAX_VERTICES   = 64;
static const uint MAX_PRIMITIVES = 126;

struct MSOut
{
    float4 Position: SV_Position;
    float3 WorldPosition;
    float3 WorldNormal;
    float2 UV;
};

struct Material
{
    float3           Albedo;
    float3           Emissive;
    float            Metallic;
    float            Roughness;
    float            AO;
    Bindless::Handle AlbedoMap;
    Bindless::Handle NormalMap;
    Bindless::Handle MetalRoughnessMap;
    Bindless::Handle AOMap;
    Bindless::Handle EmissiveMap;
};

struct Mesh
{
    Bindless::Handle Vertex;
    Bindless::Handle Meshlet;
    Bindless::Handle MeshletIndex;
    Bindless::Handle MeshletPrimitive;
};

public struct Push
{
    float3x4                            ModelRaw;
    float3 normalRow0; Bindless::Handle Global;
    float3 normalRow1; Bindless::Handle Mesh;
    float3 normalRow2; Bindless::Handle Material;

    property float4x4 ModelMatrix
    {
        get { return float4x4(ModelRaw, float4(0.0, 0.0, 0.0, 1.0)); }
    }

    property float3x3 NormalMatrix
    {
        get { return float3x3(normalRow0, normalRow1, normalRow2); }
    }
};

[vk::push_constant] ConstantBuffer<Push> push;

[vk::binding(Bindless::STORAGE_BUFFER)] StructuredBuffer<Geometry::Meshlet>                  meshletBuffer[];
[vk::binding(Bindless::STORAGE_BUFFER)] StructuredBuffer<uint>                               indexBuffer[];
[vk::binding(Bindless::STORAGE_BUFFER)] StructuredBuffer<uint8_t, ScalarDataLayout>          primitiveBuffer[];
[vk::binding(Bindless::STORAGE_BUFFER)] StructuredBuffer<Geometry::Vertex, ScalarDataLayout> vertexBuffer[];

[vk::binding(Bindless::UNIFORM_BUFFER)] ConstantBuffer<Geometry::Global, Std430DataLayout> globalBuffer[];
[vk::binding(Bindless::UNIFORM_BUFFER)] ConstantBuffer<Material, Std430DataLayout>         materialBuffer[];
[vk::binding(Bindless::UNIFORM_BUFFER)] ConstantBuffer<Mesh, Std430DataLayout>             meshBuffer[];

// Mesh Shader --------------------

[shader("mesh")]
[numthreads(NUM_THREADS, 1, 1)]
[outputtopology("triangle")]
func meshMain(
    uint3 groupID: SV_GroupID,
    uint3 threadID: SV_GroupThreadID,
    out vertices MSOut meshVertices[MAX_VERTICES],
    out indices uint3 meshPrimitives[MAX_PRIMITIVES])
{
    let global  = push.Global.get(globalBuffer);
    let mesh    = push.Mesh.get(meshBuffer);
    let meshlet = mesh.Meshlet.get(meshletBuffer)[groupID.x];

    SetMeshOutputCounts(meshlet.VertexCount, meshlet.PrimitiveCount);

    float4x4 viewProjection = mul(global.Projection, global.View);

    // Emit vertices
    for (uint i = threadID.x; i < uint(meshlet.VertexCount); i += NUM_THREADS)
    {
        uint index = mesh.MeshletIndex.get(indexBuffer)[meshlet.VertexOffset + i];
        let  v     = mesh.Vertex.get(vertexBuffer)[index];

        float4 worldPos = mul(push.ModelMatrix, float4(v.Position, 1.0));

        meshVertices[i].Position      = mul(viewProjection, worldPos);
        meshVertices[i].WorldPosition = worldPos.xyz;
        meshVertices[i].WorldNormal   = normalize(mul(push.NormalMatrix, v.Normal));
        meshVertices[i].UV            = v.UV;
    }

    // Emit primitives
    for (uint i = threadID.x; i < uint(meshlet.PrimitiveCount); i += NUM_THREADS)
    {
        uint offset = meshlet.PrimitiveOffset + i * 3;

        meshPrimitives[i] = uint3(
            mesh.MeshletPrimitive.get(primitiveBuffer)[offset + 0], 
            mesh.MeshletPrimitive.get(primitiveBuffer)[offset + 1],
            mesh.MeshletPrimitive.get(primitiveBuffer)[offset + 2]
        );
    }
}

// Fragment Shader --------------------

[shader("fragment")]
func fragmentMain(MSOut input, out Geometry::GBuffer output)
{
    let mat = push.Material.get(materialBuffer);

    float3 Albedo           = mat.AlbedoMap.Sample(input.UV).rgb * mat.Albedo;
    float3 Normal           = mat.NormalMap.Sample(input.UV).rgb * 2.0 - 1.0;
    float3 Emissive         = mat.EmissiveMap.Sample(input.UV).rgb * mat.Emissive;
    float2 MetalicRoughness = mat.MetalRoughnessMap.Sample(input.UV).bg;
    float  Metallic         = MetalicRoughness.r * mat.Metallic;
    float  Roughness        = MetalicRoughness.g * mat.Roughness;
    float  AO               = mat.AOMap.Sample(input.UV).r * mat.AO;

    let tbn  = TBN::calcMatrix(input.WorldPosition, input.WorldNormal, input.UV);
    float3 N = normalize(length(Normal) < 0.1 ? input.WorldNormal : mul(Normal, tbn));

    output.Position = float4(input.WorldPosition, 1.0);
    output.Normal   = float4(N, 0.0);
    output.Albedo   = float4(Albedo, 1.0);
    output.ARM      = float4(AO, Roughness, Metallic, 0.0);
    output.Emissive = float4(Emissive, 1.0);
}