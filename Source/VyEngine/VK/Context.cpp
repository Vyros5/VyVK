#include <VyEngine/VK/Context.h>

#include <iostream>
#include <VyLib/Common/AnsiColor.h>
#include <VyLib/Util/String.h>

namespace Vy
{
    // ============================================================================================
    // Deletion Queue

	DeletionQueue::~DeletionQueue()
	{
		flushAll();
	}


	void DeletionQueue::schedule(TFunction<void()>&& function)
	{
		m_PendingDeletions[ m_CurrentFrameIndex ].Deletors.emplace_back( std::move( function ) );
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
			flush( i );
		}
	}

    // ============================================================================================
    // Context

	VyContext& VyContext::initialize(VyWindow& window)
	{
		auto& context = VyContext::get();

		context.m_Device.initialize( window );

		// TODO: Let the pool grow dynamically (see: https://vkguide.dev/docs/extra-chapter/abstracting_descriptors/)
		// https://github.com/TNtube/Cardia/blob/6fbde85b58bac3921ed7d12624e896750686b2db/Cardia/include/Cardia/Renderer/Descriptors.hpp

		context.m_GlobalDescriptorPool = VyDescriptorPool::Builder{}
			.setMaxSets(1000)
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
			.buildPtr();

		return context;
	}


	VyContext& VyContext::get()
	{
		static VyContext s_Instance;
		
		return s_Instance;
	}


	void VyContext::destroy()
	{
		auto& context = VyContext::get();

		context.printMemoryStats();

		context.m_DeletionQueue.flushAll();
	}


	void VyContext::destroy(VkBuffer buffer, VmaAllocation allocation)
	{
		if (buffer)
		{
			VY_ASSERT(allocation, "Buffer and allocation must be valid");

			VyContext::get().m_DeletionQueue.schedule([ = ]() 
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
			
			VyContext::get().m_DeletionQueue.schedule([ = ]()
			{
				vmaDestroyImage(VyContext::get().m_Device.allocator(), image, allocation);
			});
		}
	}
    

	void VyContext::destroy(VkImageView view)
	{
		if (view)
		{
			VyContext::get().m_DeletionQueue.schedule([ = ]()
			{
				vkDestroyImageView(VyContext::get().m_Device.handle(), view, nullptr);
			});
		}
	}


	void VyContext::destroy(VkSampler sampler)
	{
		if (sampler)
		{
			VyContext::get().m_DeletionQueue.schedule([ = ]()
			{
				vkDestroySampler(VyContext::get().m_Device.handle(), sampler, nullptr);
			});
		}
	}
	

	void VyContext::destroy(VkPipeline pipeline)
	{
		if (pipeline)
		{
			VyContext::get().m_DeletionQueue.schedule([ = ]()
			{
				vkDestroyPipeline(VyContext::get().m_Device.handle(), pipeline, nullptr);
			});
		}
	}


	void VyContext::destroy(VkPipelineLayout pipelineLayout)
	{
		if (pipelineLayout)
		{
			VyContext::get().m_DeletionQueue.schedule([ = ]()
			{
				vkDestroyPipelineLayout(VyContext::get().m_Device.handle(), pipelineLayout, nullptr);
			});
		}
	}





	void VyContext::flushDeletionQueue(U32 frameIndex)
	{
		VyContext::get().m_DeletionQueue.flush( frameIndex );
	}



	bool VyContext::allocateSet(VkDescriptorSetLayout layout, VkDescriptorSet& set)
	{
		return VyContext::get().m_GlobalDescriptorPool->allocate( layout, set );
	}


	void VyContext::releaseSets(TVector<VkDescriptorSet>& sets)
	{
		VyContext::get().m_GlobalDescriptorPool->freeDescriptors( sets );
	}


	void VyContext::cleanup()
	{
		VyContext::get().m_GlobalDescriptorPool.reset();
	}


	void VyContext::waitIdle()
	{
		vkDeviceWaitIdle(VyContext::get().m_Device);
	}


	void VyContext::printMemoryStats()
	{
		auto& context = VyContext::get();

		MemoryStats stats = context.getStats();

		std::stringstream ss;
        ss  << "\n--------------------------------------------------------------------------" << '\n'
			<< "[" << CYAN "Vulkan Memory Stats" RESET "] "                                   << '\n'
            << " - Total Allocated Bytes : " << Utils::formatBytes(stats.TotalAllocatedBytes) << '\n'
            << " - Total Available Bytes : " << Utils::formatBytes(stats.TotalAvailableBytes) << '\n'
			<< " - Total Block Bytes     : " << Utils::formatBytes(stats.TotalBlockBytes)     << '\n'
			<< " - Heap Count            : " << stats.HeapStats.size()                        << '\n'
			<< "\n--------------------------------------------------------------------------" << '\n'
        ;

		// ss  	<< " ------------------------------------" << '\n';
		// for (U32 i = 0; i < stats.HeapStats.size(); i++)
		// {
		// 	ss  << "  - [Heap " << i + 1 << "]"                                                           << '\n'
		// 		<< "    - BlockCount      : " <<                    stats.HeapStats[ i ].BlockCount       << '\n'
		// 		<< "    - BlockBytes      : " << Utils::formatBytes(stats.HeapStats[ i ].BlockBytes)      << '\n'
		// 		<< "    - AllocationCount : " <<                    stats.HeapStats[ i ].AllocationCount  << '\n'
		// 		<< "    - AllocationBytes : " << Utils::formatBytes(stats.HeapStats[ i ].AllocationBytes) << '\n'
		// 		<< " ------------------------------------" << '\n'
		// 		<< '\n'
		// 	;
		// }

		std::cout << ss.str() << std::endl;
	}


	TVector<MemoryBudget> VyContext::getBudget()
	{
        VkPhysicalDeviceMemoryProperties memProps;
        vkGetPhysicalDeviceMemoryProperties(static_cast<VkPhysicalDevice>(VyContext::get().physicalDevice()), &memProps);

		TVector<VmaBudget> vmaBudgets( memProps.memoryHeapCount );
		vmaGetHeapBudgets(VyContext::get().allocator(), vmaBudgets.data());

		TVector<MemoryBudget> budgets{};
		budgets.reserve( memProps.memoryHeapCount );

		for (U32 i = 0; i < memProps.memoryHeapCount; ++i)
		{
			budgets.push_back({
				.BlockBytes      = vmaBudgets[ i ].statistics.blockBytes,
				.AllocationBytes = vmaBudgets[ i ].statistics.allocationBytes,
				.Usage           = vmaBudgets[ i ].usage,
				.Budget          = vmaBudgets[ i ].budget
			});
		}

		return budgets;
	}


	MemoryStats VyContext::getStats()
	{
		VmaTotalStatistics stats;
		vmaCalculateStatistics(VyContext::get().allocator(), &stats);
		
		MemoryStats result{};
		result.TotalAllocatedBytes = stats.total.statistics.allocationBytes;
		result.TotalAvailableBytes = 0;

        VkPhysicalDeviceMemoryProperties memProps{};
        vkGetPhysicalDeviceMemoryProperties(static_cast<VkPhysicalDevice>(VyContext::get().physicalDevice()), &memProps);
    
		const TVector<MemoryBudget> budgets = getBudget();

		for (const auto& budget : budgets)
		{
			result.TotalAvailableBytes += budget.Budget;
			result.TotalBlockBytes     += budget.BlockBytes;
		}

		result.HeapStats.reserve( VK_MAX_MEMORY_HEAPS );

		for (U32 i = 0; i < memProps.memoryHeapCount; ++i)
		{
			const VmaStatistics& heapStats = stats.memoryHeap[ i ].statistics;

			result.HeapStats.push_back({
				.BlockCount      = heapStats.blockCount,
				.AllocationCount = heapStats.allocationCount,
				.BlockBytes      = heapStats.blockBytes,
				.AllocationBytes = heapStats.allocationBytes
			});
		}

		return result;
	}
}