import "../Modules/PBR";

// ================================================================================================

struct AmbientLight
{
    float4 Color;
};

struct DirectionalLight
{
    float4 Direction;
    float4 Color;
};

struct PointLight
{
    float4 Position;
    float4 Color;
};

struct Lighting
{
    float4           CameraPosition;
    AmbientLight     AmbientLight;
    DirectionalLight DirectionalLight;
    PointLight       PointLights[128];
    int              NumPointLights;
    float            AmbientOcclusionFactor;
    int              DebugViewMode;
};

// ================================================================================================

static const float EMISSIVE_INTENSITY = 2.0;
static const float MAX_REFLECTION_LOD = 4.0;

// ================================================================================================

[vk::binding(0, 0)] RWTexture2D<float4> sceneColorMap;
[vk::binding(1, 0)] Texture2D<float4> positionMap;
[vk::binding(2, 0)] Texture2D<float4> normalMap;
[vk::binding(3, 0)] Texture2D<float4> albedoMap;
[vk::binding(4, 0)] Texture2D<float4> armMap;
[vk::binding(5, 0)] Texture2D<float4> emissiveMap;
//[vk::binding(6, 0)] Sampler2D ssaoMap;
[vk::binding(7, 0)] ConstantBuffer<Lighting> lighting;

[vk::binding(0, 1)] SamplerCube irradianceMap;
[vk::binding(1, 1)] SamplerCube prefilteredEnvMap;
[vk::binding(2, 1)] Sampler2D   brdfLUTMap;

// ================================================================================================

//func sampleSSAO(float2 uv)->float
//{
//    float2 texelSize;
//    ssaoMap.GetDimensions(texelSize.x, texelSize.y);
//    texelSize = 1.0 / texelSize;

//    float result = 0.0;
//    for (int x = -2; x < 2; ++x)
//    {
//        for (int y = -2; y < 2; ++y)
//        {
//            float2 offset = float2(x, y) * texelSize;
//            result += ssaoMap.SampleLevel(uv + offset, 0).r;
//        }
//    }
//    return result / 16.0;
//}

// ================================================================================================

// kS: The specular reflectance of a surface, representing the intensity of the specular highlights (shininess).

// kD - (Konstant diffuse) (Diffuse Reflection Coefficient):
//      Defines how reflective the surface is to diffuse light. 
//      A higher value means the surface will appear brighter under the light.

// L - (Light Vector): 
//      A normalized vector that points from the surface point being shaded towards the light source.
//      It determines the direction of the incoming light.

// AO - (Ambient Occlusion): 
//      Defines which areas of a surface are blocked from ambient light, 
//      creating soft shadows in crevices and corners.

// R - (Roughness):
//      Controls the microsurface detail of an object, determining how blurred or sharp reflections are. 
//      A low value means a smooth, reflective surface, A high value means a rough, matte surface.

// M - (Metallic):
//      A binary or grayscale map that specifies whether a part of the surface is 
//      a metallic (conductive) or a non-metallic (dielectric) material.

// Radiance: The amount of light emitted or reflected by a surface in a specific direction.

[shader("compute")]
[numthreads(16, 16, 1)]
func computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    int2 outputSize;
    sceneColorMap.GetDimensions(outputSize.x, outputSize.y);
    
    int2   pixelCoord = int2(dispatchThreadID.xy);
    float2 uv         = float2(pixelCoord) / float2(outputSize);

    float3 position   = positionMap[pixelCoord].xyz;
    float3 normal     = normalMap[pixelCoord].xyz;
    float3 albedo     = albedoMap[pixelCoord].xyz;
    float3 arm        = armMap[pixelCoord].xyz;
    float3 emissive   = emissiveMap[pixelCoord].xyz;
    // float  ssao       = sampleSSAO(uv);

    // float ao        = arm.x * ssao;
    float ao        = arm.x;
    float roughness = arm.y;
    float metallic  = arm.z;

    float3 N = normalize(normal);
    float3 V = normalize(lighting.CameraPosition.xyz - position);
    float3 R = reflect(-V, N);

    float NdotV = max(dot(N, V), 0.0);

    // Tint reflections for metallic surfaces.
    float3 F0 = lerp(float3(PBR::F_DIELECTRIC), albedo, metallic);

    float3 Lo = emissive * EMISSIVE_INTENSITY;

    // Ambient Light
    {
        float3 F  = PBR::specularReflection(NdotV, F0, roughness);
        float3 kS = F;
        float3 kD = (1.0 - kS) * (1.0 - metallic);

        float3 diffuse   = albedo * irradianceMap.SampleLevel(N, 0).rgb;
        float3 prefilter = prefilteredEnvMap.SampleLevel(R, roughness * MAX_REFLECTION_LOD).rgb;
        float2 brdf      = brdfLUTMap.SampleLevel(float2(NdotV, roughness), 0).rg;
        float3 specular  = prefilter * (kS * brdf.x + brdf.y);

        float3 ambient   = (kD * diffuse + specular) * ao;

        Lo += ambient * lighting.AmbientLight.Color.rgb * lighting.AmbientLight.Color.w;
    }

    // Directional Light
    {
        float3 L        = normalize(lighting.DirectionalLight.Direction.xyz);
        float3 radiance = lighting.DirectionalLight.Color.rgb * lighting.DirectionalLight.Color.w;

        Lo += PBR::computeLighting(N, V, L, albedo, roughness, metallic, radiance, F0);
    }

    // Point Lights
    for (int i = 0; i < lighting.NumPointLights; i++)
    {
        PointLight light = lighting.PointLights[i];
        
        float3 L           = normalize(light.Position.xyz - position);
        float  attenuation = PBR::lightAttenuation(light.Position.xyz, position);
        float3 radiance    = light.Color.rgb * light.Color.w * attenuation;

        Lo += PBR::computeLighting(N, V, L, albedo, roughness, metallic, radiance, F0);
    }

    Lo = max(Lo, 0.0);
    sceneColorMap[pixelCoord] = float4(Lo, 1.0);

    // Debug Views
    if (lighting.DebugViewMode > 0)
    {
        switch (lighting.DebugViewMode)
        {
        case 1:
            sceneColorMap[pixelCoord] = float4(albedo, 1.0);
            break;
        case 2:
            sceneColorMap[pixelCoord] = float4(float3(ao), 1.0);
            break;
        case 3:
            sceneColorMap[pixelCoord] = float4(float3(roughness), 1.0);
            break;
        case 4:
            sceneColorMap[pixelCoord] = float4(float3(metallic), 1.0);
            break;
        case 5:
            sceneColorMap[pixelCoord] = float4(emissive, 1.0);
            break;
        }
    }
}

// ================================================================================================
