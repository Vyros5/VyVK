#include <Vy/Systems/Rendering/MasterRenderSystem.h>

#include <Vy/GFX/Context.h>

namespace Vy
{
	VyMasterRenderSystem::VyMasterRenderSystem(
		VyRenderer&              renderer,
		Shared<VyMaterialSystem> materialSystem,
		Shared<VyEnvironment>    environment
	) : 
		m_Renderer      { renderer                  },
		m_MaterialSystem{ std::move(materialSystem) },
		m_Environment   { std::move(environment)    }
	{
		m_IsRunning = true;

		// Create the descriptor resources and UBO buffers.
        createDescriptorPools();
		createUniformBuffers();
        createDescriptors();

		// Initialize Render Systems.
		createRenderSystems();

		VY_INFO_TAG("VyMasterRenderSystem", "- Initialized");
	}


	VyMasterRenderSystem::~VyMasterRenderSystem() 
	{ 
		if (m_IsRunning) 
		{
			VY_WARN_TAG("VyMasterRenderSystem", "Prematurally shutdown");
		
			shutdown();
		}
	}

	
	void VyMasterRenderSystem::shutdown() 
	{
		m_IsRunning = false;
	}


#pragma region [ Resources ]

	void VyMasterRenderSystem::createDescriptorPools()
	{
		// Global pool is created by VyContext when VyRenderer is created.

        // Create the material pool.
        m_MaterialPool = VyDescriptorPool::Builder{}
            .setMaxSets (1000)
            .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4000)
        .buildUnique();
	}


	void VyMasterRenderSystem::createDescriptors()
	{
        // Global set layout.
        m_GlobalSetLayout = VyDescriptorSetLayout::Builder{}
            .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS) // Global UBO
        .buildUnique();

        // Material set layout.
        m_MaterialSetLayout = VyDescriptorSetLayout::Builder()
            .addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT) // Albedo
            .addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT) // Normal
            .addBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT) // Roughness
            .addBinding(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT) // Metallic
        .buildUnique();

		// ----------------------------------------------------------------------------------------

        // Write the global descriptor sets.
        for (int i = 0; i < m_GlobalSets.size(); i++)
        {
            auto bufferInfo = m_UBOBuffers[i]->descriptorBufferInfo();

            VyDescriptorWriter{ *m_GlobalSetLayout, *VyContext::globalPool() }
                .writeBuffer(0, &bufferInfo)
            .build(m_GlobalSets[i]);
        }

        // Create descriptor sets for the skybox if it exists.
        if (m_Environment->getSkybox())
        {
            auto skybox = m_Environment->getSkybox();

            skybox->createDescriptorSet();
        }
	}


	void VyMasterRenderSystem::createUniformBuffers()
	{
        // Create the global UBO buffers (One per frame). 
        for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) 
        {
            m_UBOBuffers[i] = MakeUnique<VyBuffer>( VyBuffer::uniformBuffer(sizeof(GlobalUBO)), false );

            // Map the buffer's memory so that it can be written to in the update loop.
            m_UBOBuffers[i]->map();
        }
	}

#pragma endregion Resources


#pragma region [ Systems ]

	void VyMasterRenderSystem::createRenderSystems()
	{
		m_PostProcessSystem = MakeUnique<VyPostProcessSystem>(
			m_Renderer.swapchainExtent()
		);

		VY_INFO_TAG("VyMasterRenderSystem", "- VyPostProcessSystem Complete");

		// ----------------------------------------------------------------------------------------

		m_RenderSystem = MakeUnique<VyRenderSystem>(
			// m_Renderer.swapchainRenderPass(),
			m_PostProcessSystem->getHDRRenderPass(),
			TVector{
				m_GlobalSetLayout  ->handle(),
				m_MaterialSetLayout->handle()
			}
		);

		VY_INFO_TAG("VyMasterRenderSystem", "- VyRenderSystem Complete");

		// ----------------------------------------------------------------------------------------

		m_LightSystem = MakeUnique<VyLightSystem>(
			// m_Renderer.swapchainRenderPass(),
			m_PostProcessSystem->getHDRRenderPass(),
			m_GlobalSetLayout  ->handle()
		);

		VY_INFO_TAG("VyMasterRenderSystem", "- VyLightSystem Complete");

		// ----------------------------------------------------------------------------------------

		m_GridSystem = MakeUnique<VyGridSystem>(
			// m_Renderer.swapchainRenderPass(),
			m_PostProcessSystem->getHDRRenderPass(),
			m_GlobalSetLayout  ->handle()
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

#pragma endregion Systems


#pragma region [ Processes ]


	void VyMasterRenderSystem::updateUniformBuffers(VyFrameInfo& frameInfo, GlobalUBO& ubo)
	{
		// // Update material descriptor sets.
		m_MaterialSystem->updateMaterials(frameInfo, *m_MaterialSetLayout, *m_MaterialPool);

		// [ Update UBO Data ]
		{
			ubo.Projection  = frameInfo.Camera.projection();
			ubo.View        = frameInfo.Camera.view();
			ubo.InverseView = frameInfo.Camera.inverseView();
		}

		// Update light values into UBO.
		m_LightSystem->update(frameInfo, ubo);

		// Write Global UBO buffers.
		m_UBOBuffers[ frameInfo.FrameIndex ]->writeToBuffer( &ubo, sizeof(GlobalUBO), 0 );
		m_UBOBuffers[ frameInfo.FrameIndex ]->flush();
	}

	// ---------------------------------------------------------------------------------------------------------------------

	void VyMasterRenderSystem::render(VyFrameInfo& frameInfo) 
	{
		auto cmdBuffer = frameInfo.CommandBuffer;

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
		vkCmdBeginRenderPass(cmdBuffer, &hdrRenderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		{
			// Set viewport and scissor for HDR rendering.
			VKCmd::viewport(cmdBuffer, m_Renderer.swapchainExtent());
			VKCmd::scissor (cmdBuffer, m_Renderer.swapchainExtent());
			
			// [ Render Systems ]
			{
				m_SkyboxSystem->render(frameInfo);
				m_RenderSystem->render(frameInfo);
				m_LightSystem ->render(frameInfo);
				m_GridSystem  ->render(frameInfo);
			}
		}
		vkCmdEndRenderPass(cmdBuffer);

		// [ Apply Post-Processing ]
		const auto& postProcSettings = frameInfo.Scene->getPostProcessingComponent();
		{
			m_PostProcessSystem->renderPostProcess(
				cmdBuffer,
				frameInfo.FrameIndex,
				postProcSettings
			);
		}

		// [ Swapchain Final Composite Pass ]
		{
			m_Renderer.beginSwapchainRenderPass(cmdBuffer);
			{
				m_PostProcessSystem->renderFinalComposite(
					cmdBuffer,
					m_Renderer.swapchainRenderPass().handle(),
					frameInfo.FrameIndex,
					postProcSettings
				);
			}
			m_Renderer.endSwapchainRenderPass(cmdBuffer);
		}

		// [ End of Frame ]
	}

#pragma endregion Processes
}