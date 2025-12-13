#version 450

// ================================================================================================

struct PointLight 
{
    vec4 Position; // ignore w
    vec4 Color;    // w is intensity
};

struct SpotLight 
{
    vec4  Position;    // ignore w
    vec4  Direction;   // ignore w
    vec4  Color;       // w is intensity
    // float InnerCutoff; // cos of inner angle
    float OuterCutoff; // cos of outer angle
    float ConstantAtten;
    float LinearAtten;
    float QuadraticAtten;
};

struct DirectionalLight 
{
    vec4 Direction; // ignore w
    vec4 Color;     // w is intensity
};

// ================================================================================================
// Uniforms

layout(set = 0, binding = 0) uniform GlobalUBO 
{
    mat4             Projection;
    mat4             View;
    mat4             InverseView;

    vec4             AmbientLightColor; // w is intensity

    PointLight       PointLights[10];
    DirectionalLight DirectionalLights[10];
    SpotLight        SpotLights[10];
    mat4             LightSpaceMatrices[10];
    vec4             PointLightShadowData[4]; // xyz = position, w = far plane
    int              NumPointLights;
    int              NumSpotLights;
    int              NumDirectionalLights;
    int              ShadowLightCount;     // 2D shadow maps (directional + spot)
    int              CubeShadowLightCount; // Cube shadow maps (point lights)
    int              _pad1;
    int              _pad2;
    int              _pad3;
    vec4             FrustumPlanes[6];

} uUbo;

layout(push_constant) uniform Push 
{
    mat4 ModelMatrix;
    mat4 NormalMatrix;

    vec3  Albedo;
    float Metallic;
    float Roughness;
    float AO;
    vec2  TextureOffset;
    vec2  TextureScale;
    vec3  EmissionColor;
    float EmissionStrength;

} uPush;

// Material textures
layout(set = 1, binding = 0) uniform sampler2D albedoTexture;
layout(set = 1, binding = 1) uniform sampler2D normalTexture;
layout(set = 1, binding = 2) uniform sampler2D roughnessTexture;
layout(set = 1, binding = 3) uniform sampler2D metallicTexture;

// ================================================================================================

// Input
layout(location = 0) in vec3 fragPosWorld;
layout(location = 1) in vec3 fragNormalWorld;
layout(location = 2) in vec3 fragColor;
layout(location = 3) in vec2 fragUV;

// Output
layout(location = 0) out vec4 outColor;

// ================================================================================================

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

// ================================================================================================

void main() 
{
    // Sample Albedo texture and combine with material Color
    vec3 materialColor = uPush.Albedo * texture(albedoTexture, fragUV).rgb;
    
    // Sample normal map
    vec3 normalMap = texture(normalTexture, fragUV).rgb;
    vec3 N = normalize(fragNormalWorld);
    vec3 surfaceNormal = N;

    // Use normal map if not a plain white texture
    vec3 whiteTexel = vec3(1.0, 1.0, 1.0);
    
    if (length(normalMap - whiteTexel) > 0.01) 
    {
        normalMap = normalize(normalMap * 2.0 - 1.0);
        mat3 TBN = calculateTBN(N, fragPosWorld, fragUV);
        surfaceNormal = normalize(TBN * normalMap);
    }

    // Ambient + diffuse + specular
    vec3 diffuseLight  = uUbo.AmbientLightColor.xyz * uUbo.AmbientLightColor.w;
    vec3 specularLight = vec3(0.0);

    vec3 cameraPosWorld = uUbo.InverseView[3].xyz;
    vec3 viewDirection  = normalize(cameraPosWorld - fragPosWorld);

    // Point lights
    for (int i = 0; i < uUbo.NumPointLights; i++) 
    {
        PointLight light = uUbo.PointLights[i];

        vec3  directionToLight = light.Position.xyz - fragPosWorld;
        float attenuation      = 1.0 / dot(directionToLight, directionToLight);

        directionToLight = normalize(directionToLight);

        float cosAngleIncidence = max(dot(surfaceNormal, directionToLight), 0);
        vec3  intensity         = light.Color.xyz * light.Color.w * attenuation;

        diffuseLight += intensity * cosAngleIncidence;

        // Specular
        vec3  halfAngle = normalize(directionToLight + viewDirection);
        float blinnTerm = clamp(dot(surfaceNormal, halfAngle), 0, 1);
        float shininess = mix(128.0, 8.0, uPush.Roughness);
        blinnTerm = pow(blinnTerm, shininess);

        vec3 specularColor = mix(vec3(0.04), materialColor, uPush.Metallic);

        specularLight += intensity * blinnTerm * specularColor;
    }

    // https://github.com/invzz/VulkanEngine/blob/main/assets/shaders/pbr_shader.frag

    // Spot lights
    // for (int i = 0; i < uUbo.NumSpotLights; i++) 
    // {
    //     SpotLight light = uUbo.SpotLights[i];

    //     vec3  directionToLight = light.Position.xyz - fragPosWorld;
    //     float distance         = length(directionToLight);
    //     float attenuation      = 1.0 / (distance * distance);

    //     directionToLight = normalize(directionToLight);

    //     // Calculate spotlight cone
    //     float theta         = dot(directionToLight, normalize(-light.Direction.xyz));
    //     float epsilon       = light.InnerCutoff - light.OuterCutoff;
    //     float spotIntensity = clamp((theta - light.OuterCutoff) / epsilon, 0.0, 1.0);

    //     float cosAngleIncidence = max(dot(surfaceNormal, directionToLight), 0);
    //     vec3  intensity         = light.Color.xyz * light.Color.w * attenuation * spotIntensity;

    //     diffuseLight += intensity * cosAngleIncidence;

    //     // Specular
    //     vec3  halfAngle = normalize(directionToLight + viewDirection);
    //     float blinnTerm = clamp(dot(surfaceNormal, halfAngle), 0, 1);
    //     float shininess = mix(128.0, 8.0, uPush.Roughness);
    //     blinnTerm = pow(blinnTerm, shininess);

    //     vec3 specularColor = mix(vec3(0.04), materialColor, uPush.Metallic);

    //     specularLight += intensity * blinnTerm * specularColor;
    // }

    // Directional lights
    for (int i = 0; i < uUbo.NumDirectionalLights; i++) 
    {
        DirectionalLight light = uUbo.DirectionalLights[i];
    
        vec3 directionToLight = normalize(-light.Direction.xyz);

        float cosAngleIncidence = max(dot(surfaceNormal, directionToLight), 0);
        vec3  intensity         = light.Color.xyz * light.Color.w;

        diffuseLight += intensity * cosAngleIncidence;

        // Specular
        vec3  halfAngle = normalize(directionToLight + viewDirection);
        float blinnTerm = clamp(dot(surfaceNormal, halfAngle), 0, 1);
        float shininess = mix(128.0, 8.0, uPush.Roughness);
        blinnTerm = pow(blinnTerm, shininess);

        vec3 specularColor = mix(vec3(0.04), materialColor, uPush.Metallic);

        specularLight += intensity * blinnTerm * specularColor;
    }

    // Combine lighting
    vec3 lighting   = diffuseLight * materialColor + specularLight;
    vec3 emission   = uPush.EmissionColor * uPush.EmissionStrength;
    vec3 finalColor = lighting + emission;

    outColor = vec4(finalColor, 1.0);
}