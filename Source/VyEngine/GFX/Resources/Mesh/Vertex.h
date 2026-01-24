#pragma once

#include <VyEngine/VK/Core/VKCore.h>

namespace Vy
{
    struct VyVertex 
    {
        Vec3 Position { 0.0f };
        Vec3 Color    { 1.0f };
        Vec3 Normal   {      };
        Vec2 UV       {      };
        Vec4 Tangent  {      }; // w component stores handedness
        // Vec3 Tangent  {      };
        // Vec3 Bitangent{      };

        static TVector<VkVertexInputBindingDescription>   bindingDescriptions();
        static TVector<VkVertexInputAttributeDescription> attributeDescriptions();
        
        bool operator==(const VyVertex& other) const 
        {
            return Position  == other.Position 
                && Color     == other.Color 
                && Normal    == other.Normal
                && UV        == other.UV 
                && Tangent   == other.Tangent
                // && Bitangent == other.Bitangent
            ;
        }
    };
}