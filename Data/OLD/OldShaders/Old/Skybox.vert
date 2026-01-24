#version 450

layout(location = 0) out vec2 fragUV;
layout(location = 1) out vec3 fragPosition;

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

const vec3 VERTICES[36] = vec3[](

    // Front face (z = 1.0)
    vec3(-0.5, -0.5,  0.5), vec3( 0.5,  0.5,  0.5), vec3( 0.5, -0.5,  0.5),
    vec3( 0.5,  0.5,  0.5), vec3(-0.5, -0.5,  0.5), vec3(-0.5,  0.5,  0.5),

    // Back face (z = -1.0)
    vec3( 0.5, -0.5, -0.5), vec3(-0.5,  0.5, -0.5), vec3(-0.5, -0.5, -0.5), 
    vec3(-0.5,  0.5, -0.5), vec3( 0.5, -0.5, -0.5), vec3( 0.5,  0.5, -0.5), 
    
    // Top face (y = 1.0)
    vec3(-0.5,  0.5, -0.5), vec3( 0.5,  0.5,  0.5), vec3(-0.5,  0.5,  0.5), 
    vec3( 0.5,  0.5,  0.5), vec3(-0.5,  0.5, -0.5), vec3( 0.5,  0.5, -0.5), 

    // Bottom face (y = -1.0)
    vec3(-0.5, -0.5,  0.5), vec3( 0.5, -0.5, -0.5), vec3(-0.5, -0.5, -0.5), 
    vec3( 0.5, -0.5, -0.5), vec3(-0.5, -0.5,  0.5), vec3( 0.5, -0.5,  0.5), 
    
    // Right face (x = 1.0)
    vec3( 0.5, -0.5,  0.5), vec3( 0.5,  0.5, -0.5), vec3( 0.5, -0.5, -0.5),
    vec3( 0.5,  0.5, -0.5), vec3( 0.5, -0.5,  0.5), vec3( 0.5,  0.5,  0.5),
    
    // Left face (x = -1.0)
    vec3(-0.5, -0.5, -0.5), vec3(-0.5,  0.5,  0.5), vec3(-0.5, -0.5,  0.5),
    vec3(-0.5,  0.5,  0.5), vec3(-0.5, -0.5, -0.5), vec3(-0.5,  0.5, -0.5)
);

const vec2 UVS[36] = vec2[](

    // Front face
    vec2(0.0, 0.0), vec2(1.0, 1.0), vec2(1.0, 0.0), 
    vec2(1.0, 1.0), vec2(0.0, 0.0), vec2(0.0, 1.0), 
    
    // Back face
    vec2(1.0, 0.0), vec2(0.0, 1.0), vec2(0.0, 0.0), 
    vec2(0.0, 1.0), vec2(1.0, 0.0), vec2(1.0, 1.0), 
    
    // Top face
    vec2(0.0, 1.0), vec2(1.0, 0.0), vec2(0.0, 0.0), 
    vec2(1.0, 0.0), vec2(0.0, 1.0), vec2(1.0, 1.0), 
    
    // Bottom face
    vec2(0.0, 0.0), vec2(1.0, 1.0), vec2(0.0, 1.0), 
    vec2(1.0, 1.0), vec2(0.0, 0.0), vec2(1.0, 0.0), 
    
    // Right face
    vec2(1.0, 0.0), vec2(0.0, 1.0), vec2(1.0, 1.0), 
    vec2(0.0, 1.0), vec2(1.0, 0.0), vec2(0.0, 0.0), 
    
    // Left face
    vec2(0.0, 0.0), vec2(1.0, 1.0), vec2(1.0, 0.0), 
    vec2(1.0, 1.0), vec2(0.0, 0.0), vec2(0.0, 1.0)
);


void main() 
{
	mat4 viewNoTranslation = mat4(mat3(ubo.View));

	vec4 pos = ubo.Projection * viewNoTranslation * vec4(VERTICES[gl_VertexIndex], 1.0);

	fragPosition = VERTICES[gl_VertexIndex].xyz;
	gl_Position  = pos.xyww;
	fragUV       = UVS[gl_VertexIndex];
}