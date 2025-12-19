#include <Vy/Systems/Rendering/RenderSystem.h>

#include <Vy/GFX/Context.h>
#include <Vy/Globals.h>

namespace Vy 
{
    VyRenderSystem::VyRenderSystem(
        VkRenderPass                   renderPass, 
        TVector<VkDescriptorSetLayout> descSetLayouts)
    {
        createPipeline(renderPass, descSetLayouts);
    }


    VyRenderSystem::~VyRenderSystem()
    {
    }


    void VyRenderSystem::createPipeline(VkRenderPass& renderPass, TVector<VkDescriptorSetLayout> descSetLayouts)
    {
        m_Pipeline = VyPipeline::GraphicsBuilder{}
            .addDescriptorSetLayouts(descSetLayouts)
            .addPushConstantRange   (VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(MaterialPushConstantData))
            .addShaderStage         (VK_SHADER_STAGE_VERTEX_BIT,   "Material.vert.spv")
            .addShaderStage         (VK_SHADER_STAGE_FRAGMENT_BIT, "Material.frag.spv")
            .addColorAttachment     (VK_FORMAT_R16G16B16A16_SFLOAT)
            .setDepthAttachment     (VK_FORMAT_D32_SFLOAT)
            .setRenderPass          (renderPass)
        .buildUnique();
    }


    void VyRenderSystem::render(const VyFrameInfo& frameInfo) 
    {
        // Bind pipeline.
        m_Pipeline->bind(frameInfo.CommandBuffer);
        
        // Bind Global descriptor set ( 0 ).
        m_Pipeline->bindDescriptorSet(frameInfo.CommandBuffer, 0, frameInfo.GlobalDescriptorSet);

        // 
        auto view = frameInfo.Scene->registry().view<ModelComponent, TransformComponent>();
        
        for (auto&& [ entity, model, transform ] : view.each())
        {
            MaterialPushConstantData push{};
            {
                push.ModelMatrix  = transform.matrix();
                push.NormalMatrix = transform.normalMatrix();
            }

            // Optional material.
            if (auto* material = frameInfo.Scene->registry().try_get<MaterialComponent>(entity))
            {
                if (material->Material)
                {
                    // Copy material data.
                    const auto& matData = material->Material->getData();
                    {
                        push.Albedo           = matData.Albedo;
                        push.Metallic         = matData.Metallic;
                        push.Roughness        = matData.Roughness;
                        push.AO               = matData.AO;
                        push.TextureOffset    = matData.TextureOffset;
                        push.TextureScale     = matData.TextureScale;
                        
                        push.EmissionColor    = matData.EmissionColor;
                        push.EmissionStrength = matData.EmissionStrength;
                    }
                    
                    // Bind material descriptor set ( 1 ). 
                    VkDescriptorSet materialDescriptorSet = material->Material->descriptorSet();
                    
                    m_Pipeline->bindDescriptorSet(frameInfo.CommandBuffer, 1, materialDescriptorSet);
                }
            }
            else {
                // Optional color fallback.
                if (auto* color = frameInfo.Scene->registry().try_get<ColorComponent>(entity))
                {
                    push.Albedo = color->Color;
                }

                // Default emission for non-material objects.
                push.EmissionColor    = Vec3(0.0f);
                push.EmissionStrength =      0.0f;
            }

            // Push material constants data.
            m_Pipeline->pushConstants(frameInfo.CommandBuffer, 
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 
                push
            );
            
            // Bind and draw model data.
            model.Model->bind(frameInfo.CommandBuffer);
            model.Model->draw(frameInfo.CommandBuffer);
        }
    }
}
