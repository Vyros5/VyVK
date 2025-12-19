#pragma once

#include <Vy/GFX/Backend/Buffer/Buffer.h>
#include <Vy/GFX/Backend/Image/Image.h>
#include <Vy/GFX/Resources/Texture.h>
#include <Vy/GFX/Backend/Descriptors.h>

namespace Vy
{
    struct VyMaterialData 
    {
        Vec3  Albedo          { 1.0f, 1.0f, 1.0f };
        float Metallic        { 0.0f };
        float Roughness       { 0.5f };
        float AO              { 1.0f };
        
        Vec2  TextureOffset   { 0.0f, 0.0f };
        Vec2  TextureScale    { 1.0f, 1.0f };

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

        void setAlbedo       (const Vec3& value) { m_Data.Albedo        = value; }
        void setMetallic     (float       value) { m_Data.Metallic      = value; }
        void setRoughness    (float       value) { m_Data.Roughness     = value; }
        void setAO           (float       value) { m_Data.AO            = value; }
        void setTextureOffset(const Vec2& value) { m_Data.TextureOffset = value; }
        void setTextureScale (const Vec2& value) { m_Data.TextureScale  = value; }

        void loadAlbedoTexture(const String& filepath);
        void loadNormalTexture(const String& filepath);
        void loadRoughnessMap (const String& filepath);
        void loadMetallicMap  (const String& filepath);

        void setEmissionColor   (const Vec3& c)          { m_Data.EmissionColor    = c; }
        void setEmissionStrength(float s)                { m_Data.EmissionStrength = s; }
        void setEmission        (const Vec3& c, float s) { m_Data.EmissionColor    = c; m_Data.EmissionStrength = s; }

        // Getters
        const VyMaterialData& getData() const { return m_Data; }
        
        VkDescriptorSet descriptorSet() const 
        { 
            return m_DescriptorSet; 
        }
        
        bool hasTextures() const 
        { 
            return 
                m_HasAlbedoTexture    || 
                m_HasNormalTexture    || 
                m_HasRoughnessTexture || 
                m_HasMetallicTexture; 
        }

        // Update descriptor set
        void updateDescriptorSet(VyDescriptorSetLayout& setLayout, VyDescriptorPool& pool);

        bool albedoLoadFailed() const { return m_FailedAlbedo; }

    private:
        void createDefaultTexture();
        void createTextureImage(const String& filepath, VyImage& image);
        void createTextureImageView(VyImage& image, VyImageView& imageView);
        void createTextureSampler(VySampler& sampler);

        VyMaterialData m_Data;

        // Texture resources.
        VyImage        m_AlbedoTextureImage          ;
        VyImageView    m_AlbedoTextureImageView      ;
        VySampler      m_AlbedoTextureSampler        ;

        VyImage        m_NormalTextureImage          ;
        VyImageView    m_NormalTextureImageView      ;
        VySampler      m_NormalTextureSampler        ;

        VyImage        m_RoughnessTextureImage       ;
        VyImageView    m_RoughnessTextureImageView   ;
        VySampler      m_RoughnessTextureSampler     ;

        VyImage        m_MetallicTextureImage        ;
        VyImageView    m_MetallicTextureImageView    ;
        VySampler      m_MetallicTextureSampler      ;

        // Default white texture for when no texture is loaded
        VyImage        m_DefaultTextureImage             ;
        VyImageView    m_DefaultTextureImageView         ;
        VySampler      m_DefaultTextureSampler           ;

        VkDescriptorSet m_DescriptorSet = VK_NULL_HANDLE;

        bool m_HasAlbedoTexture    = false;
        bool m_HasNormalTexture    = false;
        bool m_HasRoughnessTexture = false;
        bool m_HasMetallicTexture  = false;

        bool m_FailedAlbedo = false;
    };
}