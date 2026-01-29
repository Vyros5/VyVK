#include <VyEngine/Core/Stats.h>

#include <VyEngine/VK/Context.h>

#include <iostream>
#include <VyLib/Common/AnsiColor.h>
#include <VyLib/Util/String.h>

namespace Vy
{
    void VyStats::nextFrame()
    {
        m_FrameCount++;
    }
    

	void VyStats::print()
	{
		MemoryStats stats = this->getMemoryStats();

		std::stringstream ss;
        ss  << "\n--------------------------------------------------------------------------" << '\n'
			<< "[" << CYAN "VyEngine Stats" RESET "] "                                        << '\n'
            << " - Total Frames          : " << m_FrameCount                                  << '\n'
            << " - Total Allocated Bytes : " << Utils::formatBytes(stats.TotalAllocatedBytes) << '\n'
            << " - Total Available Bytes : " << Utils::formatBytes(stats.TotalAvailableBytes) << '\n'
			<< " - Total Block Bytes     : " << Utils::formatBytes(stats.TotalBlockBytes)     << '\n'
			<< " - Heap Count            : " << stats.HeapStats.size()                        << '\n'
			<< "--------------------------------------------------------------------------" << '\n'
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


	TVector<MemoryBudget> VyStats::getMemoryBudget()
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


	MemoryStats VyStats::getMemoryStats()
	{
		VmaTotalStatistics stats;
		vmaCalculateStatistics(VyContext::get().allocator(), &stats);
		
		MemoryStats result{};
		result.TotalAllocatedBytes = stats.total.statistics.allocationBytes;
		result.TotalAvailableBytes = 0;

        VkPhysicalDeviceMemoryProperties memProps{};
        vkGetPhysicalDeviceMemoryProperties(static_cast<VkPhysicalDevice>(VyContext::get().physicalDevice()), &memProps);
    
		const TVector<MemoryBudget> budgets = this->getMemoryBudget();

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