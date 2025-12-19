#pragma once

#include <Vy/GFX/Backend/Device.h>

namespace Vy
{
    /**
     * @brief Manages a Vulkan descriptor pool and handles descriptor set allocations.
     *
     * This class provides RAII-style management for `VkDescriptorPool` objects, including
     * creation, destruction, and descriptor set allocation. Descriptor sets can be allocated,
     * freed, or the entire pool can be reset.
     */
    class VyDescriptorPool 
    {
        friend class VyDescriptorWriter;

    public:
		// ----------------------------------------------------------------------------------------

        /**
         * @brief Builder class for creating a VyDescriptorPool.
         *
         * This class allows for a more flexible and readable way to create a VyDescriptorPool
         * by chaining method calls to set various parameters.
         */
        class Builder 
        {
        public:
            Builder() = default;

            /**
             * @brief Adds a single descriptor type and count to the pool configuration.
			 * 
             * @param descriptorType The type of descriptor (e.g., VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER).
             * @param count          Number of descriptors of the specified type.
			 * 
             * @return Reference to the Builder for method chaining.
             */
            Builder& addPoolSize(
                VkDescriptorType descriptorType, 
                U32              count
            );
            
            /**
             * @brief Adds multiple pool sizes in one call.
			 * 
             * @param poolSizes Span of VkDescriptorPoolSize entries.
			 * 
             * @return Reference to the Builder for method chaining.
             */
            Builder& addPoolSizes(
                TSpan<VkDescriptorPoolSize> poolSizes
            );

            /**
             * @brief Sets creation flags for the descriptor pool.
			 * 
             * @param flags Vulkan descriptor pool creation flags.
			 * 
             * @return Reference to the Builder for method chaining.
             */
			Builder& setPoolFlags(VkDescriptorPoolCreateFlags flags);

            /**
             * @brief Sets the maximum number of descriptor sets the pool can allocate.
			 * 
             * @param count The max number of descriptor sets.
			 * 
             * @return Reference to the Builder for method chaining.
             */
			Builder& setMaxSets(U32 count);
			
            /**
             * @brief Builds and returns a unique VyDescriptorPool instance.
			 * 
             * @return A unique pointer to the created VyDescriptorPool.
             */
			VY_NODISCARD
			Unique<VyDescriptorPool> buildUnique() const;

            /**
             * @brief Builds and returns a unique VyDescriptorPool instance.
			 * 
             * @return The created VyDescriptorPool.
             */
			VY_NODISCARD
			VyDescriptorPool build() const;

        private:
			TVector<VkDescriptorPoolSize> m_PoolSizes{};
			U32                           m_MaxSets   = 1000;
			VkDescriptorPoolCreateFlags   m_PoolFlags = 0;
        };

		// ----------------------------------------------------------------------------------------

        VyDescriptorPool() = default;

        VyDescriptorPool(
            U32                                  maxSets,
            VkDescriptorPoolCreateFlags          poolFlags,
            const TVector<VkDescriptorPoolSize>& poolSizes
        );
        
		VyDescriptorPool(const VyDescriptorPool&) = delete;
		VyDescriptorPool(VyDescriptorPool&& other) noexcept;

		~VyDescriptorPool();

		VyDescriptorPool& operator=(const VyDescriptorPool&) = delete;
		VyDescriptorPool& operator=(VyDescriptorPool&& other) noexcept;

        /**
        * @brief Retrieves the underlying Vulkan descriptor pool handle.
		* 
        * @return The VkDescriptorPool handle.
        */
        VY_NODISCARD
		VkDescriptorPool handle() const { return m_Pool; }

        VY_NODISCARD
		operator VkDescriptorPool() const { return m_Pool; }

        /**
         * @brief Allocates a descriptor set from the pool.
         *
         * @param descriptorSetLayout Layout used for the descriptor set.
         * @param descriptor          Output reference to the allocated descriptor set.
		 * @param pNext               Optional pointer for extended allocation info (default: nullptr).
         * 
         * @return true if allocation succeeded; false if the pool is out of memory or fragmented.
         * @throws std::runtime_error on critical Vulkan allocation errors.
         */
		bool allocateDescriptorSet(
			const VkDescriptorSetLayout descriptorSetLayout, 
			VkDescriptorSet&            descriptor,
			const void*                 pNext = nullptr
		) const;

        /**
         * @brief Frees a batch of descriptor sets previously allocated from this pool.
		 * 
         * @param descriptors The descriptor sets to be freed.
         */
		void freeDescriptors(TVector<VkDescriptorSet>& descriptors) const;

        /**
         * @brief Resets the descriptor pool, freeing all descriptor sets.
         *
         * This allows reusing the pool without recreating it.
         */
		void resetPool();

    private:
        VkDescriptorPool m_Pool{ VK_NULL_HANDLE };
    };
}