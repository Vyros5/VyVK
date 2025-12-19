#pragma once

#include <Vy/Systems/Rendering/IRenderSystem.h>

#include <Vy/GFX/Backend/Descriptors.h>
#include <Vy/GFX/Backend/Device.h>
#include <Vy/Scene/ECS/Components/PostProcessingComponent.h>

namespace Vy 
{
    class VyPostProcessSystem : public IRenderSystem
    {
    public:
        VyPostProcessSystem(VkExtent2D extent);

        VyPostProcessSystem(const VyPostProcessSystem&)            = delete;
        VyPostProcessSystem& operator=(const VyPostProcessSystem&) = delete;

        ~VyPostProcessSystem() override;

        virtual void render(const VyFrameInfo& frameInfo) override {};

        // Get the HDR render pass for rendering the scene
        VkRenderPass  getHDRRenderPass()                const { return m_HDRRenderPass; }
        VkFramebuffer getHDRFramebuffer(int frameIndex) const { return m_HDRFramebuffers[frameIndex]; }
        
        // Apply post-processing effects
        void renderPostProcess(
            VkCommandBuffer                cmdBuffer,
            int                            frameIndex,
            const PostProcessingComponent& settings
        );

        // Render final composite to current render pass
        void renderFinalComposite(
            VkCommandBuffer                cmdBuffer,
            VkRenderPass                   swapchainRenderPass,
            int                            frameIndex,
            const PostProcessingComponent& settings
        );

        // Recreate resources when window is resized.
        void recreate(VkExtent2D newExtent);

    private:

        void createHDRResources();
        void createBloomResources();
        void createRenderPasses();
        void createFramebuffers();
        void createDescriptorSetLayouts();
        void createDescriptorSets();
        void createPipelines();
        
        void cleanup();

        // ---------------------------------------------------------------

        VkExtent2D m_Extent;

        // ---------------------------------------------------------------
        // HDR scene render target
        VkRenderPass           m_HDRRenderPass;
        TVector<VkFramebuffer> m_HDRFramebuffers;

        TVector<VyImage>       m_HDRImages;
        TVector<VyImageView>   m_HDRImageViews;

        TVector<VyImage>       m_HDRDepthImages;
        TVector<VyImageView>   m_HDRDepthImageViews;

        // ---------------------------------------------------------------
        // Bloom ping-pong buffers
        VkRenderPass           m_BloomRenderPass;
        TVector<VkFramebuffer> m_BloomFramebuffers[2];

        TVector<VyImage>       m_BloomImages    [2];
        TVector<VyImageView>   m_BloomImageViews[2];

        // ---------------------------------------------------------------
        // Descriptor pools and layouts
        Unique<VyDescriptorPool>      m_DescriptorPool;
        Unique<VyDescriptorSetLayout> m_BrightnessExtractSetLayout;
        Unique<VyDescriptorSetLayout> m_BlurSetLayout;
        Unique<VyDescriptorSetLayout> m_PostProcessSetLayout;

        // ---------------------------------------------------------------
        // Descriptor sets
        TVector<VkDescriptorSet> m_BrightnessExtractDescriptorSets;
        TVector<VkDescriptorSet> m_BlurDescriptorSets[2];
        TVector<VkDescriptorSet> m_PostProcessDescriptorSets;

        // ---------------------------------------------------------------
        // Pipelines
        Unique<VyPipeline> m_BrightnessExtractPipeline;

        Unique<VyPipeline> m_BlurPipeline;
        
        Unique<VyPipeline> m_PostProcessPipeline;
        VkPipelineLayout   m_PostProcessPipelineLayout;

        // ---------------------------------------------------------------
        // Samplers
        VySampler m_HDRSampler;
        VySampler m_BloomSampler;
    };
}