#include <VyEngine/GFX/Systems/MaterialSystem.h>

#include <VyEngine/VK/Context.h>
#include <VyEngine/Globals.h>

#include <VyEngine/Scene/ECS/Components.h>
// #include <VyEngine/GFX/Resources/Texture/Texture.h>

namespace Vy
{
    Unique<VyDescriptorSetLayout> 
    VyMaterialSystem::createMaterialSetLayout()
    {
        return VyDescriptorSetLayout::Builder{}
            .setName   ("material")
            // .setLayoutFlags(VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT)
            .addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT) // Albedo
            .addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT) // Normal
            .addBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT) // Roughness
            .addBinding(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT) // Metallic
            .buildPtr();
    }


    void VyMaterialSystem::updateMaterials(
        VyFrameInfo&           frameInfo, 
        VyDescriptorSetLayout& materialSetLayout, 
        VyDescriptorPool&      materialPool)
    {
        // Iterate over all entities with materials and update their descriptor sets.
        auto view = frameInfo.Scene->registry().view<MaterialComponent>();
        
        for (auto&& [ entity, matComp ] : view.each())
        {
            if (matComp.Material)
            {
                matComp.Material->updateDescriptorSet( materialSetLayout, materialPool );
            }
        }
    }
}


// namespace Vy
// {
//     VyMaterialSystem::VyMaterialSystem()
//     {
//         createDefaultTextures();
        
//         createMaterialDescriptorSetLayout();
//         createMaterialDescriptorPool();
//     }


//     void VyMaterialSystem::createDefaultTextures()
//     {
//         m_DefaultWhiteTexture  = VyTexture::createWhiteTexture();
//         m_DefaultNormalTexture = VyTexture::createNormalTexture();
//     }

    
//     void VyMaterialSystem::createMaterialDescriptorSetLayout()
//     {
//         m_MaterialSetLayout = VyDescriptorSetLayout::Builder{}
//             .addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT) // albedo
//             .addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT) // normal
//             .addBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT) // metallic
//             .addBinding(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT) // roughness
//             .addBinding(4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT) // ao
//             .buildPtr();
//     }

    
//     void VyMaterialSystem::createMaterialDescriptorPool()
//     {
//         m_MaterialDescriptorPool = VyDescriptorPool::Builder{}
//             .setMaxSets (1000) // Support many materials
//             .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 5000)
//             .buildPtr();
//     }


//     VkDescriptorSet VyMaterialSystem::getMaterialDescriptorSet(const VyPBRMaterial& material)
//     {
//         // Use material pointer as hash key.
//         size_t materialHash = reinterpret_cast<size_t>( &material );

//         // Check cache first.
//         auto it = m_MaterialDescriptorCache.find( materialHash );

//         if (it != m_MaterialDescriptorCache.end())
//         {
//             return it->second;
//         }

//         // Create new descriptor set.
//         VkDescriptorSet descriptorSet;
//         if (!m_MaterialDescriptorPool->allocate( m_MaterialSetLayout->handle(), descriptorSet ))
//         {
//             VY_THROW_RUNTIME_ERROR("Failed to allocate material descriptor set!");
//         }

//         // Write descriptor set with textures or default fallbacks.
//         VyDescriptorWriter writer( *m_MaterialSetLayout, *m_MaterialDescriptorPool );

//         // Get descriptor infos into local variables (descriptorImageInfo returns by value).
//         VkDescriptorImageInfo albedoInfo = material.AlbedoMap 
//             ? material.AlbedoMap   ->descriptorImageInfo() 
//             : m_DefaultWhiteTexture->descriptorImageInfo();

//         VkDescriptorImageInfo normalInfo = material.NormalMap 
//             ? material.NormalMap    ->descriptorImageInfo() 
//             : m_DefaultNormalTexture->descriptorImageInfo();

//         VkDescriptorImageInfo metallicInfo = material.MetallicMap 
//             ? material.MetallicMap ->descriptorImageInfo() 
//             : m_DefaultWhiteTexture->descriptorImageInfo();

//         VkDescriptorImageInfo roughnessInfo = material.RoughnessMap 
//             ? material.RoughnessMap->descriptorImageInfo() 
//             : m_DefaultWhiteTexture->descriptorImageInfo();

//         VkDescriptorImageInfo aoInfo = material.AOMap 
//             ? material.AOMap       ->descriptorImageInfo() 
//             : m_DefaultWhiteTexture->descriptorImageInfo();

//         writer.writeImage( 0, &albedoInfo    );
//         writer.writeImage( 1, &normalInfo    );
//         writer.writeImage( 2, &metallicInfo  );
//         writer.writeImage( 3, &roughnessInfo );
//         writer.writeImage( 4, &aoInfo        );

//         writer.update( descriptorSet );

//         // Cache the descriptor set
//         m_MaterialDescriptorCache[ materialHash ] = descriptorSet;

//         return descriptorSet;
//     }
// }