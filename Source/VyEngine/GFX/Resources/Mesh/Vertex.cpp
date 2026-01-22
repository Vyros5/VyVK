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
                , vertex.Bitangent
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
            // attributeDescriptions.push_back( { 4, 0, VK_FORMAT_R32G32B32_SFLOAT,    offsetof(VyVertex, Tangent  ) } );
            // attributeDescriptions.push_back( { 5, 0, VK_FORMAT_R32G32B32_SFLOAT,    offsetof(VyVertex, Bitangent) } );
        }
        
        return attributeDescriptions;
    }
}