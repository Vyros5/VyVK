#include <Vy/Systems/Rendering/SkyboxSystem.h>

#include <Vy/GFX/Context.h>
#include <Vy/Globals.h>

namespace Vy 
{
    VySkyboxSystem::VySkyboxSystem(
        VkRenderPass          renderPass,
        VkDescriptorSetLayout globalSetLayout,
        Shared<VyEnvironment> environment)
    {
        m_Skybox = environment->getSkybox();

        createPipeline(renderPass, { globalSetLayout, m_Skybox->descriptorSetLayout() });
    }


    VySkyboxSystem::~VySkyboxSystem()
    {
    }

    
    void VySkyboxSystem::createPipeline(
        VkRenderPass&                  renderPass, 
        TVector<VkDescriptorSetLayout> descSetLayouts)
    {
        m_Pipeline = VyPipeline::GraphicsBuilder{}
            .addDescriptorSetLayouts(descSetLayouts)
            .addShaderStage         (VK_SHADER_STAGE_VERTEX_BIT,   "Skybox.vert.spv")
            .addShaderStage         (VK_SHADER_STAGE_FRAGMENT_BIT, "Skybox.frag.spv")
            // No depth writing, but depth testing is enabled (equal or less than) to render behind opaque objects.
            .setDepthTest           (true, false, VK_COMPARE_OP_LESS_OR_EQUAL) // Draw skybox behind everything.
            .addColorAttachment     (VK_FORMAT_R16G16B16A16_SFLOAT)
            .setCullMode            (VK_CULL_MODE_BACK_BIT)
            .setFrontFace           (VK_FRONT_FACE_COUNTER_CLOCKWISE) 
            .setRenderPass          (renderPass)
            .clearVertexDescriptions() // Clear default vertex bindings and attributes.
        .buildUnique();
    }


    void VySkyboxSystem::render(const VyFrameInfo& frameInfo)
    {
        m_Pipeline->bind(frameInfo.CommandBuffer);

        auto globSets = TVector{ frameInfo.GlobalDescriptorSet, m_Skybox->descriptorSet() };

        // Bind global and skybox descriptor set.
        m_Pipeline->bindDescriptorSets(frameInfo.CommandBuffer, 0, globSets);

        // Draw 36 vertices (12 triangles) for a cube.
        vkCmdDraw(frameInfo.CommandBuffer, 36, 1, 0, 0);
    }

}