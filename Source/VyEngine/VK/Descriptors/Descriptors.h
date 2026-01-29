#pragma once

#include <VyEngine/VK/Descriptors/Pool.h>
#include <VyEngine/VK/Descriptors/SetLayout.h>
#include <VyEngine/VK/Descriptors/Writer.h>

namespace Vy
{
	// class VyBuffer;
	// class VyTexture;

    // class VyDescriptorSet
    // {
	// public:
	// 	class Builder
	// 	{
	// 	public:
	// 		Builder(VyDescriptorSetLayout& setLayout);

	// 		Builder(const Builder&) = delete;
			
    //         ~Builder() = default;

	// 		Builder& operator=(const Builder&) noexcept = delete;

	// 		Builder& addBuffer(BindingIndex binding, const VyBuffer& buffer);
	// 		Builder& addTexture(BindingIndex binding, const VyTexture& texture);
	// 		Builder& addTexture(BindingIndex binding, Shared<VyTexture> texture);
			
    //         VyDescriptorSet build();

	// 		Unique<VyDescriptorSet> buildPtr();

	// 	private:
	// 		VyDescriptorSetLayout& m_SetLayout;
	// 		VyDescriptorWriter     m_Writer;
	// 	};

	// 	VyDescriptorSet(VyDescriptorSetLayout& setLayout);
	// 	VyDescriptorSet(VyDescriptorSetLayout& setLayout, VyDescriptorPool& pool);

	// 	VyDescriptorSet(const VyDescriptorSet&) = delete;
	// 	VyDescriptorSet(VyDescriptorSet&&) = default;
		
    //     ~VyDescriptorSet() = default;

	// 	VyDescriptorSet& operator=(const VyDescriptorSet&) = delete;
	// 	VyDescriptorSet& operator=(VyDescriptorSet&&) = default;

	// 	operator VkDescriptorSet() const { return m_DescriptorSet; }
	// 	VkDescriptorSet handle() const { return m_DescriptorSet; }

	// 	void bind(
    //         VkCommandBuffer     commandBuffer, 
    //         VkPipelineLayout    pipelineLayout, 
	// 		VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS
    //     ) const;

	// private:
	// 	VkDescriptorSet m_DescriptorSet{ VK_NULL_HANDLE };
    // };
} 
