#pragma once

#include <VyEngine/VK/Device/Device.h>
#include <VyEngine/VK/Buffer/Buffer.h>

#include <VyEngine/GFX/Resources/Texture/Texture.h>

#include <VyEngine/VK/Context.h>

namespace Vy
{
    enum class TextureSemantic 
    { 
        BaseColor, 
        Normal, 
        ORM, 
        Emissive 
    };

    class VyTextureManager 
    {
    public:
        VyTextureManager(U32 maxTextures);

        ~VyTextureManager();

        VyTextureManager(const VyTextureManager&) = delete;
        VyTextureManager& operator=(const VyTextureManager&) = delete;

        int getOrLoadTexture(const TString& path, TextureSemantic semantic);

        int                   defaultAlbedoIndex() const { return m_DefaultAlbedoIndex; }
        int                   defaultNormalIndex() const { return m_DefaultNormalIndex; }
        VkDescriptorSet       descriptorSet()      const { return m_TextureDescriptorSet; }
        VkDescriptorSetLayout descriptorLayout()   const { return m_TextureSetLayout->handle(); }

    private:
        void updateDescriptorSet();
        void createDefaultAlbedoTexture();
        void createDefaultNormalTexture();
        void initializeDescriptorSet();


        Unique<VyDescriptorPool>      m_TexturePool{};
        Unique<VyDescriptorSetLayout> m_TextureSetLayout{};
        VkDescriptorSet               m_TextureDescriptorSet{VK_NULL_HANDLE};

        TVector<Shared<VyTexture>>     m_Textures;
        THashMap<TString, int>         m_TexturesIndexMap;
        TVector<VkDescriptorImageInfo> m_ImageInfos;

        Shared<VyTexture> m_DefaultAlbedoTexture;
        Shared<VyTexture> m_DefaultNormalTexture;
        int               m_DefaultAlbedoIndex = 0;
        int               m_DefaultNormalIndex = 0;

        U32 m_MaxTextures{0};
    };
}