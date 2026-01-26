#pragma once

#include <VyEngine/VK/Core/VKCore.h>
#include <VyEngine/VK/Buffer/Buffer.h>
#include <VyEngine/GFX/Resources/Texture/Texture.h>
#include <VyEngine/VK/Descriptors/Descriptors.h>
namespace Vy
{
    /**
     * VyVertex - Standard vertex format for rendering.
     *
     * Layout:
     *   Location 0: position (vec3)
     *   Location 1: color (vec3)
     *   Location 2: normal (vec3)
     *   Location 3: uv (vec2)
     *   Location 4: tangent (vec4, xyz=tangent, w=bitangent sign)
     *
     * Total size: 64 bytes
     */
    struct VyVertex 
    {
        Vec3 Position { 0.0f };
        Vec3 Color    { 1.0f };
        Vec3 Normal   {      };
        Vec2 UV       {      };
        Vec4 Tangent  {      };

        static TVector<VkVertexInputBindingDescription>   bindingDescriptions();
        static TVector<VkVertexInputAttributeDescription> attributeDescriptions();
        
        bool operator==(const VyVertex& other) const 
        {
            return Position  == other.Position 
                && Color     == other.Color 
                && Normal    == other.Normal
                && UV        == other.UV 
                && Tangent   == other.Tangent
            ;
        }
    };


    /**
     * VyAABB - Axis-aligned bounding box
     */
    struct VyAABB 
    {
        Vec3 Min { FLT_MAX };
        Vec3 Max {-FLT_MAX };

        VyAABB() = default;

        VyAABB(const Vec3& min_, const Vec3& max_)
            : Min(min_), Max(max_)
        {
        }

        void expand(const Vec3& point) 
        {
            Min = glm::min(Min, point);
            Max = glm::max(Max, point);
        }

        void expand(const VyAABB& other) 
        {
            Min = glm::min(Min, other.Min);
            Max = glm::max(Max, other.Max);
        }

        Vec3 center() const 
        {
            return (Min + Max) * 0.5f;
        }

        Vec3 extents() const 
        {
            return (Max - Min) * 0.5f;
        }

        bool isValid() const 
        {
            return Min.x <= Max.x 
                && Min.y <= Max.y 
                && Min.z <= Max.z
            ;
        }
    };

// /**
//  * AlphaMode - How to interpret alpha channel
//  */
// enum class AlphaMode : uint32_t {
//     Opaque = 0,      // Alpha is ignored, material is fully opaque
//     Mask = 1,        // Alpha is used for binary transparency (alpha test)
//     Blend = 2        // Alpha is used for blending
// };

// /**
//  * MaterialParams - PBR material parameters (CPU-side)
//  *
//  * These are passed to shaders via push constants or UBO
//  */
// struct MaterialParams {
//     // std140 layout (for push constants):
//     // - vec4 = 16 bytes aligned to 16
//     // - vec3 = 12 bytes but ALIGNED TO 16! (this is the key!)
//     // - float/uint = 4 bytes aligned to 4

//     glm::vec4 baseColorFactor{1.0f, 1.0f, 1.0f, 1.0f};  // 16 bytes, offset 0
//     glm::vec3 emissiveFactor{0.0f, 0.0f, 0.0f};         // 12 bytes, offset 16
//     float     _emissivePad = 0.0f;                       // 4 bytes padding (vec3 takes 16 bytes in std140!)

//     float     metallicFactor = 1.0f;                     // 4 bytes, offset 32
//     float     roughnessFactor = 1.0f;                    // 4 bytes, offset 36
//     float     alphaCutoff = 0.5f;                        // 4 bytes, offset 40
//     AlphaMode alphaMode = AlphaMode::Opaque;             // 4 bytes, offset 44

//     // Texture presence flags (16 bytes aligned)
//     uint32_t hasBaseColorTex = 0;                        // 4 bytes, offset 48
//     uint32_t hasNormalTex = 0;                           // 4 bytes, offset 52
//     uint32_t hasMetallicRoughnessTex = 0;                // 4 bytes, offset 56
//     uint32_t hasEmissiveTex = 0;                         // 4 bytes, offset 60

//     uint32_t hasOcclusionTex = 0;                        // 4 bytes, offset 64
//     uint32_t _pad0 = 0;                                  // 4 bytes, offset 68
//     uint32_t _pad1 = 0;                                  // 4 bytes, offset 72
//     uint32_t _pad2 = 0;                                  // 4 bytes, offset 76

//     // Total: 80 bytes (std140 aligned)

//     // Default constructor
//     MaterialParams() = default;
// };

// // Ensure proper alignment for GPU (std140/std430)
// static_assert(sizeof(MaterialParams) % 16 == 0, "MaterialParams must be 16-byte aligned");

// /**
//  * VyMaterial - PBR material with textures and parameters
//  *
//  * Descriptor Set Layout (Set 1):
//  *   Binding 0: Base Color / Albedo texture (RGBA)
//  *   Binding 1: Normal map (RGB, tangent space)
//  *   Binding 2: Metallic-Roughness texture (B=metallic, G=roughness)
//  *   Binding 3: Emissive texture (RGB)
//  *   Binding 4: Occlusion texture (R channel)
//  *
//  * Design notes:
//  * - Textures are non-owning pointers (Model owns the actual Texture objects)
//  * - VyMaterial owns its descriptor set
//  * - Supports both textured and non-textured materials (uses white/default textures)
//  */
// class VyMaterial {
// public:
//     VyMaterial() = default;
//     ~VyMaterial() = default;

//     // Move-only
//     VyMaterial(const VyMaterial&) = delete;
//     VyMaterial& operator=(const VyMaterial&) = delete;
//     VyMaterial(VyMaterial&&) noexcept = default;
//     VyMaterial& operator=(VyMaterial&&) noexcept = default;

//     // --- Static Factory ---

//     /**
//      * Create descriptor set layout for materials (Set 1)
//      *
//      * Bindings:
//      *   0: Base Color / Albedo (COMBINED_IMAGE_SAMPLER)
//      *   1: Normal Map (COMBINED_IMAGE_SAMPLER)
//      *   2: Metallic-Roughness (COMBINED_IMAGE_SAMPLER)
//      *   3: Emissive (COMBINED_IMAGE_SAMPLER)
//      *   4: Occlusion (COMBINED_IMAGE_SAMPLER)
//      *
//      * All bindings are available in fragment shader stage.
//      *
//      * @param device Device reference
//      * @return DescriptorSetLayout for Set 1
//      */
//     static VyDescriptorSetLayout createDescriptorSetLayout();

//     // --- Creation ---

//     /**
//      * Initialize material with textures and parameters
//      *
//      * @param device Device reference
//      * @param allocator Descriptor allocator for Set 1
//      * @param layout Descriptor set layout for materials (Set 1)
//      * @param params VyMaterial parameters
//      * @param albedo Base color texture (optional, can be nullptr)
//      * @param normal Normal map texture (optional)
//      * @param metallicRoughness Metallic-Roughness texture (optional)
//      * @param emissive Emissive texture (optional)
//      * @param occlusion Occlusion texture (optional)
//      * @param defaultWhite Default white texture for missing albedo/emissive
//      * @param defaultNormal Default normal map (flat, pointing up)
//      * @param defaultMetallicRoughness Default MR texture (non-metallic, rough)
//      */
//     void init(
//         // DescriptorAllocator& allocator,
//         const VyDescriptorSetLayout& layout,
//         const MaterialParams& params,
//         VyTexture* albedo,
//         VyTexture* normal,
//         VyTexture* metallicRoughness,
//         VyTexture* emissive,
//         VyTexture* occlusion,
//         VyTexture* defaultWhite,
//         VyTexture* defaultNormal,
//         VyTexture* defaultMetallicRoughness
//     );

//     // --- Accessors ---

//     VkDescriptorSet descriptorSet() const { return descriptorSet_; }
//     const MaterialParams& params() const { return params_; }
//     MaterialParams& params() { return params_; }

//     // Check if material has specific textures
//     bool hasAlbedoTexture() const { return params_.hasBaseColorTex != 0; }
//     bool hasNormalTexture() const { return params_.hasNormalTex != 0; }
//     bool hasMetallicRoughnessTexture() const { return params_.hasMetallicRoughnessTex != 0; }
//     bool hasEmissiveTexture() const { return params_.hasEmissiveTex != 0; }
//     bool hasOcclusionTexture() const { return params_.hasOcclusionTex != 0; }

//     // Get alpha mode
//     AlphaMode alphaMode() const { return params_.alphaMode; }
//     bool isOpaque() const { return params_.alphaMode == AlphaMode::Opaque; }
//     bool isBlended() const { return params_.alphaMode == AlphaMode::Blend; }
//     bool isMasked() const { return params_.alphaMode == AlphaMode::Mask; }

//     // Name (for debugging)
//     void setName(const std::string& name) { name_ = name; }
//     const std::string& name() const { return name_; }

// private:
//     VkDescriptorSet descriptorSet_ = VK_NULL_HANDLE;
//     MaterialParams  params_;
//     std::string     name_;
// };

// /**
//  * MaterialBuilder - Fluent interface for constructing materials
//  *
//  * Usage:
//  *   VyMaterial mat = MaterialBuilder()
//  *       .withAlbedo(albedoTex)
//  *       .withNormal(normalTex)
//  *       .withBaseColorFactor({0.8f, 0.2f, 0.2f, 1.0f})
//  *       .withMetallicFactor(0.0f)
//  *       .withRoughnessFactor(0.5f)
//  *       .build(device, allocator, layout, defaultTextures);
//  */
// class MaterialBuilder {
// public:
//     MaterialBuilder() = default;

//     MaterialBuilder& withAlbedo(VyTexture* tex) {
//         albedo_ = tex;
//         return *this;
//     }

//     MaterialBuilder& withNormal(VyTexture* tex) {
//         normal_ = tex;
//         return *this;
//     }

//     MaterialBuilder& withMetallicRoughness(VyTexture* tex) {
//         metallicRoughness_ = tex;
//         return *this;
//     }

//     MaterialBuilder& withEmissive(VyTexture* tex) {
//         emissive_ = tex;
//         return *this;
//     }

//     MaterialBuilder& withOcclusion(VyTexture* tex) {
//         occlusion_ = tex;
//         return *this;
//     }

//     MaterialBuilder& withBaseColorFactor(const glm::vec4& color) {
//         params_.baseColorFactor = color;
//         return *this;
//     }

//     MaterialBuilder& withEmissiveFactor(const glm::vec3& color) {
//         params_.emissiveFactor = color;
//         return *this;
//     }

//     MaterialBuilder& withMetallicFactor(float metallic) {
//         params_.metallicFactor = metallic;
//         return *this;
//     }

//     MaterialBuilder& withRoughnessFactor(float roughness) {
//         params_.roughnessFactor = roughness;
//         return *this;
//     }

//     MaterialBuilder& withAlphaMode(AlphaMode mode) {
//         params_.alphaMode = mode;
//         return *this;
//     }

//     MaterialBuilder& withAlphaCutoff(float cutoff) {
//         params_.alphaCutoff = cutoff;
//         return *this;
//     }

//     MaterialBuilder& withName(const std::string& name) {
//         name_ = name;
//         return *this;
//     }

//     VyMaterial build(
//         // DescriptorAllocator& allocator,
//         const VyDescriptorSetLayout& layout,
//         VyTexture* defaultWhite,
//         VyTexture* defaultNormal,
//         VyTexture* defaultMetallicRoughness
//     );

// private:
//     MaterialParams params_;
//     VyTexture* albedo_ = nullptr;
//     VyTexture* normal_ = nullptr;
//     VyTexture* metallicRoughness_ = nullptr;
//     VyTexture* emissive_ = nullptr;
//     VyTexture* occlusion_ = nullptr;
//     std::string name_;
// };


//     /**
//      * VyMesh - Single drawable mesh with vertex/index buffers
//      *
//      * Design notes:
//      * - Owns vertex and index buffers
//      * - References a VyMaterial (non-owning)
//      * - Stores local VyAABB for culling
//      * - Provides draw() method for command recording
//      */
//     class VyMesh 
//     {
//     public:
//         VyMesh()  = default;
//         ~VyMesh() = default;

//         // Move-only
//         VyMesh(const VyMesh&)            = delete;
//         VyMesh& operator=(const VyMesh&) = delete;

//         VyMesh(VyMesh&&) noexcept            = default;
//         VyMesh& operator=(VyMesh&&) noexcept = default;

//         /**
//          * Create mesh from vertex and index data
//          *
//          * @param device Device reference
//          * @param uploader Staging uploader for GPU upload
//          * @param vertices VyVertex data
//          * @param indices Index data (U32)
//          * @param material VyMaterial reference (can be nullptr)
//          * @param debugName Debug name for buffers
//          */
//         void create(
//             const TVector<VyVertex>& vertices,
//             const TVector<U32>&      indices,
//             VyMaterial*              material  = nullptr,
//             const TString&           debugName = "mesh"
//         );

//         /**
//          * Draw the mesh
//          *
//          * @param cmdBuffer    Command buffer
//          * @param bBindMaterial If true, binds the material descriptor set (Set 1)
//          * @param materialSetLayout Pipeline layout for material binding
//          */
//         void draw(
//             VkCommandBuffer  cmdBuffer, 
//             bool             bBindMaterial = false, 
//             VkPipelineLayout materialSetLayout = VK_NULL_HANDLE
//         );

//         // Accessors
//         U32            vertexCount() const { return m_VertexCount; }
//         U32            indexCount()  const { return m_IndexCount; }
//         VyMaterial*    material()    const { return m_Material; }
//         const VyAABB&  bounds()      const { return m_Bounds; }
//         const TString& name()        const { return m_Name; }

//         VkBuffer vertexBuffer() const { return m_VertexBuffer->handle(); }
//         VkBuffer indexBuffer()  const { return m_IndexBuffer ->handle(); }

//         // Setters
//         void setMaterial(VyMaterial* mat) { m_Material = mat;  }
//         void setName(const TString& name) { m_Name     = name; }

//     private:
//         void computeBounds(const TVector<VyVertex>& vertices);

//     private:
//         Unique<VyBuffer> m_VertexBuffer;
//         Unique<VyBuffer> m_IndexBuffer;
//         U32              m_VertexCount = 0;
//         U32              m_IndexCount = 0;
//         VyMaterial*      m_Material = nullptr;  // Non-owning
//         VyAABB           m_Bounds;
//         TString          m_Name;
//     };

//     /**
//      * MeshBuilder - Helper for building meshes
//      *
//      * Usage:
//      *   TVector<VyVertex> verts = { ... };
//      *   TVector<U32> inds = { ... };
//      *   VyMesh mesh = MeshBuilder()
//      *       .setVertices(verts)
//      *       .setIndices(inds)
//      *       .setMaterial(material)
//      *       .build(device, uploader);
//      */
//     class MeshBuilder 
//     {
//     public:
//         MeshBuilder() = default;

//         MeshBuilder& setVertices(const TVector<VyVertex>& vertices) 
//         {
//             m_Vertices = vertices;
//             return *this;
//         }

//         MeshBuilder& setIndices(const TVector<U32>& indices) 
//         {
//             m_Indices = indices;
//             return *this;
//         }

//         MeshBuilder& setMaterial(VyMaterial* pMaterial) 
//         {
//             m_Material = pMaterial;
//             return *this;
//         }

//         MeshBuilder& setName(const TString& name) 
//         {
//             m_Name = name;
//             return *this;
//         }

//         VyMesh build();

//     private:
//         TVector<VyVertex> m_Vertices;
//         TVector<U32>      m_Indices;
//         VyMaterial*       m_Material = nullptr;
//         TString           m_Name;
//     };
}