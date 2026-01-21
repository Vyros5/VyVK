#pragma once

#include <VyEngine/GFX/Data/FrameInfo.h>

#include <VyEngine/VK/Pipeline/Pipeline.h>
#include <VyEngine/VK/Descriptors/Descriptors.h>

namespace Vy
{
    class VyLightSystem 
    {
    public:
        VyLightSystem(
            VkRenderPass          renderPass, 
            VkDescriptorSetLayout globalSetLayout
        );
        
        VyLightSystem(const VyLightSystem&)            = delete;
        VyLightSystem& operator=(const VyLightSystem&) = delete;
        
        ~VyLightSystem();

        void update(VyFrameInfo& frameInfo, GlobalUbo& ubo);
        void render(VyFrameInfo& frameInfo);

    private:
        void createPointLightPipeline      (VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
        // void createDirectionalLightPipeline(VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
        // void createSpotLightPipeline       (VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
    
        // [ Point-light ]
        Unique<VyPipeline> m_PointPipeline;
        
        // [ Directional-light ]
        // Unique<VyPipeline> m_DirectionalPipeline;

        // [ Spot-light x]
        // Unique<VyPipeline> m_SpotPipeline;

        TMap<float, EntityHandle> m_SortedLights;
    };
}