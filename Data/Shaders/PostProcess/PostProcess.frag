#version 450

layout(location = 0) in vec2 fragUV;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform sampler2D sceneTexture;
layout(set = 0, binding = 1) uniform sampler2D bloomTexture;

layout(push_constant) uniform Push 
{
    float BloomIntensity;
    float Exposure;
    float Gamma;
    int   bloomEnabled;
    float Contrast;
    float Saturation;
    float Vibrance;

} uPush;

// ================================================================================================

// HLSL Saturate
// https://github.com/DrDesten/ShaderCore/blob/ea198ee5547733ffabf78de47bf462808fe7447f/core/utils.glsl

float saturate(float a) 
{
    return clamp(a, 0.0, 1.0);
}
vec2 saturate(vec2 a) 
{
    return clamp(a, 0.0, 1.0);
}
vec3 saturate(vec3 a) 
{
    return clamp(a, 0.0, 1.0);
}
vec4 saturate(vec4 a) 
{
    return clamp(a, 0.0, 1.0);
}

// ================================================================================================

// Convert RGB to HSV
vec3 rgb2hsv(vec3 c) 
{
    vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
    vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;

    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}


// Convert HSV to RGB
vec3 hsv2rgb(vec3 c) 
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);

    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}


// Apply contrast adjustment
vec3 applyContrast(vec3 color, float contrast) 
{
    // Contrast adjustment
    return clamp((color - 0.5) * (1.0 + contrast) + 0.5, 0.0, 1.0);
}


// Apply saturation adjustment
vec3 applySaturation(vec3 color, float saturation) 
{
    // Calculate luminance
    float lum = dot(color, vec3(0.2126, 0.7152, 0.0722)); 

    // Interpolate between grayscale and original color
    return mix(vec3(lum), color, 1.0 + saturation);
}


// Apply vibrance adjustment
vec3 applyVibrance(vec3 color, float vibrance) 
{
    // Convert to HSV
    vec3 hsv = rgb2hsv(color);
    
    // Calculate the amount of saturation boost based on current saturation
    float satBoost = (1.0 - hsv.y) * vibrance;
    hsv.y = clamp(hsv.y + satBoost, 0.0, 1.0);
    
    // Convert back to RGB
    return hsv2rgb(hsv);
}

// Apply gamma adjustment
// vec3 applyGamma(vec3 color, float gamma) 
// {
//     return pow(max(color, vec3(0.0)), vec3(1.0 / gamma));
// } // pow(color, vec3(1.0 / uPush.Gamma));


// ACES Filmic Tone Mapping
vec3 ACESFilm(vec3 x) 
{
    float a = 2.51;
    float b = 0.03;
    float c = 2.43;
    float d = 0.59;
    float e = 0.14;

    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}


void main() 
{
    // Sample HDR scene
    vec3 hdrColor = texture(sceneTexture, fragUV).rgb;
    
    // Add bloom if enabled
    if (uPush.bloomEnabled != 0) 
    {
        vec3 bloomColor = texture(bloomTexture, fragUV).rgb;
    
        hdrColor += bloomColor * uPush.BloomIntensity;
    }
    
    // Apply exposure
    vec3 color = hdrColor * uPush.Exposure;
    
    // Apply ACES tone mapping
    color = ACESFilm(color);
    
    // Apply contrast
    color = applyContrast(color, uPush.Contrast);
    
    // Apply vibrance
    color = applyVibrance(color, uPush.Vibrance);
    
    // Apply saturation
    color = applySaturation(color, uPush.Saturation);
    
    // Apply gamma correction
    color = pow(color, vec3(1.0 / uPush.Gamma));
    
    outColor = vec4(color, 1.0);
}