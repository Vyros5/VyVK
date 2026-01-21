#define PI 3.141592653589793
#define PI2 6.283185307179586
#define EPS 1e-3

float D_GGX_TR(float ndoth, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;

    float nom = a2;
    float denom = ndoth * ndoth * (a2 - 1.0) + 1.0;
    denom = PI * denom * denom;

    return nom / max(denom, 0.0001f);
}

float GeometrySchlickGGX(float dotp, float k)
{
    float nom = dotp;
    float denom = dotp * (1.0 - k) + k;
    return nom / denom;
}

float GeometrySmith(float ndotv, float ndotl, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0f;

    return GeometrySchlickGGX(ndotv, k) * GeometrySchlickGGX(ndotl, k);
}

float3 fresnelSchlick(float cosTheta, float3 F0)
{
    return F0 + (1.0f - F0) * pow(clamp(1.0f - cosTheta, 0.0f, 1.0f), 5.0f);
}

float3 F_SchlickR(float cosTheta, float3 F0, float roughness)
{
    float smothness = 1.0 - roughness;
    return F0 + (max(float3(smothness, smothness, smothness), F0) - F0) * pow(1.0f - cosTheta, 5.0f);
}

float3 DirectLighting(float3 n, float3 v, float3 albedo, float3 F0, float roughness, float metallic, float shadow, float3 worldPos)
{
    float ndotv = clamp(dot(n, v), 0.0, 1.0);
    float3 f = fresnelSchlick(ndotv, F0);
    float3 Lo = float3(0.0f, 0.0f, 0.0f);
    
    for (int i = 0; i < ubo.numLights; i++)
    {
        float3 l = normalize(ubo.pointLights[i].position.xyz - worldPos);
        float ndotl = clamp(dot(n, l), 0.0, 1.0);
        if (ndotl > 0.0)
        {
            float3 lightColor = ubo.pointLights[i].color.rgb; 
            float3 h = normalize(v + l);
            float ndoth = clamp(dot(n, h), 0.0, 1.0);
            float ndf = D_GGX_TR(ndoth, roughness);
            float g = GeometrySmith(ndotv, ndotl, roughness);

            float3 nom = ndf * g * f;
            float denom = 4 * ndotv * ndotl + 0.001f;
            float3 specular = nom / denom;

            float3 ks = f;
            float3 kd = (float3(1.0f, 1.0f, 1.0f) - ks);
            kd *= (1.0f - metallic);

            float sha = i == 0.0f ? shadow : 1.0f;
            Lo += sha * lightColor * (kd * albedo / PI + specular) * ndotl;
        }
    }

    return Lo;
}