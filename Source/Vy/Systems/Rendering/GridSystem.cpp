#include <Vy/Systems/Rendering/GridSystem.h>

#include <Vy/GFX/Context.h>

namespace Vy
{
    VyGridSystem::VyGridSystem(
        VkRenderPass          renderPass, 
        VkDescriptorSetLayout globalSetLayout)
    {
        createPipeline(renderPass, { globalSetLayout });
    }


    VyGridSystem::~VyGridSystem()
    {
    }


    void VyGridSystem::createPipeline(
        VkRenderPass&                  renderPass, 
        TVector<VkDescriptorSetLayout> descSetLayouts)
    {
        m_Pipeline = VyPipeline::GraphicsBuilder{}
            .addDescriptorSetLayouts(descSetLayouts)
            .addShaderStage         (VK_SHADER_STAGE_VERTEX_BIT,   "Grid.vert.spv")
            .addShaderStage         (VK_SHADER_STAGE_FRAGMENT_BIT, "Grid.frag.spv")
            .addColorAttachment     (VK_FORMAT_R16G16B16A16_SFLOAT, true)
            .setDepthAttachment     (VK_FORMAT_D32_SFLOAT)
            .setCullMode            (VK_CULL_MODE_FRONT_BIT)
            .setDepthTest           (true, false, VK_COMPARE_OP_LESS_OR_EQUAL)
            .setRenderPass          (renderPass)
			.clearVertexDescriptions() // Clear default vertex bindings and attributes.
        .buildUnique();
    }


    void VyGridSystem::render(const VyFrameInfo& frameInfo) 
    {
        m_Pipeline->bind(frameInfo.CommandBuffer);

        // Set: 0 - Global Descriptor Set
        m_Pipeline->bindDescriptorSet(frameInfo.CommandBuffer, 0, frameInfo.GlobalDescriptorSet);

        // Draw grid (assuming full-screen quad).
        vkCmdDraw(frameInfo.CommandBuffer, 6, 1, 0, 0);
    }
}