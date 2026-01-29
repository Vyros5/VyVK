// #pragma once

// #include <VyEngine/GFX/Resources/Texture/Texture.h>

// #include <VyEngine/VK/Image/Image.h>
// #include <VyEngine/VK/Image/ImageView.h>
// #include <VyEngine/VK/Image/Sampler.h>

// namespace Vy
// {
//     class Cubemap
//     {
//     public:
//         Cubemap() {}

//         Cubemap(const TString& folderPath, const TString& extension);

//         ~Cubemap() {}

//         void createFromHdri(const TString& filepath, U32 resolution = 512);
//         void generateIBL(U32 irradianceResolution = 32);

//         VkDescriptorImageInfo imageInfo()           const { return m_Texture.descriptorImageInfo(); };
//         VkDescriptorImageInfo irradianceImageInfo() const { return m_Irradiance.descriptorImageInfo(); };

//     private:
//         void createCubemap(const TArray<TString, 6>& facePaths);
//         void createTexture(const TString& folderPath, const TString& extension = "png");
//         void equirect2Cubemap(const TString& filepath);

//         void generateIrradiance(U32 resolution);
//         void prefilterMap();
//         void loadHDR(const TString& filepath);

//         void convertEquirectToCube();

//     private:
//         struct EquirectTexture
//         {
//             VyImage     Image;
//             VyImageView View;
//             VySampler   Sampler;

//             VY_NODISCARD VkDescriptorImageInfo descriptorImageInfo() const
//             {
//                 return VkDescriptorImageInfo{
//                     .sampler     = Sampler.handle(),
//                     .imageView   = View   .handle(),
//                     .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
//                 };
//             }

//         } m_Equirect;

//         struct SkyboxTexture
//         {
//             VyImage     Image;
//             VyImageView View;
//             VySampler   Sampler;

//             VY_NODISCARD VkDescriptorImageInfo descriptorImageInfo() const
//             {
//                 return VkDescriptorImageInfo{
//                     .sampler     = Sampler.handle(),
//                     .imageView   = View   .handle(),
//                     .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
//                 };
//             }

//         } m_Skybox;

//         struct BRDFTexture
//         {
//             VyImage     Image;
//             VyImageView View;
//             VySampler   Sampler;

//             VY_NODISCARD VkDescriptorImageInfo descriptorImageInfo() const
//             {
//                 return VkDescriptorImageInfo{
//                     .sampler     = Sampler.handle(),
//                     .imageView   = View   .handle(),
//                     .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
//                 };
//             }
            
//         } m_BRDF;

//         struct IrradianceTexture
//         {
//             VyImage     Image;
//             VyImageView View;
//             VySampler   Sampler;

//             VY_NODISCARD VkDescriptorImageInfo descriptorImageInfo() const
//             {
//                 return VkDescriptorImageInfo{
//                     .sampler     = Sampler.handle(),
//                     .imageView   = View   .handle(),
//                     .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
//                 };
//             }
            
//         } m_Irradiance;

//         struct PrefilteredTexture
//         {
//             VyImage     Image;
//             VyImageView View;
//             VySampler   Sampler;

//             VY_NODISCARD VkDescriptorImageInfo descriptorImageInfo() const
//             {
//                 return VkDescriptorImageInfo{
//                     .sampler     = Sampler.handle(),
//                     .imageView   = View   .handle(),
//                     .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
//                 };
//             }
            
//         } m_Prefiltered;


//         struct CubemapTexture
//         {
//             VyImage     Image;
//             VyImageView View;
//             VySampler   Sampler;

//             int Size;
//             U32 MipLevels;

//             VY_NODISCARD VkDescriptorImageInfo descriptorImageInfo() const
//             {
//                 return VkDescriptorImageInfo{
//                     .sampler     = Sampler.handle(),
//                     .imageView   = View   .handle(),
//                     .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
//                 };
//             }

//         } m_Texture;

//         struct HDRITexture
//         {
//             VyImage     Image;
//             VyImageView View;
//             VySampler   Sampler;

//             VkExtent2D Extent;
//             U32 MipLevels;

//             VY_NODISCARD VkDescriptorImageInfo descriptorImageInfo() const
//             {
//                 return VkDescriptorImageInfo{
//                     .sampler     = Sampler.handle(),
//                     .imageView   = View   .handle(),
//                     .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
//                 };
//             }

//         } m_HDRI;
//     };
// }