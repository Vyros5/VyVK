#pragma once

#include <Vy/Systems/Rendering/IRenderSystem.h>

#include <Vy/Managers/Lighting/LightManager.h>

#include <Vy/GFX/Backend/Descriptors.h>
#include <Vy/GFX/Backend/Device.h>

namespace Vy 
{
    class VyRenderSystem : public IRenderSystem
    {
    public:
        VyRenderSystem(
            VkRenderPass          renderPass, 
            VkDescriptorSetLayout globalSetLayout,
            VkDescriptorSetLayout materialSetLayout
        );
        
        // VyRenderSystem(
        //     VkRenderPass          renderPass, 
        //     VkDescriptorSetLayout globalSetLayout,
        //     VkDescriptorSetLayout materialSetLayout,
        //     VkDescriptorSetLayout shadowSamplerLayout,
        //     VyLightManager        lightManager
        // );

        // VyRenderSystem(
        //     VyRenderPass&         renderPass, 
        //     VkDescriptorSetLayout globalSetLayout,
        //     VkDescriptorSetLayout materialSetLayout
        // );

        VyRenderSystem(const VyRenderSystem&)            = delete;
        VyRenderSystem& operator=(const VyRenderSystem&) = delete;

        ~VyRenderSystem() override;

        virtual void render(const VyFrameInfo& frameInfo) override;

        // void render(const VyFrameInfo& frameInfo, VkDescriptorSet shadowSamplerDescriptorSet);

        void createPipeline(VkRenderPass& renderPass, TVector<VkDescriptorSetLayout> descSetLayouts);

    private:

        Unique<VyPipeline> m_Pipeline;
    };
}
