#pragma once

// #include <Vy/Asset/Asset.h>
// #include <Vy/GFX/Backend/Pipeline.h>
#include <Vy/GFX/Backend/Resources/Buffer.h>
#include <Vy/GFX/Backend/Resources/Image.h>
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
        
        VkDescriptorSet getDescriptorSet() const 
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
        void createTextureImage(const String& filepath, VkImage& image, VmaAllocation& imageMemory);
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
// 	struct MaterialData
// 	{
// 		VyAssetHandle AlbedoMap = VyAssetHandle::Invalid();
// 		VyAssetHandle NormalMap = VyAssetHandle::Invalid();

// 		VyAssetHandle MetallicRoughnessMap = VyAssetHandle::Invalid();
// 		VyAssetHandle AOMap = VyAssetHandle::Invalid();
// 		VyAssetHandle EmissiveMap = VyAssetHandle::Invalid();

// 		Vec4 AlbedoColor = Vec4(1.0f);
// 		float Metallic = 0.0f;
// 		float Roughness = 0.0f;
// 		Vec3 EmissiveFactor = Vec3(0.0f);

// 		// Json::Value serialize() const;

// 		// static Optional<MaterialData> deserialize(const Json::Value& root);
// 	};


// 	class Material : public VyAsset
// 	{
// 	public:
// 		class Builder 
//         {
// 		public:
// 			explicit Builder() = default;

// 			Builder& assetHandle(const VyAssetHandle& handle) { m_Handle = handle; return *this; }
//             Builder& albedoMap(const VyAssetHandle& handle) { m_Data.AlbedoMap = handle; return *this; }
//             Builder& normalMap(const VyAssetHandle& handle) { m_Data.NormalMap = handle; return *this; }
//             Builder& metallicRoughnessMap(const VyAssetHandle& handle) { m_Data.MetallicRoughnessMap = handle; return *this; }
//             Builder& ambientOcclusionMap(const VyAssetHandle& handle) { m_Data.AOMap = handle; return *this; }
//             Builder& emissiveMap(const VyAssetHandle& handle) { m_Data.EmissiveMap = handle; return *this; }

// 			Builder& albedoColor(const Vec4& color)   { m_Data.AlbedoColor = color; return *this; }
// 			Builder& metallicFactor(float metallic)   { m_Data.Metallic = metallic; return *this; }
// 			Builder& roughnessFactor(float roughness) { m_Data.Roughness = roughness; return *this; }
// 			Builder& emissiveFactor(Vec3 emissive)    { m_Data.EmissiveFactor = emissive; return *this; }

// 			Material build();

// 		private:
// 			MaterialData  m_Data;
// 			VyAssetHandle m_Handle = VyAssetHandle::Invalid();
// 		};

// 	public:

// 		Material(MaterialData materialData, VyAssetHandle assetHandle);
	
//         void bind(const VyPipeline& pipeline, VkCommandBuffer commandBuffer) const;
		
//         void reload() override;
		
//         bool checkForDirtyInDependencies() override;

// 	private:

// 		void createDescriptorSet();
	
//         MaterialData m_Data;
	
//         // std::unique_ptr<VkDescriptorSet>
//         VkDescriptorSet           m_DescriptorSet;
// 		std::unique_ptr<VyBuffer> m_MaterialBuffer;
// 	};
// }