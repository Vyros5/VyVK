#pragma once

#include <Vy/Systems/Rendering/IRenderSystem.h>

namespace Vy 
{
    class VyGridSystem : public IRenderSystem
    {
    public:
        VyGridSystem(
            VkRenderPass          renderPass, 
            VkDescriptorSetLayout globalSetLayout
        );

        // VyGridSystem(
        //     VyRenderPass&         renderPass, 
        //     VkDescriptorSetLayout globalSetLayout
        // );

        VyGridSystem(const VyGridSystem&)            = delete;
        VyGridSystem& operator=(const VyGridSystem&) = delete;

        ~VyGridSystem() override;

        virtual void render(const VyFrameInfo& frameInfo) override;

        void createPipeline(VkRenderPass& renderPass, TVector<VkDescriptorSetLayout> descSetLayouts);

    private:

        Unique<VyPipeline> m_Pipeline;
    };
} 
