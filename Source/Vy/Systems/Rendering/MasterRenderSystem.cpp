#include <Vy/Systems/Rendering/MasterRenderSystem.h>

#include <Vy/GFX/Context.h>

namespace Vy
{
	VyMasterRenderSystem::VyMasterRenderSystem(
		VyRenderer&                   renderer,
		Shared<VyDescriptorSetLayout> globalSetLayout,
		Shared<VyDescriptorSetLayout> materialSetLayout,
		Shared<VyMaterialSystem>      materialSystem,
		Shared<VyDescriptorPool>      materialPool,
		Shared<VyEnvironment>         environment
	) : 
		m_Renderer         { renderer                     },
		m_GlobalSetLayout  { std::move(globalSetLayout)   },
		m_MaterialSetLayout{ std::move(materialSetLayout) },
		m_MaterialSystem   { std::move(materialSystem)    },
		m_MaterialPool     { std::move(materialPool)      },
		m_Environment      { std::move(environment)       } //,
		// m_LightManager     { static_cast<U32>(MAX_FRAMES_IN_FLIGHT), 128 }
	{
		m_IsRunning = true;

		// m_LightManager = MakeShared<VyLightManager>(MAX_FRAMES_IN_FLIGHT, 128);

		createRenderSystems();
	}


	VyMasterRenderSystem::~VyMasterRenderSystem() 
	{ 
		if (m_IsRunning) 
		{
			VY_WARN_TAG("VyMasterRenderSystem", "Prematurally shutdown");
		
			deinitialize();
		}
	}

	
	void VyMasterRenderSystem::deinitialize() 
	{
		m_IsRunning = false;
	}


	void VyMasterRenderSystem::createRenderSystems()
	{
		m_PostProcessSystem = MakeUnique<VyPostProcessSystem>(
			m_Renderer.swapchainExtent()
		);

		VY_INFO_TAG("VyMasterRenderSystem", "- VyPostProcessSystem Complete");

		// ----------------------------------------------------------------------------------------

		// m_ShadowSystem = MakeUnique<VyShadowSystem>(
		// 	// m_LightManager
		// );

		// VY_INFO_TAG("VyMasterRenderSystem", "- VyShadowSystem Complete");

		// ----------------------------------------------------------------------------------------

		m_RenderSystem = MakeUnique<VyRenderSystem>(
			// m_Renderer.swapchainRenderPass(),
			m_PostProcessSystem->getHDRRenderPass(),
			m_GlobalSetLayout  ->handle(),
			m_MaterialSetLayout->handle()
		);

		VY_INFO_TAG("VyMasterRenderSystem", "- VyRenderSystem Complete");

		// ----------------------------------------------------------------------------------------

		m_LightSystem = MakeUnique<VyLightSystem>(
			// m_Renderer.swapchainRenderPass(),
			m_PostProcessSystem->getHDRRenderPass(),
			m_GlobalSetLayout->handle()
		);

		VY_INFO_TAG("VyMasterRenderSystem", "- VyLightSystem Complete");

		// ----------------------------------------------------------------------------------------

		m_GridSystem = MakeUnique<VyGridSystem>(
			// m_Renderer.swapchainRenderPass(),
			m_PostProcessSystem->getHDRRenderPass(),
			m_GlobalSetLayout->handle()
		);

		VY_INFO_TAG("VyMasterRenderSystem", "- VyGridSystem Complete");

		// ----------------------------------------------------------------------------------------

		m_SkyboxSystem = MakeUnique<VySkyboxSystem>(
			// m_Renderer.swapchainRenderPass(),
			m_PostProcessSystem->getHDRRenderPass(),
			m_GlobalSetLayout->handle(),
			m_Environment
		);

		VY_INFO_TAG("VyMasterRenderSystem", "- VySkyboxSystem Complete");

		// ----------------------------------------------------------------------------------------
	}


	void VyMasterRenderSystem::update(VyFrameInfo& frameInfo, GlobalUBO& ubo)
	{
		// // Update materials descriptor sets.
		m_MaterialSystem->updateMaterials(frameInfo, *m_MaterialSetLayout, *m_MaterialPool);

		// [ Update UBO Data ]
		{
			ubo.Projection  = frameInfo.Camera.projection();
			ubo.View        = frameInfo.Camera.view();
			ubo.InverseView = frameInfo.Camera.inverseView();

			// Calculate Frustum Planes for Culling (Normalized)
			// Mat4 vp   = ubo.Projection * ubo.View;
			// Mat4 vpT  = glm::transpose(vp);
			// Vec4 row0 = vpT[0];
			// Vec4 row1 = vpT[1];
			// Vec4 row2 = vpT[2];
			// Vec4 row3 = vpT[3];

			// ubo.FrustumPlanes[0] = row3 + row0; // Left
			// ubo.FrustumPlanes[1] = row3 - row0; // Right
			// ubo.FrustumPlanes[2] = row3 + row1; // Bottom
			// ubo.FrustumPlanes[3] = row3 - row1; // Top
			// ubo.FrustumPlanes[4] = row2;        // Near
			// ubo.FrustumPlanes[5] = row3 - row2; // Far

			// for (int i = 0; i < 6; i++)
			// {
			// 	float length = glm::length(Vec3(ubo.FrustumPlanes[i]));

			// 	ubo.FrustumPlanes[i] /= length;
			// }
		}

		// Update light values into UBO.
		m_LightSystem->update(frameInfo, ubo);
	}


	void VyMasterRenderSystem::render(VyFrameInfo& frameInfo) 
	{
		TArray<VkClearValue, 2> clearValues{};
		{
			clearValues[0].color        = {{ 0.01f, 0.01f, 0.01f, 1.0f }};
			clearValues[1].depthStencil = { 1.0f, 0 };
		}

		// Render scene to HDR buffer.
		VkRenderPassBeginInfo hdrRenderPassInfo{ VKInit::renderPassBeginInfo() };
		{
			hdrRenderPassInfo.renderPass        = m_PostProcessSystem->getHDRRenderPass();
			hdrRenderPassInfo.framebuffer       = m_PostProcessSystem->getHDRFramebuffer(frameInfo.FrameIndex);

			hdrRenderPassInfo.renderArea.offset = { 0, 0 };
			hdrRenderPassInfo.renderArea.extent = m_Renderer.swapchainExtent();
			
			hdrRenderPassInfo.clearValueCount   = static_cast<U32>(clearValues.size());
			hdrRenderPassInfo.pClearValues      = clearValues.data();
		}

		// [ HDR Render Pass ]
		vkCmdBeginRenderPass(frameInfo.CommandBuffer, &hdrRenderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		{
			// Set viewport and scissor for HDR rendering.
			VKCmd::viewport(frameInfo.CommandBuffer, m_Renderer.swapchainExtent());
			VKCmd::scissor (frameInfo.CommandBuffer, m_Renderer.swapchainExtent());
			
			// [ Render Systems ]
			{
				// m_SkyboxSystem->render(frameInfo);
				m_RenderSystem->render(frameInfo);
				m_LightSystem ->render(frameInfo);
				m_GridSystem  ->render(frameInfo);
			}
		}
		vkCmdEndRenderPass(frameInfo.CommandBuffer);
	}


	void VyMasterRenderSystem::renderPostProcess(VyFrameInfo& frameInfo)
	{
		m_PostProcessSystem->renderPostProcess(
			frameInfo.CommandBuffer,
			frameInfo.FrameIndex,
			frameInfo.Scene->getPostProcessingComponent()
		);
	}


	void VyMasterRenderSystem::renderFinalComposite(VyFrameInfo& frameInfo)
	{
		m_PostProcessSystem->renderFinalComposite(
			frameInfo.CommandBuffer,
			m_Renderer.swapchainRenderPass().handle(),
			frameInfo.FrameIndex,
			frameInfo.Scene->getPostProcessingComponent()
		);
	}

}