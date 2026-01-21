
float getShadow(float4 ShadowCoords, Texture2D shadowMap, SamplerState shadowSampler)
{
    ShadowCoords.xyz /= ShadowCoords.w;
    float depth = ShadowCoords.z;
    float2 uv = ShadowCoords.xy * 0.5f + float2(0.5f, 0.5f);
    bool inmap = uv.x <= 1.0f && uv.x >= 0.0f && uv.y <= 1.0f && uv.y >= 0.0f;
    float depthInMap = inmap ? shadowMap.Sample(shadowSampler, uv).r : 1.0f;

    return depth < depthInMap ? 1.0f : 0.0f;

}