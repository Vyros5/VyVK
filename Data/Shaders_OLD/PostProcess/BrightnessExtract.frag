#version 450

// ================================================================================================
// Uniforms

layout(set = 0, binding = 0) uniform sampler2D uHDRTexture;

layout(push_constant) uniform Push 
{
    float Threshold;

} uPush;

// ================================================================================================
// Input

layout(location = 0) in vec2 fragUV;

// ================================================================================================
// Output

layout(location = 0) out vec4 outColor;

// ================================================================================================

void main() 
{
    vec3 color = texture(uHDRTexture, fragUV).rgb;
    
    // Calculate luminance
    float brightness = dot(color, vec3(0.2126, 0.7152, 0.0722));
    
    // Extract bright pixels above threshold.
    if (brightness > uPush.Threshold) 
    {
        outColor = vec4(color, 1.0);
    } 
    else 
    {
        outColor = vec4(0.0, 0.0, 0.0, 1.0);
    }
}