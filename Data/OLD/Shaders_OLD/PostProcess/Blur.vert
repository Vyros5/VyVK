#version 450

// ================================================================================================
// Input

// ================================================================================================
// Output

layout(location = 0) out vec2 fragUV;

// ================================================================================================

void main() 
{
    // Generate a fullscreen triangle
    vec2 POSITIONS[3] = vec2[](
        vec2(-1.0, -1.0),
        vec2( 3.0, -1.0),
        vec2(-1.0,  3.0)
    );
    
    vec2 TEX_COORDS[3] = vec2[](
        vec2(0.0, 0.0),
        vec2(2.0, 0.0),
        vec2(0.0, 2.0)
    );
    
    gl_Position = vec4(POSITIONS[gl_VertexIndex], 0.0, 1.0);

    fragUV = TEX_COORDS[gl_VertexIndex];
}