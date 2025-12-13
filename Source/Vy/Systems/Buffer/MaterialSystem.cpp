#include <Vy/Systems/Buffer/MaterialSystem.h>

#include <Vy/GFX/Context.h>
#include <Vy/Globals.h>

namespace Vy
{
    VyMaterialSystem::~VyMaterialSystem()
    {
    }


    Shared<VyDescriptorSetLayout> VyMaterialSystem::createMaterialSetLayout()
    {
        return VyDescriptorSetLayout::Builder()
            .addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT) // Albedo
            .addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT) // Normal
            .addBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT) // Roughness
            .addBinding(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT) // Metallic
            .buildUnique();
    }


    void VyMaterialSystem::updateMaterials(
        const VyFrameInfo&     frameInfo, 
        VyDescriptorSetLayout& materialSetLayout, 
        VyDescriptorPool&      materialPool)
    {
        // Iterate over entities with a MaterialComponent and update material descriptor sets.
        auto view = frameInfo.Scene->getEntitiesWith<MaterialComponent>();

        for (auto&& [ entity, material ] : view.each())
        {
            if (material.Material)
            {
                material.Material->updateDescriptorSet(materialSetLayout, materialPool);
            }
        }
    }
}