#version 450
#pragma shader_stage(fragment)
// #extension GL_KHR_vulkan_glsl: enable

#include "../../Shared/Defines/GPUSharedDefines.h"

layout (location = 0) in vec3 fragPosition;
layout (location = 1) in vec3 fragColor;
layout (location = 2) in vec3 fragNormal;
layout (location = 3) in vec4 fragTangent;
layout (location = 4) in vec2 fragUV;
layout (location = 5) in vec2 fragUV1;

layout (set = 1, binding = 1) uniform sampler2D uDiffuseMap;
layout (set = 1, binding = 2) uniform sampler2D uNormalMap;
layout (set = 1, binding = 3) uniform sampler2D roughnessMetallicMap;
layout (set = 1, binding = 4) uniform sampler2D uEmissiveMap;
layout (set = 1, binding = 5) uniform sampler2D uRoughnessMap;
layout (set = 1, binding = 6) uniform sampler2D uMetallicMap;

layout (location = 0) out vec4 outColor;

struct PointLight {
    vec4 Position; // ignore w
    vec4 Color; // w is intensity
};

struct DirectionalLight {
    vec4 Direction;  // ignore w
    vec4 Color;     // w is intensity
};

layout(set = 0, binding = 0) uniform UBO
{
    mat4             Projection;
    mat4             View;
    mat4             InverseView;
    vec4             AmbientLightColor;
    PointLight       PointLights[MAX_LIGHTS];
    DirectionalLight DirectLight;
    int              LightCount;
    float            Gamma;
	float            Exposure;

} uUbo;

void main(){
    // outColor = vec4(fragColor, 0.0);
    outColor = vec4(fragColor, 1.0);
}

// layout (set = 1, binding = 0) uniform MaterialUbo 
// {
//     int   Features;
//     float Roughness;
//     float Metallic;
//     float _pad0; 

//     // byte 16 to 31
//     vec4 DiffuseColor;

//     // byte 32 to 47
//     vec3  EmissiveColor;
//     float EmissiveStrength;

//     // byte 48 to 63
//     float NormalMapIntensity;
//     float _pad1;
//     float _pad2;
//     float _pad3;

//     // byte 64 to 128
//     vec4 _pad4[4];

// } uMaterial;

// layout(push_constant) uniform Push 
// {
//     mat4 ModelMatrix;
//     mat4 NormalMatrix;

// } uPush;

// const float PI = 3.14159265359;

// vec3 Uncharted2Tonemap(vec3 x) 
// {
//     float A = 0.15;
//     float B = 0.50;
//     float C = 0.10;
//     float D = 0.20;
//     float E = 0.02;
//     float F = 0.30;
//     return ((x * (A * x + C * B) + D * E) / (x * (A * x + B) + D * F)) - E / F;
// }

// vec3 Uncharted2(vec3 color) 
// {
//     const float W = 11.2;
//     //  float exposureBias = 2.0;
//     float exposureBias = uUbo.Exposure;
//     vec3 curr = Uncharted2Tonemap(exposureBias * color);
//     vec3 whiteScale = 1.0 / Uncharted2Tonemap(vec3(W));
//     return curr * whiteScale;
// }

// float DistributionGGX(vec3 N, vec3 H, float roughness) 
// {
//     float a  = roughness * roughness;
//     float a2 = a * a;
//     float NdotH  = max(dot(N, H), 0.0);
//     float NdotH2 = NdotH*NdotH;

//     float nom   = a2;
//     float denom = (NdotH2 * (a2 - 1.0) + 1.0);

//     denom = PI * denom * denom;

//     return nom / denom;
// }

// float GeometrySchlickGGX(float NdotV, float roughness) 
// {
//     float r = (roughness + 1.0);
//     float k = (r * r) / 8.0;

//     float nom   = NdotV;
//     float denom = NdotV * (1.0 - k) + k;

//     return nom / denom;
// }

// float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) 
// {
//     float NdotV = max(dot(N, V), 0.0);
//     float NdotL = max(dot(N, L), 0.0);
//     float ggx2  = GeometrySchlickGGX(NdotV, roughness);
//     float ggx1  = GeometrySchlickGGX(NdotL, roughness);

//     return ggx1 * ggx2;
// }

// vec3 FresnelSchlick(float cosTheta, vec3 F0) 
// {
//     return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
// }

// float Rand(vec2 co) 
// {
//     return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
// }

// void main() 
// {
//     vec3 AmbientLightColor = uUbo.AmbientLightColor.xyz * uUbo.AmbientLightColor.w;
//     vec3 specularLight     = vec3(0.0);

//     vec3 surfaceNormal  = normalize(fragNormal);
//     vec3 cameraPosWorld = uUbo.View[3].xyz;
//     vec3 viewDirection  = normalize(cameraPosWorld - fragPosition);
    
//     // [ DIFFUSE ]
//     vec4 diffuseColor;
//     if (bool(uMaterial.Features & GLSL_HAS_DIFFUSE_MAP)) 
//     {
//         diffuseColor = texture(uDiffuseMap, fragUV) * uMaterial.DiffuseColor;
//     }
//     else {
//         diffuseColor = fragColor;
//     }

//     if (diffuseColor.a < 0.5) 
//     {
//         discard;
//     }

//     // [ NORMAL ]
//     vec4 normal;
//     vec3 N = normalize( fragNormal  );
//     vec3 T = normalize( fragTangent );
//     // Gram Schmidt
//     T        = normalize(T - dot(T, N) * N);
//     vec3 B   = cross(N, T);
//     mat3 TBN = mat3(T, B, N);

//     float normalMapIntensity  = uMaterial.NormalMapIntensity;
//     vec3  normalTangentSpace;

//     if (bool( uMaterial.Features & GLSL_HAS_NORMAL_MAP )) 
//     {
//         normalTangentSpace = texture(uNormalMap, fragUV).xyz * 2 - vec3(1.0, 1.0, 1.0);
//         normalTangentSpace = mix(vec3(0.0, 0.0, 1.0), normalTangentSpace, normalMapIntensity);
//         surfaceNormal      = normalize(TBN * normalTangentSpace);
        
//         normal = vec4(surfaceNormal, 1.0);
//     } 
//     else 
//     {
//         surfaceNormal = N;
//         normal = vec4(N, 1.0);
//     }


//     // [ METALLIC ROUGHNESS ]
//     float roughness;
//     float metallic;

//     if (bool(uMaterial.Features & GLSL_HAS_ROUGHNESS_METALLIC_MAP)) 
//     {
//         roughness = texture(roughnessMetallicMap, fragUV).g;
//         metallic  = texture(roughnessMetallicMap, fragUV).b;
//     } 
//     else 
//     {
//         // [ ROUGHNESS ]
//         if (bool(uMaterial.Features & GLSL_HAS_ROUGHNESS_MAP)) 
//         {
//             roughness = texture(uRoughnessMap, fragUV).r; // gray scale
//         } 
//         else {
//             roughness = uMaterial.Roughness;
//         }

//         // [ METALLIC ]
//         if (bool(uMaterial.Features & GLSL_HAS_METALLIC_MAP)) 
//         {
//             metallic = texture(uMetallicMap, fragUV).r; // gray scale
//         }
//         else {
//             metallic = uMaterial.Metallic;
//         }
//     }
//     vec4 material = vec4(normalMapIntensity, roughness, metallic, 0.0); // NOT USED ATM

//     // [ EMISSIVE ]
//     vec4 emissive;
//     vec4 emissiveColor = vec4(uMaterial.EmissiveColor.r, uMaterial.EmissiveColor.g, uMaterial.EmissiveColor.b, 1.0);

//     if (bool(uMaterial.Features & GLSL_HAS_EMISSIVE_MAP)) 
//     {
//         vec4 fragEmissiveColor = texture(uEmissiveMap, fragUV);

//         emissive = fragEmissiveColor * emissiveColor * uMaterial.EmissiveStrength;
//     } 
//     else {
//         emissive = emissiveColor * uMaterial.EmissiveStrength;
//     }

//     vec3 camPos = (inverse(uUbo.View) * vec4(0.0,0.0,0.0,1.0)).xyz;

//     N      = normalize(surfaceNormal);
//     vec3 V = normalize(camPos - fragPosition);

//     vec3 fragColor = diffuseColor.rgb;
//     vec3 F0        = vec3(0.04); 
//     F0             = mix(F0, fragColor, metallic);
//     // reflectance equation
//     vec3 Lo = vec3(0.0);

//     for (int i = 0; i < uUbo.LightCount; i++) 
//     {
//         PointLight light = uUbo.PointLights[i];
//         // calculate per-light radiance
//         vec3  L = normalize(light.Position.xyz - fragPosition);
//         vec3  H = normalize(V + L);
//         float directionToLight = length(light.Position.xyz - fragPosition);
//         float attenuation      = 1.0 / (directionToLight * directionToLight);
//         float lightIntensity   = light.Color.w;
//         vec3  radiance         = light.Color.rgb * lightIntensity * attenuation;

//         // Cook-Torrance BRDF
//         float NDF = DistributionGGX(N, H, roughness);   
//         float G   = GeometrySmith(N, V, L, roughness);  
//         vec3  F   = FresnelSchlick(clamp(dot(H, V), 0.0, 1.0), F0);

//         vec3  numerator   = NDF * G * F; 
//         float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001; // + 0.0001 to prevent divide by zero
//         vec3  specular    = numerator / denominator;

//         // kS is equal to Fresnel
//         vec3 kS = F;
//         // for energy conservation, the diffuse and specular light can't
//         // be above 1.0 (unless the surface emits light); to preserve this
//         // relationship the diffuse component (kD) should equal 1.0 - kS.
//         vec3 kD = vec3(1.0) - kS;
//         // multiply kD by the inverse metalness such that only non-metals 
//         // have diffuse lighting, or a linear blend if partly metal (pure metals
//         // have no diffuse light).
//         kD *= 1.0 - metallic;  

//         // scale light by NdotL
//         float NdotL = max(dot(N, L), 0.0);

//         // add to outgoing radiance Lo
//         Lo += (kD * fragColor / PI + specular) * radiance * NdotL;  // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
//     }

//     {
//         // calculate radiance for a directional light
//         vec3 L = normalize(-uUbo.DirectLight.Direction.xyz);
//         vec3 H = normalize(V + L);
//         float lightIntensity = uUbo.DirectLight.Color.w;
//         vec3  radiance       = uUbo.DirectLight.Color.rgb * lightIntensity;

//         // Cook-Torrance BRDF
//         float NDF = DistributionGGX(N, H, roughness);   
//         float G   = GeometrySmith(N, V, L, roughness);  
//         vec3  F   = FresnelSchlick(clamp(dot(H, V), 0.0, 1.0), F0);

//         vec3  numerator   = NDF * G * F; 
//         float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001; // + 0.0001 to prevent divide by zero
//         vec3  specular    = numerator / denominator;

//         // kS is equal to Fresnel
//         vec3 kS = F;
//         // for energy conservation, the diffuse and specular light can't
//         // be above 1.0 (unless the surface emits light); to preserve this
//         // relationship the diffuse component (kD) should equal 1.0 - kS.
//         vec3 kD = vec3(1.0) - kS;
//         // multiply kD by the inverse metalness such that only non-metals 
//         // have diffuse lighting, or a linear blend if partly metal (pure metals
//         // have no diffuse light).
//         kD *= 1.0 - metallic;  

//         // scale light by NdotL
//         float NdotL = max(dot(N, L), 0.0);
//         float litPercentage = 1.0;

//         // add to outgoing radiance Lo
//         Lo += (kD * fragColor / PI + specular) * radiance * NdotL * litPercentage;  // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
//     }

//     vec3 color = AmbientLightColor + Lo;
//     color      = Uncharted2(color);

//     // Set Gamma as 2.0, may set it changable in future
// //	color = pow(color, vec3(1.0 / 2.0));
// 	color = pow(color, vec3(1.0 / uUbo.Gamma));

//     outColor = diffuseColor * vec4(color, 1.0);
// }