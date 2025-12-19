#pragma once

#include <Vy/Systems/Rendering/IRenderSystem.h>

#include <Vy/GFX/Backend/Descriptors.h>
#include <Vy/GFX/Backend/Device.h>
#include <Vy/GFX/Resources/Cubemap.h>

namespace Vy 
{
    class VySkyboxSystem : public IRenderSystem
    {
    public:
        VySkyboxSystem(
            VkRenderPass          renderPass, 
            VkDescriptorSetLayout globalSetLayout,
            Shared<VyEnvironment> environment
        );

        VySkyboxSystem(const VySkyboxSystem&)            = delete;
        VySkyboxSystem& operator=(const VySkyboxSystem&) = delete;

        ~VySkyboxSystem() override;

        virtual void render(const VyFrameInfo& frameInfo) override;

        void createPipeline(VkRenderPass& renderPass, TVector<VkDescriptorSetLayout> descSetLayouts);

    private:

        Shared<VySkybox>   m_Skybox;
        Unique<VyPipeline> m_Pipeline;
    };
}