
struct VSInput
{
    [[vk::location(0)]] float3 Pos     : POSITION0;
    [[vk::location(1)]] float3 Normal  : NORMAL0;
    [[vk::location(2)]] float2 UV      : TEXCOORD0;
    [[vk::location(3)]] float4 Tangent : TEXCOORD1;
};

struct VSOutput
{
    float4 Pos                          : SV_Position;
    [[vk::location(0)]] float3 Normal   : NORMAL0;
    [[vk::location(1)]] float2 UV       : TEXCOORD0;
    [[vk::location(2)]] float3 Tangent  : TEXCOORD1;
    [[vk::location(3)]] float3 Position : TEXCOORD2;
};

struct PBRVSOutput
{
    float4 Pos                              : SV_Position;
    [[vk::location(0)]] float3 WorldPos     : POSITION0;
    [[vk::location(1)]] float3 Normal       : NORMAL0;
    [[vk::location(2)]] float2 UV           : TEXCOORD0;
    [[vk::location(3)]] float3 Tangent      : TEXCOORD1;
    [[vk::location(4)]] float4 ShadowCoords : TEXCOORD2;
};

struct GBufferVSOutput
{
    float4 Pos                          : SV_Position;
    [[vk::location(0)]] float3 Normal   : NORMAL0;
    [[vk::location(1)]] float2 UV       : TEXCOORD0;
    [[vk::location(2)]] float3 Tangent  : TEXCOORD1;
    [[vk::location(3)]] float3 Position : TEXCOORD2;
    [[vk::location(4)]] float3 Color    : COLOR0;
};

#ifdef SUBPASS

    struct GBufferFSOutput
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
    struct GBufferFSOutput
    {
        float4 Albedo   : SV_TARGET0;
        float4 Normal   : SV_TARGET1;
        float4 Emmisive : SV_TARGET2;

    #ifndef CALC_POSITOIN
        float4 Position : SV_TARGET3;
    #endif
    };
#endif