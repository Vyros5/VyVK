#pragma once

#include <Vy/GFX/Backend/Device.h>

#include <Vy/GFX/Backend/Resources/Buffer.h>

namespace Vy
{
    class VyTexture;

// ================================================================================================
#pragma region [ Set Layout ]
// ================================================================================================

    using VyLayoutBindingMap      = THashMap<BindingIndex, VkDescriptorSetLayoutBinding>;
    using VyLayoutBindingFlagsMap = THashMap<BindingIndex, VkDescriptorBindingFlags>;
    
    /**
     * @brief Manages a Vulkan descriptor set layout.
     *
     * This class encapsulates the creation and lifetime of a `VkDescriptorSetLayout`,
     * using a builder interface to define descriptor bindings.
     */
    class VyDescriptorSetLayout 
    {
        friend class VyDescriptorWriter;

    public:
		// ----------------------------------------------------------------------------------------

        /**
         * @brief Builder class for creating a VyDescriptorSetLayout.
         *
         * This class allows for a more flexible and readable way to create a VyDescriptorSetLayout
         * by chaining method calls to set various parameters.
         */
        class Builder 
        {
        public:
            Builder() = default;

            /**
             * @brief Adds a descriptor binding to the layout.
             *
             * @param binding        Binding index (must be unique).
             * @param descriptorType Type of descriptor (e.g., uniform buffer, sampler).
             * @param stageFlags     Shader stages that will access the binding.
             * @param count          (Optional) Number of descriptors in the binding (default is 1).
             * 
             * @return Reference to the Builder for chaining.
             *
             * @note Throws an assertion failure if the binding is already in use.
             */
			Builder& addBinding(
				BindingIndex             binding, 
				VkDescriptorType         descriptorType, 
				VkShaderStageFlags       stageFlags,
				U32                      count        = 1,
                VkDescriptorBindingFlags bindingFlags = 0
			);


            Builder& setBindingFlags(BindingIndex binding, VkDescriptorBindingFlags flags);


            Builder& setLayoutFlags(VkDescriptorSetLayoutCreateFlags flags);

            
            /**
             * @brief Finalizes and builds the VyDescriptorSetLayout.
             *
             * @return A unique pointer to the created VyDescriptorSetLayout.
             * 
             * @throws std::runtime_error if Vulkan layout creation fails.
             */
			VY_NODISCARD
			Unique<VyDescriptorSetLayout> buildUnique() const;

            /**
             * @brief Finalizes and builds the VyDescriptorSetLayout.
             *
             * @return The created VyDescriptorSetLayout.
             * 
             * @throws std::runtime_error if Vulkan layout creation fails.
             */
			VY_NODISCARD
			VyDescriptorSetLayout build(); //const;

        private:
            VyLayoutBindingMap               m_Bindings{};
            VyLayoutBindingFlagsMap          m_BindingFlags{};
            VkDescriptorSetLayoutCreateFlags m_LayoutFlags = 0;
        };

		// ----------------------------------------------------------------------------------------

        VyDescriptorSetLayout(VyLayoutBindingMap bindings);

        VyDescriptorSetLayout(
            VyLayoutBindingMap               bindings, 
            VyLayoutBindingFlagsMap          bindingFlags, 
            VkDescriptorSetLayoutCreateFlags layoutFlags
        );
        
		VyDescriptorSetLayout(const VyDescriptorSetLayout&) = delete;
		VyDescriptorSetLayout(VyDescriptorSetLayout&& other) noexcept;
		
		~VyDescriptorSetLayout();

		VyDescriptorSetLayout& operator=(const VyDescriptorSetLayout&) = delete;
		VyDescriptorSetLayout& operator=(VyDescriptorSetLayout&& other) noexcept;

		operator VkDescriptorSetLayout() const { return m_SetLayout; }

        /**
         * @brief Returns the Vulkan descriptor set layout handle.
         * 
         * @return VkDescriptorSetLayout object.
         */
		VY_NODISCARD
		VkDescriptorSetLayout handle() const { return m_SetLayout; }

        VkDescriptorSet allocateDescriptorSet() const;

    private:
		VkDescriptorSetLayout m_SetLayout{ VK_NULL_HANDLE };
		VyLayoutBindingMap    m_Bindings;
    };

#pragma endregion [ Set Layout ]


// ================================================================================================
#pragma region [ Pool ]
// ================================================================================================

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
            Builder& addPoolSize(VkDescriptorType descriptorType, U32 count);
            
            /**
             * @brief Adds multiple pool sizes in one call.
			 * 
             * @param poolSizes Span of VkDescriptorPoolSize entries.
			 * 
             * @return Reference to the Builder for method chaining.
             */
            Builder& addPoolSizes(TSpan<VkDescriptorPoolSize> poolSizes);

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

		operator VkDescriptorPool() const { return m_Pool; }

        /**
        * @brief Retrieves the underlying Vulkan descriptor pool handle.
		* 
        * @return The VkDescriptorPool handle.
        */
		VkDescriptorPool handle() const { return m_Pool; }

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

#pragma endregion [ Pool ]


// ================================================================================================
#pragma region [ Writer ]
// ================================================================================================
    
    // class VyDescriptorWriter 
    // {
    // public:
    //     // VyDescriptorWriter(VyDescriptorSetLayout& setLayout);

    //     VyDescriptorWriter(VyDescriptorSetLayout& setLayout, VyDescriptorPool& pool);

	// 	VyDescriptorWriter(const VyDescriptorWriter&) = delete;
	// 	VyDescriptorWriter(VyDescriptorWriter&&)      = default;

	// 	~VyDescriptorWriter() = default;

	// 	VyDescriptorWriter& operator=(const VyDescriptorWriter&) = delete;
	// 	VyDescriptorWriter& operator=(VyDescriptorWriter&&)      = default;

    //     VyDescriptorWriter& writeImage(BindingIndex binding, const VyTexture& texture);
    //     VyDescriptorWriter& writeImage(BindingIndex binding, const VyTexture& texture, VkImageLayout layoutOverride);
	//     VyDescriptorWriter& writeImage(BindingIndex binding, VkDescriptorImageInfo imageInfo);

    //     VyDescriptorWriter& writeBuffer(BindingIndex binding, const VyBuffer& buffer);
    //     VyDescriptorWriter& writeBuffer(BindingIndex binding, const VyBuffer& buffer, U32 index);
	//     VyDescriptorWriter& writeBuffer(BindingIndex binding, VkDescriptorBufferInfo bufferInfo);
        
    //     /**
    //      * @brief Overwrites an existing descriptor set with the stored writes.
    //      * 
    //      * @param set Reference to the descriptor set to be overwritten.
    //      */
	// 	void update(VkDescriptorSet set);

    // private:
    //     VyDescriptorPool&                                   m_Pool;
    //     VyDescriptorSetLayout&                              m_SetLayout;
	// 	TVector<Pair<BindingIndex, VkDescriptorImageInfo>>  m_ImageInfos;
	// 	TVector<Pair<BindingIndex, VkDescriptorBufferInfo>> m_BufferInfos;
    // };


    class VyDescriptorWriter 
    {
    public:

        VyDescriptorWriter(VyDescriptorSetLayout& setLayout, VyDescriptorPool& pool);

        /**
         * @brief Writes a single buffer descriptor to the specified binding.
         * 
         * @param binding    The binding index.
         * @param bufferInfo Pointer to the buffer descriptor info.
         * 
         * @return Reference to the VyDescriptorWriter instance.
         */
        VyDescriptorWriter& writeBuffer(BindingIndex binding, VkDescriptorBufferInfo* bufferInfo) 
        {
            return write(binding, bufferInfo, 1);
        }
    
        /**
         * @brief Writes multiple buffer descriptors to the specified binding.
         * 
         * @param binding     The binding index.
         * @param buffersInfo Pointer to the array of buffer descriptor infos.
         * @param count       The number of descriptors.
         * 
         * @return Reference to the VyDescriptorWriter instance.
         */
        VyDescriptorWriter& writeBuffers(BindingIndex binding, VkDescriptorBufferInfo* buffersInfo, U32 count) 
        {
            return write(binding, buffersInfo, count);
        }
    
        /**
         * @brief Writes a single image descriptor to the specified binding.
         * 
         * @param binding   The binding index.
         * @param imageInfo Pointer to the image descriptor info.
         * 
         * @return Reference to the VyDescriptorWriter instance.
         */
        VyDescriptorWriter& writeImage(BindingIndex binding, VkDescriptorImageInfo* imageInfo) 
        {
            return write(binding, imageInfo, 1);
        }
    
        /**
         * @brief Writes multiple image descriptors to the specified binding.
         * 
         * @param binding    The binding index.
         * @param imagesInfo Pointer to the array of image descriptor infos.
         * @param count      The number of descriptors.
         * 
         * @return Reference to the VyDescriptorWriter instance.
         */
        VyDescriptorWriter& writeImages(BindingIndex binding, VkDescriptorImageInfo* imagesInfo, U32 count) 
        {
            return write(binding, imagesInfo, count);
        }

		/**
		 * @brief Writes a single acceleration structure descriptor to the specified binding.
		 *
		 * @param binding   The binding index.
		 * @param writeInfo Acceleration structure descriptor info.
		 *
		 * @return Reference to the VyDescriptorWriter instance.
		 */
        VyDescriptorWriter& writeTLAS(BindingIndex binding, VkWriteDescriptorSetAccelerationStructureKHR writeInfo) 
        {
			return write(binding, &writeInfo, 1);
        }


        bool build(VkDescriptorSet& set);

        /**
         * @brief Overwrites an existing descriptor set with the stored writes.
         * 
         * @param set Reference to the descriptor set to be overwritten.
         */
        void update(VkDescriptorSet& set);

    private:
        /**
         * @brief Generic template function to write descriptor data.
         * 
         * @tparam T Type of descriptor info (VkDescriptorBufferInfo or VkDescriptorImageInfo).
         * 
         * @param binding The binding index.
         * @param info    Pointer to descriptor info.
         * @param count   Number of descriptors.
         * 
         * @return Reference to the VyDescriptorWriter instance.
         */
        template <typename T>
        VyDescriptorWriter& write(U32 binding, T* info, U32 count) 
        {
			size_t bindingCount = m_SetLayout.m_Bindings.count(binding);

            VY_ASSERT(bindingCount == 1, 
                "Layout does not contain specified binding");
            
            auto& bindingDescription = m_SetLayout.m_Bindings[binding];
            
            VY_ASSERT(bindingDescription.descriptorCount == count, 
                "Binding descriptor info count mismatch");
            
            VkWriteDescriptorSet write{ VKInit::writeDescriptorSet() };
            {
                write.descriptorType  = bindingDescription.descriptorType;
                write.dstBinding      = binding;
                write.descriptorCount = count;
                
                if constexpr (std::is_same_v<T, VkDescriptorBufferInfo>) 
                {
                    write.pBufferInfo = info;
                } 
                else if constexpr (std::is_same_v<T, VkDescriptorImageInfo>) 
                {
                    write.pImageInfo  = info;
                } 
                else if constexpr (std::is_same_v<T, VkWriteDescriptorSetAccelerationStructureKHR>) 
                {
                    write.pNext       = info;
                } 
                else 
                {
                    VY_STATIC_ASSERT(false, "Unsupported type for descriptor write");
                }
            }
            
            m_Writes.push_back(write);

            return *this;
        }

        VyDescriptorSetLayout&        m_SetLayout;
        VyDescriptorPool&             m_Pool;
        TVector<VkWriteDescriptorSet> m_Writes;
    };

#pragma endregion [ Writer ]


// ================================================================================================
#pragma region [ Set ]
// ================================================================================================

	// class VyDescriptorSet
	// {
	// public:
	// 	// ----------------------------------------------------------------------------------------

	// 	class Builder
	// 	{
	// 	public:
	// 		Builder(VyDescriptorSetLayout& setLayout);

	// 		Builder(const Builder&) = delete;

	// 		~Builder() = default;

	// 		Builder& operator=(const Builder&) noexcept = delete;

	// 		Builder& addBuffer (BindingIndex binding, const VyBuffer&   buffer );
	// 		Builder& addTexture(BindingIndex binding, const VyTexture&  texture);
	// 		Builder& addTexture(BindingIndex binding, Shared<VyTexture> texture);
			
	// 		Unique<VyDescriptorSet> buildUnique();
	// 		VyDescriptorSet         build();

	// 	private:
	// 		VyDescriptorSetLayout& m_SetLayout;
	// 		VyDescriptorWriter     m_Writer;
	// 	}; // Builder

	// 	// ----------------------------------------------------------------------------------------

    //     // VyDescriptorSet() = default;

    //     /**
    //      * @brief Construct a new VyDescriptorSet object and allocate 
    //      *        the set from the Global Descriptor Pool in VyContext.
    //      * 
    //      * @param setLayout 
    //      */
	// 	VyDescriptorSet(VyDescriptorSetLayout& setLayout);

	// 	VyDescriptorSet(VyDescriptorSetLayout& setLayout, VyDescriptorPool& pool);

	// 	VyDescriptorSet(const VyDescriptorSet&) = delete;
	// 	VyDescriptorSet(VyDescriptorSet&&)      = default;

	// 	~VyDescriptorSet() = default;

	// 	VyDescriptorSet& operator=(const VyDescriptorSet&) = delete;
	// 	VyDescriptorSet& operator=(VyDescriptorSet&&)      = default;

	// 	operator VkDescriptorSet() const { return m_Set; }
	// 	VkDescriptorSet handle()   const { return m_Set; }

	// 	void bind(
	// 		VkCommandBuffer     cmdBuffer, 
	// 		VkPipelineLayout    pipelineLayout, 
	// 		VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS
	// 	) const;

	// private:
	// 	VkDescriptorSet m_Set{ VK_NULL_HANDLE };
	// };

#pragma endregion [ Set ]


// ================================================================================================
#pragma region [ Allocator ]
// ================================================================================================

    // class VyDescriptorAllocator 
    // {
    // public:
    //     static void initialize();

    //     static bool allocate(VkDescriptorSetLayout layout, VkDescriptorSet& set);

    //     static Shared<VyDescriptorPool> globalPool();

    //     static void cleanup();
        
    //     static void release(TVector<VkDescriptorSet>& sets);

    // private:
    //     static Shared<VyDescriptorPool> s_GlobalPool; // NB : this is better, less memory overhead. efficient - i like it
    // };


	/**
	 * @brief Represents a ratio for a specific descriptor type used when allocating descriptor pools.
	 *
	 * Used to determine how many descriptors of each type should be created
	 * relative to the total number of descriptor sets in a pool.
     * 
     * i.e. Describes the distribution of a descriptor type inside a pool.
	 */
	// struct PoolSizeRatio 
    // {
	// 	VkDescriptorType Type;
	// 	float            Ratio; // Distribution estimate
	// };

	/**
	 * @brief A dynamic descriptor pool allocator that grows based on demand.
	 *
	 * VyDescriptorAllocator manages Vulkan descriptor pools and allows for dynamic allocation
	 * of descriptor sets. When no existing pool can satisfy an allocation request, it automatically
	 * creates a new pool with increased capacity based on a configurable growth factor.
	 *
	 * The allocator maintains separate lists for ready and full pools, and can reset or clear them as needed.
	 *
	 * @note Pools grow until a user-defined maximum set count is reached.
	 */
	// class VyDescriptorAllocator 
    // {
	// public:

    //     /**
    //      * @brief Construct a new Vy Descriptor Allocator object
    //      * 
    //      * @param maxSets      Max Descriptor sets.
    //      * @param poolRatios   Pool sizes.
    //      * @param growthFactor (Optional) Growth factor (Default is 1.5f)
    //      * @param maxPools     (Optional) Max pools (Default is 4092)
    //      */
	// 	VyDescriptorAllocator(
    //         U32                  maxSets, 
    //         TSpan<PoolSizeRatio> poolRatios, 
    //         float                growthFactor = 1.5f, 
    //         U32                  maxPools     = 4092
    //     );

	// 	VyDescriptorAllocator(const VyDescriptorAllocator&) = delete;
	// 	VyDescriptorAllocator& operator=(const VyDescriptorAllocator&) = delete;

	// 	/**
	// 	 * @brief Allocates a descriptor set from a pool.
	// 	 *
	// 	 * If allocation from the first pool fails, it attempts again with a new pool.
	// 	 *
	// 	 * @param setLayout     Layout used for the descriptor set.
	// 	 * @param descriptorSet Reference to the descriptor set to be allocated.
    //      * 
	// 	 * @throws std::runtime_error If allocation fails from all pools.
	// 	 */
	// 	void allocate(
    //         VkDescriptorSetLayout setLayout, 
    //         VkDescriptorSet&      descriptorSet
    //     );


	// 	/**
	// 	 * @brief Resets all descriptor pools managed by the allocator.
	// 	 *
	// 	 * Moves all full pools back into the ready pool list after resetting them.
	// 	 */
	// 	void resetPools();

	// 	/**
	// 	 * @brief Clears all descriptor pools.
	// 	 *
	// 	 * Empties both ready and full pool lists.
	// 	 */
	// 	void clearPools();

	// private:
	// 	/**
	// 	 * @brief Grows the internal sets-per-pool count by the growth factor, clamped to maxPools.
	// 	 */
	// 	void growSetCount();

	// 	/**
	// 	 * @brief Retrieves an available descriptor pool or creates a new one.
	// 	 *
	// 	 * If no pools are ready, a new pool is created using the current growth parameters.
	// 	 *
	// 	 * @return A shared pointer to a VyDescriptorPool.
	// 	 */
	// 	Shared<VyDescriptorPool> getFreePool();

	// 	/**
	// 	 * @brief Creates a new descriptor pool based on the provided set count and ratios.
	// 	 *
	// 	 * @param setCount   Number of descriptor sets for the pool.
	// 	 * @param poolRatios Ratios used to determine pool sizes.
    //      * 
	// 	 * @return A shared pointer to a newly created VyDescriptorPool.
	// 	 */
	// 	VY_NODISCARD
	// 	Shared<VyDescriptorPool> createPool(U32 setCount, TSpan<PoolSizeRatio> poolRatios) const;

    //     /** Store the ratio of descriptor binding type for each set allocated from the pool. */
	// 	TVector<PoolSizeRatio> m_Ratios;

	// 	TVector<Shared<VyDescriptorPool>> m_FullPools;
	// 	TVector<Shared<VyDescriptorPool>> m_ReadyPools;

    //     // Descriptor sets for new pools.
	// 	U32   m_SetsPerPool;

	// 	float m_GrowthFactor;
	// 	U32   m_MaxPools;
	// };

#pragma endregion [ Allocator ]



    // class DescriptorAllocator {
    // public:
    //     struct PoolSizeRatio {
    //         VkDescriptorType type;
    //         float ratio;
    //     };

    //     const std::vector<PoolSizeRatio> DEFAULT_SIZE_CONFIG =
    //     {
    //             { VK_DESCRIPTOR_TYPE_SAMPLER, 0.5f },
    //             { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 4.f },
    //             { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1.f },
    //             { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1.f },
    //             { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1.f },
    //             { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2.f },
    //             { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2.f },
    //             { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1.f },
    //             { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1.f },
    //             { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 0.5f }
    //     };

    //     static constexpr U32 SET_LIMIT_PER_POOL = 4096u;

    //     //! Initialize the Descriptor allocator, which will create descriptor pools under the hood
    //     //! \param device
    //     //! \param initialSets The base amount of pool descriptor size
    //     //! \param poolRatios The descriptor type ration setup from PoolSizeRatio
    //     //! \return
    //     bool init(U32 initialSets = 4);

    //     //! Clears all pools
    //     void clear();

    //     //! Destroys all pools
    //     void destroy();

    //     //! Get a free pool
    //     //! TODO: [FEATURE] Be able to get pools by their flag?
    //     // \return The descriptor pool
    //     VkDescriptorPool getPool();

    //     /// Allocate a descriptor set
    //     /// \param layout descriptor layout
    //     /// \return allocated set, VK_NULL_HANDLE if there was an error
    //     VkDescriptorSet allocate(VkDescriptorSetLayout layout);
    // private:
    //     //! Create a pool
    //     //! \param device
    //     //! \param setCount The set count from which the number of pool type objects will be determined
    //     //! \param poolRatios pool types themselves
    //     //! \return
    //     VkDescriptorPool createPool(U32 setCount, std::span<PoolSizeRatio> poolRatios);
    // private:

    //     std::vector<PoolSizeRatio> m_sizeRatios;
    //     std::vector<VkDescriptorPool> m_fullPools;
    //     std::vector<VkDescriptorPool> m_readyPools;

    //     U32 m_setsPerPool = 1u;
    // };
}



















// namespace Vy
// {
//     class VyDescriptorSetLayout
//     {
//     public:
//         class Builder
//         {
//         public:
//             Builder() = default;

//             Builder& addBinding(BindingIndex binding, VkDescriptorType descriptorType, VkShaderStageFlags stageFlags, U32 count = 1);
//             Unique<VyDescriptorSetLayout> buildUnique() const;

//         private:
//             VyLayoutBindingMap m_Bindings{};
//         };

//     public:
//         VyDescriptorSetLayout(VyLayoutBindingMap bindings);

//         ~VyDescriptorSetLayout();
        
//         VyDescriptorSetLayout(const VyDescriptorSetLayout&)			   = delete;
//         VyDescriptorSetLayout& operator=(const VyDescriptorSetLayout&) = delete;

//         VkDescriptorSetLayout handle() const { return m_SetLayout; }

//     private:
//         VkDescriptorSetLayout m_SetLayout;
//         VyLayoutBindingMap    m_Bindings;

//         friend class VyDescriptorWriter;
//     };


    // class VyDescriptorPool
    // {
    // public:
    //     class Builder
    //     {
    //     public:
    //         Builder() = default;

    //         Builder& addPoolSize(VkDescriptorType descriptorType, U32 count);

    //         Builder& addPoolSizes(TSpan<VkDescriptorPoolSize> poolSizes);
            
    //         Builder& setPoolFlags(VkDescriptorPoolCreateFlags flags);
            
    //         Builder& setMaxSets(U32 count);
            
    //         Unique<VyDescriptorPool> buildUnique() const;

    //     private:
    //         TVector<VkDescriptorPoolSize> m_PoolSizes{};
    //         U32                           m_MaxSets   = 1000;
    //         VkDescriptorPoolCreateFlags   m_PoolFlags = 0;
    //     };

    //     VyDescriptorPool(U32 maxSets, VkDescriptorPoolCreateFlags poolFlags, const TVector<VkDescriptorPoolSize>& poolSizes);

    //     ~VyDescriptorPool();
        
    //     VyDescriptorPool(const VyDescriptorPool&)			 = delete;
    //     VyDescriptorPool& operator=(const VyDescriptorPool&) = delete;

    //     bool allocateDescriptorSet(const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptor) const;

    //     void freeDescriptors(TVector<VkDescriptorSet>& descriptors) const;

    //     void resetPool();

    // private:
    //     VkDescriptorPool m_Pool;

    //     friend class VyDescriptorWriter;
    // };

    // class VyDescriptorWriter
    // {
    // public:
    //     VyDescriptorWriter(VyDescriptorSetLayout& setLayout, VyDescriptorPool& pool);

    //     VyDescriptorWriter& writeBuffer(BindingIndex binding, VkDescriptorBufferInfo* bufferInfo, U32 count = 1);
    //     VyDescriptorWriter& writeImage(BindingIndex binding, VkDescriptorImageInfo* imageInfo, U32 count = 1);

    //     bool build(VkDescriptorSet& set);
    //     void update(VkDescriptorSet& set);

    // private:
    //     VyDescriptorSetLayout&          m_SetLayout;
    //     VyDescriptorPool&               m_Pool;
    //     TVector<VkWriteDescriptorSet> m_Writes;
    // };



//     class VyDescriptorWriter 
//     {
//     public:

//         VyDescriptorWriter(VyDescriptorSetLayout& setLayout);

//         /**
//          * @brief Writes a single buffer descriptor to the specified binding.
//          * 
//          * @param binding    The binding index.
//          * @param bufferInfo Pointer to the buffer descriptor info.
//          * 
//          * @return Reference to the VyDescriptorWriter instance.
//          */
//         VyDescriptorWriter& writeBuffer(BindingIndex binding, VkDescriptorBufferInfo* bufferInfo) 
//         {
//             return write(binding, bufferInfo, 1);
//         }
    
//         /**
//          * @brief Writes multiple buffer descriptors to the specified binding.
//          * 
//          * @param binding     The binding index.
//          * @param buffersInfo Pointer to the array of buffer descriptor infos.
//          * @param count       The number of descriptors.
//          * 
//          * @return Reference to the VyDescriptorWriter instance.
//          */
//         VyDescriptorWriter& writeBuffers(BindingIndex binding, VkDescriptorBufferInfo* buffersInfo, U32 count) 
//         {
//             return write(binding, buffersInfo, count);
//         }
    
//         /**
//          * @brief Writes a single image descriptor to the specified binding.
//          * 
//          * @param binding   The binding index.
//          * @param imageInfo Pointer to the image descriptor info.
//          * 
//          * @return Reference to the VyDescriptorWriter instance.
//          */
//         VyDescriptorWriter& writeImage(BindingIndex binding, VkDescriptorImageInfo* imageInfo) 
//         {
//             return write(binding, imageInfo, 1);
//         }
    
//         /**
//          * @brief Writes multiple image descriptors to the specified binding.
//          * 
//          * @param binding    The binding index.
//          * @param imagesInfo Pointer to the array of image descriptor infos.
//          * @param count      The number of descriptors.
//          * 
//          * @return Reference to the VyDescriptorWriter instance.
//          */
//         VyDescriptorWriter& writeImages(BindingIndex binding, VkDescriptorImageInfo* imagesInfo, U32 count) 
//         {
//             return write(binding, imagesInfo, count);
//         }

// 		/**
// 		 * @brief Writes a single acceleration structure descriptor to the specified binding.
// 		 *
// 		 * @param binding   The binding index.
// 		 * @param writeInfo Acceleration structure descriptor info.
// 		 *
// 		 * @return Reference to the VyDescriptorWriter instance.
// 		 */
//         VyDescriptorWriter& writeTLAS(BindingIndex binding, VkWriteDescriptorSetAccelerationStructureKHR writeInfo) 
//         {
// 			return write(binding, &writeInfo, 1);
//         }

//         /**
//          * @brief Overwrites an existing descriptor set with the stored writes.
//          * 
//          * @param set Reference to the descriptor set to be overwritten.
//          */
//         void update(VkDescriptorSet& set);

//     private:
//         /**
//          * @brief Generic template function to write descriptor data.
//          * 
//          * @tparam T Type of descriptor info (VkDescriptorBufferInfo or VkDescriptorImageInfo).
//          * @param binding The binding index.
//          * @param info Pointer to descriptor info.
//          * @param count Number of descriptors.
//          * 
//          * @return Reference to the VyDescriptorWriter instance.
//          */
//         template <typename T>
//         VyDescriptorWriter& write(BindingIndex binding, T* info, U32 count) 
//         {
// 			size_t bindingCount = m_SetLayout.m_Bindings.count(binding);

//             VY_ASSERT(bindingCount == 1, 
//                 "Layout does not contain specified binding");
            
//             auto& bindingDesc = m_SetLayout.m_Bindings[binding];
            
//             VY_ASSERT(bindingDesc.descriptorCount == count, 
//                 "Binding descriptor info count mismatch");
            
//             VkWriteDescriptorSet write{ VKInit::writeDescriptorSet() };
//             {
//                 write.descriptorType  = bindingDesc.descriptorType;
//                 write.dstBinding      = binding;
//                 write.descriptorCount = count;
                
//                 if constexpr (std::is_same_v<T, VkDescriptorBufferInfo>) 
//                 {
//                     write.pBufferInfo = info;
//                 } 
//                 else if constexpr (std::is_same_v<T, VkDescriptorImageInfo>) 
//                 {
//                     write.pImageInfo = info;
//                 } 
//                 else if constexpr (std::is_same_v<T, VkWriteDescriptorSetAccelerationStructureKHR>) 
//                 {
//                     write.pNext = info;
//                 } 
//                 else 
//                 {
//                     VY_STATIC_ASSERT(false, "Unsupported type for descriptor write");
//                 }
//             }
            
//             m_Writes.push_back(write);
//             return *this;
//         }

//         VyDescriptorSetLayout&        m_SetLayout;
//         TVector<VkWriteDescriptorSet> m_Writes;
//     };












// namespace Vy
// {
// 	class DescriptorLayoutCache;
// 	class DescriptorSet;

// 	class DescriptorSetLayout
// 	{
// 	public:
// 		class Builder 
//         {
// 		public:
// 			Builder(DescriptorLayoutCache& cache) : m_Cache(cache) {}

// 			Builder& AddBinding(
// 				BindingIndex binding,
// 				VkDescriptorType descriptorType,
// 				VkShaderStageFlags stageFlags,
// 				U32 count = 1
//             );
			
//             Shared<DescriptorSetLayout> build() const;

// 		private:
// 			DescriptorLayoutCache&                m_Cache;
// 			TVector<VkDescriptorSetLayoutBinding> m_Bindings{};
// 		};

// 		DescriptorSetLayout(const TVector<VkDescriptorSetLayoutBinding>& bindings);
		
//         ~DescriptorSetLayout();

// 		DescriptorSetLayout(const DescriptorSetLayout&) = delete;
// 		DescriptorSetLayout& operator=(const DescriptorSetLayout&) = delete;
		
//         DescriptorSetLayout(DescriptorSetLayout&& other) noexcept;
// 		DescriptorSetLayout& operator=(DescriptorSetLayout&& other) noexcept;

// 		VkDescriptorSetLayout handle() const { return m_SetLayout; }
// 	private:

// 		VkDescriptorSetLayout                 m_SetLayout{};
// 		TVector<VkDescriptorSetLayoutBinding> m_Bindings;

// 		friend class DescriptorSet;
// 	};



// 	class DescriptorLayoutCache
// 	{
// 	public:
// 		DescriptorLayoutCache() = default;

// 		Shared<DescriptorSetLayout> createLayout(const TVector<VkDescriptorSetLayoutBinding>& bindings);

// 		struct DescriptorLayoutInfo 
//         {
// 			TVector<VkDescriptorSetLayoutBinding> Bindings;

// 			bool operator==(const DescriptorLayoutInfo& other) const;

// 			size_t hash() const;
// 		};

// 		friend class DescriptorSetLayout;

// 	private:

// 		struct DescriptorLayoutHash
// 		{
// 			std::size_t operator()(const DescriptorLayoutInfo& k) const
// 			{
// 				return k.hash();
// 			}
// 		};

// 		THashMap<DescriptorLayoutInfo, Shared<DescriptorSetLayout>, DescriptorLayoutHash> m_LayoutCache;
// 	};



// 	class DescriptorPool
// 	{
// 	public:
// 		class Builder 
//         {
// 		public:
// 			Builder() = default;

// 			Builder& addPoolSize(VkDescriptorType descriptorType, U32 count);
// 			Builder& setPoolFlags(VkDescriptorPoolCreateFlags flags);
// 			Builder& setMaxSets(U32 count);
			
//             DescriptorPool build() const;

// 		private:
// 			TVector<VkDescriptorPoolSize> m_PoolSizes{};
// 			U32                           m_MaxSets = 1000;
// 			VkDescriptorPoolCreateFlags   m_PoolFlags = 0;
// 		};

// 		DescriptorPool(
// 			U32                                  maxSets,
// 			VkDescriptorPoolCreateFlags          poolFlags,
// 			const TVector<VkDescriptorPoolSize>& poolSizes
//         );
		
//         ~DescriptorPool();

// 		DescriptorPool(const DescriptorPool&) = delete;
// 		DescriptorPool& operator=(const DescriptorPool&) = delete;
		
//         DescriptorPool(DescriptorPool&& other) noexcept;
// 		DescriptorPool& operator=(DescriptorPool&& other) noexcept;

// 		const VkDescriptorPool& handle() const { return m_Pool; }

// 		VkResult allocateDescriptor(const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptor) const;

// 		void freeDescriptors(TVector<DescriptorSet>& descriptors) const;
// 		void freeDescriptors(const TVector<VkDescriptorSet>& descriptors) const;

// 		void resetPool() const;

// 	private:
// 		VkDescriptorPool m_Pool {VK_NULL_HANDLE};

// 		friend class DescriptorSet;
// 	};


// 	class DescriptorAllocator
// 	{
// 	public:
// 		DescriptorAllocator() = default;

// 		void resetPools();
		
//         Optional<DescriptorSet> allocate(const DescriptorSetLayout& layout);

// 	private:
// 		DescriptorPool createPool(U32 count, VkDescriptorPoolCreateFlags flags) const;
// 		DescriptorPool& grabPool();

// 		DescriptorPool*         m_CurrentPool{};
// 		TVector<DescriptorPool> m_UsedPools;
// 		TVector<DescriptorPool> m_FreePools;

// 		const TVector<std::pair<VkDescriptorType, float>> m_PoolSizes
// 		{
// 			{ VK_DESCRIPTOR_TYPE_SAMPLER,                0.5f },
// 			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4.0f },
// 			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,          4.0f },
// 			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,          1.0f },
// 			{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,   1.0f },
// 			{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,   1.0f },
// 			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         2.0f },
// 			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,         2.0f },
// 			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1.0f },
// 			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1.0f },
// 			{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,       0.5f }
// 		};

// 		friend class DescriptorSet;
// 	};


// 	class DescriptorSet
// 	{
// 	public:
// 		class Writer
// 		{
// 		public:
// 			Writer(DescriptorAllocator& allocator, DescriptorSetLayout& setLayout);
			
//             Writer& writeBuffer(U32 binding, const VkDescriptorBufferInfo *bufferInfo);
// 			Writer& writeImage(U32 binding, const VkDescriptorImageInfo& imageInfo);

// 			Unique<DescriptorSet> buildUnique();
			
//             void overwrite(const DescriptorSet& set);
	
// 		private:
// 			DescriptorAllocator&          m_Allocator;
// 			DescriptorSetLayout&          m_SetLayout;
// 			TVector<VkWriteDescriptorSet> m_Writes;
// 		};


// 		explicit DescriptorSet(DescriptorPool& descriptorPool) : m_Pool(descriptorPool) {}
		
//         virtual ~DescriptorSet();
		
//         DescriptorSet(DescriptorSet && other) noexcept;
// 		DescriptorSet& operator=(DescriptorSet&& other) noexcept;
		
//         DescriptorSet(const DescriptorSet&) = delete;
// 		DescriptorSet& operator=(const DescriptorSet&) = delete;
		
//         const VkDescriptorSet& handle() const { return m_Set; }
// 		VkDescriptorSet&       handle()       { return m_Set; }

// 	private:
// 		DescriptorPool& m_Pool;
// 		VkDescriptorSet m_Set = VK_NULL_HANDLE;

// 		friend class DescriptorPool;
// 	};
// }

























namespace Vy
{
    // struct VkGfxDescriptorSetLayout
    // {
    //     struct Builder
    //     {
    //         THashMap<U32, VkDescriptorSetLayoutBinding> BindingsMap  = {};
    //         THashMap<U32, VkDescriptorBindingFlags>     BindingFlags = {};
    //         String                                      DebugName    = {};

    //         Builder()
    //         {
    //         }

    //         /**
    //          * @brief add resource binding information
    //          * @param bindingIndex
    //          * @param descriptorType for the buffer/image resource
    //          * @param stageFlags
    //          * @param count if passing buffer/image arrays
    //          * @param isBindless whether this binding is bindless or not
    //          * @return Builder
    //          */
    //         Builder& addBinding(
    //             U32                bindingIndex,
    //             VkDescriptorType   descriptorType,
    //             VkShaderStageFlags stageFlags,
    //             U32                count,
    //             bool               isBindless = false
    //         );

    //         /**
    //          * @brief Give a debug name for the resource
    //          * @param name of the descriptor set layout
    //          * @return Builder
    //          */
    //         Builder& setDebugName(const String& name);

    //         /**
    //          * @brief create the descriptor set layout
    //          * @param ctx containing vulkan related information
    //          * @return unique pointer to the created object
    //          */
    //         Unique<VkGfxDescriptorSetLayout> build();
    //     };

    //     VkDescriptorSetLayout                       Layout      = VK_NULL_HANDLE;
    //     THashMap<U32, VkDescriptorSetLayoutBinding> BindingsMap = {};

    //     VkGfxDescriptorSetLayout() = delete;

    //     explicit VkGfxDescriptorSetLayout(THashMap<U32, VkDescriptorSetLayoutBinding> bindings) :
    //         BindingsMap(bindings)
    //     {
    //     }

    //     ~VkGfxDescriptorSetLayout();

    //     // CLASS_UNCOPYABLE(VkGfxDescriptorSetLayout);
    // };



    // struct VkGfxDescriptorSet
    // {
    //     VkDescriptorSet               Set    = VK_NULL_HANDLE;

    //     VkGfxDescriptorSetLayout&     SetLayout;
    //     TVector<VkWriteDescriptorSet> Writes;

    //     VkGfxDescriptorSet() = delete;

    //     explicit VkGfxDescriptorSet(VkGfxDescriptorSetLayout& layout) :
    //         SetLayout(layout)
    //     {
    //     }

    //     /**
    //      * @brief Configure buffer which is part of descriptor
    //      * @param binding number in the set
    //      * @param dstIndex index of the element which has to be configured
    //      * @param count number of descriptors to update
    //      * @param bufferInfo contains buffer details for linking to the descriptor
    //      * @return gfx descriptor set
    //      */
    //     VkGfxDescriptorSet& configureBuffer(
    //         U32                     binding,
    //         U32                     dstIndex,
    //         U32                     count,
    //         VkDescriptorBufferInfo* bufferInfo
    //     );

    //     /**
    //      * @brief Configure image which is part of descriptor
    //      * @param binding number in the set
    //      * @param dstIndex index of the element which has to be configured
    //      * @param count number of descriptors to update
    //      * @param imageInfo contains image deatils for linking to the descriptor
    //      * @return gfx descriptor set
    //      */
    //     VkGfxDescriptorSet& configureImage(
    //         U32                    binding,
    //         U32                    dstIndex,
    //         U32                    count,
    //         VkDescriptorImageInfo* imageInfo
    //     );

    //     /**
    //      * @brief apply buffer/image configurations on the set. It will overwrite any
    //      * existing configurations
    //      */
    //     void applyConfiguration();
    // };


    
    // struct VkGfxDescriptorPool
    // {
    //     struct Builder
    //     {
    //         TVector<VkDescriptorPoolSize> PoolSizes = {};
    //         String                        DebugName = {};

    //         Builder()
    //         {
    //         }

    //         /**
    //          * @brief
    //          * @param descriptorType type of the descriptor required from the pool
    //          * @param count for the descriptor of the given type which will be
    //          * available in the pool
    //          * @return
    //          */
    //         Builder& addPoolSize(VkDescriptorType descriptorType, U32 count);

    //         /**
    //          * @brief Give a debug name for the resource
    //          * @param name of the descriptor pool
    //          * @return Builder
    //          */
    //         Builder& setDebugName(const String& name);

    //         /**
    //          * @brief Build the pool for descriptor sets
    //          * @param maxSets is the maximum number of descriptor sets that can be
    //          * allocated from the pool.
    //          * @param poolFlags for pool creation
    //          * @return result of the api
    //          */
    //         Unique<VkGfxDescriptorPool> build(U32 maxSets, VkDescriptorPoolCreateFlags poolFlags = 0);
    //     };

    //     VkDescriptorPool Pool = VK_NULL_HANDLE;

    //     VkGfxDescriptorPool() = delete;

    //     explicit VkGfxDescriptorPool()
    //     {
    //     }

    //     ~VkGfxDescriptorPool();

    //     // CLASS_UNCOPYABLE(VkGfxDescriptorPool);

    //     /**
    //      * @brief Allocate one descriptor set from the pool
    //      * @param descriptorSetLayout will be used for the set allocation
    //      * @param debug name for the descriptor set
    //      * @return unique pointer to the allocated descriptor set
    //      */
    //     Unique<VkGfxDescriptorSet> allocateDescriptorSet(
    //         VkGfxDescriptorSetLayout& descriptorSetLayout,
    //         const char*               pDebugName = nullptr
    //     );

    //     /**
    //      * @brief Free on or more descriptor sets
    //      * @param descriptorSets which are needed to be freed
    //      */
    //     void freeDescriptorSets(TVector<VkDescriptorSet>& descriptorSets) const;

    //     /**
    //      * @brief return all descriptor sets allocated from a given pool, rather than freeing
    //      * individual sets
    //      */
    //     void resetPool();
    // };
}