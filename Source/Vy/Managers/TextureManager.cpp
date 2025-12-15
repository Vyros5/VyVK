#include <Vy/Managers/TextureManager.h>

#include <Vy/GFX/Context.h>
#include <Vy/Globals.h>

namespace Vy
{
    VyTextureManager::VyTextureManager()
    {
        createDescriptorSetLayout();
        createDescriptorPool();
        createDescriptorSet();

        // Create a placeholder texture (white)
        m_PlaceholderTexture = VySampledTexture::createWhiteTexture();

        // Add it as index 0 so 0 is always valid
        addTexture(m_PlaceholderTexture);
    }


    VyTextureManager::~VyTextureManager() 
    {
    }


    void VyTextureManager::createDescriptorSetLayout()
    {
        m_DescriptorSetLayout = VyDescriptorSetLayout::Builder{}
            .addBinding(
                0,
                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                VK_SHADER_STAGE_FRAGMENT_BIT,
                MAX_TEXTURES,
                VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT
            )
        .buildUnique();
    }


    void VyTextureManager::createDescriptorPool()
    {
        m_DescriptorPool = VyDescriptorPool::Builder{}
            .setMaxSets  (1)
            .addPoolSize (VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_TEXTURES)
            .setPoolFlags(VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT)
        .buildUnique();
    }


    void VyTextureManager::createDescriptorSet()
    {
        if (!m_DescriptorPool->allocateDescriptorSet(m_DescriptorSetLayout->handle(), m_DescriptorSet))
        {
            throw std::runtime_error("failed to allocate global texture descriptor set!");
        }
    }


    TextureIndex VyTextureManager::addTexture(Shared<VySampledTexture> texture)
    {
        if (m_TextureIndexMap.count(texture.get()))
        {
            return m_TextureIndexMap[texture.get()];
        }

        if (m_Textures.size() >= MAX_TEXTURES)
        {
            throw std::runtime_error("Max textures exceeded in VyTextureManager");
        }

        TextureIndex index = static_cast<TextureIndex>(m_Textures.size());
        
        m_Textures.push_back(texture);

        m_TextureIndexMap[texture.get()] = index;

        VkDescriptorImageInfo imageInfo = texture->descriptorImageInfo();
        updateDescriptorSet(index, imageInfo);

        return index;
    }


    void VyTextureManager::updateDescriptorSet(BindingIndex index, VkDescriptorImageInfo& imageInfo)
    {
        VyDescriptorWriter{ *m_DescriptorSetLayout, *m_DescriptorPool }
            .writeImage(index, &imageInfo)
        .update(m_DescriptorSet);

        // VkWriteDescriptorSet write{};
        // write.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        // write.dstSet          = m_DescriptorSet;
        // write.dstBinding      = 0;
        // write.dstArrayElement = index;
        // write.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        // write.descriptorCount = 1;
        // write.pImageInfo      = &imageInfo;

        // vkUpdateDescriptorSets(VyContext::device(), 1, &write, 0, nullptr);
    }
}