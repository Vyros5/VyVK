#include <Vy/GFX/Backend/Pipeline.h>

#include <Vy/GFX/Resources/StaticMesh.h>

#include <Vy/GFX/Context.h>

#include <Vy/Globals.h>

namespace Vy
{
#pragma region [ GraphicsBuilder ]

	VyPipeline::GraphicsBuilder::GraphicsBuilder()
	{
		VyPipeline::defaultGraphicsPipelineConfig(m_GraphicsConfig);
	}


	VyPipeline::GraphicsBuilder::GraphicsBuilder(VyPipeline::EFlags flag)
	{
		if (static_cast<U32>(VyPipeline::EFlags::MeshShader) & static_cast<U32>(flag) != 0)
		{
			VyPipeline::defaultMeshPipelineConfig(m_GraphicsConfig);
		}
		else
		{
			VyPipeline::defaultGraphicsPipelineConfig(m_GraphicsConfig);
		}
	}


	VyPipeline::GraphicsBuilder::~GraphicsBuilder()
	{
		for (auto& shaderStage : m_GraphicsConfig.ShaderStages)
		{
			if (shaderStage.module)
			{
				vkDestroyShaderModule(VyContext::device(), shaderStage.module, nullptr);
			}
			else
			{
				VY_ASSERT(false, "ShaderStage.shaderModule is null");
			}
		}
	}


	VyPipeline::GraphicsBuilder& 
	VyPipeline::GraphicsBuilder::addDescriptorSetLayout(VkDescriptorSetLayout descriptorSetLayout)
	{
		m_LayoutConfig.DescriptorSetLayouts.push_back(descriptorSetLayout);

		return *this;
	}


	VyPipeline::GraphicsBuilder& 
	VyPipeline::GraphicsBuilder::addDescriptorSetLayouts(TVector<VkDescriptorSetLayout> descriptorSetLayouts)
	{
		for (auto layout : descriptorSetLayouts)
		{
			m_LayoutConfig.DescriptorSetLayouts.push_back(layout);
		}

		return *this;
	}


	VyPipeline::GraphicsBuilder& 
	VyPipeline::GraphicsBuilder::addPushConstantRange(
		VkShaderStageFlags stageFlags, 
		U32                size, 
		U32                offset)
	{
		VkPushConstantRange pushConstantRange{};
		{
			pushConstantRange.stageFlags = stageFlags;
			pushConstantRange.size       = size;
			pushConstantRange.offset     = offset;
		}

		m_LayoutConfig.PushConstantRanges.emplace_back(pushConstantRange);

		return *this;
	}


	VyPipeline::GraphicsBuilder& 
	VyPipeline::GraphicsBuilder::addShaderStage(
		VkShaderStageFlagBits stage, 
		const String&         shaderPath)
	{
		VkShaderModule shaderModule = VKUtil::createShaderModule(VyContext::device(), SHADER_DIR + shaderPath);
		
		m_ShaderModules.emplace_back(shaderModule);

		addShaderStage(stage, shaderModule, "main");

		return *this;
	}


	VyPipeline::GraphicsBuilder& 
	VyPipeline::GraphicsBuilder::addColorAttachment(
		VkFormat colorFormat, 
		bool     alphaBlending)
	{
		m_GraphicsConfig.ColorAttachmentFormats.emplace_back(colorFormat);

		m_GraphicsConfig.RenderingInfo.colorAttachmentCount    = static_cast<U32>(m_GraphicsConfig.ColorAttachmentFormats.size());
		m_GraphicsConfig.RenderingInfo.pColorAttachmentFormats = m_GraphicsConfig.ColorAttachmentFormats.data();

		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		{
			colorBlendAttachment.blendEnable         = VK_FALSE;
			colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
			colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
			colorBlendAttachment.colorBlendOp        = VK_BLEND_OP_ADD;
			colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
			colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
			colorBlendAttachment.alphaBlendOp        = VK_BLEND_OP_ADD;
			colorBlendAttachment.colorWriteMask      = { 
				VK_COLOR_COMPONENT_R_BIT | 
				VK_COLOR_COMPONENT_G_BIT | 
				VK_COLOR_COMPONENT_B_BIT | 
				VK_COLOR_COMPONENT_A_BIT 
			};
			
			if (alphaBlending)
			{
				colorBlendAttachment.blendEnable         = VK_TRUE;
				colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
				colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			}
		}

		m_GraphicsConfig.ColorBlendAttachments.emplace_back(colorBlendAttachment);

		m_GraphicsConfig.ColorBlendInfo.attachmentCount = m_GraphicsConfig.RenderingInfo.colorAttachmentCount;
		m_GraphicsConfig.ColorBlendInfo.pAttachments    = m_GraphicsConfig.ColorBlendAttachments.data();

		return *this;
	}


	VyPipeline::GraphicsBuilder& 
	VyPipeline::GraphicsBuilder::setDepthAttachment(VkFormat depthFormat)
	{
		m_GraphicsConfig.RenderingInfo.depthAttachmentFormat = depthFormat;

		return *this;
	}


	VyPipeline::GraphicsBuilder& 
	VyPipeline::GraphicsBuilder::setStencilFormat(VkFormat stencilFormat)
	{
		m_GraphicsConfig.RenderingInfo.stencilAttachmentFormat = stencilFormat;

		return *this;
	}


	VyPipeline::GraphicsBuilder& 
	VyPipeline::GraphicsBuilder::setRasterizationSamples(VkSampleCountFlagBits samples)
	{
		m_GraphicsConfig.MultisampleInfo.rasterizationSamples = samples;

		return *this;
	}


	VyPipeline::GraphicsBuilder& 
	VyPipeline::GraphicsBuilder::setDepthTest(
		bool        enableDepthTest, 
		bool        writeDepth, 
		VkCompareOp compareOp)
	{
		m_GraphicsConfig.DepthStencilInfo.depthTestEnable  = enableDepthTest;
		m_GraphicsConfig.DepthStencilInfo.depthWriteEnable = writeDepth;
		m_GraphicsConfig.DepthStencilInfo.depthCompareOp   = compareOp;

		return *this;
	}


	VyPipeline::GraphicsBuilder& 
	VyPipeline::GraphicsBuilder::setDepthBias(
		float constantFactor, 
		float clamp, 
		float slopeFactor)
	{
		m_GraphicsConfig.RasterizationInfo.depthBiasEnable         = VK_TRUE;
		m_GraphicsConfig.RasterizationInfo.depthBiasConstantFactor = constantFactor;
		m_GraphicsConfig.RasterizationInfo.depthBiasClamp          = clamp;
		m_GraphicsConfig.RasterizationInfo.depthBiasSlopeFactor    = slopeFactor;

		return *this;
	}


	VyPipeline::GraphicsBuilder& 
	VyPipeline::GraphicsBuilder::setDepthClampEnable(bool depthClampEnable)
	{
		m_GraphicsConfig.RasterizationInfo.depthClampEnable = depthClampEnable;

		return *this;
	}


	VyPipeline::GraphicsBuilder&
	VyPipeline::GraphicsBuilder::setPolygonMode(VkPolygonMode polygonMode)
	{
		m_GraphicsConfig.RasterizationInfo.polygonMode = polygonMode;

		return *this;
	}


	VyPipeline::GraphicsBuilder& 
	VyPipeline::GraphicsBuilder::setLineWidth(float lineWidth)
	{
		m_GraphicsConfig.RasterizationInfo.lineWidth = lineWidth;

		return *this;
	}


	VyPipeline::GraphicsBuilder& 
	VyPipeline::GraphicsBuilder::setCullMode(VkCullModeFlags cullMode)
	{
		m_GraphicsConfig.RasterizationInfo.cullMode = cullMode;

		return *this;
	}


	VyPipeline::GraphicsBuilder& 
	VyPipeline::GraphicsBuilder::setFrontFace(VkFrontFace frontFace)
	{
		m_GraphicsConfig.RasterizationInfo.frontFace = frontFace;

		return *this;
	}


	VyPipeline::GraphicsBuilder& 
	VyPipeline::GraphicsBuilder::setTopology(VkPrimitiveTopology topology)
	{
		m_GraphicsConfig.InputAssemblyInfo.topology = topology;

		return *this;
	}
	

	VyPipeline::GraphicsBuilder& 
	VyPipeline::GraphicsBuilder::setVertexBindingDescriptions(
		const TVector<VkVertexInputBindingDescription>& bindingDescriptions)
	{
		m_GraphicsConfig.BindingDescriptions = bindingDescriptions;

		return *this;
	}


	VyPipeline::GraphicsBuilder& 
	VyPipeline::GraphicsBuilder::setVertexAttributeDescriptions(
		const TVector<VkVertexInputAttributeDescription>& attributeDescriptions)
	{
		m_GraphicsConfig.AttributeDescriptions = attributeDescriptions;

		return *this;
	}


	VyPipeline::GraphicsBuilder& 
	VyPipeline::GraphicsBuilder::clearVertexDescriptions()
	{
		m_GraphicsConfig.BindingDescriptions   = {};
		m_GraphicsConfig.AttributeDescriptions = {};

		return *this;
	}


    VyPipeline::GraphicsBuilder& 
    VyPipeline::GraphicsBuilder::setRenderPass(
		VkRenderPass renderPass)
    {
        m_GraphicsConfig.RenderPass = renderPass;

        return *this;
    }


    VyPipeline::GraphicsBuilder& 
    VyPipeline::GraphicsBuilder::addFlag(EFlags flag)
    {
        m_GraphicsConfig.Flags = static_cast<EFlags>(static_cast<U32>(m_GraphicsConfig.Flags) | static_cast<U32>(flag));;

        return *this;
    }


	Unique<VyPipeline> 
	VyPipeline::GraphicsBuilder::buildUnique()
	{
		return MakeUnique<VyPipeline>(m_LayoutConfig, m_GraphicsConfig);
	}


	Unique<VyPipeline> 
	VyPipeline::GraphicsBuilder::buildUnique(VkPipelineLayout layout)
	{
		return MakeUnique<VyPipeline>(layout, m_GraphicsConfig);
	}


	VyPipeline 
	VyPipeline::GraphicsBuilder::build()
	{
		return VyPipeline{ m_LayoutConfig, m_GraphicsConfig };
	}


	void VyPipeline::GraphicsBuilder::addShaderStage(
		VkShaderStageFlagBits stage, 
		VkShaderModule        shaderModule, 
		const char*           entryPoint)
	{
		VkPipelineShaderStageCreateInfo info{ VKInit::pipelineShaderStageCreateInfo() };
		{
			info.stage  = stage;
			info.module = shaderModule;
			info.pName  = entryPoint;

			info.flags               = 0;
			info.pSpecializationInfo = nullptr;
		}

		m_GraphicsConfig.ShaderStages.emplace_back(info);
	}

#pragma endregion [ GraphicsBuilder ]

// ================================================================================================

#pragma region [ ComputeBuilder ]

	VyPipeline::ComputeBuilder::~ComputeBuilder()
	{
		if (m_ComputeConfig.ShaderStage.module)
		{
			vkDestroyShaderModule(VyContext::device(), m_ComputeConfig.ShaderStage.module, nullptr);
		}
			else
			{
				VY_ASSERT(false, "m_ComputeConfig.ShaderStage.module is null");
			}
	}


	VyPipeline::ComputeBuilder& 
	VyPipeline::ComputeBuilder::addDescriptorSetLayout(
		VkDescriptorSetLayout descriptorSetLayout)
	{
		m_LayoutConfig.DescriptorSetLayouts.emplace_back(descriptorSetLayout);

		return *this;
	}


	VyPipeline::ComputeBuilder& 
	VyPipeline::ComputeBuilder::addPushConstantRange(
		VkShaderStageFlags stageFlags, 
		U32                size)
	{
		VY_ASSERT(size <= VyContext::device().properties().limits.maxPushConstantsSize, 
			"Push constant size exceeds device limits");

		VkPushConstantRange pushConstantRange{};
		{
			pushConstantRange.stageFlags = stageFlags;
			pushConstantRange.size       = size;
			pushConstantRange.offset     = 0;
		}

		m_LayoutConfig.PushConstantRanges.emplace_back(pushConstantRange);

		return *this;
	}


	VyPipeline::ComputeBuilder& 
	VyPipeline::ComputeBuilder::setShaderStage(
		const Path& shaderPath, 
		const char* entryPoint)
	{
		VkShaderModule shaderModule = VKUtil::createShaderModule(VyContext::device(), SHADER_DIR / shaderPath);

		m_ComputeConfig.ShaderStage = VKUtil::createShaderStage(VK_SHADER_STAGE_COMPUTE_BIT, shaderModule, entryPoint);
		
		return *this;
	}


	Unique<VyPipeline> 
	VyPipeline::ComputeBuilder::buildUnique()
	{
		return MakeUnique<VyPipeline>(m_LayoutConfig, m_ComputeConfig);
	}


	VyPipeline 
	VyPipeline::ComputeBuilder::build()
	{
		return VyPipeline{ m_LayoutConfig, m_ComputeConfig };
	}

#pragma endregion [ ComputeBuilder ]

// ================================================================================================

#pragma region [ Pipeline ]

    VyPipeline::VyPipeline(
        const VyPipeline::LayoutConfig&   layoutConfig, 
        const VyPipeline::GraphicsConfig& graphicsConfig
    ) :
        m_BindPoint{ VK_PIPELINE_BIND_POINT_GRAPHICS },
		m_Flags    { graphicsConfig.Flags            }
    {
        createPipelineLayout(layoutConfig);
        createGraphicsPipeline(graphicsConfig);
    }


	VyPipeline::VyPipeline(
		VkPipelineLayout                  layout, 
		const VyPipeline::GraphicsConfig& graphicsConfig
	) :
        m_BindPoint{ VK_PIPELINE_BIND_POINT_GRAPHICS },
		m_Layout   { layout }
	{
		createGraphicsPipeline(graphicsConfig);
	}


    VyPipeline::VyPipeline(
        const VyPipeline::LayoutConfig&   layoutConfig, 
        const VyPipeline::ComputeConfig&  computeConfig
    ) :
        m_BindPoint{ VK_PIPELINE_BIND_POINT_COMPUTE }
    {
        createPipelineLayout(layoutConfig);
        createComputePipeline(computeConfig);
    }


	VyPipeline::VyPipeline(VyPipeline&& other) noexcept : 
		m_Layout   { other.m_Layout    }, 
		m_Pipeline { other.m_Pipeline  }, 
		m_BindPoint{ other.m_BindPoint },
		m_Flags    { other.m_Flags     }
	{
		other.m_Layout   = VK_NULL_HANDLE;
		other.m_Pipeline = VK_NULL_HANDLE;
	}

    
    VyPipeline::~VyPipeline() 
    {
        destroy();
    }


	VyPipeline& VyPipeline::operator=(VyPipeline&& other) noexcept
	{
		if (this != &other)
		{
			destroy();

			m_Layout    = other.m_Layout;
			m_Pipeline  = other.m_Pipeline;
			m_BindPoint = other.m_BindPoint;
			m_Flags     = other.m_Flags;

			other.m_Layout   = VK_NULL_HANDLE;
			other.m_Pipeline = VK_NULL_HANDLE;
		}

		return *this;
	}


    void VyPipeline::bind(VkCommandBuffer cmdBuffer) const
    {
        vkCmdBindPipeline(cmdBuffer, m_BindPoint, m_Pipeline);
    }


	void VyPipeline::bindDescriptorSet(
		VkCommandBuffer cmdBuffer, 
		SetIndex        setIndex, 
		VkDescriptorSet descriptorSet,
		U32             dynamicOffsetCount,
		const U32*      pDynamicOffsets
	) const
	{
		vkCmdBindDescriptorSets(cmdBuffer, 
			m_BindPoint, 
			m_Layout, 
			setIndex, 
			1, 
			&descriptorSet, 
			dynamicOffsetCount, 
			pDynamicOffsets
		);
	}


	void VyPipeline::bindDescriptorSets(
		VkCommandBuffer          cmdBuffer, 
		SetIndex                 setIndex, 
		TVector<VkDescriptorSet> descriptorSets,
		U32                      dynamicOffsetCount,
		const U32*               pDynamicOffsets
	) const
	{
		vkCmdBindDescriptorSets(cmdBuffer, 
			m_BindPoint, 
			m_Layout, 
			setIndex, 
			static_cast<U32>(descriptorSets.size()), 
			descriptorSets.data(), 
			dynamicOffsetCount, 
			pDynamicOffsets
		);
	}

	
	void VyPipeline::pushConstants(
		VkCommandBuffer    cmdBuffer, 
		VkShaderStageFlags stageFlags, 
		const void*        pData, 
		U32                size, 
		U32                offset) const
	{
		vkCmdPushConstants(cmdBuffer, m_Layout, stageFlags, offset, size, pData);
	}


	void VyPipeline::createPipelineLayout(const VyPipeline::LayoutConfig& config)
	{
		VkPipelineLayoutCreateInfo layoutInfo{ VKInit::pipelineLayoutCreateInfo() };
		{
			layoutInfo.setLayoutCount         = static_cast<U32>(config.DescriptorSetLayouts.size());
			layoutInfo.pSetLayouts            = config.DescriptorSetLayouts.data();

			layoutInfo.pushConstantRangeCount = static_cast<U32>(config.PushConstantRanges.size());
			layoutInfo.pPushConstantRanges    = config.PushConstantRanges.data();
		}

		VK_CHECK(vkCreatePipelineLayout(VyContext::device(), &layoutInfo, nullptr, &m_Layout));
	}


	void VyPipeline::createGraphicsPipeline(const VyPipeline::GraphicsConfig& config)
	{
        VY_ASSERT(config.RenderPass != VK_NULL_HANDLE,
            "Cannot create graphics pipeline: no RenderPass provided in config"
        );

		auto& bindingDescriptions   = config.BindingDescriptions;
		auto& attributeDescriptions = config.AttributeDescriptions;

		// Vertex Input
		VkPipelineVertexInputStateCreateInfo vertexInputInfo{ VKInit::pipelineVertexInputStateCreateInfo() };
		{
			vertexInputInfo.vertexAttributeDescriptionCount = static_cast<U32>(attributeDescriptions.size());
			vertexInputInfo.pVertexAttributeDescriptions    = attributeDescriptions.data();

			vertexInputInfo.vertexBindingDescriptionCount   = static_cast<U32>(bindingDescriptions.size());
			vertexInputInfo.pVertexBindingDescriptions      = bindingDescriptions.data();
		}

		// Pipeline Info
		VkGraphicsPipelineCreateInfo pipelineInfo{ VKInit::graphicsPipelineCreateInfo() };
		{
			pipelineInfo.pNext               = &config.RenderingInfo;
			
			pipelineInfo.stageCount          = static_cast<U32>(config.ShaderStages.size());
			pipelineInfo.pStages             = config.ShaderStages.data();

			pipelineInfo.pVertexInputState   = &vertexInputInfo;
			pipelineInfo.pInputAssemblyState = &config.InputAssemblyInfo;
			pipelineInfo.pTessellationState  = nullptr;
			pipelineInfo.pViewportState      = &config.ViewportInfo;
			pipelineInfo.pRasterizationState = &config.RasterizationInfo;
			pipelineInfo.pMultisampleState   = &config.MultisampleInfo;
			pipelineInfo.pColorBlendState    = &config.ColorBlendInfo;
			pipelineInfo.pDepthStencilState  = &config.DepthStencilInfo;
			pipelineInfo.pDynamicState       = &config.DynamicStateInfo;

			pipelineInfo.layout              = m_Layout;
			pipelineInfo.renderPass          = config.RenderPass;
			pipelineInfo.subpass             = config.Subpass;

			pipelineInfo.basePipelineHandle  = VK_NULL_HANDLE;
			pipelineInfo.basePipelineIndex   = -1;
		}

		VK_CHECK(vkCreateGraphicsPipelines(VyContext::device(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_Pipeline));
	}


	void VyPipeline::createComputePipeline(const VyPipeline::ComputeConfig& config)
	{
		VY_ASSERT(config.ShaderStage.module != VK_NULL_HANDLE, 
			"Cannot create pipeline: no shader provided");

		// Pipeline Info
		VkComputePipelineCreateInfo pipelineInfo{ VKInit::computePipelineCreateInfo() };
		{
			pipelineInfo.stage              = config.ShaderStage;
			pipelineInfo.layout             = m_Layout;

			pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
			pipelineInfo.basePipelineIndex  = -1;
		}

		VK_CHECK(vkCreateComputePipelines(VyContext::device(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_Pipeline));
	}


	void VyPipeline::destroy()
	{
		VyContext::destroy(m_Pipeline);
		m_Pipeline = VK_NULL_HANDLE;

		if (m_Layout != VK_NULL_HANDLE)
		{
			vkDestroyPipelineLayout(VyContext::device(), m_Layout, nullptr);

			m_Layout = VK_NULL_HANDLE;
		}
	}

#pragma endregion [ Pipeline ]


#pragma region [ Config ]

    void VyPipeline::defaultGraphicsPipelineConfig(VyPipeline::GraphicsConfig& config) 
    {
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Rendering Info

		config.RenderingInfo = VKInit::pipelineRenderingCreateInfo();

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Input Assembly

		// https://docs.vulkan.org/refpages/latest/refpages/source/VkPipelineInputAssemblyStateCreateInfo.html
		config.InputAssemblyInfo = VKInit::pipelineInputAssemblyStateCreateInfo();
		{
			// Triangle list:  (1,2,3), (4,5,6) 
			// Triangle strip: (1,2,3), (3,4,5)

			config.InputAssemblyInfo.topology                = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
			config.InputAssemblyInfo.primitiveRestartEnable  = VK_FALSE;
		}

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Viewport

		config.ViewportInfo = VKInit::pipelineViewportStateCreateInfo();
		{
			// Dynamic Rendering is enabled for viewport and scissors so these values are ignored (nullptr).
			config.ViewportInfo.viewportCount                = 1;
			config.ViewportInfo.pViewports                   = nullptr;
			
			config.ViewportInfo.scissorCount                 = 1;
			config.ViewportInfo.pScissors                    = nullptr;
		}

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Rasterization

		// This stage breaks up the geometry into fragments for each pixel or triangle overlaps.

		config.RasterizationInfo = VKInit::pipelineRasterizationStateCreateInfo();
		{
			// If VK_TRUE, force the Z component of gl_Position (or SV_Position) to be between 0 and 1, 
			// so values less than 0 are clamped to 0, and values greater than 1 are clamped to 1.
			// Not enabled, Usually means it's behind the camera, and if it's greater than 1 means it's far away from the camera.
			config.RasterizationInfo.depthClampEnable        = VK_FALSE;

			// Discard all primitives before rasterization, so if we do not continue in the next stage of the pipeline, 
			// we can set to VK_TRUE, but if we are using all stages, like in normal situation, this has to be VK_FALSE.
			config.RasterizationInfo.rasterizerDiscardEnable = VK_FALSE;

			config.RasterizationInfo.polygonMode             = VK_POLYGON_MODE_FILL;
			config.RasterizationInfo.cullMode                = VK_CULL_MODE_NONE;
			config.RasterizationInfo.frontFace               = VK_FRONT_FACE_CLOCKWISE;
			
			config.RasterizationInfo.depthBiasEnable         = VK_FALSE;
			config.RasterizationInfo.depthBiasConstantFactor = 0.0f; // A scalar factor controlling the constant depth value added to each fragment.
			config.RasterizationInfo.depthBiasClamp          = 0.0f; // The maximum (or minimum) depth bias of a fragment.
			config.RasterizationInfo.depthBiasSlopeFactor    = 0.0f; // A scalar factor applied to a fragment’s slope in depth bias calculations.
			config.RasterizationInfo.lineWidth               = 1.0f; // The width of rasterized line segments.
		}

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Multisample

		config.MultisampleInfo = VKInit::pipelineMultisampleStateCreateInfo();
		{
			// Sample Shading: https://docs.vulkan.org/spec/latest/chapters/primsrast.html#primsrast-sampleshading
			config.MultisampleInfo.sampleShadingEnable       = VK_FALSE;
			config.MultisampleInfo.rasterizationSamples      = VK_SAMPLE_COUNT_1_BIT;
			config.MultisampleInfo.minSampleShading          = 1.0f;
			config.MultisampleInfo.pSampleMask               = nullptr;
			config.MultisampleInfo.alphaToCoverageEnable     = VK_FALSE;
			config.MultisampleInfo.alphaToOneEnable          = VK_FALSE;
		}

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Color Blend

		config.ColorBlendInfo = VKInit::pipelineColorBlendStateCreateInfo();
		{
			config.ColorBlendInfo.logicOpEnable              = VK_FALSE;
			config.ColorBlendInfo.logicOp                    = VK_LOGIC_OP_COPY;
			config.ColorBlendInfo.attachmentCount            = static_cast<U32>(config.ColorBlendAttachments.size());
			config.ColorBlendInfo.pAttachments               = config.ColorBlendAttachments.data();
			config.ColorBlendInfo.blendConstants[0]          = 0.0f;
			config.ColorBlendInfo.blendConstants[1]          = 0.0f;
			config.ColorBlendInfo.blendConstants[2]          = 0.0f;
			config.ColorBlendInfo.blendConstants[3]          = 0.0f;
		}

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Depth Stencil

		config.DepthStencilInfo = VKInit::pipelineDepthStencilStateCreateInfo();
		{
			config.DepthStencilInfo.depthTestEnable          = VK_TRUE;
			config.DepthStencilInfo.depthWriteEnable         = VK_TRUE;
			config.DepthStencilInfo.depthCompareOp           = VK_COMPARE_OP_LESS;
			config.DepthStencilInfo.depthBoundsTestEnable    = VK_FALSE;
			config.DepthStencilInfo.stencilTestEnable        = VK_FALSE;
			config.DepthStencilInfo.minDepthBounds           = 0.0f;
			config.DepthStencilInfo.maxDepthBounds           = 1.0f;
			config.DepthStencilInfo.front                    = {};
			config.DepthStencilInfo.back                     = {};
		}

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Dynamic State

		config.DynamicStateEnables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

		config.DynamicStateInfo = VKInit::pipelineDynamicStateCreateInfo();
		{
			config.DynamicStateInfo.pDynamicStates           = config.DynamicStateEnables.data();
			config.DynamicStateInfo.dynamicStateCount        = static_cast<U32>(config.DynamicStateEnables.size());
			config.DynamicStateInfo.flags                    = 0;
		}

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Descriptions

        config.BindingDescriptions   = VyStaticMesh::vertexBindingDescriptions();
        config.AttributeDescriptions = VyStaticMesh::vertexAttributeDescriptions();
    }



	void VyPipeline::defaultMeshPipelineConfig(VyPipeline::GraphicsConfig& config)
	{
		defaultGraphicsPipelineConfig(config);

		// Mesh shaders don't use vertex input or input assembly.
        config.BindingDescriptions   = {{}};
        config.AttributeDescriptions = {{}};

		// Ignored but good practice.
		config.InputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	}

#pragma endregion [ Config ]
}