
// Light
#define MAX_LIGHTS 128

// Material
#define GLSL_HAS_DIFFUSE_MAP            (0x1 << 0x0)
#define GLSL_HAS_NORMAL_MAP             (0x1 << 0x1)
#define GLSL_HAS_ROUGHNESS_MAP          (0x1 << 0x2)
#define GLSL_HAS_METALLIC_MAP           (0x1 << 0x3)
#define GLSL_HAS_ROUGHNESS_METALLIC_MAP (0x1 << 0x4)
#define GLSL_HAS_EMISSIVE_COLOR         (0x1 << 0x5)
#define GLSL_HAS_EMISSIVE_MAP           (0x1 << 0x6)

// Skeleton
#define MAX_JOINTS          200
#define MAX_JOINT_INFLUENCE 4