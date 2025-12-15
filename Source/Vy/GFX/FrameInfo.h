#pragma once

#include <Vy/Scene/Scene.h>
#include <Vy/Scene/ECS/Components/LightComponent.h>
#include <Vy/Scene/Camera.h>

#include <Vy/GFX/Backend/VK/VKCore.h>

namespace Vy 
{

#define MAX_LIGHTS 10

    struct PointLightUBO
    {
        Vec4 Position{}; // w component unused
        Vec4 Color   {}; // w is intensity
    };

    struct SpotLightUBO
    {
        Vec4  Position   {}; // w component unused
        Vec4  Direction  {}; // w component unused
        Vec4  Color      {}; // w is intensity
        // float InnerCutoff{}; // cos of inner angle
        float OuterCutoff{}; // cos of outer angle
        float ConstantAtten{};  // Constant attenuation
        float LinearAtten{};    // Linear attenuation
        float QuadraticAtten{}; // Quadratic attenuation
    };

    struct DirectionalLightUBO
    {
        Vec4 Direction{}; // w component unused
        Vec4 Color    {}; // w is intensity
    };

    struct GlobalUBO 
    {
        Mat4                Projection  { 1.0f };
        Mat4                View        { 1.0f };
        Mat4                InverseView { 1.0f };

        Vec4                AmbientLightColor{ 1.0f, 1.0f, 1.0f, 0.02f }; // w is intensity

        PointLightUBO       PointLights       [ MAX_LIGHTS ];
        DirectionalLightUBO DirectionalLights [ MAX_LIGHTS ];
        SpotLightUBO        SpotLights        [ MAX_LIGHTS ];
        Mat4                LightSpaceMatrices[ MAX_LIGHTS ]; // Light space transformation matrices for shadows
        Vec4                PointLightShadowData[4];          // xyz = position, w = far plane (for cube shadows)
        int                 NumPointLights      { 0 };
        int                 NumDirectionalLights{ 0 };
        int                 NumSpotLights       { 0 };

        int                 ShadowLightCount    { 0 }; // Number of 2D shadow maps (directional + spot)
        int                 CubeShadowLightCount{ 0 }; // Number of cube shadow maps (point lights)
        int                 _pad1{};
        int                 _pad2{};
        int                 _pad3{};
        Vec4                FrustumPlanes[6]; // Frustum planes for culling (Left, Right, Bottom, Top, Near, Far)
    };

    
    struct GenericPushConstantData
    {
        Mat4 ModelMatrix { 1.0f };
        Mat4 NormalMatrix{ 1.0f }; // 4x4 because of alignment
    };

    struct ShadowPushConstantsData 
    {
        Mat4 LightSpaceModelMatrix{ 1.0f };
    };

    struct PostProcessPushConstantData 
    {
        float BloomIntensity; // 
        float Exposure;       // 
        float Gamma;          // 
        int   BloomEnabled;   // 
        float Contrast;       // 
        float Saturation;     // 
        float Vibrance;       // 
    };


    struct MaterialPushConstantData 
    {
        alignas(16) Mat4 ModelMatrix     { 1.0f };
        alignas(16) Mat4 NormalMatrix    { 1.0f }; // 4x4 because of alignment
        
        alignas(16) Vec3 Albedo          { 1.0f, 1.0f, 1.0f };
        float            Metallic        { 0.0f };
        float            Roughness       { 0.5f };
        float            AO              { 1.0f };
        Vec2             TextureOffset   { 0.0f, 0.0f };
        Vec2             TextureScale    { 1.0f, 1.0f };

        alignas(16) Vec3 EmissionColor   { 0.0f, 0.0f, 0.0f }; 
        float            EmissionStrength{ 0.0f };
    };


    struct VyFrameInfo 
    {
        int              FrameIndex;
        float            FrameTime;
        VkCommandBuffer  CommandBuffer;
        VkDescriptorSet  GlobalDescriptorSet;
        VkDescriptorSet  GlobalTextureSet; // Bindless
        // VkDescriptorSet  LightDescriptorSet;
        Shared<VyScene>& Scene;
        VyCamera&        Camera;
    };










    /**
     * CameraData - Per-View camera uniforms
     *
     * Update frequency: Per frame, per camera/view
     * Shader binding: set = 0, binding = 1, std140
     *
     * Contains:
     * - View and Projection matrices (and inverses)
     * - Precomputed ViewProjection matrix
     * - Camera Position and direction
     * - Near/far planes, FOV, aspect ratio
     * - Screen resolution
     *
     * Design notes:
     * - std140 layout (matrices are column-major, 16-byte aligned)
     * - Provides inverse matrices for screenspace reconstruction
     * - Explicit Position/direction for lighting calculations
     * - All commonly needed camera data in one place
     */
    // struct CameraData 
    // {
    //     // --- Matrices (64 bytes each) ---
    //     Mat4 View;                // world -> view space
    //     Mat4 Projection;          // view -> clip space (reversed-Z friendly)
    //     Mat4 ViewProjection;      // precomputed: projection * view
    //     Mat4 InvView;             // view -> world (for world-space reconstruction)
    //     Mat4 InvProjection;       // clip -> view (for screenspace -> viewspace)

    //     // --- Camera Vectors (32 bytes) ---
    //     Vec4 Position;            // xyz = world position,                 w = unused
    //     Vec4 Direction;           // xyz = forward direction (normalized), w = unused

    //     // --- Camera Parameters (16 bytes) ---
    //     Vec2 NearFar;             // x = near plane, y = far plane
    //     Vec2 ScreenSize;          // x = width, y = height (in pixels)

    //     // --- Additional Parameters (16 bytes) ---
    //     float FOV;                 // vertical field of view (radians)
    //     float AspectRatio;         // width / height

    //     U32   _pad0;
    //     U32   _pad1;

    //     // Default constructor
    //     constexpr CameraData() noexcept
    //         : View(1.0f)
    //         , Projection(1.0f)
    //         , ViewProjection(1.0f)
    //         , InvView(1.0f)
    //         , InvProjection(1.0f)
    //         , Position(0.0f, 0.0f, 0.0f, 0.0f)
    //         , Direction(0.0f, 0.0f, -1.0f, 0.0f)
    //         , NearFar(0.1f, 1000.0f)
    //         , ScreenSize(1920.0f, 1080.0f)
    //         , FOV(glm::radians(60.0f))
    //         , AspectRatio(1920.0f / 1080.0f)
    //         , _pad0(0)
    //         , _pad1(0)
    //     {
    //     }

    //     /**
    //      * Update camera from position, target, and projection parameters
    //      *
    //      * @param pos Camera position in world space
    //      * @param target Point to look at in world space
    //      * @param up Up vector (usually (0, 1, 0))
    //      * @param fovRadians Vertical field of view in radians
    //      * @param aspect Aspect ratio (width / height)
    //      * @param nearPlane Near clip plane
    //      * @param farPlane Far clip plane
    //      * @param screenWidth Screen width in pixels
    //      * @param screenHeight Screen height in pixels
    //      */
    //     void setLookAt(
    //         const Vec3& pos,
    //         const Vec3& target,
    //         const Vec3& up,
    //         float       fovRadians,
    //         float       aspect,
    //         float       nearPlane,
    //         float       farPlane,
    //         float       screenWidth,
    //         float       screenHeight
    //     ) {
    //         // Store position and compute direction
    //         Position  = Vec4(pos, 0.0f);
    //         Direction = Vec4(glm::normalize(target - pos), 0.0f);

    //         // Build view matrix
    //         View    = glm::lookAt(pos, target, up);
    //         InvView = glm::inverse(View);

    //         // Build projection matrix (Vulkan NDC: x,y in [-1,1], z in [0,1])
    //         Projection = glm::perspective(fovRadians, aspect, nearPlane, farPlane);

    //         // GLM was designed for OpenGL where clip space Z is [-1, 1] and Y is inverted
    //         // For Vulkan, we need to flip Y and use [0, 1] for Z (handled by GLM_FORCE_DEPTH_ZERO_TO_ONE)
    //         // But we still need to flip Y:
    //         Projection[1][1] *= -1.0f;

    //         InvProjection = glm::inverse(Projection);

    //         // Precompute combined matrix
    //         ViewProjection = Projection * View;

    //         // Store parameters
    //         NearFar     = Vec2(nearPlane, farPlane);
    //         ScreenSize  = Vec2(screenWidth, screenHeight);
    //         FOV         = fovRadians;
    //         AspectRatio = aspect;
    //     }

    //     /**
    //      * Update camera from view and Projection matrices directly
    //      * (Useful when integrating with external camera systems)
    //      */
    //     void setMatrices(
    //         const Mat4& viewMatrix,
    //         const Mat4& projectionMatrix,
    //         float       screenWidth,
    //         float       screenHeight
    //     ) {
    //         View           = viewMatrix;
    //         Projection     = projectionMatrix;
    //         ViewProjection = Projection * View;
    //         InvView        = glm::inverse(View);
    //         InvProjection  = glm::inverse(Projection);

    //         // Extract Position from inverse View matrix
    //         Position = Vec4(InvView[3].x, InvView[3].y, InvView[3].z, 0.0f);

    //         // Extract forward Direction from View matrix (negative Z axis)
    //         Direction = Vec4(-View[0][2], -View[1][2], -View[2][2], 0.0f);

    //         ScreenSize = Vec2(screenWidth, screenHeight);
    //     }

    //     /**
    //      * Update only Projection matrix (useful for resize)
    //      */
    //     void updateProjection(
    //         float fovRadians,
    //         float aspect,
    //         float nearPlane,
    //         float farPlane,
    //         float screenWidth,
    //         float screenHeight
    //     ) {
    //         Projection        = glm::perspective(fovRadians, aspect, nearPlane, farPlane);
    //         Projection[1][1] *= -1.0f; // Flip Y for Vulkan
    //         InvProjection     = glm::inverse(Projection);
    //         ViewProjection    = Projection * View;

    //         NearFar     = Vec2(nearPlane, farPlane);
    //         ScreenSize  = Vec2(screenWidth, screenHeight);
    //         FOV         = fovRadians;
    //         AspectRatio = aspect;
    //     }

    //     /**
    //      * Get the right vector (X axis in camera space)
    //      */
    //     constexpr Vec3 rightVector() const noexcept 
    //     {
    //         return Vec3(InvView[0].x, InvView[0].y, InvView[0].z);
    //     }

    //     /**
    //      * Get the up vector (Y axis in camera space)
    //      */
    //     constexpr Vec3 upVector() const noexcept 
    //     {
    //         return Vec3(InvView[1].x, InvView[1].y, InvView[1].z);
    //     }

    //     /**
    //      * Get the forward vector (negative Z axis in camera space)
    //      */
    //     constexpr Vec3 forwardVector() const noexcept 
    //     {
    //         return Vec3(Direction.x, Direction.y, Direction.z);
    //     }
    // };

    // // Size validation (std140 alignment)
    // static_assert(sizeof(CameraData) % 16 == 0, "CameraData must be aligned to 16 bytes for std140");
}