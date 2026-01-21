#pragma once

#include <VyEngine/VK/Buffer/Buffer.h>
#include <VyEngine/GFX/Resources/Texture/Texture.h>
#include <VyEngine/VK/Descriptors/Descriptors.h>

namespace Vy
{
    struct VyMaterialData 
    {
        Vec3  Albedo          { 1.0f, 1.0f, 1.0f };
        float Metallic        { 0.0f };
        float Roughness       { 0.5f };
        float AO              { 1.0f };
        
        Vec2  UVOffset   { 0.0f, 0.0f };
        Vec2  UVScale    { 1.0f, 1.0f };

        Vec3  EmissionColor   { 0.0f, 0.0f, 0.0f };
        float EmissionStrength{ 0.0f };
    };

    class VyMaterial 
    {
    public:
        VyMaterial();
        ~VyMaterial();

        VyMaterial(const VyMaterial&)            = delete;
        VyMaterial& operator=(const VyMaterial&) = delete;

        void setAlbedo       (const Vec3& value) { m_Data.Albedo    = value; }
        void setMetallic     (float       value) { m_Data.Metallic  = value; }
        void setRoughness    (float       value) { m_Data.Roughness = value; }
        void setAO           (float       value) { m_Data.AO        = value; }
        void setUVOffset     (const Vec2& value) { m_Data.UVOffset  = value; }
        void setUVScale      (const Vec2& value) { m_Data.UVScale   = value; }

        void loadAlbedoTexture(const TString& filepath);
        void loadNormalTexture(const TString& filepath);
        void loadRoughnessMap (const TString& filepath);
        void loadMetallicMap  (const TString& filepath);

        void setEmissionColor   (const Vec3& c)          { m_Data.EmissionColor    = c; }
        void setEmissionStrength(float s)                { m_Data.EmissionStrength = s; }
        void setEmission        (const Vec3& c, float s) { m_Data.EmissionColor    = c; m_Data.EmissionStrength = s; }

        // Getters
        const VyMaterialData& materialData() const { return m_Data; }
        
        VkDescriptorSet descriptorSet() const 
        { 
            return m_DescriptorSet; 
        }
        
        bool hasTextures() const 
        { 
            return m_HasAlbedoTexture || m_HasNormalTexture || m_HasRoughnessTexture || m_HasMetallicTexture; 
        }

        // Update descriptor set
        void updateDescriptorSet(VyDescriptorSetLayout& setLayout, VyDescriptorPool& pool);

        bool albedoLoadFailed() const { return m_FailedAlbedo; }

    private:
        void createDefaultTexture();
        void createTextureImage(const TString& filepath, VkImage& image, VmaAllocation& imageMemory);
        void createTextureImageView(VkImage image, VkImageView& imageView);
        void createTextureSampler(VkSampler& sampler);

        VyMaterialData m_Data;

        // Texture resources.
        VkImage        m_AlbedoTextureImage              = VK_NULL_HANDLE;
        VmaAllocation  m_AlbedoTextureImageAllocation    = VK_NULL_HANDLE;
        VkImageView    m_AlbedoTextureImageView          = VK_NULL_HANDLE;
        VkSampler      m_AlbedoTextureSampler            = VK_NULL_HANDLE;

        VkImage        m_NormalTextureImage              = VK_NULL_HANDLE;
        VmaAllocation  m_NormalTextureImageAllocation    = VK_NULL_HANDLE;
        VkImageView    m_NormalTextureImageView          = VK_NULL_HANDLE;
        VkSampler      m_NormalTextureSampler            = VK_NULL_HANDLE;

        VkImage        m_RoughnessTextureImage           = VK_NULL_HANDLE;
        VmaAllocation  m_RoughnessTextureImageAllocation = VK_NULL_HANDLE;
        VkImageView    m_RoughnessTextureImageView       = VK_NULL_HANDLE;
        VkSampler      m_RoughnessTextureSampler         = VK_NULL_HANDLE;

        VkImage        m_MetallicTextureImage            = VK_NULL_HANDLE;
        VmaAllocation  m_MetallicTextureImageAllocation  = VK_NULL_HANDLE;
        VkImageView    m_MetallicTextureImageView        = VK_NULL_HANDLE;
        VkSampler      m_MetallicTextureSampler          = VK_NULL_HANDLE;

        // Default white texture for when no texture is loaded
        VkImage        m_DefaultTextureImage             = VK_NULL_HANDLE;
        VmaAllocation  m_DefaultTextureImageAllocation   = VK_NULL_HANDLE;
        VkImageView    m_DefaultTextureImageView         = VK_NULL_HANDLE;
        VkSampler      m_DefaultTextureSampler           = VK_NULL_HANDLE;

        VkDescriptorSet m_DescriptorSet = VK_NULL_HANDLE;

        bool m_HasAlbedoTexture    = false;
        bool m_HasNormalTexture    = false;
        bool m_HasRoughnessTexture = false;
        bool m_HasMetallicTexture  = false;

        bool m_FailedAlbedo = false;
    };
}

// namespace Vy
// {
//     struct VyMaterialData 
//     {
//         Vec3  Albedo          { 1.0f, 1.0f, 1.0f };
//         float Metallic        { 0.0f };
//         float Roughness       { 0.5f };
//         float AO              { 1.0f };
        
//         Vec2  UVOffset        { 0.0f, 0.0f };
//         Vec2  UVScale         { 1.0f, 1.0f };

//         Vec3  EmissionColor   { 0.0f, 0.0f, 0.0f };
//         float EmissionStrength{ 0.0f };
//     };

//     class VyMaterial 
//     {
//     public:
//         VyMaterial();
//         ~VyMaterial();

//         VyMaterial(const VyMaterial&)            = delete;
//         VyMaterial& operator=(const VyMaterial&) = delete;

//         void setAlbedo       (const Vec3& value) { m_Data.Albedo        = value; }
//         void setMetallic     (float       value) { m_Data.Metallic      = value; }
//         void setRoughness    (float       value) { m_Data.Roughness     = value; }
//         void setAO           (float       value) { m_Data.AO            = value; }
//         void setUVOffset     (const Vec2& value) { m_Data.UVOffset      = value; }
//         void setUVScale      (const Vec2& value) { m_Data.UVScale       = value; }

//         void setEmissionColor   (const Vec3& c)          { m_Data.EmissionColor    = c; }
//         void setEmissionStrength(float s)                { m_Data.EmissionStrength = s; }
//         void setEmission        (const Vec3& c, float s) { m_Data.EmissionColor    = c; m_Data.EmissionStrength = s; }

//         void loadAlbedoTexture(const TString& filepath);
//         void loadNormalTexture(const TString& filepath);
//         void loadRoughnessMap (const TString& filepath);
//         void loadMetallicMap  (const TString& filepath);

//         // Getters
//         const VyMaterialData& materialData() const 
//         { 
//             return m_Data; 
//         }
        
//         VkDescriptorSet descriptorSet() const 
//         { 
//             return m_DescriptorSet; 
//         }
        
//         bool hasTextures() const 
//         { 
//             return m_HasAlbedoTexture 
//                 || m_HasNormalTexture 
//                 || m_HasRoughnessTexture 
//                 || m_HasMetallicTexture
//             ; 
//         }

//         // Update descriptor set
//         void updateDescriptorSet(VyDescriptorSetLayout& setLayout, VyDescriptorPool& pool); // 

//         bool albedoLoadFailed() const { return m_FailedAlbedo; }

//     private:
//         void createDefaultTexture();

//         VyMaterialData m_Data;

//         // Texture resources.
//         Shared<VyTexture> m_AlbedoTexture;
//         Shared<VyTexture> m_NormalTexture;
//         Shared<VyTexture> m_MetallicTexture;
//         Shared<VyTexture> m_RoughnessTexture;
//         Shared<VyTexture> m_DefaultTexture;

//         VkDescriptorSet m_DescriptorSet = VK_NULL_HANDLE;

//         bool m_FailedAlbedo = false;

//         bool m_HasAlbedoTexture; 
//         bool m_HasNormalTexture; 
//         bool m_HasRoughnessTexture;
//         bool m_HasMetallicTexture;
//     };
// }