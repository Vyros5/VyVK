#include <VyEngine/VK/Core/Test.h>

#include <VyEngine/VK/Context.h>

namespace Vy
{
    ImmediateQueue::ImmediateQueue()
    {
        initialize();
    }

    
    ImmediateQueue::~ImmediateQueue()
    {
        cleanup();
    }


    void ImmediateQueue::submitGraphics(const TFunction<void(VkCommandBuffer)>& function)
    {
        TUniqueLock<TMutex> submitLock( m_GraphicsMutex );

        VK_CHECK_SUCCESS(vkResetFences(VyContext::device(), 1, &m_GraphicsFence), 
            "Couldn't reset immediate fence");

        VK_CHECK_SUCCESS(vkResetCommandBuffer(m_GraphicsCommandBuffer, 0), 
            "Couldn't reset immediate command buffer");

        VkCommandBufferBeginInfo beginInfo{ VKInit::commandBufferBeginInfo() };
        {
            beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
            beginInfo.pInheritanceInfo = nullptr;
        }

        VK_CHECK_SUCCESS(vkBeginCommandBuffer(m_GraphicsCommandBuffer, &beginInfo), 
            "Failed to begin recording command buffer!");

        function( m_GraphicsCommandBuffer );

        VK_CHECK_SUCCESS(vkEndCommandBuffer(m_GraphicsCommandBuffer), 
            "Failed to end command buffer!");

        VkCommandBufferSubmitInfo commandBufferSubmitInfo{ VKInit::commandBufferSubmitInfo() };
        {
            commandBufferSubmitInfo.commandBuffer = m_GraphicsCommandBuffer;
            commandBufferSubmitInfo.deviceMask    = 0;
            commandBufferSubmitInfo.pNext         = nullptr;
        }

        VkSubmitInfo2 submitInfo{ VKInit::submitInfo2() };
        {
            submitInfo.pNext                  = nullptr;
            submitInfo.commandBufferInfoCount = 1;
            submitInfo.pCommandBufferInfos    = &commandBufferSubmitInfo;
        }

        TUniqueLock<TMutex> queueLock( VulkanMutex::GraphicsQueueSubmitMutex );

        VK_CHECK_SUCCESS(vkQueueSubmit2(VyContext::device().graphicsQueue(), 1, &submitInfo, m_GraphicsFence), 
            "Failed to submit immediate command to queue!");

        queueLock.unlock();

        VK_CHECK_SUCCESS(vkWaitForFences(VyContext::device(), 1, &m_GraphicsFence, true, UINT64_MAX), 
            "Failed to wait for immediate fence");
    }


    void ImmediateQueue::submitGraphics(TSpan<VkCommandBufferSubmitInfo> commandBufferSubmitInfos)
    {
        if (commandBufferSubmitInfos.size() == 0)
            return;

        TUniqueLock<TMutex> submitLock( m_GraphicsMutex );

        VK_CHECK_SUCCESS(vkResetFences(VyContext::device(), 1, &m_GraphicsFence), 
            "Couldn't reset immediate fence");

        VkSubmitInfo2 submitInfo{ VKInit::submitInfo2() };
        {
            submitInfo.pNext                  = nullptr;
            submitInfo.commandBufferInfoCount = static_cast<U32>(commandBufferSubmitInfos.size());
            submitInfo.pCommandBufferInfos    = commandBufferSubmitInfos.data();
        }

        TUniqueLock<TMutex> queueLock( VulkanMutex::GraphicsQueueSubmitMutex );
        
        VK_CHECK_SUCCESS(vkQueueSubmit2(VyContext::device().graphicsQueue(), 1, &submitInfo, m_GraphicsFence), 
            "Failed to submit immediate command to queue!");

        queueLock.unlock();

        VK_CHECK_SUCCESS(vkWaitForFences(VyContext::device(), 1, &m_GraphicsFence, true, UINT64_MAX), 
            "Failed to wait for immediate fence");
    }


    void ImmediateQueue::submitTransfer(const TFunction<void(VkCommandBuffer)>& function)
    {
        TUniqueLock<TMutex> submitLock(m_TransferMutex);

        VK_CHECK_SUCCESS(vkResetFences(VyContext::device(), 1, &m_TransferFence), 
            "Couldn't reset immediate fence");

        VK_CHECK_SUCCESS(vkResetCommandBuffer(m_TransferCommandBuffer, 0), 
            "Couldn't reset immediate command buffer");

        VkCommandBufferBeginInfo beginInfo{ VKInit::commandBufferBeginInfo() };
        {
            beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
            beginInfo.pInheritanceInfo = nullptr;
        }

        VK_CHECK_SUCCESS(vkBeginCommandBuffer(m_TransferCommandBuffer, &beginInfo), 
            "Failed to begin recording command buffer!");

        function( m_TransferCommandBuffer );

        VK_CHECK_SUCCESS(vkEndCommandBuffer(m_TransferCommandBuffer), 
            "Failed to end command buffer!");

        VkCommandBufferSubmitInfo commandBufferSubmitInfo{ VKInit::commandBufferSubmitInfo() };
        {
            commandBufferSubmitInfo.commandBuffer = m_TransferCommandBuffer;
            commandBufferSubmitInfo.deviceMask    = 0;
            commandBufferSubmitInfo.pNext         = nullptr;
        }

        VkSubmitInfo2 submitInfo{ VKInit::submitInfo2() };
        {
            submitInfo.pNext                  = nullptr;
            submitInfo.commandBufferInfoCount = 1;
            submitInfo.pCommandBufferInfos    = &commandBufferSubmitInfo;
        }

        TUniqueLock<TMutex> queueLock( VulkanMutex::TransferQueueSubmitMutex );

        VK_CHECK_SUCCESS(vkQueueSubmit2(VyContext::device().transferQueue(), 1, &submitInfo, m_TransferFence), 
            "Failed to submit immediate command to queue!");
        
        queueLock.unlock();

        VK_CHECK_SUCCESS(vkWaitForFences(VyContext::device(), 1, &m_TransferFence, true, UINT64_MAX), 
            "Failed to wait for immediate fence");
    }


    void ImmediateQueue::initialize()
    {
        auto queueFamilyIndices = VyContext::device().findQueueFamilies();

        initResource(m_GraphicsFence, m_GraphicsCommandPool, m_GraphicsCommandBuffer, queueFamilyIndices.GraphicsFamily.value());
        initResource(m_TransferFence, m_TransferCommandPool, m_TransferCommandBuffer, queueFamilyIndices.TransferFamily.value());
    }


    void ImmediateQueue::cleanup()
    {
        if (m_GraphicsCommandPool != VK_NULL_HANDLE)
        {
            vkDestroyCommandPool(VyContext::device(), m_GraphicsCommandPool, nullptr);
            m_GraphicsCommandPool = VK_NULL_HANDLE;
        }

        if (m_GraphicsFence != VK_NULL_HANDLE)
        {
            vkDestroyFence(VyContext::device(), m_GraphicsFence, nullptr);
            m_GraphicsFence = VK_NULL_HANDLE;
        }

        if (m_TransferCommandPool != VK_NULL_HANDLE)
        {
            vkDestroyCommandPool(VyContext::device(), m_TransferCommandPool, nullptr);
            m_TransferCommandPool = VK_NULL_HANDLE;
        }

        if (m_TransferFence != VK_NULL_HANDLE)
        {
            vkDestroyFence(VyContext::device(), m_TransferFence, nullptr);
            m_TransferFence = VK_NULL_HANDLE;
        }
    }


    void ImmediateQueue::initResource(VkFence& fence, VkCommandPool& commandPool, VkCommandBuffer& commandBuffer, U32 queueFamilyIndex)
    {
        VkCommandPoolCreateInfo poolInfo{ VKInit::commandPoolCreateInfo() };
        {
            poolInfo.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
            poolInfo.queueFamilyIndex = queueFamilyIndex;
        }

        VK_CHECK_SUCCESS(vkCreateCommandPool(VyContext::device(), &poolInfo, nullptr, &commandPool), 
            "Failed to create immediate command pool!");

        VkCommandBufferAllocateInfo allocInfo{ VKInit::commandBufferAllocateInfo() };
        {
            allocInfo.commandPool        = commandPool;
            allocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            allocInfo.commandBufferCount = 1;
        }

        VK_CHECK_SUCCESS(vkAllocateCommandBuffers(VyContext::device(), &allocInfo, &commandBuffer), 
            "Failed to allocate immediate command buffers!");

        VkFenceCreateInfo fenceInfo{ VKInit::fenceCreateInfo( VK_FENCE_CREATE_SIGNALED_BIT ) };

        VK_CHECK_SUCCESS(vkCreateFence(VyContext::device(), &fenceInfo, nullptr, &fence), 
            "Failed to create fence!");
    }
}