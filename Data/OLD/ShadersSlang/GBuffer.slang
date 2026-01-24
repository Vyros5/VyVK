import "../Modules/Geometry";

// ================================================================================================

struct VSInput
{
    [[vk::location(0)]] float3 Position : POSITION0;
    [[vk::location(1)]] float3 Normal   : NORMAL0;
    [[vk::location(2)]] float2 UV       : TEXCOORD0;
    [[vk::location(3)]] float4 Tangent  : TEXCOORD1;
};

struct VertexOut
{
                        float4 Pos      : SV_Position;
    [[vk::location(0)]] float3 Normal   : NORMAL0;
    [[vk::location(1)]] float2 UV       : TEXCOORD0;
    [[vk::location(2)]] float3 Tangent  : TEXCOORD1;
    [[vk::location(3)]] float3 Position : TEXCOORD2;
    [[vk::location(4)]] float3 Color    : COLOR0;
};

#ifdef SUBPASS

struct FSOutput
{
    float4 Color    : SV_TARGET0;
    float4 Albedo   : SV_TARGET1;
    float4 Normal   : SV_TARGET2;
    float4 Emmisive : SV_TARGET3;
#ifndef CALC_POSITOIN
    float4 Position : SV_TARGET4;
#endif
};

#else
struct FSOutput
{
    float4 Albedo   : SV_TARGET0;
    float4 Normal   : SV_TARGET1;
    float4 Emmisive : SV_TARGET2;
#ifndef CALC_POSITOIN
    float4 Position : SV_TARGET3;
#endif
};
#endif

struct PointLight 
{
    float4 Position; // xyz = position, w = unused
    float4 Color;    // rgb = color,    a = intensity
};

struct SpotLight 
{
    float4  Position;    // xyz = position,  w = unused
    float4  Direction;   // xyz = direction, w = unused
    float4  Color;       // rgb = color,     a = intensity
    float   InnerCutoff; // cos of inner angle
    float   OuterCutoff; // cos of outer angle
    float   _pad0;
    float   _pad1;
};

struct DirectionalLight 
{
    float4 Direction; // xyz = direction, w = unused
    float4 Color;     // rgb = color,     a = intensity
};

struct CameraData
{
    float4x4 Projection;
    float4x4 View;
    float4x4 InverseView;
};

struct GlobalUBO
{
    CameraData Camera;

    float4 AmbientLightColor;
    float4 ViewPos;
    float4 CamereInfo;
    float4 Size;
    PointLight PointLights[10];
    int NumLights;
};

// ================================================================================================

[vk::push_constant] ConstantBuffer<Geometry::Push> uPush;
[vk::binding(0, 0)] ConstantBuffer<GlobalUBO>      uGlobal;

[shader("vertex")]
func vertexMain(in VSInput input, out VertexOut output)
{
    output.Position = mul(uPush.Model, float4( input.Position, 1.0)).xyz;
    output.Pos      = mul(uGlobal.Camera.Projection, mul(uGlobal.Camera.View, float4(output.Position, 1.0)));
    
    float3x3 mat = (float3x3)uPush.Model;

    output.Normal  = normalize(mul(mat, input.Normal));
    output.UV      = input.UV;
    output.Tangent = normalize(mul(mat, input.Tangent.xyz));
}


[vk::binding(0, 1)] Sampler2D albedoMapSampler;
[vk::binding(1, 1)] Sampler2D normalMapSampler;
[vk::binding(2, 1)] Sampler2D roughnessMapSampler;
[vk::binding(3, 1)] Sampler2D emissiveMapSampler;

float3 calculateNormal(VertexOut input)
{
    float3 tangentNormal = normalMapSampler.Sample(input.UV).rgb * 2.0 - 1.0;

	float3   N   = normalize(input.Normal);
	float3   T   = normalize(input.Tangent);
	float3   B   = normalize(cross(N, T));
	float3x3 TBN = transpose(float3x3(T, B, N));

	return normalize(mul(TBN, tangentNormal));
}


float linearDepth(float depth, float znear, float zfar)
{
    float z = depth * 2.0f - 1.0f;

    return (2.0f * znear * zfar) / (zfar + znear - z * (zfar - znear));
}


[shader("fragment")]
func fragmentMain(in VertexOut input, out FSOutput output)
{
    float3 albedo    = albedoMapSampler.Sample(input.UV).rgb * input.Color;
    float3 emissive  = emissiveMapSampler.Sample(input.UV).rgb;
    float3 normal    = calculateNormal(input);
    float2 rm        = roughnessMapSampler.Sample(input.UV).bg;
    float  roughness = rm.x;
    float  metallic  = rm.y;

#ifdef SUBPASS 
    albedo       = input.Color;
    roughness    = 0.8f;
    metallic     = 0.2f;
    emissive     = float3(0, 0, 0);
    normal       = normalize(input.Normal);
    output.Color = float4(0, 0, 0, 1);
#endif

    output.Albedo   = float4(albedo, roughness);
    output.Normal   = float4(normal * 0.5f + float3(0.5f, 0.5f, 0.5f), metallic);
    // output.Normal   = float4(normal, metallic);
    output.Emmisive = float4(emissive, 1.0f);

#ifndef CALC_POSITION 
    float ldepth = linearDepth(input.Pos.z, uGlobal.CamereInfo.x, uGlobal.CamereInfo.y);
    
    output.Position = float4(input.Position, ldepth);
#endif
}