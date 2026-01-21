#include <VyEngine/VK/Pipeline/Pipeline.h>

#include <VyEngine/VK/Context.h>

#include <VyEngine/Globals.h>

namespace Vy
{
#pragma region [ GraphicsBuilder ]

	VyPipeline::GraphicsBuilder::GraphicsBuilder()
	{
		VyPipeline::defaultGraphicsPipelineConfig(m_GraphicsConfig);
	}


	VyPipeline::GraphicsBuilder::GraphicsBuilder(VyPipelineFlags flag)
	{
		if (static_cast<U32>(VyPipelineFlags::MeshShader) & static_cast<U32>(flag) != 0)
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
	VyPipeline::GraphicsBuilder::setName(TString name)
	{
		m_Name = name;

		return *this;
	}

	
	VyPipeline::GraphicsBuilder& 
	VyPipeline::GraphicsBuilder::addDescriptorSetLayout(VkDescriptorSetLayout descriptorSetLayout)
	{
		m_LayoutConfig.DescriptorSetLayouts.push_back( descriptorSetLayout );

		return *this;
	}


	VyPipeline::GraphicsBuilder& 
	VyPipeline::GraphicsBuilder::addDescriptorSetLayouts(TVector<VkDescriptorSetLayout> descriptorSetLayouts)
	{
		for (auto layout : descriptorSetLayouts)
		{
			m_LayoutConfig.DescriptorSetLayouts.push_back( layout );
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

		m_LayoutConfig.PushConstantRanges.emplace_back( pushConstantRange );

		return *this;
	}


	VyPipeline::GraphicsBuilder& 
	VyPipeline::GraphicsBuilder::addShaderStage(
		VkShaderStageFlagBits stage, 
		const TString&        shaderPath)
	{
		VkShaderModule shaderModule = VKUtil::createShaderModule(VyContext::device(), SHADER_DIR + shaderPath);
		
		m_ShaderModules.emplace_back( shaderModule );

		addShaderStage(stage, shaderModule, "main");

		return *this;
	}


	VyPipeline::GraphicsBuilder& 
	VyPipeline::GraphicsBuilder::setSubpass(U32 subpass)
	{
		m_GraphicsConfig.Subpass = subpass;

		return *this;
	}


	VyPipeline::GraphicsBuilder& 
	VyPipeline::GraphicsBuilder::setFragSpecialization(VkSpecializationInfo* specInfo)
	{
		m_GraphicsConfig.pFragSpecInfo = specInfo;

		return *this;
	}


	VyPipeline::GraphicsBuilder& 
	VyPipeline::GraphicsBuilder::addColorAttachment(
		VkFormat colorFormat, 
		bool     alphaBlending)
	{
		// https://docs.vulkan.org/spec/latest/chapters/pipelines.html#VkPipelineRenderingCreateInfo

		m_GraphicsConfig.ColorAttachmentFormats.emplace_back( colorFormat );
		
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

		m_GraphicsConfig.ColorBlendAttachments.emplace_back( colorBlendAttachment );

		m_GraphicsConfig.ColorBlendInfo.attachmentCount = m_GraphicsConfig.RenderingInfo.colorAttachmentCount;
		m_GraphicsConfig.ColorBlendInfo.pAttachments    = m_GraphicsConfig.ColorBlendAttachments.data();

		return *this;
	}


	VyPipeline::GraphicsBuilder& 
	VyPipeline::GraphicsBuilder::addColorAttachment(const VkPipelineColorBlendAttachmentState& attachment)
	{
		m_GraphicsConfig.ColorBlendAttachments.emplace_back( attachment );

		m_GraphicsConfig.ColorBlendInfo.attachmentCount = static_cast<U32>(m_GraphicsConfig.ColorBlendAttachments.size());
		m_GraphicsConfig.ColorBlendInfo.pAttachments    = m_GraphicsConfig.ColorBlendAttachments.data();

		return *this;
	}


	VyPipeline::GraphicsBuilder& 
	VyPipeline::GraphicsBuilder::clearColorAttachments()
	{
		m_GraphicsConfig.ColorBlendAttachments.clear();

		m_GraphicsConfig.ColorBlendInfo.attachmentCount = 0;
		m_GraphicsConfig.ColorBlendInfo.pAttachments    = {};

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
	VyPipeline::GraphicsBuilder::setDepthTestEnable(bool enable)
	{
		m_GraphicsConfig.DepthStencilInfo.depthTestEnable  = enable;

		return *this;
	}


	VyPipeline::GraphicsBuilder& 
	VyPipeline::GraphicsBuilder::setDepthWriteEnable(bool enable)
	{
		m_GraphicsConfig.DepthStencilInfo.depthWriteEnable = enable;

		return *this;
	}


	VyPipeline::GraphicsBuilder& 
	VyPipeline::GraphicsBuilder::setDepthCompareOp(VkCompareOp compareOp)
	{
		m_GraphicsConfig.DepthStencilInfo.depthCompareOp = compareOp;

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
	VyPipeline::GraphicsBuilder::setDepthBiasEnable(bool enable)
	{
		m_GraphicsConfig.RasterizationInfo.depthBiasEnable = enable;

		return *this;
	}


	VyPipeline::GraphicsBuilder& 
	VyPipeline::GraphicsBuilder::setDepthBiasConstantFactor(float constantFactor)
	{
		m_GraphicsConfig.RasterizationInfo.depthBiasConstantFactor = constantFactor;

		return *this;
	}


	VyPipeline::GraphicsBuilder& 
	VyPipeline::GraphicsBuilder::setDepthBiasClamp(float clamp)
	{
		m_GraphicsConfig.RasterizationInfo.depthBiasClamp = clamp;

		return *this;
	}


	VyPipeline::GraphicsBuilder& 
	VyPipeline::GraphicsBuilder::setDepthBiasSlopeFactor(float slopeFactor)
	{
		m_GraphicsConfig.RasterizationInfo.depthBiasSlopeFactor = slopeFactor;

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
    VyPipeline::GraphicsBuilder::addFlag(VyPipelineFlags flag)
    {
        m_GraphicsConfig.Flags = static_cast<VyPipelineFlags>(static_cast<U32>(m_GraphicsConfig.Flags) | static_cast<U32>(flag));;

        return *this;
    }


	Unique<VyPipeline> 
	VyPipeline::GraphicsBuilder::buildPtr()
	{
		return MakeUnique<VyPipeline>( m_Name, m_LayoutConfig, m_GraphicsConfig );
	}


	Unique<VyPipeline> 
	VyPipeline::GraphicsBuilder::buildPtr(VkPipelineLayout layout)
	{
		return MakeUnique<VyPipeline>( m_Name, layout, m_GraphicsConfig );
	}


	VyPipeline 
	VyPipeline::GraphicsBuilder::build()
	{
		return VyPipeline{ m_Name, m_LayoutConfig, m_GraphicsConfig };
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
			info.pSpecializationInfo = m_GraphicsConfig.pFragSpecInfo ? m_GraphicsConfig.pFragSpecInfo : nullptr;
		}

		m_GraphicsConfig.ShaderStages.emplace_back( info );
	}

#pragma endregion [ GraphicsBuilder ]
}