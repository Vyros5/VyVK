#version 450

// ================================================================================================

struct PointLight 
{
    vec4 Position; // ignore w
    vec4 Color;    // w is intensity
};

// ================================================================================================
// Uniforms

layout(set = 0, binding = 0) uniform GlobalUBO 
{
    mat4       Projection;
    mat4       View;
    mat4       InverseView;
    
    vec4       AmbientLightColor; // w is intensity
    PointLight PointLights[10];
    int        NumPointLights;

} uUbo;

layout(push_constant) uniform Push 
{
    mat4 ModelMatrix;
    mat4 NormalMatrix;

} uPush;

// ================================================================================================

// Input
layout(location = 0) in vec3 fragPosWorld;
layout(location = 1) in vec3 fragNormalWorld;
layout(location = 2) in vec3 fragColor;
layout(location = 3) in vec2 fragUV;

// Output
layout(location = 0) out vec4 outColor;

// ================================================================================================

void main() 
{
    vec2 texCoords = fragUV;

    vec3 surfaceNormal = normalize(fragNormalWorld);

    vec3 cameraPosWorld = uUbo.InverseView[3].xyz;
    vec3 viewDirection  = normalize(cameraPosWorld - fragPosWorld);

    vec3 diffuseLight  = uUbo.AmbientLightColor.xyz * uUbo.AmbientLightColor.w;
    vec3 specularLight = vec3(0.0);

    for (int i = 0; i < uUbo.NumPointLights; i++) 
    {
        PointLight light = uUbo.PointLights[i];
        
        vec3  directionToLight = light.Position.xyz - fragPosWorld;
        float attenuation      = 1.0 / dot(directionToLight, directionToLight); // distance squared

        directionToLight = normalize(directionToLight);

        float cosAngIncidence = max(dot(surfaceNormal, directionToLight), 0);
        vec3  intensity       = light.Color.xyz * light.Color.w * attenuation;

        diffuseLight += intensity * cosAngIncidence;

        // Specular lighting
        vec3  halfAngle = normalize(directionToLight + viewDirection);
        float blinnTerm = dot(surfaceNormal, halfAngle);
        
        blinnTerm = clamp(blinnTerm, 0, 1);
        blinnTerm = pow(blinnTerm, 512.0); // higher values -> sharper highlight
        
        specularLight += intensity * blinnTerm;
    }
    
    outColor = vec4(diffuseLight * fragColor + specularLight * fragColor, 1.0);
}
