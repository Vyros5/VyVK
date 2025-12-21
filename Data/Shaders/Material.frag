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
const int MAX_DIRECT_LIGHTS = 10;
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
    // mat4 LightSpaceMatrix;

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

// Shadow map texture
// layout(set = 2, binding = 0) uniform sampler2D shadowMap;

// ================================================================================================

// Input
layout(location = 0) in vec3 fragPosWorld;
layout(location = 1) in vec3 fragNormalWorld;
layout(location = 2) in vec3 fragColor;
layout(location = 3) in vec2 fragUV;
// layout(location = 4) in vec4 fragPosLightSpace;

// Output
layout(location = 0) out vec4 outColor;

// ================================================================================================
// float textureProj(vec4 shadowCoord, vec2 off)
// {
//     float shadow = 1.0;

//     if (shadowCoord.z > -1.0 && shadowCoord.z < 1.0)
//     {
//         vec2  sampleCoords = (shadowCoord.xy / shadowCoord.w) * 0.5 + 0.5 + off;
//         float dist         = texture(shadowMap, sampleCoords).r;
//         float currentDepth = (shadowCoord.z / shadowCoord.w) - 0.005;

//         if (shadowCoord.w > 0.0 && currentDepth > dist)
//         {
//             shadow = 0.0;
//         }
//     }
    
//     return shadow;
// }

// float filterPCF(vec4 sc)
// {
//     ivec2 texDim = textureSize(shadowMap, 0);
//     float scale  = 1.5;

//     float dx = scale * 1.0 / float(texDim.x);
//     float dy = scale * 1.0 / float(texDim.y);

//     float shadowFactor = 0.0;
//     int count = 0;
//     int range = 4;

//     for (int x = -range; x <= range; x++)
//     {
//         for (int y = -range; y <= range; y++)
//         {
//             shadowFactor += textureProj(sc, vec2(dx * x, dy * y));

//             count++;
//         }
//     }

//     return shadowFactor / count;
// }





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
    // Sample Albedo texture and combine with material color.
    vec3 materialColor = uPush.Albedo * texture(albedoTexture, fragUV).rgb;
    
    // Sample normal map.
    vec3 normalMap     = texture(normalTexture, fragUV).rgb;
    vec3 N             = normalize(fragNormalWorld);
    vec3 surfaceNormal = N;

    // Use normal map if not a plain white texture.
    vec3 whiteTexel = vec3(1.0, 1.0, 1.0);
    
    if (length(normalMap - whiteTexel) > 0.01) 
    {
        normalMap     = normalize(normalMap * 2.0 - 1.0);
        mat3 TBN      = calculateTBN(N, fragPosWorld, fragUV);
        surfaceNormal = normalize(TBN * normalMap);
    }

    // Ambient + diffuse + specular
    vec3 diffuseLight  = uUbo.AmbientLightColor.xyz * uUbo.AmbientLightColor.w;
    vec3 specularLight = vec3(0.0);

    vec3 cameraPosWorld = uUbo.Camera.InverseView[3].xyz;
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
    for (int i = 0; i < uUbo.NumSpotLights; i++) 
    {
        SpotLight light = uUbo.SpotLights[i];

        vec3  directionToLight = light.Position.xyz - fragPosWorld;
        float distance         = length(directionToLight);
        float attenuation      = 1.0 / (distance * distance);

        directionToLight = normalize(directionToLight);

        // Calculate spotlight cone
        float theta         = dot(directionToLight, normalize(-light.Direction.xyz));
        float epsilon       = light.InnerCutoff - light.OuterCutoff;
        float spotIntensity = clamp((theta - light.OuterCutoff) / epsilon, 0.0, 1.0);

        float cosAngleIncidence = max(dot(surfaceNormal, directionToLight), 0);
        vec3  intensity         = light.Color.xyz * light.Color.w * attenuation * spotIntensity;

        diffuseLight += intensity * cosAngleIncidence;

        // Specular
        vec3  halfAngle = normalize(directionToLight + viewDirection);
        float blinnTerm = clamp(dot(surfaceNormal, halfAngle), 0, 1);
        float shininess = mix(128.0, 8.0, uPush.Roughness);
        blinnTerm = pow(blinnTerm, shininess);

        vec3 specularColor = mix(vec3(0.04), materialColor, uPush.Metallic);

        specularLight += intensity * blinnTerm * specularColor;
    }

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




// void main() 
// {
//     // Sample Albedo texture and combine with material Color
//     vec3 materialColor = uPush.Albedo * texture(albedoTexture, fragUV).rgb;
    
//     // Sample normal map
//     vec3 normalMap = texture(normalTexture, fragUV).rgb;
//     vec3 N = normalize(fragNormalWorld);
//     vec3 surfaceNormal = N;

//     // Use normal map if not a plain white texture
//     vec3 whiteTexel = vec3(1.0, 1.0, 1.0);
    
//     if (length(normalMap - whiteTexel) > 0.01) 
//     {
//         normalMap = normalize(normalMap * 2.0 - 1.0);
//         mat3 TBN = calculateTBN(N, fragPosWorld, fragUV);
//         surfaceNormal = normalize(TBN * normalMap);
//     }

//     // Ambient + diffuse + specular
//     vec3 diffuseLight  = uUbo.AmbientLightColor.xyz * uUbo.AmbientLightColor.w;
//     vec3 specularLight = vec3(0.0);

//     vec3 cameraPosWorld = uUbo.InverseView[3].xyz;
//     vec3 viewDirection  = normalize(cameraPosWorld - fragPosWorld);

//     // Point lights
//     for (int i = 0; i < uUbo.NumPointLights; i++) 
//     {
//         PointLight light = uUbo.PointLights[i];

//         vec3  directionToLight = light.Position.xyz - fragPosWorld;
//         float attenuation      = 1.0 / dot(directionToLight, directionToLight);

//         directionToLight = normalize(directionToLight);

//         float cosAngleIncidence = max(dot(surfaceNormal, directionToLight), 0);
//         vec3  intensity         = light.Color.xyz * light.Color.w * attenuation;

//         diffuseLight += intensity * cosAngleIncidence;

//         // Specular
//         vec3  halfAngle = normalize(directionToLight + viewDirection);
//         float blinnTerm = clamp(dot(surfaceNormal, halfAngle), 0, 1);
//         float shininess = mix(128.0, 8.0, uPush.Roughness);
//         blinnTerm = pow(blinnTerm, shininess);

//         vec3 specularColor = mix(vec3(0.04), materialColor, uPush.Metallic);

//         specularLight += intensity * blinnTerm * specularColor;
//     }

//     // https://github.com/invzz/VulkanEngine/blob/main/assets/shaders/pbr_shader.frag

//     Spot lights
//     for (int i = 0; i < uUbo.NumSpotLights; i++) 
//     {
//         SpotLight light = uUbo.SpotLights[i];

//         vec3  directionToLight = light.Position.xyz - fragPosWorld;
//         float distance         = length(directionToLight);
//         float attenuation      = 1.0 / (distance * distance);

//         directionToLight = normalize(directionToLight);

//         // Calculate spotlight cone
//         float theta         = dot(directionToLight, normalize(-light.Direction.xyz));
//         float epsilon       = light.InnerCutoff - light.OuterCutoff;
//         float spotIntensity = clamp((theta - light.OuterCutoff) / epsilon, 0.0, 1.0);

//         float cosAngleIncidence = max(dot(surfaceNormal, directionToLight), 0);
//         vec3  intensity         = light.Color.xyz * light.Color.w * attenuation * spotIntensity;

//         diffuseLight += intensity * cosAngleIncidence;

//         // Specular
//         vec3  halfAngle = normalize(directionToLight + viewDirection);
//         float blinnTerm = clamp(dot(surfaceNormal, halfAngle), 0, 1);
//         float shininess = mix(128.0, 8.0, uPush.Roughness);
//         blinnTerm = pow(blinnTerm, shininess);

//         vec3 specularColor = mix(vec3(0.04), materialColor, uPush.Metallic);

//         specularLight += intensity * blinnTerm * specularColor;
//     }

//     // Directional lights
//     for (int i = 0; i < uUbo.NumDirectionalLights; i++) 
//     {
//         DirectionalLight light = uUbo.DirectionalLights[i];
    
//         vec3 directionToLight = normalize(-light.Direction.xyz);

//         float cosAngleIncidence = max(dot(surfaceNormal, directionToLight), 0);
//         vec3  intensity         = light.Color.xyz * light.Color.w;

//         diffuseLight += intensity * cosAngleIncidence;

//         // Specular
//         vec3  halfAngle = normalize(directionToLight + viewDirection);
//         float blinnTerm = clamp(dot(surfaceNormal, halfAngle), 0, 1);
//         float shininess = mix(128.0, 8.0, uPush.Roughness);
//         blinnTerm = pow(blinnTerm, shininess);

//         vec3 specularColor = mix(vec3(0.04), materialColor, uPush.Metallic);

//         specularLight += intensity * blinnTerm * specularColor;
//     }

//     // Combine lighting
//     vec3 lighting   = diffuseLight * materialColor + specularLight;
//     vec3 emission   = uPush.EmissionColor * uPush.EmissionStrength;
//     vec3 finalColor = lighting + emission;

//     outColor = vec4(finalColor, 1.0);
// }