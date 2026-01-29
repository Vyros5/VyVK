#pragma once

#include <VyEngine/GFX/Resources/Texture/Texture.h>

namespace Vy
{
    enum class VyAlphaMode 
    {
        Opaque,
        Mask,
        Blend
    };

    struct VyPBRMaterial
    {
        // Base PBR properties
        Vec4  AlbedoFactor   { 1.0f, 1.0f, 1.0f, 1.0f };
        float MetallicFactor { 0.0f };
        float RoughnessFactor{ 0.5f };
        float AOFactor       { 1.0f };

        // Alpha Blending
        // VyAlphaMode AlphaMode  { VyAlphaMode::Opaque };
        // float      AlphaCutoff{ 0.5f  };
        // bool       DoubleSided{ false };

        // // Clearcoat layer (for car paint, lacquered surfaces)
        // float Clearcoat         { 0.0f  }; // Clearcoat strength [0, 1]
        // float ClearcoatRoughness{ 0.03f }; // Clearcoat layer roughness (typically smooth)

        // // Anisotropic reflections (for brushed metals, fabric)
        // float Anisotropic        { 0.0f }; // Anisotropy strength [0, 1]
        // float AnisotropicRotation{ 0.0f }; // Rotation of anisotropic direction [0, 1] (0 = tangent aligned)

        // // Transmission (Refraction/Transparency)
        // float Transmission       { 0.0f };             // Transmission factor [0, 1] (0 = opaque, 1 = fully transparent)
        // float IOR                { 1.5f };             // Index of Refraction (default 1.5)
        // float Thickness          { 0.0f };             // Volume thickness (0 = thin walled)
        // Vec3  AttenuationColor   { 1.0f, 1.0f, 1.0f }; // Color that white light turns into after traversing attenuationDistance
        // float AttenuationDistance{ 1.0f };             // Distance at which light color becomes attenuationColor

        // // Iridescence (Thin film interference)
        // float Iridescence         { 0.0f   }; // Iridescence intensity [0, 1]
        // float IridescenceIOR      { 1.3f   }; // IOR of the thin film
        // float IridescenceThickness{ 100.0f }; // Thickness of the thin film in nanometers (default 100nm)

        // // Emissive
        // Vec3  EmissiveFactor  { 0.0f }; // Emissive color (linear)
        // float EmissiveStrength{ 1.0f }; // Emissive strength multiplier

        // Workflow
        bool UseMetallicRoughnessTexture         { false }; // If true, metallic/roughness are packed in roughnessMap (B/G channels)
        bool UseOcclusionRoughnessMetallicTexture{ false }; // If true, occlusion/roughness/metallic are packed in roughnessMap (R/G/B channels)
        bool UseSpecularGlossinessWorkflow       { false }; // If true, use KHR_materials_pbrSpecularGlossiness workflow

        // // Specular Glossiness Workflow
        // Vec3  SpecularFactor  { 1.0f }; // Specular color (F0)
        // float GlossinessFactor{ 1.0f }; // Glossiness (1 - roughness)

        // // Texture tiling
        // float UVScale{ 1.0f }; // UV coordinate scale for texture tiling


        // Texture maps (optional - nullptr means use constant values above)
        Shared<VyTexture> AlbedoMap;             // Base color texture (sRGB)
        Shared<VyTexture> NormalMap;             // Normal map (tangent space)
        Shared<VyTexture> MetallicMap;           // Metallic texture (linear)
        Shared<VyTexture> RoughnessMap;          // Roughness texture (linear)
        Shared<VyTexture> MetallicRoughnessMap;  // Metallic-Roughness texture (linear)
        Shared<VyTexture> AOMap;                 // Ambient occlusion texture (linear)
        Shared<VyTexture> EmissiveMap;           // Emissive texture (sRGB)
        // Shared<VyTexture> SpecularGlossinessMap; // Specular (RGB) + Glossiness (A) texture
        // Shared<VyTexture> TransmissionMap;       // Transmission texture (R channel)
        // Shared<VyTexture> ClearcoatMap;          // Clearcoat texture (R channel)
        // Shared<VyTexture> ClearcoatRoughnessMap; // Clearcoat roughness texture (G channel)
        // Shared<VyTexture> ClearcoatNormalMap;    // Clearcoat normal map

        
        // Helper methods to check if textures are present
        bool hasAlbedoMap()             const { return AlbedoMap             != nullptr; }
        bool hasNormalMap()             const { return NormalMap             != nullptr; }
        bool hasMetallicMap()           const { return MetallicMap           != nullptr; }
        bool hasRoughnessMap()          const { return RoughnessMap          != nullptr; }
        bool hasMetallicRoughnessMap()  const { return MetallicRoughnessMap  != nullptr; }
        bool hasAOMap()                 const { return AOMap                 != nullptr; }
        bool hasEmissiveMap()           const { return EmissiveMap           != nullptr; }
        // bool hasSpecularGlossinessMap() const { return SpecularGlossinessMap != nullptr; }
        // bool hasTransmissionMap()       const { return TransmissionMap       != nullptr; }
        // bool hasClearcoatMap()          const { return ClearcoatMap          != nullptr; }
        // bool hasClearcoatRoughnessMap() const { return ClearcoatRoughnessMap != nullptr; }
        // bool hasClearcoatNormalMap()    const { return ClearcoatNormalMap    != nullptr; }
        
        VkDescriptorSet DescriptorSet;
    };
}