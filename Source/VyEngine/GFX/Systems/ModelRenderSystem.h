// #pragma once

// #include <VyEngine/GFX/Data/FrameInfo.h>

// #include <VyEngine/VK/Pipeline/Pipeline.h>
// #include <VyEngine/VK/Descriptors/Descriptors.h>

// namespace Vy
// {
//     class VyModelRenderSystem 
//     {
//     public:
//         VyModelRenderSystem(VkRenderPass renderPass, TVector<VkDescriptorSetLayout> globalSetLayouts);
        
//         VyModelRenderSystem(const VyModelRenderSystem&)            = delete;
//         VyModelRenderSystem& operator=(const VyModelRenderSystem&) = delete;
        
//         ~VyModelRenderSystem();
        
//         void render(VyFrameInfo& frameInfo);

//     private:
//         void createPipeline(VkRenderPass renderPass, TVector<VkDescriptorSetLayout> globalSetLayouts);

//         Unique<VyPipeline> m_Pipeline;
//     };
// }