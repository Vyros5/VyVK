// #include <VyEngine/GFX/Resources/Loaders/AssimpLoader.h>

// #include <VyEngine/Scene/ECS/Components.h>
// #include <VyEngine/Scene/Scene.h>

// #include <assimp/pbrmaterial.h>
// #include <glm/gtx/matrix_decompose.hpp>

// namespace Vy
// {
//     AssimpLoader::AssimpLoader()
//     {
//     }

//     AssimpLoader::~AssimpLoader()
//     {
//     }

//     Unique<VyScene> AssimpLoader::readScene(const TPath& filePath)
//     {
//         // VY_PROFILE_FUNCTION;

//         m_SceneDir = filePath.parent_path();

//         if (filePath.extension() == ".gltf")
//         {
//             m_IsGltf = true;
//         }

//         const aiScene* pAssimpScene;

//         {
//             // VY_PROFILE_SECTION("read_scene_file");

//             // TODO: handle tangent and bitangent calculations when normals
//             // are missing in file bcos assimp will not calculate for us
//             pAssimpScene = m_Importer.ReadFile(
//                 filePath.string(),
//                 aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace | aiProcess_JoinIdenticalVertices | aiProcess_FlipUVs | aiProcess_GenBoundingBoxes);

//             if (!pAssimpScene || pAssimpScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !pAssimpScene->mRootNode)
//             {
//                 VY_ERROR("Unable to read scene file. {}", m_Importer.GetErrorString());
//                 m_SceneDir = "";
//                 return nullptr;
//             }
//         }

//         return parseScene( pAssimpScene );
//     }


//     Unique<VyScene> AssimpLoader::parseScene(const aiScene* pAssimpScene)
//     {
//         // VY_PROFILE_FUNCTION;

//         auto pNewScene = MakeUnique<VyScene>(pAssimpScene->mName.C_Str());

//         int totalVertices = 0;
//         int totalIndices  = 0;
//         for (U32 meshIndex = 0u; meshIndex < pAssimpScene->mNumMeshes; ++meshIndex)
//         {
//             totalVertices += pAssimpScene->mMeshes[ meshIndex ]->mNumVertices;
//             totalIndices  += pAssimpScene->mMeshes[ meshIndex ]->mNumFaces * 3; // Assumption, face is triangular
//         }

//         m_TempVertices.reserve( totalVertices );
//         m_TempIndices .reserve( totalIndices  );
//         pNewScene->m_SceneMeshes.reserve( pAssimpScene->mNumMeshes );

//         m_TempVertices.clear();
//         m_TempIndices .clear();

//         if (pAssimpScene->mNumMeshes > 0)
//         {
//             parseMeshes( *pNewScene, pAssimpScene );
//         }

//         if (pAssimpScene->HasMaterials())
//         {
//             pNewScene->initMaterialCache( pAssimpScene->mNumMaterials );

//             parseMaterials(*pNewScene, pAssimpScene);
//         }

//         if (pAssimpScene->mRootNode)
//         {
//             traverseSceneNodes( *pNewScene, pAssimpScene->mRootNode, pAssimpScene, pNewScene->getRootHandle() );
//         }

//         int baseVertexOffset = 0;
//         U32 baseFirstIndex   = 0;

//         uploadVertexAndIndexBuffers(
//             m_TempVertices,
//             m_TempIndices,
//             &baseVertexOffset,
//             &baseFirstIndex);

//         for (U32 meshIndex = 0u; meshIndex < pAssimpScene->mNumMeshes; ++meshIndex)
//         {
//             GfxMeshData& meshData = pNewScene->m_SceneMeshes[meshIndex];

//             meshData.FirstIndex   = baseFirstIndex   + meshData.FirstIndex;
//             meshData.VertexOffset = baseVertexOffset + meshData.VertexOffset;
//         }

//         m_TempVertices.clear();
//         m_TempIndices .clear();

//         return pNewScene;
//     }


//     void AssimpLoader::traverseSceneNodes(VyScene& scene, const aiNode* node, const aiScene* aiScene, EntityHandle parentId)
//     {
//         // VY_PROFILE_FUNCTION;

//         auto gameObject   = MakeUnique<VyGameObject>( parseAssimpNode( node ) );
//         auto gameObjectId = gameObject->handle();

//         gameObject->setName(node->mName.C_Str());

//         if (node->mNumMeshes > 0)
//         {
//             auto& meshComponent = gameObject->add<MeshComponent>();

//             // calculate AABB for the whole mesh model
//             auto modelAABB = VyAABB{};
//             modelAABB.Min  = glm::vec3(std::numeric_limits<float>::max());
//             modelAABB.Max  = glm::vec3(std::numeric_limits<float>::lowest());

//             for (U32 index = 0u; index < node->mNumMeshes; ++index)
//             {
//                 U32 sceneMeshIndex = node->mMeshes[index];

//                 meshComponent.Meshes   .push_back(sceneMeshIndex);
//                 meshComponent.Materials.push_back(aiScene->mMeshes[sceneMeshIndex]->mMaterialIndex);

//                 aiVector3D meshMin = aiScene->mMeshes[sceneMeshIndex]->mAABB.mMin;
//                 aiVector3D meshMax = aiScene->mMeshes[sceneMeshIndex]->mAABB.mMax;

//                 modelAABB.Min      = glm::min(modelAABB.Min, glm::vec3(meshMin.x, meshMin.y, meshMin.z));
//                 modelAABB.Max      = glm::max(modelAABB.Max, glm::vec3(meshMax.x, meshMax.y, meshMax.z));
//             }

//             // object space model AABB
//             meshComponent.ObjectAABB = modelAABB;
//         }

//         // attach object to the scene
//         scene.addGameObject(std::move(gameObject), parentId);

//         // traverse children
//         if (node->mNumChildren > 0)
//         {
//             for (U32 childIndex = 0u; childIndex < node->mNumChildren; ++childIndex)
//             {
//                 traverseSceneNodes(scene, node->mChildren[childIndex], aiScene, gameObjectId);
//             }
//         }
//     }

//     VyGameObject AssimpLoader::parseAssimpNode(const aiNode* node)
//     {
//         // VY_PROFILE_FUNCTION;

//         VyGameObject newGameObject;
//         auto&      transform = newGameObject.get<TransformComponent>();

//         // get transform from assimp node
//         glm::mat4 transformation { 1.0f };

//         transformation[0][0] = node->mTransformation.a1;
//         transformation[0][1] = node->mTransformation.b1;
//         transformation[0][2] = node->mTransformation.c1;
//         transformation[0][3] = node->mTransformation.d1;

//         transformation[1][0] = node->mTransformation.a2;
//         transformation[1][1] = node->mTransformation.b2;
//         transformation[1][2] = node->mTransformation.c2;
//         transformation[1][3] = node->mTransformation.d2;

//         transformation[2][0] = node->mTransformation.a3;
//         transformation[2][1] = node->mTransformation.b3;
//         transformation[2][2] = node->mTransformation.c3;
//         transformation[2][3] = node->mTransformation.d3;

//         transformation[3][0] = node->mTransformation.a4;
//         transformation[3][1] = node->mTransformation.b4;
//         transformation[3][2] = node->mTransformation.c4;
//         transformation[3][3] = node->mTransformation.d4;

//         glm::vec3 scale;
//         glm::quat rotation;
//         glm::vec3 translation;
//         glm::vec3 skew;
//         glm::vec4 perspective;
//         glm::decompose(transformation, scale, rotation, translation, skew, perspective);

//         transform.Translation = translation;
//         transform.Scale       = scale;
//         transform.Rotation    = glm::eulerAngles(rotation);

//         return newGameObject;
//     }

//     void AssimpLoader::parseMeshes(VyScene& scene, const aiScene* aiScene)
//     {
//         // VY_PROFILE_FUNCTION;

//         for (U32 meshIndex = 0u; meshIndex < aiScene->mNumMeshes; ++meshIndex)
//         {
//             TVector<VyVertex> vertices{};
//             TVector<U32>      indices {};

//             aiMesh* mesh = aiScene->mMeshes[meshIndex];

//             vertices.reserve(mesh->mNumVertices);
//             indices .reserve(mesh->mNumFaces * 3); // Assumption, face is triangular

//             U32 indexCount   = 0u;
//             U32 firstIndex   = m_TempIndices.size();
//             int32_t  vertexOffset = m_TempVertices.size();

//             for (U32 vertexIndex = 0u; vertexIndex < mesh->mNumVertices; ++vertexIndex)
//             {
//                 VyVertex v;

//                 v.Position = glm::vec3(mesh->mVertices[vertexIndex].x, mesh->mVertices[vertexIndex].y, mesh->mVertices[vertexIndex].z);

//                 if (mesh->HasNormals())
//                 {
//                     v.Normal = glm::vec3(mesh->mNormals[vertexIndex].x, mesh->mNormals[vertexIndex].y, mesh->mNormals[vertexIndex].z);
//                 }
//                 else
//                 {
//                     VY_ERROR("Missing normals in assimp file");
//                 }

//                 if (mesh->HasTangentsAndBitangents())
//                 {
//                     v.Tangent = glm::vec3(mesh->mTangents[vertexIndex].x, mesh->mTangents[vertexIndex].y, mesh->mTangents[vertexIndex].z);

//                     auto bitangent = glm::vec3(mesh->mBitangents[vertexIndex].x, mesh->mBitangents[vertexIndex].y, mesh->mBitangents[vertexIndex].z);

//                     // flip if not right handed
//                     if (glm::dot(glm::cross(v.Normal, v.Tangent), bitangent) < 0.0f)
//                         v.Tangent *= -1.0f; // Flip tangent
//                 }
//                 else
//                 {
//                     VY_ERROR("Missing tangents and bitangents in assimp file");
//                 }

//                 if (mesh->HasTextureCoords(0))
//                 {
//                     v.UV = glm::vec2(mesh->mTextureCoords[0][vertexIndex].x, 1.f - mesh->mTextureCoords[0][vertexIndex].y);
//                 }

//                 vertices.push_back(v);
//                 m_TempVertices.push_back(v);
//             }

//             for (U32 faceIndex = 0u; faceIndex < mesh->mNumFaces; ++faceIndex)
//             {
//                 VY_ASSERT(mesh->mFaces[faceIndex].mNumIndices == 3, "Face is not triangular");
//                 for (U32 vertexIndex = 0u; vertexIndex < 3; ++vertexIndex)
//                 {
//                     U32 index = mesh->mFaces[faceIndex].mIndices[vertexIndex];
//                     indices.push_back(index);
//                     m_TempIndices.push_back(index);

//                     indexCount++;
//                 }
//             }

//             scene.m_SceneMeshes.push_back({
//                 .IndexCount   = indexCount,
//                 .FirstIndex   = firstIndex,
//                 .VertexOffset = vertexOffset,
//             });
//         }
//     }

//     TPath AssimpLoader::getGltfTexturePath(aiMaterial* mat)
//     {
//         aiString path;
//         aiReturn result = mat->GetTexture(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_BASE_COLOR_TEXTURE, &path);

//         if (result == aiReturn_FAILURE)
//             return "";

//         return TPath(path.C_Str());
//     }

//     TPath AssimpLoader::getTexturePath(aiMaterial* mat, aiTextureType type)
//     {
//         aiString path;
//         aiReturn result = mat->GetTexture(type, 0, &path);

//         if (result == aiReturn_FAILURE)
//             return "";

//         return TPath(path.C_Str());
//     }

//     TPath AssimpLoader::getGltfMRTexturePath(aiMaterial* mat)
//     {
//         aiString path;
//         aiReturn result = mat->GetTexture(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLICROUGHNESS_TEXTURE, &path);

//         if (result == aiReturn_FAILURE)
//             return "";

//         return TPath(path.C_Str());
//     }

//     int32_t AssimpLoader::read2DTexture(TPath texPath, VkFormat format) const
//     {
//         if (!texPath.empty())
//         {
//             auto texturePath = (m_SceneDir / texPath).make_preferred().string();
//             return TextureDB::cache()->createTextureAsync(texturePath, TextureType::Texture2D, format);
//         }

//         return -1;
//     }

//     void AssimpLoader::parseMaterials(VyScene& scene, const aiScene* aiScene)
//     {
//         // VY_PROFILE_FUNCTION;

//         for (U32 matIndex = 0; matIndex < aiScene->mNumMaterials; ++matIndex)
//         {
//             aiMaterial*           aiMat = aiScene->mMaterials[matIndex];
//             Material              newMaterial;

//             TPath baseColorTexturePath = "";

//             /// textures indices
//             int32_t albedoTexId            = -1;
//             int32_t normalTexId            = -1;
//             int32_t metallicRoughnessTexId = -1;
//             int32_t aoTexId                = -1;
//             int32_t emissiveTexId          = -1;

//             int32_t defaultTexId           = TextureDB::cache()->getDefaultTexture2D().id;

//             // find albedo texture and color value
//             if (m_IsGltf)
//             {
//                 baseColorTexturePath = getGltfTexturePath(aiMat);
//             }

//             if (baseColorTexturePath.empty())
//             {
//                 baseColorTexturePath = getTexturePath(aiMat, aiTextureType_BASE_COLOR);

//                 if (baseColorTexturePath.empty())
//                 {
//                     baseColorTexturePath = getTexturePath(aiMat, aiTextureType_DIFFUSE);
//                 }
//             }

//             if (!baseColorTexturePath.empty())
//             {
//                 albedoTexId = read2DTexture(baseColorTexturePath, PixelFormat::R8G8B8A8_srgb);
//             }
//             else
//             {
//                 albedoTexId = defaultTexId;
//             }

//             aiColor3D pbrBaseColor { 1.0f };
//             aiMat->Get(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_BASE_COLOR_FACTOR, pbrBaseColor);

//             float alpha = 1.f;
//             aiMat->Get(AI_MATKEY_OPACITY, alpha);

//             newMaterial.albedoColor = glm::vec4(pbrBaseColor.r, pbrBaseColor.g, pbrBaseColor.b, alpha);

//             newMaterial.albedoTexId = albedoTexId;

//             // find out normal map
//             TPath normalTexPath = getTexturePath(aiMat, aiTextureType_NORMALS);

//             if (!normalTexPath.empty())
//             {
//                 newMaterial.normalTexId = read2DTexture(normalTexPath, PixelFormat::R8G8B8A8_unorm);

//                 aiMat->Get("normalScale", 0, 0, newMaterial.normalScale);
//             }

//             // find out metallic-roughness combined texture
//             TPath mrTexPath = getGltfMRTexturePath(aiMat);

//             if (!mrTexPath.empty())
//             {
//                 newMaterial.metallicRoughnessTexId = read2DTexture(mrTexPath, PixelFormat::R8G8B8A8_unorm);
//             }
//             else
//             {
//                 newMaterial.metallicRoughnessTexId = defaultTexId;
//             }

//             aiMat->Get(AI_MATKEY_METALLIC_FACTOR, newMaterial.metal);
//             aiMat->Get(AI_MATKEY_ROUGHNESS_FACTOR, newMaterial.rough);

//             // find out Ambient occlusion texture
//             TPath aoTexPath = getTexturePath(aiMat, aiTextureType_LIGHTMAP);

//             if (!aoTexPath.empty())
//             {
//                 newMaterial.aoTexId = read2DTexture(aoTexPath, PixelFormat::R8G8B8A8_unorm);
//             }

//             aiMat->Get(AI_MATKEY_REFLECTIVITY, newMaterial.aoStrength);

//             // find out Emissive texture
//             TPath emissiveTexPath = getTexturePath(aiMat, aiTextureType_EMISSIVE);

//             if (!emissiveTexPath.empty())
//             {
//                 newMaterial.emissiveTexId = read2DTexture(emissiveTexPath, PixelFormat::R8G8B8A8_srgb);

//                 aiMat->Get("emissiveIntensity", 0, 0, newMaterial.emissiveIntensity);

//                 if (aiMat->Get(AI_MATKEY_COLOR_EMISSIVE, newMaterial.emissiveColor) != AI_SUCCESS)
//                 {
//                     newMaterial.emissiveColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
//                 }
//             }

//             scene.addMaterial(newMaterial);
//         }
//     }
// }