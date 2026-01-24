#version 450

layout (input_attachment_index = 0, binding = 0) uniform subpassInput uSamplerNormal;
layout (input_attachment_index = 1, binding = 1) uniform subpassInput uSamplerAlbedo;
layout (input_attachment_index = 2, binding = 2) uniform subpassInput uSamplerPositionDepth;
layout                             (binding = 4) uniform samplerCube  uSamplerShadowCube;

layout (location = 0) out vec4 outColor;

layout (set = 0, binding = 3) uniform CompositionUbo 
{
	vec3 ViewPos;
	vec4 AmbientLightColor; //w is intensity
	vec3 LightPosition;
	vec4 LightColor; // w is light intensity

} uUbo;

layout (push_constant) uniform Push 
{
	mat4 InvViewProj;
	vec2 InvResolution;

} uPush;

const float kSpecularStrength =  8.0;
const float kShininess        = 32.0;
const float EPSILON           = 0.15;

void main() {
	
	// Read previous pass shadow depth & G-Buffer values from previous sub pass
	vec2 clipUV = gl_FragCoord.xy * uPush.InvResolution;
	vec2 clipXY = clipUV * 2.0 - 1.0;

	vec4 clipScene = vec4(clipXY, subpassLoad(uSamplerPositionDepth).x, 1.0);

	vec4 fragPosWorld_w = uPush.InvViewProj * clipScene;
	vec3 fragPosWorld   = fragPosWorld_w.xyz / fragPosWorld_w.w;

	//Calculate shadow
	vec3  inDirToLight = fragPosWorld - uUbo.LightPosition;
	float dist         = length(inDirToLight);

	float depth  = texture(uSamplerShadowCube, vec3(inDirToLight.x, -inDirToLight.y, inDirToLight.z)).r;
	float shadow = dist < (depth + EPSILON) ? 0.0 : 0.5;

	vec3 normal    = subpassLoad(uSamplerNormal).xyz;
	vec4 fragColor = subpassLoad(uSamplerAlbedo);

	vec3 directionToView  = normalize(uUbo.ViewPos - fragPosWorld);
	vec3 directionToLight = normalize(-inDirToLight);
	vec3 halfwayDirection = normalize(directionToLight + directionToView);

	float attenuation = 1.0 / dot(-inDirToLight, -inDirToLight); // distance squared

	vec3 directionToReflection = reflect(-directionToLight, normal);  

	vec3 lightColor   = uUbo.LightColor.xyz * uUbo.LightColor.w * attenuation;
	vec3 ambientLight = uUbo.AmbientLightColor.xyz * uUbo.AmbientLightColor.w;
	vec3 diffuseLight = lightColor * max(dot(normal, normalize(directionToLight)), 0);

	float spec          = pow(max(dot(normal, halfwayDirection), 0.0), kShininess);
	vec3  specularLight = kSpecularStrength * spec * lightColor;
	
	outColor = vec4((diffuseLight + (1.0 - shadow) * ambientLight + specularLight) * fragColor.xyz, fragColor.a);
}