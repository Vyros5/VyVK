// #pragma once

// #include <VyEngine/Scene/GameObject.h>
// #include <VyEngine/GFX/Resources/Texture/Texture.h>
// #include <VyEngine/GFX/Resources/Mesh/Vertex.h>
// #include <VyEngine/VK/Buffer/Buffer.h>

// #include <VyLib/STL/Path.h>

// #include <assimp/Importer.hpp>
// #include <assimp/scene.h>
// #include <assimp/postprocess.h>

// namespace Vy
// {
//     struct MeshComponent
//     {
//         TVector<U32> Meshes     = {};
//         TVector<U32> Materials  = {};
//         VyAABB       ObjectAABB = {};
//         VyAABB       WorldAABB  = {};
//     };

//     void uploadVertexAndIndexBuffers(
//         TVector<VyVertex>&   vertices,
//         TVector<uint32_t>& indices,
//         int*                    outVertexOffset,
//         uint32_t*               outfirstIndex)
//     {
//         // *outVertexOffset = static_cast<int>(m_availableVertexOffset);
//         // *outfirstIndex   = static_cast<uint32_t>(m_availableIndexOffset);

//         // auto      totalVertices = vertices.size();
//         // auto      totalIndices  = indices.size();

//         // GfxBuffer stagingBuffer;
//         // size_t    stagingBufferSize = vertices.size() * sizeof(Vertex) + indices.size() * sizeof(uint32_t);
//         // GfxBuffer::createHostWriteBuffer(
//         //     GfxBufferUsageFlags::TransferSource,
//         //     stagingBufferSize,
//         //     1,
//         //     "staging_vertex_index_buffer",
//         //     &stagingBuffer);

//         // // upload vertex buffer
//         // stagingBuffer.writeAndFlush(0, (void*)vertices.data(), totalVertices * sizeof(Vertex));

//         // m_gfxDevice->copyBuffer(
//         //     stagingBuffer.vkBuffer,
//         //     0,
//         //     m_vertexBuffer.vkBuffer,
//         //     m_availableVertexOffset * sizeof(Vertex),
//         //     totalVertices * sizeof(Vertex));

//         // m_availableVertexOffset += totalVertices;

//         // // upload index buffer
//         // stagingBuffer.writeAndFlush(
//         //     totalVertices * sizeof(Vertex),
//         //     (void*)indices.data(),
//         //     totalIndices * sizeof(uint32_t));

//         // m_gfxDevice->copyBuffer(
//         //     stagingBuffer.vkBuffer,
//         //     totalVertices * sizeof(Vertex),
//         //     m_indexBuffer.vkBuffer,
//         //     m_availableIndexOffset * sizeof(uint32_t),
//         //     totalIndices * sizeof(uint32_t));

//         // m_availableIndexOffset += totalIndices;

//         // stagingBuffer.cleanup();
//     }



//     class VyScene;


//     class AssimpLoader
//     {
//     public:
//         AssimpLoader();
//         ~AssimpLoader();

//         Unique<VyScene> readScene(const TPath& filePath);

//     private:
//         Unique<VyScene> parseScene(const aiScene* scene);
//         void parseMeshes(VyScene& scene, const aiScene* aiScene);
//         void parseMaterials(VyScene& scene, const aiScene* aiScene);

//         void traverseSceneNodes(VyScene& scene, const aiNode* node, const aiScene* aiScene, EntityHandle parentId);

//         VyGameObject parseAssimpNode(const aiNode* node);

//         TPath getGltfTexturePath(aiMaterial* mat);
//         TPath getTexturePath(aiMaterial* mat, aiTextureType type);
//         TPath getGltfMRTexturePath(aiMaterial* mat);
//         I32   read2DTexture(TPath texPath, VkFormat format) const;

//     private:
//         Assimp::Importer m_Importer;

//         TPath m_SceneDir = "";

//         bool m_IsGltf = false;

//         TVector<VyVertex> m_TempVertices = {};
//         TVector<uint32_t> m_TempIndices  = {};
//     };
// }