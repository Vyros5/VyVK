#pragma once

#include <VyLib/Common/Numeric.h>
#include <VyLib/Core/Assert.h>

namespace Vy
{
    // https://twitter.com/SebAaltonen/status/1534416275828514817
    // https://twitter.com/SebAaltonen/status/1535175559067713536
    /**
     * VyHandle is a weak pointer like reference to real objects inside a Pool, this forms the basis for various handles
     * 
     * Handles are like weak pointers. The container has an array of generation counters, and the data getter checks whether the generation counters match. 
     * If not, then the slot was deleted (and possibly reused). The getter returns null instead of the data pointer in this case
     */
    template<typename T>
    class VyHandle
    {
    public:
        VyHandle() : 
            m_Index(0), 
            m_Generation(0) 
        {
        }

        VyHandle(const VyHandle& handle)
        {
            m_Index      = handle.m_Index;
            m_Generation = handle.m_Generation;
        }

        VyHandle& operator=(const VyHandle& handle)
        {
            m_Index      = handle.m_Index;
            m_Generation = handle.m_Generation;

            return *this;
        }

        VyHandle(VyHandle&& handle)
        {
            m_Index      = handle.m_Index;
            m_Generation = handle.m_Generation;
        }

        VyHandle& operator=(VyHandle&& handle)
        {
            if (this != &handle) 
            {
                m_Index             = handle.m_Index;
                m_Generation        = handle.m_Generation;

                handle.m_Index      = 0;
                handle.m_Generation = 0;
            }

            return *this;
        }

        inline bool isValid() const { return m_Generation != 0; }

        inline U32  getIndex() const    { return m_Index; }
        inline void setIndex(U32 index) { m_Index = index; }

        inline U32  getGeneration() const  { return m_Generation; }
        inline void setGeneration(U32 gen) { m_Generation = gen; }

        inline void destroy()
        {
            m_Generation = 0;
            m_Index      = 0;
        }

        friend bool operator!=(VyHandle const& h1, VyHandle const& h2)
        {
            return h1.m_Index != h2.m_Index;
        }

        friend bool operator==(VyHandle const& h1, VyHandle const& h2)
        {
            return h1.m_Index == h2.m_Index;
        }

    private:
        U32 m_Index      = 0; /* The array Index of the VyHandle in the pool it belongs to    */
        U32 m_Generation = 0; /* Generated index for deletion verification                  */

        /* All the handles are stored in a pool */
        template<typename U>
        friend class VyResourcePoolTyped;
    };






    // https://github.com/Impulse21/Phoenix-Engine
    
	template<typename ImplT, typename HT>
	class VyHandlePool
	{
	public:
		VyHandlePool()
			: m_Size(0)
			, m_NumActiveEntries(0)
			, m_Data(nullptr)
			, m_Generations(nullptr)
			, m_FreeList(nullptr)
		{
		}


		~VyHandlePool()
		{
			finalize();
		}


		void initialize(size_t initCapacity = 16)
		{
			m_Size             = initCapacity;
			m_NumActiveEntries = 0;

			m_Data        = new ImplT[m_Size];
			m_Generations = new U32[m_Size];
			m_FreeList    = new U32[m_Size];

			std::memset(m_Data,        0, m_Size * sizeof(ImplT));
			std::memset(m_Generations, 0, m_Size * sizeof(U32));
			std::memset(m_FreeList,    0, m_Size * sizeof(U32));

			for (size_t i = 0; i < m_Size; i++)
			{
				m_Generations[i] = 1;
			}

			m_FreeListPosition = m_Size - 1;

			for (size_t i = 0; i < m_Size; i++)
			{
				m_FreeList[i] = static_cast<U32>((m_Size - 1) - i);
			}
		}


		void finalize()
		{
			if (m_Data)
			{
				for (int i = 0; i < m_Size; i++)
				{
					m_Data[i].~ImplT();
				}

				delete[] m_Data;
				m_Data = nullptr;
			}

			if (m_FreeList)
			{
				delete[] m_FreeList;
				m_FreeList = nullptr;
			}

			if (m_Generations)
			{
				delete[] m_Generations;
				m_Generations = nullptr;
			}

			m_Size = 0;
		}


		ImplT* get(VyHandle<HT> handle)
		{
			if (!contains(handle))
			{
				return nullptr;
			}

			return m_Data + handle.m_Index;
		}


		bool contains(VyHandle<HT> handle) const
		{
			return
				handle.isValid() &&
				handle.m_Index < m_Size &&
				m_Generations[handle.m_Index] == handle.m_Generation;
		}


		template<typename... Args>
		VyHandle<HT> emplace(Args&&... args)
		{
			if (!hasSpace())
			{
				resize();
			}

			VyHandle<HT> handle;
			// Get a free index
			handle.m_Index      = m_FreeList[m_FreeListPosition];
			handle.m_Generation = m_Generations[handle.m_Index];

			m_FreeListPosition--;
			m_NumActiveEntries++;

			new (m_Data + handle.m_Index) ImplT(std::forward<Args>(args)...);

			return handle;
		}


		VyHandle<HT> insert(ImplT const& Data)
		{
			return emplace(Data);
		}


		void release(VyHandle<HT> handle)
		{
			if (!contains(handle))
			{
				return;
			}

			get(handle)->~ImplT();

			new (m_Data + handle.m_Index) ImplT();
			m_Generations[handle.m_Index] += 1;

			// To prevent the risk of re assignment, block index for being allocated
			if (m_Generations[handle.m_Index] == std::numeric_limits<U32>::max())
			{
				return;
			}

			m_FreeList[++m_FreeListPosition] = handle.m_Index;
			m_NumActiveEntries--;
		}

		bool isEmpty() const { return m_NumActiveEntries == 0; }

	private:
		void resize()
		{

			if (m_Size == 0)
			{
				initialize(16);
				return;
			}

			size_t newSize         = m_Size * 2;
			auto* newDataArray     = new ImplT[newSize];
			auto* newFreeListArray = new U32[newSize];
			auto* newGenerations   = new U32[newSize];

			std::memset(newDataArray,     0, newSize * sizeof(ImplT));
			std::memset(newFreeListArray, 0, newSize * sizeof(U32));

			for (size_t i = 0; i < newSize; i++)
			{
				newGenerations[i] = 1;
			}

			// Copy data over
			std::memcpy(newDataArray, m_Data, m_Size * sizeof(ImplT));
			// No need to copy the free list as we need to re-populate it.
			// std::memcpy(newFreeListArray, m_FreeList, m_Size * sizeof(U32));
			std::memcpy(newGenerations, m_Generations, m_Size * sizeof(U32));

			delete[] m_Data;
			delete[] m_FreeList;
			delete[] m_Generations;

			m_Data        = newDataArray;
			m_FreeList    = newFreeListArray;
			m_Generations = newGenerations;

			m_FreeListPosition = m_Size - 1;

			m_Size = newSize;

			for (size_t i = 0; i < m_Size - 1; i++)
			{
				m_FreeList[i] = static_cast<U32>((m_Size - 1) - i);
			}
		}

		bool hasSpace() const { return m_NumActiveEntries < m_Size; }

	private:
		size_t m_Size;
		size_t m_NumActiveEntries;
		size_t m_FreeListPosition;

		// free array
		U32* m_FreeList;

		// Generation array
		U32* m_Generations;

		// Data Array
		ImplT* m_Data;
	};




	template<typename ImplT, typename HT>
	class VyHandlePoolVirtual
	{
		struct VirtualPageAllocator
		{
			U8*    VirtualPtr;
			size_t TotalMemoryCommited = 0;
			size_t PageSize            = 0;
			size_t VirtualMemorySize   = 0;


			void initialize(size_t virtualMemorySize, size_t pageSize)
			{
				VirtualMemorySize = virtualMemorySize;
				PageSize          = pageSize;
				VirtualPtr        = static_cast<U8*>(VirtualAlloc(NULL, VirtualMemorySize, MEM_RESERVE, PAGE_READWRITE));

				grow();
			}

			void grow()
			{
				if ((TotalMemoryCommited + PageSize) > VirtualMemorySize)
                {
                    throw std::runtime_error("Ran out of virtual memory");
                }

				// Commit data
				VirtualAlloc(VirtualPtr + TotalMemoryCommited, PageSize, MEM_COMMIT, PAGE_READWRITE);
				TotalMemoryCommited += PageSize;
			}

			~VirtualPageAllocator()
			{
				// Free the committed memory
				if (!VirtualFree(reinterpret_cast<void*>(VirtualPtr), 0, MEM_RELEASE))
				{
					std::cerr << "Memory deallocation failed." << std::endl;
				}

				VirtualPtr = nullptr;
				TotalMemoryCommited = 0;
			}
		};

	public:
		VyHandlePoolVirtual()
			: m_Size(0)
			, m_NumActiveEntries(0)
			, m_Data(nullptr)
			, m_Generations(nullptr)
			, m_FreeList(nullptr)
		{
		}

		~VyHandlePoolVirtual()
		{
			finalize();
		}

		void initialize(size_t reservedMemory, size_t initCapacity = 16)
		{
			m_Size = initCapacity;
			m_NumActiveEntries = 0;

			m_FreeListAllocator .initialize(reservedMemory, sizeof(U32)   * initCapacity);
			m_GeneratorAllocator.initialize(reservedMemory, sizeof(U32)   * initCapacity);
			m_DataAllocator     .initialize(reservedMemory, sizeof(ImplT) * initCapacity);

			m_Data        = reinterpret_cast<ImplT*>(m_DataAllocator.VirtualPtr);
			m_Generations = reinterpret_cast<U32*>(m_GeneratorAllocator.VirtualPtr);
			m_FreeList    = reinterpret_cast<U32*>(m_FreeListAllocator.VirtualPtr);

			std::memset(m_Data,        0, m_Size * sizeof(ImplT));
			std::memset(m_Generations, 0, m_Size * sizeof(U32));
			std::memset(m_FreeList,    0, m_Size * sizeof(U32));

			for (size_t i = 0; i < m_Size; i++)
			{
				m_Generations[i] = 1;
			}

			m_FreeListPosition = m_Size - 1;

			for (size_t i = 0; i < m_Size; i++)
			{
				m_FreeList[i] = static_cast<U32>((m_Size - 1) - i);
			}
		}

		void finalize()
		{
			if (m_Data)
			{
				for (int i = 0; i < m_Size; i++)
				{
					m_Data[i].~ImplT();
				}
			}

			m_Size = 0;
		}

		ImplT* get(VyHandle<HT> handle)
		{
			if (!contains(handle))
			{
				return nullptr;
			}

			return m_Data + handle.m_Index;
		}

		bool contains(VyHandle<HT> handle) const
		{
			return
				handle.isValid() &&
				handle.m_Index < m_Size&&
				m_Generations[handle.m_Index] == handle.m_Generation;
		}


		template<typename... Args>
		VyHandle<HT> emplace(Args&&... args)
		{
			if (!hasSpace())
			{
				grow();
			}

			VyHandle<HT> handle;
			// Get a free index
			handle.m_Index = m_FreeList[m_FreeListPosition];
			handle.m_Generation = m_Generations[handle.m_Index];
			m_FreeListPosition--;
			m_NumActiveEntries++;

			new (m_Data + handle.m_Index) ImplT(std::forward<Args>(args)...);

			return handle;
		}

		VyHandle<HT> insert(ImplT const& Data)
		{
			return emplace(Data);
		}

		void release(VyHandle<HT> handle)
		{
			if (!contains(handle))
			{
				return;
			}

			get(handle)->~ImplT();
			new (m_Data + handle.m_Index) ImplT();
			m_Generations[handle.m_Index] += 1;

			// To prevent the risk of re assignment, block index for being allocated
			if (m_Generations[handle.m_Index] == std::numeric_limits<U32>::max())
			{
				return;
			}

			m_FreeList[++m_FreeListPosition] = handle.m_Index;
			m_NumActiveEntries--;
		}

		bool isEmpty() const { return m_NumActiveEntries == 0; }

	private:
		void grow()
		{
			if (m_Size == 0)
			{
				initialize(16);
				return;
			}

			m_GeneratorAllocator.grow();
			m_DataAllocator.grow();
			m_FreeListAllocator.grow();

			const int newSize = m_GeneratorAllocator.TotalMemoryCommited / sizeof(ImplT);

			for (size_t i = m_Size; i < newSize; i++)
			{
				m_Generations[i] = 1;
			}
			
			m_FreeListPosition = m_Size - 1;

			m_Size = newSize;

			for (size_t i = 0; i < m_Size - 1; i++)
			{
				m_FreeList[i] = static_cast<U32>((m_Size - 1) - i);
			}
		}

		bool hasSpace() const { return m_NumActiveEntries < m_Size; }

	private:
		size_t m_Size;
		size_t m_NumActiveEntries;
		size_t m_FreeListPosition;

		VirtualPageAllocator m_FreeListAllocator;
		VirtualPageAllocator m_GeneratorAllocator;
		VirtualPageAllocator m_DataAllocator;
		
		// free array
		U32* m_FreeList;

		// Generation array
		U32* m_Generations;

		// Data Array
		ImplT* m_Data;
	};
} // namespace Vy
