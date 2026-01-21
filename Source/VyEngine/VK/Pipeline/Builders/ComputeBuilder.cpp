#include <VyEngine/VK/Pipeline/Pipeline.h>

#include <VyEngine/VK/Context.h>
#include <VyEngine/VK/Core/Include/vk_enum_str.h>
#include <VyEngine/Globals.h>

namespace Vy
{
#pragma region [ ComputeBuilder ]

	VyPipeline::ComputeBuilder::~ComputeBuilder()
	{
		if (m_ComputeConfig.ShaderStage.module)
		{
			vkDestroyShaderModule(VyContext::device(), m_ComputeConfig.ShaderStage.module, nullptr);
		}
		else
		{
			VY_ASSERT(false, "ComputeConfig.ShaderStage.module is null");
		}
	}


	VyPipeline::ComputeBuilder& 
	VyPipeline::ComputeBuilder::setName(TString name)
	{
		m_Name = name;

		return *this;
	}


	VyPipeline::ComputeBuilder& 
	VyPipeline::ComputeBuilder::addDescriptorSetLayout(
		VkDescriptorSetLayout descriptorSetLayout)
	{
		m_LayoutConfig.DescriptorSetLayouts.emplace_back( descriptorSetLayout );

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

		m_LayoutConfig.PushConstantRanges.emplace_back( pushConstantRange );

		return *this;
	}


	VyPipeline::ComputeBuilder& 
	VyPipeline::ComputeBuilder::setShaderStage(
		const TPath& shaderPath, 
		const char*  entryPoint)
	{
		VkShaderModule shaderModule = VKUtil::createShaderModule(VyContext::device(), SHADER_DIR / shaderPath);

		m_ComputeConfig.ShaderStage = VKUtil::createShaderStage(VK_SHADER_STAGE_COMPUTE_BIT, shaderModule, entryPoint);
		
		return *this;
	}


	Unique<VyPipeline> 
	VyPipeline::ComputeBuilder::buildPtr()
	{
		return MakeUnique<VyPipeline>( m_Name, m_LayoutConfig, m_ComputeConfig );
	}


	VyPipeline 
	VyPipeline::ComputeBuilder::build()
	{
		return VyPipeline{ m_Name, m_LayoutConfig, m_ComputeConfig };
	}

#pragma endregion [ ComputeBuilder ]
}