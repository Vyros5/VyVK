#pragma once

#include <VyEngine/GFX/Resources/Mesh/Vertex.h>
#include <VyEngine/GFX/Resources/Texture/Texture.h>

#include <VyEngine/VK/Buffer/Buffer.h>
#include <VyEngine/VK/Descriptors/Descriptors.h>

// ASSIMP imports
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

enum aiTextureType;

namespace Vy
{
    struct alignas( 16 ) MaterialUbo 
    {
        Mat4 ModelMatrix;
        Mat4 ModelInverseMatrix;

        // x: color,
        // y: normal,
        // z: emissive,
        // w: occlusion
        UVec4 AlbedoNormalEmissiveOcclusionTextureHandles{ 0 };

        // x: roughness,
        // y: metallic,
        // z: flags
        // w: ignore
        UVec4 RoughnessMetallicFlags{ 0 };

        // x: normal_scale
        // y: occlusion_factor
        // z: roughness_factor
        // w: metallic_factor
        Vec4 NormalOcclusionRoughnessMetallicFactor{ 1.0f };

        Vec3 EmissiveFactor{ 0.0f };
        Vec4 AlbedoFactor  { 1.0f };
    };


    struct SubMesh
    {
        U32 FirstIndex;
        U32 IndexCount;
        
        U32 MaterialIndex; 
    };

    struct MaterialInfo
    {
        TString AlbedoPath       { "NULL" };
        TString NormalPath       { "NULL" };
        TString MetallicRoughPath{ "NULL" };
        TString OcclusionPath    { "NULL" };
        TString EmissivePath     { "NULL" };

        U32    AlbedoIndex       { UINT32_MAX };
        U32    NormalIndex       { UINT32_MAX };
        U32    MetallicRoughIndex{ UINT32_MAX };
        U32    OcclusionIndex    { UINT32_MAX };
        U32    EmissiveIndex     { UINT32_MAX };

        Vec4  AlbedoFactor   { 1.0f };
        Vec3  EmissiveFactor { 0.0f };
        float MetallicFactor { 1.0f };
        float RoughnessFactor{ 1.0f };
        float OcclusionFactor{ 1.0f }; // Strength
        float NormalScale    { 1.0f };
    };


    // for each mesh
    struct VyMaterialSet 
    {
        U32 AlbedoTextureHandle;
        U32 NormalTextureHandle;
        U32 OcclusionTextureHandle;
        U32 RoughnessMetalnessTextureHandle;
        U32 EmissiveTextureHandle;

        Vec4  AlbedoFactor   { 1.0f };
        Vec3  EmissiveFactor { 0.0f };
        float MetallicFactor { 1.0f };
        float RoughnessFactor{ 1.0f };
        float OcclusionFactor{ 1.0f };
        float NormalScale    { 1.0f };

        Mat4 Transform{};
        Mat4 InverseTransform{};

        VyMaterialSet()  = default;
        ~VyMaterialSet() = default;
    };


    class VyModel 
    {
    public:
        struct Data 
        {
            TVector<VyVertex> Vertices{};
            TVector<U32>      Indices {};

			TVector<SubMesh>           Submeshes{};
			TVector<MaterialInfo>      Materials{};
			TVector<Unique<VyTexture>> Textures;

            void load(const TString& path);

            bool loadModel(const TPath& path, bool bAllUniqueVertices = false);

            bool loadModelAssimp(const TPath& path);

            // static Data makeSimpleCube(bool bInvert = false);
            // static void computeTangentBasis(VyVertex& v0, VyVertex& v1, VyVertex& v2, Vec3* pTanOut);

        private:
            void processNode(aiNode* pNode, const aiScene* pScene);
            void processMesh(aiMesh* pMesh, const aiScene* pScene);
            // void loadMaterialTextures(aiMaterial* pMaterial, aiTextureType type, const TPath& directory);
            // TVector<VyTexture> loadMaterialTextures(aiMaterial* pMaterial, aiTextureType type, const TPath& directory);
        };
        
        VyModel(const VyModel&)            = delete;
        VyModel& operator=(const VyModel&) = delete;
        
        VyModel(const Data& data);
        // VyModel(const Builder& builder);
        ~VyModel();
        
        static Shared<VyModel> loadFromFile(const TPath& filepath);

        static Shared<VyModel> createFromFile(const TString& filepath);

        void bind(VkCommandBuffer cmdBuffer) const;
        void draw(VkCommandBuffer cmdBuffer) const;
        void drawIndexed(VkCommandBuffer cmdBuffer, U32 indexCount, U32 firstIndex) const;

        void createDescriptorSets(VyDescriptorSetLayout& setLayout, VyDescriptorPool& pool);
        
        Data& getData() { return m_Data; };

    private:
        void createVertexBuffer(const TVector<VyVertex>& vertices);
        void createIndexBuffer (const TVector<U32>&      indices );
        
        Unique<VyBuffer> m_VertexBuffer;
        U32              m_VertexCount;

        Unique<VyBuffer> m_IndexBuffer;
        U32              m_IndexCount;
        
        bool m_HasIndexBuffer = false;

        Data m_Data; 

        VkDescriptorSet m_MeshDescriptorSet;
        Unique<VyBuffer> m_MaterialBuffer;
        VyMaterialSet m_MaterialSet;
        U32 m_Flags = 0;

        // TVector<Submesh>          m_Submeshes;
        // TVector<TString>          m_TexturePaths;
        // TMap<MaterialID, TString> m_MaterialIdToTexturePath;

        // BoundingBox m_AABB;
    };
}