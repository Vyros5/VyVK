#version 450

// ================================================================================================

struct PointLight 
{
    vec4 Position; // xyz = position, w = unused
    vec4 Color;    // rgb = color,    a = intensity
};

struct SpotLight 
{
    vec4  Position;    // xyz = position,  w = unused
    vec4  Direction;   // xyz = direction, w = unused
    vec4  Color;       // rgb = color,     a = intensity
    float InnerCutoff; // cos of inner angle
    float OuterCutoff; // cos of outer angle
    float _pad0;
    float _pad1;
};

struct DirectionalLight 
{
    mat4 LightViewProj;
    vec4 Direction; // xyz = direction, w = unused
    vec4 Color;     // rgb = color,     a = intensity
};

struct CameraData
{
    mat4 Projection;
    mat4 View;
    mat4 InverseView;
};

const int MAX_POINT_LIGHTS  = 10;
const int MAX_DIRECT_LIGHTS = 1;
const int MAX_SPOT_LIGHTS   = 10;

// ================================================================================================
// Uniforms

layout(set = 0, binding = 0) uniform GlobalUBO 
{
    CameraData       Camera;

    vec4             AmbientLightColor; // rgb = color, a = intensity

    PointLight       PointLights      [ MAX_POINT_LIGHTS  ];
    DirectionalLight DirectionalLights[ MAX_DIRECT_LIGHTS ];
    SpotLight        SpotLights       [ MAX_SPOT_LIGHTS   ];

    int              NumPointLights;
    int              NumDirectionalLights;
    int              NumSpotLights;

} uUbo;

layout(push_constant) uniform Push 
{
    mat4 ModelMatrix;
    mat4 NormalMatrix;

} uPush;

// Shadow map texture
layout(set = 0, binding = 1) uniform sampler2D uShadowMap;

// Material textures
layout(set = 1, binding = 0) uniform sampler2D uAlbedoTexture;
layout(set = 1, binding = 1) uniform sampler2D uNormalTexture;
layout(set = 1, binding = 2) uniform sampler2D uRoughnessTexture;
layout(set = 1, binding = 3) uniform sampler2D uMetallicTexture;
layout(set = 1, binding = 4) uniform sampler2D uAOTexture;

const float PI = 3.14159265359;

// ================================================================================================
// Input

layout(location = 0) in vec3 fragPosWorld;
layout(location = 1) in vec3 fragNormalWorld;
layout(location = 2) in vec3 fragColor;
layout(location = 3) in vec2 fragUV;
layout(location = 4) in vec4 fragPosLightSpace;

// ================================================================================================
// Output

layout(location = 0) out vec4 outColor;

// ================================================================================================

float DistributionGGX(vec3 N, vec3 H, float roughness) 
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float denom = (NdotH2 * (a2 - 1.0) + 1.0);

    return a2 / (PI * denom * denom);
}

float GeometrySchlickGGX(float NdotV, float roughness) 
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float denom = NdotV * (1.0 - k) + k;

    return NdotV / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) 
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0) 
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

// pseudo random number generator
float rand(vec2 co) 
{
    return fract(sin(dot(co.xy, vec2(12.9898, 78.233))) * 43758.5453);
}

// PCF
float shadowCalculation(vec3 normal, vec3 lightDir) 
{
    vec3  projCoords   = fragPosLightSpace.xyz / fragPosLightSpace.w;
    vec2  uv           = projCoords.xy * 0.5 + 0.5;
    float currentDepth = projCoords.z ;
    float depth        = texture(uShadowMap, uv).x; 

    float bias = max(0.0002 * (1.0 - dot(normal, lightDir)), 0.00001);

    float shadow = 0.0;

    int  kernelSize = 2;
    vec2 texelSize = 1.0 / textureSize(uShadowMap, 0);

    for(int x = -kernelSize; x <= kernelSize; ++x)
    {
        for(int y = -kernelSize; y <= kernelSize; ++y)
        {
            vec2 randomOffset = vec2(rand(uv + vec2(x, y)), rand(uv - vec2(x, y))) * texelSize;

            float pcfDepth = texture(uShadowMap, uv + vec2(x, y) * texelSize + randomOffset).r;

            shadow += (currentDepth - bias) < pcfDepth ? 1.0 : 0.1;        
        }    
    }
    shadow /= (kernelSize * 2 + 1) * (kernelSize * 2 + 1);
    
    return shadow;
}

// Calculate tangent-bitangent-normal matrix for normal mapping
mat3 calculateTBN(vec3 normal, vec3 pos, vec2 uv) 
{
    vec3 dp1  = dFdx(pos);
    vec3 dp2  = dFdy(pos);
    vec2 duv1 = dFdx(uv);
    vec2 duv2 = dFdy(uv);
    
    vec3 dp2perp   = cross(dp2, normal);
    vec3 dp1perp   = cross(normal, dp1);
    vec3 tangent   = dp2perp * duv1.x + dp1perp * duv2.x;
    vec3 bitangent = dp2perp * duv1.y + dp1perp * duv2.y;
    
    float invmax = inversesqrt(max(dot(tangent, tangent), dot(bitangent, bitangent)));
    
    return mat3(tangent * invmax, bitangent * invmax, normal);
}

// vec3 getNormal() 
// {
//     vec3 normal = fragNormalWorld;
//     if (push.HasNormalMap == 1) 
//     {
//         vec3 tangentNormal = texture(uNormalTexture, fragUV).rgb * 2.0 - 1.0;
//         vec3 T = normalize(mat3(push.NormalMatrix) * vec3(1.0, 0.0, 0.0));
//         vec3 B = normalize(mat3(push.NormalMatrix) * vec3(0.0, 1.0, 0.0));
//         vec3 N = normalize(mat3(push.NormalMatrix) * fragNormalWorld);
//         mat3 TBN = mat3(T, B, N);
//         normal = normalize(TBN * tangentNormal);
//     }
//     return normalize(normal);
// }

void main() 
{
    vec3  albedo    = pow(texture(uAlbedoTexture, fragUV).rgb, vec3(2.2)); 
    float metallic  = texture(uMetallicTexture, fragUV).r;
    float roughness = clamp(texture(uRoughnessTexture, fragUV).r, 0.05, 1.0); 
    float ao        = texture(uAOTexture, fragUV).r;

    // Sample normal map.
    vec3 normalMap     = texture(uNormalTexture, fragUV).rgb;
    vec3 N             = normalize(fragNormalWorld);
    // vec3 surfaceNormal = N;

    // Use normal map if not a plain white texture.
    vec3 whiteTexel = vec3(1.0, 1.0, 1.0);
    
    if (length(normalMap - whiteTexel) > 0.01) 
    {
        normalMap     = normalize(normalMap * 2.0 - 1.0);
        mat3 TBN      = calculateTBN(N, fragPosWorld, fragUV);
        // surfaceNormal = normalize(TBN * normalMap);
        N = normalize(TBN * normalMap);
    }

    // vec3 N = normalize(getNormal());

    // View direction (Camera Position - Frag World Position)
    vec3 V = normalize(uUbo.Camera.InverseView[3].xyz - fragPosWorld);

    // Specular Color(?)
    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);

    vec3 Lo = vec3(0.0);
    for (int i = 0; i < uUbo.NumPointLights; ++i) 
    {
        PointLight light = uUbo.PointLights[i];
    
        vec3 L = normalize(light.Position.xyz - fragPosWorld);
        vec3 H = normalize(V + L);

        float dist        = length(light.Position.xyz - fragPosWorld);
        float attenuation = 1.0 / (dist * dist);
        vec3  radiance    = light.Color.rgb * light.Color.a * attenuation;

        // Cook-Torrance BRDF
        float NDF = DistributionGGX(N, H, roughness);
        float G   = GeometrySmith(N, V, L, roughness);
        vec3  F   = fresnelSchlick(max(dot(H, V), 0.0), F0);

        vec3  nominator   = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001; 
        vec3  specular    = nominator / denominator;

        // kS is the specular reflectance, kD is diffuse reflectance
        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - metallic;

        float NdotL = max(dot(N, L), 0.0);
        Lo += (kD * albedo / PI + specular) * radiance * NdotL;
    }

    DirectionalLight dirLight = uUbo.DirectionalLights[0];

    vec3 L_dir        = normalize(-vec3(dirLight.Direction));
    vec3 H_dir        = normalize(V + L_dir);
    vec3 radiance_dir = dirLight.Color.rgb * dirLight.Color.a;

    float NDF_dir = DistributionGGX(N, H_dir, roughness);
    float G_dir   = GeometrySmith(N, V, L_dir, roughness);
    vec3  F_dir   = fresnelSchlick(max(dot(H_dir, V), 0.0), F0);

    vec3  nominator_dir   = NDF_dir * G_dir * F_dir;
    float denominator_dir = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L_dir), 0.0) + 0.001;
    vec3  specular_dir    = nominator_dir / denominator_dir;

    vec3 kS_dir = F_dir;
    vec3 kD_dir = vec3(1.0) - kS_dir;
    kD_dir *= 1.0 - metallic;

    float NdotL_dir = max(dot(N, L_dir), 0.0);
    Lo += (kD_dir * albedo / PI + specular_dir) * radiance_dir * NdotL_dir;
    
    float shadow = shadowCalculation(N, vec3(dirLight.Direction));

    Lo = clamp(Lo, vec3(0.0), vec3(10.0)); 

    // Ambient Lighting
    vec3 ambient = (uUbo.AmbientLightColor.rgb * uUbo.AmbientLightColor.a) * albedo * ao;
    vec3 color   = ambient + Lo; 
    color *= shadow;

    // Gamma Correction
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0 / 2.2));
    
    outColor = vec4(color, 1.0);
}