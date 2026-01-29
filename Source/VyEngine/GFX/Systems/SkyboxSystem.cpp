#include <VyEngine/GFX/Systems/SkyboxSystem.h>

#include <VyEngine/VK/Context.h>
#include <VyEngine/Globals.h>

namespace Vy
{
    // struct SkyboxPushConstants
    // {
    //     Mat4 ViewProjection;
    // };


    VySkyboxSystem::VySkyboxSystem(VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout)
    {
        createDescriptorSetLayout();

        createPipeline( renderPass, globalSetLayout );
    }


    VySkyboxSystem::~VySkyboxSystem()
    {
    }


    void VySkyboxSystem::createDescriptorSetLayout()
    {
        m_DescriptorSetLayout = VyDescriptorSetLayout::Builder{}
            .addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT) // Skybox Sampler
            .buildPtr();

        m_DescriptorPool = VyDescriptorPool::Builder{}
            .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, static_cast<U32>( MAX_FRAMES_IN_FLIGHT ))
            .setMaxSets (static_cast<U32>( MAX_FRAMES_IN_FLIGHT ))
            .buildPtr();

        // Allocate descriptor sets.
        m_DescriptorSets.resize( MAX_FRAMES_IN_FLIGHT );

        m_DescriptorSets = m_DescriptorPool->allocateSets( *m_DescriptorSetLayout, MAX_FRAMES_IN_FLIGHT );
    }


    void VySkyboxSystem::createPipeline(VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout)
    {
        auto setLayouts = TVector{ 
            globalSetLayout,                // Global set (0)
            m_DescriptorSetLayout->handle() // Skybox set (1)
        };

        auto builder = VyPipeline::GraphicsBuilder{};
        {
            builder.setName( "skybox" );
            
            builder.addDescriptorSetLayouts( setLayouts );
            
            builder.addShaderStage(VK_SHADER_STAGE_VERTEX_BIT,   "Sky/Skybox.vert.spv");
            builder.addShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT, "Sky/Skybox.frag.spv");
            
            // No alpha blending or depth.
            builder.addColorAttachment( VK_FORMAT_R16G16B16A16_SFLOAT );

            // Set multisampled. (SAMPLE_COUNT_8_BIT)
            builder.setRasterizationSamples( VyContext::device().msaaSampleCountFlagBits() );

            // Draw triangles.
            builder.setTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

            // Disable culling for debugging.
            builder.setCullMode(VK_CULL_MODE_BACK_BIT);

            // Disable depth test - skybox renders first, everything else will overdraw.
            builder.setDepthTest(false, false, VK_COMPARE_OP_LESS_OR_EQUAL);

            // Use Counter-Clockwise faces.
            builder.setFrontFace(VK_FRONT_FACE_COUNTER_CLOCKWISE);

            // No vertex input - vertices generated in the shader.
            builder.clearVertexDescriptions();
            
            builder.setRenderPass( renderPass );
        }

        m_Pipeline = builder.buildPtr();
    }


    void VySkyboxSystem::render(VyFrameInfo& frameInfo, VySkybox* pSkybox)
    {
        if (pSkybox)
        {
            // Update descriptor set with skybox texture.
            VkDescriptorImageInfo imageInfo = pSkybox->descriptorImageInfo();

            VyDescriptorWriter{ *m_DescriptorSetLayout, *m_DescriptorPool }
                .writeImage( 0, &imageInfo )
                .update( m_DescriptorSets[ frameInfo.FrameIndex ] );

            // Bind the skybox pipeline and descriptor sets.
            m_Pipeline->bind( frameInfo.CommandBuffer );

            // Set 0 - Global UBO
            // Set 1 - Skybox Sampler
            TVector<VkDescriptorSet> globSet = { 
                frameInfo.GlobalSet,
                m_DescriptorSets[ frameInfo.FrameIndex ]
            };

            m_Pipeline->bindDescriptorSets(frameInfo.CommandBuffer,
                0, 
                globSet
            );

            // Draw 36 vertices for a cube.
            // 3 Vertices per triangle, 2 Tris per face, 6 Faces (12 Tris total) = 36 Vertices.
            vkCmdDraw(frameInfo.CommandBuffer, 36, 1, 0, 0);
        }
    }
}