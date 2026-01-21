#pragma once

#include <VyEngine/VK/Core/Include/vkImpl.h>
#include <VyEngine/VK/Core/Include/vk_enum_str.h>

#include <VyLib/VyLib.h>

namespace Vy::VKInit
{
    inline VkApplicationInfo applicationInfo() 
    {
        VkApplicationInfo ret{};
        ret.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;

        return ret;
    }

    inline VkInstanceCreateInfo instanceCreateInfo() 
    {
        VkInstanceCreateInfo ret{};
        ret.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

        return ret;
    }

    inline VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfoEXT() 
    {
        VkDebugUtilsMessengerCreateInfoEXT ret{};
        ret.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;

        return ret;
    } 

    inline VkDebugUtilsObjectNameInfoEXT debugUtilsObjectNameInfoEXT() 
    {
        VkDebugUtilsObjectNameInfoEXT ret{};
        ret.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;

        return ret;
    }

    inline VkDeviceCreateInfo deviceCreateInfo() 
    {
        VkDeviceCreateInfo ret{};
        ret.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

        return ret;
    }

    inline VkDeviceQueueCreateInfo deviceQueueCreateInfo() 
    {
        VkDeviceQueueCreateInfo ret{};
        ret.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;

        return ret;
    }

    inline VkSwapchainCreateInfoKHR swapchainCreateInfoKHR() 
    {
        VkSwapchainCreateInfoKHR ret{};
        ret.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;

        return ret;
    }

    inline VkImageViewCreateInfo imageViewCreateInfo() 
    {
        VkImageViewCreateInfo ret{};
        ret.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;

        return ret;
    }

    inline VkImageCreateInfo imageCreateInfo() 
    {
        VkImageCreateInfo ret{};
        ret.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;

        return ret;
    }

    inline VkSemaphoreCreateInfo semaphoreCreateInfo() 
    {
        VkSemaphoreCreateInfo ret{};
        ret.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        return ret;
    }

    inline VkSemaphoreTypeCreateInfo semaphoreTypeCreateInfo() 
    {
        VkSemaphoreTypeCreateInfo ret{};
        ret.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;

        return ret;
    }

    inline VkSemaphoreSignalInfo semaphoreSignalInfo() 
    {
        VkSemaphoreSignalInfo ret{};
        ret.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SIGNAL_INFO;

        return ret;
    }

    inline VkSemaphoreWaitInfo semaphoreWaitInfo() 
    {
        VkSemaphoreWaitInfo ret{};
        ret.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;

        return ret;
    }

    inline VkSemaphoreSubmitInfo semaphoreSubmitInfo() 
    {
        VkSemaphoreSubmitInfo ret{};
        ret.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;

        return ret;
    }

    inline VkVertexInputAttributeDescription vertexInputAttributeDescription(
        U32       binding,
        U32       location,
        VkFormat format,
        U32       offset)
    {
        VkVertexInputAttributeDescription vInputAttribDescription{};
        {
            vInputAttribDescription.location = location;
            vInputAttribDescription.binding  = binding;
            vInputAttribDescription.format   = format;
            vInputAttribDescription.offset   = offset;
        }

        return vInputAttribDescription;
    }

    inline VkVertexInputBindingDescription vertexInputBindingDescription(
        U32               binding,
        U32               stride,
        VkVertexInputRate inputRate)
    {
        VkVertexInputBindingDescription vInputBindDescription{};
        {
            vInputBindDescription.binding   = binding;
            vInputBindDescription.stride    = stride;
            vInputBindDescription.inputRate = inputRate;
        }

        return vInputBindDescription;
    }

    inline VkFenceCreateInfo fenceCreateInfo() 
    {
        VkFenceCreateInfo ret{};
        ret.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

        return ret;
    }

	inline VkFenceCreateInfo fenceCreateInfo(VkFenceCreateFlags flags)
	{
		VkFenceCreateInfo ret{};
		ret.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		ret.flags = flags;

		return ret;
	}

    inline VkMemoryAllocateInfo memoryAllocateInfo() 
    {
        VkMemoryAllocateInfo ret{};
        ret.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;

        return ret;
    }

    inline VkDescriptorPoolCreateInfo descriptorPoolCreateInfo() 
    {
        VkDescriptorPoolCreateInfo ret{};
        ret.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;

        return ret;
    }

    inline VkCommandPoolCreateInfo commandPoolCreateInfo() 
    {
        VkCommandPoolCreateInfo ret{};
        ret.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;

        return ret;
    }

    inline VkCommandBufferAllocateInfo commandBufferAllocateInfo() 
    {
        VkCommandBufferAllocateInfo ret{};
        ret.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;

        return ret;
    }

    inline VkCommandBufferBeginInfo commandBufferBeginInfo() 
    {
        VkCommandBufferBeginInfo ret{};
        ret.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        return ret;
    }

    inline VkCommandBufferSubmitInfo commandBufferSubmitInfo() 
    {
        VkCommandBufferSubmitInfo ret{};
        ret.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;

        return ret;
    }

    inline VkRenderPassBeginInfo renderPassBeginInfo() 
    {
        VkRenderPassBeginInfo ret{};
        ret.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;

        return ret;
    }

    inline VkRenderPassCreateInfo renderPassCreateInfo() 
    {
        VkRenderPassCreateInfo ret{};
        ret.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;

        return ret;
    }

    inline VkFramebufferCreateInfo framebufferCreateInfo() 
    {
        VkFramebufferCreateInfo ret{};
        ret.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;

        return ret;
    }

    inline VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo() 
    {
        VkDescriptorSetLayoutCreateInfo ret{};
        ret.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;

        return ret;
    }

    inline VkSubmitInfo submitInfo() 
    {
        VkSubmitInfo ret{};
        ret.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        return ret;
    }

    inline VkSubmitInfo2 submitInfo2() 
    {
        VkSubmitInfo2 ret{};
        ret.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;

        return ret;
    }

    inline VkDependencyInfo dependencyInfo()
    {
        VkDependencyInfo ret{};
        ret.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;

        return ret;
    }

    inline VkShaderModuleCreateInfo shaderModuleCreateInfo() 
    {
        VkShaderModuleCreateInfo ret{};
        ret.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;

        return ret;
    }

    inline VkSamplerCreateInfo samplerCreateInfo() 
    {
        VkSamplerCreateInfo ret{};
        ret.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;

        return ret;
    }

    inline VkPresentInfoKHR presentInfoKHR() 
    {
        VkPresentInfoKHR ret{};
        ret.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        return ret;
    }

    inline VkPresentIdKHR presentIdKHR() 
    {
        VkPresentIdKHR ret{};
		{
			ret.sType          = VK_STRUCTURE_TYPE_PRESENT_ID_KHR;
			ret.pNext          = nullptr;
			ret.swapchainCount = 0;
			ret.pPresentIds    = nullptr;
		}

		return ret;
    }

    inline VkBufferCreateInfo bufferCreateInfo() 
    {
        VkBufferCreateInfo ret{};
        ret.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;

        return ret;
    }

    inline VkMemoryBarrier memoryBarrier() 
    {
        VkMemoryBarrier ret{};
        ret.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;

        return ret;
    }
	
	/** @brief Initialize an image memory barrier with no image transfer ownership */
	inline VkImageMemoryBarrier imageMemoryBarrier()
	{
		VkImageMemoryBarrier ret{};
		ret.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		ret.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		ret.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		return ret;
	}

	/** @brief Initialize a buffer memory barrier with no image transfer ownership */
	inline VkBufferMemoryBarrier bufferMemoryBarrier()
	{
		VkBufferMemoryBarrier ret{};
		ret.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
		ret.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		ret.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		return ret;
	}

    inline VkImageMemoryBarrier2 imageMemoryBarrier2() 
    {
        VkImageMemoryBarrier2 ret{};
        ret.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;

        return ret;
    }

    inline VkImageBlit2 imageBlit2() 
    {
        VkImageBlit2 ret{};
        ret.sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2;

        return ret;
    }

    inline VkBlitImageInfo2 blitImageInfo2() 
    {
        VkBlitImageInfo2 ret{};
        ret.sType = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2;

        return ret;
    }

    inline VkDescriptorSetAllocateInfo descriptorSetAllocateInfo() 
    {
        VkDescriptorSetAllocateInfo ret{};
        ret.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;

        return ret;
    }
    
    inline VkDescriptorSetVariableDescriptorCountAllocateInfo descriptorSetVariableDescriptorCountAllocateInfo() 
    {
        VkDescriptorSetVariableDescriptorCountAllocateInfo ret{};
        ret.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO;

        return ret;
    }

    inline VkWriteDescriptorSet writeDescriptorSet() 
    {
        VkWriteDescriptorSet ret{};
        ret.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;

        return ret;
    }

    inline VkPhysicalDeviceVulkan13Features physicalDeviceVulkan13Features() 
    {
        VkPhysicalDeviceVulkan13Features ret{};
        ret.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;

        return ret;
    }

    inline VkPhysicalDeviceFeatures2 physicalDeviceFeatures2() 
    {
        VkPhysicalDeviceFeatures2 ret{};
        ret.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;

        return ret;
    }

    inline VkRenderingInfo renderingInfo()
    {
        VkRenderingInfo ret{};
        ret.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;

        return ret;
    }

    inline VkRenderingInfoKHR renderingInfoKHR()
    {
        VkRenderingInfoKHR ret{};
        ret.sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR;

        return ret;
    }

    inline VkRenderingAttachmentInfoKHR renderingAttachmentInfoKHR()
    {
        VkRenderingAttachmentInfoKHR ret{};
        ret.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;

        return ret;
    }

    inline VkRenderingAttachmentInfo renderingAttachmentInfo()
    {
        VkRenderingAttachmentInfo ret{};
        ret.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;

        return ret;
    }


    inline VkDescriptorSetLayoutBindingFlagsCreateInfo descriptorSetLayoutBindingFlagsCreateInfo()
    {
        VkDescriptorSetLayoutBindingFlagsCreateInfo ret{};
        ret.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;

        return ret;
    }

    inline VkBufferDeviceAddressInfo bufferDeviceAddressInfo()
    {
        VkBufferDeviceAddressInfo ret{};
        ret.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;

        return ret;
    }


    // ========================================================================================

    VY_NODISCARD inline VkImageSubresourceRange imageSubresourceRange(VkImageAspectFlags aspectMask) 
    {
        VkImageSubresourceRange subImage{};
        {
            subImage.aspectMask     = aspectMask;
            subImage.baseMipLevel   = 0;
            subImage.levelCount     = 1;
            subImage.baseArrayLayer = 0;
            subImage.layerCount     = 1;
        }

        return subImage;
    }

    inline VkPushConstantRange pushConstantRange(VkShaderStageFlags stageFlags, U32 size, U32 offset) 
    {
        VkPushConstantRange ret{};
        {
            ret.stageFlags = stageFlags;
            ret.offset     = offset;
            ret.size       = size;
        }

        return ret;
    }












	inline VkEventCreateInfo eventCreateInfo()
	{
		VkEventCreateInfo eventCreateInfo{};
		eventCreateInfo.sType = VK_STRUCTURE_TYPE_EVENT_CREATE_INFO;
		return eventCreateInfo;
	}

	inline VkViewport viewport(
		float width,
		float height,
		float minDepth,
		float maxDepth)
	{
		VkViewport viewport{};
		viewport.width = width;
		viewport.height = height;
		viewport.minDepth = minDepth;
		viewport.maxDepth = maxDepth;
		return viewport;
	}

	inline VkRect2D rect2D(
		int32_t width,
		int32_t height,
		int32_t offsetX,
		int32_t offsetY)
	{
		VkRect2D rect2D{};
		rect2D.extent.width = width;
		rect2D.extent.height = height;
		rect2D.offset.x = offsetX;
		rect2D.offset.y = offsetY;
		return rect2D;
	}


	inline VkBufferCreateInfo bufferCreateInfo(
		VkBufferUsageFlags usage,
		VkDeviceSize size)
	{
		VkBufferCreateInfo bufCreateInfo{};
		bufCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufCreateInfo.usage = usage;
		bufCreateInfo.size = size;
		return bufCreateInfo;
	}

	inline VkDescriptorPoolCreateInfo descriptorPoolCreateInfo(
		U32 poolSizeCount,
		VkDescriptorPoolSize* pPoolSizes,
		U32 maxSets)
	{
		VkDescriptorPoolCreateInfo descriptorPoolInfo{};
		descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descriptorPoolInfo.poolSizeCount = poolSizeCount;
		descriptorPoolInfo.pPoolSizes = pPoolSizes;
		descriptorPoolInfo.maxSets = maxSets;
		return descriptorPoolInfo;
	}

	inline VkDescriptorPoolCreateInfo descriptorPoolCreateInfo(
		const TVector<VkDescriptorPoolSize>& poolSizes,
		U32 maxSets)
	{
		VkDescriptorPoolCreateInfo descriptorPoolInfo{};
		descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descriptorPoolInfo.poolSizeCount = static_cast<U32>(poolSizes.size());
		descriptorPoolInfo.pPoolSizes = poolSizes.data();
		descriptorPoolInfo.maxSets = maxSets;
		return descriptorPoolInfo;
	}

	inline VkDescriptorPoolSize descriptorPoolSize(
		VkDescriptorType type,
		U32 descriptorCount)
	{
		VkDescriptorPoolSize descriptorPoolSize{};
		descriptorPoolSize.type = type;
		descriptorPoolSize.descriptorCount = descriptorCount;
		return descriptorPoolSize;
	}

    VY_NODISCARD inline VkDescriptorSetLayoutBinding descriptorSetLayoutBinding(
		VkDescriptorType   type, 
		VkShaderStageFlags stageFlags, 
		U32                binding,
		U32                descriptorCount = 1) 
    {
        VkDescriptorSetLayoutBinding layoutBinding{};
        {
            layoutBinding.binding            = binding;
            layoutBinding.descriptorCount    = descriptorCount;
            layoutBinding.descriptorType     = type;
            layoutBinding.stageFlags         = stageFlags;
            layoutBinding.pImmutableSamplers = nullptr;
        }

        return layoutBinding;
    }


	inline VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo(
		const VkDescriptorSetLayoutBinding* pBindings,
		U32 bindingCount)
	{
		VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{};
		descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		descriptorSetLayoutCreateInfo.pBindings = pBindings;
		descriptorSetLayoutCreateInfo.bindingCount = bindingCount;
		return descriptorSetLayoutCreateInfo;
	}

	inline VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo(
		const TVector<VkDescriptorSetLayoutBinding>& bindings)
	{
		VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{};
		descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		descriptorSetLayoutCreateInfo.pBindings = bindings.data();
		descriptorSetLayoutCreateInfo.bindingCount = static_cast<U32>(bindings.size());
		return descriptorSetLayoutCreateInfo;
	}







    // MARK: Pipeline

    inline VkPipelineShaderStageCreateInfo pipelineShaderStageCreateInfo() 
    {
        VkPipelineShaderStageCreateInfo ret{};
        ret.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;

        return ret;
    }


    inline VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo() 
    {
        VkGraphicsPipelineCreateInfo ret{};
		{
			ret.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
			ret.basePipelineIndex = -1;
			ret.basePipelineHandle = VK_NULL_HANDLE;
		}

        return ret;
    }

	inline VkGraphicsPipelineCreateInfo pipelineCreateInfo()
	{
		VkGraphicsPipelineCreateInfo ret{};
		ret.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		ret.basePipelineIndex = -1;
		ret.basePipelineHandle = VK_NULL_HANDLE;
		return ret;
	}

	inline VkGraphicsPipelineCreateInfo pipelineCreateInfo(
		VkPipelineLayout      layout,
		VkRenderPass          renderPass,
		VkPipelineCreateFlags flags = 0)
	{
		VkGraphicsPipelineCreateInfo ret{};
		ret.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		ret.layout = layout;
		ret.renderPass = renderPass;
		ret.flags = flags;
		ret.basePipelineIndex = -1;
		ret.basePipelineHandle = VK_NULL_HANDLE;
		return ret;
	}


    inline VkComputePipelineCreateInfo computePipelineCreateInfo() 
    {
        VkComputePipelineCreateInfo ret{};
        ret.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;

        return ret;
    }

	inline VkComputePipelineCreateInfo computePipelineCreateInfo(
		VkPipelineLayout      layout,
		VkPipelineCreateFlags flags = 0)
	{
		VkComputePipelineCreateInfo ret{};
		{
			ret.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
			ret.layout = layout;
			ret.flags = flags;
		}

		return ret;
	}


    inline VkPipelineRenderingCreateInfo pipelineRenderingCreateInfo()
    {
        VkPipelineRenderingCreateInfo ret{};
        ret.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;

        return ret;
    }


    inline VkPipelineViewportStateCreateInfo pipelineViewportStateCreateInfo() 
    {
        VkPipelineViewportStateCreateInfo ret{};
        ret.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;

        return ret;
    }

	inline VkPipelineViewportStateCreateInfo pipelineViewportStateCreateInfo(
		U32                                viewportCount,
		U32                                scissorCount,
		VkPipelineViewportStateCreateFlags flags = 0)
	{
		VkPipelineViewportStateCreateInfo ret{};
		{
			ret.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
			ret.viewportCount = viewportCount;
			ret.scissorCount = scissorCount;
			ret.flags = flags;
		}

		return ret;
	}


    inline VkPipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo()
    {
        VkPipelineInputAssemblyStateCreateInfo ret{};
        ret.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;

        return ret;
    }

	inline VkPipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo(
		VkPrimitiveTopology                     topology,
		VkPipelineInputAssemblyStateCreateFlags flags,
		VkBool32                                primitiveRestartEnable)
	{
		VkPipelineInputAssemblyStateCreateInfo ret{};
		{
			ret.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
			ret.topology = topology;
			ret.flags = flags;
			ret.primitiveRestartEnable = primitiveRestartEnable;
		}

		return ret;
	}


    inline VkPipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo()
    {
        VkPipelineRasterizationStateCreateInfo ret{};
        ret.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;

        return ret;
    }

	inline VkPipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo(
		VkPolygonMode                           polygonMode,
		VkCullModeFlags                         cullMode,
		VkFrontFace                             frontFace,
		VkPipelineRasterizationStateCreateFlags flags = 0)
	{
		VkPipelineRasterizationStateCreateInfo ret{};
		{
			ret.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
			ret.polygonMode = polygonMode;
			ret.cullMode = cullMode;
			ret.frontFace = frontFace;
			ret.flags = flags;
			ret.depthClampEnable = VK_FALSE;
			ret.lineWidth = 1.0f;
		}

		return ret;
	}


    inline VkPipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo()
    {
        VkPipelineMultisampleStateCreateInfo ret{};
        ret.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;

        return ret;
    }

	inline VkPipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo(
		VkSampleCountFlagBits                 rasterizationSamples,
		VkPipelineMultisampleStateCreateFlags flags = 0)
	{
		VkPipelineMultisampleStateCreateInfo ret{};
		{
			ret.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
			ret.rasterizationSamples = rasterizationSamples;
			ret.flags = flags;
		}

		return ret;
	}


    inline VkPipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo()
    {
        VkPipelineColorBlendStateCreateInfo ret{};
        ret.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;

        return ret;
    }

	inline VkPipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo(
		U32                                        attachmentCount,
		const VkPipelineColorBlendAttachmentState* pAttachments)
	{
		VkPipelineColorBlendStateCreateInfo ret{};
		{
			ret.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
			ret.attachmentCount = attachmentCount;
			ret.pAttachments = pAttachments;
		}

		return ret;
	}


    inline VkPipelineDepthStencilStateCreateInfo pipelineDepthStencilStateCreateInfo()
    {
        VkPipelineDepthStencilStateCreateInfo ret{};
        ret.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;

        return ret;
    }

	inline VkPipelineDepthStencilStateCreateInfo pipelineDepthStencilStateCreateInfo(
		VkBool32    depthTestEnable,
		VkBool32    depthWriteEnable,
		VkCompareOp depthCompareOp)
	{
		VkPipelineDepthStencilStateCreateInfo ret{};
		{
			ret.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
			ret.depthTestEnable = depthTestEnable;
			ret.depthWriteEnable = depthWriteEnable;
			ret.depthCompareOp = depthCompareOp;
			ret.back.compareOp = VK_COMPARE_OP_ALWAYS;
		}

		return ret;
	}


    inline VkPipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo()
    {
        VkPipelineDynamicStateCreateInfo ret{};
        ret.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;

        return ret;
    }

	inline VkPipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo(
		const TVector<VkDynamicState>&    pDynamicStates,
		VkPipelineDynamicStateCreateFlags flags = 0)
	{
		VkPipelineDynamicStateCreateInfo ret{};
		{
			ret.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
			ret.pDynamicStates = pDynamicStates.data();
			ret.dynamicStateCount = static_cast<U32>(pDynamicStates.size());
			ret.flags = flags;
		}

		return ret;
	}

	inline VkPipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo(
		const VkDynamicState*             pDynamicStates,
		U32                               dynamicStateCount,
		VkPipelineDynamicStateCreateFlags flags = 0)
	{
		VkPipelineDynamicStateCreateInfo ret{};
		{
			ret.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
			ret.pDynamicStates = pDynamicStates;
			ret.dynamicStateCount = dynamicStateCount;
			ret.flags = flags;
		}

		return ret;
	}



	inline VkPipelineColorBlendAttachmentState pipelineColorBlendAttachmentState(
		VkColorComponentFlags colorWriteMask,
		VkBool32              blendEnable)
	{
		VkPipelineColorBlendAttachmentState ret{};
		{
			ret.colorWriteMask = colorWriteMask;
			ret.blendEnable    = blendEnable;
		}

		return ret;
	}


    inline VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo() 
    {
        VkPipelineVertexInputStateCreateInfo ret{};
        ret.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

        return ret;
    }

	inline VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo(
		const TVector<VkVertexInputBindingDescription>&   vertexBindingDescriptions,
		const TVector<VkVertexInputAttributeDescription>& vertexAttributeDescriptions)
	{
		VkPipelineVertexInputStateCreateInfo ret{};
		{
			ret.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
			ret.vertexBindingDescriptionCount = static_cast<U32>(vertexBindingDescriptions.size());
			ret.pVertexBindingDescriptions = vertexBindingDescriptions.data();
			ret.vertexAttributeDescriptionCount = static_cast<U32>(vertexAttributeDescriptions.size());
			ret.pVertexAttributeDescriptions = vertexAttributeDescriptions.data();
		}

		return ret;
	}


    inline VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo() 
    {
        VkPipelineLayoutCreateInfo ret{};
        ret.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

        return ret;
    }

	inline VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo(
		U32 setLayoutCount)
	{
		VkPipelineLayoutCreateInfo ret{};
		{
			ret.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			ret.setLayoutCount = setLayoutCount;
		}

		return ret;
	}

	inline VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo(
		const VkDescriptorSetLayout* pSetLayouts,
		U32                          setLayoutCount)
	{
		VkPipelineLayoutCreateInfo ret{};
		{
			ret.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			ret.setLayoutCount = setLayoutCount;
			ret.pSetLayouts    = pSetLayouts;
		}

		return ret;
	}



	inline VkDescriptorSetAllocateInfo descriptorSetAllocateInfo(
		VkDescriptorPool descriptorPool,
		const VkDescriptorSetLayout* pSetLayouts,
		U32 descriptorSetCount)
	{
		VkDescriptorSetAllocateInfo descriptorSetAllocateInfo{};
		descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		descriptorSetAllocateInfo.descriptorPool = descriptorPool;
		descriptorSetAllocateInfo.pSetLayouts = pSetLayouts;
		descriptorSetAllocateInfo.descriptorSetCount = descriptorSetCount;
		return descriptorSetAllocateInfo;
	}

	inline VkDescriptorImageInfo descriptorImageInfo(VkSampler sampler, VkImageView imageView, VkImageLayout imageLayout)
	{
		VkDescriptorImageInfo descriptorImageInfo{};
		descriptorImageInfo.sampler = sampler;
		descriptorImageInfo.imageView = imageView;
		descriptorImageInfo.imageLayout = imageLayout;
		return descriptorImageInfo;
	}

	inline VkWriteDescriptorSet writeDescriptorSet(
		VkDescriptorSet dstSet,
		VkDescriptorType type,
		U32 binding,
		VkDescriptorBufferInfo* bufferInfo,
		U32 descriptorCount = 1)
	{
		VkWriteDescriptorSet writeDescriptorSet{};
		writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSet.dstSet = dstSet;
		writeDescriptorSet.descriptorType = type;
		writeDescriptorSet.dstBinding = binding;
		writeDescriptorSet.pBufferInfo = bufferInfo;
		writeDescriptorSet.descriptorCount = descriptorCount;
		return writeDescriptorSet;
	}

	inline VkWriteDescriptorSet writeDescriptorSet(
		VkDescriptorSet dstSet,
		VkDescriptorType type,
		U32 binding,
		VkDescriptorImageInfo* imageInfo,
		U32 descriptorCount = 1)
	{
		VkWriteDescriptorSet writeDescriptorSet{};
		writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSet.dstSet = dstSet;
		writeDescriptorSet.descriptorType = type;
		writeDescriptorSet.dstBinding = binding;
		writeDescriptorSet.pImageInfo = imageInfo;
		writeDescriptorSet.descriptorCount = descriptorCount;
		return writeDescriptorSet;
	}

















	inline VkPipelineTessellationStateCreateInfo pipelineTessellationStateCreateInfo(U32 patchControlPoints)
	{
		VkPipelineTessellationStateCreateInfo pipelineTessellationStateCreateInfo{};
		pipelineTessellationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
		pipelineTessellationStateCreateInfo.patchControlPoints = patchControlPoints;
		return pipelineTessellationStateCreateInfo;
	}


	inline VkBindSparseInfo bindSparseInfo()
	{
		VkBindSparseInfo ret{};
		ret.sType = VK_STRUCTURE_TYPE_BIND_SPARSE_INFO;
		return ret;
	}

	/** @brief Initialize a map entry for a shader specialization constant */
	inline VkSpecializationMapEntry specializationMapEntry(U32 constantID, U32 offset, size_t size)
	{
		VkSpecializationMapEntry ret{};
		ret.constantID = constantID;
		ret.offset = offset;
		ret.size = size;
		return ret;
	}

	/** @brief Initialize a specialization constant info structure to pass to a shader stage */
	inline VkSpecializationInfo specializationInfo(U32 mapEntryCount, const VkSpecializationMapEntry* mapEntries, size_t dataSize, const void* data)
	{
		VkSpecializationInfo ret{};
		ret.mapEntryCount = mapEntryCount;
		ret.pMapEntries = mapEntries;
		ret.dataSize = dataSize;
		ret.pData = data;
		return ret;
	}

	/** @brief Initialize a specialization constant info structure to pass to a shader stage */
	inline VkSpecializationInfo specializationInfo(const TVector<VkSpecializationMapEntry>& mapEntries, size_t dataSize, const void* data)
	{
		VkSpecializationInfo ret{};
		ret.mapEntryCount = static_cast<U32>(mapEntries.size());
		ret.pMapEntries = mapEntries.data();
		ret.dataSize = dataSize;
		ret.pData = data;
		return ret;
	}

	// Ray tracing related
	inline VkAccelerationStructureGeometryKHR accelerationStructureGeometryKHR()
	{
		VkAccelerationStructureGeometryKHR ret{};
		ret.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
		return ret;
	}

	inline VkAccelerationStructureBuildGeometryInfoKHR accelerationStructureBuildGeometryInfoKHR()
	{
		VkAccelerationStructureBuildGeometryInfoKHR ret{};
		ret.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
		return ret;
	}

	inline VkAccelerationStructureBuildSizesInfoKHR accelerationStructureBuildSizesInfoKHR()
	{
		VkAccelerationStructureBuildSizesInfoKHR ret{};
		ret.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
		return ret;
	}

	inline VkRayTracingShaderGroupCreateInfoKHR rayTracingShaderGroupCreateInfoKHR()
	{
		VkRayTracingShaderGroupCreateInfoKHR ret{};
		ret.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
		return ret;
	}

	inline VkRayTracingPipelineCreateInfoKHR rayTracingPipelineCreateInfoKHR()
	{
		VkRayTracingPipelineCreateInfoKHR ret{};
		ret.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
		return ret;
	}

	inline VkWriteDescriptorSetAccelerationStructureKHR writeDescriptorSetAccelerationStructureKHR()
	{
		VkWriteDescriptorSetAccelerationStructureKHR ret{};
		ret.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
		return ret;
	}
}