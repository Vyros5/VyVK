#include <Vy/Systems/Rendering/ShadowMapSystem.h>

#include <Vy/GFX/Context.h>
#include <Vy/Globals.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// namespace Vy
// {
//     struct SimpleShadowPushConstantData {
//         glm::vec4 lightPos{0.0f};
//     };

// 	ShadowRenderSystem::ShadowRenderSystem(Renderer& renderer, TVector<Unique<VyDescriptorSetLayout>>& globalDescriptorSetLayout)
// 	    : m_Renderer(renderer),
//         m_UBOBuffers(MAX_FRAMES_IN_FLIGHT), 
//         m_UBODescriptorSets(MAX_FRAMES_IN_FLIGHT), 
//         m_Resources(std::make_unique<Framebuffer>()) 
//     {
// 		for (int i = 0; i < m_UBOBuffers.size(); ++i) 
//         {
// 			// Create a UBO buffer. This will just be one instance per frame.
// 			m_UBOBuffers[i] = std::make_unique<VyBuffer>( VyBuffer::uniformBuffer(sizeof(ShadowUbo)), false);
// 			    // We could make it device local but the performance gains could be cancelled out from writing to the UBO every frame.

// 			// Map the buffer's memory so we can begin writing to it.
// 			m_UBOBuffers[i]->map();
// 		}
// 		createResources();

// 		createDescriptorSetLayout();
// 		createDescriptorSet();

// 		// We only care about the dynamic UBO for this render system.
// 		createPipelines(globalDescriptorSetLayout[1]);
// 	}

// 	void ShadowRenderSystem::createResources() 
//     {
// 		AttachmentCreateInfo attachmentCreateInfo{};
// 		attachmentCreateInfo.Format = VK_FORMAT_D32_SFLOAT;
// 		attachmentCreateInfo.Width = SHADOW_WIDTH;
// 		attachmentCreateInfo.Height = SHADOW_HEIGHT;
// 		attachmentCreateInfo.ImageSampleCount = VK_SAMPLE_COUNT_1_BIT;
// 		attachmentCreateInfo.LayerCount = 6;
// 		attachmentCreateInfo.Usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
// 		attachmentCreateInfo.LoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
// 		attachmentCreateInfo.StoreOp = VK_ATTACHMENT_STORE_OP_STORE;
// 		attachmentCreateInfo.StencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
// 		attachmentCreateInfo.StencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
// 		attachmentCreateInfo.Flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

// 		m_Resources->createAttachment(attachmentCreateInfo);
// 		m_Resources->createSampler(SHAODW_FILTER, SHAODW_FILTER, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER);
// 		m_Resources->createMultiViewRenderPass(attachmentCreateInfo.LayerCount);
// 	}

// 	// Create a Descriptor Set Layout for a Uniform Buffer Object (UBO) & Textures.
// 	void ShadowRenderSystem::createDescriptorSetLayout() 
//     {
// 		m_DescriptorSetLayout = VyDescriptorSetLayout::Builder{}
//             .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT) // Binding 0: Vertex shader uniform buffer
//         .buildUnique();
// 	}

// 	// Create Descriptor Set.
// 	void ShadowRenderSystem::createDescriptorSet() {
// 		// Create descriptor sets for UBO.
// 		for (int i = 0; i < m_UBODescriptorSets.size(); ++i) 
//         {
// 			auto bufferInfo = m_UBOBuffers[i]->descriptorBufferInfo();

// 			VyDescriptorWriter(*m_DescriptorSetLayout, *VyContext::globalPool())
// 			    .writeBuffer(0, &bufferInfo)
// 			    .build(m_UBODescriptorSets[i]);
// 		}
// 	}


// 	// Create the graphics pipeline defined in aspen_pipeline.cpp
// 	void ShadowRenderSystem::createPipelines(Unique<VyDescriptorSetLayout>& globalDescriptorSetLayout) 
//     {
//         m_OmniShadowMappingPipeline = VyPipeline::GraphicsBuilder{}
//             .addDescriptorSetLayout(m_DescriptorSetLayout->handle())
//             .addDescriptorSetLayout(globalDescriptorSetLayout->handle())
//             .addPushConstantRange(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(SimpleShadowPushConstantData))
//             .addShaderStage(VK_SHADER_STAGE_VERTEX_BIT,   "Shadows/ShadowMap.vert.spv")
//             .addShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT, "Shadows/ShadowMap.frag.spv")
//             // Cull front faces to reduce peter-panning
//             // .setCullMode(VK_CULL_MODE_FRONT_BIT)
//             // .setCullMode(VK_CULL_MODE_BACK_BIT)
            
//             // Depth bias to prevent shadow acne
//             .setDepthBias(/*ConstantFactor*/ 1.25f, /*Clamp*/ 0.0f, /*SlopeFactor*/ 1.75f)

//             .setDepthTest(/*DepthTest*/ true, /*DepthWrite*/ true, VK_COMPARE_OP_LESS_OR_EQUAL)

//             // .addColorAttachment(VK_FORMAT_R16G16B16A16_SFLOAT, false)
//             // .setDepthAttachment(VK_FORMAT_D32_SFLOAT)
//             .setRenderPass(m_Resources->renderPass())
//         .buildUnique();

// 		// pipelineConfig.rasterizationInfo.cullMode = VK_CULL_MODE_BACK_BIT;
// 		// pipelineConfig.rasterizationInfo.cullMode = VK_CULL_MODE_FRONT_BIT;
// 	}

// 	VyRenderInfo ShadowRenderSystem::prepareRenderInfo() 
//     {
// 		VyRenderInfo renderInfo{};
// 		renderInfo.RenderPass  = m_Resources->renderPass();
// 		renderInfo.Framebuffer = m_Resources->framebuffer();

// 		TVector<VkClearValue> clearValues{1};
// 		// clearValues[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
// 		clearValues[0].depthStencil = {1.0f, 0};
// 		renderInfo.ClearValues = clearValues;

// 		VkViewport viewport{};
// 		viewport.x = 0.0f;
// 		viewport.y = 0.0f;
// 		viewport.width = SHADOW_WIDTH;
// 		viewport.height = SHADOW_HEIGHT;
// 		viewport.minDepth = 0.0f;
// 		viewport.maxDepth = 1.0f;
// 		renderInfo.Viewport = viewport;

// 		renderInfo.Scissor = VkRect2D{{0, 0}, {SHADOW_WIDTH, SHADOW_HEIGHT}};

// 		return renderInfo;
// 	}

	
//     void ShadowRenderSystem::updateUBOs(VyFrameInfo& frameInfo) 
//     {
// 		ShadowUbo shadowUbo{};

// 		// Invert X and half Z.
// 		const Mat4 clip(-1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 0.5f, 1.0f);

// 		shadowUbo.ProjectionMatrix = clip * glm::perspective(static_cast<float>(glm::pi<float>() / 2.0f), 1.0f, 0.01f, 25.0f);

// 		for (int i = 0; i < 6; ++i) 
//         {
// 			Mat4 viewMatrix = Mat4(1.0f);
// 			// Mat4 viewMatrix = glm::translate(Mat4(1.0f), Vec3{0.0f, -1.0f, 2.5f});
// 			// Vec3 lightPos = Vec3{0.0f, -1.0f, 2.5f};
// 			auto pointLightGroup      = frameInfo.Scene->registry().group<PointLightComponent>(entt::get<TransformComponent>);
// 			auto& pointLightTransform = pointLightGroup.get<TransformComponent>(pointLightGroup[0]);

// 			Vec3 lightPos = pointLightTransform.Translation;

// 			switch (i) 
//             {
// 				case 0: // POSITIVE_X
// 					viewMatrix = glm::lookAt(lightPos, lightPos + Vec3(1.0f, 0.0f, 0.0f), Vec3(0.0f, 1.0f, 0.0f));
// 					// viewMatrix = glm::rotate(viewMatrix, glm::radians(90.0f), Vec3(0.0f, 1.0f, 0.0f));
// 					// viewMatrix = glm::rotate(viewMatrix, glm::radians(180.0f), Vec3(1.0f, 0.0f, 0.0f));
// 					break;
// 				case 1: // NEGATIVE_X
// 					viewMatrix = glm::lookAt(lightPos, lightPos + Vec3(-1.0f, 0.0f, 0.0f), Vec3(0.0f, 1.0f, 0.0f));
// 					// viewMatrix = glm::rotate(viewMatrix, glm::radians(-90.0f), Vec3(0.0f, 1.0f, 0.0f));
// 					// viewMatrix = glm::rotate(viewMatrix, glm::radians(180.0f), Vec3(1.0f, 0.0f, 0.0f));
// 					break;
// 				case 2: // POSITIVE_Y
// 					viewMatrix = glm::lookAt(lightPos, lightPos + Vec3(0.0f, 1.0f, 0.0f), Vec3(0.0f, 0.0f, -1.0f));
// 					// viewMatrix = glm::rotate(viewMatrix, glm::radians(-90.0f), Vec3(1.0f, 0.0f, 0.0f));
// 					break;
// 				case 3: // NEGATIVE_Y
// 					viewMatrix = glm::lookAt(lightPos, lightPos + Vec3(0.0f, -1.0f, 0.0f), Vec3(0.0f, 0.0f, 1.0f));
// 					// viewMatrix = glm::rotate(viewMatrix, glm::radians(90.0f), Vec3(1.0f, 0.0f, 0.0f));
// 					break;
// 				case 4: // POSITIVE_Z
// 					viewMatrix = glm::lookAt(lightPos, lightPos + Vec3(0.0f, 0.0f, 1.0f), Vec3(0.0f, 1.0f, 0.0f));
// 					// viewMatrix = glm::rotate(viewMatrix, glm::radians(180.0f), Vec3(1.0f, 0.0f, 0.0f));
// 					break;
// 				case 5: // NEGATIVE_Z
// 					viewMatrix = glm::lookAt(lightPos, lightPos + Vec3(0.0f, 0.0f, -1.0f), Vec3(0.0f, 1.0f, 0.0f));
// 					// viewMatrix = glm::rotate(viewMatrix, glm::radians(180.0f), Vec3(0.0f, 0.0f, 1.0f));
// 					break;
// 			}
// 			shadowUbo.ViewMatrices[i] = viewMatrix;
// 			// shadowUbo.viewMatries[i] = glm::inverse(viewMatrix);
// 		}

// 		m_UBOBuffers[frameInfo.FrameIndex]->writeToBuffer(&shadowUbo); // Write info to the UBO.
// 		m_UBOBuffers[frameInfo.FrameIndex]->flush();
// 	}


// 	void ShadowRenderSystem::render(VyFrameInfo& frameInfo) 
//     {
// 		// Bind the graphics pipieline.
// 		m_OmniShadowMappingPipeline->bind(frameInfo.CommandBuffer);

// 		auto pointLightGroup = frameInfo.Scene->registry().group<PointLightComponent>(entt::get<TransformComponent>);

// 		for (const auto& pointLightEntity : pointLightGroup) 
//         {
// 			int index = 0;
// 			auto& pointLightTransform = pointLightGroup.get<TransformComponent>(pointLightEntity);
// 			// auto  renderGroup = frameInfo.Scene->registry().group<ModelComponent>(entt::get<TransformComponent>);
			
//             auto view = frameInfo.Scene->registry().view<ModelComponent, TransformComponent>();
            
//             for (auto&& [ entity, model, transform ] : view.each())
//             {
//                 U32 dynamicOffset = index * frameInfo.DynamicOffset;

//                 TVector<VkDescriptorSet> descriptorSetsCombined { 
//                     m_UBODescriptorSets[frameInfo.FrameIndex], 
//                     frameInfo.DescriptorSets[1] 
//                 };
                
//                 m_OmniShadowMappingPipeline->bindDescriptorSets(frameInfo.CommandBuffer, 0, descriptorSetsCombined, 1, &dynamicOffset);

// 				SimpleShadowPushConstantData push{};
//                 {
//                     push.lightPos = glm::vec4(pointLightTransform.Translation, 1.0f);
//                 }

//                 m_OmniShadowMappingPipeline->pushConstants(frameInfo.CommandBuffer, VK_SHADER_STAGE_VERTEX_BIT, &push);

//                 model.Model->bind(frameInfo.CommandBuffer);
//                 model.Model->draw(frameInfo.CommandBuffer);

// 				++index;
// 			}
// 		}
// 	}

// 	void ShadowRenderSystem::onResize() {
// 		// m_Resources->clearFramebuffer();
// 		// createResources();
// 	}
// }











namespace Vy
{
    // VyShadowMapSystem::VyShadowMapSystem(
    //     VkRenderPass          shadowRenderPass, 
    //     VkDescriptorSetLayout globalSetLayout,
    //     VkExtent2D            shadowMapExtent
    // ) :
    //     m_ShadowRenderPass{ shadowRenderPass },
    //     m_ShadowMapExtent { shadowMapExtent  }
    // {
    //     // LOG_DEBUG(Vulkan, "VyShadowMapSystem created");

    //     // m_ShadowMap = MakeUnique<VyShadowMap>(2048, 2048);

    //     createPipeline(shadowRenderPass, globalSetLayout);
    // }

    // VyShadowMapSystem::~VyShadowMapSystem() 
    // {
    //     // LOG_DEBUG(Vulkan, "VyShadowMapSystem destroyed");
    // }


    // void VyShadowMapSystem::createPipeline(VkRenderPass renderPass, VkDescriptorSetLayout& globalSetLayout) 
    // {
    //     m_Pipeline = VyPipeline::GraphicsBuilder{}
    //         .addDescriptorSetLayout(globalSetLayout)
    //         .addPushConstantRange(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(ShadowMapPushConstantsData))
    //         .addShaderStage(VK_SHADER_STAGE_VERTEX_BIT,   "Shadows/ShadowMap.vert.spv")
    //         .addShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT, "Shadows/ShadowMap.frag.spv")
    //         // Cull front faces to reduce peter-panning
    //         // .setCullMode(VK_CULL_MODE_FRONT_BIT)
    //         .setCullMode(VK_CULL_MODE_BACK_BIT) // Ensure proper face culling
            
    //         // Depth bias to prevent shadow acne
    //         .setDepthBias(/*ConstantFactor*/ 1.25f, /*Clamp*/ 0.0f, /*SlopeFactor*/ 1.75f)

    //         .setDepthTest(/*DepthTest*/ true, /*DepthWrite*/ true, VK_COMPARE_OP_LESS)

    //         .addColorAttachment(VK_FORMAT_R16G16B16A16_SFLOAT, false)
    //         // .setDepthAttachment(VK_FORMAT_D32_SFLOAT)
    //         .setRenderPass(renderPass)
    //     .buildUnique();
    // }


    // void VyShadowMapSystem::render(const VyFrameInfo& frameInfo, Mat4& lightViewProj)  
    // {
    //     // Begin ShadowMap Render Pass
    //     // m_ShadowMap->beginRenderPass(frameInfo.CommandBuffer);
    //     // {
    //         m_Pipeline->bind(frameInfo.CommandBuffer);

    //         // Bind Shadow descriptor set ( 0 ).
    //         m_Pipeline->bindDescriptorSet(frameInfo.CommandBuffer, 0, frameInfo.ShadowDescriptorSet);
            
    //         auto view = frameInfo.Scene->registry().view<ModelComponent, TransformComponent>();
            
    //         for (auto&& [ entity, model, transform ] : view.each())
    //         {
    //             if (!model.Model) 
    //             {
    //                 continue;
    //             }

    //             ShadowMapPushConstantsData push{};
    //             {
    //                 push.ModelMatrix = transform.matrix();
    //             }

    //             m_Pipeline->pushConstants(frameInfo.CommandBuffer, 
    //                 VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 
    //                 &push
    //             );

    //             model.Model->bind(frameInfo.CommandBuffer);
    //             model.Model->draw(frameInfo.CommandBuffer);
    //         }
    //     // }
    //     // m_ShadowMap->endRenderPass(frameInfo.CommandBuffer);
    // }


    // Mat4 VyShadowMapSystem::getLightViewProjection(const Vec3& dirLightPos, const Vec3& cameraPosition, float sceneRadius)
    // {
    //     float zNear         =  0.01f;
    //     float zFar          = 100.0f;

    //     float lightSize     = sceneRadius * 2.0f;
        
    //     Vec3  lightTarget   = cameraPosition;
    //     Vec3  lightPosition = lightTarget - dirLightPos * sceneRadius;

    //     Mat4 depthProjectionMatrix = glm::ortho(-lightSize, lightSize, -lightSize, lightSize, zNear, zFar);
    //     Mat4 depthViewMatrix       = glm::lookAt(lightPosition, lightTarget, Vec3(0.0f, 1.0f, 0.0f));

    //     return depthProjectionMatrix * depthViewMatrix;
    // }
}