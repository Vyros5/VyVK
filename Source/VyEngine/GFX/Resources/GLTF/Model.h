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

    class VyGLTFModel
    {
    public:
        struct VyMaterial
        {
            Shared<VyTexture> AlbedoTexture;
            Shared<VyTexture> EmissiveTexture;
            Shared<VyTexture> MetallicRoughnessTexture;
            Shared<VyTexture> OcclusionTexture;
            Shared<VyTexture> NormalTexture;

            Vec4  AlbedoFactor     = Vec4(1.0f);
            Vec4  EmissiveFactor   = Vec4(1.0f);
            float EmissiveStrength = 1.0f;
            float MetallicFactor   = 1.0f;
            float RoughnessFactor  = 1.0f;

            VkDescriptorSet DescriptorSet;
        };

        struct VyPrimitive
        {
            U32 FirstIndex;
            U32 IndexCount;

            U32 FirstVertex;
            U32 VertexCount;

            VyMaterial Material;
        };

        // struct Builder 
        // {
        //     TVector<VyVertex>    Vertices{};
        //     TVector<U32>         Indices {};
        //     TVector<VyPrimitive> Primitives;

        //     void loadModel(const TString& filepath);

        // private:
        //     void loadObjModel(const TString& filepath);
        //     void loadGltfModel(const TString& filepath);

        //     void computeTangents();
        // };

        VyGLTFModel(const TString& filepath, VyDescriptorSetLayout& materialSetLayout, VyDescriptorPool& descriptorPool);
        
        ~VyGLTFModel();

        // static Shared<VyGLTFModel> createFromFile(const TString& filepath);

        static Shared<VyGLTFModel> createFromFile(const TString& filepath, VyDescriptorSetLayout& materialSetLayout, VyDescriptorPool& descriptorPool)
        {
            return MakeShared<VyGLTFModel>( filepath, materialSetLayout, descriptorPool );
        }

        void bind(VkCommandBuffer cmdBuffer);
        void draw(VkCommandBuffer cmdBuffer, VkPipelineLayout pipelineLayout, int setCount, bool bRenderMaterial);

        void createDescriptorSet(VyMaterial& material, VyDescriptorSetLayout& materialSetLayout, VyDescriptorPool& descriptorPool);

    private:
        void createVertexBuffer(const TVector<VyVertex>& vertices);
        void createIndexBuffer(const TVector<U32>& indices);

        // VkFormat accessorFormat(tinygltf::Model& model, U32 index) const;
        // VkFormat imageFormat(tinygltf::Model& model, U32 index) const;

        Unique<VyBuffer>  m_VertexBuffer;
        TVector<VyVertex> m_Vertices;

        Unique<VyBuffer> m_IndexBuffer;
        TVector<U32>     m_Indices;

        bool m_HasIndexBuffer = false;

        TVector<VyPrimitive>       m_Primitives;
        TVector<Shared<VyTexture>> m_Textures;

        Shared<VyTexture> m_DefaultTexture;
        Shared<VyTexture> m_DefaultNormal;
        Shared<VyTexture> m_DefaultMetallicRoughness;
        
    };
}