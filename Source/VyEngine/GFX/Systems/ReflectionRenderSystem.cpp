// #include <VyEngine/GFX/Systems/ReflectionRenderSystem.h>

// #include <VyEngine/VK/Context.h>
// #include <VyEngine/Globals.h>

// #include <VyEngine/GFX/Resources/Texture/Texture.h>
// #include <VyEngine/Scene/ECS/Components.h>

// #include <iostream>
// #include <VyLib/Common/AnsiColor.h>

// namespace Vy
// {

// 	struct MappingsPushConstantData {
// 		glm::mat4 ModelMatrix{ 1.f };
// 		glm::mat4 NormalMatrix{ 1.f };
// 	};

// 	struct UVReflectionMapPushConstantData{
// 		glm::mat4 ModelMatrix{ 1.f };
// 		float     Reflection;
// 	};


// 	//Add here descriptor set
// 	VyReflectionRenderSystem::VyReflectionRenderSystem(
//         VkRenderPass mappingsRenderPass, 
//         VkDescriptorSetLayout mappingsSetLayout, 
//         VkRenderPass uvReflectionMapRenderPass, 
//         VkDescriptorSetLayout uvReflectionMapSetLayout)
//     {
// 		createMappingsPipeline(mappingsRenderPass, mappingsSetLayout);
// 		createUVReflectionMapPipeline(uvReflectionMapRenderPass, uvReflectionMapSetLayout);
// 	}

// 	VyReflectionRenderSystem ::~VyReflectionRenderSystem () 
//     {
// 	}


// 	void VyReflectionRenderSystem ::createMappingsPipeline(VkRenderPass mappingsRenderPass, VkDescriptorSetLayout mappingsSetLayout) 
//     {
//         auto builder = VyPipeline::GraphicsBuilder{};
//         {
//             builder.setName( "mappings" );
            
//             builder.addDescriptorSetLayout( mappingsSetLayout );
            
//             builder.addPushConstantRange(VK_SHADER_STAGE_VERTEX_BIT, sizeof(MappingsPushConstantData));
            
//             builder.addShaderStage(VK_SHADER_STAGE_VERTEX_BIT,   "Mappings.vert.spv");
//             builder.addShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT, "Mappings.frag.spv");
            
//             // 1 Attachment - No alpha blending.
//             builder.addColorAttachment(VK_FORMAT_R16G16B16A16_SFLOAT, false);
//             builder.setDepthAttachment(VK_FORMAT_D32_SFLOAT);

//             // builder.setCullMode (VK_CULL_MODE_BACK_BIT);
//             // builder.setFrontFace(VK_FRONT_FACE_COUNTER_CLOCKWISE);
//             builder.setRenderPass( mappingsRenderPass );

//             builder.setSubpass( 0 );
//         }

//         m_MappingsPipeline = builder.buildPtr();
// 	}


// 	void VyReflectionRenderSystem ::createUVReflectionMapPipeline(VkRenderPass uvReflectionMapRenderPass, VkDescriptorSetLayout uvReflectionMapSetLayout) 
//     {
//         auto builder = VyPipeline::GraphicsBuilder{};
//         {
//             builder.setName( "uv_reflection" );
            
//             builder.addDescriptorSetLayout( uvReflectionMapSetLayout );
            
//             builder.addPushConstantRange(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(UVReflectionMapPushConstantData));
            
//             builder.addShaderStage(VK_SHADER_STAGE_VERTEX_BIT,   "UVReflection.vert.spv");
//             builder.addShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT, "UVReflection.frag.spv");
            
//             // 1 Attachment - No alpha blending.
//             builder.addColorAttachment(VK_FORMAT_R16G16B16A16_SFLOAT, false);
//             builder.setDepthAttachment(VK_FORMAT_D32_SFLOAT);

//             // builder.setCullMode (VK_CULL_MODE_BACK_BIT);
//             // builder.setFrontFace(VK_FRONT_FACE_COUNTER_CLOCKWISE);
//             builder.setRenderPass( uvReflectionMapRenderPass );

//             builder.setSubpass( 0 );
//         }

//         m_UVReflectionMapPipeline = builder.buildPtr();
// 	}

// 	void VyReflectionRenderSystem ::renderMappings(VyFrameInfo& frameInfo) 
//     {
// 		m_MappingsPipeline->bind( frameInfo.CommandBuffer );

//         // Set: 0 - Mappings Set
//         m_MappingsPipeline->bindDescriptorSet(frameInfo.CommandBuffer, 
//             0, 
//             frameInfo.MappingsSet
//         );

//         auto view = frameInfo.Scene->registry().view<ModelComponent, TransformComponent>();
        
//         for (auto&& [ entity, modelComp, transform ] : view.each())
//         {
// 			MappingsPushConstantData push{};
//             {
//                 push.ModelMatrix  = transform.modelMatrix(); 
//                 push.NormalMatrix = transform.normalMatrix();
//             }

//             // Push constants data.
//             m_MappingsPipeline->pushConstants(frameInfo.CommandBuffer, 
//                 VK_SHADER_STAGE_VERTEX_BIT, 
//                 &push, 
//                 sizeof(MappingsPushConstantData), 
//                 0
//             );

//             modelComp.Model->bind( frameInfo.CommandBuffer );
//             modelComp.Model->draw( frameInfo.CommandBuffer );
// 		}
// 	}

	
//     void VyReflectionRenderSystem ::renderUVReflectionMap(VyFrameInfo& frameInfo) 
//     {
// 		m_UVReflectionMapPipeline->bind( frameInfo.CommandBuffer );

//         // Set: 0 - UV Reflection Set
//         m_UVReflectionMapPipeline->bindDescriptorSet(frameInfo.CommandBuffer, 
//             0, 
//             frameInfo.UVReflectionSet
//         );

//         auto view = frameInfo.Scene->registry().view<ModelComponent, UVReflectionComponent, TransformComponent>();
        
//         for (auto&& [ entity, modelComp, uvReflection, transform ] : view.each())
//         {
// 			UVReflectionMapPushConstantData push{};
//             {
//                 push.ModelMatrix = transform.modelMatrix(); 
//                 push.Reflection  = uvReflection.Reflection;
//             }

//             // Push constants data.
//             m_UVReflectionMapPipeline->pushConstants(frameInfo.CommandBuffer, 
//                 VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
//                 &push, 
//                 sizeof(UVReflectionMapPushConstantData), 
//                 0
//             );

//             modelComp.Model->bind( frameInfo.CommandBuffer );
//             modelComp.Model->draw( frameInfo.CommandBuffer );
// 		}
// 	}
// }