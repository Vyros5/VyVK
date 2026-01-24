#version 450

#extension GL_EXT_multiview : enable

layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec4 vColor;
layout (location = 2) in vec3 vNormal;
layout (location = 3) in vec2 vUV;

layout (location = 0) out vec3 worldPos;
layout (location = 1) out vec3 lightPos;

layout (push_constant) uniform Push 
{
	mat4 ModelMatrix;

} uPush;

layout(set = 0, binding = 0) uniform ShadowUbo 
{
	mat4 LightProjectionView[6];
    vec3 LightPosition;

} uUbo;

const vec2 QUAD_VERTICES[6] = vec2[](
    vec2(-1.0, -1.0),
    vec2(-1.0,  1.0),
    vec2( 1.0, -1.0),
    vec2( 1.0, -1.0),
    vec2(-1.0,  1.0),
    vec2( 1.0,  1.0)
);

//Use depth buffer

void main() 
{
	vec4 inPos         = vec4(vPosition, 1.0);
	vec4 positionWorld = uPush.ModelMatrix * inPos;
	
    gl_Position = uUbo.LightProjectionView[ gl_ViewIndex ] * positionWorld;

	worldPos = positionWorld.xyz;
	lightPos = uUbo.LightPosition.xyz;
}