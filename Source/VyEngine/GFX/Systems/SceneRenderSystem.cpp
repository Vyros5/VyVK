// #include <VyEngine/GFX/Systems/SceneRenderSystem.h>

// #include <VyEngine/VK/Context.h>
// #include <VyEngine/Globals.h>

// #include <VyEngine/GFX/Resources/Texture/Texture.h>
// #include <VyEngine/Scene/ECS/Components.h>

// #include <iostream>
// #include <VyLib/Common/AnsiColor.h>

// namespace Vy
// {
// 	struct GBufferPushConstantData {
// 		glm::mat4 ModelMatrix{ 1.f };
// 		glm::mat4 NormalMatrix{ 1.f };
// 	};

// 	struct CompositionPushConstantData {
// 		glm::mat4 InvViewProj{ 1.f };
// 		glm::vec2 InvResolution{ 1.f };
// 	};

// 	VySceneRenderSystem::VySceneRenderSystem(
//         VkRenderPass lightingRenderPass, 
//         VkDescriptorSetLayout gBufferSetLayout, 
// 		VkDescriptorSetLayout compositionSetLayout, 
//         VkRenderPass postProcessingRenderPass, 
//         VkDescriptorSetLayout postProcessingSetLayout)
//     {
// 		createGBufferPipeline(lightingRenderPass, gBufferSetLayout);
// 		createCompositionPipeline(lightingRenderPass, compositionSetLayout);
// 		createPostProcessingPipeline(postProcessingRenderPass, postProcessingSetLayout);
// 	}


//     VySceneRenderSystem::~VySceneRenderSystem() 
//     {
// 	}


// 	void VySceneRenderSystem::createGBufferPipeline(VkRenderPass lightingRenderPass, VkDescriptorSetLayout gBufferSetLayout) 
//     {
//         auto builder = VyPipeline::GraphicsBuilder{};
//         {
//             builder.setName( "gbuffer" );
            
//             builder.addDescriptorSetLayout( gBufferSetLayout );
            
//             builder.addPushConstantRange(VK_SHADER_STAGE_VERTEX_BIT, sizeof(GBufferPushConstantData));
            
//             builder.addShaderStage(VK_SHADER_STAGE_VERTEX_BIT,   "GBuffer.vert.spv");
//             builder.addShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT, "GBuffer.frag.spv");
            
//             // 2 Attachments - No alpha blending.
//             // builder.addColorAttachments(VK_FORMAT_R16G16B16A16_SFLOAT, 2);
//             builder.addColorAttachment(VK_FORMAT_R16G16B16A16_SFLOAT, false);
//             builder.addColorAttachment(VK_FORMAT_R16G16B16A16_SFLOAT, false);
            
//             builder.setDepthAttachment(VK_FORMAT_D32_SFLOAT);

//             // builder.setCullMode (VK_CULL_MODE_BACK_BIT);
//             // builder.setFrontFace(VK_FRONT_FACE_COUNTER_CLOCKWISE);
//             builder.setRenderPass( lightingRenderPass );

//             builder.setSubpass( 0 );
//         }

//         m_GBufferPipeline = builder.buildPtr();
// 	}


// 	void VySceneRenderSystem::createCompositionPipeline(VkRenderPass lightingRenderPass, VkDescriptorSetLayout compositionSetLayout) 
//     {
//         auto builder = VyPipeline::GraphicsBuilder{};
//         {
//             builder.setName( "composition" );
            
//             builder.addDescriptorSetLayout( compositionSetLayout );
            
//             builder.addPushConstantRange(VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(CompositionPushConstantData));
            
//             builder.addShaderStage(VK_SHADER_STAGE_VERTEX_BIT,   "Composition.vert.spv");
//             builder.addShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT, "Composition.frag.spv");
            
//             // 1 Attachment - No alpha blending.
//             builder.addColorAttachment(VK_FORMAT_R16G16B16A16_SFLOAT);

//             builder.setDepthAttachment(VK_FORMAT_D32_SFLOAT);

//             // builder.setCullMode (VK_CULL_MODE_BACK_BIT);
//             // builder.setFrontFace(VK_FRONT_FACE_COUNTER_CLOCKWISE);
//             builder.setRenderPass( lightingRenderPass );

//             // No Vertices.
//             builder.clearVertexDescriptions();

//             builder.setSubpass( 1 );
//         }

//         m_CompositionPipeline = builder.buildPtr();
// 	}


// 	void VySceneRenderSystem::createPostProcessingPipeline(VkRenderPass postProcessingRenderPass, VkDescriptorSetLayout postProcessingSetLayout) 
//     {
//         auto builder = VyPipeline::GraphicsBuilder{};
//         {
//             builder.setName( "post_processing" );
            
//             builder.addDescriptorSetLayout( postProcessingSetLayout );
            
//             builder.addShaderStage(VK_SHADER_STAGE_VERTEX_BIT,   "PostProcessing.vert.spv");
//             builder.addShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT, "PostProcessing.frag.spv");
            
//             // 1 Attachment - No alpha blending.
//             builder.addColorAttachment(VK_FORMAT_R16G16B16A16_SFLOAT, false);
//             builder.setDepthAttachment(VK_FORMAT_D32_SFLOAT);

//             // builder.setCullMode (VK_CULL_MODE_BACK_BIT);
//             // builder.setFrontFace(VK_FRONT_FACE_COUNTER_CLOCKWISE);
//             builder.setRenderPass( postProcessingRenderPass );

//             // No Vertices.
//             builder.clearVertexDescriptions();

//             builder.setSubpass( 0 );
//         }

//         m_PostProcessingPipeline = builder.buildPtr();
// 	}


// 	void VySceneRenderSystem::render(VyFrameInfo& frameInfo, Mat4 invViewProj, Vec2 invResolution) 
//     {
// 		// First subpass
// 		m_GBufferPipeline->bind( frameInfo.CommandBuffer );
//         {
//             // Set: 0 - GBuffer Set
//             m_GBufferPipeline->bindDescriptorSet(frameInfo.CommandBuffer, 
//                 0, 
//                 frameInfo.GBufferSet
//             );

//             auto view = frameInfo.Scene->registry().view<ModelComponent, TransformComponent>();
            
//             for (auto&& [ entity, modelComp, transform ] : view.each())
//             {
//                 if (!modelComp.Model) continue;

//                 GBufferPushConstantData push{};
//                 {
//                     push.ModelMatrix  = transform.modelMatrix(); 
//                     push.NormalMatrix = transform.normalMatrix();
//                 }

//                 // Push constants data.
//                 m_GBufferPipeline->pushConstants(frameInfo.CommandBuffer, 
//                     VK_SHADER_STAGE_VERTEX_BIT, 
//                     &push, 
//                     sizeof(GBufferPushConstantData), 
//                     0
//                 );

//                 modelComp.Model->bind( frameInfo.CommandBuffer );
//                 modelComp.Model->draw( frameInfo.CommandBuffer );
//             }
//         }

// 		// Subpass Transition (GBuffer -> Composition)
// 		vkCmdNextSubpass(frameInfo.CommandBuffer, VK_SUBPASS_CONTENTS_INLINE);

// 		// Second subpass		
// 		m_CompositionPipeline->bind( frameInfo.CommandBuffer );
//         {
//             // Set: 0 - Composition Set
//             m_CompositionPipeline->bindDescriptorSet(frameInfo.CommandBuffer, 
//                 0, 
//                 frameInfo.CompositionSet
//             );

//             CompositionPushConstantData push{};
//             {
//                 push.InvViewProj   = invViewProj;
//                 push.InvResolution = invResolution;
//             }

//             // Push composition constants data.
//             m_CompositionPipeline->pushConstants(frameInfo.CommandBuffer, 
//                 VK_SHADER_STAGE_FRAGMENT_BIT, 
//                 &push, 
//                 sizeof(CompositionPushConstantData), 
//                 0
//             );

//             // Draw quad.
//             vkCmdDraw(frameInfo.CommandBuffer, 6, 1, 0, 0);
//         }
// 	}


// 	void VySceneRenderSystem::renderPostProcessing(VyFrameInfo& frameInfo) 
//     {
// 		m_PostProcessingPipeline->bind( frameInfo.CommandBuffer );

//         // Set: 0 - Post Processing Set
//         m_PostProcessingPipeline->bindDescriptorSet(frameInfo.CommandBuffer, 
//             0, 
//             frameInfo.PostProcessingSet
//         );

//         // Draw quad.
// 		vkCmdDraw(frameInfo.CommandBuffer, 6, 1, 0, 0);
// 	}
// }