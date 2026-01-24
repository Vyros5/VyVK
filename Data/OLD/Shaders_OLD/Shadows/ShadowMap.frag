#version 450

layout(location = 0) out vec4 color;

void main() 
{
    // Depth is automatically written to depth buffer.
    
    color = vec4(1.0, 1.0, 1.0, 1.0);
}