#pragma once

#include <VyLib/VyLib.h>

namespace Vy
{
	struct AllocationStats
	{
		USize TotalAllocated = 0;
		USize TotalFreed     = 0;
	};

	struct Allocation
	{
		void*       Memory   = 0;
		USize       Size     = 0;
		const char* Category = 0;
	};

	namespace Memory 
    {
		const AllocationStats& getAllocationStats();
	}

	template <class T>
	struct Mallocator
	{
		typedef T value_type;

		Mallocator() = default;

		template <class U> 
        constexpr Mallocator(const Mallocator <U>&) noexcept 
        {
        }


		T* allocate(USize n)
		{
			if (n > std::numeric_limits<USize>::max() / sizeof(T))
            {
                throw std::bad_array_new_length();
            }

			if (auto p = static_cast<T*>(std::malloc(n * sizeof(T))))
			{
				return p;
			}

			throw std::bad_alloc();
		}


		void deallocate(T* p, USize n) noexcept
		{
			std::free(p);
		}
	};

	struct AllocatorData
	{
		using MapAlloc      = Mallocator<std::pair<const void* const, Allocation>>;
		using StatsMapAlloc = Mallocator<std::pair<const char* const, AllocationStats>>;

        using AllocationMap      = std::map<const void*, Allocation, std::less<>, MapAlloc>;
		using AllocationStatsMap = std::map<const char*, AllocationStats, std::less<const char*>, StatsMapAlloc>;

		AllocationMap      m_AllocationMap;
		AllocationStatsMap m_AllocationStatsMap;

		std::mutex m_Mutex, m_StatsMutex;
	};


	class Allocator
	{
	public:
		static void init();

		static void* allocateRaw(USize size);

		static void* allocate(USize size);
		static void* allocate(USize size, const char* desc);
		static void* allocate(USize size, const char* file, int line);
		static void  free(void* memory);

		static const AllocatorData::AllocationStatsMap& getAllocationStats() 
        { 
            return s_Data->m_AllocationStatsMap; 
        }

	private:
		inline static AllocatorData* s_Data = nullptr;
	};


	struct STLAllocator
	{
		STLAllocator() = default;
		STLAllocator(const char* name)
			: Name(name)
		{

		}

		static void* allocate(USize size, int /*flags*/ = 0)
		{
			return Allocator::allocate(size);
		}

		static void deallocate(void* memory, USize /*size*/ = 0)
		{
			Allocator::free(memory);
		}

		static void* allocate(USize size, char const* file, int line, unsigned int type, char const* function, int blockType)
		{
			return Allocator::allocate(size, file);
		}

		static void* allocate(unsigned __int64 size, unsigned __int64 alignment, unsigned __int64 offset, char const* file, int line, unsigned int type, char const* function, int blockType)
		{
			return Allocator::allocate(size, file);
		}

		static void* allocate(unsigned __int64 size, unsigned __int64 alignment, unsigned __int64 offset = 0, int flags = 0)
		{
			return Allocator::allocate(size);
		}

		// Define equality operator
		bool operator==(const STLAllocator&) const noexcept
		{
			// Since EASTL allocators are typically stateless, they are often considered equal
			return true;
		}

		// Define inequality operator
		bool operator!=(const STLAllocator&) const noexcept
		{
			return false;
		}


		// Optional: Track debug information
		const char* get_name() const { return Name; }
		void set_name(const char* name) { Name = name; }

		const char* Name = "MyEASTLAllocator";
	};


}

_NODISCARD _Ret_notnull_ _Post_writable_byte_size_(size) _VCRT_ALLOCATOR
void* __cdecl operator new[](size_t size, char const* file, int line, unsigned int type, char const* function, int blockType); // For EASTL

_NODISCARD _Ret_notnull_ _Post_writable_byte_size_(size) _VCRT_ALLOCATOR
void* __cdecl operator new[](unsigned __int64 size, unsigned __int64 alignment, unsigned __int64 offset, char const* file, int line, unsigned int type, char const* function, int blockType); // for EASTL

#undef BEY_TRACK_MEMORY
#if BEY_TRACK_MEMORY

#if VY_PLATFORM_WINDOWS

_NODISCARD _Ret_notnull_ _Post_writable_byte_size_(size) _VCRT_ALLOCATOR
void* __CRTDECL operator new(size_t size);

_NODISCARD _Ret_notnull_ _Post_writable_byte_size_(size) _VCRT_ALLOCATOR
void* __CRTDECL operator new[](size_t size);

_NODISCARD _Ret_notnull_ _Post_writable_byte_size_(size) _VCRT_ALLOCATOR
void* __CRTDECL operator new(size_t size, const char* desc);

_NODISCARD _Ret_notnull_ _Post_writable_byte_size_(size) _VCRT_ALLOCATOR
void* __CRTDECL operator new[](size_t size, const char* desc);

_NODISCARD _Ret_notnull_ _Post_writable_byte_size_(size) _VCRT_ALLOCATOR
void* __CRTDECL operator new(size_t size, const char* file, int line);

_NODISCARD _Ret_notnull_ _Post_writable_byte_size_(size) _VCRT_ALLOCATOR
void* __CRTDECL operator new[](size_t size, const char* file, int line);



void __CRTDECL operator delete(void* memory);
void __CRTDECL operator delete(void* memory, const char* desc);
void __CRTDECL operator delete(void* memory, const char* file, int line);
void __CRTDECL operator delete[](void* memory);
void __CRTDECL operator delete[](void* memory, const char* desc);
void __CRTDECL operator delete[](void* memory, const char* file, int line);

#define hnew new(__FILE__, __LINE__)
#define hdelete delete

#else
#warning "Memory tracking not available on non-Windows platform"
#define hnew new
#define hdelete delete

#endif

#else

#define hnew new
#define hdelete delete

#endif