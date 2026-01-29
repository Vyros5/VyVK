#version 450

// ================================================================================================
// CONSTANTS

#define MAX_POINT_LIGHTS 10
#define MAX_SPOT_LIGHTS  10

const float EPSILON = 0.00001;
const float PI = 3.14159265359;

const vec3 gridSamplingDisk[20] = vec3[]
(
    vec3(1, 1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1, 1,  1), 
    vec3(1, 1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
    vec3(1, 1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1, 1,  0),
    vec3(1, 0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1, 0, -1),
    vec3(0, 1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0, 1, -1)
);

const mat4 BIAS = mat4( 
    0.5, 0.0, 0.0, 0.0,
    0.0, 0.5, 0.0, 0.0,
    0.0, 0.0, 1.0, 0.0,
    0.5, 0.5, 0.0, 1.0 
);

// Tweakable multipliers    
// TODO: This should be configurable during runtime
const float ALBEDO_INTENSITY      = 1.0;
const float METALLIC_MULTIPLIER   = 1.0;
const float ROUGHNESS_MULTIPLIER  = 0.6;
const float AO_MULTIPLIER         = 0.04;
const float NORMAL_STRENGTH       = 1.0;
const float EMISSIVE_INTENSITY    = 0.2;

// ================================================================================================
// MAP FLAGS - (Must match the 'VyMaterialFlags' enum)

const uint HAS_ALBEDO_MAP             = 1 << 0;
const uint HAS_METALLIC_MAP           = 1 << 1;
const uint HAS_ROUGHNESS_MAP          = 1 << 2;
const uint HAS_METALLIC_ROUGHNESS_MAP = 1 << 3;
const uint HAS_NORMAL_MAP             = 1 << 4;
const uint HAS_AO_MAP                 = 1 << 5;
const uint HAS_EMISSIVE_MAP           = 1 << 6;
const uint HAS_SPECULAR_GLOSS_MAP     = 1 << 7;
const uint HAS_TRANSMISSION_MAP       = 1 << 8;
const uint HAS_CLEARCOAT_MAP          = 1 << 9;

const uint USE_MR_COMBINED            = 1 << 10;
const uint USE_ARM_COMBINED           = 1 << 11;

// ================================================================================================
// INPUT

layout (location = 0) in vec3 fragColor;   
layout (location = 1) in vec3 fragModelWS;
layout (location = 2) in vec3 fragNormalWS;
layout (location = 3) in vec2 fragUV;
layout (location = 4) in vec3 fragTangent;  // unused
layout (location = 5) in vec4 fragViewPos;  // unused

layout (location = 7) in vec3 fragModelPos; // unused

// ================================================================================================
// OUTPUT

layout (location = 0) out vec4 outColor;

// ================================================================================================
// DESCRIPTOR SET 0 : GLOBAL

struct PointLightData
{
	vec4 Position;     // xyz = position,  w = unused
	vec4 Color;        // rgb = color,     a = intensity
};

struct SpotLightData
{
	vec4 Position;     // xyz = position,  w = unused
	vec4 Color;        // rgb = color,     a = intensity
	vec4 Direction;    // xyz = direction, w = unused
	vec4 Cutoffs;      // x = innerCutoff, y = outerCutoff, zw = unused
};

struct DirectionalLightData
{
	vec4 Direction;    // xyz = direction, w = ambientStrength
	vec4 Color;        // rgb = color,     a = intensity
};

struct CameraData
{
    mat4 Projection;
    mat4 View;
    mat4 InverseView;
};

layout (set = 0, binding = 0) uniform GlobalUbo
{
	CameraData           Camera;

	DirectionalLightData DirectionalLight;

	PointLightData       PointLights[ MAX_POINT_LIGHTS ];
	SpotLightData        SpotLights [ MAX_SPOT_LIGHTS  ];

	int                  PointLightsCount;
	int                  SpotLightsCount;

} uUbo;

// ================================================================================================
// DESCRIPTOR SET 0 : DEFAULT TEXTURE

layout (set = 0, binding = 1) uniform sampler2D DefaultTexture; // unused

// ================================================================================================
// DESCRIPTOR SET 1 : MATERIAL TEXTURES

layout (set = 1, binding = 0) uniform sampler2D uAlbedoMap;
layout (set = 1, binding = 1) uniform sampler2D uNormalMap;
// layout (set = 1, binding = -) uniform sampler2D uMetallicMap;  // TODO
// layout (set = 1, binding = -) uniform sampler2D uRoughnessMap; // TODO
layout (set = 1, binding = 2) uniform sampler2D uMetallicRoughnessMap;
layout (set = 1, binding = 3) uniform sampler2D uAOMap;
layout (set = 1, binding = 4) uniform sampler2D uEmissiveMap;
layout (set = 1, binding = 5) uniform PBRParameters 
{
    vec4  AlbedoFactor;
    float MetallicFactor;
    float RoughnessFactor;
    float AOFactor;
    float _pad0;

    vec3  EmissiveFactor;
    float EmissiveStrength;

} uParameters;

// ================================================================================================
// CONSTANT PUSH : MAIN

layout (push_constant) uniform Push 
{
	mat4 ModelMatrix;
	mat4 NormalMatrix;

    uint Flags;
    
} uPush;

// ================================================================================================
// HELPER FUNCTIONS

// https://learnopengl.com/PBR/Lighting
// https://github.com/drsnuggles8/OloEngineBase/blob/master/OloEditor/assets/shaders/include/PBRCommon.glsl

// ------------------------------------------------------------------------------------------------

// Standard gamma value for sRGB is 2.2.
const float SRGB_GAMMA_VALUE = 2.2;

// Gamma Correction
vec3 LinearToSRGB(vec3 color) 
{
    return pow(color, vec3(1.0 / SRGB_GAMMA_VALUE)); 
}

vec3 SRGBtoLinear(vec3 color) 
{ 
    return pow(color, vec3(SRGB_GAMMA_VALUE)); 
}

vec4 SRGBtoLinear(vec4 color)
{
	vec3 linOut = pow(color.xyz, vec3(SRGB_GAMMA_VALUE));

	return vec4(linOut, color.w);
}

// ------------------------------------------------------------------------------------------------

// ACES Filmic Tone Mapping (Narkowicz 2015)
vec3 ACESFilm(vec3 x)
{
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;

    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

vec3 ACESFilmGammaCorrection(vec3 color)
{
    color = ACESFilm(color);

    return LinearToSRGB(color); 
}

// ------------------------------------------------------------------------------------------------

vec3 HDRToneMap(vec3 color)
{
    return color / (color + vec3(1.0));
}

vec3 HDRToneMapGammaCorrection(vec3 color)
{
    color = HDRToneMap(color);

    return LinearToSRGB(color); 
}

// ------------------------------------------------------------------------------------------------

vec3 GetNormalFromMap()
{
    vec3 tangentNormal = texture(uNormalMap, fragUV).xyz * 2.0 - 1.0;

    vec3 Q1  = dFdx(fragModelWS);
    vec3 Q2  = dFdy(fragModelWS);
    vec2 st1 = dFdx(fragUV);
    vec2 st2 = dFdy(fragUV);

    vec3 N   =  normalize(fragNormalWS);
    vec3 T   =  normalize(Q1 * st2.t - Q2 * st1.t);
    vec3 B   = -normalize(cross(N, T));

    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}

// ------------------------------------------------------------------------------------------------

// Approximate the ratio between how much the surface reflects and how much it refracts.
// The F0 parameter is the surface reflection at zero incidence or how much the surface reflects
// if looking directly at the surface.
vec3 FresnelSchlick(float cosTheta, vec3 F0) 
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 FresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness) 
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// ------------------------------------------------------------------------------------------------

// Approximate the relative surface area of microfacets exactly aligned to H.
// Using Trowbridge-Reitz GGX.
float DistributionGGX(vec3 N, vec3 H, float roughness) 
{
    // Based on observations by Disney and adopted by Epic Games, the lighting looks more correct
    // squaring the roughness in both the geometry and normal distribution function.

    float a      = roughness * roughness; // alpha
    float a2     = a * a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
          denom = PI * denom * denom;

    return num / denom;
}

float DistributionGGX(float NdotH, float roughness) 
{
    float a     = roughness * roughness; // alpha
    float a2    = a * a;

    float num   = a2;
    float denom = (NdotH * NdotH) * (a2 - 1.0) + 1.0;
          denom = PI * denom * denom;

    return num / denom;
}

// ------------------------------------------------------------------------------------------------

// Approximate the relative surface area where micro-facet details occlude light.
float GeometrySchlickGGX(float NdotV, float roughness) 
{
    float r     = (roughness + 1.0);
    float k     = (r * r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return num / denom;
}

// Smith's method: Take into account view direction (obstruction) and light direction (shadowing).
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) 
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);

    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

float GeometrySmith(float NdotV, float NdotL, float roughness) 
{
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

// ------------------------------------------------------------------------------------------------

// Cook-Torrance BRDF implementation
vec3 CookTorranceBRDF(
    vec3  N,              // 
    vec3  V,              // 
    vec3  L,              // 
    vec3  albedoValue,    // 
    float metallicValue,  // 
    float roughnessValue) // 
{
    vec3 H = normalize(V + L);

    // Calculate F0 based on metallic workflow
    vec3 F0 = vec3(0.04);                          // Dielectric materials are assumed to have a constant F0 value of 0.04.
         F0 = mix(F0, albedoValue, metallicValue); // Metal will tint the base reflectivity by the surface's color.

    // Calculate the three components of the BRDF

    // Fresnel (F) 
    vec3  F = FresnelSchlick(max(dot(H, V), 0.0), F0);

    // Normal Distribution Function (D)
    float D = DistributionGGX(N, H, roughnessValue);

    // Geometry (G)
    float G = GeometrySmith(N, V, L, roughnessValue);
    
    // Calculate Specular BRDF
    vec3  numerator   = D * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + EPSILON; // Epsilon (0.00001) to prevent divide by zero.
    vec3  specular    = numerator / denominator;

    // Calculate Diffuse Contribution

    // Specular ratio - Energy of light that gets reflected.
    vec3 kS = F;

    // Diffuse ratio - Remaining energy for refraction.
    vec3 kD = vec3(1.0) - kS;

    // multiply kD by the inverse metalness such that only non-metals have diffuse lighting, 
    // or a linear blend if partly metal (pure metals have no diffuse light).

    kD *= 1.0 - metallicValue; // Metallic surfaces don't refract light.)
    
    return kD * albedoValue / PI + specular;
}

// ================================================================================================

vec3 PointLightCalculation(
    vec3           albedoValue, 
    float          metallicValue, 
    float          roughnessValue, 
    vec3           V,      // viewToFragPos
    vec3           N,      // normalFromMap
    PointLightData light)
{
    // Light direction.
    vec3  L                = light.Position.xyz - fragModelWS;
    vec3  H                = normalize(V + L);
    float lightToPixelDist = length(L);
    L = normalize(L);
    
    // Attenuate light by the inverse square law.
    float attenuation = 1.0 / (lightToPixelDist * lightToPixelDist);
    vec3  radiance    = light.Color.rgb * light.Color.w * attenuation;

    vec3 BRDF = CookTorranceBRDF(N, V, L, albedoValue, metallicValue, roughnessValue);

    // Calculate the light's contribution to the reflectance equation. (ie. scale light by NdotL)
    float NdotL = max(dot(N, L), 0.0);

    // Add to outgoing radiance Lo.
    vec3 Lo = BRDF * radiance * NdotL;

    return Lo;
}

// ================================================================================================

vec3 DirectionalLightCalculation(
    vec3           albedoValue, 
    float          metallicValue, 
    float          roughnessValue, 
    vec3           V,      // viewToFragPos
    vec3           N,      // normalFromMap
    vec3           lightDirection, 
    vec4           lightColor)
{
    vec3 L = normalize(-lightDirection);

    float NdotL = max(0.0, dot(N, L));
    float NdotV = max(0.0, dot(N, V));

    vec3  H     = normalize(V + L);
    float NdotH = max(0.0, dot(N, H));

    // Cook-Torrance BRDF ------------------------------------------------
    
    // Fresnel (F) 
    vec3 F0 = vec3(0.04);                          // Dielectric materials are assumed to have a constant F0 value of 0.04.
         F0 = mix(F0, albedoValue, metallicValue); // Metal will tint the base reflectivity by the surface's color.

    vec3  F = FresnelSchlick(max(dot(H, V), 0.0), F0);

    // Normal Distribution Function (D)
    float D = DistributionGGX(N, H, roughnessValue);

    // Geometry (G)
    float G = GeometrySmith(N, V, L, roughnessValue);
    
    // Cook-Torrance BRDF
    vec3  nominator   = F * G * D;
    float denominator = max(EPSILON, 4.0 * NdotV * NdotL);
    vec3  specular    = nominator / denominator;
          specular    = clamp(specular, vec3(0.0), vec3(10.0));

    // -------------------------------------------------------------------

    // Specular ratio. (kS is equal to Fresnel)
    vec3 kS = F;

    // Diffuse ratio.
    // for energy conservation, the diffuse and specular light can't be above 1.0 (unless the surface emits light); 
    // to preserve this relationship the diffuse component (kD) should equal 1.0 - kS.

    vec3 kD = vec3(1.0) - kS;

    // multiply kD by the inverse metalness such that only non-metals have diffuse lighting, 
    // or a linear blend if partly metal (pure metals have no diffuse light).

    kD *= 1.0 - metallicValue; // Metallic surfaces don't refract light, so we nullify the diffuse term.
    
    vec3 diffuse = kD * albedoValue;

    vec3 result = (diffuse + specular) * (lightColor.xyz * lightColor.w) * NdotL;

    return result;
}

// ================================================================================================

vec3 SpotLightCalculation(
    vec3           albedoValue, 
    float          metallicValue, 
    float          roughnessValue, 
    vec3           V,      // viewToFragPos
    vec3           N,      // normalFromMap
    SpotLightData  light)
{
    // Light direction.
    vec3  L                = light.Position.xyz - fragModelWS;
    float lightToPixelDist = length(L);
    L = normalize(L);
    
    // Attenuate light by the inverse square law.
    float attenuation = 1.0 / (lightToPixelDist * lightToPixelDist);
    vec3  radiance    = light.Color.rgb * light.Color.w * attenuation;
    
    vec3 H = normalize(V + L);
    
    // Compute the BRDF term using the Cook-Torrance BRDF
    // Fresnel (F)
    // Dielectric materials are assumed to have a constant F0 value of 0.04.
    vec3 F0 = vec3(0.04);
    // Metal will tint the base reflectivity by the surface's color.
    F0 = mix(F0, albedoValue, metallicValue);

    vec3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);
    
    // Normal distribution function (D)
    float NDF = DistributionGGX(N, H, roughnessValue);
    // Geometry (G)
    float G = GeometrySmith(N, V, L, roughnessValue);
    
    // Cook-Torrance BRDF
    vec3  numerator   = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001; // Prevent divide by zero.
    vec3  specular    = numerator / denominator;
    
    // Specular ratio.
    vec3 kS = F;
    // Diffuse ratio.
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallicValue; // Metallic surfaces don't refract light, so we nullify the diffuse term.
    
    // spotlight (soft edge)
    float theta     = dot(L, normalize(-light.Direction.xyz)); 
    float ep        = (light.Cutoffs.x - light.Cutoffs.y); //postion w is cutoff, direction w is outer cutoff
    float intensity = clamp((theta - light.Cutoffs.y) / ep, 0.0, 1.0);
    kD       *= intensity;
    specular *= intensity;
    
    // Calculate the light's contribution to the reflectance equation.
    float NdotL = max(dot(N, L), 0.0);
    vec3  Lo    = (kD * albedoValue / PI + specular) * radiance * NdotL;

    return Lo;
}

vec3 CalculateEmissive()
{
    vec3 emissive = uParameters.EmissiveFactor.rgb * uParameters.EmissiveStrength;

    if ((uPush.Flags & HAS_EMISSIVE_MAP) != 0u)
    {
        vec3 emissiveTex = SRGBtoLinear(texture(uEmissiveMap, fragUV).rgb); // * EMISSIVE_INTENSITY;

        emissive *= emissiveTex;
    }

    return emissive;
}

// ================================================================================================
// MAIN

void main()
{
    // Material Properties -----------------------------------------------

    float roughness = 0.0;
    float metallic  = 0.0;
    float ao        = 0.0;

    vec3 albedo     = vec3(1.0);
    // vec3 emissive   = vec3(0.0);
    vec3 N          = vec3(0.0);

    bool aoHandled  = false;

    // [ Albedo ]
    if ((uPush.Flags & HAS_ALBEDO_MAP) != 0u)
    {
        // albedo = texture(uAlbedoMap, fragUV).rgb * uParameters.AlbedoFactor.rgb;
        albedo = pow(texture(uAlbedoMap, fragUV).rgb, vec3(2.2));
    }

    // [ AO-Roughness-Metallic Packed ]
    if ((uPush.Flags & USE_ARM_COMBINED) != 0u)
    {
        vec3 arm = texture(uMetallicRoughnessMap, fragUV).rgb;

        ao        = clamp(arm.r * AO_MULTIPLIER,        0.0,  1.0);
        roughness = clamp(arm.g * ROUGHNESS_MULTIPLIER, 0.04, 1.0);
        metallic  = clamp(arm.b * METALLIC_MULTIPLIER,  0.0,  1.0);

        aoHandled = true;
    }
    
    // [ Metallic-Roughness Packed (glTF) ]
    else if ((uPush.Flags & USE_MR_COMBINED) != 0u)
    {
        vec3 mr = texture(uMetallicRoughnessMap, fragUV).rgb;

        metallic  = clamp(mr.b * uParameters.MetallicFactor,  0.0,  1.0);
        roughness = clamp(mr.g * uParameters.RoughnessFactor, 0.04, 1.0);
        // metallic  = clamp(mr.b * METALLIC_MULTIPLIER,  0.0,  1.0);
        // roughness = clamp(mr.g * ROUGHNESS_MULTIPLIER, 0.04, 1.0);
    }
    else
    {
        // TODO: Sample individual roughness and metallic textures.
        
        // // [ Metallic ]
        // if ((uPush.Flags & HAS_METALLIC_MAP) != 0u)
        // {
        //     metallic = texture(uMetallicMap, fragUV).r;
        // }

        // // [ Roughness ]
        // if ((uPush.Flags & HAS_ROUGHNESS_MAP) != 0u)
        // {
        //     roughness = texture(uRoughnessMap, fragUV).r;
        // }
    }

    // [ AO (Ambient Occlusion) ]
    if (!aoHandled && (uPush.Flags & HAS_AO_MAP) != 0u)
    {
        ao *= clamp(texture(uAOMap, fragUV).r * AO_MULTIPLIER, 0.0, 1.0);
    }

    // [ Emissive ]
    vec3 emissive = CalculateEmissive();

    // [ Surface Normal ]
    if ((uPush.Flags & HAS_NORMAL_MAP) != 0u)
    {
        N = GetNormalFromMap();
    }
    else
    {
        N = normalize(fragNormalWS);
    }

    // -------------------------------------------------------------------

    // Camera Position (World Space)
    vec3 cameraPosWS = uUbo.Camera.InverseView[3].xyz;
    
    // View direction
    vec3 V = normalize(cameraPosWS - fragModelWS);

    // Total reflected radiance back to the viewer.
    vec3 Lo = vec3(0.0);

    // [ Point Lights ]
    for(int i = 0; i < uUbo.PointLightsCount; i++)
    {
        PointLightData light = uUbo.PointLights[ i ];

        Lo += PointLightCalculation(
            albedo, 
            metallic, 
            roughness, 
            V, 
            N, 
            light
        );
    }

    // [ Spot Lights ]
    for(int j = 0; j < uUbo.SpotLightsCount; j++)
    {
        SpotLightData light = uUbo.SpotLights[ j ];

        Lo += SpotLightCalculation(
            albedo, 
            metallic, 
            roughness, 
            V, 
            N, 
            light
        );
    }

    // Improvised ambient term.
    vec3 ambient = uUbo.DirectionalLight.Direction.w * albedo * ao;

    // [ Direct Light ]
    vec3 dirLi = DirectionalLightCalculation(
        albedo, 
        metallic, 
        roughness, 
        V, 
        N, 
        uUbo.DirectionalLight.Direction.xyz, 
        uUbo.DirectionalLight.Color
    );

    // [ Final Composite ]
    vec3 color = ambient + Lo + dirLi;

    if ((uPush.Flags & HAS_EMISSIVE_MAP) != 0u)
    {
        color += emissive;
    }

    // [ Tone Mapping / Gamma Correction (sRGB) ]
    color = ACESFilmGammaCorrection( color ); 
    
    outColor = vec4(color, 1.0);
}

// ================================================================================================