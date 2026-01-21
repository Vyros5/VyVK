#pragma once

#include <VyEngine/VK/Core/VKCore.h>
#include <VyLib/STL/Mutex.h>

namespace Vy
{
    class VulkanMutex
    {
    public:
        static inline TMutex GraphicsQueueSubmitMutex;
        static inline TMutex TransferQueueSubmitMutex;
    };
    // https://github.com/TamasPetii/SynapseEngine/blob/5571fde8c456c1d50dc8f41c285bde6ba06c261f/Engine/Vulkan/ImmediateQueue.h

	class ImmediateQueue
	{
	public:
		ImmediateQueue();
		~ImmediateQueue();

		void submitGraphics(const TFunction<void(VkCommandBuffer)>& function);
		void submitGraphics(TSpan<VkCommandBufferSubmitInfo> commandBufferSubmitInfos);
		void submitTransfer(const TFunction<void(VkCommandBuffer)>& function);

	private:
		void initialize();
		void cleanup();

		void initResource(VkFence& fence, VkCommandPool& commandPool, VkCommandBuffer& commandBuffer, uint32_t queueFamilyIndex);
	private:
		
		TMutex          m_GraphicsMutex;
		VkFence         m_GraphicsFence = VK_NULL_HANDLE;
		VkCommandPool   m_GraphicsCommandPool = VK_NULL_HANDLE;
		VkCommandBuffer m_GraphicsCommandBuffer = VK_NULL_HANDLE;

		TMutex          m_TransferMutex;
		VkFence         m_TransferFence = VK_NULL_HANDLE;
		VkCommandPool   m_TransferCommandPool = VK_NULL_HANDLE;
		VkCommandBuffer m_TransferCommandBuffer = VK_NULL_HANDLE;
	};
}