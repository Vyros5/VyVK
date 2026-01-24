#version 450

layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec4 vColor;
layout (location = 2) in vec3 vNormal;
layout (location = 3) in vec2 kUV;

layout (location = 0) out vec4 fragColor;
layout (location = 1) out vec4 fragNormalWorld;

layout(set = 0, binding = 0) uniform GBufferUbo 
{
	mat4 Projection;
	mat4 View;
	vec3 LightPosition;

} uUbo;

layout(push_constant) uniform Push 
{
	mat4 ModelMatrix; //Projection * View * model
	mat4 NormalMatrix;

} uPush;

void main() 
{
	vec4 positionWorld = uPush.ModelMatrix * vec4(vPosition, 1.0);

	gl_Position = uUbo.Projection * uUbo.View * positionWorld;

	fragColor       = vColor;
	fragNormalWorld = vec4(normalize(mat3(uPush.NormalMatrix) * vNormal), 1.0);
}