#version 450

layout (binding = 0) uniform sampler2D uUVReflection;
layout (binding = 1) uniform sampler2D uLightingMap;

layout (set = 0, binding = 2) uniform PostProcessingBufferUbo 
{
	vec2 InvResolution;

} ubo;

layout (location = 0) out vec4 outColor;

void main() 
{
	vec2 clipUV = gl_FragCoord.xy * ubo.InvResolution;

	vec4  uv    = texture(uUVReflection, clipUV);
	float alpha = clamp(uv.b, 0, 1);
	
	vec4 color          = texture(uLightingMap, clipUV);
	vec4 reflectedColor = texture(uLightingMap, uv.xy);

	vec4 sum = vec4(0.0);

	sum += texture(uLightingMap, uv.xy - 24 * ubo.InvResolution) * 0.0162162162;
	sum += texture(uLightingMap, uv.xy - 18 * ubo.InvResolution) * 0.0540540541;
	sum += texture(uLightingMap, uv.xy - 12 * ubo.InvResolution) * 0.1216216216;
	sum += texture(uLightingMap, uv.xy -  6 * ubo.InvResolution) * 0.1945945946;
	
	sum += texture(uLightingMap, uv.xy) * 0.2270270270;
	
	sum += texture(uLightingMap, uv.xy +  6 * ubo.InvResolution) * 0.1945945946;
	sum += texture(uLightingMap, uv.xy + 12 * ubo.InvResolution) * 0.1216216216;
	sum += texture(uLightingMap, uv.xy + 18 * ubo.InvResolution) * 0.0540540541;
	sum += texture(uLightingMap, uv.xy + 24 * ubo.InvResolution) * 0.0162162162;

	outColor = vec4(mix(color, vec4(sum.xyz, 1.0), alpha / 2.0).xyz, 1.0);
}