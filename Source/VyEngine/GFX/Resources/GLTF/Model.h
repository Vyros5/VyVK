#pragma once

#include <VyEngine/GFX/Resources/Mesh/Vertex.h>
#include <VyEngine/GFX/Resources/Texture/Texture.h>

#include <VyEngine/VK/Buffer/Buffer.h>
#include <VyEngine/VK/Descriptors/Descriptors.h>

// Forward declarations
// namespace tinygltf 
// {
//     class Model;
//     class Skin;
//     class Animation;
//     class Node;
// }

namespace Vy
{
    // https://github.com/LeandroSWR/VulkanTests/blob/e5936776ba9bc1c2e38b3dbaf2dcf48dc23a6110/VulkanTests/src/vt_model.cpp
    // https://github.com/zohebhynus/VulkanGameEngine/blob/main/src/Model.cpp
    // https://github.com/Rubenschuit/vulkan_engine/blob/5cb399f0625151698d0a4f2b8a166396c91f4949/engine/src/game/ve_model.cpp#L24

    enum class VyAlphaMode 
    {
        Opaque,
        Mask,
        Blend
    };
    struct VyPBRMaterial
    {
        TString Name{"unknown"};

        struct PBRParamaters
        {
            // Base PBR properties
            Vec4  AlbedoFactor   { 1.0f, 1.0f, 1.0f, 1.0f };
            float MetallicFactor { 0.0f };
            float RoughnessFactor{ 0.5f };
            float AOFactor       { 1.0f };
            float _pad0{};
            
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
            Vec3  EmissiveFactor  { 0.0f }; // Emissive color (linear)
            float EmissiveStrength{ 1.0f }; // Emissive strength multiplier
            
        } Parameters;

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

        bool HasAlbedoMap { false };
        bool HasNormalMap{ false };
        bool HasMetallicRoughnessMap { false };
        bool HasAOMap { false };
        bool HasEmissiveMap { false };
        
        // Helper methods to check if textures are present
        bool hasAlbedoMap()             const { return HasAlbedoMap; } //{ return AlbedoMap             != nullptr; }
        bool hasNormalMap()             const { return HasNormalMap; } //{ return NormalMap             != nullptr; }
        bool hasMetallicMap()           const { return MetallicMap           != nullptr; }
        bool hasRoughnessMap()          const { return RoughnessMap          != nullptr; }
        bool hasMetallicRoughnessMap()  const { return HasMetallicRoughnessMap; } //{ return MetallicRoughnessMap  != nullptr; }
        bool hasAOMap()                 const { return HasAOMap; } //{ return AOMap                 != nullptr; }
        bool hasEmissiveMap()           const { return HasEmissiveMap; } //{ return EmissiveMap           != nullptr; }
        // bool hasSpecularGlossinessMap() const { return SpecularGlossinessMap != nullptr; }
        // bool hasTransmissionMap()       const { return TransmissionMap       != nullptr; }
        // bool hasClearcoatMap()          const { return ClearcoatMap          != nullptr; }
        // bool hasClearcoatRoughnessMap() const { return ClearcoatRoughnessMap != nullptr; }
        // bool hasClearcoatNormalMap()    const { return ClearcoatNormalMap    != nullptr; }
        
        Shared<VyBuffer> MaterialUBO;
        VkDescriptorSet DescriptorSet;
    };

    class VyGLTFModel
    {
    public:


        enum VyMaterialFlags
        {
            VY_HAS_ALBEDO_MAP             = 1 << 0,
            VY_HAS_METALLIC_MAP           = 1 << 1,
            VY_HAS_ROUGHNESS_MAP          = 1 << 2,
            VY_HAS_METALLIC_ROUGHNESS_MAP = 1 << 3,
            VY_HAS_NORMAL_MAP             = 1 << 4,
            VY_HAS_AO_MAP                 = 1 << 5,
            VY_HAS_EMISSIVE_MAP           = 1 << 6,
            VY_HAS_SPECULAR_GLOSS_MAP     = 1 << 7,
            VY_HAS_TRANSMISSION_MAP       = 1 << 8,
            VY_HAS_CLEARCOAT_MAP          = 1 << 9,

            VY_USE_MR_PACKED              = 1 << 10,
            VY_USE_ARM_PACKED             = 1 << 11,
        };

        struct ModelLoadInfo
        {
            TString Name;

            U32     SceneCount = 0;
            U32     NodeCount  = 0;
            U32     MeshCount  = 0;

            U32     VertexCount  = 0;
            U32     IndexCount   = 0;
            U32     TextureCount = 0;
        };

        struct VyPrimitive
        {
            U32 FirstIndex;
            U32 IndexCount;

            U32 FirstVertex;
            U32 VertexCount;

            VyPBRMaterial Material;
        };

        VyGLTFModel(const TString& filepath, VyDescriptorSetLayout& materialSetLayout, VyDescriptorPool& descriptorPool);
        
        ~VyGLTFModel();

        static Shared<VyGLTFModel> createFromFile(const TString& filepath, VyDescriptorSetLayout& materialSetLayout, VyDescriptorPool& descriptorPool)
        {
            return MakeShared<VyGLTFModel>( filepath, materialSetLayout, descriptorPool );
        }

        void bind(VkCommandBuffer cmdBuffer);
        void draw(VkCommandBuffer cmdBuffer, VkPipelineLayout pipelineLayout, int setCount, bool bRenderMaterial);

        void createDescriptorSet(VyPBRMaterial& material, VyDescriptorSetLayout& materialSetLayout, VyDescriptorPool& descriptorPool);

        const TVector<VyPrimitive>& primitives() const { return m_Primitives; }

    private:
        void createVertexBuffer(const TVector<VyVertex>& vertices);
        void createIndexBuffer (const TVector<U32>&      indices );

        Unique<VyBuffer>  m_VertexBuffer;
        TVector<VyVertex> m_Vertices;

        Unique<VyBuffer> m_IndexBuffer;
        TVector<U32>     m_Indices;

        bool m_HasIndexBuffer = false;

        TVector<VyPrimitive>       m_Primitives;
        TVector<Shared<VyTexture>> m_Textures;

        Shared<VyTexture> m_DefaultWhite;
        Shared<VyTexture> m_DefaultNormal;
    };

    // class GLTFImporter
    // {
    // public:
    //     bool load(VyGLTFModel::Builder& builder, const TString& filepath, bool bFlipX, bool bFlipY, bool bFlipZ) ;
    // };
}