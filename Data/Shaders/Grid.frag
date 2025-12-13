#version 450

// ================================================================================================

struct PointLight 
{
    vec4 Position; // ignore w
    vec4 Color;    // w is intensity
};

// ================================================================================================
// Uniforms

layout(set = 0, binding = 0) uniform GlobalUBO 
{
    mat4       Projection;
    mat4       View;
    mat4       InverseView;
    
    vec4       AmbientLightColor; // w is intensity
    PointLight PointLights[10];
    int        NumPointLights;

} uUbo;

// ================================================================================================

// Input
layout(location = 1) in  vec3 nearPoint;
layout(location = 2) in  vec3 farPoint;
layout(location = 3) in  mat4 fragView;
layout(location = 7) in  mat4 fragProj;

// Output
layout(location = 0) out vec4 outColor;

// ================================================================================================

const float AXIS_THRESHOLD =   0.1f; // Threshold for axis line coloring
const float GRID_SCALE     =  10.0f; // Grid scale factor
const float FADE_LIMIT     =   0.7f; // Fading start threshold for linear depth

const float NEAR_PLANE     =  0.01f;
const float FAR_FLANE      =  10.0f;

// ================================================================================================




// Grid generation.
vec4 grid(vec3 fragPos3D, float scale) 
{
    // Scale variable to set the distance between the lines.
    vec2  coord = fragPos3D.xz * scale;

    vec2  derivative = fwidth(coord);
    vec2  grid       = abs(fract(coord - 0.5f) - 0.5f) / derivative;
    float line       = min(grid.x, grid.y);
    float minimumZ   = min(derivative.y, 1);
    float minimumX   = min(derivative.x, 1);

    vec4  color = vec4(0.30f, 0.30f, 0.30f, 1.0f - min(line, 1.0f));

    // Z-Axis
    if(fragPos3D.x > -AXIS_THRESHOLD * minimumX && fragPos3D.x < AXIS_THRESHOLD * minimumX)
    {
        // Blue
        color.z = 1.0f; 
    }
    
    // X-Axis
    if(fragPos3D.z > -AXIS_THRESHOLD * minimumZ && fragPos3D.z < AXIS_THRESHOLD * minimumZ)
    {
        // Red
        color.x = 1.0f; 
    }

    // Reset the color's RGB if its alpha means it is not visible (allows to add grids without whitening the grid color)
    return color.a >0.01f
        ? color
        : vec4(0.0f, 0.0f, 0.0f, 0.0f);
}

// Compute depth in clip space.
float computeDepth(vec3 pos) 
{
    vec4 clipSpacePos = fragProj * fragView * vec4(pos.xyz, 1.0f);

    return (clipSpacePos.z / clipSpacePos.w);
}


// Compute linear depth.
float computeLinearDepth(vec3 pos) 
{
    vec4  clipSpacePos = fragProj * fragView * vec4(pos.xyz, 1.0f);

    // Put back between -1 and 1
    float clipSpaceDepth = (clipSpacePos.z / clipSpacePos.w) * 2.0f - 1.0f;
    
    // Get linear value between 0.01 and 100
    float linearDepth = (2.0f * NEAR_PLANE * FAR_FLANE) / (FAR_FLANE + NEAR_PLANE - clipSpaceDepth * (FAR_FLANE - NEAR_PLANE));
    
    // Normalize
    return linearDepth / FAR_FLANE;
}

// ================================================================================================

void main() 
{
    float t = -nearPoint.y / (farPoint.y - nearPoint.y);

    vec3 fragPos3D = nearPoint + t * (farPoint - nearPoint);

    gl_FragDepth = computeDepth(fragPos3D);

    float linearDepth = computeLinearDepth(fragPos3D);
    float fading      = max(0.0f, (FADE_LIMIT - linearDepth));

    // Adding multiple resolutions for the grid
    outColor    = (grid(fragPos3D, GRID_SCALE) + grid(fragPos3D, 1.0f)) * float(t > 0.0f);
    outColor.a *= fading;
}