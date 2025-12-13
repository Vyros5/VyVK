#include <Vy/GFX/Resources/StaticMesh.h>

#include <Vy/GFX/Context.h>
#include <Vy/Globals.h>
// #include <tiny_obj_loader.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace Vy
{
    TVector<VkVertexInputBindingDescription> 
    VyStaticMesh::vertexBindingDescriptions() 
    {
        TVector<VkVertexInputBindingDescription> bindingDescriptions(1);
        {
            bindingDescriptions[0].binding   = 0;
            bindingDescriptions[0].stride    = sizeof(VyVertex);
            bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        }

        return bindingDescriptions;
    }


    TVector<VkVertexInputAttributeDescription> 
    VyStaticMesh::vertexAttributeDescriptions() 
    {
        TVector<VkVertexInputAttributeDescription> attributeDescriptions{};
        {
            attributeDescriptions.push_back({ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(VyVertex, Position) });
            attributeDescriptions.push_back({ 1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(VyVertex, Normal  ) });
            attributeDescriptions.push_back({ 2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(VyVertex, Color   ) });
            attributeDescriptions.push_back({ 3, 0, VK_FORMAT_R32G32_SFLOAT,    offsetof(VyVertex, UV      ) });
        }

        return attributeDescriptions;
    }


    TVector<VkVertexInputAttributeDescription> 
    VyStaticMesh::vertexAttributeDescriptionOnlyPositon() 
    {
        TVector<VkVertexInputAttributeDescription> attributeDescriptions{};
        {
            attributeDescriptions.push_back({ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(VyVertex, Position) });
        }

        return attributeDescriptions;
    }


    VyStaticMesh::VyStaticMesh(TVector<VyVertex>& vertices, TVector<U32>& indices)
    {
        createVertexBuffers(vertices);
        createIndexBuffers(indices);
    }


    VyStaticMesh::~VyStaticMesh() 
    {
    }


    void VyStaticMesh::createVertexBuffers(TVector<VyVertex>& vertices) 
    {
        m_VertexCount = static_cast<U32>(vertices.size());

        VY_ASSERT(m_VertexCount >= 3, "Vertex count must be at least 3");

        VkDeviceSize bufferSize = sizeof(vertices[0]) * m_VertexCount;
        U32          vertexSize = sizeof(vertices[0]);

        VyBuffer stagingBuffer{ VyBuffer::stagingBuffer(vertexSize, m_VertexCount) };

        stagingBuffer.map();
        stagingBuffer.writeToBuffer((void*)vertices.data(), bufferSize);

        m_VertexBuffer = MakeUnique<VyBuffer>( VyBuffer::vertexBuffer(vertexSize, m_VertexCount), false );

        VyContext::device().copyBuffer(stagingBuffer.handle(), m_VertexBuffer->handle(), bufferSize);
    }


    void VyStaticMesh::createIndexBuffers(TVector<U32>& indices) 
    {
        m_IndexCount = static_cast<U32>(indices.size());

        if (m_IndexCount <= 0)
        {
            return;
        }

        VkDeviceSize bufferSize = sizeof(indices[0]) * m_IndexCount;
        U32          indexSize  = sizeof(indices[0]);

        VyBuffer stagingBuffer{ VyBuffer::stagingBuffer(indexSize, m_IndexCount) };

        stagingBuffer.map();
        stagingBuffer.writeToBuffer((void*)indices.data(), bufferSize);

        m_IndexBuffer = MakeUnique<VyBuffer>( VyBuffer::indexBuffer(indexSize, m_IndexCount), false );

        VyContext::device().copyBuffer(stagingBuffer.handle(), m_IndexBuffer->handle(), bufferSize);
    }


    void VyStaticMesh::draw(VkCommandBuffer cmdBuffer) 
    {
        if (m_IndexBuffer) 
        {
            vkCmdDrawIndexed(cmdBuffer, m_IndexCount, 1, 0, 0, 0);
        } 
        else 
        {
            vkCmdDraw(cmdBuffer, m_VertexCount, 1, 0, 0);
        }
    }


    void VyStaticMesh::bind(VkCommandBuffer cmdBuffer) 
    {
        VkBuffer     buffers[] = { m_VertexBuffer->handle() };
        VkDeviceSize offsets[] = { 0 };

        vkCmdBindVertexBuffers(cmdBuffer, 0, 1, buffers, offsets);

        if (m_IndexBuffer) 
        {
            vkCmdBindIndexBuffer(cmdBuffer, m_IndexBuffer->handle(), 0, VK_INDEX_TYPE_UINT32);
        }
    }


    Unique<VyStaticMesh> 
    VyStaticMesh::create(const Path& file)
    {
        VyStaticMesh::Builder builder;

        // Load static meshes from the MODELS_DIR.
        builder.loadModel(MODELS_DIR / file);

		return VyStaticMesh::create(builder.Vertices, builder.Indices);
    }


    Unique<VyStaticMesh> 
    VyStaticMesh::create(TVector<VyVertex>& vertices, TVector<U32>& indices) 
    {
        return MakeUnique<VyStaticMesh>(vertices, indices);
    }


    void VyStaticMesh::Builder::loadModel(const Path& file) 
    {
        if (!FS::exists(file))
        {
            VY_THROW_RUNTIME_ERROR("Failed to find file: " + file.string());
        }

        // Create Assimp importer.
        Assimp::Importer importer;
        
        // Import settings for better compatibility and performance.
        const aiScene* pScene = importer.ReadFile(file.string(), 
            aiProcess_Triangulate           | // Triangulates all faces of all meshes.
            aiProcess_FlipUVs               | // Flips all UV coordinates along the y-axis.
            aiProcess_GenNormals            | // Generates normals for all faces of all meshes.
            aiProcess_CalcTangentSpace      | // Calculates the tangents and bitangents for the imported meshes.
            aiProcess_JoinIdenticalVertices | // Identifies and joins identical vertex data sets within all imported meshes.
            aiProcess_SortByPType           | // Splits meshes with more than one primitive type in homogeneous sub-meshes.
            aiProcess_ImproveCacheLocality  | // Reorders triangles for better vertex cache locality.
            aiProcess_OptimizeMeshes        | // A post-processing step to reduce the number of meshes. (reduce the number of draw calls)
            aiProcess_OptimizeGraph         | // A post-processing step to optimize the scene hierarchy.
            aiProcess_ValidateDataStructure   // Validates the imported scene data structure.
        );

        // Check for errors.
        if (!pScene || pScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !pScene->mRootNode) 
        {
            VY_ERROR_TAG("VyStaticMesh", "Assimp Error: {}", importer.GetErrorString());

            VY_THROW_RUNTIME_ERROR("Failed to load model: " + file.string());
        }

        // Clear existing data.
        Vertices.clear();
        Indices .clear();

        // Process the root node recursively.
        processNode(pScene->mRootNode, pScene);
    }


    void VyStaticMesh::Builder::processNode(aiNode* pNode, const aiScene* pScene) 
    {
        // Process all the node's meshes.
        for (U32 i = 0; i < pNode->mNumMeshes; i++) 
        {
            aiMesh* pMesh = pScene->mMeshes[pNode->mMeshes[i]];

            processMesh(pMesh, pScene);
        }

        // Process all the node's children.
        for (U32 i = 0; i < pNode->mNumChildren; i++) 
        {
            processNode(pNode->mChildren[i], pScene);
        }
    }

    
    void VyStaticMesh::Builder::processMesh(aiMesh* pMesh, const aiScene* pScene) 
    {
        THashMap<VyVertex, U32> uniqueVertices{};
        
        // Process Vertices.
        for (U32 i = 0; i < pMesh->mNumVertices; i++) 
        {
            VyVertex vertex{};

            // [ Position ]
            if (pMesh->mVertices) 
            {
                vertex.Position = {
                    pMesh->mVertices[i].x,
                    pMesh->mVertices[i].y,
                    pMesh->mVertices[i].z
                };
            }

            // [ Normal ]
            if (pMesh->mNormals) 
            {
                vertex.Normal = {
                    pMesh->mNormals[i].x,
                    pMesh->mNormals[i].y,
                    pMesh->mNormals[i].z
                };
            } 
            else 
            {
                vertex.Normal = { 0.0f, 1.0f, 0.0f }; // Default up normal
            }

            // [ UV ]
            if (pMesh->mTextureCoords[0]) 
            {
                vertex.UV = {
                    pMesh->mTextureCoords[0][i].x,
                    pMesh->mTextureCoords[0][i].y
                };
            } 
            else 
            {
                vertex.UV = { 0.0f, 0.0f };
            }

            // [ Color ]
            if (pMesh->mColors[0]) 
            {
                vertex.Color = {
                    pMesh->mColors[0][i].r,
                    pMesh->mColors[0][i].g,
                    pMesh->mColors[0][i].b
                };
            } 
            else 
            {
                vertex.Color = { 1.0f, 1.0f, 1.0f }; // Default white
            }

//             if (pMesh->HasTangentsAndBitangents())
//             {
//                 vertex.Tangent = Vec3(
//                     pMesh->mTangents[vIdx].x,
//                     pMesh->mTangents[vIdx].y,
//                     pMesh->mTangents[vIdx].z
//                 );

//                 vertex.Bitangent = Vec3(
//                     pMesh->mBitangents[vIdx].x,
//                     pMesh->mBitangents[vIdx].y,
//                     pMesh->mBitangents[vIdx].z
//                 );
//             }

            // Check for unique Vertices to avoid duplicates.
            if (uniqueVertices.count(vertex) == 0) 
            {
                uniqueVertices[vertex] = static_cast<U32>(Vertices.size());
            
                Vertices.push_back(vertex);
            }
        }

        // Process Indices
        for (U32 i = 0; i < pMesh->mNumFaces; i++) 
        {
            aiFace face = pMesh->mFaces[i];
            
            for (U32 j = 0; j < face.mNumIndices; j++) 
            {
                VyVertex vertex{};
                
                // Get vertex data for this index.
                U32 vertexIndex = face.mIndices[j];
                
                if (pMesh->mVertices) 
                {
                    vertex.Position = {
                        pMesh->mVertices[vertexIndex].x,
                        pMesh->mVertices[vertexIndex].y,
                        pMesh->mVertices[vertexIndex].z
                    };
                }

                if (pMesh->mNormals) 
                {
                    vertex.Normal = {
                        pMesh->mNormals[vertexIndex].x,
                        pMesh->mNormals[vertexIndex].y,
                        pMesh->mNormals[vertexIndex].z
                    };
                } 
                else 
                {
                    vertex.Normal = {0.0f, 1.0f, 0.0f};
                }

                if (pMesh->mTextureCoords[0]) 
                {
                    vertex.UV = {
                        pMesh->mTextureCoords[0][vertexIndex].x,
                        pMesh->mTextureCoords[0][vertexIndex].y
                    };
                } 
                else 
                {
                    vertex.UV = {0.0f, 0.0f};
                }

                if (pMesh->mColors[0]) 
                {
                    vertex.Color = {
                        pMesh->mColors[0][vertexIndex].r,
                        pMesh->mColors[0][vertexIndex].g,
                        pMesh->mColors[0][vertexIndex].b
                    };
                } 
                else 
                {
                    vertex.Color = {1.0f, 1.0f, 1.0f};
                }

                Indices.push_back(uniqueVertices[vertex]);
            }
        }

        // Process material.
        if (pMesh->mMaterialIndex >= 0) 
        {
            aiMaterial* material = pScene->mMaterials[ pMesh->mMaterialIndex ];

            // For now, we'll just note that materials exist.

            // Future enhancement: load textures here ! 

#if 0
            auto LoadMatTexture = [&](aiTextureType type, U32& targetIdx, VkFormat format)
                {
                    U32 count = material->GetTextureCount(type);

                    for (U32 mIdx{}; mIdx < count; ++mIdx)
                    {
                        aiString texturePath;
                        material->GetTexture(type, mIdx, &texturePath);

                        String fullPath = "Textures/" + String(texturePath.C_Str());

                        auto [it, succeeded] = PathToIndex.insert({ fullPath, static_cast<U32>(PathToIndex.size())});
                        targetIdx = it->second;

                        if (succeeded)
                        {
                            Textures.emplace_back(fullPath, format);
                        }
                    }
                };

            // -- Diffuse --
            auto& mat = SubMeshes.back().Material;
            LoadMatTexture(aiTextureType_DIFFUSE,           mat.AlbedoIndex,   VK_FORMAT_R8G8B8A8_SRGB);

            LoadMatTexture(aiTextureType_SPECULAR,          mat.SpecularIndex,  VK_FORMAT_R8G8B8A8_UNORM);
            LoadMatTexture(aiTextureType_SHININESS,         mat.ShininessIndex, VK_FORMAT_R8G8B8A8_UNORM);

            LoadMatTexture(aiTextureType_HEIGHT,            mat.HeightIndex,    VK_FORMAT_R8G8B8A8_UNORM);
            LoadMatTexture(aiTextureType_NORMALS,           mat.NormalIndex,    VK_FORMAT_R8G8B8A8_UNORM);

            LoadMatTexture(aiTextureType_DIFFUSE_ROUGHNESS, mat.RoughnessIndex, VK_FORMAT_R8G8B8A8_UNORM);
            LoadMatTexture(aiTextureType_METALNESS,         mat.MetalnessIndex, VK_FORMAT_R8G8B8A8_UNORM);

            LoadMatTexture(aiTextureType_OPACITY,           mat.OpacityIndex,   VK_FORMAT_R8G8B8A8_UNORM);
#endif
        }
    }


    void VyStaticMesh::Builder::loadMaterialTextures(aiMaterial* mat, int type, const String& directory) 
    {
        // This method can be implemented later for automatic texture loading
        // For now, textures are loaded manually through the material system.
    }






    // Unique<VyStaticMesh> VyStaticMesh::create(const Path& file)
    // {
    //     if (!FS::exists(file))
    //     {
    //         VY_THROW_RUNTIME_ERROR("Failed to find file: " + file.string());
    //     }

	//     TVector<VyVertex> vertices{}; // List of vertices in the model.
	//     TVector<U32>      indices {}; // List of indices for indexed rendering.

	// 	tinyobj::attrib_t            attrib;
    //     TVector<tinyobj::shape_t>    shapes;
    //     TVector<tinyobj::material_t> materials;
    //     String                       warn, err;

    //     if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, file.string().c_str())) 
    //     {
    //         VY_ERROR("{} : {}", warn, err);
    //         VY_THROW_RUNTIME_ERROR("Failed to load model from " + file.string());
    //     }

    //     vertices.clear();
    //     indices .clear();

    //     THashMap<VyVertex, U32> uniqueVertices{};

    //     for (const auto& shape : shapes) 
    //     {
    //         for (const auto& index : shape.mesh.indices) 
    //         {
    //             VyVertex vertex{};

    //             if (index.vertex_index >= 0) 
    //             {
    //                 vertex.Position = Vec3(
    //                     attrib.vertices[3 * index.vertex_index + 0],
    //                     attrib.vertices[3 * index.vertex_index + 1],
    //                     attrib.vertices[3 * index.vertex_index + 2]
    //                     // 1.0f // unused
    //                 );

    //                 if (!attrib.colors.empty())
    //                 {
    //                     vertex.Color = Vec3(
    //                         attrib.colors[3 * index.vertex_index + 0],
    //                         attrib.colors[3 * index.vertex_index + 1],
    //                         attrib.colors[3 * index.vertex_index + 2]
    //                         // 1.0f // unused
    //                     );
    //                 }
    //                 else
    //                 {
    //                     vertex.Color = Vec3(1.0f);
    //                 }
    //             }

    //             if (index.normal_index >= 0 && !attrib.normals.empty()) 
    //             {
    //                 vertex.Normal = Vec3(
    //                     attrib.normals[3 * index.normal_index + 0],
    //                     attrib.normals[3 * index.normal_index + 1],
    //                     attrib.normals[3 * index.normal_index + 2]
    //                     // 1.0f // unused
    //                 );
    //             }
    //             else
    //             {
    //                 vertex.Normal = Vec3(0.0f);
    //             }

    //             if (index.texcoord_index >= 0 && !attrib.texcoords.empty()) 
    //             {
    //                 vertex.UV = Vec2(
	// 					attrib.texcoords[2 * index.texcoord_index + 0],
	//                     attrib.texcoords[2 * index.texcoord_index + 1] // 1.0f - ...
    //                     // 1.0f, // unused
    //                     // 1.0f  // unused
    //                 );
    //             }
    //             else 
    //             {
	// 				vertex.UV = Vec2(0.0f);
    //             }

	// 			if (!uniqueVertices.contains(vertex)) 
    //             {
	// 			    uniqueVertices[ vertex ] = static_cast<U32>(vertices.size());

	// 			    vertices.push_back(vertex);
	// 			}
				
    //             indices.push_back(uniqueVertices[ vertex ]);
    //         }
    //     }

        // Iterate through triangles and calculate per-triangle tangents and bitangents
        // Only calculate tangents if the model has UV coordinates
        // if (!attrib.texcoords.empty()) 
        // {
        //     for (size_t i = 0; i < indices.size(); i += 3) 
        //     {
        //         VyVertex& v0 = vertices[indices[i + 0]];
        //         VyVertex& v1 = vertices[indices[i + 1]];
        //         VyVertex& v2 = vertices[indices[i + 2]];

        //         Vec3 edge1 = v1.Position - v0.Position;
        //         Vec3 edge2 = v2.Position - v0.Position;

        //         Vec2 deltaUV1 = Vec2(v1.UV) - Vec2(v0.UV);
        //         Vec2 deltaUV2 = Vec2(v2.UV) - Vec2(v0.UV);

        //         float det = (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

        //         // Check for zero determinant to avoid division by zero.
        //         if (glm::abs(det) < 1e-6f) 
        //         {
        //             v0.Tangent = Vec4(1.0f, 0.0f, 0.0f, 1.0f);
        //             v1.Tangent = Vec4(1.0f, 0.0f, 0.0f, 1.0f);
        //             v2.Tangent = Vec4(1.0f, 0.0f, 0.0f, 1.0f);

        //             continue; // Skip to the next triangle.
        //         }

        //         float f = 1.0f / det;

        //         Vec3 tangent;
        //         tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        //         tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        //         tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

        //         tangent = glm::normalize(tangent);

        //         float handedness = (glm::dot(glm::cross(Vec3(v0.Normal), Vec3(v1.Normal)), tangent) < 0.0f) ? -1.0f : 1.0f;
        //         Vec4 tangent4 = Vec4(tangent, handedness);

        //         v0.Tangent = tangent4;
        //         v1.Tangent = tangent4;
        //         v2.Tangent = tangent4;
        //     }
        // }
        // else 
        // {
        //     // Assign default tangents if no UVs are present.
        //     for (VyVertex& v : vertices) 
        //     {
        //         v.Tangent = Vec4(1.0f, 0.0f, 0.0f, 1.0f);
        //     }
        // }

	// 	return VyStaticMesh::create(vertices, indices);
    // }
}