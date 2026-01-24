struct VSInput
{
    [[vk::location(0)]] float3 Pos : POSITION0;
    [[vk::location(1)]] float3 Normal : NORMAL0;
    [[vk::location(2)]] float2 UV : TEXCOORD0;
    [[vk::location(3)]] float4 Tangent : TEXCOORD1;
};

struct VSOutput
{
    float4 Pos : SV_Position;
    [[vk::location(0)]] float3 Normal : NORMAL0;
    [[vk::location(1)]] float2 UV : TEXCOORD0;
    [[vk::location(2)]] float3 Tangent : TEXCOORD1;
    [[vk::location(3)]] float3 Position : TEXCOORD2;
};

struct PointLight
{
    float4 position; // ignore w
    float4 color; // w is intensity
};

struct GlobalUBO
{
    float4x4 projection;
    float4x4 view;
    float4 ambientLightColor;
    float4 viewPos;
    float4 camereInfo;
    float4 size;
    PointLight pointLights[10];
    int numLights;
};

cbuffer ubo : register(b0)
{
    GlobalUBO ubo;
}

struct PushConsts
{
    float4x4 modelMatrix;
    float4x4 normalMatrix;
};

[[vk::push_constant]] PushConsts pushConsts;

VSOutput main(VSInput input)
{
    VSOutput output = (VSOutput) 0;
    output.Position = mul(pushConsts.modelMatrix, float4(input.Pos, 1.0)).xyz;
    output.Pos = mul(ubo.projection, mul(ubo.view, float4(output.Position, 1.0)));
    float3x3 mat = (float3x3) pushConsts.modelMatrix;
    output.Normal = normalize(mul(mat, input.Normal));
    output.UV = input.UV;
    output.Tangent = normalize(mul(mat, input.Tangent.xyz));
    
    return output;
}