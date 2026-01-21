#include <VyEngine/VK/Core/VKCore.h>

#include <VyEngine/VK/Core/Include/vkImpl.h>
#include <VyEngine/VK/Context.h>

namespace Vy
{

#pragma region [ VKDbg ]

	void VKDbg::cmdBeginLabel(VkCommandBuffer cmdBuffer, const char* labelName, Vec4 color)
	{
	#ifdef VK_RENDERER_DEBUG
		VkDebugUtilsLabelEXT label = { VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT };
		label.pLabelName           = labelName;
		memcpy(label.color, &color, sizeof(float) * 4);

		vkCmdBeginDebugUtilsLabelEXT(cmdBuffer, &label);
	#endif
	}


	void VKDbg::cmdInsertLabel(VkCommandBuffer cmdBuffer, const char* labelName, Vec4 color)
	{
	#ifdef VK_RENDERER_DEBUG
		VkDebugUtilsLabelEXT label = { VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT };
		label.pLabelName           = labelName;
		memcpy(label.color, &color, sizeof(float) * 4);

		vkCmdInsertDebugUtilsLabelEXT(cmdBuffer, &label);
	#endif
	}


	void VKDbg::cmdEndLabel(VkCommandBuffer cmdBuffer)
	{
	#ifdef VK_RENDERER_DEBUG
		vkCmdEndDebugUtilsLabelEXT(cmdBuffer);
	#endif
	}


	void VKDbg::queueBeginLabel(VkQueue queue, const char* labelName, Vec4 color)
	{
	#ifdef VK_RENDERER_DEBUG
		VkDebugUtilsLabelEXT label = { VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT };
		label.pLabelName           = labelName;
		memcpy(label.color, &color, sizeof(float) * 4);

		vkQueueBeginDebugUtilsLabelEXT(queue, &label);
	#endif
	}


	void VKDbg::queueInsertLabel(VkQueue queue, const char* labelName, Vec4 color)
	{
	#ifdef VK_RENDERER_DEBUG
		VkDebugUtilsLabelEXT label = { VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT };
		label.pLabelName           = labelName;
		memcpy(label.color, &color, sizeof(float) * 4);

		vkQueueInsertDebugUtilsLabelEXT(queue, &label);
	#endif
	}


	void VKDbg::queueEndLabel(VkQueue queue)
	{
	#ifdef VK_RENDERER_DEBUG
		vkQueueEndDebugUtilsLabelEXT(queue);
	#endif
	}


	void VKDbg::setObjectName(VkObjectType objectType, U64 objectHandle, const char* objectName)
	{
	#ifdef VK_RENDERER_DEBUG
		VkDebugUtilsObjectNameInfoEXT nameInfo{ VKInit::debugUtilsObjectNameInfoEXT() };
		{
			nameInfo.objectType   = objectType;
			nameInfo.objectHandle = objectHandle;
			nameInfo.pObjectName  = objectName;
		}

		vkSetDebugUtilsObjectNameEXT(VyContext::device(), &nameInfo);
	#endif
	}


	void VKDbg::setObjectName(VkBuffer object, const char* objectName)
	{
		VKDbg::setObjectName(VK_OBJECT_TYPE_BUFFER, (U64)object, objectName);
	}

	void VKDbg::setObjectName(VkImage object, const char* objectName)
	{
		VKDbg::setObjectName(VK_OBJECT_TYPE_IMAGE, (U64)object, objectName);
	}

	void VKDbg::setObjectName(VkImageView object, const char* objectName)
	{
		VKDbg::setObjectName(VK_OBJECT_TYPE_IMAGE_VIEW, (U64)object, objectName);
	}

	void VKDbg::setObjectName(VkSampler object, const char* objectName)
	{
		VKDbg::setObjectName(VK_OBJECT_TYPE_SAMPLER, (U64)object, objectName);
	}

	void VKDbg::setObjectName(VkRenderPass object, const char* objectName)
	{
		VKDbg::setObjectName(VK_OBJECT_TYPE_RENDER_PASS, (U64)object, objectName);
	}

	void VKDbg::setObjectName(VkFramebuffer object, const char* objectName)
	{
		VKDbg::setObjectName(VK_OBJECT_TYPE_FRAMEBUFFER, (U64)object, objectName);
	}

	void VKDbg::setObjectName(VkPipeline object, const char* objectName)
	{
		VKDbg::setObjectName(VK_OBJECT_TYPE_PIPELINE, (U64)object, objectName);
	}

	void VKDbg::setObjectName(VkPipelineLayout object, const char* objectName)
	{
		VKDbg::setObjectName(VK_OBJECT_TYPE_PIPELINE_LAYOUT, (U64)object, objectName);
	}

	void VKDbg::setObjectName(VkDescriptorSet object, const char* objectName)
	{
		VKDbg::setObjectName(VK_OBJECT_TYPE_DESCRIPTOR_SET, (U64)object, objectName);
	}

	void VKDbg::setObjectName(VkDescriptorPool object, const char* objectName)
	{
		VKDbg::setObjectName(VK_OBJECT_TYPE_DESCRIPTOR_POOL, (U64)object, objectName);
	}

	void VKDbg::setObjectName(VkDescriptorSetLayout object, const char* objectName)
	{
		VKDbg::setObjectName(VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT, (U64)object, objectName);
	}

	void VKDbg::setObjectName(VkSemaphore object, const char* objectName)
	{
		VKDbg::setObjectName(VK_OBJECT_TYPE_SEMAPHORE, (U64)object, objectName);
	}

	void VKDbg::setObjectName(VkFence object, const char* objectName)
	{
		VKDbg::setObjectName(VK_OBJECT_TYPE_FENCE, (U64)object, objectName);
	}



	// void VKDbg::setObjectTag(VkObjectType objectType, U64 objectHandle, U64 tagId, void* tag, size_t tagSize)
	// {
	// #ifdef VK_RENDERER_DEBUG
	// 	VkDebugUtilsObjectTagInfoEXT tagInfo = { VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_TAG_INFO_EXT };
	// 	tagInfo.objectType                   = objectType;
	// 	tagInfo.objectHandle                 = objectHandle;
	// 	tagInfo.tagName                      = tagId;
	// 	tagInfo.tagSize                      = tagSize;
	// 	tagInfo.pTag                         = tag;

	// 	vkSetDebugUtilsObjectTagEXT(VyContext::device(), &tagInfo);
	// #endif
	// }

#pragma endregion
}