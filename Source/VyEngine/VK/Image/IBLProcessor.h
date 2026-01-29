#pragma once

#include <VyEngine/GFX/Resources/Texture/Texture.h>

#include <VyEngine/VK/Image/Image.h>
#include <VyEngine/VK/Image/ImageView.h>
#include <VyEngine/VK/Image/Sampler.h>
#include <VyEngine/VK/Descriptors/Descriptors.h>
#include <VyEngine/VK/Pipeline/Pipeline.h>

namespace Vy
    {
    // IBL (Image-Based Lighting) processor
    // Handles HDR environment map loading and preprocessing for PBR rendering
    class IBLProcessor 
    {
    public:
        IBLProcessor() = default;
        ~IBLProcessor();

        bool initialize();
        // void cleanup();

        // Load and process an HDR environment map
        // This performs: equirect->cubemap, prefilter specular, generate irradiance
        bool loadEnvironmentMap(const TString& hdrPath);

        // Generate BRDF LUT (call once, reusable for all environments)
        bool generateBRDFLUT();

        // Get processed textures for rendering
        VkImageView getEnvironmentCubemapView() const { return m_EnvCubemap.View.handle(); }
        VkImageView getIrradianceCubemapView()  const { return m_Irradiance.View.handle(); }
        VkImageView getPrefilteredCubemapView() const { return m_Prefiltered.View.handle(); }
        VkImageView getBRDFLUTView()            const { return m_BRDF.View.handle(); }
        VkSampler   getCubemapSampler()         const { return m_CubemapSampler.handle(); }
        VkSampler   getBRDFSampler()            const { return m_BRDFSampler.handle(); }

        // Check if IBL is ready
        bool isReady() const { return m_EnvCubemap.View.handle() != VK_NULL_HANDLE; }

        // Cubemap size configuration
        static constexpr U32 ENV_CUBEMAP_SIZE = 512;
        static constexpr U32 IRRADIANCE_SIZE = 32;
        static constexpr U32 PREFILTER_SIZE = 128;
        static constexpr U32 PREFILTER_MIP_LEVELS = 5;
        static constexpr U32 BRDF_LUT_SIZE = 512;

    private:
        bool loadHDRImage(const TString& path, TVector<float>& pixels, U32& width, U32& height);
        bool createEquirectTexture(const TVector<float>& pixels, U32 width, U32 height);
        bool createCubemapImages();
        bool createComputePipelines();
        bool createDescriptors();

        void executeEquirectToCubemap();
        void executeIrradianceConvolution();
        void executePrefilterEnvironment();
        void executeBRDFIntegration();
        void updateDescriptorSet(VkDescriptorSet descSet, VkImageView inputView, VkSampler sampler, VkImageView outputView);

        // Equirectangular HDR input
        struct Equirect_t
        {
            VyImage     Image;
            VyImageView View;
        
        } m_Equirect;

        // Environment cubemap (HDR)
        struct EnvCubemap_t
        {
            VyImage     Image;
            VyImageView View;
        
        } m_EnvCubemap;

        // Irradiance cubemap (diffuse IBL)
        struct IrradianceCubemap_t
        {
            VyImage     Image;
            VyImageView View;
        
        } m_Irradiance;

        // Prefiltered environment map (specular IBL with roughness mips)
        struct PrefilteredCubemap_t
        {
            VyImage     Image;
            VyImageView View;

            TVector<VyImageView> MipViews;
        
        } m_Prefiltered;

        // BRDF LUT
        struct BRDFLUT_t
        {
            VyImage     Image;
            VyImageView View;
        
        } m_BRDF;

        // Samplers
        VySampler m_CubemapSampler;
        VySampler m_BRDFSampler;

        // Compute pipelines
        Unique<VyPipeline> m_EquirectToCubemapPipeline;
        Unique<VyPipeline> m_IrradiancePipeline;
        Unique<VyPipeline> m_PrefilterPipeline;
        Unique<VyPipeline> m_BRDFPipeline;
        VkPipelineLayout   m_PipelineLayout;

        // Descriptor sets
        Unique<VyDescriptorSetLayout> m_DescriptorLayout;
        Unique<VyDescriptorPool>      m_DescriptorPool;

        VkDescriptorSet m_EquirectDescSet;
        VkDescriptorSet m_IrradianceDescSet;
        VkDescriptorSet m_PrefilterDescSet;
        VkDescriptorSet m_BRDFDescSet;
    };
}