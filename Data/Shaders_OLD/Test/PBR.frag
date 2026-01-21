#version 450

#define LIGHT_TYPE_POINT       0
#define LIGHT_TYPE_SPOT        1
#define LIGHT_TYPE_DIRECTIONAL 2

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 3) in vec3 fragPosWorld;
layout(location = 4) in vec3 fragNormalWorld;
layout(location = 5) in vec4 fragTangentWorld;

layout(set = 1, binding = 0) uniform sampler2D uDiffuseTexSampler;
layout(set = 1, binding = 1) uniform sampler2D uNormalTexSampler;
layout(set = 1, binding = 2) uniform sampler2D uMetallicRoughnessTexSampler;

// Push constant to indicate if normal texture is available
layout(push_constant) uniform Push 
{
    mat4  modelMatrix;
    mat4  NormalMatrix;
    vec2  UVScale;
    vec2  UVOffset;
    float UVRotation;
    int   HasNormalTexture;
    int   HasMetallicRoughnessTexture;
    float MetallicFactor;
    float RoughnessFactor;

} uPush;

layout(location = 0) out vec4 outColor;


struct LightUBO 
{
    vec4  Position;           // xyz = position, w = unused
    vec4  Color;              // rgb = color,    a = intensity
    int   LightType;
    float SpotInnerConeAngle; // Cosine of inner cone angle (precomputed on CPU)
    float SpotOuterConeAngle; // Cosine of outer cone angle (precomputed on CPU)
    float _padding;           // for alignment
};

struct CameraData
{
    mat4 Projection;
    mat4 View;
    mat4 InverseView;
};

const int MAX_LIGHTS = 10;

layout(set = 0, binding = 0) uniform GlobalUBO 
{
    CameraData      Camera;

    vec4            AmbientLightColor; // rgb = color, a = intensity

    mat4            LightSpaceMatrix;
    vec4            ShadowLightDirection;

    LightUBO        Lights[ MAX_LIGHTS ];
    int             NumLights;

} uUbo;


layout(set = 0, binding = 1) uniform sampler2DShadow uShadowMap;


float calculateShadow(vec3 fragPosWorld, vec3 normal) 
{
    // Check if there's no shadow caster active
    if (uUbo.ShadowLightDirection.w < 0.0)
        return 1.0;

    // Check if surface faces away from light - if so, it's in shadow anyway
    vec3  lightDir = normalize(uUbo.ShadowLightDirection.xyz);
    float NdotL    = dot(normal, -lightDir);

    // If surface faces away from light, it's in shadow (no need to sample shadow map)
    if ((NdotL <= 0.0)) 
    {
        return 1.0; // Fully shadowed
    }

    vec4 fragPosLightSpace = uUbo.LightSpaceMatrix * vec4(fragPosWorld, 1.0);
    vec3 projCoords        = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords             = projCoords * 0.5 + 0.5;

    if (projCoords.z > 1.0 || 
        projCoords.x < 0.0 || projCoords.x > 1.0 ||
        projCoords.y < 0.0 || projCoords.y > 1.0) 
    {
        return 1.0;
    }

    float bias = 0.005;

    // Simple slope-scale bias
    // vec3 lightDir = normalize(uUbo.ShadowLightDirection.xyz);
    // float bias = max(0.05 * (1.0 - dot(normal, -lightDir)), 0.005);

    // Angle - dependent
    //bias : more bias at grazing angles
    // vec3 lightDir = normalize(uUbo.ShadowLightDirection.xyz);
    // float cosTheta = clamp(dot(normal, -lightDir), 0.0, 1.0);
    // float bias = 0.005 * tan(acos(cosTheta)); // Increases at shallow angles
    // bias = clamp(bias, 0.0, 0.01); // Clamp to prevent extreme values

    projCoords.z -= bias;

    // PCF Filtering
    float shadow    = 0.0;
    vec2  texelSize = 1.0 / textureSize(uShadowMap, 0);

    for (int x = -1; x <= 1; ++x) 
    {
        for (int y = -1; y <= 1; ++y) 
        {
            shadow += texture(uShadowMap, vec3(projCoords.xy + vec2(x, y) * texelSize, projCoords.z));
        }
    }

    return shadow / 9.0;

    return shadow;
}


void main() 
{
    float uFogStart = 20.0f;
    float uFogEnd   = 70.0f;
    vec3  uFogColor = vec3(0.0f, 0.0f, 0.0f);

    vec3  camWorldPos = uUbo.Camera.InverseView[3].xyz;
    float dist        = distance(camWorldPos, fragPosWorld);
    float fogFactor   = (uFogEnd - dist) / (uFogEnd - uFogStart);
    fogFactor         = clamp(fogFactor, 0.0, 1.0);

    if (fogFactor == 0.0) discard;

    vec3 diffuseLight  = vec3(0.0);
    vec3 specularLight = vec3(0.0);

    // Start with vertex normal
    vec3 surfaceNormal = normalize(fragNormalWorld);

    // Apply normal mapping if available
    if (uPush.HasNormalTexture > 0) 
    {
        vec3 normalMapSample = texture(uNormalTexSampler, fragTexCoord).rgb;

        // Convert from [0,1] to [-1,1] range
        normalMapSample = normalMapSample * 2.0 - 1.0;

        // Use proper tangent space from vertex attributes
        vec3 N = normalize(fragNormalWorld);
        vec3 T = normalize(fragTangentWorld.xyz);

        // Re-orthogonalize the tangent with respect to the normal
        T = normalize(T - dot(T, N) * N);

        // Calculate bitangent
        vec3 B = normalize(cross(N, T)) * fragTangentWorld.w;

        // Create TBN matrix - the column order is important
        mat3 TBN = mat3(T, B, N);

        // Transform normal from tangent space to world space
        surfaceNormal = normalize(TBN * normalMapSample);
    }

    // Sample metallic-roughness texture if available
    float roughness = uPush.RoughnessFactor; // Use material factor as default
    float metallic  = uPush.MetallicFactor; // Use material factor as default

    if (uPush.HasMetallicRoughnessTexture > 0) 
    {
        vec3 metallicRoughnessSample = texture(uMetallicRoughnessTexSampler, fragTexCoord).rgb;

        roughness *= metallicRoughnessSample.g; // Green channel = roughness * factor
        metallic  *= metallicRoughnessSample.b; // Blue channel  = metallic * factor
    }

    // Clamp to prevent artifacts
    roughness = clamp(roughness, 0.04, 1.0);

    vec3 viewDirection = normalize(camWorldPos - fragPosWorld);

    // Sample base color texture
    vec4 texColor = texture(uDiffuseTexSampler, fragTexCoord);

    // Base reflictivity (F0) for dielectrics and metals
    vec3 F0 = mix(vec3(0.04), texColor.rgb, metallic);

    // SHADOWS
    float shadow = calculateShadow(fragPosWorld, surfaceNormal);

    // LIGHTING
    for (int i = 0; i < uUbo.NumLights; i++)
    {
        LightUBO light = uUbo.Lights[i];

        vec3  directionToLight;
        float attenuation; // distance squared
        vec3  radiance;

        if (light.Color.w == 0) continue;

        if (light.LightType == LIGHT_TYPE_DIRECTIONAL) 
        {
            // Position represents direction (already normalized in CPU)
            directionToLight = -normalize(light.Position.xyz); // Light direction points TOWARD light
            attenuation      = 1.0; // No attenuation
        }
        else 
        {
            // POINT LIGHT (default)
            directionToLight = light.Position.xyz - fragPosWorld;
            float dist       = length(directionToLight);
            attenuation      = 1.0 / (dist * dist);
            directionToLight = normalize(directionToLight);
        }
        radiance = light.Color.xyz * light.Color.w * attenuation;

        vec3 halfVector = normalize(directionToLight + viewDirection);

        // Dot products
        float NdotL = max(dot(surfaceNormal, directionToLight), 0.0);
        float NdotV = max(dot(surfaceNormal, viewDirection),    0.0);
        float NdotH = max(dot(surfaceNormal, halfVector),       0.0);
        float VdotH = max(dot(viewDirection, halfVector),       0.0);

        // SPECULAR
        // 1. Normal Distribution Function (GGX/Trowbridge-Reitz)
        float alpha   = roughness * roughness;
        float alphaSq = alpha * alpha;
        float denom   = NdotH * NdotH * (alphaSq - 1.0) + 1.0;
        float D       = alphaSq / (3.14159265359 * denom * denom);

        // 2. Simplified Fresnel (Schlick approximation)
        vec3 F = F0 + (1.0 - F0) * pow(1.0 - VdotH, 5.0);

        // 3. Geometry function (Smith's Schlick-GGX)
        float k   = (roughness + 1.0) * (roughness + 1.0) / 8.0;
        float G_V = NdotV / (NdotV * (1.0 - k) + k);
        float G_L = NdotL / (NdotL * (1.0 - k) + k);
        float G   = G_V * G_L;

        // 4. Cook-Torrance BRDF
        vec3 specular = (D * F * G) / max(4.0 * NdotV * NdotL, 0.001);

        // 5. Energy conservation (kD + kS = 1)
        vec3 kS = F; // Specular contribution
        vec3 kD = (1.0 - kS) * (1.0 - metallic); // Diffuse contribution (metals = no diffuse)

        // 6. Lambertian diffuse
        vec3 diffuse = kD * texColor.rgb / 3.14159265359;

        // Combine diffuse and specular
        vec3 BRDF = diffuse + specular;

        // Add to accumulator
        diffuseLight += BRDF * radiance * NdotL * shadow;
    }

    // Add ambient lighting
    vec3 ambient    = uUbo.AmbientLightColor.xyz * uUbo.AmbientLightColor.w * texColor.rgb;
    vec3 finalColor = ambient + diffuseLight;

    finalColor = mix(uFogColor, finalColor, fogFactor);

    outColor = vec4(finalColor * fragColor, texColor.a);
}