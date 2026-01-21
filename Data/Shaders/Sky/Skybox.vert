#version 450

layout (location = 0) out vec3 fragUVW;

// ================================================================================================
// Uniforms

layout (set = 0, binding = 0) uniform GlobalUBO 
{
    mat4 Projection;
    mat4 View;
    mat4 InverseView;
    //...

} uUbo;

// ================================================================================================

// Vertices for a unit cube
vec3 POSITIONS2[36] = vec3[](

    vec3(-1.0f,  1.0f, -1.0f), 
    vec3(-1.0f, -1.0f, -1.0f),
    vec3( 1.0f, -1.0f, -1.0f),
    vec3( 1.0f, -1.0f, -1.0f),
    vec3( 1.0f,  1.0f, -1.0f),
    vec3(-1.0f,  1.0f, -1.0f),

    vec3(-1.0f, -1.0f,  1.0f), 
    vec3(-1.0f, -1.0f, -1.0f),
    vec3(-1.0f,  1.0f, -1.0f),
    vec3(-1.0f,  1.0f, -1.0f),
    vec3(-1.0f,  1.0f,  1.0f),
    vec3(-1.0f, -1.0f,  1.0f),

    vec3( 1.0f, -1.0f, -1.0f),
    vec3( 1.0f, -1.0f,  1.0f),
    vec3( 1.0f,  1.0f,  1.0f),
    vec3( 1.0f,  1.0f,  1.0f),
    vec3( 1.0f,  1.0f, -1.0f),
    vec3( 1.0f, -1.0f, -1.0f),

    vec3(-1.0f, -1.0f,  1.0f),
    vec3(-1.0f,  1.0f,  1.0f),
    vec3( 1.0f,  1.0f,  1.0f),
    vec3( 1.0f,  1.0f,  1.0f),
    vec3( 1.0f, -1.0f,  1.0f),
    vec3(-1.0f, -1.0f,  1.0f),

    vec3(-1.0f,  1.0f, -1.0f),
    vec3( 1.0f,  1.0f, -1.0f),
    vec3( 1.0f,  1.0f,  1.0f),
    vec3( 1.0f,  1.0f,  1.0f),
    vec3(-1.0f,  1.0f,  1.0f),
    vec3(-1.0f,  1.0f, -1.0f),

    vec3(-1.0f, -1.0f, -1.0f),
    vec3(-1.0f, -1.0f,  1.0f),
    vec3( 1.0f, -1.0f, -1.0f),
    vec3( 1.0f, -1.0f, -1.0f),
    vec3(-1.0f, -1.0f,  1.0f),
    vec3( 1.0f, -1.0f,  1.0f)
);

// Cube vertices (no vertex buffer needed - generate from gl_VertexIndex)
vec3 POSITIONS[ 36 ] = vec3[]
(
    // Front face
    vec3(-1.0, -1.0,  1.0),
    vec3( 1.0, -1.0,  1.0),
    vec3 (1.0,  1.0,  1.0),
    vec3( 1.0,  1.0,  1.0),
    vec3(-1.0,  1.0,  1.0),
    vec3(-1.0, -1.0,  1.0),
    // Back face
    vec3( 1.0, -1.0, -1.0),
    vec3(-1.0, -1.0, -1.0),
    vec3(-1.0,  1.0, -1.0),
    vec3(-1.0,  1.0, -1.0),
    vec3( 1.0,  1.0, -1.0),
    vec3( 1.0, -1.0, -1.0),
    // Top face
    vec3(-1.0,  1.0,  1.0),
    vec3( 1.0,  1.0,  1.0),
    vec3( 1.0,  1.0, -1.0),
    vec3( 1.0,  1.0, -1.0),
    vec3(-1.0,  1.0, -1.0),
    vec3(-1.0,  1.0,  1.0),
    // Bottom face
    vec3(-1.0, -1.0, -1.0),
    vec3( 1.0, -1.0, -1.0),
    vec3( 1.0, -1.0,  1.0),
    vec3( 1.0, -1.0,  1.0),
    vec3(-1.0, -1.0,  1.0),
    vec3(-1.0, -1.0, -1.0),
    // Right face
    vec3( 1.0, -1.0,  1.0),
    vec3( 1.0, -1.0, -1.0),
    vec3( 1.0,  1.0, -1.0),
    vec3( 1.0,  1.0, -1.0),
    vec3( 1.0,  1.0,  1.0),
    vec3( 1.0, -1.0,  1.0),
    // Left face
    vec3(-1.0, -1.0, -1.0),
    vec3(-1.0, -1.0,  1.0),
    vec3(-1.0,  1.0,  1.0),
    vec3(-1.0,  1.0,  1.0),
    vec3(-1.0,  1.0, -1.0),
    vec3(-1.0, -1.0, -1.0)
);

// layout (push_constant) uniform PushConstants
// {
//     mat4 ViewProjection;

// } uPush;


void main()
{
    // Get the vertex position from the hardcoded array
    vec3 pos = POSITIONS2[ gl_VertexIndex ];

    // Remove translation from the view matrix for the skybox
    // This makes the skybox appear infinitely far away and fixed
    mat4 rotView = mat4(mat3(uUbo.View));

    // Calculate the clip-space position.
    gl_Position = uUbo.Projection * rotView * vec4(pos, 1.0);

    // Set z-component to w for max depth (ensures it's drawn behind everything).
    gl_Position.z = gl_Position.w;

    // The output texture coordinate is simply the vertex position,
    // which will be used as the cubemap lookup vector in the fragment shader.
    fragUVW = pos;

    // Flip y-axis for vulkan.
    fragUVW.y *= -1;
    
    // // Use position as texture coordinate (cubemap sampling)
    // fragTexCoord = pos;

    // // Transform position by view-projection
    // // Note: We remove translation from view matrix so skybox stays at origin
    // vec4 clipPos = uPush.ViewProjection * vec4(pos, 1.0);

    // // Set z = w so depth is always 1.0 (at far plane)
    // // This ensures skybox renders behind everything
    // gl_Position = clipPos.xyww;
}