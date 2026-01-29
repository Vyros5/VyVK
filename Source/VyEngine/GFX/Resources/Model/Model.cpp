#include <VyEngine/GFX/Resources/Model/Model.h>

#include <VyEngine/GFX/Resources/Model/Utils/aiConversion.h>

#include <tiny_gltf.h>

#include <VyLib/Util/Hash.h>
#include <VyLib/Core/Math.h>
#include <VyEngine/Globals.h>
#include <iostream>
#include <VyLib/Common/AnsiColor.h>
#include <VyLib/Util/String.h>

// namespace std 
// {
//     template <>
//     struct hash<Vy::VyVertex> 
//     {
//         size_t operator()(Vy::VyVertex const& vertex) const 
//         {
//             size_t seed = 0;
            
//             Vy::Hash::hashCombine(seed
//                 , vertex.Position
//                 , vertex.Color
//                 , vertex.Normal
//                 , vertex.UV
//                 , vertex.Tangent
//                 // , vertex.Bitangent
//             );
            
//             return seed;
//         }
//     };
// }

// namespace Vy
// {
//     VyMesh::VyMesh(TVector<VyVertex> vertices, TVector<U32> indices) :
//         m_Vertices{ vertices },
//         m_Indices { indices  }
//     {
//         createBuffers();
//     }


//     void VyMesh::createBuffers()
//     {
//         {
//             U32 vertexCount = static_cast<U32>( m_Vertices.size() );
            
//             VY_ASSERT(vertexCount >= 3, "Vertex count must be at least 3");
            
//             U32 vertexSize = sizeof(m_Vertices[0]);
            
//             m_VertexBuffer = MakeUnique<VyBuffer>( VyBuffer::vertexBuffer( "mesh", vertexSize, vertexCount ) );
            
//             m_VertexBuffer->upload( m_Vertices );
//         }

//         {
//             U32 indexCount = static_cast<U32>( m_Indices.size() );
            
//             if (indexCount <= 0) { return; }
            
//             U32 indexSize = sizeof(m_Indices[0]);
            
//             m_IndexBuffer = MakeUnique<VyBuffer>( VyBuffer::indexBuffer( "mesh", indexSize, indexCount ) );
            
//             m_IndexBuffer->upload( m_Indices );
//         }
//     }


//     void VyMesh::destroy()
//     {
//         m_VertexBuffer.reset();
//         m_IndexBuffer .reset();

//         m_Vertices.clear();
//         m_Indices .clear();
//     }

// }


// namespace Vy
// {
//     void VyModel::loadModel(const TString& filepath)
//     {
// 		Assimp::Importer importer;

// 		const aiScene* pScene = importer.ReadFile(filepath, 
//             aiProcess_CalcTangentSpace |
// 			aiProcess_Triangulate |
// 			aiProcess_JoinIdenticalVertices |
// 			aiProcess_SortByPType |
// 			aiProcess_FlipUVs |
// 			aiProcess_GenSmoothNormals
//         );

// 		if (pScene == nullptr)
// 		{
// 			std::cout << "Unable to read file! error message: \n" << std::endl;
// 			throw std::runtime_error(importer.GetErrorString());
// 		}

// 		processScene( pScene );
//     }


//     void VyModel::processScene(const aiScene* pScene)
//     {
// 		aiNode* pNode = pScene->mRootNode;
		
//         auto numMeshes    = pNode->mNumMeshes;
// 		auto numMaterials = pScene->mNumMaterials;
		
//         VY_INFO_TAG("VyModel", "Number of meshes    : {}", numMeshes);
//         VY_INFO_TAG("VyModel", "Number of materials : {}", numMaterials);

//         // Process the root node recursively.
//         processNode( pNode, pScene );
//     }


//     VyPBRMaterial VyModel::processMaterial(aiMesh* pMesh, const aiScene* pScene)
//     {
//         // [ Materials ]
//         if (pMesh->mMaterialIndex >= 0)
//         {
//             return;
//         }

//         aiMaterial* pMat = pScene->mMaterials[ pMesh->mMaterialIndex ];

//         VyPBRMaterial m;

//         if (pMat->GetTextureCount(aiTextureType_DIFFUSE) != 0) 
//         {
//             TString texPath = getTexturePath(pMat, aiTextureType_DIFFUSE);

//             m.AlbedoMap = VyTexture::createFromFilepath( texPath );
//         }

//         if (pMat->GetTextureCount(aiTextureType_GLTF_METALLIC_ROUGHNESS) != 0) 
//         {
//             TString texPath = getTexturePath(pMat, aiTextureType_GLTF_METALLIC_ROUGHNESS);

//             m.MetallicRoughnessMap = VyTexture::createFromFilepath( texPath );
//         }

//         if (pMat->GetTextureCount(aiTextureType_AMBIENT_OCCLUSION) != 0) 
//         {
//             TString texPath = getTexturePath(pMat, aiTextureType_AMBIENT_OCCLUSION);

//             m.AOMap = VyTexture::createFromFilepath( texPath );
//         }

//         if (pMat->GetTextureCount(aiTextureType_NORMALS) != 0) 
//         {
//             TString texPath = getTexturePath(pMat, aiTextureType_NORMALS);

//             m.NormalMap = VyTexture::createFromFilepath( texPath );
//         }

//         if (pMat->GetTextureCount(aiTextureType_EMISSIVE) != 0) 
//         {
//             TString texPath = getTexturePath(pMat, aiTextureType_EMISSIVE);

//             m.EmissiveMap = VyTexture::createFromFilepath( texPath );
//         }

//         return m;
//     }


//     void VyModel::processNode(aiNode* pNode, const aiScene* pScene)
//     {
//         // Process all the node's meshes.
//         for (U32 i = 0; i < pNode->mNumMeshes; i++)
//         {
//             aiMesh* pMesh = pScene->mMeshes[ pNode->mMeshes[ i ]];

//             m_Meshes.emplace_back( processMesh( pMesh, pScene ) );
//         }

//         // Recursively traverse and process all the node's children.
//         for (U32 i = 0; i < pNode->mNumChildren; i++)
//         {
//             processNode( pNode->mChildren[i], pScene );
//         }
//     }


//     VyMesh VyModel::processMesh(aiMesh* pMesh, const aiScene* pScene)
//     {
//         TVector<VyVertex>  meshVertices( pMesh->mNumVertices);
//         TVector<U32>       meshIndices;// ( pMesh->mNumIndices );

//         // Initialize vertices
//         for (size_t i = 0; i < pMesh->mNumVertices; i++) 
//         {
//             VyVertex& vertex = meshVertices[ i ];

//             // Positions
//             vertex.Position = Utils::AiToGlm( pMesh->mVertices[i] );

//             // Normals
//             if (pMesh->HasNormals()) 
//             {
//                 vertex.Normal = Utils::AiToGlm( pMesh->mNormals[i] );

//             } else {
//                 vertex.Normal = {0.0f, 0.0f, 0.0f};
//             }

//             // Texture Coordinates
//             if (pMesh->HasTextureCoords(0)) 
//             {
//                 vertex.UV = Utils::AiToGlm( pMesh->mTextureCoords[0][i] );

//             } else {
//                 vertex.UV = {0.0f, 0.0f};
//             }

//             // Colors
//             if (pMesh->HasVertexColors(0)) 
//             {
//                 vertex.Color = Utils::AiToGlm( pMesh->mColors[0][i] );

//             } else {
//                 vertex.Color = { 1.0f, 1.0f, 1.0f };
//             }

//             if (pMesh->HasTangentsAndBitangents()) 
//             {
//                 vertex.Tangent   = Vec4(Utils::AiToGlm( pMesh->mTangents  [ i ] ), 0.0f);
//                 // vertex.Bitangent = Utils::AiToGlm( pMesh->mBitangents[ i ] );

//             } else {
//                 vertex.Tangent   = { 0.0f, 0.0f, 0.0f, 0.0f };
//                 // vertex.Bitangent = { 0.0f, 0.0f, 0.0f };
//             }

//             // Process indices
//             for (U32 i = 0; i < pMesh->mNumFaces; i++)
//             {
//                 aiFace face = pMesh->mFaces[ i ];

//                 for (U32 j = 0; j < face.mNumIndices; j++)
//                 {
//                     meshIndices.push_back( static_cast<U32>(face.mIndices[j]) );
//                 }
//             }
//         }

//         return VyMesh{ meshVertices, meshIndices };
//     }



//     TString VyModel::getTexturePath(aiMaterial* pMaterial, aiTextureType type)
//     {
//         aiString texturePath;

//         if (pMaterial->GetTexture(type, /*index=*/0, &texturePath) != AI_SUCCESS) 
//         {
//             return {};
//         }
        
//         return getDirectory() + "/" + texturePath.C_Str();
//     }


//     TString VyModel::getDirectory() const
//     {
//         return m_Filepath.substr(0, m_Filepath.find_last_of("/"));
//     }




//     VyModel::VyModel(const TString& filepath, VyDescriptorSetLayout& materialSetLayout, VyDescriptorPool& descriptorPool)
//     {
//         m_DefaultWhite  = VyTexture::createWhiteTexture(); 
//         m_DefaultNormal = VyTexture::createNormalTexture();
    
//         tinygltf::Model    gltfModel;
//         tinygltf::TinyGLTF loader;
//         TString            err;
//         TString            warn;

//         // Load Model
//         if (filepath.find(".glb") != TString::npos)
//         {
//             if (!loader.LoadBinaryFromFile( &gltfModel, &err, &warn, filepath ))
//             {
//                 throw std::runtime_error("Failed to load glb file!");
//             }
//         }
//         if (filepath.find(".gltf") != TString::npos)
//         {
//             if (!loader.LoadASCIIFromFile( &gltfModel, &err, &warn, filepath ))
//             {
//                 throw std::runtime_error("Failed to load gltf file!");
//             }
//         }

//         if (!warn.empty())
//         {
//             std::cout << YELLOW << "[GLTFImporter] Warning: " << RESET << warn << std::endl;
//         }

//         if (!err.empty())
//         {
//             std::cerr << RED << "[GLTFImporter] Error: " << RESET << err << std::endl;
//         }

//         ModelLoadInfo info{};
//         info.Name = Utils::filenameFromPath( filepath );

//         auto path = TPath( filepath );

//         for (auto& texture : gltfModel.images)
//         {
//             TString texPath = path.parent_path().append(texture.uri).generic_string();

//             m_Textures.push_back( VyTexture::createFromFilepath( texPath ));

//             info.TextureCount++;
//         }

//         for (auto& scene : gltfModel.scenes)
//         {
//             for (size_t i = 0; i < scene.nodes.size(); i++)
//             {
//                 auto& node = gltfModel.nodes[ i ];

//                 U32 vertexOffset = 0;
//                 U32 indexOffset  = 0;

//                 for (auto& gltfPrimitive : gltfModel.meshes[ node.mesh ].primitives)
//                 {
//                     U32 vertexCount = 0;
//                     U32 indexCount  = 0;

//                     bool bHasIndices = gltfPrimitive.indices > -1;

//                     // Buffer pointers & data strides.
//                     const float* pPosBuffer      = nullptr;
//                     const float* pNormalBuffer   = nullptr;
//                     const float* pTangentBuffer  = nullptr;
//                     const float* pUV0Buffer      = nullptr;
//                     const float* pColor0Buffer   = nullptr;
                    
//                     int posByteStride;
//                     int normByteStride;
//                     int tangentByteStride;
//                     int uv0ByteStride;
//                     int color0ByteStride;

//                     // [ Position Attributes (REQUIRED) ]
//                     auto positionEntry = gltfPrimitive.attributes.find("POSITION");
//                     if(positionEntry != gltfPrimitive.attributes.end())
//                     {
//                         const auto& accessor   = gltfModel.accessors  [static_cast<size_t>(positionEntry->second)];
//                         const auto& bufferView = gltfModel.bufferViews[static_cast<size_t>(accessor.bufferView)  ];
//                         const auto& buffer     = gltfModel.buffers    [static_cast<size_t>(bufferView.buffer)    ];
                        
//                         pPosBuffer = reinterpret_cast<const float*>(&(buffer.data[accessor.byteOffset + bufferView.byteOffset]));
                        
//                         posByteStride = accessor.ByteStride(bufferView) 
//                             ? (accessor.ByteStride(bufferView) / sizeof(float)) 
//                             : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC3);

//                         // Set vertex count.
//                         vertexCount = static_cast<U32>(accessor.count);
//                     }
//                     else {
//                         VY_THROW_RUNTIME_ERROR("No Position attributes found.");
//                     }

//                     // [ Normal Attributes (optional) ]
//                     auto normEntry = gltfPrimitive.attributes.find("NORMAL");
//                     if(normEntry != gltfPrimitive.attributes.end())
//                     {
//                         const auto& accessor   = gltfModel.accessors  [static_cast<size_t>(normEntry->second)  ];
//                         const auto& bufferView = gltfModel.bufferViews[static_cast<size_t>(accessor.bufferView)];
//                         const auto& buffer     = gltfModel.buffers    [static_cast<size_t>(bufferView.buffer)  ];

//                         pNormalBuffer = reinterpret_cast<const float*>(&(buffer.data[bufferView.byteOffset + accessor.byteOffset]));

//                         normByteStride = accessor.ByteStride(bufferView) 
//                             ? (accessor.ByteStride(bufferView) / sizeof(float)) 
//                             : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC3); 
//                     }

//                     // [ Tangent Attributes (optional) ]
//                     auto tangentEntry = gltfPrimitive.attributes.find("TANGENT");
//                     if (tangentEntry != gltfPrimitive.attributes.end()) 
//                     {
//                         const auto& accessor   = gltfModel.accessors  [static_cast<size_t>(tangentEntry->second)];
//                         const auto& bufferView = gltfModel.bufferViews[static_cast<size_t>(accessor.bufferView) ];
//                         const auto& buffer     = gltfModel.buffers    [static_cast<size_t>(bufferView.buffer)   ];

//                         pTangentBuffer = reinterpret_cast<const float*>(&(buffer.data[bufferView.byteOffset + accessor.byteOffset]));
                    
//                         tangentByteStride = accessor.ByteStride(bufferView) 
//                             ? (accessor.ByteStride(bufferView) / sizeof(float)) 
//                             : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC4);
//                     }

//                     // [ UV 0 Attributes (optional) ]
//                     auto uvEntry0 = gltfPrimitive.attributes.find("TEXCOORD_0");
//                     if (uvEntry0 != gltfPrimitive.attributes.end()) 
//                     {
//                         const auto& accessor   = gltfModel.accessors  [static_cast<size_t>(uvEntry0->second)   ];
//                         const auto& bufferView = gltfModel.bufferViews[static_cast<size_t>(accessor.bufferView)];
//                         const auto& buffer     = gltfModel.buffers    [static_cast<size_t>(bufferView.buffer)  ];

//                         pUV0Buffer = reinterpret_cast<const float*>(&(buffer.data[bufferView.byteOffset + accessor.byteOffset]));
                    
//                         uv0ByteStride = accessor.ByteStride(bufferView) 
//                             ? (accessor.ByteStride(bufferView) / sizeof(float)) 
//                             : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC2); 
//                     }

//                     // [ Color 0 Attributes (optional) ]
//                     auto colorEntry = gltfPrimitive.attributes.find("COLOR_0");
//                     if (colorEntry != gltfPrimitive.attributes.end()) 
//                     {
//                         const auto& accessor   = gltfModel.accessors  [static_cast<size_t>(colorEntry->second) ];
//                         const auto& bufferView = gltfModel.bufferViews[static_cast<size_t>(accessor.bufferView)];
//                         const auto& buffer     = gltfModel.buffers    [static_cast<size_t>(bufferView.buffer)  ];

//                         pColor0Buffer = reinterpret_cast<const float*>(&(buffer.data[bufferView.byteOffset + accessor.byteOffset]));
                    
//                         color0ByteStride = accessor.ByteStride(bufferView) 
//                             ? (accessor.ByteStride(bufferView) / sizeof(float)) 
//                             : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC3); 
//                     }

//                     // [ Populate Vertex Data ]
//                     for (U32 vIndex = 0; vIndex < vertexCount; vIndex++)
//                     {
//                         VyVertex v{};
                        
//                         v.Position =                                 glm::make_vec3(&pPosBuffer    [vIndex * posByteStride    ]                );
//                         v.Normal   = glm::normalize(pNormalBuffer  ? glm::make_vec3(&pNormalBuffer [vIndex * normByteStride   ]) : Vec3{ 0.0f });
//                         v.Tangent  = glm::normalize(pTangentBuffer ? glm::make_vec4(&pTangentBuffer[vIndex * tangentByteStride]) : Vec4{ 0.0f });
//                         v.UV       = pUV0Buffer                    ? glm::make_vec2(&pUV0Buffer    [vIndex * uv0ByteStride    ]) : Vec2{ 0.0f };
//                         v.Color    = pColor0Buffer                 ? glm::make_vec3(&pColor0Buffer [vIndex * color0ByteStride ]) : Vec3{ 1.0f };

//                         m_Vertices.push_back( v );
//                     }

//                     // [ INDICES ]
//                     {
//                         if (bHasIndices) 
//                         {
//                             const auto& indexAccessor   = gltfModel.accessors  [gltfPrimitive.indices > -1 ? gltfPrimitive.indices : 0];
//                             const auto& indexBufferView = gltfModel.bufferViews[static_cast<size_t>(indexAccessor.bufferView)         ];
//                             const auto& indexBuffer     = gltfModel.buffers    [static_cast<size_t>(indexBufferView.buffer)           ];
                            
//                             // Set index count.
//                             indexCount = static_cast<U32>( indexAccessor.count );

//                             // Raw index data to cast.
//                             const void* pIndexData = &(indexBuffer.data[indexAccessor.byteOffset + indexBufferView.byteOffset]);

//                             // TODO: Raw indexing of the indices array would be considerably faster for index buffer here than push_back.
//                             switch (indexAccessor.componentType) 
//                             {
//                                 case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT: 
//                                 {
//                                     const U32* pCastData = static_cast<const U32*>(pIndexData);

//                                     for (U32 i = 0; i < indexCount; i++) 
//                                     {
//                                         m_Indices.push_back( pCastData[ i ] );
//                                     }

//                                 } break;
                                
//                                 case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT: 
//                                 {
//                                     const U16* pCastData = static_cast<const U16*>(pIndexData);

//                                     for (U32 i = 0; i < indexCount; i++) 
//                                     {
//                                         m_Indices.push_back( static_cast<U32>(pCastData[ i ]) );
//                                     }
                                    
//                                 } break;
                                
//                                 case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE: 
//                                 {
//                                     const U8* pCastData = static_cast<const U8*>(pIndexData);

//                                     for (U32 i = 0; i < indexCount; i++) 
//                                     {
//                                         m_Indices.push_back( static_cast<U32>(pCastData[ i ]) );
//                                     }
                                    
//                                 } break;
                                
//                                 default:
//                                 {
//                                     VY_THROW_RUNTIME_ERROR("Attempted to load model Indices with an unsupported data type.");
//                                 }
//                             }
//                         }
//                     }

//                     // [ MATERIALS ]
//                     VyPBRMaterial material{};
//                     {
//                         if (gltfPrimitive.material != -1)
//                         {
//                             tinygltf::Material& gltfMaterial = gltfModel.materials[ gltfPrimitive.material ];

//                             // [ PBR Metallic Roughness Factors ]
//                             const auto& pbr = gltfMaterial.pbrMetallicRoughness;

//                             // [ PBR - Albedo ]
//                             if (pbr.baseColorTexture.index != -1)
//                             {
//                                 U32 textureIndex = pbr.baseColorTexture.index;
//                                 U32 imageIndex   = gltfModel.textures[textureIndex].source;

//                                 material.AlbedoMap = m_Textures[ imageIndex ];
//                             }
//                             else {
//                                 material.AlbedoMap = m_DefaultWhite;
//                             }

//                             // [ PBR - MetallicRoughness ]
//                             if (pbr.metallicRoughnessTexture.index != -1)
//                             {
//                                 U32 textureIndex = pbr.metallicRoughnessTexture.index;
//                                 U32 imageIndex   = gltfModel.textures[textureIndex].source;
                                
//                                 material.MetallicRoughnessMap        = m_Textures[imageIndex];
//                                 material.UseMetallicRoughnessTexture = true;

//                                 // [ ARM ]
//                                 if (gltfMaterial.occlusionTexture.index == pbr.metallicRoughnessTexture.index)
//                                 {
//                                     material.UseOcclusionRoughnessMetallicTexture = true;
//                                 }
//                             }
//                             else {
//                                 material.MetallicRoughnessMap = m_DefaultNormal;
//                             }

//                             // [ Ambient Occlusion ]
//                             if (gltfMaterial.occlusionTexture.index != -1)
//                             {
//                                 U32 textureIndex = gltfMaterial.occlusionTexture.index;
//                                 U32 imageIndex   = gltfModel.textures[textureIndex].source;

//                                 material.AOMap = m_Textures[imageIndex];
//                             }
//                             else {
//                                 material.AOMap = m_DefaultNormal;
//                             }

//                             // [ Emissive ]
//                             if (gltfMaterial.emissiveTexture.index != -1)
//                             {
//                                 U32 textureIndex = gltfMaterial.emissiveTexture.index;
//                                 U32 imageIndex   = gltfModel.textures[textureIndex].source;

//                                 material.EmissiveMap = m_Textures[imageIndex];
//                             }
//                             else {
//                                 material.EmissiveMap = m_DefaultWhite;
//                             }

//                             // [ Normal ]
//                             if (gltfMaterial.normalTexture.index != -1)
//                             {
//                                 U32 textureIndex = gltfMaterial.normalTexture.index;
//                                 U32 imageIndex   = gltfModel.textures[textureIndex].source;
                                
//                                 material.NormalMap = m_Textures[imageIndex];
//                             }
//                             else {
//                                 material.NormalMap = m_DefaultNormal;
//                             }
//                         }

//                         // [ Fallback ]
//                         else {
//                             material.AlbedoMap            = m_DefaultWhite;
//                             material.MetallicRoughnessMap = m_DefaultNormal;
//                             material.EmissiveMap          = m_DefaultWhite;
//                             material.AOMap                = m_DefaultNormal;
//                             material.NormalMap            = m_DefaultNormal;
//                         }
//                     }

//                     createDescriptorSet(material, materialSetLayout, descriptorPool);

//                     VyPrimitive primitive{};
//                     {
//                         primitive.FirstIndex  = indexOffset;
//                         primitive.IndexCount  = indexCount;
//                         primitive.FirstVertex = vertexOffset;
//                         primitive.VertexCount = vertexCount;
//                         primitive.Material    = material;
//                     }
                    
//                     m_Primitives.push_back( primitive );

//                     vertexOffset += vertexCount;
//                     indexOffset  += indexCount;

//                     {
//                         info.VertexCount = vertexCount;
//                         info.IndexCount  = indexCount;

//                         info.MeshCount++;
//                     }

//                 } // [ End of mesh loop ]

//                 info.NodeCount++;

//             } // [ End of node loop ]

//             createVertexBuffer( m_Vertices );
//             createIndexBuffer ( m_Indices  );

//             info.SceneCount++;

//         } // [ End of scene loop ]

// 		std::stringstream ss;
//         ss  << "\n--------------------------------------------------------------------------" << '\n'
// 			<< "[" << CYAN "VyModel Load Stats" RESET "] " << '\n'
//             << " - Name       : " << info.Name             << '\n';
//         if (info.SceneCount > 0)
//         {
//             ss  << " - Scenes     : " << info.SceneCount       << '\n';
//         }
//         if (info.NodeCount > 0)
//         {
//             ss  << " - Nodes      : " << info.NodeCount        << '\n';
//         }
//         ss  << " - Meshes     : " << info.MeshCount        << '\n'
//             << " - Vertices   : " << info.VertexCount      << '\n'
// 			<< " - Indices    : " << info.IndexCount       << '\n'
//             << " - Textures   : " << info.TextureCount     << '\n'
// 			<< "--------------------------------------------------------------------------"   << '\n'
//         ;

//         std::cout << ss.str() << std::endl;
//     }


//     void VyModel::createDescriptorSet(
//         VyPBRMaterial&         material, 
//         VyDescriptorSetLayout& materialSetLayout, 
//         VyDescriptorPool&      descriptorPool)
//     {
//         VkDescriptorImageInfo albedoInfo   = material.AlbedoMap           ->descriptorImageInfo();
//         VkDescriptorImageInfo normalInfo   = material.NormalMap           ->descriptorImageInfo();
//         VkDescriptorImageInfo mrInfo       = material.MetallicRoughnessMap->descriptorImageInfo();
//         VkDescriptorImageInfo aoInfo       = material.AOMap               ->descriptorImageInfo();
//         VkDescriptorImageInfo emissiveInfo = material.EmissiveMap         ->descriptorImageInfo();

//         VyDescriptorWriter( materialSetLayout, descriptorPool )
//             .writeImage( 0, &albedoInfo   )
//             .writeImage( 1, &normalInfo   )
//             .writeImage( 2, &mrInfo       )
//             .writeImage( 3, &aoInfo       )
//             .writeImage( 4, &emissiveInfo )
//             .build( material.DescriptorSet );
//     }

//     // =====================================================================================================================

//     VyModel::~VyModel()
//     {
//     }

//     // =====================================================================================================================

//     void VyModel::bind(VkCommandBuffer cmdBuffer)
//     {
//         VkBuffer     buffers[] = { m_VertexBuffer->handle() };
//         VkDeviceSize offsets[] = { 0 };

//         vkCmdBindVertexBuffers(cmdBuffer, 0, 1, buffers, offsets);
        
//         if (m_HasIndexBuffer) 
//         {
//             vkCmdBindIndexBuffer(cmdBuffer, m_IndexBuffer->handle(), 0, VK_INDEX_TYPE_UINT32);
//         }
//     }

//     // =====================================================================================================================

//     void VyModel::draw(VkCommandBuffer cmdBuffer, VkPipelineLayout pipelineLayout, int setCount, bool bRenderMaterial)
//     {
//         for (auto& primitive : m_Primitives)
//         {
//             if (m_HasIndexBuffer)
//             {
//                 if (bRenderMaterial)
//                 {
//                     TVector<VkDescriptorSet> sets = { 
//                         primitive.Material.DescriptorSet 
//                     };

//                     vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, 
//                         pipelineLayout,
//                         setCount, 
//                         sets.size(), sets.data(), 
//                         0, nullptr
//                     );
//                 }

//                 vkCmdDrawIndexed(cmdBuffer, primitive.IndexCount, 1, primitive.FirstIndex, primitive.FirstVertex, 0);
//             }
//             else
//             {
//                 vkCmdDraw(cmdBuffer, primitive.VertexCount, 1, 0, 0);
//             }
//         }
//     }

//     // =====================================================================================================================

//     void VyModel::createVertexBuffer(const TVector<VyVertex>& vertices) 
//     {
//         U32 vertexCount = static_cast<U32>( vertices.size() );

//         VY_ASSERT(vertexCount >= 3, "Vertex count must be at least 3");
        
//         U32 vertexSize = sizeof(vertices[0]);

//         m_VertexBuffer = MakeUnique<VyBuffer>( VyBuffer::vertexBuffer( "model", vertexSize, vertexCount ) );
        
//         m_VertexBuffer->upload( vertices );
//     }

//     // =====================================================================================================================

//     void VyModel::createIndexBuffer(const TVector<U32>& indices) 
//     {
//         U32 indexCount   = static_cast<U32>( indices.size() );
//         m_HasIndexBuffer = indexCount > 0;
        
//         if (!m_HasIndexBuffer) { return; }

//         U32 indexSize = sizeof(indices[0]);

//         m_IndexBuffer = MakeUnique<VyBuffer>( VyBuffer::indexBuffer( "model", indexSize, indexCount ) );
        
//         m_IndexBuffer->upload( indices );
//     }

//     // =====================================================================================================================

// }