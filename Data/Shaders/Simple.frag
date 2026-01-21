#version 450

layout (location = 0) in vec3 fragPosition;
layout (location = 1) in vec3 fragColor;
layout (location = 2) in vec3 fragNormal;
layout (location = 3) in vec4 fragTangent;
layout (location = 4) in vec2 fragUV;

layout (location = 0) out vec4 outColor;

struct PointLight 
{
    vec4 Position;  // ignore w
    vec4 Color;     // w is intensity
};

layout (set = 0, binding = 0) uniform UBO
{
    mat4             Projection;
    mat4             View;
    mat4             InverseView;
    vec4             AmbientLightColor;
    PointLight       PointLights[ 16 ];
    int              LightCount;

} uUbo;

layout (set = 0 ,binding = 1) uniform sampler2D uSampler;

layout (push_constant) uniform Push 
{
    mat4 ModelMatrix; 
    mat4 NormalMatrix;

} uPush;

void main() 
{
    vec3 diffuseLight  = uUbo.AmbientLightColor.xyz * uUbo.AmbientLightColor.w;
    vec3 specularLight = vec3(0.0);
    vec3 surfaceNormal = normalize(fragNormal);

    vec3 cameraPosWorld = uUbo.InverseView[3].xyz;
    vec3 viewDirection  = normalize(cameraPosWorld - fragPosition);

    for (int i = 0; i < uUbo.LightCount; i++)
    {
        PointLight light = uUbo.PointLights[ i ];

        vec3 directionToLight = light.Position.xyz - fragPosition;
        float attenuation     = 1.0 / dot(directionToLight, directionToLight); // distance squared easy way to calculate it 
        
        directionToLight = normalize(directionToLight);

        float cosAngIncidence = max(dot(surfaceNormal, directionToLight), 0);
        vec3 intensity        = light.Color.xyz * light.Color.w * attenuation;

        diffuseLight += intensity * cosAngIncidence;

        //specular Light
        vec3  halfAngle = normalize(directionToLight + viewDirection);
        float blinnTerm = dot(surfaceNormal, halfAngle);
        blinnTerm = clamp(blinnTerm, 0.0, 1.0);
        blinnTerm = pow(blinnTerm, 32.0);
        
        specularLight += intensity * blinnTerm;
    }
    // no control term for specularlight yet 

    vec3 imageColor = texture(uSampler, fragUV).rgb;

    outColor = vec4((diffuseLight * fragColor + specularLight * fragColor) * imageColor, 1.0);
}