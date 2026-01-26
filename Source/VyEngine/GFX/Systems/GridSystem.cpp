#include <VyEngine/GFX/Systems/GridSystem.h>

#include <VyEngine/VK/Context.h>

namespace Vy
{
    VyGridSystem::VyGridSystem(
        VkRenderPass          renderPass, 
        VkDescriptorSetLayout globalSetLayout)
    {
        createPipeline( renderPass, globalSetLayout );
    }


    VyGridSystem::~VyGridSystem()
    {
    }


    void VyGridSystem::createPipeline(
        VkRenderPass          renderPass, 
        VkDescriptorSetLayout descSetLayout)
    {
        auto builder = VyPipeline::GraphicsBuilder{};
        {
            builder.setName( "grid" );
            
            builder.addDescriptorSetLayout( descSetLayout );
            
            builder.addShaderStage(VK_SHADER_STAGE_VERTEX_BIT,   "Grid.vert.spv");
            builder.addShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT, "Grid.frag.spv");
            
            // Set multisampled.
            builder.setRasterizationSamples( VyContext::device().msaaSampleCountFlagBits() );
            
            // Alpha blending.
            builder.addColorAttachment(VK_FORMAT_R16G16B16A16_SFLOAT, true);
            builder.setDepthAttachment(VK_FORMAT_D32_SFLOAT);

            builder.setCullMode(VK_CULL_MODE_FRONT_BIT);
            builder.setDepthTest(true, false, VK_COMPARE_OP_LESS_OR_EQUAL);

            // Clear default vertex bindings and attributes.
            builder.clearVertexDescriptions();

            builder.setRenderPass( renderPass );
        }

        m_Pipeline = builder.buildPtr();
    }


    void VyGridSystem::render(const VyFrameInfo& frameInfo) 
    {
        m_Pipeline->bind( frameInfo.CommandBuffer );

        // Set: 0 - Global Set
        m_Pipeline->bindDescriptorSet(frameInfo.CommandBuffer, 
            0, 
            frameInfo.GlobalSet
        );

        // Draw grid (assuming full-screen quad).
        // Since the vertex data is generated in the vertex shader, no need to bind a vertex buffer
        // Draw call - 6 vertices for the two-triangle grid (as indicated by the gridPlane array in the vertex shader)
        vkCmdDraw(frameInfo.CommandBuffer, 6, 1, 0, 0);
    }
}