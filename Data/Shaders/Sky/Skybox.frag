#version 450

// ================================================================================================
// INPUT

layout (location = 0) in  vec3 fragUVW;

// ================================================================================================
// OUTPUT

layout (location = 0) out vec4 outColor;

// ================================================================================================
// DESCRIPTOR SET 1 : SKYBOX

layout (set = 1, binding = 0) uniform samplerCube uSkyboxSampler;

// ================================================================================================
// MAIN

void main()
{
    // Sample the cubemap using the interpolated texture coordinate (view direction).
    outColor = texture( uSkyboxSampler, fragUVW );
}

// ================================================================================================
