#include <VyEngine/GFX/Resources/Mesh/Vertex.h>

#include <VyLib/Util/Hash.h>
#include <VyLib/Core/Math.h>

#include <glm/gtx/hash.hpp>

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
            );
            
            return seed;
        }
    };
}

namespace Vy
{
    TVector<VkVertexInputBindingDescription> VyVertex::bindingDescriptions() 
    {
        TVector<VkVertexInputBindingDescription> bindingDescriptions( 1 );
        {
            bindingDescriptions[0].binding   = 0;
            bindingDescriptions[0].stride    = sizeof(VyVertex);
            bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        }

        return bindingDescriptions;
    }


    TVector<VkVertexInputAttributeDescription> VyVertex::attributeDescriptions() 
    {
        TVector<VkVertexInputAttributeDescription> attributeDescriptions{};
        {
            attributeDescriptions.push_back( { 0, 0, VK_FORMAT_R32G32B32_SFLOAT,    offsetof(VyVertex, Position ) } );
            attributeDescriptions.push_back( { 1, 0, VK_FORMAT_R32G32B32_SFLOAT,    offsetof(VyVertex, Color    ) } );
            attributeDescriptions.push_back( { 2, 0, VK_FORMAT_R32G32B32_SFLOAT,    offsetof(VyVertex, Normal   ) } );
            attributeDescriptions.push_back( { 3, 0, VK_FORMAT_R32G32_SFLOAT,       offsetof(VyVertex, UV       ) } );
            attributeDescriptions.push_back( { 4, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(VyVertex, Tangent  ) } );
            // attributeDescriptions.push_back( { 4, 0, VK_FORMAT_R32G32B32_SFLOAT,    offsetof(VyVertex, Tangent  ) } );
            // attributeDescriptions.push_back( { 5, 0, VK_FORMAT_R32G32B32_SFLOAT,    offsetof(VyVertex, Bitangent) } );
        }
        
        return attributeDescriptions;
    }



    // void VyMesh::create(
    //     const TVector<VyVertex>& vertices,
    //     const TVector<U32>&      indices,
    //     VyMaterial*              pMaterial,
    //     const TString&           debugName)
    // {
    //     m_VertexCount = static_cast<U32>(vertices.size());
    //     m_IndexCount  = static_cast<U32>(indices.size());
    //     m_Material    = pMaterial;
    //     m_Name        = debugName;

    //     if (m_VertexCount == 0) 
    //     {
    //         throw std::runtime_error("VyMesh::create: vertex count is 0");
    //     }

    //     // Compute bounding box
    //     computeBounds(vertices);

    //     // Create vertex buffer
    //     {
    //         VY_ASSERT(m_VertexCount >= 3, "Vertex count must be at least 3");
            
    //         U32 vertexSize = sizeof(vertices[0]);
            
    //         m_VertexBuffer = MakeUnique<VyBuffer>( VyBuffer::vertexBuffer( "mesh", vertexSize, m_VertexCount ) );
            
    //         m_VertexBuffer->upload( vertices );
    //     }

    //     // Create index buffer (if indices provided)
    //     if (m_IndexCount > 0)
    //     {
    //         U32 indexSize = sizeof(indices[0]);
            
    //         m_IndexBuffer = MakeUnique<VyBuffer>( VyBuffer::indexBuffer( "mesh", indexSize, m_IndexCount ) );
            
    //         m_IndexBuffer->upload( indices );
    //     }
    // }


    // void VyMesh::draw(
    //     VkCommandBuffer  cmdBuffer, 
    //     bool             bBindMaterial, 
    //     VkPipelineLayout materialSetLayout)
    // {
    //     VkBuffer     buffers[] = { m_VertexBuffer->handle() };
    //     VkDeviceSize offsets[] = { 0 };

    //     vkCmdBindVertexBuffers(cmdBuffer, 0, 1, buffers, offsets);

    //     // Bind material descriptor set if requested.
    //     if (bBindMaterial && m_Material && m_Material->descriptorSet() != VK_NULL_HANDLE)
    //     {
    //         VkDescriptorSet matSet = m_Material->descriptorSet();

    //         vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, materialSetLayout, 1, 1, &matSet, 0, nullptr);
    //     }

    //     // Draw indexed or non-indexed
    //     if (m_IndexCount > 0) 
    //     {
    //         vkCmdBindIndexBuffer(cmdBuffer, m_IndexBuffer->handle(), 0, VK_INDEX_TYPE_UINT32);

    //         vkCmdDrawIndexed(cmdBuffer, m_IndexCount, 1, 0, 0, 0);
    //     } 
    //     else 
    //     {
    //         vkCmdDraw(cmdBuffer, m_VertexCount, 1, 0, 0);
    //     }
    // }


    // void VyMesh::computeBounds(const TVector<VyVertex>& vertices) 
    // {
    //     m_Bounds = VyAABB();
        
    //     for (const auto& v : vertices) 
    //     {
    //         m_Bounds.expand( v.Position );
    //     }
    // }


    // VyMesh MeshBuilder::build() 
    // {
    //     VyMesh mesh;
    //     mesh.create( m_Vertices, m_Indices, m_Material, m_Name );

    //     return mesh;
    // }
}