#include <Vy/GFX/Resources/Mesh.h>

#include <Vy/GFX/Context.h>


#include <meshoptimizer.h>

#include <Vy/GFX/Resources/Importers/OBJImporter.h>
#include <Vy/GFX/Resources/Importers/GLTFImporter.h>

namespace Vy
{
    Model::Model(const Builder& builder) : 
        m_Materials{builder.Materials}, 
        m_SubMeshes{builder.SubMeshes}, 
        m_Animations{builder.Animations}, 
        m_Nodes{builder.Nodes},
        m_MorphTargetSets{builder.MorphTargetSets}, 
        m_FilePath{builder.FilePath}
    {
        createVertexBuffers(builder.Vertices);
        createIndexBuffers(builder.Indices);
        generateMeshlets(builder.Vertices, builder.Indices);
    }

    
    Unique<Model> Model::createModelFromFile(const String& filepath, bool flipX, bool flipY, bool flipZ)
    {
        // std::cout << "[" << GREEN << "Model" << RESET << "]: Loading model from file: " << filepath << std::endl;
        Builder builder;
        builder.loadModelFromFile(filepath, flipX, flipY, flipZ);
        // std::cout << "[" << GREEN << "Model" << RESET << "]: " << filepath << " with " << builder.vertices.size() << " vertices " << std::endl;
        return std::make_unique<Model>(builder);
        return nullptr;
    }


    Model::~Model() = default;


    void Model::bind(VkCommandBuffer commandBuffer) const
    {
        VkBuffer     buffers[] = {m_VertexBuffer->handle()};
        VkDeviceSize offsets[] = {0};

        vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);

        if (m_IndexBuffer)
        {
            vkCmdBindIndexBuffer(commandBuffer, m_IndexBuffer->handle(), 0, VK_INDEX_TYPE_UINT32);
        }
    }


    void Model::draw(VkCommandBuffer commandBuffer) const
    {
        if (m_IndexBuffer)
        {
            vkCmdDrawIndexed(commandBuffer, m_IndexCount, 1, 0, 0, 0);
            return;
        }
        else
        {
            vkCmdDraw(commandBuffer, m_VertexCount, 1, 0, 0);
        }
    }


    void Model::drawSubMesh(VkCommandBuffer commandBuffer, size_t subMeshIndex) const
    {
        if (subMeshIndex >= m_SubMeshes.size())
        {
            return;
        }

        const auto& subMesh = m_SubMeshes[subMeshIndex];

        if (m_IndexBuffer)
        {
            vkCmdDrawIndexed(commandBuffer, subMesh.IndexCount, 1, subMesh.IndexOffset, 0, 0);
        }
        else
        {
            vkCmdDraw(commandBuffer, subMesh.IndexCount, 1, subMesh.IndexOffset, 0);
        }
    }


    void Model::bindAlternateVertexBuffer(VkCommandBuffer commandBuffer, VkBuffer alternateVertexBuffer) const
    {
        VkBuffer     buffers[] = {alternateVertexBuffer};
        VkDeviceSize offsets[] = {0};

        vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);

        if (m_IndexBuffer)
        {
            vkCmdBindIndexBuffer(commandBuffer, m_IndexBuffer->handle(), 0, VK_INDEX_TYPE_UINT32);
        }
    }


    void Model::createIndexBuffers(const TVector<U32>& indices)
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

        m_IndexBuffer = std::make_unique<VyBuffer>( 
            VyBuffer::indexBuffer(
                indexSize, 
                m_IndexCount, 
                VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT
            ) 
        );

        // copy data from staging buffer to index buffer
        VyContext::device().copyBuffer(stagingBuffer.handle(), m_IndexBuffer->handle(), bufferSize);
    }


    void Model::createVertexBuffers(const TVector<Vertex>& vertices)
    {
        m_VertexCount = static_cast<U32>(vertices.size());
        
        assert(m_VertexCount >= 3 && "Vertex count must be at least 3");

        VkDeviceSize bufferSize = sizeof(vertices[0]) * m_VertexCount;
        U32          vertexSize = sizeof(vertices[0]);

        VyBuffer stagingBuffer{ VyBuffer::stagingBuffer(vertexSize, m_VertexCount) };

        stagingBuffer.map();
        stagingBuffer.writeToBuffer((void*)vertices.data(), bufferSize);

        m_VertexBuffer = std::make_unique<VyBuffer>( 
            VyBuffer::vertexBuffer(
                vertexSize, 
                m_VertexCount, 
                VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT
            ) 
        );

        // copy data from staging buffer to vertex buffer
        VyContext::device().copyBuffer(stagingBuffer.handle(), m_VertexBuffer->handle(), bufferSize);
    }


    TVector<VkVertexInputBindingDescription> Model::Vertex::getBindingDescriptions()
    {
        return {
            {
                    .binding   = 0,
                    .stride    = sizeof(Vertex),
                    .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
            },
        };
    }


    TVector<VkVertexInputAttributeDescription> Model::Vertex::getAttributeDescriptions()
    {
        TVector<VkVertexInputAttributeDescription> attributeDescriptions;
        attributeDescriptions.reserve(4);

        // Position attribute
        attributeDescriptions.push_back({
                .location = 0,
                .binding  = 0,
                .format   = VK_FORMAT_R32G32B32_SFLOAT,
                .offset   = offsetof(Vertex, Position),
        });
        // Color attribute
        attributeDescriptions.push_back({
                .location = 1,
                .binding  = 0,
                .format   = VK_FORMAT_R32G32B32_SFLOAT,
                .offset   = offsetof(Vertex, Color),
        });
        // Normal attribute
        attributeDescriptions.push_back({
                .location = 2,
                .binding  = 0,
                .format   = VK_FORMAT_R32G32B32_SFLOAT,
                .offset   = offsetof(Vertex, Normal),
        });
        // UV attribute
        attributeDescriptions.push_back({
                .location = 3,
                .binding  = 0,
                .format   = VK_FORMAT_R32G32_SFLOAT,
                .offset   = offsetof(Vertex, UV),
        });

        return attributeDescriptions;
    }


    void Model::Builder::loadModelFromFile(const String& filepath, bool flipX, bool flipY, bool flipZ)
    {
        FilePath = filepath;
        OBJImporter importer;

        if (!importer.load(*this, filepath, flipX, flipY, flipZ))
        {
            throw std::runtime_error("Failed to load OBJ file: " + filepath);
        }
    }


    Unique<Model> Model::createModelFromGLTF(const String& filepath, bool flipX, bool flipY, bool flipZ)
    {
        VY_INFO_TAG("Model", "Loading glTF model from file: {0}", filepath);

        Builder builder;
        builder.loadModelFromGLTF(filepath, flipX, flipY, flipZ);

        VY_INFO_TAG("Model", "{0}, with {1} vertices", filepath, builder.Vertices.size());

        return std::make_unique<Model>(builder);
    }


    void Model::Builder::loadModelFromGLTF(const String& filepath, bool flipX, bool flipY, bool flipZ)
    {
        FilePath = filepath;
        GLTFImporter importer;

        if (!importer.load(*this, filepath, flipX, flipY, flipZ))
        {
            throw std::runtime_error("Failed to load glTF file: " + filepath);
        }
    }


    size_t Model::memorySize() const
    {
        size_t totalSize = 0;

        // Vertex buffer
        totalSize += m_VertexCount * sizeof(Vertex);

        // Index buffer
        if (m_IndexBuffer)
        {
            totalSize += m_IndexCount * sizeof(U32);
        }

        return totalSize;
    }


    void Model::generateMeshlets(const TVector<Vertex>& vertices, const TVector<U32>& indices)
    {
        if (indices.empty())
        {
            return;
        }

        const size_t maxVertices  = 64;
        const size_t maxTriangles = 124;
        const float  coneWeight   = 0.0f;

        // Clear existing meshlets
        m_Meshlets.clear();
        TVector<unsigned int>  allMeshletVertices;
        TVector<unsigned char> allMeshletTriangles;

        // If no submeshes, create a default one
        if (m_SubMeshes.empty())
        {
            SubMesh sm{};
            {
                sm.IndexOffset = 0;
                sm.IndexCount  = static_cast<U32>(indices.size());
                sm.MaterialId  = 0;
            }

            m_SubMeshes.push_back(sm);
        }

        for (auto& subMesh : m_SubMeshes)
        {
            size_t maxMeshlets = meshopt_buildMeshletsBound(subMesh.IndexCount, maxVertices, maxTriangles);

            TVector<meshopt_Meshlet> localMeshlets(maxMeshlets);
            TVector<unsigned int>    localMeshletVertices(maxMeshlets * maxVertices);
            TVector<unsigned char>   localMeshletTriangles(maxMeshlets * maxTriangles * 3);

            size_t meshletCount = meshopt_buildMeshlets(
                localMeshlets.data(),
                localMeshletVertices.data(),
                localMeshletTriangles.data(),
                &indices[subMesh.IndexOffset],
                subMesh.IndexCount,
                &vertices[0].Position.x,
                vertices.size(),
                sizeof(Vertex),
                maxVertices,
                maxTriangles,
                coneWeight
            );

            const meshopt_Meshlet& last = localMeshlets[ meshletCount - 1 ];

            localMeshletVertices .resize(last.vertex_offset + last.vertex_count);
            localMeshletTriangles.resize(last.triangle_offset + ((last.triangle_count * 3 + 3) & ~3));

            // Update SubMesh info
            subMesh.MeshletOffset = static_cast<U32>(m_Meshlets.size());
            subMesh.MeshletCount  = static_cast<U32>(meshletCount);

            // Offsets for this batch
            U32 vertexOffsetBase   = static_cast<U32>(allMeshletVertices .size());
            U32 triangleOffsetBase = static_cast<U32>(allMeshletTriangles.size());

            // Append data
            allMeshletVertices .insert(allMeshletVertices .end(), localMeshletVertices .begin(), localMeshletVertices .end());
            allMeshletTriangles.insert(allMeshletTriangles.end(), localMeshletTriangles.begin(), localMeshletTriangles.end());

            for (size_t i = 0; i < meshletCount; ++i)
            {
                const auto& m = localMeshlets[i];

                meshopt_Bounds bounds = meshopt_computeMeshletBounds(
                    &localMeshletVertices[m.vertex_offset],
                    &localMeshletTriangles[m.triangle_offset],
                    m.triangle_count,
                    &vertices[0].Position.x,
                    vertices.size(),
                    sizeof(Vertex)
                );

                Meshlet myMeshlet{};
                {
                    myMeshlet.VertexOffset   = m.vertex_offset + vertexOffsetBase;
                    myMeshlet.TriangleOffset = m.triangle_offset + triangleOffsetBase;
                    myMeshlet.VertexCount    = m.vertex_count;
                    myMeshlet.TriangleCount  = m.triangle_count;
                    
                    memcpy(myMeshlet.Center, bounds.center, sizeof(float) * 3);
                    myMeshlet.Radius = bounds.radius;

                    memcpy(myMeshlet.ConeAxis, bounds.cone_axis, sizeof(float) * 3);
                    myMeshlet.ConeCutoff = bounds.cone_cutoff;
                }

                m_Meshlets.push_back(myMeshlet);
            }
        }

        // Create buffers

        // Meshlet Buffer
        {
            VkDeviceSize bufferSize = sizeof(Meshlet) * m_Meshlets.size();

            VyBuffer stagingBuffer{ VyBuffer::stagingBuffer(sizeof(Meshlet), static_cast<U32>(m_Meshlets.size())) };

            stagingBuffer.map();
            stagingBuffer.writeToBuffer(m_Meshlets.data());

            m_MeshletBuffer = std::make_unique<VyBuffer>( 
                VyBuffer::storageBuffer(
                    sizeof(Meshlet), 
                    static_cast<U32>(m_Meshlets.size()), 
                    VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT
                ) 
            );

            VyContext::device().copyBuffer(stagingBuffer.handle(), m_MeshletBuffer->handle(), bufferSize);
            // VK_PIPELINE_STAGE_ALL_COMMANDS_BIT
            // VK_ACCESS_SHADER_READ_BIT
        }

        // Meshlet Vertices Buffer
        {
            VkDeviceSize bufferSize = sizeof(unsigned int) * allMeshletVertices.size();

            VyBuffer stagingBuffer{ VyBuffer::stagingBuffer(sizeof(unsigned int), static_cast<U32>(allMeshletVertices.size())) };

            stagingBuffer.map();
            stagingBuffer.writeToBuffer(allMeshletVertices.data());

            m_MeshletVerticesBuffer = std::make_unique<VyBuffer>(
                VyBuffer::storageBuffer(
                    sizeof(unsigned int),
                    static_cast<U32>(allMeshletVertices.size()),
                    VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT
                ) 
            );

            VyContext::device().copyBuffer(stagingBuffer.handle(), m_MeshletVerticesBuffer->handle(), bufferSize);
            // VK_PIPELINE_STAGE_ALL_COMMANDS_BIT
            // VK_ACCESS_SHADER_READ_BIT
        }

        // Meshlet Triangles Buffer
        {
            VkDeviceSize bufferSize = sizeof(unsigned char) * allMeshletTriangles.size();

            VyBuffer stagingBuffer{ VyBuffer::stagingBuffer(sizeof(unsigned char), static_cast<U32>(allMeshletTriangles.size())) };

            stagingBuffer.map();
            stagingBuffer.writeToBuffer(allMeshletTriangles.data());

            m_MeshletTrianglesBuffer = std::make_unique<VyBuffer>(
                VyBuffer::storageBuffer(
                    sizeof(unsigned char),
                    static_cast<U32>(allMeshletTriangles.size()),
                    VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT
                ) 
            );
            
            VyContext::device().copyBuffer(stagingBuffer.handle(), m_MeshletTrianglesBuffer->handle(), bufferSize);
            // VK_PIPELINE_STAGE_ALL_COMMANDS_BIT
            // VK_ACCESS_SHADER_READ_BIT
        }

        VY_INFO_TAG("Model", "Generated {0} meshlets", m_Meshlets.size());
    }
}

// #include <Vy/Asset/AssetManager.h>
// #include <Vy/Engine.h>

// #include <assimp/Importer.hpp>
// #include <assimp/scene.h>
// #include <assimp/postprocess.h>

// namespace Vy
// {
// 	TVector<VkVertexInputBindingDescription> Vertex::getBindingDescriptions()
// 	{
// 		TVector<VkVertexInputBindingDescription> bindingDescriptions(1);
// 		bindingDescriptions[0].binding = 0;
// 		bindingDescriptions[0].stride = sizeof(Vertex);
// 		bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
// 		return bindingDescriptions;
// 	}

// 	TVector<VkVertexInputAttributeDescription> Vertex::getAttributeDescriptions()
// 	{
// 		TVector<VkVertexInputAttributeDescription> attributeDescriptions;

// 		auto& position = attributeDescriptions.emplace_back();
// 		position.binding = 0;
// 		position.location = 0;
// 		position.format = VK_FORMAT_R32G32B32_SFLOAT;
// 		position.offset = offsetof(Vertex, Position);

// 		auto& normal = attributeDescriptions.emplace_back();
// 		normal.binding = 0;
// 		normal.location = 1;
// 		normal.format = VK_FORMAT_R32G32B32_SFLOAT;
// 		normal.offset = offsetof(Vertex, Normal);

// 		auto& textureCoord = attributeDescriptions.emplace_back();
// 		textureCoord.binding = 0;
// 		textureCoord.location = 2;
// 		textureCoord.format = VK_FORMAT_R32G32_SFLOAT;
// 		textureCoord.offset = offsetof(Vertex, TextureCoord);

// 		auto& tangent = attributeDescriptions.emplace_back();
// 		tangent.binding = 0;
// 		tangent.location = 3;
// 		tangent.format = VK_FORMAT_R32G32_SFLOAT;
// 		tangent.offset = offsetof(Vertex, Tangent);

// 		return attributeDescriptions;
// 	}






// 	namespace {
// 		Path GetPathRelativeToMesh(const Path& meshPath, const Path& path)
// 		{
// 			const auto parentPath = meshPath.parent_path();
// 			return parentPath / path;
// 		}

// 		TVector<VyAssetHandle> ProcessMaterialsFromScene(const Path& modelPath, const aiScene* scene) 
//         {
// 			auto& assetManager = VyEngine::assetManager();
// 			// const auto& renderer = Application::Get().getRenderer();

// 			TVector<VyAssetHandle> result;

// 			for (U32 i = 0; i < scene->mNumMaterials; i++)
// 			{
// 				aiMaterial* pMaterial = scene->mMaterials[i];
// 				MaterialData mat;

// 				aiString texturePath;
// 				aiColor4D textureColor;
// 				ai_real textureFloat;

// 				const auto whiteHandle = assetManager.getHandleFromRelative("resources/textures/white.jpg");

// 				if (AI_SUCCESS == aiGetMaterialColor(pMaterial, AI_MATKEY_COLOR_DIFFUSE, &textureColor))
//                 {
//                     mat.AlbedoColor = Vec4(textureColor.r, textureColor.g, textureColor.b, textureColor.a);
//                 }
// 				if (AI_SUCCESS == pMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath))
//                 {
//                     mat.AlbedoMap = assetManager.getHandleFromAbsolute(GetPathRelativeToMesh(modelPath, texturePath.C_Str()));
//                 }
// 				else
//                 {
//                     mat.AlbedoMap = whiteHandle;
//                 }

// 				if (AI_SUCCESS == aiGetMaterialFloat(pMaterial, AI_MATKEY_METALLIC_FACTOR, &textureFloat))
//                 {
//                     mat.Metallic = textureFloat;
//                 }
// 				if (AI_SUCCESS == aiGetMaterialFloat(pMaterial, AI_MATKEY_ROUGHNESS_FACTOR, &textureFloat))
//                 {
//                     mat.Roughness = textureFloat;
//                 }
// 				if (AI_SUCCESS == pMaterial->GetTexture(aiTextureType_METALNESS, 0, &texturePath))
//                 {
//                     mat.MetallicRoughnessMap = assetManager.getHandleFromAbsolute(GetPathRelativeToMesh(modelPath, texturePath.C_Str()));
//                 }
// 				else
//                 {
//                     mat.MetallicRoughnessMap = whiteHandle;
//                 }

// 				if (AI_SUCCESS == pMaterial->GetTexture(aiTextureType_NORMALS, 0, &texturePath))
//                 {
//                     mat.NormalMap = assetManager.getHandleFromAbsolute(GetPathRelativeToMesh(modelPath, texturePath.C_Str()));
//                 }
// 				else
//                 {
//                     mat.NormalMap = assetManager.getHandleFromRelative("resources/textures/normal.jpg");
//                 }

// 				if (AI_SUCCESS == pMaterial->GetTexture(aiTextureType_AMBIENT_OCCLUSION, 0, &texturePath))
//                 {
//                     mat.AOMap = assetManager.getHandleFromAbsolute(GetPathRelativeToMesh(modelPath, texturePath.C_Str()));
//                 }
// 				else
//                 {
//                     mat.AOMap = whiteHandle;
//                 }

// 				if (AI_SUCCESS == aiGetMaterialColor(pMaterial, AI_MATKEY_EMISSIVE_INTENSITY, &textureColor))
//                 {
//                     mat.EmissiveFactor = Vec3(textureColor.r, textureColor.g, textureColor.b);
//                 }
// 				if (AI_SUCCESS == pMaterial->GetTexture(aiTextureType_EMISSIVE, 0, &texturePath))
//                 {
//                     mat.EmissiveMap = assetManager.getHandleFromAbsolute(GetPathRelativeToMesh(modelPath, texturePath.C_Str()));
//                 }
// 				else
//                 {
//                     mat.EmissiveMap = whiteHandle;
//                 }

// 				VyAssetHandle matHandle;
// 				assetManager.addAssetEntry(std::make_shared<Material>(mat, matHandle));

// 				result.push_back(matHandle);
// 			}

// 			return result;
// 		}
// 	}


// 	Model Model::fromFile(const Path& absolutePath)
// 	{
// 		Model model;
// 		Assimp::Importer importer;
// 		const aiScene*   scene = importer.ReadFile(absolutePath.string(), aiProcessPreset_TargetRealtime_Fast);

// 		if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
// 		{
// 			VY_ERROR("Failed to load model : {0}", importer.GetErrorString());
// 			return {};
// 		}

// 		model.m_MaterialHandles = ProcessMaterialsFromScene(absolutePath, scene);

// 		for (U32 ind = 0; ind < scene->mNumMeshes; ind++) 
//         {
// 			auto& mesh = model.m_Meshes.emplace_back();

// 			TVector<Vertex>& vertices = mesh.getVertices();
// 			TVector<U32>&    indices  = mesh.getIndices();

// 			aiMesh* ai_mesh = scene->mMeshes[ind];

// 			mesh.m_MaterialIndex = ai_mesh->mMaterialIndex;
// 			// walk through each of the model's vertices
// 			vertices.reserve(ai_mesh->mNumVertices);

// 			for(unsigned i = 0; i < ai_mesh->mNumVertices; i++) 
//             {
// 				Vertex vertex{};

// 				vertex.Position.x = ai_mesh->mVertices[i].x;
// 				vertex.Position.y = ai_mesh->mVertices[i].y;
// 				vertex.Position.z = ai_mesh->mVertices[i].z;

// 				if (ai_mesh->HasNormals())
// 				{
// 					vertex.Normal.x = ai_mesh->mNormals[i].x;
// 					vertex.Normal.y = ai_mesh->mNormals[i].y;
// 					vertex.Normal.z = ai_mesh->mNormals[i].z;
// 				}
				
// 				if (ai_mesh->mTextureCoords[0]) // does the model contain texture coordinates?
// 				{
// 					// a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't
// 					// use models where a vertex can have multiple texture coordinates, so we always take the first set (0).
// 					vertex.TextureCoord.x = ai_mesh->mTextureCoords[0][i].x;
// 					vertex.TextureCoord.y = 1.0f - ai_mesh->mTextureCoords[0][i].y;
// 				}
// 				else
// 				{
//                     vertex.TextureCoord = Vec2(0.0f, 0.0f);
//                 }	

// 				vertices.emplace_back(vertex);
// 			}

// 			for (unsigned int faceIndex = 0; faceIndex < ai_mesh->mNumFaces; faceIndex++)
// 			{
// 				aiFace face = ai_mesh->mFaces[faceIndex];

// 				// retrieve all indices of the face and store them in the indices vector
// 				indices.reserve(indices.size() + face.mNumIndices);
// 				indices.insert(indices.end(), face.mIndices, face.mIndices + face.mNumIndices);
// 			}
// 		}

// 		return model;
// 	}
// }


// namespace Vy
// {
// 	Material Material::Builder::build()
// 	{
// 		VY_ASSERT(m_Handle.isValid(), "Invalid handle for material");

// 		return { m_MaterialData, m_Handle };
// 	}

// 	Material::Material(MaterialData materialData, VyAssetHandle assetHandle) : 
//         VyAsset(std::move(assetHandle)), 
//         m_MaterialData(std::move(materialData))
// 	{
// 		createDescriptorSet();
// 	}

// 	void Material::bind(const VyPipeline& pipeline, VkCommandBuffer commandBuffer) const
// 	{
// 		vkCmdBindDescriptorSets(
// 			commandBuffer,
// 			VK_PIPELINE_BIND_POINT_GRAPHICS,
// 			pipeline.layout(),
// 			1, 1,
// 			&m_DescriptorSet,
// 			0, nullptr);
// 	}

// 	void Material::createDescriptorSet()
// 	{
// 		// auto& renderer = Application::Get().GetRenderer();

// 		m_MaterialBuffer = std::make_unique<VyBuffer>( VyBuffer::uniformBuffer(sizeof(Vec4)) );

// 		m_MaterialBuffer->map();
// 		m_MaterialBuffer->write(&m_MaterialData.AlbedoColor, sizeof(Vec4), 0);
// 		// m_MaterialBuffer->UploadData(sizeof(Vec4), &m_MaterialData.AlbedoColor);

// 		// VyDescriptorWriter descriptorWriter(VyEngine::materialSetLayout(), VyContext::globalPool());

// 		auto& assetManager = VyEngine::assetManager();

// 		auto albedoMap = assetManager.load<Texture>(m_MaterialData.AlbedoMap);
// 		auto normalMap = assetManager.load<Texture>(m_MaterialData.NormalMap);
// 		auto metallicRoughnessMap = assetManager.load<Texture>(m_MaterialData.MetallicRoughnessMap);
// 		auto AOMap = assetManager.load<Texture>(m_MaterialData.AOMap);

// 		auto bufferInfo = m_MaterialBuffer->descriptorBufferInfo();

// 		// descriptorWriter
//         VyDescriptorWriter{ *VyEngine::materialSetLayout(), *VyContext::globalPool() }
// 			.writeImage (0, &albedoMap->descriptorImageInfo())
// 			.writeImage (1, &normalMap->descriptorImageInfo())
// 			.writeImage (2, &metallicRoughnessMap->descriptorImageInfo())
// 			.writeImage (3, &AOMap->descriptorImageInfo())
// 			.writeBuffer(4, &bufferInfo)
// 			.build( m_DescriptorSet );
// 	}


// 	void Material::reload()
// 	{
// 		createDescriptorSet();
// 	}


// 	inline Json::Value serializeVec3(const Vec3& v) 
// 	{
// 		Json::Value root;
// 		root["x"] = v.x;
// 		root["y"] = v.y;
// 		root["z"] = v.z;
// 		return root;
// 	}

// 	inline Json::Value serializeVec4(const Vec4& v) 
// 	{
// 		Json::Value root;
// 		root["x"] = v.x;
// 		root["y"] = v.y;
// 		root["z"] = v.z;
// 		root["w"] = v.w;
// 		return root;
// 	}

// 	inline Optional<Vec3> deserializeVec3(const Json::Value& root) 
// 	{
// 		if (!root.isMember("x") || 
// 			!root.isMember("y") || 
// 			!root.isMember("z"))
// 		{
// 			return std::nullopt;
// 		}

// 		Vec3 ret;
// 		ret.x = root["x"].asFloat();
// 		ret.y = root["y"].asFloat();
// 		ret.z = root["z"].asFloat();
// 		return ret;
// 	}

// 	inline Optional<Vec4> deserializeVec4(const Json::Value& root) 
// 	{
// 		if (!root.isMember("x") || 
// 			!root.isMember("y") || 
// 			!root.isMember("z") ||
// 			!root.isMember("w"))
// 		{
// 			return std::nullopt;
// 		}

// 		Vec4 ret;
// 		ret.x = root["x"].asFloat();
// 		ret.y = root["y"].asFloat();
// 		ret.z = root["z"].asFloat();
// 		ret.w = root["w"].asFloat();
// 		return ret;
// 	}

// 	bool Material::checkForDirtyInDependencies()
// 	{
// 		auto& assetManager = VyEngine::assetManager();

// 		bool isDirty = false;
// 		isDirty |= assetManager.isDirty(m_MaterialData.AlbedoMap);
// 		isDirty |= assetManager.isDirty(m_MaterialData.NormalMap);
// 		isDirty |= assetManager.isDirty(m_MaterialData.MetallicRoughnessMap);
// 		isDirty |= assetManager.isDirty(m_MaterialData.AOMap);
// 		isDirty |= assetManager.isDirty(m_MaterialData.EmissiveMap);

// 		return isDirty;
// 	}

// 	Json::Value MaterialData::serialize() const
// 	{
// 		Json::Value output;

// 		output["AlbedoMap"] = AlbedoMap.ID.toString();
// 		output["NormalMap"] = NormalMap.ID.toString();
// 		output["MetallicRoughnessMap"] = MetallicRoughnessMap.ID.toString();
// 		output["AOMap"] = AOMap.ID.toString();
// 		output["EmissiveMap"] = EmissiveMap.ID.toString();
// 		output["AlbedoColor"] = serializeVec4(AlbedoColor);
// 		output["MetallicFactor"] = Metallic;
// 		output["RoughnessFactor"] = Roughness;
// 		output["EmissiveFactor"] = serializeVec3(EmissiveFactor);

// 		return output;
// 	}




// 	Optional<MaterialData> MaterialData::deserialize(const Json::Value& root)
// 	{
// 		MaterialData data;

// 		if (root.isMember("AlbedoMap")) 
//         {
// 			data.AlbedoMap = VyAssetHandle(VyUUID::fromString(root["AlbedoMap"].asString()));
// 		}
// 		if (root.isMember("NormalMap")) 
//         {
// 			data.NormalMap = VyAssetHandle(VyUUID::fromString(root["NormalMap"].asString()));
// 		}
// 		if (root.isMember("MetallicRoughnessMap")) 
//         {
// 			data.MetallicRoughnessMap = VyAssetHandle(VyUUID::fromString(root["MetallicRoughnessMap"].asString()));
// 		}
// 		if (root.isMember("AOMap")) 
//         {
// 			data.AOMap = VyAssetHandle(VyUUID::fromString(root["AOMap"].asString()));
// 		}
// 		if (root.isMember("EmissiveMap")) 
//         {
// 			data.EmissiveMap = VyAssetHandle(VyUUID::fromString(root["EmissiveMap"].asString()));
// 		}
// 		if (root.isMember("AlbedoColor")) 
//         {
// 			data.AlbedoColor = *deserializeVec4(root["AlbedoColor"]);
// 		}
// 		if (root.isMember("MetallicFactor")) 
//         {
// 			data.Metallic = root["MetallicFactor"].asFloat();
// 		}
// 		if (root.isMember("RoughnessFactor")) 
//         {
// 			data.Roughness = root["RoughnessFactor"].asFloat();
// 		}
// 		if (root.isMember("EmissiveFactor")) 
//         {
// 			data.EmissiveFactor = *deserializeVec3(root["EmissiveFactor"]);
// 		}

// 		return data;
// 	}
// }


// namespace Vy
// {
// 	SubMeshRenderer::SubMeshRenderer(Mesh &mesh) :
// 		m_VertexCount(static_cast<U32>(mesh.getVertices().size())),
// 		m_IndexCount (static_cast<U32>(mesh.getIndices() .size())),
// 		m_VertexBuffer( VyBuffer::vertexBuffer(sizeof(mesh.getVertices()[0]) * m_VertexCount) ),
// 		m_IndexBuffer ( VyBuffer::indexBuffer (sizeof(mesh.getIndices ()[0]) * m_IndexCount ) )
// 	{
// 		submitSubMesh(mesh);
// 	}


// 	SubMeshRenderer::~SubMeshRenderer() = default;

// 	SubMeshRenderer::SubMeshRenderer(SubMeshRenderer&& other) noexcept :
// 		m_VertexCount(other.m_VertexCount),
// 		m_IndexCount(other.m_IndexCount),
// 		m_VertexBuffer(std::move(other.m_VertexBuffer)),
// 		m_IndexBuffer(std::move(other.m_IndexBuffer))
// 	{
// 	}


// 	void SubMeshRenderer::submitSubMesh(Mesh &subMesh)
// 	{
// 		VkDeviceSize bufferSize = sizeof(subMesh.getVertices()[0]) * m_VertexCount;

//         m_VertexBuffer.map();
//         m_VertexBuffer.write(subMesh.getVertices().data(), bufferSize, 0);
//         // m_VertexBuffer.unmap();

// 		// m_VertexBuffer.UploadData(bufferSize, subMesh.getVertices().data());

// 		bufferSize = sizeof(subMesh.getIndices()[0]) * m_IndexCount;

//         VyBuffer stagingBuffer{ VyBuffer::stagingBuffer(bufferSize) };

//         stagingBuffer.map();
//         stagingBuffer.write(subMesh.getIndices().data(), bufferSize, 0);
//         // stagingBuffer.unmap();

// 		// stagingBuffer.UploadData(bufferSize, subMesh.getIndices().data());
// 		// m_Device.CopyBuffer(stagingBuffer.handle(), m_IndexBuffer.handle(), bufferSize);
//         VyContext::device().copyBuffer(stagingBuffer.handle(), m_IndexBuffer.handle(), bufferSize);
//     }


// 	void SubMeshRenderer::bind(VkCommandBuffer cmdBuffer) const
// 	{
// 		const VkBuffer         buffers[] = { m_VertexBuffer.handle() };
// 		constexpr VkDeviceSize offsets[] = { 0 };

// 		vkCmdBindVertexBuffers(cmdBuffer, 0, 1, buffers, offsets);
// 		vkCmdBindIndexBuffer(cmdBuffer, m_IndexBuffer.handle(), 0, VK_INDEX_TYPE_UINT32);
// 	}


// 	void SubMeshRenderer::draw(VkCommandBuffer cmdBuffer) const
// 	{
// 		vkCmdDrawIndexed(cmdBuffer, m_IndexCount, 1, 0, 0, 0);
// 		// vkCmdDraw(cmdBuffer, m_VertexCount, 1, 0, 0);
// 	}
// }



// namespace Vy
// {
// 	MeshRenderer::MeshRenderer(VyAssetHandle handle, Model model) : 
//         VyAsset(std::move(handle)), 
//         m_Model(std::move(model))
// 	{
// 		init();
// 	}


// 	void MeshRenderer::reload()
// 	{
// 		if (!m_Handle.isValid())
// 			return;

// 		auto& assetManager = VyEngine::assetManager();

// 		auto path = assetManager.absolutePathFromHandle(this->assetHandle());

// 		auto materials = m_Model.getMaterialHandles();

// 		m_Model = Model::fromFile(path);

// 		m_Model.getMaterialHandles() = materials;

// 		m_SubMeshRenderers.clear();
// 		m_Materials       .clear();

// 		init();
// 	}


// 	void MeshRenderer::draw(const VyPipeline& pipeline, VkCommandBuffer cmdBuffer) const
// 	{
// 		for (size_t i = 0; i < m_SubMeshRenderers.size(); i++)
// 		{
// 			const auto materialIndex = m_Model.getMeshes()[i].getMaterialIndex();

// 			if (m_Materials.size() > materialIndex)
// 			{
// 				auto& materialInstance = m_Materials[materialIndex];

// 				materialInstance->bind(pipeline, cmdBuffer);
// 			}

// 			m_SubMeshRenderers[i].bind(cmdBuffer);
// 			m_SubMeshRenderers[i].draw(cmdBuffer);
// 		}
// 	}


// 	void MeshRenderer::init()
// 	{
// 		auto& assetManager = VyEngine::assetManager();

// 		for (const auto& material : m_Model.getMaterialHandles())
// 		{
// 			// auto str = material.ID.toString();
// 			auto mat = assetManager.load<Material>(material);

// 			m_Materials.emplace_back(mat);
// 		}

// 		for (auto& mesh : m_Model.getMeshes())
// 		{
// 			m_SubMeshRenderers.emplace_back(mesh);
// 		}
// 	}
// }