#include <Vy/GFX/Context.h>


namespace Vy
{
    // ============================================================================================
    // Deletion Queue

	DeletionQueue::~DeletionQueue()
	{
		flushAll();
	}


	void DeletionQueue::schedule(Function<void()>&& function)
	{
		m_PendingDeletions[ m_CurrentFrameIndex ].Deletors.emplace_back( std::move(function) );
	}


	void DeletionQueue::flush(U32 frameIndex)
	{
		m_CurrentFrameIndex = frameIndex;

		auto& deletors = m_PendingDeletions[ frameIndex ].Deletors;
        
		for (auto& DeleteFunc : deletors)
		{
			if (DeleteFunc)
            {
                DeleteFunc();
            }
		}

		deletors.clear();
	}


	void DeletionQueue::flushAll()
	{
		for (U32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			flush(i);
		}
	}

    // ============================================================================================
    // Context

	VyContext& VyContext::initialize(VyWindow& window)
	{
		auto& context = get();

		context.m_Device.initialize(window);

		// TODO: Let the pool grow dynamically (see: https://vkguide.dev/docs/extra-chapter/abstracting_descriptors/)
		// https://github.com/TNtube/Cardia/blob/6fbde85b58bac3921ed7d12624e896750686b2db/Cardia/include/Cardia/Renderer/Descriptors.hpp

		context.m_GlobalPool = VyDescriptorPool::Builder{}
			.setMaxSets(MAX_DESCRIPTOR_SETS)
			.setPoolFlags(VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT)
			.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         1000)
			.addPoolSize(VK_DESCRIPTOR_TYPE_SAMPLER,                 500)
			.addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4000)
			.addPoolSize(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,          4000)
			.addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,          1000)
			.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,   1000)
			.addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,   1000)
			.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         2000)
			.addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,         2000)
			.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000)
			.addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000)
			.addPoolSize(VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,        500)
		.buildUnique();

		return context;
	}


	void VyContext::destroy()
	{
		auto& context = get();
		
		context.m_DeletionQueue.flushAll();
	}


	VyContext& VyContext::get()
	{
		static VyContext s_Instance;
		
		return s_Instance;
	}


	void VyContext::destroy(VkBuffer buffer, VmaAllocation allocation)
	{
		if (buffer)
		{
			VY_ASSERT(allocation, "Buffer and allocation must be valid");

			VyContext::get().m_DeletionQueue.schedule([=]() 
			{ 
				vmaDestroyBuffer(VyContext::get().m_Device.allocator(), buffer, allocation);
			});
		}
	}


	void VyContext::destroy(VkImage image, VmaAllocation allocation)
	{
		if (image)
		{
			VY_ASSERT(allocation, "Image and allocation must be valid");
			
			VyContext::get().m_DeletionQueue.schedule([=]()
			{
				vmaDestroyImage(VyContext::get().m_Device.allocator(), image, allocation);
			});
		}
	}
    

	void VyContext::destroy(VkImageView view)
	{
		if (view)
		{
			VyContext::get().m_DeletionQueue.schedule([=]()
			{
				vkDestroyImageView(VyContext::get().m_Device.handle(), view, nullptr);
			});
		}
	}


	void VyContext::destroy(VkSampler sampler)
	{
		if (sampler)
		{
			VyContext::get().m_DeletionQueue.schedule([=]()
			{
				vkDestroySampler(VyContext::get().m_Device.handle(), sampler, nullptr);
			});
		}
	}
	

	void VyContext::destroy(VkPipeline pipeline)
	{
		if (pipeline)
		{
			VyContext::get().m_DeletionQueue.schedule([=]()
			{
				vkDestroyPipeline(VyContext::get().m_Device.handle(), pipeline, nullptr);
			});
		}
	}


	void VyContext::destroy(VkPipelineLayout pipelineLayout)
	{
		if (pipelineLayout)
		{
			VyContext::get().m_DeletionQueue.schedule([=]()
			{
				vkDestroyPipelineLayout(VyContext::get().m_Device.handle(), pipelineLayout, nullptr);
			});
		}
	}


	void VyContext::flushDeletionQueue(U32 frameIndex)
	{
		VyContext::get().m_DeletionQueue.flush(frameIndex);
	}



	bool VyContext::allocateSet(VkDescriptorSetLayout layout, VkDescriptorSet& set)
	{
		return get().m_GlobalPool->allocateDescriptorSet(layout, set);
	}


	void VyContext::releaseSets(TVector<VkDescriptorSet>& sets)
	{
		get().m_GlobalPool->freeDescriptors(sets);
	}


	void VyContext::cleanup()
	{
		get().m_GlobalPool.reset();
	}


	void VyContext::waitIdle()
	{
		vkDeviceWaitIdle(get().m_Device);
	}
}