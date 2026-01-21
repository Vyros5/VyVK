#include <VyEngine/GFX/Systems/DeferredSystem.h>

#include <VyEngine/GFX/Resources/Mesh/Vertex.h>

#include <VyEngine/VK/Context.h>
#include <iostream>
#include <VyEngine/Scene/ECS/Components.h>

namespace Vy
{
	DeferredRenderSystem::DeferredRenderSystem(
        VkExtent2D extent, 
        VkFormat   swapFormat, 
        Shared<HDRImage>& hdrImage, 
        TVector<Light>& lights
    ) : 
        m_CPULights{lights}, 
        m_HDRImage(hdrImage)
	{
		assert(VyContext::device().limits().maxPushConstantsSize > sizeof(GeometryPassPush) && "Max supported push constant data is smaller than 256 bytes");
		Initialize(extent, swapFormat);
	}

	DeferredRenderSystem::~DeferredRenderSystem()
	{
		vkDestroyDescriptorPool(VyContext::device(), m_LightingPassDescriptorPool, nullptr);
		vkDestroyDescriptorPool(VyContext::device(), m_BlitDescriptorPool, nullptr);
		// vkDestroyPipelineLayout(VyContext::device(), m_LightPipelineLayout, nullptr);
		// vkDestroyPipelineLayout(VyContext::device(), m_BlitPipelineLayout, nullptr);
		// vkDestroyPipelineLayout(VyContext::device(), m_GeometryPipelineLayout, nullptr);
		// vkDestroyPipelineLayout(VyContext::device(), m_DepthPrepassPipelineLayout, nullptr);
		vkDestroyDescriptorSetLayout(VyContext::device(), m_LightingPassDescriptorSetLayout, nullptr);
		vkDestroyDescriptorSetLayout(VyContext::device(), m_PointLightsDescriptorSetLayout, nullptr);
		vkDestroyDescriptorSetLayout(VyContext::device(), m_BlitDescriptorSetLayout, nullptr);
		
        VyTexture::cleanupBindless();
	}

	void DeferredRenderSystem::Initialize(VkExtent2D extent, VkFormat swapFormat)
	{
		m_GBuffer.create(extent.width, extent.height);
		m_LightingPassBuffer.create(extent.width, extent.height); 
		// CreateDepthPrepassPipelineLayout();
		CreateDepthPrepassPipeline();
		// CreateGeometryPipelineLayout();
		CreateGeometryPipeline();
		CreateLightingPipelineLayout();
		CreateLightingPipeline();
		CreateLightsBuffer(m_CPULights.size());
		CreateLightingDescriptorSet();

		// CreateBlitPipelineLayout();
		CreateBlitPipeline(swapFormat);
		CreateBlitDescriptorSet();
	}



#pragma region DEPTH_PREPASS_PIPELINE


	void DeferredRenderSystem::CreateDepthPrepassPipeline()
	{
		auto bindDescs = VyVertex::bindingDescriptions();
		auto attrDescs = VyVertex::attributeDescriptions();

        auto builder = VyPipeline::GraphicsBuilder{};
        {
            builder.setName( "depth_prepass" );
            
            builder.addDescriptorSetLayout( VyTexture::s_BindlessSetLayout );
            
            builder.addPushConstantRange(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(DepthPush));
            
            builder.addShaderStage(VK_SHADER_STAGE_VERTEX_BIT,   "DepthPrepass.vert.spv");
            builder.addShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT, "DepthPrepass.frag.spv");
            
            // No color attachments.
            builder.clearColorAttachments();
            builder.setDepthAttachment( GBuffer::DEPTH_FORMAT );

            builder.setVertexBindingDescriptions  ( { bindDescs[0] } );               // Position
            builder.setVertexAttributeDescriptions( { attrDescs[0], attrDescs[3] } ); // Position, Normal
            // builder.setFrontFace(VK_FRONT_FACE_COUNTER_CLOCKWISE);
            // builder.setRenderPass( renderPass );
        }

		m_DepthPrepassPipeline = builder.buildPtr();
	}

	void DeferredRenderSystem::RenderDepthPrepass(VyFrameInfo& frameInfo)
	{
		m_DepthPrepassPipeline->bind( frameInfo.CommandBuffer );
		
		auto layout = m_DepthPrepassPipeline->layout();

        VyTexture::bind(frameInfo.CommandBuffer, layout);

		auto projectionViewMatrix = frameInfo.Camera.projection() * frameInfo.Camera.view();

        auto view = frameInfo.Scene->registry().view<ModelComponent, TransformComponent>();

		for (auto&& [ entity, modelComp, transform ] : view.each())
		{
			for (auto& sm : modelComp.Model->getData().Submeshes)
			{
				auto& mat = modelComp.Model->getData().Materials[ sm.MaterialIndex ];
				
				auto modelMatrix = transform.modelMatrix();
                
                DepthPush push{};
                {
                    push.MVP         = projectionViewMatrix * modelMatrix;
                    push.AlbedoIndex = mat.AlbedoIndex;
                }

				m_DepthPrepassPipeline->pushConstants(frameInfo.CommandBuffer,
					VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 
					&push, 
					sizeof(DepthPush)
				);

				modelComp.Model->bind(frameInfo.CommandBuffer);
				modelComp.Model->drawIndexed(frameInfo.CommandBuffer, sm.IndexCount, sm.FirstIndex);
			}
		}
	}


#pragma endregion

#pragma region GEOMETRY_PIPELINE

	void DeferredRenderSystem::CreateGeometryPipeline()
	{
        auto builder = VyPipeline::GraphicsBuilder{};
        {
            builder.setName( "geometry" );
            
            builder.addDescriptorSetLayout( VyTexture::s_BindlessSetLayout );
            
            builder.addPushConstantRange(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(GeometryPassPush));
            
            builder.addShaderStage(VK_SHADER_STAGE_VERTEX_BIT,   "GeometryPass.vert.spv");
            builder.addShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT, "GeometryPass.frag.spv");
            
            // No color attachments.
            builder.addColorAttachment( GBuffer::POS_FORMAT );
            builder.addColorAttachment( GBuffer::NORM_FORMAT );
            builder.addColorAttachment( GBuffer::ALBEDO_FORMAT );
            builder.addColorAttachment( GBuffer::METALROUGH_FORMAT );
            builder.addColorAttachment( GBuffer::OCCLUSION_FORMAT );
            builder.setDepthAttachment( GBuffer::DEPTH_FORMAT );

            builder.setVertexBindingDescriptions  ( VyVertex::bindingDescriptions()   );
            builder.setVertexAttributeDescriptions( VyVertex::attributeDescriptions() );

            builder.setDepthTest(true, false, VK_COMPARE_OP_LESS_OR_EQUAL);
            // builder.setRenderPass( renderPass );
        }

		m_GeometryPipeline = builder.buildPtr();
	}


	void DeferredRenderSystem::RenderGeometry(VyFrameInfo& frameInfo)
	{

		m_GeometryPipeline->bind( frameInfo.CommandBuffer );

		auto layout = m_GeometryPipeline->layout();

        VyTexture::bind(frameInfo.CommandBuffer, layout);

		auto projectionViewMatrix = frameInfo.Camera.projection() * frameInfo.Camera.view();

        auto view = frameInfo.Scene->registry().view<ModelComponent, TransformComponent>();

		for (auto&& [ entity, modelComp, transform ] : view.each())
		{
            if (!modelComp.Model) continue;

			for (auto& sm : modelComp.Model->getData().Submeshes)
			{
				auto& mat = modelComp.Model->getData().Materials[ sm.MaterialIndex ];
				
                auto modelMatrix = transform.modelMatrix();
				
                GeometryPassPush push{};
				{
                    push.Transform       = projectionViewMatrix * modelMatrix;
                    push.ModelMatrix     = modelMatrix;
                    push.AlbedoIndex     = mat.AlbedoIndex;
                    push.NormalIndex     = mat.NormalIndex;
                    push.MetalRoughIndex = mat.MetallicRoughIndex;
                    push.OcclusionIndex  = mat.OcclusionIndex;
                }

				m_GeometryPipeline->pushConstants(frameInfo.CommandBuffer,
					VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 
					&push, 
					sizeof(GeometryPassPush)
				);

				modelComp.Model->bind(frameInfo.CommandBuffer);
				modelComp.Model->drawIndexed(frameInfo.CommandBuffer, sm.IndexCount, sm.FirstIndex);
			}
		}
	}

	// void DeferredRenderSystem::UpdateGeometry(std::vector<GameObject>& gameObjects, float deltaTime)
	// {
	// 	//update gameobjects here
	// }


	void DeferredRenderSystem::RecreateGBuffer(VkExtent2D extent, VkFormat swapFormat)
	{
		vkDeviceWaitIdle(VyContext::device());

		m_GBuffer.cleanup();
		m_GBuffer.create(extent.width, extent.height);

		vkDestroyDescriptorPool(VyContext::device(), m_LightingPassDescriptorPool, nullptr);
		CreateLightingDescriptorSet();
	}
#pragma endregion



#pragma region LIGHTING_PIPELINE
	void DeferredRenderSystem::CreateLightingPipelineLayout()
	{
		// g-buffer array (binding 0) and separate depth sampler (binding 5)
		VkDescriptorSetLayoutBinding descBinding{};
        {
            descBinding.binding         = 0;
            descBinding.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descBinding.descriptorCount = 5;
            descBinding.stageFlags      = VK_SHADER_STAGE_FRAGMENT_BIT; 
        }

		VkDescriptorSetLayoutBinding depthBinding{};
        {
            depthBinding.binding         = 5;
            depthBinding.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            depthBinding.descriptorCount = 1;
            depthBinding.stageFlags      = VK_SHADER_STAGE_FRAGMENT_BIT;
        }

		VkDescriptorSetLayoutBinding HDRBinding{};
        {
            HDRBinding.binding          = 6;
            HDRBinding.descriptorType   = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            HDRBinding.descriptorCount  = 1;
            HDRBinding.stageFlags       = VK_SHADER_STAGE_FRAGMENT_BIT;
        }

		VkDescriptorSetLayoutBinding irrBinding{};
        { 
            irrBinding.binding          = 7;
            irrBinding.descriptorType   = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            irrBinding.descriptorCount  = 1;
            irrBinding.stageFlags       = VK_SHADER_STAGE_FRAGMENT_BIT;
        }

		std::array<VkDescriptorSetLayoutBinding, 4> bindings{ 
            descBinding, 
            depthBinding, 
            HDRBinding, 
            irrBinding 
        };

		VkDescriptorSetLayoutCreateInfo descSetInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
		descSetInfo.bindingCount = static_cast<U32>(bindings.size());
		descSetInfo.pBindings = bindings.data();
		vkCreateDescriptorSetLayout(VyContext::device(), &descSetInfo, nullptr, &m_LightingPassDescriptorSetLayout);

		// 2) Lights set (set 1):
		VkDescriptorSetLayoutBinding bLight{};
		bLight.binding = 0;
		bLight.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		bLight.descriptorCount = 1;
		bLight.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		VkDescriptorSetLayoutCreateInfo lightInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
		lightInfo.bindingCount = 1;
		lightInfo.pBindings = &bLight;
		vkCreateDescriptorSetLayout(VyContext::device(), &lightInfo, nullptr, &m_PointLightsDescriptorSetLayout);


		VkDescriptorSetLayout setLayouts[] = {
			m_LightingPassDescriptorSetLayout,  // set 0
			m_PointLightsDescriptorSetLayout    // set 1
		};
		// push const for camera pos and screen texel
		VkPushConstantRange pc{};
		pc.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		pc.offset = 0;
		pc.size = sizeof(LightingPassPush);

		// VkPipelineLayoutCreateInfo plInfo{ VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
		// plInfo.setLayoutCount = 2;
		// plInfo.pSetLayouts = setLayouts;
		// plInfo.pushConstantRangeCount = 1;
		// plInfo.pPushConstantRanges = &pc;
		// vkCreatePipelineLayout(VyContext::device(), &plInfo, nullptr, m_LightPipeline->layout());
	}


	void DeferredRenderSystem::CreateLightingDescriptorSet()
	{
		VkDescriptorPoolSize poolSizes[2]{};
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[0].descriptorCount = 6;
		poolSizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		poolSizes[1].descriptorCount = 1;

		VkDescriptorPoolCreateInfo poolInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
		poolInfo.poolSizeCount = 2;
		poolInfo.pPoolSizes = poolSizes;
		poolInfo.maxSets = 2;
		if (vkCreateDescriptorPool(VyContext::device(), &poolInfo, nullptr, &m_LightingPassDescriptorPool) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create lighting descriptor pool");
		}

		VkDescriptorSetAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
		allocInfo.descriptorPool = m_LightingPassDescriptorPool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &m_LightingPassDescriptorSetLayout;
		if (vkAllocateDescriptorSets(VyContext::device(), &allocInfo, &m_LightDescriptorSet) != VK_SUCCESS) {
			throw std::runtime_error("Failed to allocate lighting descriptor set");
		}

		std::array<VkDescriptorImageInfo, 5> imageInfos = {
            VkDescriptorImageInfo{
                m_GBuffer.positionSampler(),
                m_GBuffer.positionView(),
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
            },
            VkDescriptorImageInfo{
                m_GBuffer.normalSampler(),
                m_GBuffer.normalView(),
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
            },
            VkDescriptorImageInfo{
                m_GBuffer.albedoSpecSampler(),
                m_GBuffer.albedoSpecView(),
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
            },
            VkDescriptorImageInfo{
                m_GBuffer.metalRoughSampler(),
                m_GBuffer.metalRoughView(),
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
            },
			VkDescriptorImageInfo{
				m_GBuffer.occlusionSampler(),
				m_GBuffer.occlusionView(),
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
			}
		};

		VkDescriptorImageInfo depthInfo{
            m_GBuffer.depthSampler(),
            m_GBuffer.depthView(),
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
		};

		VkWriteDescriptorSet writeGBuffer{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
		writeGBuffer.dstSet = m_LightDescriptorSet;
		writeGBuffer.dstBinding = 0;
		writeGBuffer.dstArrayElement = 0; 
		writeGBuffer.descriptorCount = static_cast<U32>(imageInfos.size());
		writeGBuffer.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		writeGBuffer.pImageInfo = imageInfos.data();

		VkDescriptorImageInfo HDRInfo{};
		HDRInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		HDRInfo.imageView = m_HDRImage->cubeMapView();
		HDRInfo.sampler = m_HDRImage->cubeMapSampler();

		VkWriteDescriptorSet writeHDR{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
		writeHDR.dstSet = m_LightDescriptorSet;
		writeHDR.dstBinding = 6;
		writeHDR.dstArrayElement = 0;
		writeHDR.descriptorCount = 1;
		writeHDR.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		writeHDR.pImageInfo = &HDRInfo;

		VkDescriptorImageInfo irrInfo{};
		irrInfo.sampler = m_HDRImage->irradianceSampler();
		irrInfo.imageView = m_HDRImage->irradianceView();
		irrInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VkWriteDescriptorSet writeIrr{
            VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            nullptr,                
            m_LightDescriptorSet,   
            7,                     
            0,                      
            1,                      
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            &irrInfo,               
            nullptr, nullptr
		};

		VkWriteDescriptorSet writeDepth{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
		writeDepth.dstSet = m_LightDescriptorSet;
		writeDepth.dstBinding = 5;
		writeDepth.dstArrayElement = 0;
		writeDepth.descriptorCount = 1;
		writeDepth.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		writeDepth.pImageInfo = &depthInfo;

		std::array<VkWriteDescriptorSet, 4> writes{ 
            writeGBuffer, 
            writeDepth, 
            writeHDR, 
            writeIrr 
        };

		// 3) Allocate & write the Lights set (set 1):
		VkDescriptorSetAllocateInfo alloc1{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
		alloc1.descriptorPool = m_LightingPassDescriptorPool;
		alloc1.descriptorSetCount = 1;
		alloc1.pSetLayouts = &m_PointLightsDescriptorSetLayout;
		vkAllocateDescriptorSets(VyContext::device(), &alloc1, &m_PointLightsDescriptorSet);

		VkDescriptorBufferInfo bufInfo{};
		bufInfo.buffer = m_LightsBuffer->handle();
		bufInfo.offset = 0;
		bufInfo.range = sizeof(Light) * m_MaxLights;

		VkWriteDescriptorSet writeBuf{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
		writeBuf.dstSet = m_PointLightsDescriptorSet;
		writeBuf.dstBinding = 0;
		writeBuf.descriptorCount = 1;
		writeBuf.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		writeBuf.pBufferInfo = &bufInfo;

		vkUpdateDescriptorSets(VyContext::device(), 1, &writeBuf, 0, nullptr);
		vkUpdateDescriptorSets(VyContext::device(), static_cast<U32>(writes.size()), writes.data(), 0, nullptr); 
	}


	void DeferredRenderSystem::CreateLightingPipeline()
	{
        auto builder = VyPipeline::GraphicsBuilder{};
        {
            builder.setName( "lighting" );
            
            builder.addDescriptorSetLayout( VyTexture::s_BindlessSetLayout );
            
            builder.addPushConstantRange(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(GeometryPassPush));
            
            builder.addShaderStage(VK_SHADER_STAGE_VERTEX_BIT,   "Triangle.vert.spv");
            builder.addShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT, "LightingPass.frag.spv");
            
            // No color attachments.
            builder.addColorAttachment( LightBuffer::HDR_FORMAT );
            builder.setDepthAttachment( VK_FORMAT_UNDEFINED     );

            // builder.setVertexBindingDescriptions  ( VyVertex::bindingDescriptions()   );
            // builder.setVertexAttributeDescriptions( VyVertex::attributeDescriptions() );

            // builder.setDepthTest(true, false, VK_COMPARE_OP_LESS_OR_EQUAL);
            // builder.setRenderPass( renderPass );
        }

		m_LightPipeline = builder.buildPtr();
	}


	void DeferredRenderSystem::RenderLighting(VyFrameInfo& frameInfo, VkExtent2D extent)
	{
		// copy into ssbo
		U32 count = std::min((size_t)m_CPULights.size(), m_MaxLights);

        m_LightsBuffer->write(m_CPULights.data(), sizeof(Light) * count, 0);

		// void* ptr = nullptr;
		// vkMapMemory(VyContext::device(), m_LightsBufferMemory, 0, sizeof(Light) * count, 0, &ptr);
		// memcpy(ptr, m_CPULights.data(), sizeof(Light) * count);
		// vkUnmapMemory(VyContext::device(), m_LightsBufferMemory); 

		LightingPassPush pushConstantData;
        {
            pushConstantData.Resolution = Vec2(
                static_cast<float>(extent.width),
                static_cast<float>(extent.height)
            );
            
            pushConstantData.CameraPos  = frameInfo.Camera.position();
            pushConstantData.LightCount = count;
            pushConstantData.View       = frameInfo.Camera.view();
            pushConstantData.Projection = frameInfo.Camera.projection(); 
        }

		VkDescriptorSet sets[] = { 
            m_LightDescriptorSet, 
            m_PointLightsDescriptorSet 
        };

		
		vkCmdBindDescriptorSets(frameInfo.CommandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			m_LightPipeline->layout(), 
            0, 2, 
            sets, 
            0, nullptr
        );

		vkCmdPushConstants(frameInfo.CommandBuffer,
			m_LightPipeline->layout(), 
			VK_SHADER_STAGE_FRAGMENT_BIT,
			0, sizeof(pushConstantData),
			&pushConstantData
        );

		m_LightPipeline->bind(frameInfo.CommandBuffer);

		// draw triangle trick
		vkCmdDraw(frameInfo.CommandBuffer, 3, 1, 0, 0); 
	}


	void DeferredRenderSystem::CreateLightsBuffer(size_t maxLights)
	{
		m_MaxLights = maxLights;
		VkDeviceSize bufferSize = sizeof(Light) * m_MaxLights;

        m_LightsBuffer = MakeUnique<VyBuffer>( VyBuffer::storageBuffer("lights", bufferSize ) );
	}

#pragma endregion

#pragma region BLITTING 

	// void DeferredRenderSystem::CreateBlitPipelineLayout()
	// {
		// VkDescriptorSetLayoutBinding binding{};
		// binding.binding = 0;
		// binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		// binding.descriptorCount = 1;
		// binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		// VkDescriptorSetLayoutCreateInfo descSetInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
		// descSetInfo.bindingCount = 1;
		// descSetInfo.pBindings = &binding;
		// if (vkCreateDescriptorSetLayout(VyContext::device(), &descSetInfo, nullptr,
		// 	&m_BlitDescriptorSetLayout) != VK_SUCCESS) {
		// 	throw std::runtime_error("Failed to create blit descriptor set layout");
		// }
		
		// VkPipelineLayoutCreateInfo plInfo{ VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
		// plInfo.setLayoutCount = 1;
		// plInfo.pSetLayouts = &m_BlitDescriptorSetLayout;
		// if (vkCreatePipelineLayout(VyContext::device(), &plInfo, nullptr,
		// 	&m_BlitPipelineLayout) != VK_SUCCESS) {
		// 	throw std::runtime_error("Failed to create blit pipeline layout");
		// }

	// }

	void DeferredRenderSystem::CreateBlitPipeline(VkFormat swapFormat)
	{
		VkDescriptorSetLayoutBinding binding{};
        {
            binding.binding         = 0;
            binding.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            binding.descriptorCount = 1;
            binding.stageFlags      = VK_SHADER_STAGE_FRAGMENT_BIT;
        }

		auto descSetInfo{ VKInit::descriptorSetLayoutCreateInfo( &binding, 1 ) };

        if (vkCreateDescriptorSetLayout(VyContext::device(), &descSetInfo, nullptr, &m_BlitDescriptorSetLayout) != VK_SUCCESS) 
        {
			throw std::runtime_error("Failed to create blit descriptor set layout");
		}

        // ----------------------------------------------------------------------------------------

        auto builder = VyPipeline::GraphicsBuilder{};
        {
            builder.setName( "blitting" );
            
            builder.addDescriptorSetLayout( m_BlitDescriptorSetLayout );
            
            // builder.addPushConstantRange(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(GeometryPassPush));
            
            builder.addShaderStage(VK_SHADER_STAGE_VERTEX_BIT,   "Triangle.vert.spv");
            builder.addShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT, "Blit.frag.spv");
            
            builder.addColorAttachment( swapFormat );
            builder.setDepthAttachment( VK_FORMAT_UNDEFINED );

            // no vertex buffers: triangle uses gl_VertexIndex
            builder.clearVertexDescriptions();
        }

		m_BlitPipeline = builder.buildPtr();
	}



	void DeferredRenderSystem::CreateBlitDescriptorSet()
	{
		VkDescriptorPoolSize poolSize{};
		poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSize.descriptorCount = 1;

		VkDescriptorPoolCreateInfo poolInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
		poolInfo.poolSizeCount = 1;
		poolInfo.pPoolSizes = &poolSize;
		poolInfo.maxSets = 1;
		if (vkCreateDescriptorPool(VyContext::device(), &poolInfo, nullptr,
			&m_BlitDescriptorPool) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create blit descriptor pool");
		}

		VkDescriptorSetAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
		allocInfo.descriptorPool = m_BlitDescriptorPool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &m_BlitDescriptorSetLayout;
		if (vkAllocateDescriptorSets(VyContext::device(), &allocInfo,
			&m_BlitDescriptorSet) != VK_SUCCESS) {
			throw std::runtime_error("Failed to allocate blit descriptor set");
		}

		VkDescriptorImageInfo imageInfo{};
		imageInfo.sampler = m_LightingPassBuffer.sampler();
		imageInfo.imageView = m_LightingPassBuffer.imageView();
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		 
		VkWriteDescriptorSet write{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
		write.dstSet = m_BlitDescriptorSet;
		write.dstBinding = 0;
		write.dstArrayElement = 0;
		write.descriptorCount = 1;
		write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		write.pImageInfo = &imageInfo;

		vkUpdateDescriptorSets(VyContext::device(), 1, &write, 0, nullptr);
	}


	void DeferredRenderSystem::CycleDebugOutput()
	{
		int mode = static_cast<int>(m_DebugOutput);
		mode = (mode + 1) % static_cast<int>(DebugOutput::COUNT);
		m_DebugOutput = static_cast<DebugOutput>(mode);

		static const char* names[] = {
            "Lighting",
            "Position",
            "Normal",
            "Albedo",
            "MetalRough",
            "Occlusion",
            "Depth"
		};

		std::cout << "Debug output: " << names[mode] << std::endl;
	}


	void DeferredRenderSystem::RenderBlit(VkCommandBuffer cmdBuffer)
	{
		VkDescriptorImageInfo imageInfo{};

		switch (m_DebugOutput) {
		case DebugOutput::Lighting:
			imageInfo.sampler     = m_LightingPassBuffer.sampler();
			imageInfo.imageView   = m_LightingPassBuffer.imageView();
			imageInfo.imageLayout = m_LightingPassBuffer.Layout;
			break;
		case DebugOutput::Position:
			imageInfo.sampler     = m_GBuffer.positionSampler();
			imageInfo.imageView   = m_GBuffer.positionView();
			imageInfo.imageLayout = m_GBuffer.PositionLayout;
			break;
		case DebugOutput::Normal:
			imageInfo.sampler     = m_GBuffer.normalSampler();
			imageInfo.imageView   = m_GBuffer.normalView();
			imageInfo.imageLayout = m_GBuffer.NormalLayout;
			break;
		case DebugOutput::Albedo:
			imageInfo.sampler     = m_GBuffer.albedoSpecSampler();
			imageInfo.imageView   = m_GBuffer.albedoSpecView();
			imageInfo.imageLayout = m_GBuffer.AlbedoLayout;
			break;
		case DebugOutput::MetalRough:
			imageInfo.sampler     = m_GBuffer.metalRoughSampler();
			imageInfo.imageView   = m_GBuffer.metalRoughView();
			imageInfo.imageLayout = m_GBuffer.MetalRoughLayout;
			break;
		case DebugOutput::Occlusion:
			imageInfo.sampler     = m_GBuffer.occlusionSampler();
			imageInfo.imageView   = m_GBuffer.occlusionView();
			imageInfo.imageLayout = m_GBuffer.OcclusionLayout;
			break;
		case DebugOutput::Depth:
			imageInfo.sampler     = m_GBuffer.depthSampler();
			imageInfo.imageView   = m_GBuffer.depthView();
			imageInfo.imageLayout = m_GBuffer.DepthLayout;
			break;
		default:
			break;
		}

		VkWriteDescriptorSet write{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
		write.dstSet = m_BlitDescriptorSet;
		write.dstBinding = 0;
		write.descriptorCount = 1;
		write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		write.pImageInfo = &imageInfo;
		vkUpdateDescriptorSets(VyContext::device(), 1, &write, 0, nullptr);

		// bind blit/tone-map pipeline
		m_BlitPipeline->bind(cmdBuffer);

		vkCmdBindDescriptorSets(
			cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
			m_BlitPipeline->layout(), 
			0, 1,
			&m_BlitDescriptorSet, 0, nullptr
		);

		// Fullscreen triangle
		vkCmdDraw(cmdBuffer, 3, 1, 0, 0);
	}
#pragma endregion
}