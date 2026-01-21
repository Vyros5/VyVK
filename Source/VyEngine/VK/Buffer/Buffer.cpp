#include <VyEngine/VK/Buffer/Buffer.h>

#include <VyEngine/VK/Context.h>
#include <VyLib/Util/String.h>

#include <iostream>
#include <VyLib/Common/AnsiColor.h>

namespace Vy
{

// https://github.com/haosulab/sapien-vulkan-2/blob/master/src/core/buffer.cpp#L68

// =========================================================================================================================
#pragma region [ Factory Methods ]
// =========================================================================================================================

	VyBufferInfo 
	VyBuffer::uniformBuffer(TString name, VkDeviceSize instanceSize, U32 instanceCount /*MAX_FRAMES_IN_FLIGHT*/)
	{
		VY_ASSERT(instanceSize  > 0, "Cannot create uniform buffer of size 0");
		VY_ASSERT(instanceCount > 0, "Cannot create uniform buffer with 0 instances");

		return VyBufferInfo{
			.DebugName          = name + "_uniform_buffer",
			.InstanceSize       = instanceSize, 
			.InstanceCount      = instanceCount, 
			.UsageFlags         = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			.AllocFlags         = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT, 
			.MinOffsetAlignment = VyContext::device().properties().limits.minUniformBufferOffsetAlignment
		};
	}


	VyBufferInfo 
	VyBuffer::storageBuffer(TString name, VkDeviceSize instanceSize, U32 instanceCount /*1*/, VkBufferUsageFlags otherUsage /*0*/)
	{
		VY_ASSERT(instanceSize  > 0, "Cannot create storage buffer of size 0");
		VY_ASSERT(instanceCount > 0, "Cannot create storage buffer with 0 instances");

		return VyBufferInfo{
			.DebugName          = name + "_storage_buffer",
			.InstanceSize       = instanceSize, 
			.InstanceCount      = instanceCount, 
			.UsageFlags         = otherUsage | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			.AllocFlags         = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT, 
			.MinOffsetAlignment = VyContext::device().properties().limits.minStorageBufferOffsetAlignment
		};
	}


	VyBufferInfo 
	VyBuffer::vertexBuffer(TString name, VkDeviceSize instanceSize, U32 instanceCount /*1*/, VkBufferUsageFlags otherUsage /*0*/)
	{
		VY_ASSERT(instanceSize  > 0, "Cannot create vertex buffer of size 0");
		VY_ASSERT(instanceCount > 0, "Cannot create vertex buffer with 0 instances");

		return VyBufferInfo{
			.DebugName          = name + "_vertex_buffer",
			.InstanceSize       = instanceSize, 
			.InstanceCount      = instanceCount, 
			.UsageFlags         = otherUsage | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			.AllocFlags         = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT
		};
	}


	VyBufferInfo 
	VyBuffer::indexBuffer(TString name, VkDeviceSize instanceSize, U32 instanceCount /*1*/, VkBufferUsageFlags otherUsage /*0*/)
	{
		VY_ASSERT(instanceSize  > 0, "Cannot create index buffer of size 0");
		VY_ASSERT(instanceCount > 0, "Cannot create index buffer with 0 instances");

		return VyBufferInfo{
			.DebugName          = name + "_index_buffer",
			.InstanceSize       = instanceSize, 
			.InstanceCount      = instanceCount, 
			.UsageFlags         = otherUsage | VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			.AllocFlags         = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT
		};
	}


	VyBufferInfo 
	VyBuffer::stagingBuffer(TString name, VkDeviceSize instanceSize, U32 instanceCount /*1*/, VkBufferUsageFlags otherUsage /*0*/, bool bPersistent /*true*/)
	{
		VY_ASSERT(instanceSize  > 0, "Cannot create staging buffer of size 0");
		VY_ASSERT(instanceCount > 0, "Cannot create staging buffer with 0 instances");

		if (bPersistent)
		{
			return VyBufferInfo{
				.DebugName          = name + "_staging_buffer",
				.InstanceSize       = instanceSize, 
				.InstanceCount      = instanceCount, 
				.UsageFlags         = otherUsage | VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
				.AllocFlags         = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT
			};
		}

		return VyBufferInfo{
			.DebugName          = name + "_staging_buffer",
			.InstanceSize       = instanceSize, 
			.InstanceCount      = instanceCount, 
			.UsageFlags         = otherUsage | VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
			.AllocFlags         = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
		};
	}

#pragma endregion Factory Methods


// =========================================================================================================================
#pragma region [ Main ]
// =========================================================================================================================

	VyBuffer::VyBuffer(const VyBufferInfo& info) :
		m_InstanceSize { info.InstanceSize  },
        m_InstanceCount{ info.InstanceCount }, 
        m_BufferUsage  { info.UsageFlags    },
		m_DebugName         { info.DebugName     }
	{
		m_AlignmentSize = computeAlignment(info.InstanceSize, info.MinOffsetAlignment);
		m_BufferSize    = m_AlignmentSize * m_InstanceCount;

		VkBufferCreateInfo bufferInfo{ VKInit::bufferCreateInfo() };
		{
			bufferInfo.size        = m_BufferSize;
			bufferInfo.usage       = m_BufferUsage;
			bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		}

		VmaAllocationCreateInfo allocCreateInfo{};
		{
			allocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
			allocCreateInfo.flags = info.AllocFlags;
		}

		create( bufferInfo, allocCreateInfo );
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
        m_AlignmentSize = computeAlignment(instanceSize, minOffsetAlignment);
        m_BufferSize    = m_AlignmentSize * instanceCount;

		VkBufferCreateInfo bufferInfo{ VKInit::bufferCreateInfo() };
		{
			bufferInfo.size        = m_BufferSize;
			bufferInfo.usage       = m_BufferUsage;
			bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		}

		VmaAllocationCreateInfo allocCreateInfo{};
		{
			allocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
			allocCreateInfo.flags = allocFlags;
		}

		create( bufferInfo, allocCreateInfo );
    }


	VyBuffer::VyBuffer(VyBuffer&& other) noexcept
	{
		moveFrom( std::move( other ) );
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
            
			moveFrom( std::move( other ) );
		}

		return *this;
	}


	void VyBuffer::create(
		VkBufferCreateInfo&      bufferInfo,
		VmaAllocationCreateInfo& allocCreateInfo)
	{
		VK_CHECK_SUCCESS(vmaCreateBuffer(
			VyContext::allocator(), 
			&bufferInfo, 
			&allocCreateInfo, 
			&m_Buffer, 
			&m_Allocation, 
			nullptr
		), "Failed to create buffer!");

		// [ Set mapped data ]
		// if (allocCreateInfo.flags & VMA_ALLOCATION_CREATE_MAPPED_BIT) 
		vmaGetAllocationInfo(VyContext::allocator(), m_Allocation, &m_AllocationInfo);
		m_pMappedData = m_AllocationInfo.pMappedData;

		printInfo();

		VKDbg::setObjectName(VK_OBJECT_TYPE_BUFFER, (U64)m_Buffer, m_DebugName.c_str());
		vmaSetAllocationName(VyContext::allocator(), m_Allocation, m_DebugName.c_str());
	}


	void VyBuffer::destroy()
	{
		if (m_Buffer != VK_NULL_HANDLE) 
		{
			// Unmap if currently mapped but not persistantly mapped.
			if (m_pMappedData != nullptr && !(m_AllocationInfo.pMappedData)) 
			{
				vmaUnmapMemory(VyContext::allocator(), m_Allocation);
			}

			// Destroy buffer and free memory.
			VyContext::destroy(m_Buffer, m_Allocation);

			m_Buffer     = VK_NULL_HANDLE;
			m_Allocation = VK_NULL_HANDLE;

			// std::cout << "[VyBuffer] Destroyed '" << m_DebugName << "'" << std::endl;
		}
	}

#pragma endregion Main


// =========================================================================================================================
#pragma region [ Descriptor ]
// =========================================================================================================================

	VkDescriptorBufferInfo 
	VyBuffer::descriptorBufferInfo(VkDeviceSize instanceSize, VkDeviceSize offset) const
	{
		VY_ASSERT((instanceSize == VK_WHOLE_SIZE && offset == 0) || (offset + instanceSize <= m_BufferSize),
			"Requested descriptor buffer info exceeds buffer size!");

		return VkDescriptorBufferInfo{ m_Buffer, offset, instanceSize };
	}


	VkDescriptorBufferInfo 
	VyBuffer::descriptorBufferInfoForIndex(int index) const
	{
		VY_ASSERT(index < m_InstanceCount, 
			"Requested descriptor buffer info index exceeds instance count!");

		return descriptorBufferInfo( m_AlignmentSize, index * m_AlignmentSize );
	}

#pragma endregion Descriptor


// =========================================================================================================================
#pragma region [ Map ]
// =========================================================================================================================

	void VyBuffer::map()
	{
		VY_ASSERT(!m_pMappedData, "Buffer is already mapped!");

		VK_CHECK_SUCCESS(vmaMapMemory(VyContext::allocator(), m_Allocation, &m_pMappedData), 
			"Failed to map memory!");
	}


	void VyBuffer::unmap()
	{
		// Only unmap if it's not a persistent mapping.
		if (m_pMappedData != nullptr && m_AllocationInfo.pMappedData == nullptr) 
		{
			vmaUnmapMemory(VyContext::allocator(), m_Allocation);

			m_pMappedData = nullptr;
		}
	}

#pragma endregion Map


// =========================================================================================================================
#pragma region [ Write ]
// =========================================================================================================================

	void VyBuffer::write(const void* pData)
	{
		write( pData, m_BufferSize, 0 );
	}


	void VyBuffer::write(const void* pData, VkDeviceSize instanceSize, VkDeviceSize offset)
	{
		VY_ASSERT(pData,         "Data pointer is null");
		VY_ASSERT(m_pMappedData, "Called write on buffer before map");

		// std::stringstream ss;
        // ss << "Write '" GREEN << m_DebugName << RESET CYAN "' (" << Utils::formatBytes(static_cast<U64>(instanceSize)) << ")" << RESET;
		// VY_TRACE_TAG("VyBuffer", "{}", ss.str());

		std::memcpy(static_cast<U8*>(m_pMappedData) + offset, pData, instanceSize);
		
		flush( instanceSize, offset );
	}


	void VyBuffer::writeToIndex(const void* pData, int index)
	{
		VY_ASSERT(pData,                   "Data pointer is null");
		VY_ASSERT(m_pMappedData,           "Called write on buffer before map");
		VY_ASSERT(index < m_InstanceCount, "Requested write index exceeds instance count");

		std::memcpy(static_cast<U8*>(m_pMappedData) + (index * m_AlignmentSize), pData, m_InstanceSize);

		flushIndex( index );
	}


	void VyBuffer::singleWrite(const void* pData)
	{
		VY_ASSERT(pData, "Data pointer is null");

		if (m_InstanceCount == 1)
		{
			singleWrite( pData, m_InstanceSize, 0 );
		}
		else // Copy data to all instances.
		{
			map();
			{
				for (U32 i = 0; i < m_InstanceCount; i++)
				{
					std::memcpy(static_cast<U8*>(m_pMappedData) + (i * m_AlignmentSize), pData, m_InstanceSize);
				}
				
				flush( m_BufferSize, 0 );
			}
			unmap();
		}
	}


	void VyBuffer::singleWrite(const void* pData, VkDeviceSize instanceSize, VkDeviceSize offset)
	{
		VY_ASSERT(pData, "Data pointer is null!");

		VY_ASSERT((instanceSize == VK_WHOLE_SIZE && offset == 0) || (offset + instanceSize <= m_BufferSize), 
			"Single write exceeds buffer size!");

		VK_CHECK(vmaCopyMemoryToAllocation(VyContext::allocator(), pData, m_Allocation, offset, instanceSize));
	}

#pragma endregion Write


// =========================================================================================================================
#pragma region [ Flush ]
// =========================================================================================================================

	// Note: flushing after mapping is only needed on non host coherent memory
	// (AMD, Intel, NVIDIA) driver currently provide HOST_COHERENT flag on all memory types that are HOST_VISIBLE
	void VyBuffer::flush(VkDeviceSize instanceSize, VkDeviceSize offset)
	{
		VY_ASSERT(m_pMappedData, 
			"Called flush on buffer before map");

		VY_ASSERT((instanceSize == VK_WHOLE_SIZE && offset == 0) || (offset + instanceSize <= m_BufferSize), 
			"Flush range exceeds buffer size!");
		
		VK_CHECK(vmaFlushAllocation(VyContext::allocator(), m_Allocation, offset, instanceSize));
	}


	void VyBuffer::flushIndex(int index)
	{
		VY_ASSERT(index < m_InstanceCount, 
			"Requested flush index exceeds instance count!");

		flush(m_AlignmentSize, index * m_AlignmentSize);
	}

#pragma endregion Flush


// =========================================================================================================================
#pragma region [ Invalidate ]
// =========================================================================================================================

	// Note: invalidating before mapping is only needed on non host coherent memory
	// (AMD, Intel, NVIDIA) driver currently provide HOST_COHERENT flag on all memory types that are HOST_VISIBLE
	void VyBuffer::invalidate(VkDeviceSize size, VkDeviceSize offset)
	{
		VY_ASSERT(m_Allocation, 
			"Buffer must have a valid allocation");

		vmaInvalidateAllocation(VyContext::allocator(), m_Allocation, offset, size);
	}

	
	void VyBuffer::invalidateIndex(int index)
	{
		VY_ASSERT(index < m_InstanceCount, 
			"Requested invalidate index exceeds instance count");

		invalidate(m_AlignmentSize, index * m_AlignmentSize);
	}

#pragma endregion Invalidate


// =========================================================================================================================
#pragma region [ Upload ]
// =========================================================================================================================

	void VyBuffer::upload(const void* pData, VkDeviceSize instanceSize)
	{
		// TODO: Use a global shared staging buffer (creating a new one for each upload is inefficient)

		VyBuffer stagingBuffer{ VyBuffer::stagingBuffer( "upload", instanceSize ) };

		stagingBuffer.singleWrite( pData, instanceSize, 0 );
		
		stagingBuffer.copyTo( *this, instanceSize );
	}

#pragma endregion Upload


// =========================================================================================================================
#pragma region [ Copy ]
// =========================================================================================================================

	void VyBuffer::copy(const void* pData, VkDeviceSize instanceSize, int index)
	{
		VY_ASSERT(pData,                        "Data pointer is null");
		VY_ASSERT(m_pMappedData,                "Called copy on buffer before map");
		VY_ASSERT(instanceSize <= m_BufferSize, "Copy size exceeds buffer size");
		VY_ASSERT(index < m_InstanceCount,      "Requested copy index exceeds instance count");

		VK_CHECK(vmaCopyMemoryToAllocation(VyContext::allocator(), pData, m_Allocation, index * m_AlignmentSize, instanceSize));
	}


	void VyBuffer::copyTo(VyBuffer& dstBuffer, VkDeviceSize instanceSize)
	{
		VY_ASSERT((instanceSize == VK_WHOLE_SIZE) || (instanceSize <= m_BufferSize && instanceSize <= dstBuffer.m_BufferSize),
			"Copy size exceeds source or destination buffer size");

		VyContext::device().copyBuffer(m_Buffer, dstBuffer.m_Buffer, instanceSize);
	}


	void VyBuffer::copyTo(VkCommandBuffer cmdBuffer, VyBuffer& dstBuffer, U32 srcIndex, U32 dstIndex) const
	{
		VY_ASSERT(srcIndex < m_InstanceCount,           "Source index exceeds instance count");
		VY_ASSERT(dstIndex < dstBuffer.m_InstanceCount, "Destination index exceeds instance count");

		VkBufferCopy copy{};
		{
			copy.srcOffset = srcIndex * m_AlignmentSize;
			copy.dstOffset = dstIndex * dstBuffer.m_AlignmentSize;
			copy.size      = m_InstanceSize;
		}
		
		vkCmdCopyBuffer(cmdBuffer, 
			m_Buffer, 
			dstBuffer.m_Buffer, 
			1, &copy
		);
	}

#pragma endregion Copy


// =========================================================================================================================
#pragma region [ Other ]
// =========================================================================================================================

	void VyBuffer::setName(const TString& name) const
	{
        // VyDebugLabel::nameBuffer(m_Buffer, name);
        
		// if (m_Allocation) 
		// {
        //     VyDebugLabel::nameAllocation(m_Allocation, name);
        // }
	}


	void VyBuffer::printInfo()
	{
		std::stringstream ss;
		
        ss << "Created '" GREEN << m_DebugName << RESET CYAN "' (" << Utils::formatBytes(static_cast<U64>(m_BufferSize)) << ")" << RESET;

		VY_TRACE_TAG("VyBuffer", "{}", ss.str());
	}


	void VyBuffer::moveFrom(VyBuffer&& other)
	{
		m_Buffer        = std::exchange(other.m_Buffer,        VK_NULL_HANDLE);
		m_Allocation    = std::exchange(other.m_Allocation,    VK_NULL_HANDLE);
		m_BufferSize    = std::exchange(other.m_BufferSize,    0);
		m_InstanceSize  = std::exchange(other.m_InstanceSize,  0);
		m_AlignmentSize = std::exchange(other.m_AlignmentSize, 0);
		m_InstanceCount = std::exchange(other.m_InstanceCount, 0);
		m_BufferUsage   = std::exchange(other.m_BufferUsage,   0);
		m_pMappedData   = std::exchange(other.m_pMappedData,   nullptr);
		m_DebugName     = std::exchange(other.m_DebugName,     "unnamed");
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


    VkDeviceSize 
	VyBuffer::computeAlignment(VkDeviceSize instanceSize, VkDeviceSize minOffsetAlignment)
	{
		if (minOffsetAlignment > 0)
		{
			return (instanceSize + minOffsetAlignment - 1) & ~(minOffsetAlignment - 1);
		}

		return instanceSize;
	}

}



    // struct CreateHandleResult
    // {
    //     VkBuffer buffer;
    //     VmaAllocation allocation;
    //     VmaAllocationInfo allocInfo;
    //     bool isOk;
    // };

    // static CreateHandleResult CreateHandle(U32 size, Buffer::Usage usage, Buffer::Property properties)
    // {
    //     VkBuffer buffer;
    //     VkBufferCreateInfo bufferInfo{};
    //     bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    //     bufferInfo.size = size;
    //     bufferInfo.usage = static_cast<std::underlying_type_t<Buffer::Usage>>(usage);
    //     bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    //     VmaAllocationCreateInfo allocCreateInfo = {};
    //     allocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
    //     allocCreateInfo.flags = static_cast<std::underlying_type_t<Buffer::Property>>(properties);
    //     VmaAllocation allocation;
    //     VmaAllocationInfo allocInfo;

    //     vmaCreateBuffer(VyContext::allocator(), &bufferInfo, &allocCreateInfo, &buffer, &allocation, &allocInfo);

    //     if (buffer == VK_NULL_HANDLE && allocation == VK_NULL_HANDLE)
    //     {
    //         return {VK_NULL_HANDLE, VK_NULL_HANDLE, allocInfo, false};
    //     }
        
    //     else if (buffer == VK_NULL_HANDLE && allocation != VK_NULL_HANDLE)
    //     {
    //         vmaDestroyBuffer(VyContext::allocator(), buffer, allocation);
            
    //         return {VK_NULL_HANDLE, VK_NULL_HANDLE, allocInfo, false};
    //     }
        
    //     else if (buffer != VK_NULL_HANDLE && allocation == VK_NULL_HANDLE)
    //     {
    //         vkDestroyBuffer(VyContext::device(), buffer, nullptr);

    //         return {VK_NULL_HANDLE, VK_NULL_HANDLE, allocInfo, false};
    //     }
    //     else
    //     {
    //         return {buffer, allocation, allocInfo, true};
    //     }
    // }
    // // bool Buffer::SyncCopy(const GraphicsCommandPool& commandPool, Buffer& src, Buffer& dst, size_t size)
    // // {
    // //     return SyncCopy(commandPool, src, dst, size, 0, 0);
    // // }
    // // bool Buffer::SyncCopy(const GraphicsCommandPool& commandPool, Buffer& src, Buffer& dst, size_t size,
    // //                     size_t srcOffset, size_t dstOffset)
    // // {
    // //     // TODO: error handle
    // //     auto fenceOpt = Fence::Create();
    // //     auto fence = std::move(fenceOpt.value());
    // //     auto commandBuffer = GraphicsCommandBuffer::Create(commandPool);
    // //     commandBuffer->BeginSingleTime();
    // //     VkBufferCopy copyRegion{};
    // //     copyRegion.srcOffset = srcOffset;
    // //     copyRegion.dstOffset = dstOffset;
    // //     copyRegion.size = size;
    // //     vkCmdCopyBuffer(commandBuffer->handle(), src.handle(), dst.handle(), 1, &copyRegion);
    // //     commandBuffer->End();
    // //     commandBuffer->BeginSubmit()
    // //         .Fence(fence)
    // //         .EndSubmit();
    // //     fence.Wait();
    // //     return true;
    // // }

    // std::optional<Buffer> Buffer::Create(size_t size, Usage usage, Property properties)
    // {
    //     auto handleRes = CreateHandle(size, usage, properties);
    //     if (handleRes.isOk == false)
    //     {
    //         return std::nullopt;
    //     }

    //     auto res = Buffer(handleRes.buffer, handleRes.allocation, size, usage, properties, handleRes.allocInfo);

    //     return res;
    // }

    // U32 Buffer::size() const
    // {
    //     return m_Size;
    // }

    // VkBuffer Buffer::handle() const
    // {
    //     return m_Handle;
    // }

    // Buffer::Usage Buffer::usage() const
    // {
    //     return m_Usage;
    // }

    // void Buffer::setData(const uint8_t* data, size_t size)
    // {
    //     if (isPersistent())
    //     {
    //         memcpy(m_AllocInfo.pMappedData, data, size);
    //     }
    //     else
    //     {
    //         void* pMappedData;
    //         vmaMapMemory(VyContext::allocator(), m_Allocation, &pMappedData);
    //         memcpy(pMappedData, data, size);
    //         vmaUnmapMemory(VyContext::allocator(), m_Allocation);
    //     }
    // }

    // Buffer::~Buffer()
    // {
    //     if (m_Handle != VK_NULL_HANDLE)
    //     {
    //         vmaDestroyBuffer(VyContext::allocator(), m_Handle, m_Allocation);
    //     }
    // }

    // Buffer::Buffer(Buffer&& other) noexcept
    // {
    //     m_Handle = other.m_Handle;
    //     m_Size = other.m_Size;
    //     m_Allocation = other.m_Allocation;
    //     m_Usage = other.m_Usage;
    //     m_AllocInfo = other.m_AllocInfo;
    //     other.m_Handle = VK_NULL_HANDLE;
    //     other.m_Allocation = VK_NULL_HANDLE;
    // }


    // Buffer& Buffer::operator=(Buffer&& other) noexcept
    // {
    //     if (this != &other)
    //     {
    //         if (m_Handle != VK_NULL_HANDLE)
    //         {
    //             vmaDestroyBuffer(VyContext::allocator(), m_Handle, m_Allocation);
    //         }

    //         m_Handle = other.m_Handle;
    //         m_Size = other.m_Size;
    //         m_Allocation = other.m_Allocation;
    //         m_Usage = other.m_Usage;
    //         m_AllocInfo = other.m_AllocInfo;
    //         other.m_Handle = VK_NULL_HANDLE;
    //         other.m_Allocation = VK_NULL_HANDLE;
    //     }
    //     return *this;
    // }

    // Buffer::Buffer(VkBuffer handle, VmaAllocation allocation, size_t size, Usage usage, Property properties, VmaAllocationInfo allocInfo) :
    //     m_Handle(handle), m_Size(size), m_Allocation(allocation), m_Usage(usage), m_Properties(properties), m_AllocInfo(allocInfo)
    // {
    // }

    // void* Buffer::beginMapImpl()
    // {
    //     if (isPersistent())
    //     {
    //         return m_AllocInfo.pMappedData;
    //     }
    //     else
    //     {
    //         void* pMappedData;
    //         vmaMapMemory(VyContext::allocator(), m_Allocation, &pMappedData);
    //         return pMappedData;
    //     }
    // }
    
    
    // void Buffer::endMapImpl()
    // {
    //     if (!isPersistent())
    //     {
    //         vmaUnmapMemory(VyContext::allocator(), m_Allocation);
    //     }
    // }


    // std::optional<Buffer> Buffer::CreateForSSBO(size_t size)
    // {
    //     return Create(size,
    //                 Buffer::PackUsages(Buffer::Usage::Storage, Buffer::Usage::TransferDst),
    //                 Buffer::PackProperties(Buffer::Property::DeviceLocal, Buffer::Property::HostVisible));
    // }


    // std::optional<Buffer> Buffer::CreateForUBO(size_t size)
    // {
    //     return Create(size,
    //                 Buffer::PackUsages(Buffer::Usage::Uniform, Buffer::Usage::TransferDst),
    //                 Buffer::PackProperties(Buffer::Property::DeviceLocal, Buffer::Property::HostVisible));
    // }


    // bool Buffer::SyncCopy(Buffer& src, Buffer& dst, size_t size, size_t srcOffset, size_t dstOffset)
    // {
    //     // return SyncCopy(GRC::GetGraphicsCommandPool(), src, dst, size, srcOffset, dstOffset);
    // }