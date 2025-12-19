#pragma once

#include <Vy/GFX/Backend/Device.h>

namespace Vy
{
    // TODO: Implement
    // https://registry.khronos.org/VulkanSC/specs/1.0-extensions/man/html/VkBufferUsageFlagBits.html
    enum class BufferUsageFlags : U32 
    {
        None                = 0,
        TransferSrc         = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        TransferDst         = VK_BUFFER_USAGE_TRANSFER_DST_BIT, 
        UniformTexelBuffer  = VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT,
        StorageTexelBuffer  = VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT,
        UniformBuffer       = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        StorageBuffer       = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
        IndexBuffer         = VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VertexBuffer        = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        IndirectBuffer      = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
        ShaderDeviceAddress = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
    };

    struct VyBufferDesc
    {
        VkDeviceSize             InstanceSize       { 0 };
        U32                      InstanceCount      { 1 };
        VkBufferUsageFlags       UsageFlags         { 0 };
        VmaAllocationCreateFlags AllocFlags         { 0 };
        VkDeviceSize             MinOffsetAlignment { 0 };
    };

    /**
     * @brief A class representing a Vulkan buffer.
     *
     * This class encapsulates the creation and management of Vulkan buffers.
     * 
     * It provides functionality for mapping, unmapping, writing data, 
     * flushing, and retrieving descriptor information. 
     * 
     * It also supports writing and managing data for individual instances within the buffer.
     */
    class VyBuffer 
    {
    public:
		/// @brief Factory methods for common buffer types
		/// @note Use multiple instances only if intended to be used with dynamic offsets (accessed by multiple descriptors)
        // ========================================================================================
        
        /**
         * @brief Factory Method for creating a descriptor for a Uniform Buffer object. (UBO)
         * 
         * @param instanceSize  The size of each instance within the buffer. (In Bytes)
         * @param instanceCount The number of instances in the buffer.
         * 
         * @return The created Uniform Buffer Description.
         */
		static VyBufferDesc uniformBuffer(VkDeviceSize instanceSize, U32 instanceCount = MAX_FRAMES_IN_FLIGHT);

        /**
         * @brief Factory Method for creating a descriptor for a Vertex Buffer object. (VBO)
         * 
         * @param instanceSize  The size of each instance within the buffer. (In Bytes)
         * @param instanceCount The number of instances in the buffer.
         * 
         * @return The created Vertex Buffer Description.
         */
		static VyBufferDesc vertexBuffer(VkDeviceSize instanceSize, U32 instanceCount = 1, VkBufferUsageFlags otherUsage = 0);

        /**
         * @brief Factory Method for creating a descriptor for an Index Buffer object. (IBO)
         * 
         * @param instanceSize  The size of each instance within the buffer. (In Bytes)
         * @param instanceCount The number of instances in the buffer.
         * 
         * @return The created Index Buffer Description.
         */
		static VyBufferDesc indexBuffer(VkDeviceSize instanceSize, U32 instanceCount = 1, VkBufferUsageFlags otherUsage = 0);

        /**
         * @brief Factory Method for creating a descriptor for a Storage Buffer object. (SBO) / (SSBO)
         * 
         * @param instanceSize  The size of each instance within the buffer. (In Bytes)
         * @param instanceCount The number of instances in the buffer.
         * 
         * @return The created Storage Buffer Description.
         */
		static VyBufferDesc storageBuffer(VkDeviceSize instanceSize, U32 instanceCount = 1, VkBufferUsageFlags otherUsage = 0);

        /**
         * @brief Factory Method for creating a Staging Buffer object.
         * 
         * @param instanceSize  The size of each instance within the buffer. (In Bytes)
         * 
         * @return The created Staging Buffer Description.
         */
		static VyBufferDesc stagingBuffer(VkDeviceSize instanceSize);

        /**
         * @brief Factory Method for creating a Staging Buffer object.
         * 
         * @param instanceSize  The size of each instance within the buffer. (In Bytes)
         * @param instanceCount The number of instances in the buffer.
         * 
         * @return The created Staging Buffer Description.
         */
		static VyBufferDesc stagingBuffer(VkDeviceSize instanceSize, U32 instanceCount, VmaAllocationCreateFlags otherFlags = 0);


        // ========================================================================================
        
        /**
         * @brief Constructor for the VyBuffer class.
         *
         * @param InstanceSize       The size of each instance within the buffer. (In Bytes)
         * @param InstanceCount      The number of instances in the buffer.
         * @param UsageFlags         Vulkan buffer usage flags.
         * @param AllocFlags         VMA allocation flags.
         * @param MinOffsetAlignment Minimum offset alignment for the buffer.
         */
        VyBuffer(const VyBufferDesc& desc, bool bPersistentMapped = true);

        /**
         * @brief Constructor for the VyBuffer class.
         *
         * @param instanceSize       The size of each instance within the buffer. (In Bytes)
         * @param instanceCount      The number of instances in the buffer.
         * @param usageFlags         Vulkan buffer usage flags.
         * @param allocFlags         VMA allocation flags.
         * @param minOffsetAlignment Minimum offset alignment for the buffer. (Optional)
         */
        VyBuffer(
            VkDeviceSize             instanceSize,
            U32                      instanceCount,
            VkBufferUsageFlags       usageFlags,
            VmaAllocationCreateFlags allocFlags,
            VkDeviceSize             minOffsetAlignment = 1
        );
        
        /**
         * @brief Deleted copy constructor.
         */
		VyBuffer(const VyBuffer&) = delete;

        /**
         * @brief Move constructor.
         */
		VyBuffer(VyBuffer&& other) noexcept;

        /**
         * @brief Deleted copy assignment operator.
         */
		VyBuffer& operator=(const VyBuffer&) = delete;

        /**
         * @brief Move assignment operator.
         */
		VyBuffer& operator=(VyBuffer&& other) noexcept;

        /**
         * @brief Destructor for the VyBuffer class.
         *
         * Releases the Vulkan buffer and memory.
         */
        ~VyBuffer();

        // ========================================================================================
        
        /**
         * @brief Returns the Vulkan buffer handle.
         *
         * @return The Vulkan buffer handle.
         */
		operator VkBuffer() const { return m_Buffer; }

        /**
         * @brief Returns the Vulkan buffer handle.
         *
         * @return The Vulkan buffer handle.
         */
		VY_NODISCARD VkBuffer handle() const { return m_Buffer; }

        /**
         * @brief Returns the total buffer size.
         *
         * @return The buffer size.
         */
		VY_NODISCARD VkDeviceSize bufferSize() const { return m_BufferSize; }

        /**
         * @brief Returns the alignment size for instances.
         *
         * @return The alignment size.
         */
		VY_NODISCARD VkDeviceSize alignmentSize() const { return m_AlignmentSize; }

        /**
         * @brief Returns the size of each instance.
         *
         * @return The instance size.
         */
		VY_NODISCARD VkDeviceSize instanceSize() const { return m_InstanceSize; }

        /**
         * @brief Returns the number of instances in the buffer.
         *
         * @return The number of instances.
         */
		VY_NODISCARD U32 instanceCount() const { return m_InstanceCount; }

        /**
         * @brief Returns the buffer usage flags.
         *
         * @return The buffer usage flags.
         */
		VY_NODISCARD VkBufferUsageFlags usage() const { return m_BufferUsage; }


        void* mappedData() const { return m_MappedData; }


        VkDeviceAddress deviceAddress() const;

        // ========================================================================================
        
        /**
         * @brief Returns the descriptor buffer info for the buffer.
         *
         * @param instanceSize   (Optional) Size of the memory range of the descriptor.
         * @param offset (Optional) Byte offset from beginning.
         *
         * @return VkDescriptorBufferInfo of specified offset and range.
         */
		VY_NODISCARD VkDescriptorBufferInfo descriptorBufferInfo(VkDeviceSize instanceSize = VK_WHOLE_SIZE, VkDeviceSize offset = 0) const;

        /**
         * @brief Create a buffer info descriptor.
         *
         * @param index Specifies the region given by index * alignmentSize.
         * 
         * @return VkDescriptorBufferInfo for instance at index.
         */
		VY_NODISCARD VkDescriptorBufferInfo descriptorBufferInfoForIndex(int index) const;

        // ========================================================================================
        
        /**
         * @brief Map the buffer memory to allow writing to it. 
         * 
         * If successful, `m_MappedData` points to the specified buffer range.
         */
		void map();
        
        /**
         * @brief Unmap the mapped buffer memory.
         */
        void unmap();

        // ========================================================================================
        
        /**
         * @brief Only writes data to the buffer.
         *
         * @note VyBuffer MUST be mapped before calling.
         * 
         * @param data Pointer to the data to copy.
         */
		void write(const void* data);

        /**
         * @brief Copies the specified data to the mapped buffer. Default value writes whole buffer range.
         *
         * @note VyBuffer MUST be mapped before calling.
         * 
         * @param data         Pointer to the data to copy.
         * @param instanceSize Size of the data to copy. Pass VK_WHOLE_SIZE to flush the complete buffer range.
         * @param offset       Byte offset from beginning of mapped region.
         */
		void write(const void* data, VkDeviceSize instanceSize, VkDeviceSize offset);

        void writeToBuffer(const void* data);
        void writeToBuffer(const void* data, VkDeviceSize instanceSize, VkDeviceSize offset = 0);

        /**
         * @brief Writes data to a specific instance within the buffer.
         * Copies 'instanceSize' bytes of data to the mapped buffer at an offset of `index * alignmentSize`.
         *
         * @note VyBuffer MUST be mapped before calling.
         * 
         * @param data  The data to write.
         * @param index The index of the instance to write to.
         */
		void writeToIndex(const void* data, int index);

        /**
         * @brief Maps, writes data, then unmaps the buffer.
         * 
         * @param data The data to write.
         */
		void singleWrite(const void* data);

        /**
         * @brief Maps, writes data, then unmaps the buffer.
         * 
         * @param data         The data to write.
         * @param instanceSize Size of the data to copy.
         * @param offset       Byte offset from beginning of mapped region.
         */
		void singleWrite(const void* data, VkDeviceSize instanceSize, VkDeviceSize offset);

        // ========================================================================================
        
        /**
         * @brief Flushes the mapped memory range of the buffer to make it visible to the device.
         *
         * @note Only required for non-coherent memory, 'write' functions already flush.
         * 
         * @param instanceSize (Optional) Size of the memory range to flush. Pass VK_WHOLE_SIZE to flush the complete buffer range.
         * @param offset       (Optional) The offset from the beginning of the buffer to flush.
         */
		void flush(VkDeviceSize instanceSize = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

        /**
         * @brief Flush the memory range at `index * alignmentSize` of the buffer to make it visible to the device.
         *
         * @note Only required for non-coherent memory, 'write' functions already flush.
         * 
         * @param index Used in offset calculation.
         */
		void flushIndex(int index);

        // ========================================================================================

        /**
         * Invalidate a memory range of the buffer to make it visible to the host.
         *
         * @note Only required for non-coherent memory.
         *
         * @param size   (Optional) Size of the memory range to invalidate. Pass VK_WHOLE_SIZE to invalidate the complete buffer range.
         * @param offset (Optional) Byte offset from the beginning.
         */
        void invalidate(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

        /**
        * Invalidate a memory range of the buffer to make it visible to the host.
        *
        * @note Only required for non-coherent memory.
        *
        * @param index Specifies the region to invalidate: index * m_AlignmentSize.
        */
        void invalidateIndex(int index);

        // ========================================================================================

        /**
         * @brief Uploads data to the buffer using a staging buffer (Used for device local memory).
         *
         * @param data The data to write.
         * @param instanceSize The size of the data to copy. (In Bytes)
         */
		void upload(const void* data, VkDeviceSize instanceSize);

        
		template<typename T>
		void upload(const TVector<T>& data)
		{
			VY_ASSERT(!data.empty(),                           "Data vector is empty");
			VY_ASSERT(sizeof(T) * data.size() <= m_BufferSize, "Data size exceeds buffer size");

			upload(data.data(), sizeof(T) * data.size());
		}


        /**
         * @brief Copy the buffer to another buffer.
         *
         * @param dest The VyBuffer to copy to.
         * @param instanceSize The size of the data to copy.
         */
		void copyTo(VyBuffer& dest, VkDeviceSize instanceSize);

    private:
        /**
         * @brief Calculates the aligned instance size.
         * 
         * Returns the minimum instance size required to be compatible with devices minOffsetAlignment
         * (rounds up instanceSize to the next multiple of minOffsetAlignment, if 1 -> returns instanceSize
         *
         * @param instanceSize       The original instance size.
         * @param minOffsetAlignment The minimum required alignment, in bytes, for the offset member.
         *                           (eg. minUniformBufferOffsetAlignment)
         * 
         * @return The alignment size.
         */
		static VkDeviceSize computeAlignment(VkDeviceSize instanceSize, VkDeviceSize minOffsetAlignment);

        void moveFrom(VyBuffer&& other);

		void destroy();

    private:

		VkBuffer           m_Buffer        { VK_NULL_HANDLE };
		VmaAllocation      m_Allocation    { VK_NULL_HANDLE };
		VkDeviceSize       m_BufferSize    { 0 };
		VkDeviceSize       m_InstanceSize  { 0 };
		VkDeviceSize       m_AlignmentSize { 0 };
		U32                m_InstanceCount { 0 };
		VkBufferUsageFlags m_BufferUsage   { 0 };
		void*              m_MappedData    { nullptr };

        bool               m_IsPersistentMapped{ false };
    };
}