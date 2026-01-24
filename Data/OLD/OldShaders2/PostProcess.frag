#version 450

layout(location = 0) in vec2 inUV;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform sampler2D uSceneColor;
layout(set = 0, binding = 1) uniform sampler2D uDepthMap;

layout(push_constant) uniform PushConstants
{
    float Exposure;
    float Contrast;
    float Saturation;
    float Vignette;
    float BloomIntensity;
    float BloomThreshold;
    int   EnableFXAA;
    int   EnableBloom;
    float FxaaSpanMax;
    float FxaaReduceMul;
    float FxaaReduceMin;
    int   EnableSSAO;
    float SsaoRadius;
    float SsaoBias;
    int   ToneMappingMode; // 0: None, 1: ACES
    float _pad0;
    vec4  SunScreenPos; // xy = screen pos [0,1], z = isVisible (1.0/0.0), w = padding
    float GodRayDensity;
    float GodRayWeight;
    float GodRayDecay;
    float GodRayExposure;
    mat4  InverseProjection;
    mat4  Projection;

} uPush;

// FXAA implementation
// #define FXAA_SPAN_MAX   8.0
// #define FXAA_REDUCE_MUL (1.0 / 8.0)
// #define FXAA_REDUCE_MIN (1.0 / 128.0)

vec3 applyFXAA(vec2 texCoords)
{
    vec2 texSize   = textureSize(uSceneColor, 0);
    vec2 inverseVP = 1.0 / texSize;

    vec3 rgbNW = texture(uSceneColor, texCoords + (vec2(-1.0, -1.0) * inverseVP)).xyz;
    vec3 rgbNE = texture(uSceneColor, texCoords + (vec2( 1.0, -1.0) * inverseVP)).xyz;
    vec3 rgbSW = texture(uSceneColor, texCoords + (vec2(-1.0,  1.0) * inverseVP)).xyz;
    vec3 rgbSE = texture(uSceneColor, texCoords + (vec2( 1.0,  1.0) * inverseVP)).xyz;
    vec3 rgbM  = texture(uSceneColor, texCoords).xyz;

    vec3  luma   = vec3(0.299, 0.587, 0.114);
    float lumaNW = dot(rgbNW, luma);
    float lumaNE = dot(rgbNE, luma);
    float lumaSW = dot(rgbSW, luma);
    float lumaSE = dot(rgbSE, luma);
    float lumaM  = dot(rgbM, luma);

    float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
    float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));

    vec2 dir;
    dir.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));
    dir.y =  ((lumaNW + lumaSW) - (lumaNE + lumaSE));

    float dirReduce = max((lumaNW + lumaNE + lumaSW + lumaSE) * (0.25 * uPush.FxaaReduceMul), uPush.FxaaReduceMin);

    float rcpDirMin = 1.0 / (min(abs(dir.x), abs(dir.y)) + dirReduce);

    dir = min(vec2(uPush.FxaaSpanMax, uPush.FxaaSpanMax), max(vec2(-uPush.FxaaSpanMax, -uPush.FxaaSpanMax), dir * rcpDirMin)) * inverseVP;

    vec3 rgbA = (1.0 / 2.0) * (texture(uSceneColor, texCoords.xy + dir * (1.0 / 3.0 - 0.5)).xyz + texture(uSceneColor, texCoords.xy + dir * (2.0 / 3.0 - 0.5)).xyz);

    vec3 rgbB = rgbA * (1.0 / 2.0) +
                (1.0 / 4.0) * (texture(uSceneColor, texCoords.xy + dir * (0.0 / 3.0 - 0.5)).xyz + texture(uSceneColor, texCoords.xy + dir * (3.0 / 3.0 - 0.5)).xyz);

    float lumaB = dot(rgbB, luma);

    if ((lumaB < lumaMin) || (lumaB > lumaMax)) return rgbA;
    return rgbB;
}

vec3 applyBloom(vec3 color, vec2 uv)
{
    vec3 bloomColor = vec3(0.0);

    // Sample lower mips
    // Weights can be adjusted
    bloomColor += textureLod(uSceneColor, uv, 1.0).rgb * 1.0;
    bloomColor += textureLod(uSceneColor, uv, 2.0).rgb * 0.8;
    bloomColor += textureLod(uSceneColor, uv, 3.0).rgb * 0.6;
    bloomColor += textureLod(uSceneColor, uv, 4.0).rgb * 0.4;
    bloomColor += textureLod(uSceneColor, uv, 5.0).rgb * 0.2;

    bloomColor = max(bloomColor - vec3(uPush.BloomThreshold), vec3(0.0));

    return color + bloomColor * uPush.BloomIntensity;
}

// Helper to reconstruct view position
vec3 getViewPos(vec2 uv)
{
    float depth = texture(uDepthMap, uv).r;
    // Vulkan depth is [0, 1].
    vec4 clipPos = vec4(uv * 2.0 - 1.0, depth, 1.0);
    vec4 viewPos = uPush.InverseProjection * clipPos;
    return viewPos.xyz / viewPos.w;
}

// Interleaved Gradient Noise
float interleavedGradientNoise(vec2 position_screen)
{
    vec3 magic = vec3(0.06711056, 0.00583715, 52.9829189);

    return fract(magic.z * fract(dot(position_screen, magic.xy)));
}

const vec3 kernel[16] = vec3[](
    vec3( 0.0248,  0.0000,  0.0969),
    vec3(-0.0323,  0.0296,  0.0938),
    vec3( 0.0054, -0.0610,  0.0962),
    vec3( 0.0500,  0.0652,  0.1028),
    vec3(-0.1070, -0.0189,  0.1123),
    vec3( 0.1196, -0.0761,  0.1233),
    vec3(-0.0473,  0.1761,  0.1345),
    vec3(-0.1063, -0.2047,  0.1446),
    vec3( 0.2697,  0.0985,  0.1523),
    vec3(-0.3250,  0.1341,  0.1563),
    vec3( 0.1797, -0.3841,  0.1552),
    vec3( 0.1509,  0.4811,  0.1478),
    vec3(-0.5118, -0.2966,  0.1326),
    vec3( 0.6696, -0.1472,  0.1085),
    vec3(-0.4518,  0.6427,  0.0740),
    vec3(-0.1144, -0.8832,  0.0278)
);

float computeSSAO(vec2 uv)
{
    vec3 viewPos = getViewPos(uv);

    float depth = texture(uDepthMap, uv).r;
    if (depth >= 0.9999) return 1.0;

    vec3 dX     = dFdx(viewPos);
    vec3 dY     = dFdy(viewPos);
    vec3 normal = normalize(cross(dX, dY));

    // Random vector based on screen position
    vec2  screenPos = gl_FragCoord.xy;
    float noiseVal  = interleavedGradientNoise(screenPos);

    // Create random rotation vector
    vec3 randomVec = normalize(vec3(noiseVal * 2.0 - 1.0, (1.0 - noiseVal) * 2.0 - 1.0, 0.0));

    // Create TBN matrix
    vec3 tangent   = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN       = mat3(tangent, bitangent, normal);

    float occlusion = 0.0;
    int   samples   = 16;
    float radius    = uPush.SsaoRadius;
    float bias      = uPush.SsaoBias;

    for (int i = 0; i < samples; ++i)
    {
        vec3 samplePos = viewPos + (TBN * kernel[i]) * radius;

        vec4 offset = uPush.Projection * vec4(samplePos, 1.0);
        offset.xyz /= offset.w;
        offset.xy   = offset.xy * 0.5 + 0.5;

        float sampleDepth = getViewPos(offset.xy).z;

        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(viewPos.z - sampleDepth));
        
        occlusion += (sampleDepth >= samplePos.z + bias ? 1.0 : 0.0) * rangeCheck;
    }

    occlusion = 1.0 - (occlusion / float(samples));

    return occlusion;
}

void main()
{
    vec3 color;

    if (uPush.EnableFXAA == 1)
    {
        color = applyFXAA(inUV);
    }
    else
    {
        color = texture(uSceneColor, inUV).rgb;
    }

    // SSAO
    if (uPush.EnableSSAO == 1)
    {
        float ssao = computeSSAO(inUV);
        color *= ssao;
    }

    // Bloom
    if (uPush.EnableBloom == 1 && uPush.BloomIntensity > 0.0)
    {
        color = applyBloom(color, inUV);
    }

    // God Rays (Volumetric Light Scattering)
    if (uPush.SunScreenPos.z > 0.5)
    {
        int  NUM_SAMPLES   = 100; // Increased samples

        vec2 deltaTexCoord = (inUV - uPush.SunScreenPos.xy);
        deltaTexCoord *= 1.0 / float(NUM_SAMPLES) * uPush.GodRayDensity;

        float illuminationDecay = 1.0;
        vec3  godRayColor       = vec3(0.0);
        vec2  coord             = inUV;

        // Dithering: Randomize start position to trade banding for noise
        float noise = interleavedGradientNoise(gl_FragCoord.xy);
        coord -= deltaTexCoord * noise;

        for (int i = 0; i < NUM_SAMPLES; i++)
        {
            coord -= deltaTexCoord;

            // Boundary check
            if (coord.x < 0.0 || coord.x > 1.0 || coord.y < 0.0 || coord.y > 1.0) break;

            float d = texture(uDepthMap, coord).r;

            // Only accumulate if it's the sky (depth ~ 1.0)
            if (d >= 0.99)
            {
                vec3 sampleColor = texture(uSceneColor, coord).rgb;
                sampleColor *= illuminationDecay * uPush.GodRayWeight;
                godRayColor += sampleColor;
            }

            illuminationDecay *= uPush.GodRayDecay;
        }

        color += godRayColor * uPush.GodRayExposure;
    }

    // Exposure
    color *= uPush.Exposure;

    // Contrast
    color = (color - 0.5) * uPush.Contrast + 0.5;
    color = max(color, 0.0); // Prevent negative colors

    // Saturation
    float luminance = dot(color, vec3(0.2126, 0.7152, 0.0722));
    color           = mix(vec3(luminance), color, uPush.Saturation);

    vec3 mapped = color;

    if (uPush.ToneMappingMode == 1) // ACES Filmic
    {
        float a = 2.51f;
        float b = 0.03f;
        float c = 2.43f;
        float d = 0.59f;
        float e = 0.14f;
        
        mapped  = clamp((color * (a * color + b)) / (color * (c * color + d) + e), 0.0, 1.0);
    }

    // Gamma correction
    const float gamma = 2.2;
    mapped            = pow(mapped, vec3(1.0 / gamma));

    // Vignette
    float dist = length(inUV - 0.5);
    float vig  = 1.0 - smoothstep(0.4, 1.5, dist * uPush.Vignette);
    mapped *= vig;

    outColor = vec4(mapped, 1.0);
}