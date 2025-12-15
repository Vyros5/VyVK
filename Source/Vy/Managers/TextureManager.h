#pragma once

#include <Vy/GFX/Resources/Texture.h>
#include <Vy/GFX/Backend/Descriptors.h>

namespace Vy
{
    using TextureIndex = U32;

    class VyTextureManager
    {
    public:
        static constexpr U32 MAX_TEXTURES = 1024;

        VyTextureManager();
        ~VyTextureManager();

        VyTextureManager(const VyTextureManager&)            = delete;
        VyTextureManager& operator=(const VyTextureManager&) = delete;

        // Returns the global index of the texture
        TextureIndex addTexture(Shared<VySampledTexture> texture);

        VkDescriptorSetLayout descriptorSetLayout() const { return m_DescriptorSetLayout->handle(); }
        VkDescriptorSet       descriptorSet()       const { return m_DescriptorSet; }

    private:
        void createDescriptorSetLayout();
        void createDescriptorPool();
        void createDescriptorSet();
        void updateDescriptorSet(BindingIndex index, VkDescriptorImageInfo& imageInfo);

    private:
        Unique<VyDescriptorSetLayout> m_DescriptorSetLayout;
        Unique<VyDescriptorPool>      m_DescriptorPool;
        VkDescriptorSet               m_DescriptorSet;

        TVector<Shared<VySampledTexture>>         m_Textures;
        THashMap<VySampledTexture*, TextureIndex> m_TextureIndexMap;

        // Placeholder texture for empty slots
        Shared<VySampledTexture> m_PlaceholderTexture;
    };
}