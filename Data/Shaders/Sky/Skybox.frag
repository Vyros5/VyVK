#version 450

// ================================================================================================
// Input

layout(location = 0) in  vec3  fragUVW;

// ================================================================================================
// Output

layout(location = 0) out vec4  outColor;

// ================================================================================================

layout (set = 1, binding = 0) uniform samplerCube uSkyboxSampler;

void main()
{
    // Sample the cubemap using the interpolated texture coordinate (view direction).
    outColor = texture( uSkyboxSampler, fragUVW );
}

// void main()
// {
//     // Flip Y axis to correct vertical orientation
//     vec3 texCoord = vec3(fragTexCoord.x, -fragTexCoord.y, fragTexCoord.z);
//     vec3 color    = texture(uSkyboxSampler, texCoord).rgb;

//     outColor = vec4(color, 1.0);
// }