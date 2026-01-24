// #pragma once

// #include <VyEngine/GFX/Data/FrameInfo.h>

// #include <VyEngine/VK/Pipeline/Pipeline.h>
// #include <VyEngine/VK/Descriptors/Descriptors.h>

// namespace Vy
// {
// 	class VyShadowRenderSystem {
// 	public:
// 		// Constant depth bias factor (always applied)
// 		static constexpr float kDepthBiasConstant = 0.75f;
// 		// Slope depth bias factor, applied depending on polygon's slope
// 		static constexpr float kDepthBiasSlope = 0.25f;

// 		VyShadowRenderSystem(VkRenderPass renderPass, VkDescriptorSetLayout shadowSetLayout);
// 		~VyShadowRenderSystem();

// 		VyShadowRenderSystem(const VyShadowRenderSystem&) = delete;
// 		VyShadowRenderSystem& operator=(const VyShadowRenderSystem&) = delete;

// 		void render(VyFrameInfo& frameInfo);

// 	private:
// 		void createShadowPipeline(VkRenderPass renderPass, VkDescriptorSetLayout shadowSetLayout);

// 		Unique<VyPipeline> m_ShadowPipeline;
// 	};
// }