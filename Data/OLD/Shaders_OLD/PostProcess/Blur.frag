#version 450

// ================================================================================================
// Uniforms

layout(set = 0, binding = 0) uniform sampler2D uInputTexture;

layout(push_constant) uniform Push 
{
    vec2 Direction;

} uPush;

// ================================================================================================
// Input

layout(location = 0) in vec2 fragUV;

// ================================================================================================
// Output

layout(location = 0) out vec4 outColor;

// ================================================================================================

// Gaussian blur weights
const float WEIGHTS[5] = float[]
( 
    0.227027, 
    0.1945946, 
    0.1216216, 
    0.054054, 
    0.016216 
);

void main() 
{
    vec2 texelSize = 1.0 / textureSize(uInputTexture, 0);
    vec3 result    = texture(uInputTexture, fragUV).rgb * WEIGHTS[0];
    
    for(int i = 1; i < 5; i++) 
    {
        vec2 offset = uPush.Direction * texelSize * float(i);
        
        result += texture(uInputTexture, fragUV + offset).rgb * WEIGHTS[i];
        result += texture(uInputTexture, fragUV - offset).rgb * WEIGHTS[i];
    }
    
    outColor = vec4(result, 1.0);
}