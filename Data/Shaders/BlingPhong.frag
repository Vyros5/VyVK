#version 450

// ================================================================================================
// Input

layout (location = 0) in vec3 fragPosWorld;
layout (location = 1) in vec3 fragNormalWorld;
layout (location = 2) in vec3 fragColor;
layout (location = 3) in vec2 fragUV;

// ================================================================================================
// Output

layout (location = 0) out vec4 outColor;

// ================================================================================================
// Structs

struct PointLight 
{
    vec4 Position; // xyz = position, w = unused
    vec4 Color;    // rgb = color,    a = intensity
};

// ================================================================================================
// Uniforms

layout (set = 0, binding = 0) uniform GlobalUBO 
{
    mat4             Projection;
    mat4             View;
    mat4             InverseView;
    vec4             AmbientLightColor; // rgb = color, a = intensity
    PointLight       PointLights      [ 16 ];
    int              NumPointLights;

} uUbo;

layout (push_constant) uniform Push 
{
    mat4  ModelMatrix;
    mat4  NormalMatrix;

} uPush;

// ================================================================================================

void main() 
{
    vec3 diffuseLight  = uUbo.AmbientLightColor.xyz * uUbo.AmbientLightColor.w;
    vec3 specularLight = vec3(0.0);
    vec3 surfaceNormal = normalize( fragNormalWorld );

    vec3 cameraPosWorld = uUbo.InverseView[3].xyz;
    vec3 viewDirection  = normalize( cameraPosWorld - fragPosWorld );

    // [ Point lights ]
    for (int i = 0; i < uUbo.NumPointLights; i++) 
    {
        PointLight light = uUbo.PointLights[ i ];

        vec3  directionToLight = light.Position.xyz - fragPosWorld;
        float attenuation      = 1.0 / dot( directionToLight, directionToLight ); // distance squared

        directionToLight = normalize( directionToLight );

        float cosAngleIncidence = max( dot( surfaceNormal, directionToLight ), 0.0 );
        vec3  lightColor        = light.Color.xyz * light.Color.w * attenuation; // w is intensity

        diffuseLight += lightColor * cosAngleIncidence;

        // [ Specular Lighting ]
        vec3  halfAngle = normalize( directionToLight + viewDirection );
		float blinnTerm = dot( surfaceNormal, halfAngle );
		blinnTerm = clamp( blinnTerm, 0.0, 1.0 );
		blinnTerm = pow  ( blinnTerm, 512.0); // higher values -> sharper highlight

        specularLight += lightColor * blinnTerm;
    }

    vec3 finalColor = diffuseLight * fragColor + specularLight * fragColor;

    outColor = vec4(finalColor, 1.0);
}