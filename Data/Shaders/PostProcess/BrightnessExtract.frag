#version 450

layout(location = 0) in vec2 fragUV;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform sampler2D hdrTexture;

layout(push_constant) uniform Push 
{
    float Threshold;

} uPush;

void main() 
{
    vec3 color = texture(hdrTexture, fragUV).rgb;
    
    // Calculate luminance
    float brightness = dot(color, vec3(0.2126, 0.7152, 0.0722));
    
    // Extract bright pixels above threshold
    if (brightness > uPush.Threshold) 
    {
        outColor = vec4(color, 1.0);
    } 
    else 
    {
        outColor = vec4(0.0, 0.0, 0.0, 1.0);
    }
}