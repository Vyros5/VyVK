#pragma once

#include <Vy/GFX/Resources/ShadowMap.h>
#include <Vy/GFX/Backend/Descriptors.h>
#include <Vy/GFX/Backend/Pipeline.h>
#include <Vy/GFX/FrameInfo.h>
#include <Vy/Systems/Rendering/IRenderSystem.h>
#include <Vy/GFX/Backend/Resources/Framebuffer.h>
#include <Vy/GFX/Renderer.h>
// Offscreen frame buffer properties
// #define DEFAULT_SHADOWMAP_FILTER VK_FILTER_LINEAR
// #define DEPTH_FORMAT VK_FORMAT_D16_UNORM
// #define SHADOWMAP_DIM 2048

namespace Vy
{
	// class ShadowRenderSystem {
	// public:
	// 	static constexpr int SHADOW_WIDTH = 1024;
	// 	static constexpr int SHADOW_HEIGHT = 1024;
	// 	static constexpr VkFilter SHAODW_FILTER = VK_FILTER_LINEAR;

	// 	struct ShadowUbo {
	// 		Mat4 ProjectionMatrix{1.0f};
	// 		Mat4 ViewMatrices[6];
	// 	};

	// 	ShadowRenderSystem(Renderer& renderer, TVector<Unique<VyDescriptorSetLayout>>& globalDescriptorSetLayout);
	// 	~ShadowRenderSystem() = default;

	// 	ShadowRenderSystem(const ShadowRenderSystem&) = delete;
	// 	ShadowRenderSystem& operator=(const ShadowRenderSystem&) = delete;

	// 	ShadowRenderSystem(ShadowRenderSystem&&) = delete;            // Move Constructor
	// 	ShadowRenderSystem& operator=(ShadowRenderSystem&&) = delete; // Move Assignment Operator

	// 	void render(VyFrameInfo& frameInfo);
	// 	void updateUBOs(VyFrameInfo& frameInfo);
	// 	void createResources();
	// 	VyRenderInfo prepareRenderInfo();
	// 	void onResize();

	// 	Shared<Framebuffer> getResources() {
	// 		return m_Resources;
	// 	}

	// 	TVector<VkDescriptorSet>& getDescriptorSets() {
	// 		return m_UBODescriptorSets;
	// 	}

	// private:
	// 	void createDescriptorSetLayout();
	// 	void createDescriptorSet();
	// 	void createPipelines(Unique<VyDescriptorSetLayout>& globalDescriptorSetLayout);

	// 	Renderer& m_Renderer;
	// 	Shared<Framebuffer> m_Resources;
	// 	Unique<VyPipeline> m_ShadowMappingPipeline;
	// 	Unique<VyPipeline> m_OmniShadowMappingPipeline;

	// 	Unique<VyDescriptorSetLayout> m_DescriptorSetLayout{};
	// 	TVector<VkDescriptorSet> m_UBODescriptorSets;

	// 	TVector<Unique<VyBuffer>> m_UBOBuffers;
	// };


    // class VyShadowMapSystem 
    // {
    // public:
    //     explicit VyShadowMapSystem(
	// 		VkRenderPass          shadowRenderPass, 
	// 		VkDescriptorSetLayout globalSetLayout,
	// 		VkExtent2D            shadowMapExtent
	// 	);

    //     ~VyShadowMapSystem();

    //     VyShadowMapSystem(const VyShadowMapSystem&)            = delete;
    //     VyShadowMapSystem& operator=(const VyShadowMapSystem&) = delete;

    //     void render(const VyFrameInfo& frameInfo, Mat4& lightViewProj);
        
	// 	static Mat4 getLightViewProjection(const Vec3& dirLightPos, const Vec3& cameraPosition, float sceneRadius);

    //     // VkDescriptorImageInfo descriptorImageInfo()
    //     // {
    //     //     return m_ShadowMap->descriptorImageInfo();
    //     // }

    // private:
    //     void createPipeline(VkRenderPass renderPass, VkDescriptorSetLayout& globalSetLayout);
        
	// 	// VkImageView createShadowMapImageView(int shadowMapExtent);
        
	// 	// void transitionImageLayout(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, VkImageAspectFlags aspectMask);

    //     Unique<VyPipeline>  m_Pipeline;
    //     // Unique<VyShadowMap> m_ShadowMap;


    //     // Shadow map specific resources
    //     VkRenderPass m_ShadowRenderPass; // Render pass for shadow map rendering
    //     VkExtent2D   m_ShadowMapExtent;  // Resolution of the shadow map
    //     // Mat4 m_LightSpaceMatrix{};
    // };
}