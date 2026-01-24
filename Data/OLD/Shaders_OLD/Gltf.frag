#version 450
#extension GL_KHR_vulkan_glsl : enable

// Material textures (set = 2)
layout(set = 2, binding = 0) uniform sampler2D materialSampler;
layout(set = 2, binding = 1) uniform sampler2D normalSampler;

// Inputs
layout(location = 0) in vec3 inNormal;
layout(location = 1) in vec4 inColor;
layout(location = 2) in vec2 inUV;
layout(location = 3) in vec3 inViewVec;
layout(location = 4) in vec3 inLightVec;
layout(location = 5) in vec4 inTangent;
layout(location = 6) in vec4 inLightColor;

// Final output
layout(location = 0) out vec4 outFragColor;


// Constants
layout(constant_id = 0) const bool  ALPHA_MASK = false;
layout(constant_id = 1) const float ALPHA_MASK_CUTOFF = 0.0;


void main()
{
    vec4 texColor = texture(materialSampler, inUV) * inColor;

    if (ALPHA_MASK && texColor.a < ALPHA_MASK_CUTOFF) 
    {
        discard;
    }

    vec3 N   = normalize(inNormal);
    vec3 T   = normalize(inTangent.xyz);
    vec3 B   = cross(N, T) * inTangent.w;
    mat3 TBN = mat3(T, B, N);
    vec3 normalMap = texture(normalSampler, inUV).xyz * 2.0 - vec3(1.0);
    N = normalize(TBN * normalMap);

    // Light and view vectors are passed from vertex shader (in world space)
    vec3 L = normalize(inLightVec);
    vec3 V = normalize(inViewVec);
    vec3 R = reflect(-L, N);

    // unpack light color & intensity
    vec3 lightCol = inLightColor.rgb * inLightColor.a;

    const float ambientFactor = 0.1;
    vec3  ambient  = ambientFactor * lightCol;
    vec3  diffuse  = max(dot(N, L), 0.0) * lightCol;
    float specAmt  = pow(max(dot(R, V), 0.0), 32.0) * inLightColor.a;
    vec3  specular = specAmt * lightCol;

    // combine with texture
    vec3 result = texColor.rgb * (ambient + diffuse) + specular;
    
    outFragColor = vec4(result, texColor.a);
}