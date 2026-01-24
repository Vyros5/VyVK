struct VSOutput
{
    float4 Pos : SV_Position;
    [[vk::location(0)]] float3 WorldPos : POSITION0;
    [[vk::location(1)]] float3 Normal : NORMAL0;
    [[vk::location(2)]] float2 UV : TEXCOORD0;
    [[vk::location(3)]] float3 Tangent : TEXCOORD1;
    [[vk::location(4)]] float4 ShadowCoords : TEXCOORD2;
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

struct ShadowUBO
{
    float4x4 lightVP;
    float4 shadowBias; // (normalbias, w, 0, 0)
};

cbuffer shadowUbo : register(b0, space2)
{
    ShadowUBO shadowUbo;
}

struct PushConsts
{
    float4x4 modelMatrix;
    float4x4 normalMatrix;
};

[[vk::push_constant]] PushConsts pushConsts;

Texture2D textureColor : register(t0, space1);
SamplerState samplerColor : register(s0, space1);

Texture2D textureNormal : register(t1, space1);
SamplerState samplerNormal : register(s1, space1);

Texture2D textureRoughness : register(t2, space1);
SamplerState samplerRoughness : register(s2, space1);

Texture2D textureEmissive : register(t3, space1);
SamplerState samplerEmissive : register(s3, space1);

Texture2D textureShadow : register(t1, space2);
SamplerState samplerShadow : register(s1, space2);

#include "shadow.hlsl"
#include "Lighting.hlsl"
#include "common.hlsl"

float4 main(VSOutput input) :SV_TARGET
{
    float3 albedo = textureColor.Sample(samplerColor, input.UV).rgb;
    float3 emmisive = textureEmissive.Sample(samplerEmissive, input.UV).rgb;
    albedo = pow(albedo, float3(2.2f, 2.2f, 2.2f));
    
    float3 normal = calculateNormal(input.Normal, input.Tangent, textureNormal, samplerNormal, input.UV);
    float2 rm = textureRoughness.Sample(samplerRoughness, input.UV).gb;
    float roughness = rm.x;
    float metallic = rm.y;
    
    float3 viewDir = normalize(ubo.viewPos.xyz - input.WorldPos);
    
    float3 F0 = float3(0.04f, 0.04f, 0.04f);
    F0 = lerp(F0, albedo, metallic);
    
    //float4 shadowCoords = mul(shadowUbo.lightVP, float4(input.WorldPos + shadowUbo.shadowBias.x * normal, 1.0));
    float4 shadowCoords = input.ShadowCoords;
    float shadow = getShadow(shadowCoords, textureShadow, samplerShadow);
    float3 Lo = DirectLighting(normal, viewDir, albedo, F0, roughness, metallic, 1, input.WorldPos);
   
    Lo = pow(Lo, float3(0.45f, 0.45f, 0.45f));
    return float4(Lo + emmisive, 1.0f);

}