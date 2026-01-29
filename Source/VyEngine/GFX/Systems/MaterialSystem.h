// #pragma once

// #include <VyEngine/VK/Pipeline/Pipeline.h>
// #include <VyEngine/GFX/Data/FrameInfo.h>
// // #include <VyEngine/GFX/Resources/Material/Material.h>
// #include <VyEngine/GFX/Resources/Mesh/Material.h>

// #include <VyEngine/VK/Descriptors/Descriptors.h>

// namespace Vy
// {
//     class VyMaterialSystem 
//     {
//     public:
//         VyMaterialSystem();
//         ~VyMaterialSystem() = default;

//         VyMaterialSystem(const VyMaterialSystem&)            = delete;
//         VyMaterialSystem& operator=(const VyMaterialSystem&) = delete;

//         Unique<VyDescriptorSetLayout> createMaterialSetLayout();
            
//         void updateMaterials(
//             VyFrameInfo&           frameInfo, 
//             VyDescriptorSetLayout& materialSetLayout, 
//             VyDescriptorPool&      materialPool
//         );

//     private:
//     };
// }

// namespace Vy
// {
//     /**
//      * @brief Manages material descriptor sets and default textures
//      *
//      * Handles:
//      * - Material descriptor set creation and caching
//      * - Default fallback textures
//      * - Material resource management
//      */
//     class VyMaterialSystem
//     {
//     public:
//         VyMaterialSystem();
//         ~VyMaterialSystem() = default;

//         VyMaterialSystem(const VyMaterialSystem&)            = delete;
//         VyMaterialSystem& operator=(const VyMaterialSystem&) = delete;

//         // Get or create material descriptor set for a given material.
//         VkDescriptorSet getMaterialDescriptorSet(const VyPBRMaterial& material);

//         // Clear the descriptor cache (call when materials are modified).
//         void clearDescriptorCache()
//         { 
//             m_MaterialDescriptorCache.clear(); 
//         }

//         // Access to descriptor set layout
//         VkDescriptorSetLayout descriptorSetLayout() const 
//         { 
//             return m_MaterialSetLayout->handle(); 
//         }

//     private:
//         void createMaterialDescriptorSetLayout();
//         void createMaterialDescriptorPool();
//         void createDefaultTextures();


//         // Material descriptor system
//         Unique<VyDescriptorSetLayout> m_MaterialSetLayout;
//         Unique<VyDescriptorPool>      m_MaterialDescriptorPool;

//         // Cache for material descriptor sets (key = material pointer address as hash).
//         THashMap<size_t, VkDescriptorSet> m_MaterialDescriptorCache;

//         // Default textures for missing material maps.
//         Shared<VyTexture> m_DefaultWhiteTexture;
//         Shared<VyTexture> m_DefaultNormalTexture;
//     };
// }