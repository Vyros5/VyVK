#pragma once

#include <VyEngine/GFX/Data/FrameInfo.h>

#include <VyEngine/VK/Pipeline/Pipeline.h>
#include <VyEngine/VK/Descriptors/Descriptors.h>

namespace Vy
{
    class VyPointLightSystem
    {
    public:
        VyPointLightSystem(VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
        ~VyPointLightSystem();

        VyPointLightSystem(const VyPointLightSystem&) = delete;
        VyPointLightSystem& operator=(const VyPointLightSystem&) = delete;

        void update(VyFrameInfo& frameInfo, GlobalUbo& globalUBO);
        void render(VyFrameInfo& frameInfo, GlobalUbo& globalUBO);

        Vec3 Point = { 1.0f, -6.0f, 0.0f };
    private:
        void createPipeline(
            VkRenderPass          renderPass, 
            VkDescriptorSetLayout globalSetLayout
        );

        Unique<VyPipeline>        m_Pipeline;
        TMap<float, EntityHandle> m_SortedLights;
    };
}

// namespace Vy
// {
//     class VyPointLightSystem 
//     {
//     public:
//         VyPointLightSystem(
//             VkRenderPass          renderPass, 
//             VkDescriptorSetLayout gBufferSetLayout, 
//             VkDescriptorSetLayout compositionSetLayout
//         );
        
//         VyPointLightSystem(const VyPointLightSystem&)            = delete;
//         VyPointLightSystem& operator=(const VyPointLightSystem&) = delete;
        
//         ~VyPointLightSystem();

//         // void update(VyFrameInfo& frameInfo, GlobalUbo& ubo);
//         void render(VyFrameInfo& frameInfo);

//     private:
//         void createPointLightPipeline(
//             VkRenderPass          renderPass, 
//             VkDescriptorSetLayout gBufferSetLayout, 
//             VkDescriptorSetLayout compositionSetLayout
//         );

//         Unique<VyPipeline> m_Pipeline;
//     };
// }