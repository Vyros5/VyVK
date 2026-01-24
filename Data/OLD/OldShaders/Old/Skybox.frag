#version 450

layout (location = 0) in  vec3 fragUV;
layout (location = 1) in  vec3 fragPosition;

layout (location = 0) out vec4 outColor;

struct PointLight
{
    vec4 Position; // w is radius
    vec4 Color;    // w is intensity
};

// Has to be the same order as GlobalUBO struct or data will be in the wrong position.
layout(set = 0, binding = 0) uniform GlobalUBO 
{
    mat4       Projection;
    mat4       View;
    mat4       InverseView;
    vec4       AmbientLightColor; // w is intensity
    PointLight PointLights[10];
    int        NumLights;

} ubo;

const vec3 skyColorTop     = vec3(0, 0.282, 0.729);
const vec3 skyColorHorizon = vec3(0.529, 0.808, 0.922);
const vec3 groundColor     = vec3(0.5, 0.5, 0.5);

void main() 
{
	float t = clamp((fragPos.y + 1.0) / 2.0, 0.0, 1.0);
	
    vec3 skyColor;
	
    if (fragPos.y < 0.0)
	{
		skyColor = mix(skyColorTop, skyColorHorizon, t);
	}
	else
	{
		skyColor = mix(skyColorHorizon, groundColor, t);
	}
	
    vec3 fragColor = skyColor;

	outColor = vec4(fragColor, 1.0);
}