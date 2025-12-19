#pragma once

#include <Vy/Systems/Rendering/IRenderSystem.h>

#include <Vy/GFX/Backend/Descriptors.h>
#include <Vy/GFX/Backend/Device.h>

namespace Vy 
{
    class VyRenderSystem : public IRenderSystem
    {
    public:
        VyRenderSystem(
            VkRenderPass                   renderPass, 
            TVector<VkDescriptorSetLayout> descSetLayouts
        );

        VyRenderSystem(const VyRenderSystem&)            = delete;
        VyRenderSystem& operator=(const VyRenderSystem&) = delete;

        ~VyRenderSystem() override;
        
        virtual void render(const VyFrameInfo& frameInfo) override;

        void createPipeline(VkRenderPass& renderPass, TVector<VkDescriptorSetLayout> descSetLayouts);

    private:
        Unique<VyPipeline> m_Pipeline;
    };
}
