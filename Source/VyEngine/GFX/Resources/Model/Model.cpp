#include <VyEngine/GFX/Resources/Model/Model.h>
#include <VyEngine/VK/Context.h>

#include <VyEngine/GFX/Resources/Model/UFBXImporter.h>

// #define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include <VyLib/Util/Hash.h>



extern Vy::Shared<Vy::VyTexture> g_DefaultTexture;

namespace std 
{
    template <>
    struct hash<Vy::Vertex> 
    {
        size_t operator()(const Vy::Vertex& vertex) const 
        {
            size_t seed = 0;
            
            Vy::Hash::hashCombine(seed, 
                vertex.Position, 
                vertex.Color, 
                vertex.Normal, 
                vertex.UV, 
                vertex.Tangent
            );
            
            return seed;
        }
    };
}


namespace Vy
{
    TVector<VkVertexInputBindingDescription> Vertex::bindingDescriptions() 
    {
        TVector<VkVertexInputBindingDescription> bindingDescriptions( 1 );
        {
            bindingDescriptions[0].binding   = 0;
            bindingDescriptions[0].stride    = sizeof(Vertex);
            bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        }

        return bindingDescriptions;
    }


    TVector<VkVertexInputAttributeDescription> Vertex::attributeDescriptions() 
    {
        TVector<VkVertexInputAttributeDescription> attributeDescriptions{};
        {
            attributeDescriptions.push_back( { 0, 0, VK_FORMAT_R32G32B32_SFLOAT,    offsetof(Vertex, Position) } );
            attributeDescriptions.push_back( { 1, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Vertex, Color   ) } );
            attributeDescriptions.push_back( { 2, 0, VK_FORMAT_R32G32B32_SFLOAT,    offsetof(Vertex, Normal  ) } );
            attributeDescriptions.push_back( { 3, 0, VK_FORMAT_R32G32_SFLOAT,       offsetof(Vertex, UV      ) } );
            attributeDescriptions.push_back( { 4, 0, VK_FORMAT_R32G32B32_SFLOAT,    offsetof(Vertex, Tangent ) } );
            attributeDescriptions.push_back( { 5, 0, VK_FORMAT_R32G32B32A32_SINT,   offsetof(Vertex, JointIds) } );
            attributeDescriptions.push_back( { 6, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Vertex, Weights ) } );
        }
        
        return attributeDescriptions;
    }


    MaterialDescriptor::MaterialDescriptor(VyMaterialOLD& material, VyMaterialOLD::MaterialTexturesArray& textures) 
    {
        material.MaterialBuffer = std::make_shared<VyBuffer>( 
            VyBuffer::uniformBuffer( 
                "material", 
                sizeof(VyMaterialOLD::PBRMaterial) 
            ) 
        );

        // Textures
        Shared<VyTexture>  diffuseMap;
        Shared<VyTexture>  normalMap;
        Shared<VyTexture>  roughnessMetallicMap;
        Shared<VyTexture>  emissiveMap;
        Shared<VyTexture>  roughnessMap;
        Shared<VyTexture>  metallicMap;
        Shared<VyTexture>& dummy = g_DefaultTexture;

        diffuseMap           = textures[VyMaterialOLD::DIFFUSE_MAP_INDEX]            
                                ? textures[VyMaterialOLD::DIFFUSE_MAP_INDEX]            
                                : dummy;

        normalMap            = textures[VyMaterialOLD::NORMAL_MAP_INDEX]             
                                ? textures[VyMaterialOLD::NORMAL_MAP_INDEX]             
                                : dummy;

        roughnessMetallicMap = textures[VyMaterialOLD::ROUGHNESS_METALLIC_MAP_INDEX] 
                                ? textures[VyMaterialOLD::ROUGHNESS_METALLIC_MAP_INDEX] 
                                : dummy;

        emissiveMap          = textures[VyMaterialOLD::EMISSIVE_MAP_INDEX]           
                                ? textures[VyMaterialOLD::EMISSIVE_MAP_INDEX]           
                                : dummy;

        roughnessMap         = textures[VyMaterialOLD::ROUGHNESS_MAP_INDEX]          
                                ? textures[VyMaterialOLD::ROUGHNESS_MAP_INDEX]          
                                : dummy;

        metallicMap          = textures[VyMaterialOLD::METALLIC_MAP_INDEX]           
                                ? textures[VyMaterialOLD::METALLIC_MAP_INDEX]           
                                : dummy;

        {
            Unique<VyDescriptorSetLayout> localDescriptorSetLayout = VyDescriptorSetLayout::Builder{}
                .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         VK_SHADER_STAGE_FRAGMENT_BIT)
                .addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
                .addBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
                .addBinding(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
                .addBinding(4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
                .addBinding(5, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
                .addBinding(6, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
                .buildPtr();

            auto bufferInfo = material.MaterialBuffer->descriptorBufferInfo();
            auto imageInfo0 = diffuseMap             ->descriptorImageInfo();
            auto imageInfo1 = normalMap              ->descriptorImageInfo();
            auto imageInfo2 = roughnessMetallicMap   ->descriptorImageInfo();
            auto imageInfo3 = emissiveMap            ->descriptorImageInfo();
            auto imageInfo4 = roughnessMap           ->descriptorImageInfo();
            auto imageInfo5 = metallicMap            ->descriptorImageInfo();

            VyDescriptorWriter( *localDescriptorSetLayout, *VyContext::globalPool() )
                .writeBuffer(0, &bufferInfo)
                .writeImage (1, &imageInfo0)
                .writeImage (2, &imageInfo1)
                .writeImage (3, &imageInfo2)
                .writeImage (4, &imageInfo3)
                .writeImage (5, &imageInfo4)
                .writeImage (6, &imageInfo5)
                .build( m_DescriptorSet );
        }
    }

    
    MaterialDescriptor::MaterialDescriptor(MaterialDescriptor const& other) 
    {
        m_DescriptorSet = other.m_DescriptorSet;
    }



    Unique<MeshModel> MeshModel::createMeshFromFile(TStringView filePath) 
    {
        UFBXImporter importer{ filePath.data() };

        if (!importer.loadModel()) 
        {
            VY_ERROR("Failed to load Model file {0}", filePath.data());
            return nullptr;
        }
        
        return MakeUnique<MeshModel>( importer );
    }


    MeshModel::MeshModel(const UFBXImporter& importer) 
    {
        copyMeshes        (importer.Meshes  );
        createVertexBuffer(importer.Vertices);
        createIndexBuffer (importer.Indices );

        m_Skeleton    = importer.Skeleton;
        m_SkeletonUbo = importer.SkeletonUbo;
        m_Vertices    = importer.Vertices;
        m_Indices     = importer.Indices;
    }


    MeshModel::~MeshModel() 
    {
        vkDeviceWaitIdle(VyContext::device());
    }


    void MeshModel::copyMeshes(const TVector<VyMesh>& meshes) 
    {
        for (auto& mesh : meshes) 
        {
            m_Meshes.push_back( mesh );
        }
    }


    void MeshModel::createVertexBuffer(const TVector<Vertex>& vertices) 
    {
        m_VertexCount = static_cast<U32>(vertices.size());

        VY_ASSERT(m_VertexCount >= 3, "Vertex count must be at least 3");
        
        VkDeviceSize bufferSize = sizeof(vertices[0]) * m_VertexCount;
        U32          vertexSize = sizeof(vertices[0]);
        
        VyBuffer stagingBuffer{ VyBuffer::stagingBuffer("mesh", vertexSize, m_VertexCount) };
        
        stagingBuffer.write( vertices.data() );
        
        m_VertexBuffer = MakeUnique<VyBuffer>( VyBuffer::vertexBuffer("mesh", vertexSize, m_VertexCount) );
        
        VyContext::device().copyBuffer( stagingBuffer.handle(), m_VertexBuffer->handle(), bufferSize );
    }


    void MeshModel::createIndexBuffer(const TVector<U32>& indices) 
    {
        m_IndexCount     = static_cast<U32>(indices.size());
        m_HasIndexBuffer = m_IndexCount > 0;
        
        if (!m_HasIndexBuffer) { return; }
        
        VkDeviceSize bufferSize = sizeof(indices[0]) * m_IndexCount;
        U32          indexSize  = sizeof(indices[0]);
        
        VyBuffer stagingBuffer{ VyBuffer::stagingBuffer("mesh", indexSize, m_IndexCount) };
        
        stagingBuffer.write( indices.data() );
        
        m_IndexBuffer = MakeUnique<VyBuffer>( VyBuffer::indexBuffer("mesh", indexSize, m_IndexCount) );
        
        VyContext::device().copyBuffer( stagingBuffer.handle(), m_IndexBuffer->handle(), bufferSize );
    }


    void MeshModel::updateAnimation(U32 frameCounter) 
    {
        m_Skeleton->update();

        // update ubo
        m_SkeletonUbo->write(m_Skeleton->SkeletonUbo.JointsMatrices.data());
    }


    void MeshModel::bind(VkCommandBuffer cmdBuffer) 
    {
        VkBuffer     buffers[] = { m_VertexBuffer->handle() };
        VkDeviceSize offsets[] = { 0 };

        vkCmdBindVertexBuffers(cmdBuffer, 0, 1, buffers, offsets);
        
        if (m_HasIndexBuffer) 
        {
            vkCmdBindIndexBuffer(cmdBuffer, m_IndexBuffer->handle(), 0, VK_INDEX_TYPE_UINT32);
        }
    }


    void MeshModel::draw(const VyFrameInfo& frameInfo, const VkPipelineLayout& pipelineLayout) 
    {
        for (auto& mesh : m_Meshes) 
        {
            bindDescriptors(frameInfo, pipelineLayout, mesh);
            
            drawMesh(frameInfo.CommandBuffer, mesh);
        }
    }


    void MeshModel::drawMesh(const VkCommandBuffer& cmdBuffer, const VyMesh& mesh) const 
    {
        if (m_HasIndexBuffer) 
        {
            vkCmdDrawIndexed(cmdBuffer, mesh.IndexCount, 1, mesh.FirstIndex, mesh.FirstVertex, 0);
        } 
        else {
            vkCmdDraw(cmdBuffer, mesh.VertexCount, 1, mesh.FirstVertex, 0);
        }
    }


    void MeshModel::bindDescriptors(const VyFrameInfo& frameInfo, const VkPipelineLayout& pipelineLayout, VyMesh& mesh) 
    {
        mesh.Material.MaterialBuffer->write( &mesh.Material.PbrMaterial );

        const VkDescriptorSet& materialDescriptorSet = mesh.Material.MaterialDescriptor->descriptorSet();
        const VkDescriptorSet& skeletonDescriptorSet = mesh.SkeletonDescriptorSet;

        TVector<VkDescriptorSet> descriptorSets = {
            frameInfo.GlobalSet, 
            materialDescriptorSet, 
            skeletonDescriptorSet 
        };
        
        vkCmdBindDescriptorSets(
            frameInfo.CommandBuffer,                  // VkCommandBuffer        cmdBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,          // VkPipelineBindPoint    pipelineBindPoint,
            pipelineLayout,                           // VkPipelineLayout       layout,
            0,                                        // uint32_t               firstSet,
            static_cast<U32>(descriptorSets.size()),  // uint32_t               descriptorSetCount,
            descriptorSets.data(),                    // const VkDescriptorSet* pDescriptorSets,
            0,                                        // uint32_t               dynamicOffsetCount,
            nullptr                                   // const uint32_t*        pDynamicOffsets);
        );
    }


    MeshModel::Bounds MeshModel::bounds() const 
    {
        Vec3 lower{std::numeric_limits<float>::max()};
        Vec3 upper{std::numeric_limits<float>::lowest()};

        for (auto& v : m_Vertices) 
        {
            lower = min(v.Position, lower);
            upper = max(v.Position, upper);
        }

        return {lower, upper};
    }
    

    float MeshModel::width() const 
    {
        auto b = bounds();
        return b.Upper.x - b.Lower.x;
    }








    // bool Model::Data::loadModel(const TString& filepath, bool bAllUniqueVertices) 
    // {
    //     tinyobj::attrib_t            attrib;
    //     TVector<tinyobj::shape_t>    shapes;
    //     TVector<tinyobj::material_t> materials;
    //     TString                      warn, err;
        
    //     if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filepath.c_str())) 
    //     {
    //         throw std::runtime_error(warn + err);
    //     }
        
    //     Vertices.clear();
    //     Indices .clear();
        
    //     THashMap<VyVertex, U32> uniqueVertices{};

    //     for (const auto& shape: shapes) 
    //     {
    //         for (const auto& index: shape.mesh.indices) 
    //         {
    //             VyVertex vertex{};
                
    //             if (index.vertex_index >= 0) 
    //             {
    //                 vertex.Position = {
    //                     attrib.vertices[3 * index.vertex_index + 0],
    //                     attrib.vertices[3 * index.vertex_index + 1],
    //                     attrib.vertices[3 * index.vertex_index + 2]
    //                 };

    //                 vertex.Color = {
    //                     attrib.colors[3 * index.vertex_index + 0],
    //                     attrib.colors[3 * index.vertex_index + 1],
    //                     attrib.colors[3 * index.vertex_index + 2]
    //                 };
    //             }
    //             if (index.normal_index >= 0) 
    //             {
    //                 vertex.Normal = {
    //                     attrib.normals[3 * index.normal_index + 0],
    //                     attrib.normals[3 * index.normal_index + 1],
    //                     attrib.normals[3 * index.normal_index + 2]
    //                 };
    //             }
    //             if (index.texcoord_index >= 0) 
    //             {
    //                 vertex.UV = {
    //                     attrib.texcoords[2 * index.texcoord_index + 0],
    //                     attrib.texcoords[2 * index.texcoord_index + 1]
    //                 };
    //             }
                
    //             // bAllUniqueVertices == True treats ALL vertices as unique (bypass overlapping UV bug)
    //             if (uniqueVertices.count( vertex ) == 0 || bAllUniqueVertices) 
    //             {
    //                 uniqueVertices[ vertex ] = static_cast<U32>(Vertices.size());
                    
    //                 Vertices.push_back( vertex );
    //             }

    //             Indices.push_back( uniqueVertices[ vertex ] );
    //         }
            
    //     }
        
    //     TVector<Vec3> tangents  ( Vertices.size(), Vec3(0.0f) );
    //     TVector<Vec3> bitangents( Vertices.size(), Vec3(0.0f) );
    //     Vec3          tanBasis[ 2 ];
        
    //     // Compute Tangent Basis for each triangle
    //     for (size_t i = 0; i < Indices.size(); i += 3) // <- 3
    //     {
    //         computeTangentBasis(
    //             Vertices.at( Indices[ i     ] ), 
    //             Vertices.at( Indices[ i + 1 ] ), 
    //             Vertices.at( Indices[ i + 2 ] ), 
    //             tanBasis
    //         );

    //         tangents  .at( Indices[ i     ] ) += tanBasis[ 0 ];
    //         tangents  .at( Indices[ i + 1 ] ) += tanBasis[ 0 ];
    //         tangents  .at( Indices[ i + 2 ] ) += tanBasis[ 0 ];
    //         bitangents.at( Indices[ i     ] ) += tanBasis[ 1 ];
    //         bitangents.at( Indices[ i + 1 ] ) += tanBasis[ 1 ];
    //         bitangents.at( Indices[ i + 2 ] ) += tanBasis[ 1 ];
    //     }
        
    //     // Assign oriented Tangent Basis to each vertex
    //     for (size_t i = 0; i < Vertices.size(); i++) 
    //     {
    //         Vec3 N = Vertices.at( i ).Normal;
    //         Vec3 T = tangents.at( i );

    //         // Re-Orthogonalize, then Normalize
    //         T = glm::normalize( T - (glm::dot( T, N ) * N) );
            
    //         float w = glm::dot( glm::cross( N, T ), bitangents.at( i ) ) < 0.0f ? -1.0f : 1.0f;
            
    //         Vertices.at( i ).Tangent = { T, w };
    //     }    
    // }


    // void VyModel::Data::computeTangentBasis(VyVertex& v0, VyVertex& v1, VyVertex& v2, Vec3* pTanOut)
    // {
    //     // Edges of the triangle : Position delta
    //     Vec3 deltaPos1 = v1.Position - v0.Position;
    //     Vec3 deltaPos2 = v2.Position - v0.Position;
        
    //     // UV delta
    //     Vec2 deltaUV1 = v1.UV0 - v0.UV0;
    //     Vec2 deltaUV2 = v2.UV0 - v0.UV0;
        
    //     if (v1.UV0 == v0.UV0 && v2.UV0 == v0.UV0) 
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


    // bool VyModel::Data::loadModel(const TPath& path, bool bAllUniqueVertices) 
    // {
    //     tinyobj::attrib_t            attrib;
    //     TVector<tinyobj::shape_t>    shapes;
    //     TVector<tinyobj::material_t> materials;
    //     TString                      warn, err;
        
    //     if (!tinyobj::LoadObj( &attrib, &shapes, &materials, &warn, &err, path.string().c_str() )) 
    //     {
    //         VY_THROW_RUNTIME_ERROR( warn + err );

    //         return false;
    //     }
        
    //     Vertices.clear();
    //     Indices .clear();
        
    //     THashMap<VyVertex, U32> uniqueVertices{};

    //     for (const auto& shape: shapes) 
    //     {
    //         for (const auto& index: shape.mesh.indices) 
    //         {
    //             VyVertex vertex{};
                
    //             if (index.vertex_index >= 0) 
    //             {
    //                 vertex.Position = {
    //                     attrib.vertices[3 * index.vertex_index + 0],
    //                     attrib.vertices[3 * index.vertex_index + 1],
    //                     attrib.vertices[3 * index.vertex_index + 2]
    //                 };

    //                 vertex.Color = {
    //                     attrib.colors[3 * index.vertex_index + 0],
    //                     attrib.colors[3 * index.vertex_index + 1],
    //                     attrib.colors[3 * index.vertex_index + 2]
    //                 };
    //             }
    //             if (index.normal_index >= 0) 
    //             {
    //                 vertex.Normal = {
    //                     attrib.normals[3 * index.normal_index + 0],
    //                     attrib.normals[3 * index.normal_index + 1],
    //                     attrib.normals[3 * index.normal_index + 2]
    //                 };
    //             }
    //             if (index.texcoord_index >= 0) 
    //             {
    //                 vertex.UV0 = {
    //                     attrib.texcoords[2 * index.texcoord_index + 0],
    //                     attrib.texcoords[2 * index.texcoord_index + 1]
    //                 };
    //             }
                
    //             if (!uniqueVertices.contains( vertex )) 
    //             {
    //                 uniqueVertices[ vertex ] = static_cast<U32>( Vertices.size() );
                    
    //                 Vertices.push_back( vertex );
    //             }

    //             // bAllUniqueVertices == True treats ALL vertices as unique (bypass overlapping UV bug).
    //             if (uniqueVertices.count( vertex ) == 0 || bAllUniqueVertices) 
    //             {
    //                 uniqueVertices[ vertex ] = static_cast<U32>( Vertices.size() );
                    
    //                 Vertices.push_back( vertex );
    //             }

    //             Indices.push_back( uniqueVertices[ vertex ] );
    //         }
    //     }
        
    //     TVector<Vec3> tangents  ( Vertices.size(), Vec3(0.0f) );
    //     TVector<Vec3> bitangents( Vertices.size(), Vec3(0.0f) );
    //     Vec3          tanBasis[ 2 ];
        
    //     // Compute Tangent Basis for each triangle.
    //     for (size_t i = 0; i < Indices.size(); i += 3) // <- 3
    //     {
    //         computeTangentBasis(
    //             Vertices.at( Indices[ i     ] ), 
    //             Vertices.at( Indices[ i + 1 ] ), 
    //             Vertices.at( Indices[ i + 2 ] ), 
    //             tanBasis
    //         );

    //         tangents  .at( Indices[ i     ] ) += tanBasis[ 0 ];
    //         tangents  .at( Indices[ i + 1 ] ) += tanBasis[ 0 ];
    //         tangents  .at( Indices[ i + 2 ] ) += tanBasis[ 0 ];
    //         bitangents.at( Indices[ i     ] ) += tanBasis[ 1 ];
    //         bitangents.at( Indices[ i + 1 ] ) += tanBasis[ 1 ];
    //         bitangents.at( Indices[ i + 2 ] ) += tanBasis[ 1 ];
    //     }
        
    //     // Assign oriented Tangent Basis to each vertex.
    //     for (size_t i = 0; i < Vertices.size(); i++) 
    //     {
    //         Vec3 N = Vertices.at( i ).Normal;
    //         Vec3 T = tangents.at( i );

    //         // Re-Orthogonalize, then Normalize.
    //         T = glm::normalize( T - (glm::dot( T, N ) * N) );
            
    //         float w = glm::dot( glm::cross( N, T ), bitangents.at( i ) ) < 0.0f ? -1.0f : 1.0f;
            
    //         Vertices.at( i ).Tangent = { T, w };
    //     }

    //     return true;
    // }


    // VyModel::VyModel(const Data& data)
    // {
    //     createVertexBuffer(data.Vertices);
    //     createIndexBuffer (data.Indices);
    // }

    
    // VyModel::~VyModel() 
    // {

    // }


    // Unique<VyModel> VyModel::loadFromFile(const TPath& path, bool bAllUniqueVertices) 
    // {
    //     Data data{};

    //     if (!data.loadModel( path, bAllUniqueVertices ))
    //     {
    //         VY_ERROR_TAG("VyModel", "Failed to load Model file: {0}", path.string());
            
    //         return nullptr;
    //     }

    //     return MakeUnique<VyModel>( data );
    // }


    // void VyModel::bind(VkCommandBuffer cmdBuffer) const
    // {
    //     VkBuffer     buffers[] = { m_VertexBuffer->handle() };
    //     VkDeviceSize offsets[] = { 0 };

    //     vkCmdBindVertexBuffers(cmdBuffer, 0, 1, buffers, offsets);
        
    //     if (m_HasIndexBuffer) 
    //     {
    //         vkCmdBindIndexBuffer(cmdBuffer, m_IndexBuffer->handle(), 0, VK_INDEX_TYPE_UINT32);
    //     }
    // }


    // void VyModel::draw(VkCommandBuffer cmdBuffer) const
    // {
    //     if (m_HasIndexBuffer) 
    //     {
    //         vkCmdDrawIndexed(cmdBuffer, m_IndexCount, 1, 0, 0, 0);
    //     } 
    //     else 
    //     {
    //         vkCmdDraw(cmdBuffer, m_VertexCount, 1, 0, 0);
    //     }
    // }


    // void VyModel::createVertexBuffer(const TVector<VyVertex>& vertices) 
    // {
    //     m_VertexCount = static_cast<U32>(vertices.size());

    //     VY_ASSERT(m_VertexCount >= 3, "Vertex count must be at least 3");
        
    //     VkDeviceSize bufferSize = sizeof(vertices[0]) * m_VertexCount;
    //     U32          vertexSize = sizeof(vertices[0]);
        
    //     VyBuffer stagingBuffer{ VyBuffer::stagingBuffer( "model", vertexSize, m_VertexCount ) };
        
    //     stagingBuffer.write( vertices.data() );
        
    //     m_VertexBuffer = MakeUnique<VyBuffer>( VyBuffer::vertexBuffer( "model", vertexSize, m_VertexCount ) );
        
    //     VyContext::device().copyBuffer( stagingBuffer.handle(), m_VertexBuffer->handle(), bufferSize );
    // }


    // void VyModel::createIndexBuffer(const TVector<U32>& indices) 
    // {
    //     m_IndexCount     = static_cast<U32>(indices.size());
    //     m_HasIndexBuffer = m_IndexCount > 0;
        
    //     if (!m_HasIndexBuffer) { return; }
        
    //     VkDeviceSize bufferSize = sizeof(indices[0]) * m_IndexCount;
    //     U32          indexSize  = sizeof(indices[0]);
        
    //     VyBuffer stagingBuffer{ VyBuffer::stagingBuffer( "model", indexSize, m_IndexCount ) };
        
    //     stagingBuffer.write( indices.data() );
        
    //     m_IndexBuffer = MakeUnique<VyBuffer>( VyBuffer::indexBuffer( "model", indexSize, m_IndexCount ) );
        
    //     VyContext::device().copyBuffer( stagingBuffer.handle(), m_IndexBuffer->handle(), bufferSize );
    // }
}