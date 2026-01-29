// #pragma once

// #include <VyEngine/GFX/Resources/Model/IModel.h>
// #include <VyEngine/GFX/Resources/Mesh/Vertex.h>
// #include <VyEngine/GFX/Resources/Material/Material.h>

// #include <VyEngine/VK/Buffer/Buffer.h>
// #include <VyEngine/VK/Descriptors/Descriptors.h>

// #include <assimp/Importer.hpp>
// #include <assimp/MemoryIOWrapper.h>
// #include <assimp/postprocess.h>
// #include <assimp/scene.h>

// // Forward declarations
// namespace tinygltf 
// {
//     class Model;
//     // class Skin;
//     // class Animation;
//     class Node;
//     class Scene;
// }

// namespace Vy
// {
//     struct ModelLoadInfo
//     {
//         TString Name;

//         U32     SceneCount = 0;
//         U32     NodeCount  = 0;
//         U32     MeshCount  = 0;

//         U32     VertexCount  = 0;
//         U32     IndexCount   = 0;
//         U32     TextureCount = 0;
//     };


//     struct VyPrimitive
//     {
//         U32 FirstIndex;
//         U32 IndexCount;

//         U32 FirstVertex;
//         U32 VertexCount;

//         VyPBRMaterial Material;
//     };


//     struct VyMaterialInfo
//     {
//         TString      Name;

//         // VyPBRMaterial PbrMaterial;
//         int           MaterialId; // Index in the materials array

//         // Texture paths from MTL file (relative to OBJ file location)
//         TString AlbedoTexPath;
//         TString NormalTexPath;
//         TString MetallicRoughnessTexPath;
//         TString MetallicTexPath;
//         TString RoughnessTexPath;
//         TString AOTexPath;
//         TString EmissiveTexPath;
//         // TString SpecularGlossinessTexPath;
//         // TString TransmissionTexPath;
//         // TString ClearcoatTexPath;
//         // TString ClearcoatRoughnessTexPath;
//         // TString ClearcoatNormalTexPath;
//     };

// 	class VyMesh
// 	{
// 	public:

// 		VyMesh(TVector<VyVertex> vertices, TVector<U32> indices);

// 		~VyMesh() = default;

// 	public:

// 		inline TVector<VyVertex>& vertices() { return m_Vertices; }
// 		inline TVector<U32>&      indices()  { return m_Indices;  }

// 		inline size_t vertexCount() const { return m_Vertices.size(); }
//         inline size_t indexCount()  const { return m_Indices.size(); }

// 		inline VyBuffer* vertexBuffer() const { return m_VertexBuffer.get(); }
// 		inline VyBuffer* indexBuffer()  const { return m_IndexBuffer.get(); }

// 	public:

// 		// draws the mesh
// 		void draw(VkCommandBuffer commandBuffer, VkPipelineLayout layout, VkDescriptorSet& descriptorSet);

// 		void createBuffers();

// 		void destroy();

// 	private:

//         Unique<VyBuffer>  m_VertexBuffer;
//         TVector<VyVertex> m_Vertices;

//         Unique<VyBuffer> m_IndexBuffer;
//         TVector<U32>     m_Indices;
// 	};

//     class VyModel : public IModel
//     {
//     public:

//         VyModel(
//             const TString&         filepath, 
//             VyDescriptorSetLayout& materialSetLayout, 
//             VyDescriptorPool&      descriptorPool
//         );
        
//         ~VyModel() override;

//         static Shared<VyModel> createFromFile(const TString& filepath, VyDescriptorSetLayout& materialSetLayout, VyDescriptorPool& descriptorPool)
//         {
//             return MakeShared<VyModel>( filepath, materialSetLayout, descriptorPool );
//         }

//         void bind(VkCommandBuffer cmdBuffer) override;

//         void draw(VkCommandBuffer cmdBuffer, VkPipelineLayout pipelineLayout, int setCount, bool bRenderMaterial);

//         void createDescriptorSet(VyPBRMaterial& material, VyDescriptorSetLayout& materialSetLayout, VyDescriptorPool& descriptorPool);

//         const TVector<VyPrimitive>& primitives() const { return m_Primitives; }

//     private:
//         void createVertexBuffer(const TVector<VyVertex>& vertices);
//         void createIndexBuffer (const TVector<U32>&      indices );

// 		void loadModel(const TString& filepath);
// 		void processScene(const aiScene* pScene);
// 		void processNode(aiNode* pNode, const aiScene* pScene);
//         VyMesh processMesh(aiMesh* pMesh, const aiScene* pScene);
//         VyPBRMaterial processMaterial(aiMesh* pMesh, const aiScene* pScene);


//         TString getTexturePath(aiMaterial* pMaterial, aiTextureType type);
//         TString getDirectory() const;

//         TVector<VyMesh> m_Meshes;
//         // Unique<VyBuffer>  m_VertexBuffer;
//         // TVector<VyVertex> m_Vertices;

//         // Unique<VyBuffer> m_IndexBuffer;
//         // TVector<U32>     m_Indices;

//         // bool m_HasIndexBuffer = false;

//         TVector<VyPrimitive>       m_Primitives;
//         TVector<Shared<VyTexture>> m_Textures;

//         Shared<VyTexture> m_DefaultWhite;
//         Shared<VyTexture> m_DefaultNormal;

//         TString m_Filepath;
//     };
// }