// #include <VyEngine/GFX/Resources/Model/UFBXImporter.h>

// #include <VyEngine/VK/Descriptors/Descriptors.h>
// #include <VyEngine/VK/Context.h>

// #include <VyLib/Include/ufbx/ufbx.h>

// #include <VyLib/STL/Path.h>

// extern Vy::Shared<Vy::VyBuffer> g_DummyBuffer;

// namespace Vy
// {
//     UFBXImporter::UFBXImporter(const TString& filePath)
//         : m_Filepath(filePath) 
//     {
//         m_Path = Utils::getPathWithoutFileName(filePath);
//     }


//     bool UFBXImporter::loadModel(const U32 instanceCount) 
//     {
//         ufbx_load_opts loadOptions{};
//         {
//             loadOptions.ignore_animation              = true;
//             loadOptions.load_external_files           = true;
//             loadOptions.ignore_missing_external_files = true;
//             loadOptions.generate_missing_normals      = true;
//             loadOptions.target_axes                   = ufbx_axes_left_handed_y_up;
//             loadOptions.target_unit_meters = 1.0f;
//         }

//         // load raw data of the file (can be fbx or obj)
//         ufbx_error ufbxError;

//         m_ModelScene = ufbx_load_file(m_Filepath.data(), &loadOptions, &ufbxError);

//         if (m_ModelScene == nullptr) 
//         {
//             char errorBuffer[512];
//             ufbx_format_error(errorBuffer, sizeof(errorBuffer), &ufbxError);
//             VY_ERROR("UFBXImporter::load error: file: {0}, error: {1}", m_Filepath, errorBuffer);
//             return false;
//         }

//         if (!m_ModelScene->meshes.count) 
//         {
//             VY_ERROR("ufbxBuilder::load: no meshes found in {0}", m_Filepath);
//             return false;
//         }

//         loadSkeletons();
//         loadMaterials();

//         m_InstanceCount = instanceCount;
//         for (m_InstanceIndex = 0; m_InstanceIndex < m_InstanceCount; ++m_InstanceIndex) 
//         {
//             loadNode(m_ModelScene->root_node);
//         }

//         ufbx_free_scene(m_ModelScene);
//         return true;
//     }


//     void UFBXImporter::loadMaterials() 
//     {
//         U32 numMaterials = static_cast<U32>(m_ModelScene->materials.count);
//         Materials.resize( numMaterials );

//         for (U32 materialIndex = 0; materialIndex < numMaterials; ++materialIndex) 
//         {
//             const ufbx_material* pFBXMaterial = m_ModelScene->materials[materialIndex];
//             // PrintProperties(pFBXMaterial);

//             loadMaterial(pFBXMaterial, UFBX_MATERIAL_PBR_BASE_COLOR,      materialIndex);
//             loadMaterial(pFBXMaterial, UFBX_MATERIAL_PBR_ROUGHNESS,       materialIndex);
//             loadMaterial(pFBXMaterial, UFBX_MATERIAL_PBR_METALNESS,       materialIndex);
//             loadMaterial(pFBXMaterial, UFBX_MATERIAL_PBR_NORMAL_MAP,      materialIndex);
//             loadMaterial(pFBXMaterial, UFBX_MATERIAL_PBR_EMISSION_COLOR,  materialIndex);
//             loadMaterial(pFBXMaterial, UFBX_MATERIAL_PBR_EMISSION_FACTOR, materialIndex);

//             m_MaterialNameToIndex[ pFBXMaterial->name.data ] = materialIndex;
//         }
//     }


//     void UFBXImporter::loadMaterial(const ufbx_material* pFBXMaterial, ufbx_material_pbr_map materialProperty, int materialIndex) 
//     {
//         VyMaterialOLD&                        material         = Materials[ materialIndex ];
//         VyMaterialOLD::PBRMaterial&           pbrMaterial      = material.PbrMaterial;
//         VyMaterialOLD::MaterialTexturesArray& materialTextures = material.MaterialTextures;

//         switch (materialProperty) 
//         {
//             // aka albedo aka diffuse color
//             case UFBX_MATERIAL_PBR_BASE_COLOR: 
//             {
//                 ufbx_material_map const& materialMap = pFBXMaterial->pbr.base_color;
                
//                 if (materialMap.has_value) 
//                 {
//                     const ufbx_material_map& baseFactorMaterialMap = pFBXMaterial->pbr.base_factor;
//                     float baseFactor = baseFactorMaterialMap.has_value ? (float)baseFactorMaterialMap.value_real : 1.0f;
                    
//                     if (materialMap.texture) 
//                     {
//                         if (auto texture = loadTexture(materialMap, VyTexture::USE_SRGB)) 
//                         {
//                             materialTextures[VyMaterialOLD::DIFFUSE_MAP_INDEX] = texture;
//                             pbrMaterial.Features |= VyMaterialOLD::HAS_DIFFUSE_MAP;
//                             pbrMaterial.DiffuseColor.r = baseFactor;
//                             pbrMaterial.DiffuseColor.g = baseFactor;
//                             pbrMaterial.DiffuseColor.b = baseFactor;
//                             pbrMaterial.DiffuseColor.a = baseFactor;
//                         }
//                     } else {
//                         pbrMaterial.DiffuseColor.r = (float)materialMap.value_vec4.x * baseFactor;
//                         pbrMaterial.DiffuseColor.g = (float)materialMap.value_vec4.y * baseFactor;
//                         pbrMaterial.DiffuseColor.b = (float)materialMap.value_vec4.z * baseFactor;
//                         pbrMaterial.DiffuseColor.a = (float)materialMap.value_vec4.w * baseFactor;
//                     }
//                 }
            
//             } break;
            
//             case UFBX_MATERIAL_PBR_ROUGHNESS: 
//             {
//                 ufbx_material_map const& materialMap = pFBXMaterial->pbr.roughness;
                
//                 if (materialMap.has_value) 
//                 {
//                     if (materialMap.texture) 
//                     {
//                         if (auto texture = loadTexture(materialMap, VyTexture::USE_UNORM)) 
//                         {
//                             materialTextures[ VyMaterialOLD::ROUGHNESS_MAP_INDEX ] = texture;
                            
//                             pbrMaterial.Features |= VyMaterialOLD::HAS_ROUGHNESS_MAP;
//                         }
//                     } 
//                     else // Constant material property
//                     {
//                         pbrMaterial.Roughness = (float)materialMap.value_real;
//                     }
//                 }

//             } break;

//             case UFBX_MATERIAL_PBR_METALNESS: 
//             {
//                 ufbx_material_map const& materialMap = pFBXMaterial->pbr.metalness;

//                 if (materialMap.has_value) 
//                 {
//                     if (materialMap.texture) 
//                     {
//                         if (auto texture = loadTexture(materialMap, VyTexture::USE_UNORM)) 
//                         {
//                             materialTextures[ VyMaterialOLD::METALLIC_MAP_INDEX ] = texture;

//                             pbrMaterial.Features |= VyMaterialOLD::HAS_METALLIC_MAP;
//                         }
//                     } 
//                     else  // Constant material property
//                     {
//                         pbrMaterial.Metallic = (float)materialMap.value_real;
//                     }
//                 }

//             } break;

//             case UFBX_MATERIAL_PBR_NORMAL_MAP: 
//             {
//                 ufbx_material_map const& materialMap = pFBXMaterial->pbr.normal_map;

//                 if (materialMap.texture) 
//                 {
//                     if (auto texture = loadTexture(materialMap, VyTexture::USE_UNORM)) 
//                     {
//                         materialTextures[ VyMaterialOLD::NORMAL_MAP_INDEX ] = texture;

//                         pbrMaterial.Features |= VyMaterialOLD::HAS_NORMAL_MAP;
//                     }
//                 }

//             } break;

//             case UFBX_MATERIAL_PBR_EMISSION_COLOR: 
//             {
//                 ufbx_material_map const& materialMap = pFBXMaterial->pbr.emission_color;
            
//                 if (materialMap.texture) 
//                 {
//                     if (auto texture = loadTexture(materialMap, VyTexture::USE_SRGB)) 
//                     {
//                         materialTextures[ VyMaterialOLD::EMISSIVE_MAP_INDEX ] = texture;

//                         pbrMaterial.Features |= VyMaterialOLD::HAS_EMISSIVE_MAP;
//                         pbrMaterial.EmissiveColor = Vec3(1.0f);
//                     }
//                 } 
//                 else {
//                     Vec3 emissiveColor(
//                         materialMap.value_vec3.x, 
//                         materialMap.value_vec3.y, 
//                         materialMap.value_vec3.z
//                     );

//                     pbrMaterial.EmissiveColor = emissiveColor;
//                 }

//             } break;

//             case UFBX_MATERIAL_PBR_EMISSION_FACTOR: 
//             {
//                 ufbx_material_map const& materialMap = pFBXMaterial->pbr.emission_factor;
            
//                 if (materialMap.has_value) 
//                 {
//                     pbrMaterial.EmissiveStrength = (float)materialMap.value_real;
//                 }

//             } break;

//             default: 
//             {
//                 VY_ASSERT(false, "VyMaterialOLD Property not recognized");
            
//             } break;
//         }
//     }


//     Shared<VyTexture> UFBXImporter::loadTexture(ufbx_material_map const& materialMap, bool useSRGB) 
//     {
//         Shared<VyTexture> texture;
        
//         auto createTexture = [&](ufbx_string const& str) 
//         {
//             TString filepath( str.data );

//             if ( Utils::fileExists (filepath) && 
//                 !Utils::isDirectory(filepath)) 
//             {
//                 texture = VyTexture::createFromFile( filepath, useSRGB );

//                 if (texture) 
//                 {
//                     // m_textures.push_back(texture);
//                     return true;
//                 }
//             }
//             return false;
//         };

//         if (createTexture(materialMap.texture->filename)) 
//         {
//             return texture;
//         }
        
//         if (createTexture(materialMap.texture->absolute_filename)) 
//         {
//             return texture;
//         }
        
//         if (createTexture(materialMap.texture->relative_filename)) 
//         {
//             return texture;
//         }
        
//         if (!texture) 
//         {
//             TString textureName = materialMap.texture->filename.data;
//             textureName         = textureName.substr(textureName.find_last_of("\\") + 1);
//             // m_Filepath;
//             TString texturepath( Utils::getPathWithoutFileName(m_Filepath) + textureName );

//             texture = VyTexture::createFromFile( texturepath, useSRGB );
//             if (texture) 
//             {
//                 // m_textures.push_back(texture);
//                 return texture;
//             }
//         }

//         TString filepath(materialMap.texture->filename.data);
//         VY_ERROR("UFBXImporter::loadTexture(): file '{0}' not found", filepath);
//         return nullptr;
//     }


//     void UFBXImporter::loadNode(const ufbx_node* pFBXNode) 
//     {
//         //ufbx_matrix transform = ufbx_matrix_mul(&parentTransform, &pFBXNode->node_to_parent);

//         //nodes.push_back(Node{});
//         //nodes.back().parentNode = parentNode;
//         //nodes.back().transform  = ufbxToglm(pFBXNode->node_to_parent);

//     //   I32 thisID                          = (I32)nodes.size() - 1;
//         //m_nodeMap[pFBXNode->name.data] = thisID;
//         //if (parentNode >= 0) {
//         //	nodes[parentNode].children.push_back(thisID);
//         //}

//         ufbx_mesh* pFBXMesh = pFBXNode->mesh;
//         if (pFBXMesh) 
//         {
//             U32 meshCount = static_cast<U32>(pFBXMesh->material_parts.count);

//             if (meshCount > 0) 
//             {
//                 Vertices.clear();
//                 Indices .clear();
//                 Meshes  .clear();

//                 Meshes.resize( meshCount );
                
//                 for (U32 meshIndex = 0; meshIndex < meshCount; ++meshIndex) 
//                 {
//                     loadMesh( pFBXNode, meshIndex );

//                     TString materialName  = pFBXNode->mesh->materials.data[ meshIndex ]->name.data;
//                     U32     materialIndex = m_MaterialNameToIndex[materialName];
                    
//                     assignMaterial( Meshes[ meshIndex ], materialIndex );
//                 }

//                 if (m_FBXNoTangents)  // at least one mesh did not have tangents
//                 {
//                     calculateTangents();
//                 }
//             }
//         }

//         U32 childCount = static_cast<U32>(pFBXNode->children.count);
        
//         for (U32 childIndex = 0; childIndex < childCount; ++childIndex) 
//         {
//             loadNode( pFBXNode->children[ childIndex ] );
//         }
//     }


//     void UFBXImporter::loadMesh(const ufbx_node* pFBXNode, const U32 meshIndex) 
//     {
//         ufbx_mesh*            pFBXMesh    = pFBXNode->mesh;
//         const ufbx_mesh_part& fbxMeshPart = pFBXNode->mesh->material_parts[ meshIndex ];
//         size_t                faceCount   = fbxMeshPart.num_faces;

//         if (!fbxMeshPart.num_triangles) 
//         {
//             VY_ERROR("UFBXImporter::loadMesh: only triangle meshes are supported!");
//             return;
//         }

//         size_t numVerticesBefore = Vertices.size();
//         size_t numIndicesBefore  = Indices .size();

//         VyMesh& mesh = Meshes[ meshIndex ];
//         {
//             mesh.FirstVertex   = static_cast<U32>(numVerticesBefore);
//             mesh.FirstIndex    = static_cast<U32>(numIndicesBefore );
//             mesh.IndexCount    = 0;
//             mesh.InstanceCount = m_InstanceCount;
//         }

//         ufbx_material_map& baseColorMap = pFBXNode->materials[ meshIndex ]->pbr.base_color;

//         Vec4 diffuseColor = baseColorMap.has_value 
//             ? Vec4(
//                 baseColorMap.value_vec4.x, 
//                 baseColorMap.value_vec4.y, 
//                 baseColorMap.value_vec4.z, 
//                 baseColorMap.value_vec4.w
//             )
//             : Vec4(1.0f);

//     #pragma region Vertices

//         bool hasTangents            = pFBXMesh->vertex_tangent.exists;
//         bool hasUVs                 = pFBXMesh->uv_sets.count;
//         bool hasVertexColors        = pFBXMesh->vertex_color.exists;
        
//         ufbx_skin_deformer* pFBXSkin = nullptr;
        
//         if (pFBXMesh->skin_deformers.count) 
//         {
//             pFBXSkin = pFBXMesh->skin_deformers.data[0];
//         }

//         m_FBXNoTangents = m_FBXNoTangents || (!hasTangents);
        
//         for (size_t fbxFaceIndex = 0; fbxFaceIndex < faceCount; ++fbxFaceIndex) 
//         {
//             ufbx_face& fbxFace        = pFBXMesh->faces[fbxMeshPart.face_indices.data[fbxFaceIndex]];
//             size_t triangleIndexCount = pFBXMesh->max_face_triangles * 3;
//             TVector<U32> verticesPerFaceIndexBuffer(triangleIndexCount);
//             size_t triangleCount = ufbx_triangulate_face(verticesPerFaceIndexBuffer.data(), triangleIndexCount, pFBXMesh, fbxFace);
//             size_t vertexCountPerFace = triangleCount * 3;

//             for (U32 vertexPerFace = 0; vertexPerFace < vertexCountPerFace; ++vertexPerFace) 
//             {
//                 U32 vertexPerFaceIndex = verticesPerFaceIndexBuffer[vertexPerFace];

//                 Vertex vertex{};

//                 U32 fbxVertexIndex     = pFBXMesh->vertex_indices[vertexPerFaceIndex];
//                 ufbx_vec3& positionFbx = pFBXMesh->vertices[fbxVertexIndex];
//                 vertex.Position        = Vec3(positionFbx.x, positionFbx.y, positionFbx.z);

//                 U32 fbxNormalIndex = pFBXMesh->vertex_normal.indices[vertexPerFaceIndex];
//                 VY_ASSERT(fbxNormalIndex < pFBXMesh->vertex_normal.values.count, "loadMesh: memory violation normals");
                
//                 ufbx_vec3& normalFbx = pFBXMesh->vertex_normal.values.data[fbxNormalIndex];
//                 vertex.Normal        = Vec3(normalFbx.x, normalFbx.y, normalFbx.z);

//                 if (hasTangents) 
//                 {
//                     U32 fbxTangentIndex = pFBXMesh->vertex_tangent.indices[vertexPerFaceIndex];
//                     VY_ASSERT(fbxTangentIndex < pFBXMesh->vertex_tangent.values.count, "loadMesh: memory violation tangents");
                    
//                     ufbx_vec3& tangentFbx = pFBXMesh->vertex_tangent.values.data[fbxTangentIndex];
//                     vertex.Tangent        = Vec3(tangentFbx.x, tangentFbx.y, tangentFbx.z);
//                 }

//                 if (hasUVs) 
//                 {
//                     U32 fbxUVIndex = pFBXMesh->vertex_uv.indices[vertexPerFaceIndex];
//                     VY_ASSERT(fbxUVIndex < pFBXMesh->vertex_uv.values.count, "loadMesh: memory violation uv coordinates");
                    
//                     ufbx_vec2& uvFbx = pFBXMesh->vertex_uv.values.data[fbxUVIndex];
//                     vertex.UV        = Vec2(uvFbx.x, uvFbx.y);
//                 }

//                 if (hasVertexColors) 
//                 {
//                     U32 fbxColorIndex   = pFBXMesh->vertex_color.indices[vertexPerFaceIndex];
//                     ufbx_vec4& colorFbx = pFBXMesh->vertex_color.values.data[fbxColorIndex];

//                     // convert from sRGB to linear
//                     Vec3 linearColor = glm::pow(Vec3(colorFbx.x, colorFbx.y, colorFbx.z), Vec3(2.2f));
//                     Vec4 vertexColor(linearColor.x, linearColor.y, linearColor.z, colorFbx.w);
//                     vertex.Color = vertexColor * diffuseColor;
//                 } 
//                 else {
//                     vertex.Color = diffuseColor;
//                 }

//                 if (pFBXSkin) 
//                 {
//                     ufbx_skin_vertex skinVertex = pFBXSkin->vertices[fbxVertexIndex];
//                     size_t numWeights = skinVertex.num_weights < MAX_JOINT_INFLUENCE ? skinVertex.num_weights : MAX_JOINT_INFLUENCE;

//                     for (size_t weightIndex = 0; weightIndex < numWeights; ++weightIndex) 
//                     {
//                         ufbx_skin_weight skinWeight = pFBXSkin->weights.data[skinVertex.weight_begin + weightIndex];
//                         //int jointIndex              = skinWeight.cluster_index;
//                         //float weight                = (float)skinWeight.weight;
//                         vertex.JointIds[ weightIndex ] = skinWeight.cluster_index;
//                         vertex.Weights [ weightIndex ] = (float)skinWeight.weight;

//                         //switch (weightIndex) {
//                         //	case 0:
//                         //		vertex.jointIds.x = jointIndex;
//                         //		vertex.weights.x  = weight;
//                         //		break;
//                         //	case 1:
//                         //		vertex.jointIds.y = jointIndex;
//                         //		vertex.weights.y  = weight;
//                         //		break;
//                         //	case 2:
//                         //		vertex.jointIds.z = jointIndex;
//                         //		vertex.weights.z  = weight;
//                         //		break;
//                         //	case 3:
//                         //		vertex.jointIds.w = jointIndex;
//                         //		vertex.weights.w  = weight;
//                         //		break;
//                         //	default:
//                         //		break;
//                         //}
//                     }

//                     // normalize weights
//                     float weightSum = vertex.Weights.x + vertex.Weights.y + vertex.Weights.z + vertex.Weights.w;
                    
//                     if (weightSum > std::numeric_limits<float>::epsilon()) 
//                     {
//                         vertex.Weights = vertex.Weights / weightSum;
//                     }
//                 }

//                 Vertices.push_back( vertex );
//             }
//         }
//     #pragma endregion

//     #pragma region Indices

//         U32 meshAllVertices = static_cast<U32>(Vertices.size() - numVerticesBefore);

//         ufbx_vertex_stream stream;
//         {
//             stream.data         = &Vertices[numVerticesBefore];
//             stream.vertex_count = meshAllVertices;
//             stream.vertex_size  = sizeof(Vertex);
//         }

//         Indices.resize( numIndicesBefore + meshAllVertices );

//         ufbx_error ufbxError;
//         size_t vertexCount = ufbx_generate_indices(&stream, 1, &Indices[numIndicesBefore], meshAllVertices, nullptr, &ufbxError);

//         if (ufbxError.type != UFBX_ERROR_NONE) 
//         {
//             char errorBuffer[512];
//             ufbx_format_error(errorBuffer, sizeof(errorBuffer), &ufbxError);

//             VY_ERROR(
//                 "ufbxBuilder: creation of index buffer failed, file: {0}, error: {1},  node: {2}",
//                 m_Filepath,
//                 errorBuffer,
//                 pFBXNode->name.data
//             );
//         }

//         Vertices.resize( numVerticesBefore + vertexCount );

//         mesh.VertexCount = static_cast<U32>(vertexCount);
//         mesh.IndexCount  = meshAllVertices;
//     #pragma endregion
//     }

    
//     void UFBXImporter::assignMaterial(VyMesh& mesh, const int materialIndex) 
//     {
//         // material
//         {
//             if (!(static_cast<size_t>(materialIndex) < Materials.size())) 
//             {
//                 VY_ERROR("assignMaterial: materialIndex must be less than Materials.size()");
//             }

//             VyMaterialOLD& material = mesh.Material;

//             // material
//             if (materialIndex != -1) 
//             {
//                 material = Materials[ materialIndex ];
//                 // material.materialTextures = m_materialTextures[materialIndex];
//             }

//             // create material descriptor
//             material.MaterialDescriptor = MakeShared<MaterialDescriptor>(mesh.Material, mesh.Material.MaterialTextures);
//         }
//         VY_INFO("VyMaterialOLD assigned (ufbx): material index {0}", materialIndex);

//         if (SkeletonUbo) 
//         {
//             mesh.SkeletonBuffer = SkeletonUbo;
//         } 
//         else {
//             mesh.SkeletonBuffer = g_DummyBuffer;
//         }

//         Unique<VyDescriptorSetLayout> localDescriptorSetLayout = VyDescriptorSetLayout::Builder{}
//             .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
//             .buildPtr();
        
//         VkDescriptorBufferInfo bufferInfo = mesh.SkeletonBuffer->descriptorBufferInfo();

//         VyDescriptorWriter( *localDescriptorSetLayout, *VyContext::globalPool() )
//             .writeBuffer(0, &bufferInfo)
//             .build(mesh.SkeletonDescriptorSet);
//     }


//     void UFBXImporter::calculateTangents() 
//     {
//         if (Indices.size()) 
//         {
//             calculateTangentsFromIndexBuffer( Indices );
//         } 
//         else {
//             U32 vertexCount = static_cast<U32>(Vertices.size());
            
//             if (vertexCount) 
//             {
//                 TVector<U32> indices;
//                 indices.resize( vertexCount );
                
//                 for (U32 i = 0; i < vertexCount; i++) 
//                 {
//                     indices[i] = i;
//                 }
                
//                 calculateTangentsFromIndexBuffer( indices );
//             }
//         }
//     }

    
//     void UFBXImporter::calculateTangentsFromIndexBuffer(const TVector<U32>& indices) 
//     {
//         U32  count        = 0;
//         U32  vertexIndex1 = 0;
//         U32  vertexIndex2 = 0;
//         U32  vertexIndex3 = 0;
//         Vec3 position1    = Vec3{0.0f};
//         Vec3 position2    = Vec3{0.0f};
//         Vec3 position3    = Vec3{0.0f};
//         Vec2 uv1          = Vec2{0.0f};
//         Vec2 uv2          = Vec2{0.0f};
//         Vec2 uv3          = Vec2{0.0f};

//         for (U32 index : indices) 
//         {
//             auto& vertex = Vertices[index];

//             switch (count) {
//                 case 0:
//                     position1    = vertex.Position;
//                     uv1          = vertex.UV;
//                     vertexIndex1 = index;
//                     break;
//                 case 1:
//                     position2    = vertex.Position;
//                     uv2          = vertex.UV;
//                     vertexIndex2 = index;
//                     break;
//                 case 2:
//                     position3    = vertex.Position;
//                     uv3          = vertex.UV;
//                     vertexIndex3 = index;

//                     Vec3 edge1    = position2 - position1;
//                     Vec3 edge2    = position3 - position1;
//                     Vec2 deltaUV1 = uv2 - uv1;
//                     Vec2 deltaUV2 = uv3 - uv1;

//                     float dU1 = deltaUV1.x;
//                     float dU2 = deltaUV2.x;
//                     float dV1 = deltaUV1.y;
//                     float dV2 = deltaUV2.y;
//                     float E1x = edge1.x;
//                     float E2x = edge2.x;
//                     float E1y = edge1.y;
//                     float E2y = edge2.y;
//                     float E1z = edge1.z;
//                     float E2z = edge2.z;

//                     float factor;
//                     if ((dU1 * dV2 - dU2 * dV1) > std::numeric_limits<float>::epsilon()) 
//                     {
//                         factor = 1.0f / (dU1 * dV2 - dU2 * dV1);
//                     } else {
//                         factor = 100000.0f;
//                     }

//                     Vec3 tangent;

//                     tangent.x = factor * (dV2 * E1x - dV1 * E2x);
//                     tangent.y = factor * (dV2 * E1y - dV1 * E2y);
//                     tangent.z = factor * (dV2 * E1z - dV1 * E2z);
//                     if (tangent.x == 0.0f && tangent.y == 0.0f && tangent.z == 0.0f) 
//                     {
//                         tangent = Vec3(1.0f, 0.0f, 0.0f);
//                     }

//                     Vertices[vertexIndex1].Tangent = tangent;
//                     Vertices[vertexIndex2].Tangent = tangent;
//                     Vertices[vertexIndex3].Tangent = tangent;

//                     break;
//             }

//             count = (count + 1) % 3;
//         }
//     }

//     bool UFBXImporter::loadAnimations() {
//         ufbx_load_opts loadOptions{};
//         loadOptions.ignore_geometry               = true;
//         loadOptions.load_external_files           = true;
//         loadOptions.ignore_missing_external_files = true;
//         loadOptions.generate_missing_normals      = true;
//         loadOptions.target_axes                   = ufbx_axes_left_handed_y_up;
//         loadOptions.target_unit_meters            = 1.0f;

//         // load raw data of the file (can be fbx or obj)
//         ufbx_error ufbxError;

//         m_ModelScene = ufbx_load_file(m_Filepath.data(), &loadOptions, &ufbxError);

//         if (m_ModelScene == nullptr) 
//         {
//             char errorBuffer[512];
//             ufbx_format_error(errorBuffer, sizeof(errorBuffer), &ufbxError);
//             VY_ERROR("UFBXImporter::load error: file: {0}, error: {1}", m_Filepath, errorBuffer);
//             return false;
//         }

//         if (!m_ModelScene->meshes.count) {
//             VY_ERROR("ufbxBuilder::load: no meshes found in {0}", m_Filepath);
//             return false;
//         }

//         loadSkeletons();
//         loadAnimationClips();

//         ufbx_free_scene(m_ModelScene);
//         return true;
//     }

//     bool UFBXImporter::addAnimation() 
//     {
//         return false;
//     }

//     void UFBXImporter::loadSkeletons() 
//     {
//         U32 boneCount = 0;

//         if (m_ModelScene->bones.count == 0) 
//         {
//             return;
//         }

//         Skeleton = MakeShared<VySkeleton>();

//         for (size_t nodeIndex = 0; nodeIndex < m_ModelScene->nodes.count; ++nodeIndex) {
//             ufbx_node* pNode = m_ModelScene->nodes.data[nodeIndex];
            
//             Bone bone{};
//             {
//                 bone.Name            = pNode->name.data;
//                 bone.ParentIndex     = pNode->parent ? Skeleton->BoneMap.at(pNode->parent->name.data) : -1;
//                 bone.LocalTransform  = ufbxToglm(pNode->node_to_parent);
//                 bone.GlobalTransform = ufbxToglm(pNode->node_to_world);
//                 bone.OffsetMatrix    = ufbxToglm(pNode->geometry_to_node);
//             }
//             I32 boneIndex = static_cast<int>(Skeleton->Bones.size());
//             Skeleton->BoneMap[bone.Name] = boneIndex;
//             Skeleton->Bones.push_back(bone);
            
//             if (bone.ParentIndex != -1) 
//             {
//                 Skeleton->Bones[bone.ParentIndex].Children.push_back(bone);
//             }
            
//             ++boneCount;
//         }

//         Skeleton->SkeletonUbo.JointsMatrices.resize( boneCount );

//         if (boneCount != 0) 
//         {
//             size_t bufferSize = boneCount * sizeof(Mat4);  // in bytes

//             SkeletonUbo = MakeShared<VyBuffer>( VyBuffer::uniformBuffer("skeleton", bufferSize) );
//         }
//     }


//     void UFBXImporter::loadAnimationClips() 
//     {
//         Animations = std::make_unique<VyAnimations>();

//         size_t numberOfAnimations = m_ModelScene->anim_stacks.count;
        
//         for (size_t animationIndex = 0; animationIndex < numberOfAnimations; ++animationIndex) 
//         {
//             ufbx_anim_stack& fbxAnimation = *m_ModelScene->anim_stacks.data[animationIndex];

//             const float target_framerate = 30.0f;
//             const int   max_frames       = 4096;

//             // Sample the animation evenly at `target_framerate` if possible while limiting the maximum
//             // number of frames to `max_frames` by potentially dropping FPS.
//             float duration  = (float)fbxAnimation.time_end - (float)fbxAnimation.time_begin;
//             int   numFrames = glm::clamp((int)(duration * target_framerate), 2, max_frames);
//             float framerate = (float)(numFrames - 1) / duration;

//             TStringView animationName( fbxAnimation.name.data );

//             Shared<AnimationClip> animation = MakeShared<AnimationClip>( animationName );

//             animation->setFirstKeyFrameTime((float)fbxAnimation.time_begin);
//             animation->setLastKeyFrameTime ((float)fbxAnimation.time_end);
//             animation->setFramerate        (framerate);
//             animation->setTotalFrameCount  (numFrames);

//             animation->AnimNodesList.resize( Skeleton->Bones.size() );

//             for (size_t boneIndex = 0; boneIndex < m_ModelScene->nodes.count; ++boneIndex) 
//             {
//                 ufbx_node* pNode                   = m_ModelScene->nodes.data[boneIndex];
//                 AnimationClip::AnimNode& animNode = animation->AnimNodesList[boneIndex];

//                 animNode.Rot.resize(numFrames);
//                 animNode.Pos.resize(numFrames);
//                 animNode.Scl.resize(numFrames);

//                 for (size_t frameIndex = 0; frameIndex < numFrames; frameIndex++) {
//                     double time = fbxAnimation.time_begin + (double)frameIndex / framerate;

//                     ufbx_transform transform = ufbx_evaluate_transform(fbxAnimation.anim, pNode, time);
//                     animNode.Rot[frameIndex] = ufbxToglm(transform.rotation);
//                     animNode.Pos[frameIndex] = ufbxToglm(transform.translation);
//                     animNode.Scl[frameIndex] = ufbxToglm(transform.scale);

//                     if (frameIndex > 0) 
//                     {
//                         // Negated quaternions are equivalent, but interpolating between ones of different
//                         // polarity takes a the longer path, so flip the quaternion if necessary.
//                         if (glm::dot(animNode.Rot[frameIndex], animNode.Rot[frameIndex - 1]) < 0.0f) 
//                         {
//                             animNode.Rot[frameIndex] *= -1;
//                         }
//                     }
//                 }
//             }

//             Animations->push( animation );
//         }
//     }


//     Mat4 UFBXImporter::ufbxToglm(const ufbx_matrix& ufbxMat) 
//     {
//         Mat4 glmMat4{};
//         for (U32 column = 0; column < 4; ++column) {
//             glmMat4[column].x = ufbxMat.cols[column].x;
//             glmMat4[column].y = ufbxMat.cols[column].y;
//             glmMat4[column].z = ufbxMat.cols[column].z;
//             glmMat4[column].w = column < 3 ? 0.0f : 1.0f;
//         }
//         return glmMat4;
//     }

//     Vec3 UFBXImporter::ufbxToglm(const ufbx_vec3& ufbxVec3) 
//     {
//         return Vec3(ufbxVec3.x, ufbxVec3.y, ufbxVec3.z);
//     }

//     Quat UFBXImporter::ufbxToglm(const ufbx_quat& ufbxQuat) 
//     {
//         Quat quatGLM{};
//         quatGLM.x = ufbxQuat.x;
//         quatGLM.y = ufbxQuat.y;
//         quatGLM.z = ufbxQuat.z;
//         quatGLM.w = ufbxQuat.w;
//         return quatGLM;
//     }
// }