float linearDepth(float depth, float znear, float zfar)
{
    float z = depth * 2.0f - 1.0f;
    return (2.0f * znear * zfar) / (zfar + znear - z * (zfar - znear));
}

float3 ReconstructWorldPos(float2 uv, float linearDepth, float3 lt, float3 l2r, float3 t2b, float iznear)
{
    uv.y = 1.0f - uv.y;
    
    float zscale = linearDepth * iznear;
    float3 viewpos = lt + l2r * uv.x + t2b * uv.y;
    
    return viewpos * zscale;
}

float3 calculateNormal(float3 normal, float3 tangent, Texture2D textureNormal, SamplerState samplerNormal, float2 uv)
{
    float3 tangentNormal = textureNormal.Sample(samplerNormal, uv).rgb * 2.0f - float3(1.0f, 1.0f, 1.0f);
    
    float3 N = normalize(normal);
    float3 T = normalize(tangent);
    float3 B = normalize(cross(N, T));
    float3x3 TBN = transpose(float3x3(T, B, N));
    return normalize(mul(TBN, tangentNormal));
}