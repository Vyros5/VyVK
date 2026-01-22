#pragma once

#include <VyEngine/GFX/Data/FrameInfo.h>

#include <VyEngine/VK/Pipeline/Pipeline.h>
#include <VyEngine/VK/Descriptors/Descriptors.h>

namespace Vy
{
    class VyPointLightSystem 
    {
    public:
        VyPointLightSystem(
            VkRenderPass          renderPass, 
            VkDescriptorSetLayout gBufferSetLayout, 
            VkDescriptorSetLayout compositionSetLayout
        );
        
        VyPointLightSystem(const VyPointLightSystem&)            = delete;
        VyPointLightSystem& operator=(const VyPointLightSystem&) = delete;
        
        ~VyPointLightSystem();

        // void update(VyFrameInfo& frameInfo, GlobalUbo& ubo);
        void render(VyFrameInfo& frameInfo);

    private:
        void createPointLightPipeline(
            VkRenderPass          renderPass, 
            VkDescriptorSetLayout gBufferSetLayout, 
            VkDescriptorSetLayout compositionSetLayout
        );

        Unique<VyPipeline> m_PointPipeline;
    };
}