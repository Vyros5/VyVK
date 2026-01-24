#version 450

// ================================================================================================
// CONSTANTS

#define CASCADE_SHADOW_MAP_COUNT 4

// ================================================================================================
// INPUT

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in vec2 aUV;
layout (location = 4) in vec4 aTangent;

// ================================================================================================
// DESCRIPTOR SET 1 : DIRECTIONAL LIGHT PROJECTIONS FOR CASCADED SHADOW MAP

layout (set = 1, binding = 0) uniform CascadedShadowPassUBO
{
	mat4 LightProjection[CASCADE_SHADOW_MAP_COUNT];
	vec4 CascadeSplits;

} uCascadedShadowPassUBO;

// ================================================================================================
// CONSTANT PUSH : CASCADED SHADOW MAP

layout (push_constant) uniform Push 
{
	mat4 ModelMatrix;
	int  CascadeIndex;

} uPush;

// ================================================================================================

void main()
{
	gl_Position = uCascadedShadowPassUBO.LightProjection[ uPush.CascadeIndex ] * uPush.ModelMatrix * vec4(aPosition, 1.0f);
}

// ================================================================================================