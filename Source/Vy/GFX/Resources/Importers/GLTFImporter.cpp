#include <Vy/GFX/Resources/Importers/GLTFImporter.h>
#include <VyLib/Util/Hash.h>

// Don't define STB_IMAGE_IMPLEMENTATION - it's already defined in Texture.cpp
// tinygltf will use the stb functions already linked
// #define STB_IMAGE_WRITE_IMPLEMENTATION
#include <tiny_gltf.h>


#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

// Hash function for Model::Vertex
namespace std 
{
    template <> 
    struct hash<Vy::Model::Vertex>
    {
        size_t operator()(Vy::Model::Vertex const& vertex) const
        {
            size_t seed = 0;
            Vy::Hash::hashCombine(seed, vertex.Position, vertex.Color, vertex.Normal, vertex.UV);
            return seed;
        }
    };
} // namespace std


namespace Vy
{
    // Helper function to get texture path from glTF, handling both URI and embedded images
    static String getTexturePath(const tinygltf::Model& model, int textureIndex, const String& baseDir, const String& cacheDir)
    {
        if (textureIndex < 0 || textureIndex >= static_cast<int>(model.textures.size()))
        {
            return "";
        }

        const tinygltf::Texture& texture = model.textures[textureIndex];

        if (texture.source < 0 || texture.source >= static_cast<int>(model.images.size()))
        {
            return "";
        }

        const tinygltf::Image& image = model.images[texture.source];

        // If image has a URI, it's an external file
        if (!image.uri.empty())
        {
            // Check if it's a data URI (base64 embedded)
            if (image.uri.find("data:") == 0)
            {
                // It's a data URI - tinygltf has already decoded it into image.image
                // We need to write it to a cache file
                String extension = ".png"; // Default to PNG

                if (image.mimeType == "image/jpeg")
                {
                    extension = ".jpg";
                }
                else if (image.mimeType == "image/png")
                {
                    extension = ".png";
                }

                String cachePath = cacheDir + "/texture_" + std::to_string(texture.source) + extension;

                // Create cache directory if it doesn't exist
                std::filesystem::create_directories(cacheDir);

                // Write the image data to file
                std::ofstream outFile(cachePath, std::ios::binary);
                if (outFile.is_open())
                {
                    outFile.write(reinterpret_cast<const char*>(image.image.data()), image.image.size());
                    outFile.close();

                    return cachePath;
                }
                else
                {
                    VY_WARN_TAG("GLTFImporter", "Failed to write cached texture: {}", cachePath);
                    return "";
                }
            }
            else
            {
                // Regular file URI - return path relative to base directory
                return baseDir + image.uri;
            }
        }
        // Image is embedded in a bufferView
        else if (image.bufferView >= 0)
        {
            // Image data is embedded in the glTF file
            // tinygltf has already loaded it into image.image
            String extension = ".png"; // Default to PNG

            if (image.mimeType == "image/jpeg")
            {
                extension = ".jpg";
            }
            else if (image.mimeType == "image/png")
            {
                extension = ".png";
            }

            String cachePath = cacheDir + "/embedded_texture_" + std::to_string(texture.source) + extension;

            // Create cache directory if it doesn't exist
            std::filesystem::create_directories(cacheDir);

            // Write the image data to file
            std::ofstream outFile(cachePath, std::ios::binary);
            if (outFile.is_open())
            {
                outFile.write(reinterpret_cast<const char*>(image.image.data()), image.image.size());
                outFile.close();
                return cachePath;
            }
            else
            {
                VY_WARN_TAG("GLTFImporter", "Failed to write embedded texture: {}", cachePath);
                return "";
            }
        }

        return "";
    }




    bool GLTFImporter::load(Model::Builder& builder, const String& filepath, bool flipX, bool flipY, bool flipZ)
    {
        tinygltf::Model    gltfModel;
        tinygltf::TinyGLTF loader;
        String             err;
        String             warn;

        // Determine file type and load
        bool ret = false;

        if (filepath.find(".glb") != String::npos)
        {
            ret = loader.LoadBinaryFromFile(&gltfModel, &err, &warn, filepath);
        }
        else
        {
            ret = loader.LoadASCIIFromFile(&gltfModel, &err, &warn, filepath);
        }

        if (!warn.empty())
        {
            VY_WARN_TAG("GLTFImporter", "Warning: {}", warn);
        }

        if (!err.empty())
        {
            VY_ERROR_TAG("GLTFImporter", "Error: {}", err);
            return false;
        }

        if (!ret)
        {
            VY_ERROR_TAG("GLTFImporter", "Failed to load glTF file: {}", filepath);
            return false;
        }

        VY_INFO_TAG("GLTFImporter", "File loaded successfully: {}", filepath);

        // Check if model has animations (we'll skip baking transforms if it does)
        bool hasAnimations = !gltfModel.animations.empty();
        if (hasAnimations)
        {
            VY_INFO_TAG("GLTFImporter", "Model has animations - vertices will remain in local space");
        }

        // Get base directory for texture paths
        String baseDir  = filepath.substr(0, filepath.find_last_of("/\\") + 1);
        String cacheDir = baseDir + ".gltf_texture_cache";

        // Flip multipliers
        float xMultiplier = flipX ? -1.0f : 1.0f;
        float yMultiplier = flipY ? -1.0f : 1.0f;
        float zMultiplier = flipZ ? -1.0f : 1.0f;

        builder.Vertices .clear();
        builder.Indices  .clear();
        builder.Materials.clear();
        builder.SubMeshes.clear();

        // Track vertex offsets and counts for each mesh primitive (for morph targets)
        // Key: "meshIndex_primitiveIndex", Value: vertex offset/count in builder.vertices
        THashMap<String, U32> primitiveVertexOffsets;
        THashMap<String, U32> primitiveVertexCounts;
        // Map from builder vertex index to original glTF position index (for morph targets)
        THashMap<U32, U32> vertexToPositionIndex;

        // Load materials first
        for (size_t i = 0; i < gltfModel.materials.size(); i++)
        {
        const auto& gltfMat = gltfModel.materials[i];

        Model::MaterialInfo matInfo;
        matInfo.Name       = gltfMat.name;
        matInfo.MaterialId = static_cast<int>(i);

        // glTF uses PBR metallic-roughness workflow
        const auto& pbr = gltfMat.pbrMetallicRoughness;

        // Base color (albedo)
        matInfo.PBRMaterial.Albedo = Vec4(pbr.baseColorFactor[0], pbr.baseColorFactor[1], pbr.baseColorFactor[2], pbr.baseColorFactor[3]);

        // Alpha Mode
        if (gltfMat.alphaMode == "MASK")
        {
            matInfo.PBRMaterial.AlphaMode   = AlphaMode::Mask;
            matInfo.PBRMaterial.AlphaCutoff = static_cast<float>(gltfMat.alphaCutoff);
        }
        else if (gltfMat.alphaMode == "BLEND")
        {
            matInfo.PBRMaterial.AlphaMode = AlphaMode::Blend;
        }
        else
        {
            matInfo.PBRMaterial.AlphaMode = AlphaMode::Opaque;
        }

        // Metallic and roughness
        matInfo.PBRMaterial.Metallic  = static_cast<float>(pbr.metallicFactor);
        matInfo.PBRMaterial.Roughness = static_cast<float>(pbr.roughnessFactor);
        matInfo.PBRMaterial.AO        = 1.0f;

        // Extract texture paths (handles both external URIs and embedded images)
        if (pbr.baseColorTexture.index >= 0)
        {
            matInfo.DiffuseTexPath = getTexturePath(gltfModel, pbr.baseColorTexture.index, baseDir, cacheDir);
        }

        if (gltfMat.normalTexture.index >= 0)
        {
            matInfo.NormalTexPath = getTexturePath(gltfModel, gltfMat.normalTexture.index, baseDir, cacheDir);
        }

        if (pbr.metallicRoughnessTexture.index >= 0)
        {
            matInfo.RoughnessTexPath                        = getTexturePath(gltfModel, pbr.metallicRoughnessTexture.index, baseDir, cacheDir);
            matInfo.PBRMaterial.UseMetallicRoughnessTexture = true;

            if (gltfMat.occlusionTexture.index == pbr.metallicRoughnessTexture.index)
            {
                matInfo.PBRMaterial.UseOcclusionRoughnessMetallicTexture = true;
            }
        }

        if (gltfMat.occlusionTexture.index >= 0)
        {
            matInfo.AOTexPath = getTexturePath(gltfModel, gltfMat.occlusionTexture.index, baseDir, cacheDir);
        }

        // Emissive Factor
        matInfo.PBRMaterial.EmissiveColor = Vec3(gltfMat.emissiveFactor[0], gltfMat.emissiveFactor[1], gltfMat.emissiveFactor[2]);

        if (gltfMat.emissiveTexture.index >= 0)
        {
            matInfo.EmissiveTexPath = getTexturePath(gltfModel, gltfMat.emissiveTexture.index, baseDir, cacheDir);
        }

        // Parse Extensions

        // Emissive Strength
        if (gltfMat.extensions.find("KHR_materials_emissive_strength") != gltfMat.extensions.end())
        {
            const auto& ext = gltfMat.extensions.at("KHR_materials_emissive_strength");

            if (ext.Has("emissiveStrength"))
            {
                matInfo.PBRMaterial.EmissiveStrength = static_cast<float>(ext.Get("emissiveStrength").GetNumberAsDouble());
            }
        }

        // Transmission
        if (gltfMat.extensions.find("KHR_materials_transmission") != gltfMat.extensions.end())
        {
            const auto& ext = gltfMat.extensions.at("KHR_materials_transmission");

            if (ext.Has("transmissionFactor"))
            {
                matInfo.PBRMaterial.Transmission = static_cast<float>(ext.Get("transmissionFactor").GetNumberAsDouble());
            }
        }

        // IOR
        if (gltfMat.extensions.find("KHR_materials_ior") != gltfMat.extensions.end())
        {
            const auto& ext = gltfMat.extensions.at("KHR_materials_ior");

            if (ext.Has("ior"))
            {
                matInfo.PBRMaterial.IOR = static_cast<float>(ext.Get("ior").GetNumberAsDouble());
            }
        }

        // Iridescence
        if (gltfMat.extensions.find("KHR_materials_iridescence") != gltfMat.extensions.end())
        {
            const auto& ext = gltfMat.extensions.at("KHR_materials_iridescence");

            if (ext.Has("iridescenceFactor"))
            {
                matInfo.PBRMaterial.Iridescence = static_cast<float>(ext.Get("iridescenceFactor").GetNumberAsDouble());
            }
            
            if (ext.Has("iridescenceIor"))
            {
                matInfo.PBRMaterial.IridescenceIOR = static_cast<float>(ext.Get("iridescenceIor").GetNumberAsDouble());
            }

            if (ext.Has("iridescenceThicknessMaximum"))
            {
                matInfo.PBRMaterial.IridescenceThickness = static_cast<float>(ext.Get("iridescenceThicknessMaximum").GetNumberAsDouble());
            }
        }

        // Clearcoat
        if (gltfMat.extensions.find("KHR_materials_clearcoat") != gltfMat.extensions.end())
        {
            const auto& ext = gltfMat.extensions.at("KHR_materials_clearcoat");

            if (ext.Has("clearcoatFactor"))
            {
                matInfo.PBRMaterial.Clearcoat = static_cast<float>(ext.Get("clearcoatFactor").GetNumberAsDouble());
            }

            if (ext.Has("clearcoatRoughnessFactor"))
            {
                matInfo.PBRMaterial.ClearcoatRoughness = static_cast<float>(ext.Get("clearcoatRoughnessFactor").GetNumberAsDouble());
            }
        }

        // Texture Transform (KHR_texture_transform)

        // We currently only support a single global UV scale, so we check textures in priority order
        const tinygltf::ExtensionMap* textureExtensions = nullptr;

        if (gltfMat.normalTexture.index >= 0 && gltfMat.normalTexture.extensions.count("KHR_texture_transform"))
        {
            textureExtensions = &gltfMat.normalTexture.extensions;
        }
        else if (pbr.baseColorTexture.index >= 0 && pbr.baseColorTexture.extensions.count("KHR_texture_transform"))
        {
            textureExtensions = &pbr.baseColorTexture.extensions;
        }
        else if (pbr.metallicRoughnessTexture.index >= 0 && pbr.metallicRoughnessTexture.extensions.count("KHR_texture_transform"))
        {
            textureExtensions = &pbr.metallicRoughnessTexture.extensions;
        }
        else if (gltfMat.occlusionTexture.index >= 0 && gltfMat.occlusionTexture.extensions.count("KHR_texture_transform"))
        {
            textureExtensions = &gltfMat.occlusionTexture.extensions;
        }

        if (textureExtensions)
        {
            const auto& ext = textureExtensions->at("KHR_texture_transform");

            if (ext.Has("scale"))
            {
                const auto& scale = ext.Get("scale");

                if (scale.IsArray() && scale.ArrayLen() >= 1)
                {
                    // Use the X scale as the uniform scale
                    matInfo.PBRMaterial.UVScale = static_cast<float>(scale.Get(0).GetNumberAsDouble());
                }
            }
        }

        builder.Materials.push_back(matInfo);

        String alphaModeStr = "OPAQUE";

        if (matInfo.PBRMaterial.AlphaMode == AlphaMode::Mask)
        {
            alphaModeStr = "MASK";
        }
        else if (matInfo.PBRMaterial.AlphaMode == AlphaMode::Blend)
        {
            alphaModeStr = "BLEND";
        }

        VY_INFO_TAG("GLTFImporter", "[Material] {0} -> PBR(albedo=<{1}, {2}, {3}>, metallic=<{4}>, roughness=<{5}>, alphaMode=<{6}>)", 
            matInfo.Name, matInfo.PBRMaterial.Albedo.r, matInfo.PBRMaterial.Albedo.g, matInfo.PBRMaterial.Albedo.b,
            matInfo.PBRMaterial.Metallic,  matInfo.PBRMaterial.Roughness, alphaModeStr
        );
        // std::cout << "[" << GREEN << " Material " << RESET << "] " << BLUE << matInfo.name << RESET << " -> PBR(albedo=" << matInfo.PBRMaterial.albedo.r << ","
                    // << matInfo.PBRMaterial.albedo.g << "," << matInfo.PBRMaterial.albedo.b << ", metallic=" << matInfo.PBRMaterial.metallic
                    // << ", roughness=" << matInfo.PBRMaterial.roughness << ", alphaMode=" << alphaModeStr << ")" << std::endl;
        }

        // Process all meshes in the scene
        const tinygltf::Scene& scene = gltfModel.scenes[gltfModel.defaultScene >= 0 ? gltfModel.defaultScene : 0];

        THashMap<Model::Vertex, U32> uniqueVertices{};
        THashMap<int, TVector<U32>>  indicesByMaterial;

        // MARK: ProcessNode 

        // Lambda to process a node recursively
        std::function<void(int, const Mat4&)> processNode = [&](int nodeIndex, const Mat4& parentTransform) 
        {
            const tinygltf::Node& node = gltfModel.nodes[nodeIndex];

            // Compute node's local transformation matrix
            Mat4 nodeTransform = Mat4(1.0f);

            if (node.matrix.size() == 16)
            {
                // Node has a transformation matrix
                nodeTransform = glm::make_mat4(node.matrix.data());
            }
            else
            {
                // Compute from TRS (Translation, Rotation, Scale)
                if (node.translation.size() == 3)
                {
                    nodeTransform = glm::translate(nodeTransform, Vec3(node.translation[0], node.translation[1], node.translation[2]));
                }

                if (node.rotation.size() == 4)
                {
                    Quat q(
                        static_cast<float>(node.rotation[3]),
                        static_cast<float>(node.rotation[0]),
                        static_cast<float>(node.rotation[1]),
                        static_cast<float>(node.rotation[2])
                    );
                    nodeTransform *= glm::mat4_cast(q);
                }

                if (node.scale.size() == 3)
                {
                    nodeTransform = glm::scale(nodeTransform, Vec3(node.scale[0], node.scale[1], node.scale[2]));
                }
            }

            // Combine with parent transformation
            Mat4 globalTransform = parentTransform * nodeTransform;

            // Process mesh if present
            if (node.mesh >= 0)
            {
                const tinygltf::Mesh& mesh      = gltfModel.meshes[node.mesh];
                int                   meshIndex = node.mesh;

                for (size_t primIdx = 0; primIdx < mesh.primitives.size(); primIdx++)
                {
                    const auto& primitive = mesh.primitives[primIdx];

                    // Record the starting vertex offset for this primitive (for morph targets)
                    U32    primitiveVertexOffset = static_cast<U32>(builder.Vertices.size());
                    String key                   = std::to_string(meshIndex) + "_" + std::to_string(primIdx);
                    primitiveVertexOffsets[key]  = primitiveVertexOffset;

                    // Check if this primitive has morph targets - if so, disable deduplication
                    bool hasMorphTargets = !primitive.targets.empty();

                    int          materialId    = primitive.material; // Get accessors for vertex attributes
                    const auto&  posAccessor   = gltfModel.accessors[primitive.attributes.at("POSITION")];
                    const auto&  posBufferView = gltfModel.bufferViews[posAccessor.bufferView];
                    const auto&  posBuffer     = gltfModel.buffers[posBufferView.buffer];
                    const float* positions     = reinterpret_cast<const float*>(&posBuffer.data[posBufferView.byteOffset + posAccessor.byteOffset]);

                    const float* normals   = nullptr;
                    const float* texCoords = nullptr;

                    if (primitive.attributes.find("NORMAL") != primitive.attributes.end())
                    {
                        const auto& normAccessor   = gltfModel.accessors[primitive.attributes.at("NORMAL")];
                        const auto& normBufferView = gltfModel.bufferViews[normAccessor.bufferView];
                        const auto& normBuffer     = gltfModel.buffers[normBufferView.buffer];
                        normals                    = reinterpret_cast<const float*>(&normBuffer.data[normBufferView.byteOffset + normAccessor.byteOffset]);
                    }

                    if (primitive.attributes.find("TEXCOORD_0") != primitive.attributes.end())
                    {
                        const auto& uvAccessor   = gltfModel.accessors[primitive.attributes.at("TEXCOORD_0")];
                        const auto& uvBufferView = gltfModel.bufferViews[uvAccessor.bufferView];
                        const auto& uvBuffer     = gltfModel.buffers[uvBufferView.buffer];
                        texCoords                = reinterpret_cast<const float*>(&uvBuffer.data[uvBufferView.byteOffset + uvAccessor.byteOffset]);
                    }

                    // Check if primitive has indices
                    if (primitive.indices < 0)
                    {
                        // No indices - use direct vertex access (not commonly used, skip for now)
                        VY_WARN_TAG("GLTFImporter", "Warning: Primitive without indices not supported yet");
                        continue;
                    }

                    // Get indices
                    const auto& indAccessor   = gltfModel.accessors[primitive.indices];
                    const auto& indBufferView = gltfModel.bufferViews[indAccessor.bufferView];
                    const auto& indBuffer     = gltfModel.buffers[indBufferView.buffer];
                    const U8*   indData       = &indBuffer.data[indBufferView.byteOffset + indAccessor.byteOffset];

                    // Process indices based on component type
                    for (size_t i = 0; i < indAccessor.count; i++)
                    {
                        U32 index = 0;

                        if (indAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
                        {
                            index = reinterpret_cast<const U16*>(indData)[i];
                        }
                        else if (indAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT)
                        {
                            index = reinterpret_cast<const U32*>(indData)[i];
                        }
                        else if (indAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE)
                        {
                            index = indData[i];
                        }

                        Model::Vertex vertex{};
                        vertex.MaterialId = materialId;

                        // Position - apply node transformation only if no animations
                        Vec3 worldPos;
                        if (hasAnimations)
                        {
                            // Keep vertices in local space for animations
                            worldPos = Vec3(positions[index * 3 + 0], positions[index * 3 + 1], positions[index * 3 + 2]);
                        }
                        else
                        {
                            // Bake transform into vertices
                            Vec4 localPos    = Vec4(positions[index * 3 + 0], positions[index * 3 + 1], positions[index * 3 + 2], 1.0f);
                            Vec4 transformed = globalTransform * localPos;
                            worldPos         = Vec3(transformed);
                        }

                        vertex.Position = {
                            xMultiplier * worldPos.x,
                            yMultiplier * worldPos.y,
                            zMultiplier * worldPos.z,
                        };

                        // Normal - apply normal transformation only if no animations
                        if (normals)
                        {
                            Vec3 worldNormal;

                            if (hasAnimations)
                            {
                                // Keep normals in local space
                                worldNormal = Vec3(normals[index * 3 + 0], normals[index * 3 + 1], normals[index * 3 + 2]);
                            }
                            else
                            {
                                // Transform normals
                                Mat3 normalMatrix = glm::transpose(glm::inverse(Mat3(globalTransform)));
                                Vec3 localNormal  = Vec3(normals[index * 3 + 0], normals[index * 3 + 1], normals[index * 3 + 2]);
                                worldNormal       = glm::normalize(normalMatrix * localNormal);
                            }

                            vertex.Normal = {
                                xMultiplier * worldNormal.x,
                                yMultiplier * worldNormal.y,
                                zMultiplier * worldNormal.z,
                            };
                        }
                        else
                        {
                            vertex.Normal = {0.0f, 1.0f, 0.0f};
                        }

                        // Texture coordinates
                        if (texCoords)
                        {
                            vertex.UV = {
                                texCoords[index * 2 + 0],
                                1.0f - texCoords[index * 2 + 1],
                            };
                        }
                        else
                        {
                            vertex.UV = {0.0f, 0.0f};
                        }

                        // Color (default to white)
                        vertex.Color = {1.0f, 1.0f, 1.0f};

                        // Add to vertex buffer (disable deduplication for morph targets)
                        if (hasMorphTargets)
                            {
                            // No deduplication - store mapping from vertex index to original glTF position index
                            U32 vertexIdx = static_cast<U32>(builder.Vertices.size());

                            builder.Indices.push_back(vertexIdx);
                            builder.Vertices.push_back(vertex);
                            indicesByMaterial[materialId].push_back(vertexIdx);

                            // Store mapping: builder vertex index -> original glTF position index
                            vertexToPositionIndex[vertexIdx] = index;
                        }
                        else
                        {
                            // Normal deduplication for non-morph meshes
                            if (uniqueVertices.count(vertex) == 0)
                            {
                                uniqueVertices[vertex] = static_cast<U32>(builder.Vertices.size());
                                builder.Vertices.push_back(vertex);
                            }

                            U32 vertexIndex = uniqueVertices[vertex];
                            builder.Indices.push_back(vertexIndex);
                            indicesByMaterial[materialId].push_back(vertexIndex);
                        }
                    }

                    // Store the actual vertex count for this primitive (for morph targets)
                    U32 primitiveVertexCount = static_cast<U32>(builder.Vertices.size()) - primitiveVertexOffset;
                    primitiveVertexCounts[key]    = primitiveVertexCount;

                    VY_INFO_TAG("GLTFImporter", "Mesh {0} prim {1} added {2} vertices", meshIndex, primIdx, primitiveVertexCount);
                }
            }

            // Process children recursively
            for (int childIndex : node.children)
            {
                processNode(childIndex, globalTransform);
            }
        }; // End of processNode

        // Process all root nodes
        for (int nodeIndex : scene.nodes)
        {
            processNode(nodeIndex, Mat4(1.0f));
        }

        // Create sub-meshes from grouped indices
        U32 currentOffset = 0;
        for (auto& [matId, matIndices] : indicesByMaterial)
        {
            if (!matIndices.empty())
            {
                Model::SubMesh subMesh;
                {
                    subMesh.MaterialId  = matId;
                    subMesh.IndexOffset = currentOffset;
                    subMesh.IndexCount  = static_cast<U32>(matIndices.size());
                }

                builder.SubMeshes.push_back(subMesh);

                currentOffset += subMesh.IndexCount;
            }
        }

        // Rebuild indices array grouped by material
        TVector<U32> groupedIndices;
        groupedIndices.reserve(builder.Indices.size());

        for (const auto& subMesh : builder.SubMeshes)
        {
            const auto& matIndices = indicesByMaterial[subMesh.MaterialId];

            groupedIndices.insert(groupedIndices.end(), matIndices.begin(), matIndices.end());
        }

        builder.Indices = std::move(groupedIndices);

        VY_INFO_TAG("GLTFImporter", "Loaded {0} materials, {1} sub-meshes", builder.Materials.size(), builder.SubMeshes.size());

        // Load nodes (store original transforms before animation)
        builder.Nodes.resize(gltfModel.nodes.size());
        
        for (size_t i = 0; i < gltfModel.nodes.size(); i++)
        {
            const auto& gltfNode = gltfModel.nodes[i];
            auto&       node     = builder.Nodes[i];

            node.Name = gltfNode.name;
            node.Mesh = gltfNode.mesh;

            if (gltfNode.matrix.size() == 16)
            {
                node.Matrix = glm::make_mat4(gltfNode.matrix.data());
            }
            else
            {
                if (gltfNode.translation.size() == 3)
                {
                    node.Translation = Vec3(gltfNode.translation[0], gltfNode.translation[1], gltfNode.translation[2]);
                }
                if (gltfNode.rotation.size() == 4)
                {
                    node.Rotation = Quat(
                        static_cast<float>(gltfNode.rotation[3]),
                        static_cast<float>(gltfNode.rotation[0]),
                        static_cast<float>(gltfNode.rotation[1]),
                        static_cast<float>(gltfNode.rotation[2])
                    );
                }
                if (gltfNode.scale.size() == 3)
                {
                    node.Scale = Vec3(gltfNode.scale[0], gltfNode.scale[1], gltfNode.scale[2]);
                }
            }

            for (int childIdx : gltfNode.children)
            {
                node.Children.push_back(childIdx);
            }

            // Load morph target weights if present
            if (!gltfNode.weights.empty())
            {
                node.MorphWeights.resize(gltfNode.weights.size());

                for (size_t w = 0; w < gltfNode.weights.size(); w++)
                {
                    node.MorphWeights[w] = static_cast<float>(gltfNode.weights[w]);
                }
            }
        }

        // Load morph targets from meshes
        for (size_t meshIdx = 0; meshIdx < gltfModel.meshes.size(); meshIdx++)
        {
            const auto& gltfMesh = gltfModel.meshes[meshIdx];

            for (size_t primIdx = 0; primIdx < gltfMesh.primitives.size(); primIdx++)
            {
                const auto& primitive = gltfMesh.primitives[primIdx];

                if (primitive.targets.empty())
                {
                    continue; // No morph targets
                }

                Model::MorphTargetSet morphSet;

                // Get the vertex offset and count for this primitive
                String key = std::to_string(meshIdx) + "_" + std::to_string(primIdx);

                VY_INFO_TAG("GLTFImporter", "Looking up morph target key:");

                if (primitiveVertexOffsets.find(key) != primitiveVertexOffsets.end())
                {
                    morphSet.VertexOffset = primitiveVertexOffsets[key];
                    morphSet.VertexCount  = primitiveVertexCounts[key]; // Use actual vertex count

                    // Store position index mapping for morph targets
                    morphSet.PositionIndices.resize(morphSet.VertexCount);

                    for (U32 i = 0; i < morphSet.VertexCount; i++)
                    {
                        U32 vertexIdx               = morphSet.VertexOffset + i;
                        morphSet.PositionIndices[i] = vertexToPositionIndex[vertexIdx];
                    }

                    VY_INFO_TAG("GLTFImporter", "Found vertex offset: {0}, count: {1}", morphSet.VertexOffset, morphSet.VertexCount);
                }
                else
                {
                    morphSet.VertexOffset = 0;
                    morphSet.VertexCount  = gltfModel.accessors[primitive.attributes.at("POSITION")].count;

                    VY_WARN_TAG("GLTFImporter", "Warning: Could not find vertex offset for mesh {0} primitive {1}", meshIdx, primIdx);
                }

                // Initialize weights from mesh or node
                if (!gltfMesh.weights.empty())
                {
                    morphSet.Weights.resize(gltfMesh.weights.size());

                    for (size_t w = 0; w < gltfMesh.weights.size(); w++)
                    {
                        morphSet.Weights[w] = static_cast<float>(gltfMesh.weights[w]);
                    }
                }
                else
                {
                    morphSet.Weights.resize(primitive.targets.size(), 0.0f);
                }

                // Load each morph target
                for (const auto& target : primitive.targets)
                {
                    Model::MorphTarget morphTarget;

                    // Load position deltas
                    if (target.find("POSITION") != target.end())
                    {
                        const auto&  posAccessor   = gltfModel.accessors[target.at("POSITION")];
                        const auto&  posBufferView = gltfModel.bufferViews[posAccessor.bufferView];
                        const auto&  posBuffer     = gltfModel.buffers[posBufferView.buffer];
                        const float* positions     = reinterpret_cast<const float*>(&posBuffer.data[posBufferView.byteOffset + posAccessor.byteOffset]);

                        morphTarget.PositionDeltas.resize(posAccessor.count);

                        for (size_t i = 0; i < posAccessor.count; i++)
                        {
                            morphTarget.PositionDeltas[i] = Vec3(positions[i * 3 + 0], positions[i * 3 + 1], positions[i * 3 + 2]);
                        }
                    }

                    // Load normal deltas
                    if (target.find("NORMAL") != target.end())
                    {
                        const auto&  normAccessor   = gltfModel.accessors[target.at("NORMAL")];
                        const auto&  normBufferView = gltfModel.bufferViews[normAccessor.bufferView];
                        const auto&  normBuffer     = gltfModel.buffers[normBufferView.buffer];
                        const float* normals        = reinterpret_cast<const float*>(&normBuffer.data[normBufferView.byteOffset + normAccessor.byteOffset]);

                        morphTarget.NormalDeltas.resize(normAccessor.count);

                        for (size_t i = 0; i < normAccessor.count; i++)
                        {
                            morphTarget.NormalDeltas[i] = Vec3(normals[i * 3 + 0], normals[i * 3 + 1], normals[i * 3 + 2]);
                        }
                    }

                    morphSet.Targets.push_back(morphTarget);
                }

                if (!morphSet.Targets.empty())
                {
                    builder.MorphTargetSets.push_back(morphSet);

                    VY_INFO_TAG("GLTFImporter", "Loaded {0} morph targets for mesh {1}", morphSet.Targets.size(), meshIdx);
                }
            }
        }

        // Load animations
        for (const auto& gltfAnim : gltfModel.animations)
        {
            Model::Animation animation;
            animation.Name = gltfAnim.name.empty() ? "animation_" + std::to_string(builder.Animations.size()) : gltfAnim.name;

            // Load samplers
            for (const auto& gltfSampler : gltfAnim.samplers)
            {
                Model::AnimationSampler sampler;

                // Get time values
                const auto&  timeAccessor   = gltfModel.accessors[gltfSampler.input];
                const auto&  timeBufferView = gltfModel.bufferViews[timeAccessor.bufferView];
                const auto&  timeBuffer     = gltfModel.buffers[timeBufferView.buffer];
                const float* times          = reinterpret_cast<const float*>(&timeBuffer.data[timeBufferView.byteOffset + timeAccessor.byteOffset]);

                sampler.Times.resize(timeAccessor.count);

                for (size_t i = 0; i < timeAccessor.count; i++)
                {
                    sampler.Times[i] = times[i];

                    if (sampler.Times[i] > animation.Duration)
                    {
                        animation.Duration = sampler.Times[i];
                    }
                }

                // Get output values
                const auto&  outputAccessor   = gltfModel.accessors[gltfSampler.output];
                const auto&  outputBufferView = gltfModel.bufferViews[outputAccessor.bufferView];
                const auto&  outputBuffer     = gltfModel.buffers[outputBufferView.buffer];
                const float* outputs          = reinterpret_cast<const float*>(&outputBuffer.data[outputBufferView.byteOffset + outputAccessor.byteOffset]);

                // Store values (type determined by channel target path)
                if (outputAccessor.type == TINYGLTF_TYPE_VEC3)
                {
                    sampler.Translations.resize(outputAccessor.count);
                    sampler.Scales.resize(outputAccessor.count);

                    for (size_t i = 0; i < outputAccessor.count; i++)
                    {
                        sampler.Translations[i] = Vec3(outputs[i * 3 + 0], outputs[i * 3 + 1], outputs[i * 3 + 2]);
                        sampler.Scales[i]       = sampler.Translations[i]; // Same storage
                    }
                }
                else if (outputAccessor.type == TINYGLTF_TYPE_VEC4)
                {
                    sampler.Rotations.resize(outputAccessor.count);

                    for (size_t i = 0; i < outputAccessor.count; i++)
                    {
                        sampler.Rotations[i] = Quat(outputs[i * 4 + 3], outputs[i * 4 + 0], outputs[i * 4 + 1], outputs[i * 4 + 2]);
                    }
                }
                else if (outputAccessor.type == TINYGLTF_TYPE_SCALAR)
                {
                    // Morph target weights - multiple scalars per keyframe
                    // Count weights per keyframe by dividing total count by time count
                    size_t weightsPerFrame = outputAccessor.count / timeAccessor.count;
                    
                    sampler.MorphWeights.resize(timeAccessor.count);

                    for (size_t i = 0; i < timeAccessor.count; i++)
                    {
                        sampler.MorphWeights[i].resize(weightsPerFrame);

                        for (size_t w = 0; w < weightsPerFrame; w++)
                        {
                            sampler.MorphWeights[i][w] = outputs[i * weightsPerFrame + w];
                        }
                    }
                }

                // Interpolation type
                if (gltfSampler.interpolation == "LINEAR")
                {
                    sampler.Interpolation = Model::AnimationSampler::LINEAR;
                }
                else if (gltfSampler.interpolation == "STEP")
                {
                    sampler.Interpolation = Model::AnimationSampler::STEP;
                }
                else if (gltfSampler.interpolation == "CUBICSPLINE")
                {
                    sampler.Interpolation = Model::AnimationSampler::CUBICSPLINE;
                }

                animation.Samplers.push_back(sampler);
            }

            // Load channels
            for (const auto& gltfChannel : gltfAnim.channels)
            {
                Model::AnimationChannel channel;
                channel.SamplerIndex = gltfChannel.sampler;
                channel.TargetNode   = gltfChannel.target_node;

                if (gltfChannel.target_path == "translation")
                {
                    channel.TargetPath = Model::AnimationChannel::TRANSLATION;
                }
                else if (gltfChannel.target_path == "rotation")
                {
                    channel.TargetPath = Model::AnimationChannel::ROTATION;
                }
                else if (gltfChannel.target_path == "scale")
                {
                    channel.TargetPath = Model::AnimationChannel::SCALE;
                }
                else if (gltfChannel.target_path == "weights")
                {
                    channel.TargetPath = Model::AnimationChannel::WEIGHTS;

                    VY_INFO_TAG("GLTFImporter", "Found morph target weight animation channel");
                }
                else
                {
                    // Skip unsupported paths
                    continue;
                }

                animation.Channels.push_back(channel);
            }

            if (animation.Channels.empty())
            {
                VY_WARN_TAG("GLTFImporter", "Warning: Animation '{0}' has no supported channels, skipping", animation.Name);
                continue;
            }

            builder.Animations.push_back(animation);

            VY_INFO_TAG("GLTFImporter", "Loaded animation: '{0}' ({1}s, {2} channels)", animation.Name, animation.Duration, animation.Channels.size());
        }

        return true;
    }
}