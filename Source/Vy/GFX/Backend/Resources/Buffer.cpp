#include <Vy/GFX/Backend/Resources/Buffer.h>

#include <Vy/GFX/Context.h>

namespace Vy
{

#pragma region [ Factory Methods ]

	// ========================================================================================

	VyBufferDesc VyBuffer::uniformBuffer(VkDeviceSize instanceSize, U32 instanceCount)
	{
		VY_ASSERT(instanceSize  > 0, "Cannot create uniform buffer of size 0");
		VY_ASSERT(instanceCount > 0, "Cannot create uniform buffer with 0 instances");

		return VyBufferDesc{
			.InstanceSize       = instanceSize, 
			.InstanceCount      = instanceCount, 
			.UsageFlags         = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			.AllocFlags         = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT, 
			.MinOffsetAlignment = VyContext::device().properties().limits.minUniformBufferOffsetAlignment
		};
	}


	VyBufferDesc VyBuffer::storageBuffer(VkDeviceSize instanceSize, U32 instanceCount, VkBufferUsageFlags otherUsage)
	{
		VY_ASSERT(instanceSize  > 0, "Cannot create storage buffer of size 0");
		VY_ASSERT(instanceCount > 0, "Cannot create storage buffer with 0 instances");

		return VyBufferDesc{
			.InstanceSize       = instanceSize, 
			.InstanceCount      = instanceCount, 
			.UsageFlags         = otherUsage | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			.AllocFlags         = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT, 
			.MinOffsetAlignment = VyContext::device().properties().limits.minStorageBufferOffsetAlignment
		};
	}


	VyBufferDesc VyBuffer::vertexBuffer(VkDeviceSize instanceSize, U32 instanceCount, VkBufferUsageFlags otherUsage)
	{
		VY_ASSERT(instanceSize  > 0, "Cannot create vertex buffer of size 0");
		VY_ASSERT(instanceCount > 0, "Cannot create vertex buffer with 0 instances");

		return VyBufferDesc{
			.InstanceSize       = instanceSize, 
			.InstanceCount      = instanceCount, 
			.UsageFlags         = otherUsage | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			.AllocFlags         = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT
		};
	}


	VyBufferDesc VyBuffer::indexBuffer(VkDeviceSize instanceSize, U32 instanceCount, VkBufferUsageFlags otherUsage)
	{
		VY_ASSERT(instanceSize  > 0, "Cannot create index buffer of size 0");
		VY_ASSERT(instanceCount > 0, "Cannot create index buffer with 0 instances");

		return VyBufferDesc{
			.InstanceSize       = instanceSize, 
			.InstanceCount      = instanceCount, 
			.UsageFlags         = otherUsage | VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			.AllocFlags         = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT
		};
	}


	VyBufferDesc 
    VyBuffer::stagingBuffer(VkDeviceSize instanceSize)
	{
		VY_ASSERT(instanceSize  > 0, "Cannot create staging buffer of size 0");

		return VyBufferDesc{
			.InstanceSize       = instanceSize, 
			.InstanceCount      = 1, 
			.UsageFlags         = VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
			.AllocFlags         = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
		};
	}


	VyBufferDesc 
	VyBuffer::stagingBuffer(VkDeviceSize instanceSize, U32 instanceCount, VmaAllocationCreateFlags otherFlags)
	{
		VY_ASSERT(instanceSize  > 0, "Cannot create staging buffer of size 0");
		VY_ASSERT(instanceCount > 0, "Cannot create staging buffer with 0 instances");

		return VyBufferDesc{
			.InstanceSize       = instanceSize, 
			.InstanceCount      = instanceCount, 
			.UsageFlags         = VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
			.AllocFlags         = otherFlags | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
		};
	}

#pragma endregion Factory Methods

    // ========================================================================================

    VkDeviceSize 
	VyBuffer::computeAlignment(VkDeviceSize instanceSize, VkDeviceSize minOffsetAlignment)
	{
		if (minOffsetAlignment > 0)
		{
			return (instanceSize + minOffsetAlignment - 1) & ~(minOffsetAlignment - 1);
		}

		return instanceSize;
	}

    // ========================================================================================

	VyBuffer::VyBuffer(const VyBufferDesc& desc, bool bPersistentMapped) :
		m_InstanceSize { desc.InstanceSize  },
        m_InstanceCount{ desc.InstanceCount }, 
        m_BufferUsage  { desc.UsageFlags    }
	{
		// if true, creates a dynamic memory, such as the Uniform Buffer, to use in case of frequent write on CPU and frequent read on GPU.
		m_IsPersistentMapped = bPersistentMapped;

		m_AlignmentSize = computeAlignment(m_InstanceSize, desc.MinOffsetAlignment);
		m_BufferSize    = m_AlignmentSize * m_InstanceCount;

        // Create the buffer from the device.
		VyContext::device().createBuffer(
			m_Buffer, 
			m_Allocation, 
			m_BufferSize, 
			m_BufferUsage, 
			desc.AllocFlags, 
			VMA_MEMORY_USAGE_AUTO
		);

		if (m_IsPersistentMapped)
		{
			VmaAllocationInfo allocInfo;
			vmaGetAllocationInfo(VyContext::allocator(), m_Allocation, &allocInfo);
			
			m_MappedData = allocInfo.pMappedData;
		}
	}


    VyBuffer::VyBuffer(
        VkDeviceSize             instanceSize,
        U32                      instanceCount,
        VkBufferUsageFlags       usageFlags,
        VmaAllocationCreateFlags allocFlags,
        VkDeviceSize             minOffsetAlignment
    ) : 
        m_InstanceSize { instanceSize  },
        m_InstanceCount{ instanceCount },
        m_BufferUsage  { usageFlags    }
    {
        m_AlignmentSize = computeAlignment(m_InstanceSize, minOffsetAlignment);
        m_BufferSize    = m_AlignmentSize * instanceCount;

        // Create the buffer from the device.
		VyContext::device().createBuffer(
			m_Buffer, 
			m_Allocation, 
			m_BufferSize, 
			m_BufferUsage, 
			allocFlags, 
			VMA_MEMORY_USAGE_AUTO
		);
    }


	VyBuffer::VyBuffer(VyBuffer&& other) noexcept
	{
		moveFrom(std::move(other));
	}


    VyBuffer::~VyBuffer() 
    {
        destroy();
    }


	VyBuffer& VyBuffer::operator=(VyBuffer&& other) noexcept
	{
		if (this != &other)
		{
			destroy();
            
			moveFrom(std::move(other));
		}

		return *this;
	}

    // ========================================================================================

	VkDescriptorBufferInfo 
	VyBuffer::descriptorBufferInfo(VkDeviceSize instanceSize, VkDeviceSize offset) const
	{
		return VkDescriptorBufferInfo{ m_Buffer, offset, instanceSize };
	}


	VkDescriptorBufferInfo 
	VyBuffer::descriptorBufferInfoForIndex(int index) const
	{
		return descriptorBufferInfo(m_AlignmentSize, index * m_AlignmentSize);
	}

    // ========================================================================================

	void VyBuffer::map()
	{
		VY_ASSERT(!m_MappedData, "Buffer is already mapped");

		VK_CHECK(vmaMapMemory(VyContext::allocator(), m_Allocation, &m_MappedData));
	}


	void VyBuffer::unmap()
	{
		if (m_MappedData)
		{
			vmaUnmapMemory(VyContext::allocator(), m_Allocation);
			
			m_MappedData = nullptr;
		}
	}

    // ========================================================================================

	void VyBuffer::write(const void* data)
	{
		write(data, m_BufferSize, 0);
	}


	void VyBuffer::write(const void* data, VkDeviceSize instanceSize, VkDeviceSize offset)
	{
		VY_ASSERT(data,         "Data pointer is null");
		VY_ASSERT(m_MappedData, "Called write on buffer before map");

		memcpy(static_cast<U8*>(m_MappedData) + offset, data, instanceSize);
		
		flush(instanceSize, offset);
	}


	void VyBuffer::writeToBuffer(const void* data)
	{
		writeToBuffer(data, m_BufferSize, 0);
	}


	void VyBuffer::writeToBuffer(const void* data, VkDeviceSize instanceSize, VkDeviceSize offset)
	{
		VY_ASSERT(data,         "Data pointer is null");
		VY_ASSERT(m_MappedData, "Called write on buffer before map");

        if (instanceSize == VK_WHOLE_SIZE) 
		{
            memcpy(m_MappedData, data, m_BufferSize);
        } 
		else 
		{
            char* memOffset = (char*)m_MappedData;
			
            memOffset += offset;

            memcpy(memOffset, data, instanceSize);
        }
	}


	void VyBuffer::writeToIndex(const void* data, int index)
	{
		VY_ASSERT(data,                    "Data pointer is null");
		VY_ASSERT(m_MappedData,            "Called write on buffer before map");
		VY_ASSERT(index < m_InstanceCount, "Requested write index exceeds instance count");

		memcpy(static_cast<U8*>(m_MappedData) + (index * m_AlignmentSize), data, m_InstanceSize);

		flushIndex(index);
	}


	void VyBuffer::singleWrite(const void* data)
	{
		VY_ASSERT(data, "Data pointer is null");

		if (m_InstanceCount == 1)
		{
			singleWrite(data, m_InstanceSize, 0);
		}
		else // Copy data to all instances.
		{
			map();

			for (U32 i = 0; i < m_InstanceCount; i++)
			{
				memcpy(static_cast<U8*>(m_MappedData) + (i * m_AlignmentSize), data, m_InstanceSize);
			}

			flush(m_BufferSize, 0);

			unmap();
		}
	}


	void VyBuffer::singleWrite(const void* data, VkDeviceSize instanceSize, VkDeviceSize offset)
	{
		VY_ASSERT(data, "Data pointer is null");
		VY_ASSERT((instanceSize == VK_WHOLE_SIZE && offset == 0) || (offset + instanceSize <= m_BufferSize), 
			"Single write exceeds buffer size"
		);

		VK_CHECK(vmaCopyMemoryToAllocation(VyContext::allocator(), data, m_Allocation, offset, instanceSize));
	}

    // ========================================================================================

	// Note: flushing after mapping is only needed on non host coherent memory
	// (AMD, Intel, NVIDIA) driver currently provide HOST_COHERENT flag on all memory types that are HOST_VISIBLE
	void VyBuffer::flush(VkDeviceSize instanceSize, VkDeviceSize offset)
	{
		VY_ASSERT(m_MappedData, "Called flush on buffer before map");
		VY_ASSERT((instanceSize == VK_WHOLE_SIZE && offset == 0) || (offset + instanceSize <= m_BufferSize), 
			"Flush range exceeds buffer size"
		);
		
		VK_CHECK(vmaFlushAllocation(VyContext::allocator(), m_Allocation, offset, instanceSize));
	}


	void VyBuffer::flushIndex(int index)
	{
		VY_ASSERT(index < m_InstanceCount, "Requested flush index exceeds instance count");

		flush(m_AlignmentSize, index * m_AlignmentSize);
	}

    // ========================================================================================

	// Note: invalidating before mapping is only needed on non host coherent memory
	// (AMD, Intel, NVIDIA) driver currently provide HOST_COHERENT flag on all memory types that are HOST_VISIBLE
	void VyBuffer::invalidate(VkDeviceSize size, VkDeviceSize offset)
	{
		VY_ASSERT(m_Allocation, "Buffer must have a valid allocation");

		vmaInvalidateAllocation(VyContext::allocator(), m_Allocation, offset, size);
	}

	void VyBuffer::invalidateIndex(int index)
	{
		VY_ASSERT(index < m_InstanceCount, "Requested invalidate index exceeds instance count");

		invalidate(m_AlignmentSize, index * m_AlignmentSize);
	}

    // ========================================================================================

	void VyBuffer::upload(const void* data, VkDeviceSize instanceSize)
	{
		// TODO: Use a global shared staging buffer (creating a new one for each upload is inefficient)
		VyBuffer stagingBuffer{ VyBuffer::stagingBuffer(instanceSize) };

		stagingBuffer.singleWrite(data, instanceSize, 0);
		
		stagingBuffer.copyTo(*this, instanceSize);
	}

    // ========================================================================================

	void VyBuffer::copyTo(VyBuffer& dest, VkDeviceSize instanceSize)
	{
		VY_ASSERT((instanceSize == VK_WHOLE_SIZE) || (instanceSize <= m_BufferSize && instanceSize <= dest.m_BufferSize),
			"Copy size exceeds source or destination buffer size");

		VyContext::device().copyBuffer(m_Buffer, dest.m_Buffer, instanceSize);
	}

    // ========================================================================================

	void VyBuffer::moveFrom(VyBuffer&& other)
	{
		m_Buffer        = std::exchange(other.m_Buffer,        VK_NULL_HANDLE);
		m_Allocation    = std::exchange(other.m_Allocation,    VK_NULL_HANDLE);
		m_BufferSize    = std::exchange(other.m_BufferSize,    0);
		m_InstanceSize  = std::exchange(other.m_InstanceSize,  0);
		m_AlignmentSize = std::exchange(other.m_AlignmentSize, 0);
		m_InstanceCount = std::exchange(other.m_InstanceCount, 0);
		m_BufferUsage   = std::exchange(other.m_BufferUsage,   0);
		m_MappedData    = std::exchange(other.m_MappedData,    nullptr);
	}

	
	VkDeviceAddress VyBuffer::deviceAddress() const
	{
		if (m_Buffer)
		{
			VkBufferDeviceAddressInfo addressInfo{ VKInit::bufferDeviceAddressInfo() };
			{
				addressInfo.pNext  = nullptr;
				addressInfo.buffer = m_Buffer;
			}

			return vkGetBufferDeviceAddress(VyContext::device(), &addressInfo);
		}

		return 0;
	}

	void VyBuffer::destroy()
	{
        unmap();

		VyContext::destroy(m_Buffer, m_Allocation);
		
		m_Buffer     = VK_NULL_HANDLE;
		m_Allocation = VK_NULL_HANDLE;
	}
}