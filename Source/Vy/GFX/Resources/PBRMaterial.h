#pragma once

#include <Vy/GFX/Resources/Texture.h>

namespace Vy
{
    class VySampledTexture; // Forward declaration

    enum class AlphaMode
    {
        Opaque,
        Mask,
        Blend
    };

    struct VyPBRMaterial
    {
        // Base PBR properties
        Vec4  Albedo{1.0f, 1.0f, 1.0f, 1.0f};
        float Metallic{0.0f};
        float Roughness{0.5f};
        float AO{1.0f};

        // Alpha Blending
        AlphaMode AlphaMode{AlphaMode::Opaque};
        float     AlphaCutoff{0.5f};

        // Clearcoat layer (for car paint, lacquered surfaces)
        float Clearcoat{0.0f};           // Clearcoat strength [0, 1]
        float ClearcoatRoughness{0.03f}; // Clearcoat layer roughness (typically smooth)

        // Anisotropic reflections (for brushed metals, fabric)
        float Anisotropic{0.0f};         // Anisotropy strength [0, 1]
        float AnisotropicRotation{0.0f}; // Rotation of anisotropic direction [0, 1] (0 = tangent aligned)

        // Transmission (Refraction/Transparency)
        float Transmission{0.0f}; // Transmission factor [0, 1] (0 = opaque, 1 = fully transparent)
        float IOR{1.5f};          // Index of Refraction (default 1.5)

        // Iridescence (Thin film interference)
        float Iridescence{0.0f};            // Iridescence intensity [0, 1]
        float IridescenceIOR{1.3f};         // IOR of the thin film
        float IridescenceThickness{100.0f}; // Thickness of the thin film in nanometers (default 100nm)

        // Emissive
        Vec3  EmissiveColor{0.0f};    // Emissive color (linear)
        float EmissiveStrength{1.0f}; // Emissive strength multiplier

        // Workflow
        bool UseMetallicRoughnessTexture{false};          // If true, metallic/roughness are packed in roughnessMap (B/G channels)
        bool UseOcclusionRoughnessMetallicTexture{false}; // If true, occlusion/roughness/metallic are packed in roughnessMap (R/G/B channels)

        // Texture tiling
        float UVScale{1.0f}; // UV coordinate scale for texture tiling

        // Texture maps (optional - nullptr means use constant values above)
        Shared<VySampledTexture> AlbedoMap;    // Base color texture (sRGB)
        Shared<VySampledTexture> NormalMap;    // Normal map (tangent space)
        Shared<VySampledTexture> MetallicMap;  // Metallic texture (linear)
        Shared<VySampledTexture> RoughnessMap; // Roughness texture (linear)
        Shared<VySampledTexture> AOMap;        // Ambient occlusion texture (linear)
        Shared<VySampledTexture> EmissiveMap;  // Emissive texture (sRGB)

        // Helper methods to check if textures are present
        bool hasAlbedoMap()    const { return AlbedoMap    != nullptr; }
        bool hasNormalMap()    const { return NormalMap    != nullptr; }
        bool hasMetallicMap()  const { return MetallicMap  != nullptr; }
        bool hasRoughnessMap() const { return RoughnessMap != nullptr; }
        bool hasAOMap()        const { return AOMap        != nullptr; }
        bool hasEmissiveMap()  const { return EmissiveMap  != nullptr; }
    };
}