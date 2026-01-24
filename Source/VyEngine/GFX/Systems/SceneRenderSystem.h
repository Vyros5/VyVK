// #pragma once

// #include <VyEngine/GFX/Data/FrameInfo.h>

// #include <VyEngine/VK/Pipeline/Pipeline.h>
// #include <VyEngine/VK/Descriptors/Descriptors.h>

// namespace Vy
// {
// 	class VySceneRenderSystem 
//     {
// 	public:
// 		static constexpr int SIERPINSKI_DEPTH = 3;

// 		static constexpr int NUMBER_OF_TRIANGLE_VERTICES = 3;

// 		VySceneRenderSystem(
//             VkRenderPass          renderPass, 
//             VkDescriptorSetLayout gBufferSetLayout, 
// 			VkDescriptorSetLayout compositionSetLayout, 
//             VkRenderPass          postProcessingRenderPass, 
//             VkDescriptorSetLayout postProcessingSetLayout
//         );
		
//         ~VySceneRenderSystem();

// 		VySceneRenderSystem(const VySceneRenderSystem&) = delete;
// 		VySceneRenderSystem& operator=(const VySceneRenderSystem&) = delete;

// 		void render(VyFrameInfo& frameInfo, Mat4 invViewProj, Vec2 invResolution);
// 		void renderPostProcessing(VyFrameInfo& frameInfo);

// 	private:
// 		void createGBufferPipeline(VkRenderPass lightingRenderPass, VkDescriptorSetLayout gBufferLayout);
// 		void createCompositionPipeline(VkRenderPass lightingRenderPass, VkDescriptorSetLayout compositionSetLayout);
// 		void createPostProcessingPipeline(VkRenderPass postProcessingRenderPass, VkDescriptorSetLayout postProcessingSetLayout);

// 		Unique<VyPipeline> m_GBufferPipeline;
// 		Unique<VyPipeline> m_CompositionPipeline;
// 		Unique<VyPipeline> m_PostProcessingPipeline;
// 	};
// }