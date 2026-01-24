#version 450

// ================================================================================================

#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_scalar_block_layout : require

// ================================================================================================
// Outputs

layout(location = 0) out vec4 outColor;

// ================================================================================================
// Inputs

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragmentWorldPos;
layout(location = 2) in vec3 fragmentNormalWorld;
layout(location = 3) in vec2 fragUV;

layout(location = 4) in flat uint inMeshletId;
layout(location = 5) in flat vec3 inConeAxis;

// ================================================================================================

struct PointLight
{
    vec4 Position; // xyz = position, w = unused
    vec4 Color;    // rgb = color,    a = intensity
};

struct DirectionalLight
{
    vec4 Direction; // xyz = direction, w = unused
    vec4 Color;     // rgb = color,     a = intensity
};

struct SpotLight
{
    vec4  Position;       // xyz = position,  w = unused
    vec4  Direction;      // xyz = direction, w = unused
    vec4  Color;          // rgb = color,     a = intensity
    float OuterCutoff;    // cos of outer angle
    float ConstantAtten;
    float LinearAtten;
    float QuadraticAtten;
};

layout(set = 0, binding = 0) uniform UBO
{
    mat4             Projection;
    mat4             View;
    vec4             AmbientLightColor;
    vec4             CameraPosition;
    PointLight       PointLights[16];
    DirectionalLight DirectionalLights[16];
    SpotLight        SpotLights[16];
    mat4             LightSpaceMatrices[16];
    vec4             PointLightShadowData[4]; // xyz = position, w = far plane
    int              PointLightCount;
    int              DirectionalLightCount;
    int              SpotLightCount;
    int              ShadowLightCount;     // 2D shadow maps (directional + spot)
    int              CubeShadowLightCount; // Cube shadow maps (point lights)
    int              DebugMode;            // 0: None, 1: Albedo, 2: Normal, 3: Roughness, 4: Metallic, 5: Lighting
    int              _pad0;
    int              _pad1;
    vec4             FrustumPlanes[6];
    vec4             FogColor;       // xyz = Horizon Color, w = density
    vec4             FogZenithColor; // xyz = Zenith Color, w = unused
    float            FogHeight;
    float            FogHeightDensity;
    float            _pad2;
    float            _pad3;

} uUbo;

// Global textures (set 1)
layout(set = 1, binding = 0) uniform sampler2D uGlobalTextures[];

// Shadow maps (set 2) - array of shadow maps for multiple lights
layout(set = 2, binding = 0) uniform sampler2DShadow uShadowMaps[4];

// Cube shadow maps for point lights (set 2, binding 1)
layout(set = 2, binding = 1) uniform samplerCube uCubeShadowMaps[4];

// IBL textures (set 3)
layout(set = 3, binding = 0) uniform samplerCube uIrradianceMap;
layout(set = 3, binding = 1) uniform samplerCube uPrefilterMap;
layout(set = 3, binding = 2) uniform sampler2D   uBrdfLUT;

// Push constants: Only material portion visible in fragment shader (offset 128)
layout(set = 4, binding = 0) uniform MaterialData
{
    vec4  Albedo;
    vec4  EmissiveInfo;             // rgb: color, a: strength
    vec4  SpecularGlossinessFactor; // rgb: specular, a: glossiness
    vec4  AttenuationColorAndDist;  // rgb: color, a: distance
    mat4  Params;                   // Packed float parameters
    uvec4 FlagsAndIndices0;         // Packed uint parameters
    uvec4 Indices1;
    uvec4 Indices2;
    uvec4 Indices3;

} uMaterial;

const float PI = 3.14159265359;

// ================================================================================================

// GGX/Trowbridge-Reitz Normal Distribution Function
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a      = roughness * roughness;
    float a2     = a * a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom       = PI * denom * denom;

    return num / denom;
}

// ================================================================================================

// Smith's Geometry Shadowing Function (Schlick-GGX)
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return num / denom;
}

// ================================================================================================

float GeometrySmith(float NdotV, float NdotL, float roughness)
{
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

// ================================================================================================

// Fresnel-Schlick Approximation
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// ================================================================================================

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// ================================================================================================

// Calculate shadow factor using PCF (Percentage Closer Filtering)
float calculateShadow(vec3 worldPos, int lightIndex)
{
    if (lightIndex >= uUbo.ShadowLightCount) return 1.0;

    // Transform world position to light space
    vec4 lightSpacePos = uUbo.LightSpaceMatrices[lightIndex] * vec4(worldPos, 1.0);

    // Perspective divide (needed for spotlight perspective projection)
    vec3 projCoords = lightSpacePos.xyz / lightSpacePos.w;

    // Transform from [-1,1] to [0,1] for UV lookup (Vulkan already has Z in [0,1])
    projCoords.xy = projCoords.xy * 0.5 + 0.5;

    // Check if outside shadow map bounds
    if (projCoords.x < 0.0 || projCoords.x > 1.0 || 
        projCoords.y < 0.0 || projCoords.y > 1.0 || 
        projCoords.z < 0.0 || projCoords.z > 1.0)
    {
        return 1.0; // No shadow outside light frustum
    }

    // PCF 3x3 sampling for soft shadows
    float shadow    = 0.0;
    vec2  texelSize = 1.0 / textureSize(uShadowMaps[lightIndex], 0);

    for (int x = -1; x <= 1; x++)
    {
        for (int y = -1; y <= 1; y++)
        {
            vec2 offset = vec2(x, y) * texelSize;

            // sampler2DShadow returns 0 or 1 based on depth comparison
            shadow += texture(uShadowMaps[lightIndex], vec3(projCoords.xy + offset, projCoords.z));
        }
    }
    shadow /= 9.0;

    return shadow;
}

// ================================================================================================

// Calculate shadow factor for point light using cube shadow map
float calculatePointLightShadow(vec3 worldPos, int lightIndex)
{
    if (lightIndex >= uUbo.CubeShadowLightCount) return 1.0;

    vec3  lightPos = uUbo.PointLightShadowData[lightIndex].xyz;
    float farPlane = uUbo.PointLightShadowData[lightIndex].w;

    // Direction from light to fragment
    vec3  lightToFrag  = worldPos - lightPos;
    float currentDepth = length(lightToFrag);

    // Check if outside light range
    if (currentDepth > farPlane) return 1.0;

    // For Vulkan cube maps, flip Y to match the rendering coordinate system
    vec3 sampleDir = vec3(lightToFrag.x, -lightToFrag.y, lightToFrag.z);

    // Sample cube shadow map - stored value is linear depth / farPlane
    float closestDepth = texture(uCubeShadowMaps[lightIndex], sampleDir).r;

    // Normalize current depth to [0, 1] range
    float normalizedDepth = currentDepth / farPlane;

    // Bias to prevent shadow acne
    float bias = 0.02;

    // In shadow if current fragment is further than stored depth
    float shadow = (normalizedDepth > closestDepth + bias) ? 0.0 : 1.0;

    return shadow;
}

// ================================================================================================

// Anisotropic GGX Distribution
float DistributionGGXAnisotropic(vec3 N, vec3 H, vec3 T, vec3 B, float roughness, float anisotropy)
{
    float alpha = roughness * roughness;
    float at    = max(alpha * (1.0 + anisotropy), 0.001);
    float ab    = max(alpha * (1.0 - anisotropy), 0.001);

    float ToH = dot(T, H);
    float BoH = dot(B, H);
    float NoH = dot(N, H);

    float a2 = at * ab;
    vec3  v  = vec3(ab * ToH, at * BoH, a2 * NoH);
    float v2 = dot(v, v);
    float w2 = a2 / v2;

    return a2 * w2 * w2 / PI;
}

// ================================================================================================

// Specular Occlusion (Lagarde/Filament)
float computeSpecularAO(float NdotV, float ao, float roughness)
{
    return clamp(pow(NdotV + ao, exp2(-16.0 * roughness - 1.0)) - 1.0 + ao, 0.0, 1.0);
}

// ================================================================================================

vec3 evalIridescence(float NdotV, float thickness, float ior)
{
    // Simple thin-film interference approximation
    // Phase shift based on path difference
    float cosTheta2  = NdotV * NdotV;
    float sinTheta2  = 1.0 - cosTheta2;
    float sinTheta2t = sinTheta2 / (ior * ior);
    float cosTheta2t = 1.0 - sinTheta2t;
    float cosTheta_t = sqrt(max(0.0, cosTheta2t));

    // Path difference = 2 * n * d * cos(theta_t)
    // We map this to a color cycle
    float pathDiff = 2.0 * ior * thickness * cosTheta_t;

    // Map path difference to RGB phase
    // 400-700nm range roughly
    vec3 phase = vec3(pathDiff) * vec3(1.0 / 450.0, 1.0 / 550.0, 1.0 / 650.0) * 2.0 * PI;

    return vec3(cos(phase.r), cos(phase.g), cos(phase.b)) * 0.5 + 0.5;
}

// ================================================================================================

struct Surface
{
    vec3  Albedo;
    float Alpha;
    float Metallic;
    float Roughness;
    float AO;
    vec3  N;
    vec3  V;
    vec3  F0;
    vec3  T;
    vec3  B;
    float ClearcoatStrength;
    float ClearcoatRoughness;
    float Anisotropy;
    float NdotV;
    vec3  R;
    float Transmission;
};

// ================================================================================================

void calculateDirectLight(Surface surf, vec3 L, vec3 radiance, out vec3 diffuse, out vec3 specular)
{
    vec3  H   = normalize(surf.V + L);
    float NDF = DistributionGGXAnisotropic(surf.N, H, surf.T, surf.B, surf.Roughness, surf.Anisotropy);

    float NdotL = max(dot(surf.N, L), 0.0);
    float G     = GeometrySmith(surf.NdotV, NdotL, surf.Roughness);
    vec3  F     = fresnelSchlick(max(dot(H, surf.V), 0.0), surf.F0);

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - surf.Metallic;

    vec3  numerator    = NDF * G * F;
    float denominator  = 4.0 * surf.NdotV * NdotL + 0.0001;
    vec3  specularTerm = numerator / denominator;

    diffuse  = (kD * surf.Albedo / PI) * radiance * NdotL;
    specular = specularTerm * radiance * NdotL;
}

// ================================================================================================

vec3 calculateClearcoat(Surface surf, vec3 L, vec3 radiance)
{
    vec3  H   = normalize(surf.V + L);
    float NDF = DistributionGGX(surf.N, H, surf.ClearcoatRoughness);

    float NdotL = max(dot(surf.N, L), 0.0);
    float G     = GeometrySmith(surf.NdotV, NdotL, surf.ClearcoatRoughness);
    vec3  F     = fresnelSchlick(max(dot(H, surf.V), 0.0), vec3(0.04));

    vec3  numerator   = NDF * G * F;
    float denominator = 4.0 * surf.NdotV * NdotL + 0.0001;
    vec3  specular    = numerator / denominator;

    return specular * radiance * NdotL;
}

// ================================================================================================

void accumulateLight(Surface surf, vec3 L, vec3 radiance, float shadow, inout vec3 diffuseLo, inout vec3 specularLo, inout vec3 clearcoatLo)
{
    vec3 diffuse  = vec3(0.0);
    vec3 specular = vec3(0.0);

    calculateDirectLight(surf, L, radiance, diffuse, specular);

    diffuseLo  += diffuse  * shadow;
    specularLo += specular * shadow;

    if (surf.ClearcoatStrength > 0.01)
    {
        clearcoatLo += calculateClearcoat(surf, L, radiance) * shadow;
    }
}

// ================================================================================================

Surface getSurfaceProperties()
{
    // Apply UV tiling scale
    vec2 uv = fragUV * uMaterial.Params[3][1];

    // Sample Material properties from textures or use push constants as base values
    vec4  baseColor          = uMaterial.Albedo;
    vec3  albedo             = baseColor.rgb;
    float alpha              = baseColor.a;
    float metallic           = uMaterial.Params[0][0];
    float roughness          = uMaterial.Params[0][1];
    float ao                 = uMaterial.Params[0][2];
    float clearcoatStrength  = uMaterial.Params[1][0];
    float clearcoatRoughness = uMaterial.Params[1][1];
    float anisotropy         = uMaterial.Params[1][2];

    // Texture sampling: Check flags to determine which textures are bound

    // [ Albedo/BaseColor Texture ]
    if ((uMaterial.FlagsAndIndices0.x & (1u << 0)) != 0u)
    {
        vec4 texColor = texture(uGlobalTextures[nonuniformEXT(uMaterial.FlagsAndIndices0.z)], uv);
        
        albedo *= texColor.rgb; // sRGB texture, auto-converted to linear by GPU
        alpha  *= texColor.a;
    }

    // Alpha Masking
    if (uMaterial.FlagsAndIndices0.y == 1) // MASK
    {
        if (alpha < uMaterial.Params[3][2])
        {
            discard;
        }

        alpha = 1.0; // Treat as opaque after test
    }
    else if (uMaterial.FlagsAndIndices0.y == 0) // OPAQUE
    {
        alpha = 1.0;
    }

    bool aoHandled = false;

    // [ OcclusionRoughnessMetallic Packed ]
    if ((uMaterial.FlagsAndIndices0.x & (1u << 7)) != 0u)
    {
        vec4 ormSample = texture(uGlobalTextures[nonuniformEXT(uMaterial.Indices1.y)], uv);
        
        ao             = ormSample.r;
        roughness      = ormSample.g;
        metallic       = ormSample.b;
        aoHandled      = true;
    }
    // [ MetallicRoughness Packed (glTF) ]
    else if ((uMaterial.FlagsAndIndices0.x & (1u << 6)) != 0u)
    {
        vec4 mrSample = texture(uGlobalTextures[nonuniformEXT(uMaterial.Indices1.y)], uv);
        
        metallic  *= mrSample.b;
        roughness *= mrSample.g;
    }
    else
    {
        // [ Metallic Texture ]
        if ((uMaterial.FlagsAndIndices0.x & (1u << 2)) != 0u)
        {
            metallic *= texture(uGlobalTextures[nonuniformEXT(uMaterial.Indices1.x)], uv).r;
        }

        // [ Roughness Texture ]
        if ((uMaterial.FlagsAndIndices0.x & (1u << 3)) != 0u) 
        {
            roughness *= texture(uGlobalTextures[nonuniformEXT(uMaterial.Indices1.y)], uv).r;
        }
    }

    // [ Ambient Occlusion Texture ]
    if (!aoHandled && (uMaterial.FlagsAndIndices0.x & (1u << 4)) != 0u)
    {
        ao *= texture(uGlobalTextures[nonuniformEXT(uMaterial.Indices1.z)], uv).r;
    }

    vec3 N = normalize(fragmentNormalWorld);

    // Normal mapping

    // [ Normal Texture]
    if ((uMaterial.FlagsAndIndices0.x & (1u << 1)) != 0u)
    {
        vec3 tangentNormal = texture(uGlobalTextures[nonuniformEXT(uMaterial.FlagsAndIndices0.w)], uv).xyz * 2.0 - 1.0;
        tangentNormal.y    = -tangentNormal.y;

        vec3 T;
        if (abs(N.y) > 0.99)
        {
            T = vec3(1.0, 0.0, 0.0);
        }
        else
        {
            T = normalize(cross(N, vec3(0.0, 1.0, 0.0)));
        }

        vec3 B   = normalize(cross(N, T));
        T        = normalize(cross(B, N));
        mat3 TBN = mat3(T, B, N);

        N = normalize(TBN * tangentNormal);
    }

    vec3 V = normalize(uUbo.CameraPosition.xyz - fragmentWorldPos);

    // Build tangent space for anisotropy
    vec3 T = normalize(cross(N, vec3(0.0, 1.0, 0.0)));
    if (length(T) < 0.01) T = normalize(cross(N, vec3(1.0, 0.0, 0.0)));
    vec3 B = cross(N, T);

    if (anisotropy > 0.01)
    {
        float angle = uMaterial.Params[1][3] * 2.0 * PI;
        float cosA  = cos(angle);
        float sinA  = sin(angle);
        vec3  Trot  = cosA * T + sinA * B;
        vec3  Brot  = -sinA * T + cosA * B;
        T           = Trot;
        B           = Brot;
    }

    vec3 F0 = vec3(0.04);

    if (uMaterial.Indices2.y == 1)
    {
        vec3  specularColor = uMaterial.SpecularGlossinessFactor.rgb;
        float glossiness    = uMaterial.SpecularGlossinessFactor.a;

        // [ Specular Glossiness Texture ]
        if ((uMaterial.FlagsAndIndices0.x & (1u << 8)) != 0u)
        {
            vec4 sgSample = texture(uGlobalTextures[nonuniformEXT(uMaterial.Indices2.x)], uv);
        
            specularColor *= sgSample.rgb;
            glossiness    *= sgSample.a;
        }

        roughness = 1.0 - glossiness;
        F0        = specularColor;
        metallic  = 0.0;
    }
    else
    {
        // [ Index of Refraction Override ]
        if (uMaterial.Params[2][1] != 1.5)
        {
            float ior = uMaterial.Params[2][1];
            float f   = (ior - 1.0) / (ior + 1.0);
            F0        = vec3(f * f);
        }

        F0 = mix(F0, albedo, metallic);
    }

    // Iridescence
    if (uMaterial.Params[2][2] > 0.0)
    {
        vec3 iridescenceColor = evalIridescence(max(dot(N, V), 0.1), uMaterial.Params[3][0], uMaterial.Params[2][3]);
        F0                    = mix(F0, iridescenceColor, uMaterial.Params[2][2]);
    }

    // Transmission
    float transmission = uMaterial.Params[2][0];

    // [ Transmission Texture ]
    if ((uMaterial.FlagsAndIndices0.x & (1u << 9)) != 0u)
    {
        transmission *= texture(uGlobalTextures[nonuniformEXT(uMaterial.Indices2.z)], uv).r;
    }
    // Transmission is disabled for metallic materials
    // transmission *= (1.0 - metallic);

    // [ Clearcoat Textures ]
    if ((uMaterial.FlagsAndIndices0.x & (1u << 10)) != 0u)
    {
        clearcoatStrength *= texture(uGlobalTextures[nonuniformEXT(uMaterial.Indices2.w)], uv).r;
    }
    if ((uMaterial.FlagsAndIndices0.x & (1u << 11)) != 0u)
    {
        clearcoatRoughness *= texture(uGlobalTextures[nonuniformEXT(uMaterial.Indices3.x)], uv).g;
    }

    Surface surf;
    surf.Albedo             = albedo;
    surf.Alpha              = alpha;
    surf.Metallic           = metallic;
    surf.Roughness          = roughness;
    surf.AO                 = ao;
    surf.N                  = N;
    surf.V                  = V;
    surf.F0                 = F0;
    surf.T                  = T;
    surf.B                  = B;
    surf.ClearcoatStrength  = clearcoatStrength;
    surf.ClearcoatRoughness = clearcoatRoughness;
    surf.Anisotropy         = anisotropy;
    surf.NdotV              = max(dot(N, V), 0.0);
    surf.R                  = reflect(-V, N);
    surf.Transmission       = transmission;

    return surf;
}

// ================================================================================================


void calculateIBL(Surface surf, out vec3 outDiffuse, out vec3 outSpecular)
{
    vec3 F_IBL       = surf.F0;
    vec3 F_roughness = fresnelSchlickRoughness(surf.NdotV, F_IBL, surf.Roughness);

    vec3 kS = F_roughness;
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - surf.Metallic;

    vec3 irradiance = texture(uIrradianceMap, surf.N).rgb;
    vec3 diffuse    = irradiance * surf.Albedo;

    const float MAX_REFLECTION_LOD = 4.0;
    vec3        prefilteredColor   = textureLod(uPrefilterMap, surf.R, surf.Roughness * MAX_REFLECTION_LOD).rgb;

    vec2 brdf     = texture(uBrdfLUT, vec2(surf.NdotV, surf.Roughness)).rg;
    vec3 specular = prefilteredColor * (surf.F0 * brdf.x + brdf.y);

    // Horizon occlusion: dampen specular reflection for vectors pointing below the horizon
    float horizon = min(1.0 + dot(surf.R, surf.N), 1.0);
    specular *= horizon * horizon;

    // Specular Occlusion: dampen specular reflection based on AO
    float specularAO = computeSpecularAO(surf.NdotV, surf.AO, surf.Roughness);
    specular *= specularAO;

    outDiffuse  = kD * diffuse * surf.AO;
    outSpecular = specular;

    // Add simple ambient as fallback/boost to diffuse
    outDiffuse += uUbo.AmbientLightColor.xyz * uUbo.AmbientLightColor.w * surf.Albedo * surf.AO * 0.05;
}

// ================================================================================================

vec3 calculateEmissive()
{
    vec2 uv       = fragUV * uMaterial.Params[3][1];
    vec3 emissive = uMaterial.EmissiveInfo.rgb * uMaterial.EmissiveInfo.a;

    if ((uMaterial.FlagsAndIndices0.x & (1u << 5)) != 0u) // Emissive texture
    {
        vec3 emissiveTex = texture(uGlobalTextures[nonuniformEXT(uMaterial.Indices1.w)], uv).rgb;
        emissive *= emissiveTex;
    }
    return emissive;
}

// ================================================================================================

void main()
{
    Surface surf = getSurfaceProperties();

    // Reflectance equation - Base layer
    vec3 diffuseLo   = vec3(0.0);
    vec3 specularLo  = vec3(0.0);
    vec3 clearcoatLo = vec3(0.0);

    // --------------------------------------------------------------------------------------------

    // [ Point lights ]
    for (int i = 0; i < uUbo.PointLightCount; i++)
    {
        vec3  lightDir  = uUbo.PointLights[i].Position.xyz - fragmentWorldPos;
        float distance2 = dot(lightDir, lightDir);
        float intensity = uUbo.PointLights[i].Color.w;

        if (distance2 > intensity * 250.0) continue;

        float distance    = sqrt(distance2);
        vec3  L           = lightDir / distance;
        float attenuation = 1.0 / distance2;
        vec3  radiance    = uUbo.PointLights[i].Color.xyz * intensity * attenuation;

        float shadow = 1.0;
        if (i < uUbo.CubeShadowLightCount)
        {
            shadow = calculatePointLightShadow(fragmentWorldPos, i);
        }

        accumulateLight(surf, L, radiance, shadow, diffuseLo, specularLo, clearcoatLo);
    }

    // --------------------------------------------------------------------------------------------

    // [ Directional Lights ]
    for (int i = 0; i < uUbo.DirectionalLightCount; i++)
    {
        vec3 L        = normalize(-uUbo.DirectionalLights[i].Direction.xyz);
        vec3 radiance = uUbo.DirectionalLights[i].Color.xyz * uUbo.DirectionalLights[i].Color.w;

        float shadow = 1.0;
        if (i == 0 && uUbo.ShadowLightCount > 0)
        {
            shadow = calculateShadow(fragmentWorldPos, 0);
        }

        accumulateLight(surf, L, radiance, shadow, diffuseLo, specularLo, clearcoatLo);
    }

    // --------------------------------------------------------------------------------------------

    // [ Spot Lights ]
    for (int i = 0; i < uUbo.SpotLightCount; i++)
    {
        vec3  lightDir = uUbo.SpotLights[i].Position.xyz - fragmentWorldPos;
        float distance = length(lightDir);
        vec3  L        = normalize(lightDir);

        vec3  spotDir   = normalize(-uUbo.SpotLights[i].Direction.xyz);
        float theta     = dot(L, spotDir);
        float epsilon   = uUbo.SpotLights[i].Direction.w - uUbo.SpotLights[i].OuterCutoff;
        float intensity = clamp((theta - uUbo.SpotLights[i].OuterCutoff) / epsilon, 0.0, 1.0);

        float attenuation = 1.0 / (uUbo.SpotLights[i].ConstantAtten + uUbo.SpotLights[i].LinearAtten * distance + uUbo.SpotLights[i].QuadraticAtten * distance * distance);

        vec3 radiance = uUbo.SpotLights[i].Color.xyz * uUbo.SpotLights[i].Color.w * attenuation * intensity;

        int   shadowIndex = 1 + i;
        float shadow      = 1.0;
        if (shadowIndex < uUbo.ShadowLightCount)
        {
            shadow = calculateShadow(fragmentWorldPos, shadowIndex);
        }

        accumulateLight(surf, L, radiance, shadow, diffuseLo, specularLo, clearcoatLo);
    }

    // --------------------------------------------------------------------------------------------

    if (surf.ClearcoatStrength > 0.01)
    {
        specularLo = mix(specularLo, specularLo + clearcoatLo * surf.ClearcoatStrength, surf.ClearcoatStrength);
    }

    vec3 diffuseIBL, specularIBL;
    calculateIBL(surf, diffuseIBL, specularIBL);

    vec3 emissive = calculateEmissive();

    // --- Advanced Transmission (Volume & Refraction) ---
    float thickness           = uMaterial.Params[3][3];
    vec3  attenuationColor    = uMaterial.AttenuationColorAndDist.rgb;
    float attenuationDistance = uMaterial.AttenuationColorAndDist.a;
    float ior                 = uMaterial.Params[2][1];

    // 1. Volume Attenuation (Beer's Law)
    vec3 volumeTransmission = vec3(1.0);

    if (thickness > 0.0 && attenuationDistance > 0.0)
    {
        vec3 sigma         = -log(attenuationColor) / attenuationDistance;
        volumeTransmission = exp(-sigma * thickness);
    }

    // 2. Refraction (Approximation using IBL)
    // Since we don't have a scene color texture, we use the environment map (uPrefilterMap)
    // to simulate looking through the object. This gives us "deformation" of the environment.
    vec3 refractedColor = vec3(0.0);

    if (surf.Transmission > 0.0)
    {
        // Refract View vector
        vec3 R_refract = refract(-surf.V, surf.N, 1.0 / ior);

        // Check for Total Internal Reflection
        if (length(R_refract) > 0.0)
        {
            const float MAX_REFLECTION_LOD = 4.0;

            // Sample environment in the refracted direction
            refractedColor = textureLod(uPrefilterMap, R_refract, surf.Roughness * MAX_REFLECTION_LOD).rgb;

            // Apply Volume Attenuation
            refractedColor *= volumeTransmission;

            // Tint with Albedo (standard PBR transmission tint)
            refractedColor *= surf.Albedo;
        }
    }

    // Reduce Diffuse contribution based on Transmission
    // (Energy conservation: Light that is transmitted is not reflected as diffuse)
    diffuseLo  *= (1.0 - surf.Transmission);
    diffuseIBL *= (1.0 - surf.Transmission);

    // Final Composition with Premultiplied Alpha
    // Opacity = alpha * (1 - transmission)
    // Improved Heuristic: Opacity should depend on Albedo Luminance.
    float luminance          = dot(surf.Albedo, vec3(0.299, 0.587, 0.114));
    float transmissionFactor = clamp(luminance, 0.3, 0.85);

    float opacity = surf.Alpha * (1.0 - surf.Transmission * transmissionFactor);

    // Diffuse and Ambient Diffuse are modulated by opacity (background shows through)
    // Specular (Direct + IBL) is additive (sits on top)
    // Emissive is additive
    // Refraction is additive (simulating light coming through)

    vec3 finalColor = (diffuseLo + diffuseIBL) * opacity + (specularLo + specularIBL) + emissive;

    // Add Refraction
    finalColor += refractedColor * surf.Transmission;

    if (uMaterial.Params[0][3] > 0.5)
    {
        float pulse         = 0.7 + 0.3 * sin(fragmentWorldPos.x + fragmentWorldPos.y + fragmentWorldPos.z);
        float rimIntensity  = 1.0 - abs(dot(surf.N, surf.V));
        rimIntensity        = pow(rimIntensity, 2.0);
        vec3 selectionColor = vec3(1.0, 1.0, 1.0) * pulse * 0.5;

        finalColor += selectionColor * rimIntensity;
    }

    // --------------------------------------------------------------------------------------------
    // Apply Debug Modes
    // --------------------------------------------------------------------------------------------

    // [ Albedo ]
    if (uUbo.DebugMode == 1)
    {
        finalColor = surf.Albedo;
        opacity    = 1.0;
    }

    // [ Normal ]
    else if (uUbo.DebugMode == 2)
    {
        finalColor = surf.N * 0.5 + 0.5;
        opacity    = 1.0;
    }

    // [ Roughness ]
    else if (uUbo.DebugMode == 3)
    {
        finalColor = vec3(surf.Roughness);
        opacity    = 1.0;
    }

    // [ Metallic ]
    else if (uUbo.DebugMode == 4)
    {
        finalColor = vec3(surf.Metallic);
        opacity    = 1.0;
    }

    // [ Lighting Only ]
    else if (uUbo.DebugMode == 5)
    {
        finalColor = (diffuseIBL + specularIBL) + diffuseLo + specularLo;
        opacity    = 1.0;
    }

    // [ Ambient Occlusion ]
    else if (uUbo.DebugMode == 6)
    {
        finalColor = vec3(surf.AO);
        opacity    = 1.0;
    }

    // [ Meshlets ]
    else if (uUbo.DebugMode == 7)
    {
        uint hash  = inMeshletId;
        hash       = (hash ^ 61) ^ (hash >> 16);
        hash       = hash + (hash << 3);
        hash       = hash ^ (hash >> 4);
        hash       = hash * 0x27d4eb2d;
        hash       = hash ^ (hash >> 15);
        vec3 color = vec3(float(hash & 255), float((hash >> 8) & 255), float((hash >> 16) & 255)) / 255.0;
        finalColor = color;
        opacity    = 1.0;
    }

    // [ Meshlet Cones ]
    else if (uUbo.DebugMode == 8)
    {
        finalColor = inConeAxis * 0.5 + 0.5;
        opacity    = 1.0;
    }

    // --------------------------------------------------------------------------------------------

    // [ Fog ]
    float fogDensity = uUbo.FogColor.w;

    if (fogDensity > 0.0)
    {
        float distance  = length(uUbo.CameraPosition.xyz - fragmentWorldPos);
        float fogFactor = 1.0 - exp(-distance * fogDensity);

        // Height Fog
        if (uUbo.FogHeightDensity > 0.0)
        {
            float heightFactor = exp(-(fragmentWorldPos.y - uUbo.FogHeight) * uUbo.FogHeightDensity);
            fogFactor          = 1.0 - exp(-distance * fogDensity * heightFactor);
        }

        fogFactor = clamp(fogFactor, 0.0, 1.0);

        // Fog Color Mixing (Horizon -> Zenith)
        vec3  rayDir      = normalize(fragmentWorldPos - uUbo.CameraPosition.xyz);
        float t           = clamp(rayDir.y, 0.0, 1.0);
        vec3  skyFogColor = mix(uUbo.FogColor.rgb, uUbo.FogZenithColor.rgb, t);

        finalColor = mix(finalColor, skyFogColor, fogFactor);
    }

    // Output final color
    outColor = vec4(finalColor, opacity);
}