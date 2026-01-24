module PBR;

import Constants;

namespace PBR
{
    public static const float F_DIELECTRIC = 0.04;


    public float lightAttenuation(float3 lightPos, float3 fragPos)
    {
        float distance = length(lightPos - fragPos);

        return 1.0 / max(distance * distance, Constants::EPSILON);
    }


    public float3 specularReflection(float VdotH, float3 F0)
    {
        return F0 + (float3(1.0) - F0) * pow(1.0 - VdotH, 5.0);
    }


    public float3 specularReflection(float VdotH, float3 F0, float roughness)
    {
        return F0 + (max(float3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - VdotH, 0.0, 1.0), 5.0);
    }


    float schlickGGX(float NdotX, float alpha)
    {
        // Schlick-Beckman GGX
        float k = alpha / 2.0;
        return NdotX / max(NdotX * (1.0 - k) + k, Constants::EPSILON);
    }


    float normalDistribution(float NdotH, float alpha)
    {
        // Trowbridge-Reitz GGX
        float alpha2 = alpha * alpha;
        float denom = (NdotH * NdotH) * (alpha2 - 1.0) + 1.0;
        return alpha2 / max(Constants::PI * denom * denom, Constants::EPSILON);
    }


    float geometryOcclusion(float NdotV, float NdotL, float alpha)
    {
        // Smith model with Schlick-GGX
        return schlickGGX(NdotV, alpha) * schlickGGX(NdotL, alpha);
    }


    public float3 computeLighting(float3 N, float3 V, float3 L, float3 albedo, float roughness, float metallic, float3 radiance, float3 F0)
    {
        float3 H = normalize(V + L);
        
        float alpha = roughness * roughness;
        float NdotV = max(dot(N, V), 0.0);
        float NdotL = max(dot(N, L), 0.0);
        float NdotH = max(dot(N, H), 0.0);
        float VdotH = max(dot(V, H), 0.0);

        float  D = normalDistribution(NdotH, alpha);
        float  G = geometryOcclusion(NdotV, NdotL, alpha);
        float3 F = specularReflection(VdotH, F0);

        float3 kS = F;
        float3 kD = (1.0 - kS) * (1.0 - metallic);

        float3 diffuse  = kD * albedo / Constants::PI;
        float3 specular = (D * G * F) / max(4.0 * NdotV * NdotL, Constants::EPSILON);

        return (diffuse + specular) * radiance * NdotL;
    }
}