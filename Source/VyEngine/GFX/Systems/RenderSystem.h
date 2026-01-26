#pragma once

#include <VyEngine/GFX/Data/FrameInfo.h>

#include <VyEngine/VK/Pipeline/Pipeline.h>
#include <VyEngine/VK/Descriptors/Descriptors.h>

#include <VyEngine/Scene/ECS/Components.h>

#define CASCADE_SHADOW_MAP_COUNT 4

namespace Vy
{
    class VyRenderSystem
    {
    public:

        enum PushConstantType
        {
            MAIN           = 0,
            POINTSHADOW    = 1,
            SPOTSHADOW     = 2,
            CASCADEDSHADOW = 3
        };

        VyRenderSystem(
            VkRenderPass                   renderPass, 
            TVector<VkDescriptorSetLayout> setLayouts //,
            // VyDescriptorPool&              descriptorPool
        );
        
        ~VyRenderSystem();

        VyRenderSystem(const VyRenderSystem&) = delete;
        VyRenderSystem& operator=(const VyRenderSystem&) = delete;

        void renderMainPass(VyFrameInfo frameInfo);

        void renderObjects(VyFrameInfo frameInfo, PushConstantType type, int setCount, bool bRenderMaterial = true);

    private:
        void createPipeline(VkRenderPass renderpass, TVector<VkDescriptorSetLayout> setLayouts);

        Unique<VyPipeline> m_MainPipeline;
    };
}