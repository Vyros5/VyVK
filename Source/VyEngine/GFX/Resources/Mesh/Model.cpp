#include <VyEngine/GFX/Resources/Mesh/Model.h>

#include <VyEngine/VK/Context.h>
#include <VyEngine/GFX/Resources/Mesh/Vertex.h>
#include <VyEngine/GFX/Resources/Mesh/Utils/AiConversion.h>
#include <VyLib/Core/Math.h>
#include <VyLib/Util/Hash.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include <iostream>
#include <VyLib/Common/AnsiColor.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace std 
{
    template <>
    struct hash<Vy::VyVertex> 
    {
        size_t operator()(Vy::VyVertex const& vertex) const 
        {
            size_t seed = 0;
            
            Vy::Hash::hashCombine(seed
                , vertex.Position
                , vertex.Color
                , vertex.Normal
                , vertex.UV
                , vertex.Tangent
                , vertex.Bitangent
            );
            
            return seed;
        }
    };
}

namespace Vy
{
    /* Static */

    Shared<VyModel> VyModel::createFromFile(const TString& filepath)
    {
		TPath fp{ filepath };        
		TString assetDir = fp.parent_path().string() + "/";

		//READ OBJ DATA
		Data data{}; 
		data.load( filepath );


		THashMap<TString, U32>     indexMap;
		TVector<Unique<VyTexture>> textures;

		textures.reserve( data.Materials.size() * 4 ); 

		auto tryLoad = [&](TString const& filename, U32& outIndex, VkFormat format)
		{
			if (filename == "NULL") 
			{
				outIndex = UINT32_MAX;
				return;
			}

			TString full = assetDir + filename;
			auto it = indexMap.find( full );

			if (it == indexMap.end())
			{
				U32 idx = U32( textures.size() );
				
                indexMap[ full ] = idx;

				textures.emplace_back( VyTexture::createFromFile( full, format ) );

				outIndex = idx;
			}
			else 
			{
				outIndex = it->second;
			}
		};

		for (auto& mi : data.Materials)
		{
			tryLoad(mi.AlbedoPath,        mi.AlbedoIndex,        VK_FORMAT_R8G8B8A8_SRGB  /*GBuffer::ALBEDO_FORMAT*/);
			tryLoad(mi.MetallicRoughPath, mi.MetallicRoughIndex, VK_FORMAT_R8G8B8A8_SRGB  /*GBuffer::METALROUGH_FORMAT*/);
			tryLoad(mi.NormalPath,        mi.NormalIndex,        VK_FORMAT_R8G8B8A8_UNORM /*GBuffer::NORM_FORMAT*/);
			tryLoad(mi.OcclusionPath,     mi.OcclusionIndex,     VK_FORMAT_R8G8B8A8_SRGB  /*GBuffer::OCCLUSION_FORMAT*/);
		}

		data.Textures = std::move( textures );

		VyTexture::initBindless(U32(data.Textures.size()));
		VyTexture::updateBindless( &data );

        return MakeUnique<VyModel>( std::move( data ) );
    }



	void VyModel::Data::load(const TString& filepath)
	{
		Assimp::Importer importer;

		const aiScene* pScene = importer.ReadFile(filepath,
			  aiProcess_Triangulate     
			| aiProcess_FlipUVs           
			| aiProcess_CalcTangentSpace
			| aiProcess_PreTransformVertices
		);

		if (!pScene || pScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !pScene->mMeshes) 
        {
			throw std::runtime_error("Assimp error: " + TString(importer.GetErrorString()));
		}

		// -- MATERIALS --------------------------------------------------------------

		Materials.resize(pScene->mNumMaterials); 
		
        for (size_t m = 0; m < pScene->mNumMaterials; m++)
		{
			aiMaterial* pMat = pScene->mMaterials[ m ];
			auto&       mi   = Materials[ m ];
			
			auto tryTex = [&](aiTextureType type, TString& outPath) 
            {
				if (pMat->GetTextureCount(type) > 0) 
                {
					aiString path;
					pMat->GetTexture(type, 0, &path);

					outPath = path.C_Str();
				}
            };

			// BASE COLOR
			tryTex(aiTextureType_BASE_COLOR, mi.AlbedoPath);

			// METALLIC-ROUGHNESS
			// Assimp may expose the combined metallic/roughness texture under
			// different texture types depending on the importer.  The original
			// code only queried aiTextureType_SPECULAR which corresponds to the
			// legacy specular/glossiness workflow and therefore failed to locate
			// the texture for glTF PBR assets such as MetalRoughSpheres.  This
			// left the metallic-roughness channel uninitialised causing the
			// spheres to render with default values.

			// First try the dedicated PBR texture types
			tryTex(aiTextureType_METALNESS, mi.MetallicRoughPath);
			
            if (mi.MetallicRoughPath == "NULL") 
            {
				tryTex(aiTextureType_DIFFUSE_ROUGHNESS, mi.MetallicRoughPath);
			}
			
            // Fallback for older exporters that might still use the specular slot
			if (mi.MetallicRoughPath == "NULL") 
            {
                // tryTex(aiTextureType_SPECULAR, mi.MetallicRoughPath);
                
				if (pMat->GetTextureCount(aiTextureType_SPECULAR) > 0) 
                {
					aiString path;
					pMat->GetTexture(aiTextureType_SPECULAR, 0, &path);

					mi.MetallicRoughPath = path.C_Str();
				}

				else if (pMat->GetTextureCount(aiTextureType_DIFFUSE_ROUGHNESS) > 0) 
                {
					aiString path;
					pMat->GetTexture(aiTextureType_DIFFUSE_ROUGHNESS, 0, &path);

					mi.MetallicRoughPath = path.C_Str();
				}
			}

			// NORMAL MAP
			if (pMat->GetTextureCount(aiTextureType_NORMAL_CAMERA) > 0) 
            {
				aiString path; pMat->GetTexture(aiTextureType_NORMAL_CAMERA, 0, &path);

				mi.NormalPath = path.C_Str(); 
			}
			else {
				tryTex(aiTextureType_NORMALS, mi.NormalPath);
			}

			// AMBIENT OCCLUSION
			tryTex(aiTextureType_AMBIENT_OCCLUSION, mi.OcclusionPath);


			//RAW SCAN FOR PBR FACTORS (cant seem to get the macros to be recognised so doing it by hand) 
			for (unsigned int p = 0; p < pMat->mNumProperties; p++) 
            {
				auto*       pProp = pMat->mProperties[p];
				const char* pKey  = pProp->mKey.C_Str();

				if (std::strcmp(pKey, "$mat.gltf.pbrMetallicRoughness.baseColorFactor") == 0
					&& pProp->mDataLength >= sizeof(float) * 4) 
                {
					auto f = reinterpret_cast<float const*>(pProp->mData);

					mi.AlbedoFactor = Vec4(f[0], f[1], f[2], f[3]);
				}

				else if (std::strcmp(pKey, "$mat.gltf.pbrMetallicRoughness.metallicFactor") == 0
					&& pProp->mDataLength >= sizeof(float)) 
                {
					mi.MetallicFactor = *reinterpret_cast<float const*>(pProp->mData);
				}
				
                else if (std::strcmp(pKey, "$mat.gltf.pbrMetallicRoughness.roughnessFactor") == 0
					&& pProp->mDataLength >= sizeof(float)) 
                {
					mi.RoughnessFactor = *reinterpret_cast<float const*>(pProp->mData);
				}
				
                else if (std::strcmp(pKey, "$mat.gltf.occlusionStrength") == 0
					&& pProp->mDataLength >= sizeof(float)) 
                {
					mi.OcclusionFactor = *reinterpret_cast<float const*>(pProp->mData);
				}
			}
		}

		// -- COUNT TOTAL SIZE -------------------------------------------------
		U32 totalVertices = 0;
		U32 totalFaces    = 0;

		for (U32 m = 0; m < pScene->mNumMeshes; m++) 
		{
			totalVertices += pScene->mMeshes[ m ]->mNumVertices;
			totalFaces    += pScene->mMeshes[ m ]->mNumFaces;
		}

		Vertices .reserve( totalVertices      );
		Indices  .reserve( totalFaces * 3     );
		Submeshes.reserve( pScene->mNumMeshes );

		U32 globalVertexOffset = 0; 
		U32 globalIndexOffset  = 0;


		// -- MESHES, VERTICES, INDICES, SUBMESHES ----------------------------
		for (U32 m = 0; m < pScene->mNumMeshes; m++)
		{
			aiMesh* pMesh = pScene->mMeshes[ m ];

			// [ VERTICES ]
			for (U32 v = 0; v < pMesh->mNumVertices; v++) 
			{
				VyVertex vertex{};
				
                // [ Position ]
                if (pMesh->mVertices) 
                {
                    vertex.Position = Utils::AiToGlm( pMesh->mVertices[ v ] );
                }
                else {
                    VY_THROW_RUNTIME_ERROR("No vertices found!");
                }

                // [ Color ]
                if (pMesh->HasVertexColors( 0 )) 
                {
                    vertex.Color = Utils::AiToGlm( pMesh->mColors[ 0 ][ v ] );
                } 
                else {
                    vertex.Color = { 1.0f, 1.0f, 1.0f }; // Default white
                }

                // [ Normal ]
                if (pMesh->HasNormals()) 
                {
                    vertex.Normal = Utils::AiToGlm( pMesh->mNormals[ v ] );
                } 
                else {
                    vertex.Normal = { 0.0f, 0.0f, 0.0f }; // Default
                }

                // [ UV ]
                if (pMesh->HasTextureCoords( 0 )) 
                {
                    vertex.UV = Utils::AiToGlm( pMesh->mTextureCoords[ 0 ][ v ] );
                } 
                else {
                    vertex.UV = { 0.0f, 0.0f }; // Default
                }

                // [ Tangents & Bitangents ]
                if (pMesh->HasTangentsAndBitangents()) 
                {
                    vertex.Tangent   = Utils::AiToGlm( pMesh->mTangents  [ v ] );
                    vertex.Bitangent = Utils::AiToGlm( pMesh->mBitangents[ v ] );
                }
                else {
                    // you can orthonormalize later in the shader or generate here
                    vertex.Tangent   = { 1.0f, 0.0f, 0.0f };
                    vertex.Bitangent = { 0.0f, 1.0f, 0.0f };
                }

				Vertices.push_back( vertex );
			}

			// [ INDICES ]
			for (U32 f = 0; f < pMesh->mNumFaces; f++)
			{
				const aiFace& face = pMesh->mFaces[ f ];

				for (U32 idx = 0; idx < face.mNumIndices; idx++)
				{
					Indices.push_back( face.mIndices[ idx ] + globalVertexOffset );
				}
			}

			//record into submesh
			U32 faceCount = pMesh->mNumFaces;

			Submeshes.push_back({
				globalIndexOffset,
				faceCount * 3,
				pMesh->mMaterialIndex 
            });

			//bump offsets
			globalVertexOffset += pMesh->mNumVertices;
			globalIndexOffset  += faceCount * 3;
		}
	}
}

// namespace Vy
// {
// // --- Builder Class Implementation ---
//     void VyModel::Builder::loadModel(const TString& filepath) 
//     {
//         tinyobj::attrib_t            attrib;
//         TVector<tinyobj::shape_t>    shapes;
//         TVector<tinyobj::material_t> materials;
//         TString                      warn, err;

//         TString baseDir = filepath.substr(0, filepath.find_last_of('/') + 1);
        
//         if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filepath.c_str(), baseDir.c_str())) 
//         {
//             throw std::runtime_error(warn + err);
//         }

//         // Initialize bounding box
//         AABB.Min = Vec3( FLT_MAX);
//         AABB.Max = Vec3(-FLT_MAX);

//         bool bFoundAny = false;

//         // Clear previous data
//         TexturePaths           .clear();
//         MaterialIdToTexturePath.clear();

//         // Step 1: Create mapping from material ID to texture path
//         for (int i = 0; i < materials.size(); ++i)
//         {
//             const auto& mat = materials[ i ];

//             if (!mat.diffuse_texname.empty()) 
//             {
//                 MaterialIdToTexturePath[i] = mat.diffuse_texname;

//                 // Also store in TexturePaths for backwards compatibility
//                 if (std::find(TexturePaths.begin(), TexturePaths.end(), mat.diffuse_texname) == TexturePaths.end()) 
//                 {
//                     TexturePaths.push_back( mat.diffuse_texname );
//                 }
//             }
//             else {
//                 MaterialIdToTexturePath[i] = ""; // No texture for this material
//             }

//             std::cout << "Material " << i << " (" << mat.name << "): "
//                 << (mat.diffuse_texname.empty() ? "No texture" : mat.diffuse_texname) << std::endl;
//         }

//         // Step 2: Iterate through shapes to build sub-meshes
//         for (const auto& shape : shapes) 
//         {
//             TMap<int, THashMap<VyVertex, U32>> materialUniqueVertices;
//             TMap<int, TVector<U32>>            materialIndices;

//             for (size_t i = 0; i < shape.mesh.indices.size(); ++i) 
//             {
//                 const auto& index       = shape.mesh.indices     [ i ];
//                 int         material_id = shape.mesh.material_ids[ i / 3 ];

//                 if (materialUniqueVertices.find(material_id) == materialUniqueVertices.end()) 
//                 {
//                     materialUniqueVertices[ material_id ] = {};
//                     materialIndices       [ material_id ] = {};
//                 }

//                 VyVertex vertex{};
//                 if (index.vertex_index >= 0) 
//                 {
//                     vertex.Position = {
//                          attrib.vertices[3 * index.vertex_index + 0],
//                         -attrib.vertices[3 * index.vertex_index + 1],
//                          attrib.vertices[3 * index.vertex_index + 2],
//                     };

//                     // Update bounding box with this vertex position
//                     AABB.Min = glm::min(AABB.Min, vertex.Position);
//                     AABB.Max = glm::max(AABB.Max, vertex.Position);
//                     bFoundAny = true;

//                     // tinyobjloader does not guarantee colors exist, check the size
//                     if (attrib.colors.size() > 3 * index.vertex_index) 
//                     {
//                         vertex.Color = {
//                            attrib.colors[3 * index.vertex_index + 0],
//                            attrib.colors[3 * index.vertex_index + 1],
//                            attrib.colors[3 * index.vertex_index + 2],
//                         };
//                     }
//                     else {
//                         vertex.Color = { 1.0f, 1.0f, 1.0f }; // Default white color
//                     }
//                 }
//                 if (index.normal_index >= 0) 
//                 {
//                     vertex.Normal = {
//                         attrib.normals[3 * index.normal_index + 0],
//                         attrib.normals[3 * index.normal_index + 1],
//                         attrib.normals[3 * index.normal_index + 2],
//                     };
//                 }
//                 if (index.texcoord_index >= 0) 
//                 {
//                     vertex.UV = {
//                         attrib.texcoords[2 * index.texcoord_index + 0],
//                         attrib.texcoords[2 * index.texcoord_index + 1],
//                     };
//                 }

//                 if (materialUniqueVertices.at(material_id).count(vertex) == 0) 
//                 {
//                     materialUniqueVertices.at( material_id )[ vertex ] = 
//                         static_cast<U32>(materialUniqueVertices.at( material_id ).size());
//                 }

//                 materialIndices.at( material_id ).push_back(
//                     materialUniqueVertices.at( material_id )[ vertex ]
//                 );
//             }

//             // Step 3: Create sub-meshes and their Vulkan buffers
//             for (auto const& [ material_id, unique_vertices_map ] : materialUniqueVertices) 
//             {
//                 Submesh submesh{};
//                 submesh.MaterialId = material_id;
//                 submesh.IndexCount = materialIndices.at(material_id).size();

//                 TVector<VyVertex> submeshVertices( unique_vertices_map.size() );
                
//                 for (auto const& [ vertex, index ] : unique_vertices_map) 
//                 {
//                     submeshVertices[ index ] = vertex;
//                 }

//                 createVertexBuffer(submesh.VertexBuffer, submeshVertices);
//                 createIndexBuffer (submesh.IndexBuffer,  materialIndices.at( material_id ));

//                 // Debug output
//                 TString textureName = MaterialIdToTexturePath.count( material_id ) 
//                     ? MaterialIdToTexturePath[ material_id ] 
//                     : "None";

//                 std::cout << "Created submesh for material " << material_id
//                     << " with texture: " << textureName
//                     << " (vertices: " << submeshVertices.size()
//                     << ", indices: " << submesh.IndexCount << ")" << std::endl;

//                 Submeshes.push_back( std::move( submesh ) );
//             }
//         }

//         // Handle case where no vertices were found
//         if (!bFoundAny) 
//         {
//             std::cout << "Warning: No vertices found in model: " << filepath << std::endl;
//             AABB.Min = Vec3(0.0f);
//             AABB.Max = Vec3(0.0f);
//         }
//         else {
//             std::cout << "Model loaded: " << filepath << std::endl;
//             std::cout << "  Materials found: " << materials.size() << std::endl;
//             std::cout << "  Submeshes created: " << Submeshes.size() << std::endl;
//             std::cout << "  Bounding box: min(" << AABB.Min.x << ", " << AABB.Min.y << ", " << AABB.Min.z
//                 << ") max(" << AABB.Max.x << ", " << AABB.Max.y << ", " << AABB.Max.z << ")" << std::endl;
//         }
//     }


//     // =====================================================================================================================

//     void VyModel::Builder::createVertexBuffer(Unique<VyBuffer>& buffer, const TVector<VyVertex>& vertices) 
//     {
//         U32 vertexCount = static_cast<U32>( vertices.size() );

//         VY_ASSERT(vertexCount >= 3, "Vertex count must be at least 3");
        
//         U32 vertexSize = sizeof(vertices[0]);

//         buffer = MakeUnique<VyBuffer>( VyBuffer::vertexBuffer( "model", vertexSize, vertexCount ) );
        
//         buffer->upload( vertices );
//     }

//     // =====================================================================================================================

//     void VyModel::Builder::createIndexBuffer(Unique<VyBuffer>& buffer, const TVector<U32>& indices) 
//     {
// 		if (indices.empty()) {
// 			return;
// 		}

//         U32 indexCount = static_cast<U32>( indices.size() );
//         U32 indexSize  = sizeof(indices[0]);

//         buffer = MakeUnique<VyBuffer>( VyBuffer::indexBuffer( "model", indexSize, indexCount ) );
        
//         buffer->upload( indices );
//     }
// }



namespace Vy
{
    // void VyModel::Data::computeTangentBasis(VyVertex& v0, VyVertex& v1, VyVertex& v2, Vec3* pTanOut)
    // {
    //     // Edges of the triangle : Position delta
    //     Vec3 deltaPos1 = v1.Position - v0.Position;
    //     Vec3 deltaPos2 = v2.Position - v0.Position;
        
    //     // UV delta
    //     Vec2 deltaUV1 = v1.UV - v0.UV;
    //     Vec2 deltaUV2 = v2.UV - v0.UV;
        
    //     if (v1.UV == v0.UV && v2.UV == v0.UV) 
    //     {
    //         deltaUV1 = { 1.0f, 0.0f };
    //         deltaUV2 = { 0.0f, 1.0f };
    //     }
        
    //     float denom = (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
    //     float r = denom == 0.0f ? 0.0f : 1.0f / denom;
        
    //     pTanOut[ 0 ] = (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y) * r;
    //     pTanOut[ 1 ] = (deltaPos2 * deltaUV1.x - deltaPos1 * deltaUV2.x) * r;
    // }


    // VyModel::Data VyModel::Data::makeSimpleCube(bool bInvert) 
    // {
    //     VyModel::Data cubeData;

    //     cubeData.Vertices = {
    //         { {-1.0f, -1.0f,  1.0f }, {}, {}, {}, { 0.0f, 0.0f } },
    //         { { 1.0f, -1.0f,  1.0f }, {}, {}, {}, { 1.0f, 0.0f } },
    //         { { 1.0f,  1.0f,  1.0f }, {}, {}, {}, { 1.0f, 1.0f } },
    //         { {-1.0f,  1.0f,  1.0f }, {}, {}, {}, { 0.0f, 1.0f } },
    //         { {-1.0f, -1.0f, -1.0f }, {}, {}, {}, { 0.0f, 0.0f } },
    //         { { 1.0f, -1.0f, -1.0f }, {}, {}, {}, { 1.0f, 0.0f } },
    //         { { 1.0f,  1.0f, -1.0f }, {}, {}, {}, { 1.0f, 1.0f } },
    //         { {-1.0f,  1.0f, -1.0f }, {}, {}, {}, { 0.0f, 1.0f } }
    //     };

    //     if (bInvert) 
    //     {
    //         cubeData.Indices = {
    //             0, 2, 1, 2, 0, 3,
    //             4, 5, 6, 6, 7, 4,
    //             1, 6, 5, 6, 1, 2,
    //             0, 4, 7, 7, 3, 0,
    //             4, 1, 5, 1, 4, 0,
    //             3, 6, 2, 6, 3, 7
    //         };
    //     } 
    //     else 
    //     {
    //         cubeData.Indices = {
    //             2, 0, 1, 0, 2, 3,
    //             5, 4, 6, 7, 6, 4,
    //             6, 1, 5, 1, 6, 2,
    //             4, 0, 7, 3, 7, 0,
    //             1, 4, 5, 4, 1, 0,
    //             6, 3, 2, 3, 6, 7
    //         };
    //     }
    
    //     return cubeData;
    // }


    bool VyModel::Data::loadModel(const TPath& path, bool bAllUniqueVertices) 
    {
        tinyobj::attrib_t            attrib;
        TVector<tinyobj::shape_t>    shapes;
        TVector<tinyobj::material_t> materials;
        TString                      warn, err;
        
        if (!tinyobj::LoadObj( &attrib, &shapes, &materials, &warn, &err, path.string().c_str() )) 
        {
            VY_THROW_RUNTIME_ERROR( warn + err );

            return false;
        }
        
        Vertices.clear();
        Indices .clear();
        
        THashMap<VyVertex, U32> uniqueVertices{};

        for (const auto& shape: shapes) 
        {
            for (const auto& index: shape.mesh.indices) 
            {
                VyVertex vertex{};
                
                if (index.vertex_index >= 0) 
                {
                    vertex.Position = {
                        attrib.vertices[3 * index.vertex_index + 0],
                        attrib.vertices[3 * index.vertex_index + 1],
                        attrib.vertices[3 * index.vertex_index + 2]
                    };

                    vertex.Color = {
                        attrib.colors[3 * index.vertex_index + 0],
                        attrib.colors[3 * index.vertex_index + 1],
                        attrib.colors[3 * index.vertex_index + 2]
                    };
                }
                if (index.normal_index >= 0) 
                {
                    vertex.Normal = {
                        attrib.normals[3 * index.normal_index + 0],
                        attrib.normals[3 * index.normal_index + 1],
                        attrib.normals[3 * index.normal_index + 2]
                    };
                }
                if (index.texcoord_index >= 0) 
                {
                    vertex.UV = {
                        attrib.texcoords[2 * index.texcoord_index + 0],
                        attrib.texcoords[2 * index.texcoord_index + 1]
                    };
                }
                
                if (!uniqueVertices.contains( vertex )) 
                {
                    uniqueVertices[ vertex ] = static_cast<U32>( Vertices.size() );
                    
                    Vertices.push_back( vertex );
                }

                // bAllUniqueVertices == True treats ALL vertices as unique (bypass overlapping UV bug).
                if (uniqueVertices.count( vertex ) == 0 || bAllUniqueVertices) 
                {
                    uniqueVertices[ vertex ] = static_cast<U32>( Vertices.size() );
                    
                    Vertices.push_back( vertex );
                }

                Indices.push_back( uniqueVertices[ vertex ] );
            }
        }
        
        // TVector<Vec3> tangents  ( Vertices.size(), Vec3(0.0f) );
        // TVector<Vec3> bitangents( Vertices.size(), Vec3(0.0f) );
        // Vec3          tanBasis[ 2 ];
        
        // // Compute Tangent Basis for each triangle.
        // for (size_t i = 0; i < Indices.size(); i += 3) // <- 3
        // {
        //     computeTangentBasis(
        //         Vertices.at( Indices[ i     ] ), 
        //         Vertices.at( Indices[ i + 1 ] ), 
        //         Vertices.at( Indices[ i + 2 ] ), 
        //         tanBasis
        //     );

        //     tangents  .at( Indices[ i     ] ) += tanBasis[ 0 ];
        //     tangents  .at( Indices[ i + 1 ] ) += tanBasis[ 0 ];
        //     tangents  .at( Indices[ i + 2 ] ) += tanBasis[ 0 ];
        //     bitangents.at( Indices[ i     ] ) += tanBasis[ 1 ];
        //     bitangents.at( Indices[ i + 1 ] ) += tanBasis[ 1 ];
        //     bitangents.at( Indices[ i + 2 ] ) += tanBasis[ 1 ];
        // }
        
        // // Assign oriented Tangent Basis to each vertex.
        // for (size_t i = 0; i < Vertices.size(); i++) 
        // {
        //     Vec3 N = Vertices.at( i ).Normal;
        //     Vec3 T = tangents.at( i );

        //     // Re-Orthogonalize, then Normalize.
        //     T = glm::normalize( T - (glm::dot( T, N ) * N) );
            
        //     float w = glm::dot( glm::cross( N, T ), bitangents.at( i ) ) < 0.0f 
        //         ? -1.0f 
        //         : 1.0f;
            
        //     Vertices.at( i ).Tangent = { T, w };
        // }

        return true;
    }

    // VyModel::VyModel(const Builder& builder)
    // {
	// 	m_Submeshes               = std::move( builder.Submeshes               );
	// 	m_TexturePaths            = std::move( builder.TexturePaths            );
	// 	m_MaterialIdToTexturePath = std::move( builder.MaterialIdToTexturePath );

	// 	// Get bounding box from builder.
	// 	m_AABB = builder.AABB;
    // }

    // =====================================================================================================================

    VyModel::VyModel(const Data& data)
    {
        createVertexBuffer(data.Vertices);
        createIndexBuffer (data.Indices );
    }

    
    VyModel::~VyModel() 
    {
    }

    // =====================================================================================================================

    // Shared<VyModel> VyModel::createFromFile(const TString& filepath)
    // {
    //     Builder builder{};
    //     builder.loadModel( filepath );

    //     return MakeShared<VyModel>( builder );
    // }


    Shared<VyModel> VyModel::loadFromFile(const TPath& path)
    {
        Data data{};

        if (!data.loadModel( path )) // bAllUniqueVertices
        {
            VY_ERROR_TAG("VyModel", "Failed to load Model file: {0}", path.string());
            
            return nullptr;
        }

		std::stringstream ss;
        ss  << GRAY "Loaded Model: " RESET << CYAN "'" << path.string() << "'" RESET << '\n'
            << " - Total Indices  : " << data.Indices .size() << '\n'
            << " - Total Vertices : " << data.Vertices.size() << '\n'
        ;

        VY_TRACE_TAG("VyModel", "{0}", ss.str());

        return MakeShared<VyModel>( data );
    }


    void VyModel::createDescriptorSets(VyDescriptorSetLayout& setLayout, VyDescriptorPool& pool)
    {
        m_MaterialBuffer = MakeUnique<VyBuffer>( VyBuffer::uniformBuffer("material", sizeof(MaterialUbo), 1 ) );

        // update material UBO
        MaterialUbo materialUbo;
        {
            materialUbo.ModelMatrix        = m_MaterialSet.Transform;
            materialUbo.ModelInverseMatrix = m_MaterialSet.InverseTransform;
            
            materialUbo.AlbedoNormalEmissiveOcclusionTextureHandles.x = m_MaterialSet.AlbedoTextureHandle;
            materialUbo.AlbedoNormalEmissiveOcclusionTextureHandles.y = m_MaterialSet.NormalTextureHandle;
            materialUbo.AlbedoNormalEmissiveOcclusionTextureHandles.z = m_MaterialSet.EmissiveTextureHandle;
            materialUbo.AlbedoNormalEmissiveOcclusionTextureHandles.w = m_MaterialSet.OcclusionTextureHandle;
            
            materialUbo.NormalOcclusionRoughnessMetallicFactor.x = m_MaterialSet.NormalScale;
            materialUbo.NormalOcclusionRoughnessMetallicFactor.y = m_MaterialSet.OcclusionFactor;
            materialUbo.NormalOcclusionRoughnessMetallicFactor.z = m_MaterialSet.RoughnessFactor;
            materialUbo.NormalOcclusionRoughnessMetallicFactor.w = m_MaterialSet.MetallicFactor;
            
            materialUbo.RoughnessMetallicFlags.x = m_MaterialSet.RoughnessMetalnessTextureHandle;
            materialUbo.RoughnessMetallicFlags.y = m_MaterialSet.RoughnessMetalnessTextureHandle;
            materialUbo.RoughnessMetallicFlags.z = m_Flags;
            
            materialUbo.AlbedoFactor   = m_MaterialSet.AlbedoFactor;
            materialUbo.EmissiveFactor = m_MaterialSet.EmissiveFactor;
        }

        m_MaterialBuffer->write( &materialUbo, sizeof(MaterialUbo), 0 );

        auto materialInfo = m_MaterialBuffer->descriptorBufferInfo();

        auto writer = VyDescriptorWriter( setLayout, pool )
            .writeBuffer(0, &materialInfo);

        writer.build( m_MeshDescriptorSet );
    }

    // =====================================================================================================================

    void VyModel::bind(VkCommandBuffer cmdBuffer) const
    {
        VkBuffer     buffers[] = { m_VertexBuffer->handle() };
        VkDeviceSize offsets[] = { 0 };

        vkCmdBindVertexBuffers(cmdBuffer, 0, 1, buffers, offsets);
        
        if (m_HasIndexBuffer) 
        {
            vkCmdBindIndexBuffer(cmdBuffer, m_IndexBuffer->handle(), 0, VK_INDEX_TYPE_UINT32);
        }
    }

    // =====================================================================================================================

    void VyModel::draw(VkCommandBuffer cmdBuffer) const
    {
        if (m_HasIndexBuffer) 
        {
            vkCmdDrawIndexed(cmdBuffer, m_IndexCount, 1, 0, 0, 0);
        } 
        else 
        {
            vkCmdDraw(cmdBuffer, m_VertexCount, 1, 0, 0);
        }
    }

    void VyModel::drawIndexed(VkCommandBuffer cmdBuffer, U32 indexCount, U32 firstIndex) const
    {
        if (m_HasIndexBuffer) 
        {
            vkCmdDrawIndexed(cmdBuffer, indexCount, 1, firstIndex, 0, 0);
        } 
        else 
        {
            vkCmdDraw(cmdBuffer, indexCount, 1, firstIndex, 0);
        }
    }


    // =====================================================================================================================

    void VyModel::createVertexBuffer(const TVector<VyVertex>& vertices) 
    {
        m_VertexCount = static_cast<U32>( vertices.size() );

        VY_ASSERT(m_VertexCount >= 3, "Vertex count must be at least 3");
        
        U32 vertexSize = sizeof(vertices[0]);

        m_VertexBuffer = MakeUnique<VyBuffer>( VyBuffer::vertexBuffer( "model", vertexSize, m_VertexCount ) );
        
        m_VertexBuffer->upload( vertices );
    }

    // =====================================================================================================================

    void VyModel::createIndexBuffer(const TVector<U32>& indices) 
    {
        m_IndexCount     = static_cast<U32>( indices.size() );
        m_HasIndexBuffer = m_IndexCount > 0;
        
        if (!m_HasIndexBuffer) { return; }

        U32 indexSize = sizeof(indices[0]);

        m_IndexBuffer = MakeUnique<VyBuffer>( VyBuffer::indexBuffer( "model", indexSize, m_IndexCount ) );
        
        m_IndexBuffer->upload( indices );
    }

    // =====================================================================================================================

    bool VyModel::Data::loadModelAssimp(const TPath& path)
    {
        // Create Assimp importer
        Assimp::Importer importer;
        
        // Import settings for better compatibility and performance
        const aiScene* pScene = importer.ReadFile(path.string(), 
            aiProcess_Triangulate |
            aiProcess_FlipUVs |
            aiProcess_GenNormals |
            aiProcess_CalcTangentSpace |
            aiProcess_JoinIdenticalVertices |
            aiProcess_SortByPType |
            aiProcess_ImproveCacheLocality |
            aiProcess_OptimizeMeshes |
            aiProcess_OptimizeGraph |
            aiProcess_ValidateDataStructure
        );

        // Check for errors
        if (!pScene || pScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !pScene->mRootNode) 
        {
            std::cerr << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;

            return false;
        }

        // Clear existing data
        Vertices.clear();
        Indices .clear();

        // Process the root node recursively
        processNode( pScene->mRootNode, pScene );

        return true;
    }

    // =====================================================================================================================

    void VyModel::Data::processNode(aiNode* pNode, const aiScene* pScene)
    {
        // Process all the node's meshes.
        for (U32 m = 0; m < pNode->mNumMeshes; m++) 
        {
            aiMesh* pMesh = pScene->mMeshes[ pNode->mMeshes[ m ] ];
            
            processMesh( pMesh, pScene );
        }

        // Process all the node's children
        for (U32 c = 0; c < pNode->mNumChildren; c++) 
        {
            processNode( pNode->mChildren[ c ], pScene );
        }
    }

    // =====================================================================================================================

    void VyModel::Data::processMesh(aiMesh* pMesh, const aiScene* pScene)
    {
        // Record current vertex count to offset indices for this mesh
        // const U32 perMeshVertex = static_cast<U32>( Vertices.size() );
        
        // [ VERTICES ]
        for (U32 v = 0; v < pMesh->mNumVertices; v++) 
        {
            VyVertex vertex{};

            // [ Position ]
            if (pMesh->mVertices) 
            {
                vertex.Position = Utils::AiToGlm( pMesh->mVertices[ v ] );
            }
            else {
                VY_THROW_RUNTIME_ERROR("No vertices found!");
            }

            // [ Color ]
            if (pMesh->HasVertexColors( v )) 
            {
                vertex.Color = Utils::AiToGlm( pMesh->mColors[ 0 ][ v ] );
            } 
            else {
                vertex.Color = { 1.0f, 1.0f, 1.0f }; // Default white
            }

            // [ Normal ]
            if (pMesh->HasNormals()) 
            {
                vertex.Normal = Utils::AiToGlm( pMesh->mNormals[ v ] );
            } 
            else {
                vertex.Normal = { 0.0f, 1.0f, 0.0f }; // Default up normal
            }

            // [ UV ]
            if (pMesh->HasTextureCoords( 0 )) 
            {
                vertex.UV = Utils::AiToGlm( pMesh->mTextureCoords[ 0 ][ v ] );
            } 
            else {
                vertex.UV = { 0.0f, 0.0f };
            }

            // [ Tangents ]
            // if (pMesh->HasTangentsAndBitangents()) 
            // {
            //     vertex.Tangent   = Utils::AiToGlm( pMesh->mTangents  [ v ] );
            //     vertex.Bitangent = Utils::AiToGlm( pMesh->mBitangents[ v ] );
            // }
            // else {
            //     // you can orthonormalize later in the shader or generate here
            //     vertex.Tangent   = { 1.0f, 0.0f, 0.0f };
            //     vertex.Bitangent = { 0.0f, 1.0f, 0.0f };
            // }

            Vertices.push_back( vertex );
        }

        // [ INDICES ]
        for (U32 f = 0; f < pMesh->mNumFaces; f++) 
        {
            const aiFace& face = pMesh->mFaces[ f ];
            
            for (U32 index = 0; index < face.mNumIndices; index++) 
            {
                Indices.push_back( /*perMeshVertex +*/ face.mIndices[ index ] );
            }
        }

        // Process material
        if (pMesh->mMaterialIndex >= 0) 
        {
            // aiMaterial* pMaterial = pScene->mMaterials[ pMesh->mMaterialIndex ];

            // TVector<VyTexture> diffuseMaps = loadMaterialTextures(pMaterial, aiTextureType_DIFFUSE, "texture_diffuse");
            // Textures.insert( Textures.end(), diffuseMaps.begin(), diffuseMaps.end() );
            
            // TVector<VyTexture> specularMaps = loadMaterialTextures(pMaterial, aiTextureType_SPECULAR, "texture_specular");
            // Textures.insert( Textures.end(), specularMaps.begin(), specularMaps.end() );
        }
    }

    // =====================================================================================================================


    // TVector<VyTexture> VyModel::Data::loadMaterialTextures(aiMaterial* pMaterial, aiTextureType type, const TPath& texDirectory)
    // {
        // TVector<VyTexture> textures;

        // for (U32 i = 0; i < pMaterial->GetTextureCount( type ); i++)
        // {
        //     aiString path;
        //     pMaterial->GetTexture(type, i, &path);
        //     bool bSkip = false;

        //     for (U32 j = 0; j < LoadedTextures.size(); j++)
        //     {
        //         const size_t index = TString( LoadedTextures[j]->filepath().c_str() ).rfind('/');

        //         if (TString texName = TString( LoadedTextures[j]->filepath().c_str() ).substr( index + 1 ); 
        //             texName == path.C_Str())
        //         {
        //             textures.emplace_back( LoadedTextures[ j ] );
        //             bSkip = true;
        //             break;
        //         }
        //     }
        //     if (!bSkip)
        //     {
        //         const size_t index = TString( path.C_Str() ).rfind('\\');

        //         TString       texStr  = TString( path.data ).substr( index + 1 );
        //         const TString pathStr = texDirectory.string() + '/' + texStr;

        //         Shared<VyTexture> texture = VyTexture::createFromFile(pathStr);
        //         // VyTexture texture = VulkanTexture(pathStr, typeName, true);

        //         textures      .emplace_back( texture );
        //         LoadedTextures.emplace_back( texture );
        //     }
        // }

        // return textures;
    // }

    // void VyModel::Data::loadMaterialTextures(aiMaterial* pMaterial, aiTextureType type, const TPath& texDirectory)
    // {
    //     for (U32 i = 0; i < pMaterial->GetTextureCount(type); i++) 
    //     {
    //         aiString str;
    //         pMaterial->GetTexture(type, i, &str);
    //         TString textureFilename(str.C_Str());
    //         TString texturePath = "/Users/cangok/Documents/workspace/Game Engine Super/" + texDirectory.string();

    //         VY_INFO_TAG("VyModel", "Loading texture from path: {}", texturePath);
            
    //         auto texture = VyTexture::createFromFilepath( texturePath );
            
    //         Textures.push_back( texture );
    //     }

    //     // If no texture found, use a default texture
    //     if (pMaterial->GetTextureCount(type) == 0) 
    //     {
    //         VY_INFO_TAG("VyModel", "No diffuse textures found. Using default texture.");

    //         TString defaultTexturePath = "/Users/cangok/Documents/workspace/Game Engine Super/textures/default.png";
            
    //         auto defaultTexture = VyTexture::createFromFilepath( defaultTexturePath );
            
    //         Textures.push_back( defaultTexture );
    //     }
    // }


}