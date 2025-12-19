#pragma once

#include <Vy/GFX/Resources/ShadowMap.h>
#include <Vy/GFX/Backend/Descriptors.h>
#include <Vy/GFX/Backend/Pipeline.h>
#include <Vy/GFX/FrameInfo.h>
#include <Vy/Systems/Rendering/IRenderSystem.h>

namespace Vy
{
    struct ShadowUBOData 
    {
        alignas(16) Mat4 cascadeViewProj[4];
        alignas(16) Vec4 cascadeSplits;
        alignas(16) Vec4 shadowParams;
        alignas(16) Mat4 lightSpaceMatrix; 
    };

    class ShadowRenderer 
    {
    public:
        ShadowRenderer(VkDescriptorSetLayout descriptorSetLayout);
        ~ShadowRenderer();

        void render(const VyFrameInfo& frameInfo);
        
        VkImageView shadowImageView() const { return m_ShadowArrayImageView.handle(); }
        VkSampler   shadowSampler()   const { return m_ShadowSampler.handle(); }
        
        ShadowUBOData computeShadowData(const VyCamera& camera, const Vec3& lightDir, float aspectRatio);

    private:
        void createRenderPass();
        void createResources();
        void createPipeline(VkDescriptorSetLayout descriptorSetLayout);
        void calculateCascadeSplits(float nearClip, float farClip, float lambda = 0.5f);

        VkRenderPass     renderPass;
        Unique<VyPipeline>       m_ShadowPipeline;
        
        static const U32 CASCADE_COUNT = 4;
        static const U32 SHADOW_MAP_SIZE = 2048;
        
        VyImage     m_ShadowArrayImage;
        VyImageView m_ShadowArrayImageView;
        VySampler   m_ShadowSampler;
        
        TArray<VyImageView,   CASCADE_COUNT> m_CascadeViews;
        TArray<VkFramebuffer, CASCADE_COUNT> m_CascadeFramebuffers;
        
        TArray<float, 4> cascadeSplitDistances;
    };
}