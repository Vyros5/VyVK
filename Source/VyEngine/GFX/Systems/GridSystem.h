#pragma once

#include <VyEngine/VK/Pipeline/Pipeline.h>
#include <VyEngine/GFX/Data/FrameInfo.h>

#include <VyEngine/VK/Descriptors/Descriptors.h>

namespace Vy 
{
    class VyGridSystem //: public IRenderSystem
    {
    public:
        VyGridSystem(
            VkRenderPass          renderPass, 
            VkDescriptorSetLayout globalSetLayout
        );

        VyGridSystem(const VyGridSystem&)            = delete;
        VyGridSystem& operator=(const VyGridSystem&) = delete;

        ~VyGridSystem();

        void render(const VyFrameInfo& frameInfo);

    private:

        void createPipeline(VkRenderPass renderPass, VkDescriptorSetLayout descSetLayouts);

        Unique<VyPipeline> m_Pipeline;
    };
} 
