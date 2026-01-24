#version 450


layout(push_constant, std140) uniform Push 
{
    mat4 View;
    mat4 Projection;

} push;

layout(location = 0) in  vec3 inPosition;
layout(location = 0) out vec3 outPosition;

void main()
{
	outPosition = inPosition;

	vec4 clipPosition = push.Projection * mat4(mat3(push.View)) * vec4(outPosition, 1.0);

	gl_Position = clipPosition.xyww; // Ensure maximum depth (z = 1.0)
}