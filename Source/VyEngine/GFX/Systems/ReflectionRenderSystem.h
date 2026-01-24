// #pragma once

// #include <VyEngine/GFX/Data/FrameInfo.h>

// #include <VyEngine/VK/Pipeline/Pipeline.h>
// #include <VyEngine/VK/Descriptors/Descriptors.h>

// namespace Vy
// {
// 	class VyReflectionRenderSystem  
//     {
// 	public:

// 		VyReflectionRenderSystem(
//             VkRenderPass          mappingsRenderPass, 
//             VkDescriptorSetLayout mappingsSetLayout, 
//             VkRenderPass          uvReflectionMapRenderPass, 
//             VkDescriptorSetLayout uvReflectionMapSetLayout
//         );
		
//         ~VyReflectionRenderSystem ();

// 		VyReflectionRenderSystem (const VyReflectionRenderSystem &) = delete;
// 		VyReflectionRenderSystem & operator=(const VyReflectionRenderSystem &) = delete;

// 		void renderMappings(VyFrameInfo& frameInfo);
// 		void renderUVReflectionMap(VyFrameInfo& frameInfo);

// 	private:
// 		void createMappingsPipeline(VkRenderPass mappingsRenderPass, VkDescriptorSetLayout mappingsSetLayout);
// 		void createUVReflectionMapPipeline(VkRenderPass uvReflectionMapRenderPass, VkDescriptorSetLayout uvReflectionMapSetLayout);

// 		Unique<VyPipeline> m_MappingsPipeline;
// 		Unique<VyPipeline> m_UVReflectionMapPipeline;
// 	};
// }