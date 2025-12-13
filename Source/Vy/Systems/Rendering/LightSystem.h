#pragma once

#include <Vy/Systems/Rendering/IRenderSystem.h>
#include <Vy/Managers/Lighting/LightManager.h>

namespace Vy 
{
    class VyLightSystem : public IRenderSystem
    {
    public:
        VyLightSystem(
            VkRenderPass          renderPass, 
            VkDescriptorSetLayout globalSetLayout
        );

        // VyLightSystem(
        //     VyRenderPass&         renderPass, 
        //     VkDescriptorSetLayout globalSetLayout
        // );

        VyLightSystem(const VyLightSystem&)            = delete;
        VyLightSystem& operator=(const VyLightSystem&) = delete;

        ~VyLightSystem() override;

        virtual void render(const VyFrameInfo& frameInfo) override;

        virtual void update(VyFrameInfo& frameInfo, GlobalUBO& ubo) override;
        
        // Update target-locked light rotation (call when light position or target changes)
        static void updateTargetLockedLight(EntityHandle entity, VyScene* scene);
        
        // void createPipeline(VkRenderPass& renderPass, TVector<VkDescriptorSetLayout> descSetLayouts);

    private:

        void createPointLightPipeline      (VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
        void createDirectionalLightPipeline(VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
        void createSpotLightPipeline       (VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
    
        Unique<VyPipeline> m_Pipeline;

        float m_RotationSpeed{ 0.5f };

        // Point light rendering
        Unique<VyPipeline> m_PointPipeline;

        // Directional light rendering
        Unique<VyPipeline> m_DirectionalPipeline;

        // Spot light rendering
        Unique<VyPipeline> m_SpotPipeline;
    };
} 
