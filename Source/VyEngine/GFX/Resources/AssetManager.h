// #pragma once

// #include <VyEngine/GFX/Resources/Texture/Texture.h>
// #include <VyEngine/GFX/Resources/GLTF/Model.h>

// namespace Vy
// {

//     class AssetManager 
//     {
//     public:
//         AssetManager();

//         void preloadGlobalAssets();

//         Shared<VyGLTFModel> loadModel(
//             const TString& name,
//             const TString& filepath,
//             U32 gltfFlags = 0u,
//             float scale = 1.0f
//         );

//         Shared<VyGLTFModel> loadSkyboxModel(
//             const TString& modelName,
//             const TString& filepath
//         );

//         Shared<VyTexture> loadCubemap(
//             const TString&            name,
//             const TArray<TString, 6>& faces
//         );

//         Shared<VyTexture> loadCubemap(
//             const TString&    name,
//             const TString&    ktxFilename,
//             VkFormat          format,
//             VkImageUsageFlags usageFlags,
//             VkImageLayout     initialLayout
//         );

//         void generateIrradianceMap(VkQueue copyQueue);
//         void generatePrefilteredEnvMap(VkQueue copyQueue);
//         void generateBrdfLut(VkQueue copyQueue);

//         Shared<VyTexture> loadTexture(
//             const TString&    name,
//             const TString&    path,
//             VkFormat          format = VK_FORMAT_R8G8B8A8_SRGB,
//             VkImageUsageFlags usage = VK_IMAGE_USAGE_SAMPLED_BIT,
//             VkImageLayout     layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
//             bool              forceLinearTiling = false
//         );

//         // Getters
//         Shared<VyGLTFModel> getModel(const TString& name) const;

//         // name → texture lookup
//         Shared<VyTexture> getTexture(const TString& name) const;

//         // index → texture lookup
//         Shared<VyTexture> getTexture(size_t index) const;

//         // name → index lookup
//         size_t getTextureIndex(const TString& name) const;

//         // all textures in load order
//         const TVector<Shared<VyTexture>>& getAllTextures() const;

//         // Helpers
//         bool hasTexture(const TString& name) const;

//     private:
//         // Models
//         THashMap<TString, Shared<VyGLTFModel>> m_ModelCache;

//         // Textures
//         THashMap<TString, Shared<VyTexture>> m_Textures;  // name → texture
//         THashMap<TString, size_t>            m_TextureIndexMap; // name → index
//         TVector<Shared<VyTexture>>           m_TextureList;     // index → texture

//         VyTexture m_IrradianceCube;
//         VyTexture m_PrefilteredCube;
//         VyTexture m_BRDFLUT;

//         VkQueue m_TransferQueue;

//         // Helpers

//         static void registerTextureIfNeeded(
//             const TString&                         name,
//             const Shared<VyTexture>&               texture,
//             THashMap<TString, Shared<VyTexture>>&  textures,
//             THashMap<TString, size_t>&             textureIndexMap,
//             TVector<Shared<VyTexture>>&            textureList
//         );
//     };
// }