// #pragma once

// #include <VyEngine/GFX/Resources/Texture/Texture.h>
// #include <VyEngine/VK/Descriptors/Descriptors.h>

// #define COLOR_TEXTURE       0x1
// #define NORMAL_TEXTURE      0x2
// #define OCCLUSION_TEXTURE   0x4
// #define ROUGH_METAL_TEXTURE 0x8

// #define COLOR_UV            0x10
// #define NORMAL_UV           0x20
// #define OCCLUSION_UV        0x40
// #define ROUGH_METAL_UV      0x80


// namespace Vy
// {
//     class VyMaterial 
//     {
//     public:
//         VyMaterial()  = default;
//         ~VyMaterial() = default;
        
//         glm::vec4 color{1.f};
//         float metalness{0.f};
//         float roughness{1.f};
        
//         enum AlphaMode{ ALPHAMODE_OPAQUE = 0, ALPHAMODE_MASK, ALPHAMODE_BLEND };
//         AlphaMode alphaMode = ALPHAMODE_OPAQUE;
//         float alphaCutoff{0.5};
        
//         void setColorTexture(size_t id) { m_TextureBitmap |= COLOR_TEXTURE; m_ColorTextureId = id; }
//         void setNormalTexture(size_t id) { m_TextureBitmap |= NORMAL_TEXTURE; m_NormalTextureId = id; }
//         void setOcclusionTexture(size_t id) { m_TextureBitmap |= OCCLUSION_TEXTURE; m_OcclusionTextureId = id; }
//         void setRoughMetalTexture(size_t id) { m_TextureBitmap |= ROUGH_METAL_TEXTURE; m_RoughMetalTextureId = id; }
        
//         void setColorTexCoordSet(int set) { m_TextureBitmap |= (set == 1) ? COLOR_UV : 0; }
//         void setNormalTexCoordSet(int set) { m_TextureBitmap |= (set == 1) ? NORMAL_UV : 0; }
//         void setOcclusionTexCoordSet(int set) { m_TextureBitmap |= (set == 1) ? OCCLUSION_UV : 0; }
//         void setMetalRoughTexCoordSet(int set) { m_TextureBitmap |= (set == 1) ? ROUGH_METAL_UV : 0; }
        
//         using Textures = TVector<Unique<const VyTexture>>;

//         VkDescriptorImageInfo getColorDescriptor(Textures& texVec)      const { return texVec[m_ColorTextureId]->descriptorImageInfo(); }
//         VkDescriptorImageInfo getNormalDescriptor(Textures& texVec)     const { return texVec[m_NormalTextureId]->descriptorImageInfo(); }
//         VkDescriptorImageInfo getOcclusionDescriptor(Textures& texVec)  const { return texVec[m_OcclusionTextureId]->descriptorImageInfo(); }
//         VkDescriptorImageInfo getMetalRoughDescriptor(Textures& texVec) const { return texVec[m_RoughMetalTextureId]->descriptorImageInfo(); }
        
//         unsigned int getTextureBitmap(void) { return m_TextureBitmap; }
        
//     private:
//         size_t m_ColorTextureId{0};
//         size_t m_NormalTextureId{0};
//         size_t m_OcclusionTextureId{0};
//         size_t m_RoughMetalTextureId{0};
        
//         U32 m_TextureBitmap{0};
//     };

//     struct Assets 
//     {
//         TVector<Unique<const VyTexture>> Textures;
//         THashMap<TString, VyMaterial>    Materials;
//     };

    
// }