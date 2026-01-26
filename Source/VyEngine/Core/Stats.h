#pragma once

#include <VyEngine/VK/Device/Device.h>

namespace Vy
{
	struct MemoryStats
	{
		struct HeapStats
		{
			U64 BlockCount;
			U64 AllocationCount;
			U64 BlockBytes;
			U64 AllocationBytes;
		};

		U64                TotalAllocatedBytes;
		U64                TotalAvailableBytes;
		U64                TotalBlockBytes;
		TVector<HeapStats> HeapStats;
	};

	struct MemoryBudget
	{
		U64 BlockBytes;
		U64 AllocationBytes;
		U64 Usage;
		U64 Budget;
	};


    class VyStats
    {
    public:
        VyStats() = default;
        ~VyStats() = default;

		MemoryStats           getMemoryStats();
		TVector<MemoryBudget> getMemoryBudget();
        
		void print();
        
        void nextFrame();

    private:
    
        U32 m_FrameCount{ 0u };
    };
}