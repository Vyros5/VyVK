#include <VyEngine/GFX/Systems/Generators/BrdfLUTGenerationSystem.h>

#include <VyEngine/VK/Context.h>
#include <VyEngine/Globals.h>

#include <VyEngine/GFX/Resources/Mesh/Vertex.h>

namespace Vy
{
    struct PushResolution
    {
        Vec2 Resolution{ 512.0f, 512.0f };
    };


    BRDFLUTGenerationSystem::BRDFLUTGenerationSystem(VkRenderPass renderPass)
    {
        createPipeline( renderPass );
    }


    BRDFLUTGenerationSystem::~BRDFLUTGenerationSystem()
    {
    }


    void BRDFLUTGenerationSystem::generate(VkCommandBuffer cmdBuffer, U32 width, U32 height)
    {
        const U32          vertexCount = 3;
        const VkDeviceSize bufferSize  = sizeof(VyVertex) * vertexCount;
        const U32          vertexSize  = sizeof(VyVertex);

        m_VertexBuffer = MakeUnique<VyBuffer>( VyBuffer::vertexBuffer( "brdf_lut", vertexSize, vertexCount ));

        m_Pipeline->bind(cmdBuffer);

        const PushResolution resolution{{ width, height }};

        m_Pipeline->pushConstants( cmdBuffer,VK_SHADER_STAGE_FRAGMENT_BIT, &resolution, sizeof(PushResolution), 0 );

        //if (vkCmdSetCullModeEXT && vkCmdSetFrontFaceEXT)  // no need, we do throw and exception if not supported
        {
            vkCmdSetCullModeEXT (cmdBuffer, VK_CULL_MODE_NONE);
            vkCmdSetFrontFaceEXT(cmdBuffer, VK_FRONT_FACE_COUNTER_CLOCKWISE);
        }

        VkBuffer     buffers[] = { m_VertexBuffer->handle() };
        VkDeviceSize offsets[] = { 0 };

        vkCmdBindVertexBuffers(cmdBuffer, 0, 1, buffers, offsets);

        vkCmdDraw(cmdBuffer, vertexCount, 1, 0, 0);
    }


    void BRDFLUTGenerationSystem::createPipeline(VkRenderPass renderPass)
    {
        auto builder = VyPipeline::GraphicsBuilder{};
        {
            builder.setName( "brdf_lut" );
            
            builder.addPushConstantRange(VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(PushResolution), 0);
            // builder.addDescriptorSetLayouts( setLayouts );
            
            builder.addShaderStage(VK_SHADER_STAGE_VERTEX_BIT,   "Fullscreen.vert.spv");
            builder.addShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT, "GenBrdfLUT.frag.spv");
            
            // No alpha blending or depth.
            builder.addColorAttachment( VK_FORMAT_R16G16B16A16_SFLOAT );

            // Set multisampled. (SAMPLE_COUNT_8_BIT)
            builder.setRasterizationSamples( VyContext::device().msaaSampleCountFlagBits() );

            // Draw triangles.
            builder.setTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

            // Disable culling.
            builder.setCullMode(VK_CULL_MODE_NONE);

            // Disable depth test and write.
            builder.setDepthTest(false, false, VK_COMPARE_OP_LESS_OR_EQUAL);

            // Use Counter-Clockwise faces.
            builder.setFrontFace(VK_FRONT_FACE_COUNTER_CLOCKWISE);

            // No vertex input - vertices generated in the shader.
            builder.clearVertexDescriptions();
            
            builder.setRenderPass( renderPass );
        }

        m_Pipeline = builder.buildPtr();
    }
}