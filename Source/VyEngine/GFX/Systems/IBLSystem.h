// #pragma once

// #include <VyEngine/VK/Descriptors/Descriptors.h>
// #include <VyEngine/VK/Pipeline/Pipeline.h>
// #include <VyEngine/GFX/Data/FrameInfo.h>
// #include <VyEngine/GFX/Resources/Texture/Skybox.h>
// #include <VyEngine/GFX/Systems/SkyboxSystem.h>

// namespace Vy
// {
//     /**
//      * @brief Image-Based Lighting (IBL) System
//      *
//      * Generates IBL textures from an environment cubemap for PBR ambient lighting:
//      * - Irradiance Map: Diffuse ambient lighting (convolved hemisphere)
//      * - Prefiltered Environment Map: Specular ambient lighting (mip-mapped by roughness)
//      * - BRDF_LUT: 2D lookup texture for split-sum approximation
//      */
//     class VyIBLSystem
//     {
//     public:
//         struct Settings
//         {
//             int   IrradianceSize        = 64;
//             int   PrefilterSize         = 512;
//             int   PrefilterMipLevels    = 9;
//             int   BrdfLUTSize           = 512;
//             int   PrefilterSampleCount  = 4096;
//             float IrradianceSampleDelta = 0.025f;
//         };

//         VyIBLSystem();

//         ~VyIBLSystem();

//         // Non-copyable
//         VyIBLSystem(const VyIBLSystem&)            = delete;
//         VyIBLSystem& operator=(const VyIBLSystem&) = delete;

//         /**
//          * @brief Generate all IBL textures from environment cubemap
//          * @param skybox Source environment cubemap
//          */
//         void generateFromSkybox(VySkybox& skybox);

//         /**
//          * @brief Generate all IBL textures from procedural sky
//          * @param skyRenderSystem System to render the sky
//          * @param settings Sky settings (sun pos, etc)
//          */
//         // void generateFromProcedural(VySkyboxSystem& skyRenderSystem, const SkyboxSettings& settings);

//         /**
//          * @brief Request regeneration of IBL textures with new settings
//          * @param settings New IBL generation settings
//          * @param skybox   Source environment cubemap
//          *
//          * Note: Actual regeneration is deferred until the next update() call.
//          */
//         void requestRegeneration(const Settings& settings, VySkybox& skybox);

//         void update();

//         void            updateSettings(const Settings& settings);
//         const Settings& getSettings() const { return m_Settings; }

//         /**
//          * @brief Check if IBL textures have been generated
//          */
//         bool isGenerated() const 
//         { 
//             return m_Generated; 
//         }

//         // Accessors for descriptor binding
//         VkDescriptorImageInfo irradianceDescriptorImageInfo()  const;
//         VkDescriptorImageInfo prefilteredDescriptorImageInfo() const;
//         VkDescriptorImageInfo brdfLUTDescriptorImageInfo()     const;

//     private:
//         void createIrradianceMap();
//         void createPrefilteredEnvMap();
//         void createBRDFLUT();
        
//         void generateIrradianceMap(VySkybox& skybox);
//         void generatePrefilteredEnvMap(VySkybox& skybox);
//         void generateBRDFLUT();
        
//         void createIrradianceResources();
//         void createPrefilterResources();
//         void createBRDFResources();
        
//         void cleanup();
        
//         bool     m_Generated = false;
//         Settings m_Settings;

//         // Irradiance cubemap
//         VyImage        m_IrradianceImage;
//         VyImageView    m_IrradianceImageView;
//         VySampler      m_IrradianceSampler;

//         // Prefiltered environment cubemap
//         VyImage        m_PrefilteredImage;
//         VyImageView    m_PrefilteredImageView;
//         VySampler      m_PrefilteredSampler;

//         // BRDF integration LUT
//         VyImage        m_BrdfLUTImage;
//         VyImageView    m_BrdfLUTImageView;
//         VySampler      m_BrdfLUTSampler;

//         // Pipeline resources for irradiance convolution
//         VkRenderPass                  m_IrradianceRenderPass = VK_NULL_HANDLE;
//         VyPipeline                    m_IrradiancePipeline;
//         Unique<VyDescriptorSetLayout> m_IrradianceDescSetLayout;
//         Unique<VyDescriptorPool>      m_IrradianceDescPool;
//         VkDescriptorSet               m_IrradianceDescSet = VK_NULL_HANDLE;

//         // Pipeline resources for prefilter convolution
//         VkRenderPass                  m_PrefilterRenderPass = VK_NULL_HANDLE;
//         VyPipeline                    m_PrefilterPipeline;
//         Unique<VyDescriptorSetLayout> m_PrefilterDescSetLayout;
//         Unique<VyDescriptorPool>      m_PrefilterDescPool;
//         VkDescriptorSet               m_PrefilterDescSet = VK_NULL_HANDLE;

//         // Pipeline resources for BRDF LUT computation
//         VyPipeline                    m_BrdfPipeline;
//         Unique<VyDescriptorSetLayout> m_BrdfDescSetLayout;
//         Unique<VyDescriptorPool>      m_BrdfDescPool;
//         VkDescriptorSet               m_BrdfDescSet = VK_NULL_HANDLE;

//         // Deferred regeneration state
//         bool      m_RegenerationRequested = false;
//         Settings  m_NextSettings;
//         VySkybox* m_NextSkybox = nullptr;
//     };
// }