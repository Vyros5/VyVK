#pragma once

#include <VyEngine/GFX/Data/FrameInfo.h>

#include <VyEngine/VK/Pipeline/Pipeline.h>
#include <VyEngine/VK/Descriptors/Descriptors.h>

#include <VyEngine/Scene/ECS/Components.h>

// #define CASCADE_SHADOW_MAP_COUNT 4

namespace Vy
{
    // class VyIBLSystem;

    class VyRenderSystem
    {
    public:

        enum PushConstantType
        {
            MAIN           = 0,
            // POINTSHADOW    = 1,
            // SPOTSHADOW     = 2,
            // CASCADEDSHADOW = 3
        };

        VyRenderSystem(
            VkRenderPass                   renderPass, 
            TVector<VkDescriptorSetLayout> setLayouts //,
            // VyDescriptorPool&              descriptorPool
        );
        
        ~VyRenderSystem();

        VyRenderSystem(const VyRenderSystem&) = delete;
        VyRenderSystem& operator=(const VyRenderSystem&) = delete;

        void render(VyFrameInfo frameInfo);

        void renderObjects(VyFrameInfo frameInfo, PushConstantType type, int setCount, bool bRenderMaterial = true);

        // void setIBLSystem(VyIBLSystem* pIBLSystem);

    private:
        void createPipeline(VkRenderPass renderpass, TVector<VkDescriptorSetLayout> setLayouts);

        // void createIBLDescriptorResources();

        Unique<VyPipeline> m_MainPipeline;

        // VyIBLSystem* m_CurrentIBLSystem{ nullptr };

        // Unique<VyDescriptorPool>      m_IBLDescriptorPool;
        // Unique<VyDescriptorSetLayout> m_IBLDescriptorSetLayout;
        // TVector<VkDescriptorSet>      m_IBLDescriptorSets;
    };
}