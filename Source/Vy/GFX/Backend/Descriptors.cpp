#include <Vy/GFX/Backend/Descriptors.h>

#include <Vy/GFX/Context.h>
#include <Vy/GFX/Resources/Texture.h>

#include <ranges>

namespace Vy
{

// ================================================================================================
#pragma region [ Set Layout Builder ]
// ================================================================================================
	
	VyDescriptorSetLayout::Builder& 
	VyDescriptorSetLayout::Builder::addBinding(
		BindingIndex             binding, 
		VkDescriptorType         descriptorType,
		VkShaderStageFlags       stageFlags, 
		U32                      count,
		VkDescriptorBindingFlags bindingFlags)
    {
        VY_ASSERT(m_Bindings.count(binding) == 0, 
			"Binding already in use");

		VkDescriptorSetLayoutBinding layoutBinding{};
		{
			layoutBinding.binding         = binding;
			layoutBinding.descriptorType  = descriptorType;
			layoutBinding.descriptorCount = count;
			layoutBinding.stageFlags      = stageFlags;
		}

		m_Bindings    [ binding ] = layoutBinding;
		m_BindingFlags[ binding ] = bindingFlags;

        return *this;
    }


    VyDescriptorSetLayout::Builder& 
	VyDescriptorSetLayout::Builder::setBindingFlags(
		BindingIndex             binding, 
		VkDescriptorBindingFlags flags) 
	{
        VY_ASSERT(m_Bindings.count(binding) == 1, 
			"Binding does not exist");

        m_BindingFlags[ binding ] = flags;
        
		return *this;
    }


    VyDescriptorSetLayout::Builder& 
	VyDescriptorSetLayout::Builder::setLayoutFlags(
		VkDescriptorSetLayoutCreateFlags flags) 
	{
        m_LayoutFlags = flags;

        return *this;
    }

	
	Unique<VyDescriptorSetLayout> 
	VyDescriptorSetLayout::Builder::buildUnique() const
	{
		return std::make_unique<VyDescriptorSetLayout>(m_Bindings);
	}


	VyDescriptorSetLayout 
	VyDescriptorSetLayout::Builder::build() //const
	{
		return VyDescriptorSetLayout{ m_Bindings };
	}

#pragma endregion [ Set Layout Builder ]


// ================================================================================================
#pragma region [ Set Layout ]
// ================================================================================================

    VyDescriptorSetLayout::VyDescriptorSetLayout(
        VyLayoutBindingMap bindings
    ) : 
        m_Bindings{ bindings } 
    {
		TVector<VkDescriptorSetLayoutBinding> setLayoutBindings;
		setLayoutBindings.reserve(m_Bindings.size());

        for (auto kv : bindings) 
        {
            setLayoutBindings.push_back(kv.second);
        }

        VkDescriptorSetLayoutCreateInfo setLayoutInfo{ VKInit::descriptorSetLayoutCreateInfo() };
        {
            setLayoutInfo.bindingCount = static_cast<U32>(setLayoutBindings.size());
            setLayoutInfo.pBindings    = setLayoutBindings.data();
        }

		VK_CHECK(vkCreateDescriptorSetLayout(VyContext::device(), &setLayoutInfo, nullptr, &m_SetLayout));
    }


	VyDescriptorSetLayout::VyDescriptorSetLayout(
		VyLayoutBindingMap               bindings, 
		VyLayoutBindingFlagsMap          bindingFlags, 
		VkDescriptorSetLayoutCreateFlags layoutFlags
    ) : 
        m_Bindings{ bindings } 
    {
		TVector<VkDescriptorSetLayoutBinding> setLayoutBindings;
		TVector<VkDescriptorBindingFlags>     flags{};
		setLayoutBindings.reserve(m_Bindings.size());

        for (auto kv : bindings) 
        {
            setLayoutBindings.push_back(kv.second);

            // Add flags for this binding (0 if not specified).
            auto it = bindingFlags.find(kv.first);

            flags.push_back(it != bindingFlags.end() ? it->second : 0);
        }

        auto bindingFlagsInfo{ VKInit::descriptorSetLayoutBindingFlagsCreateInfo() };
		{
			bindingFlagsInfo.bindingCount  = static_cast<U32>(flags.size());
			bindingFlagsInfo.pBindingFlags = flags.data();
		}


        auto setLayoutInfo{ VKInit::descriptorSetLayoutCreateInfo() };
        {
            setLayoutInfo.bindingCount = static_cast<U32>(setLayoutBindings.size());
            setLayoutInfo.pBindings    = setLayoutBindings.data();
			setLayoutInfo.flags        = layoutFlags; // Set layout flags.

			// Only set pNext if we have binding flags.
			if (!bindingFlags.empty()) 
			{
				setLayoutInfo.pNext = &bindingFlagsInfo;
			}
        }

		VK_CHECK(vkCreateDescriptorSetLayout(VyContext::device(), &setLayoutInfo, nullptr, &m_SetLayout));
	}


	VyDescriptorSetLayout::VyDescriptorSetLayout(VyDescriptorSetLayout&& other) noexcept : 
		m_SetLayout{ other.m_SetLayout           }, 
		m_Bindings { std::move(other.m_Bindings) }
	{
		other.m_SetLayout = VK_NULL_HANDLE;
	}


	VyDescriptorSetLayout::~VyDescriptorSetLayout()
	{
		vkDestroyDescriptorSetLayout(VyContext::device(), m_SetLayout, nullptr);
	}


	VyDescriptorSetLayout& 
	VyDescriptorSetLayout::operator=(VyDescriptorSetLayout&& other) noexcept
	{
		if (this != &other)
		{
			m_SetLayout = other.m_SetLayout;
			m_Bindings  = std::move(other.m_Bindings);

			other.m_SetLayout = VK_NULL_HANDLE;
		}

		return *this;
	}


	VkDescriptorSet 
	VyDescriptorSetLayout::allocateDescriptorSet() const
	{
		VkDescriptorSet descriptorSet{ VK_NULL_HANDLE };

		// VyContext::descriptorPool().allocateDescriptorSet(m_SetLayout, descriptorSet);
		VyContext::allocateSet(m_SetLayout, descriptorSet);
		
		return descriptorSet;
	}

#pragma endregion [ Set Layout ]


// ================================================================================================
#pragma region [ Pool Builder ]
// ================================================================================================

	VyDescriptorPool::Builder& 
	VyDescriptorPool::Builder::addPoolSize(
        VkDescriptorType descriptorType, 
        U32              count)
	{
		m_PoolSizes.push_back({ descriptorType, count });
		
		return *this;
	}


	VyDescriptorPool::Builder& 
	VyDescriptorPool::Builder::addPoolSizes(
		TSpan<VkDescriptorPoolSize> poolSizes)
	{
        for (auto& poolSize : poolSizes) 
		{
            m_PoolSizes.emplace_back(poolSize);
        }
		
        return *this;
	}


	VyDescriptorPool::Builder& 
	VyDescriptorPool::Builder::setPoolFlags(
		VkDescriptorPoolCreateFlags flags)
	{
		m_PoolFlags = flags;
		
		return *this;
	}


	VyDescriptorPool::Builder& 
	VyDescriptorPool::Builder::setMaxSets(U32 count)
	{
		m_MaxSets = count;
		
		return *this;
	}


	Unique<VyDescriptorPool> 
	VyDescriptorPool::Builder::buildUnique() const
	{
		return std::make_unique<VyDescriptorPool>(m_MaxSets, m_PoolFlags, m_PoolSizes);
	}


	VyDescriptorPool 
	VyDescriptorPool::Builder::build() const
	{
		return VyDescriptorPool{ m_MaxSets, m_PoolFlags, m_PoolSizes };
	}

#pragma endregion [ Pool Builder ]


// ================================================================================================
#pragma region [ Pool ]
// ================================================================================================

    VyDescriptorPool::VyDescriptorPool(
		U32                                  maxSets, 
		VkDescriptorPoolCreateFlags          poolFlags,
		const TVector<VkDescriptorPoolSize>& poolSizes)
    {
		auto poolInfo{ VKInit::descriptorPoolCreateInfo() };
		{
			poolInfo.poolSizeCount = static_cast<U32>(poolSizes.size());
			poolInfo.pPoolSizes    = poolSizes.data();

			poolInfo.maxSets       = maxSets;
			poolInfo.flags         = poolFlags;
		}

		VK_CHECK(vkCreateDescriptorPool(VyContext::device(), &poolInfo, nullptr, &m_Pool));
    }


	VyDescriptorPool::VyDescriptorPool(VyDescriptorPool&& other) noexcept : 
		m_Pool{ other.m_Pool }
	{
		other.m_Pool = VK_NULL_HANDLE;
	}


	VyDescriptorPool::~VyDescriptorPool()
	{
		if (m_Pool)
		{
			vkDestroyDescriptorPool(VyContext::device(), m_Pool, nullptr);
		}
	}


	VyDescriptorPool& 
	VyDescriptorPool::operator=(VyDescriptorPool&& other) noexcept
	{
		if (this != &other)
		{
			m_Pool       = other.m_Pool;
			other.m_Pool = VK_NULL_HANDLE;
		}

		return *this;
	}


    bool VyDescriptorPool::allocateDescriptorSet(
		const VkDescriptorSetLayout setLayout, 
		VkDescriptorSet&            descriptor,
		const void*                 pNext) const
    {
		// TODO: Might want to create a "VyDescriptorPoolManager" class to handle this case, 
  		// and builds a new pool whenever an old pool fills up. 

		auto allocInfo{ VKInit::descriptorSetAllocateInfo() };
		{
			allocInfo.descriptorPool     = m_Pool;
			allocInfo.pSetLayouts        = &setLayout;
			allocInfo.descriptorSetCount = 1;
			allocInfo.pNext              = pNext;
		}

		VkResult result = vkAllocateDescriptorSets(VyContext::device(), &allocInfo, &descriptor);

		if (result == VK_ERROR_FRAGMENTED_POOL)
		{
            VY_ERROR("Error: Fragmentated pool.");
			return false;
		}

		if (result == VK_ERROR_OUT_OF_POOL_MEMORY) 
		{
			VY_ERROR("Error: Descriptor pool ran out of memory.");
			return false;
		}

		if (result == VK_ERROR_FRAGMENTATION)
		{
			VY_ERROR("Error: Fragmentation of pool memory while allocating descriptor sets.");
			return false;
		}

		if (result == VK_ERROR_OUT_OF_DEVICE_MEMORY) 
		{
			VY_THROW_RUNTIME_ERROR("Failed to allocate descriptor set, device out of memory!");
		}

		if (result == VK_ERROR_OUT_OF_HOST_MEMORY) 
		{
			VY_THROW_RUNTIME_ERROR("Failed to allocate descriptor set, host out of memory!");
		}

		if (result != VK_SUCCESS) 
		{
			VY_THROW_RUNTIME_ERROR("Failed to allocate descriptor set!");
		}
	
        return true;
    }


	void VyDescriptorPool::freeDescriptors(
		TVector<VkDescriptorSet>& descriptors) const
	{
		vkFreeDescriptorSets(VyContext::device(), m_Pool, static_cast<U32>(descriptors.size()), descriptors.data());
	}


	void VyDescriptorPool::resetPool()
	{
		vkResetDescriptorPool(VyContext::device(), m_Pool, 0);
	}

#pragma endregion [ Pool ]


// ================================================================================================
#pragma region [ Writer ]
// ================================================================================================

    // VyDescriptorWriter::VyDescriptorWriter(
    //     VyDescriptorSetLayout& setLayout
    // ) : 
    //     m_SetLayout{ setLayout }
    // {
    // }


	// VyDescriptorWriter::VyDescriptorWriter(
	// 	VyDescriptorSetLayout& setLayout, 
	// 	VyDescriptorPool&      pool
    // ) : 
    //     m_SetLayout{ setLayout },
	// 	m_Pool     { pool      }
    // {
    // }


	// VyDescriptorWriter& 
	// VyDescriptorWriter::writeBuffer(
	// 	BindingIndex    binding, 
	// 	const VyBuffer& buffer)
	// {
	// 	m_BufferInfos.emplace_back(binding, buffer.descriptorBufferInfo());

	// 	return *this;
	// }


	// VyDescriptorWriter& 
	// VyDescriptorWriter::writeBuffer(
	// 	BindingIndex    binding, 
	// 	const VyBuffer& buffer, 
	// 	U32             index)
	// {
	// 	m_BufferInfos.emplace_back(binding, buffer.descriptorBufferInfoForIndex(index));

	// 	return *this;
	// }


    // VyDescriptorWriter& 
    // VyDescriptorWriter::writeBuffer(
    //     BindingIndex           binding, 
    //     VkDescriptorBufferInfo bufferInfo) 
    // {
    //     VY_ASSERT(m_SetLayout.m_Bindings.count(binding) == 1, 
	// 		"Layout does not contain specified binding");

    //     m_BufferInfos.emplace_back(binding, std::move(bufferInfo));

    //     return *this;
    // }


	// VyDescriptorWriter& 
	// VyDescriptorWriter::writeImage(
	// 	BindingIndex     binding, 
	// 	const VyTexture& texture)
	// {
	// 	m_ImageInfos.emplace_back(binding, texture.descriptorImageInfo());

	// 	return *this;
	// }


	// VyDescriptorWriter& 
	// VyDescriptorWriter::writeImage(
	// 	BindingIndex     binding, 
	// 	const VyTexture& texture, 
	// 	VkImageLayout    layoutOverride)
	// {
	// 	m_ImageInfos.emplace_back(binding, texture.descriptorImageInfo(layoutOverride));

	// 	return *this;
	// }


    // VyDescriptorWriter& VyDescriptorWriter::writeImage(
    //     BindingIndex          binding, 
    //     VkDescriptorImageInfo imageInfo) 
    // {
    //     VY_ASSERT(m_SetLayout.m_Bindings.count(binding) == 1, 
	// 		"Layout does not contain specified binding");

	// 	m_ImageInfos.emplace_back(binding, std::move(imageInfo));

    //     return *this;
    // }


	// void VyDescriptorWriter::update(VkDescriptorSet set)
	// {
	// 	// m_Pool.allocateDescriptorSet(m_SetLayout.handle(), set);

	// 	TVector<VkWriteDescriptorSet> writes;

	// 	writes.reserve(m_BufferInfos.size() + m_ImageInfos.size());

	// 	// Image Infos
	// 	for (const auto& [ binding, info ] : m_ImageInfos)
	// 	{
	// 		auto& bindingDesc = m_SetLayout.m_Bindings[binding];

	// 		VY_ASSERT(bindingDesc.descriptorCount == 1, 
	// 			"Cannot write multiple images to a single descriptor");

	// 		auto write{ VKInit::writeDescriptorSet() };
	// 		{
	// 			write.descriptorType  = bindingDesc.descriptorType;
	// 			write.dstBinding      = binding;
	// 			write.pImageInfo      = &info;
	// 			write.descriptorCount = 1;
	// 			write.dstSet          = set;
	// 		}
			
	// 		writes.emplace_back(write);
	// 	}

	// 	// Buffer Infos
	// 	for (const auto& [ binding, info ] : m_BufferInfos)
	// 	{
	// 		auto& bindingDesc = m_SetLayout.m_Bindings[binding];

	// 		VY_ASSERT(bindingDesc.descriptorCount == 1, 
	// 			"Cannot write multiple buffers to a single descriptor");

	// 		auto write{ VKInit::writeDescriptorSet() };
	// 		{
	// 			write.descriptorType  = bindingDesc.descriptorType;
	// 			write.dstBinding      = binding;
	// 			write.pBufferInfo     = &info;
	// 			write.descriptorCount = 1;
	// 			write.dstSet          = set;
	// 		}
			
	// 		writes.emplace_back(write);
	// 	}

	// 	vkUpdateDescriptorSets(VyContext::device(), static_cast<U32>(writes.size()), writes.data(), 0, nullptr);
	// }

#pragma endregion [ Writer ]


    VyDescriptorWriter::VyDescriptorWriter(
		VyDescriptorSetLayout& setLayout, 
		VyDescriptorPool&      pool
	) : 
        m_SetLayout{ setLayout },
		m_Pool     { pool      }
	{
	}


	bool VyDescriptorWriter::build(VkDescriptorSet& set)
	{
        bool success = m_Pool.allocateDescriptorSet(m_SetLayout.handle(), set);
        
		if (!success) 
		{
            return false;
        }
        
		update(set);
        
		return true;
	}


    void VyDescriptorWriter::update(VkDescriptorSet& set) 
	{
        for (auto& write : m_Writes) 
		{
            write.dstSet = set;
        }

		VY_DEBUG_TAG("VyDescriptorWriter", "Updated.");

        vkUpdateDescriptorSets(VyContext::device(), static_cast<U32>(m_Writes.size()), m_Writes.data(), 0, nullptr);
    }


// ================================================================================================
#pragma region [ Set Builder ]
// ================================================================================================

	// VyDescriptorSet::Builder::Builder(VyDescriptorSetLayout& setLayout) : 
	// 	m_SetLayout{ setLayout }, 
	// 	m_Writer   { setLayout }
	// {
	// }


	// VyDescriptorSet::Builder& 
	// VyDescriptorSet::Builder::addBuffer(
	// 	BindingIndex    binding, 
	// 	const VyBuffer& buffer)
	// {
	// 	m_Writer.writeBuffer(binding, buffer);

	// 	return *this;
	// }


	// VyDescriptorSet::Builder& 
	// VyDescriptorSet::Builder::addTexture(
	// 	BindingIndex     binding, 
	// 	const VyTexture& texture)
	// {
	// 	m_Writer.writeImage(binding, texture);

	// 	return *this;
	// }


	// VyDescriptorSet::Builder& 
	// VyDescriptorSet::Builder::addTexture(
	// 	BindingIndex      binding, 
	// 	Shared<VyTexture> texture)
	// {
	// 	VY_ASSERT(texture != nullptr, "Cannot add VyTexture if it is nullptr");

	// 	return addTexture(binding, *texture);
	// }


	// VyDescriptorSet
	// VyDescriptorSet::Builder::build()
	// {
	// 	auto set = VyDescriptorSet{ m_SetLayout };
		
	// 	m_Writer.update(set.handle());

	// 	return set;
	// }


	// Unique<VyDescriptorSet> 
	// VyDescriptorSet::Builder::buildUnique()
	// {
	// 	auto set = std::make_unique<VyDescriptorSet>(m_SetLayout);
	
	// 	m_Writer.update(set->handle());
	
	// 	return set;
	// }

#pragma endregion [ Set Builder ]


// ================================================================================================
#pragma region [ Set ]
// ================================================================================================

	// VyDescriptorSet::VyDescriptorSet(
	// 	VyDescriptorSetLayout& setLayout)
	// {
	// 	VyContext::descriptorPool().allocateDescriptorSet(setLayout.handle(), m_Set);
	// }


	// VyDescriptorSet::VyDescriptorSet(
	// 	VyDescriptorSetLayout& setLayout, 
	// 	VyDescriptorPool&      pool)
	// {
	// 	pool.allocateDescriptorSet(setLayout, m_Set);
	// }

	
	// void VyDescriptorSet::bind(
	// 	VkCommandBuffer     cmdBuffer, 
	// 	VkPipelineLayout    pipelineLayout, 
	// 	VkPipelineBindPoint bindPoint) const
	// {
	// 	vkCmdBindDescriptorSets(
	// 		cmdBuffer, 
	// 		bindPoint, 
	// 		pipelineLayout, 
	// 		0, 
	// 		1, 
	// 		&m_Set, 
	// 		0, 
	// 		nullptr
	// 	);
	// }

#pragma endregion [ Set ]


// ================================================================================================
#pragma region [ Allocator ]
// ================================================================================================

	// VyDescriptorAllocator::VyDescriptorAllocator(
	// 	const U32            maxSets,
	// 	TSpan<PoolSizeRatio> poolRatios, 
	// 	const float          growthFactor, 
	// 	const U32            maxPools
	// ) :
	// 	m_SetsPerPool { maxSets      },
	// 	m_GrowthFactor{ growthFactor },
	// 	m_MaxPools    { maxPools     } 
	// {
	// 	m_Ratios.reserve(poolRatios.size());

	// 	TVector<VkDescriptorPoolSize> poolSizes;
	// 	poolSizes.reserve(poolRatios.size());

	// 	for (auto& ratio : poolRatios) 
	// 	{
	// 		m_Ratios.emplace_back(ratio);

	// 		poolSizes.emplace_back(ratio.Type, static_cast<U32>(ratio.Ratio * maxSets));
	// 	}

	// 	Shared<VyDescriptorPool> newPool = createPool(maxSets, poolRatios);

	// 	// Growth for the next allocation.
	// 	growSetCount();

	// 	m_ReadyPools.push_back(newPool);
	// }


	// void VyDescriptorAllocator::allocate(
	// 	VkDescriptorSetLayout setLayout, 
	// 	VkDescriptorSet&      descriptorSet) 
	// {
	// 	Shared<VyDescriptorPool> pool = getFreePool();

	// 	bool isAllocationSuccessful = pool->allocateDescriptorSet(setLayout, descriptorSet);

	// 	if (!isAllocationSuccessful) 
	// 	{
	// 		// If the allocation failed, move the pool to the full pools list.
	// 		m_FullPools.push_back(pool);

	// 		// Try to allocate again from the next pool.
	// 		pool = getFreePool();

	// 		isAllocationSuccessful = pool->allocateDescriptorSet(setLayout, descriptorSet);

	// 		if (!isAllocationSuccessful) 
	// 		{
	// 			VY_THROW_RUNTIME_ERROR("Failed to allocate descriptor set!");
	// 		}
	// 	}

	// 	m_ReadyPools.push_back(pool);
	// }


	// void VyDescriptorAllocator::growSetCount() 
	// {
	// 	m_SetsPerPool = static_cast<U32>(m_SetsPerPool * m_GrowthFactor);

	// 	// Sets per pool is capped to the max pools.
	// 	m_SetsPerPool = std::min(m_SetsPerPool, m_MaxPools);
	// }


	// Shared<VyDescriptorPool>
	// VyDescriptorAllocator::getFreePool() 
	// {
	// 	if (!m_ReadyPools.empty()) 
	// 	{
	// 		// Get the last ready pool.
	// 		Shared<VyDescriptorPool> pool = m_ReadyPools.back();

	// 		// Remove it from the ready pool list.
	// 		m_ReadyPools.pop_back();

	// 		return pool;
	// 	}

	// 	// No ready pools, so we need to create a new one.
	// 	Shared<VyDescriptorPool> newPool = createPool(m_SetsPerPool, m_Ratios);

	// 	// Growth for the next allocation.
	// 	growSetCount();

	// 	return newPool;
	// }


	// Shared<VyDescriptorPool> 
	// VyDescriptorAllocator::createPool(
	// 	U32                  setCount, 
	// 	TSpan<PoolSizeRatio> poolRatios) const 
	// {
	// 	TVector<VkDescriptorPoolSize> poolSizes;
		
	// 	for (auto [ type, ratio ] : poolRatios) 
	// 	{
	// 		poolSizes.emplace_back(type, static_cast<U32>(ratio * setCount));
	// 	}

	// 	Shared<VyDescriptorPool> pool = VyDescriptorPool::Builder()
	// 		.addPoolSizes(poolSizes)
	// 		.setMaxSets  (setCount)
	// 		.buildUnique();

	// 	return pool;
	// }


	// void VyDescriptorAllocator::resetPools() 
	// {
	// 	for (const auto& pool : m_ReadyPools) 
	// 	{
	// 		pool->resetPool();
	// 	}
		
	// 	for (auto& pool : m_FullPools) 
	// 	{
	// 		pool->resetPool();
			
	// 		m_ReadyPools.push_back(pool);
	// 	}

	// 	m_FullPools.clear();
	// }


	// void VyDescriptorAllocator::clearPools() 
	// {
	// 	m_ReadyPools.clear();
	// 	m_FullPools .clear();
	// }

#pragma endregion [ Allocator ]



    // bool DescriptorAllocator::init(U32 initialSets) 
	// {
    //     for (auto r: DEFAULT_SIZE_CONFIG) 
	// 	{
    //         m_sizeRatios.push_back(r);
    //     }

    //     VkDescriptorPool newPool = createPool(initialSets, m_sizeRatios);

    //     m_setsPerPool = initialSets * 2u;

    //     m_readyPools.push_back(newPool);

    //     return true;
    // }

    // void DescriptorAllocator::destroy() 
	// {
    //     for (auto pool : m_readyPools) 
	// 	{
	// 		vkDestroyDescriptorPool(VyContext::device(), pool, nullptr);
    //     }
    //     m_readyPools.clear();

    //     for (auto pool : m_fullPools) 
	// 	{
    //         vkDestroyDescriptorPool(VyContext::device(), pool, nullptr);
    //     }
    //     m_fullPools.clear();
    // }

    // void DescriptorAllocator::clear() {
    //     for (auto p: m_readyPools) {
    //         m_device->ResetDescriptorPool(p);
    //     }
    //     for (auto p: m_fullPools) {
    //         m_device->DestroyDescriptorPool(p);
    //         m_readyPools.push_back(p);
    //     }
    //     m_fullPools.clear();
    // }

    // VkDescriptorPool DescriptorAllocator::getPool() {
    //     VkDescriptorPool newPool;
    //     if (!m_readyPools.empty()) {
    //         newPool = m_readyPools.back();
    //         m_readyPools.pop_back();
    //     }
    //     else {
    //         //! Need to create a new pool
    //         newPool = createPool(m_setsPerPool, m_sizeRatios);

    //         m_setsPerPool = m_setsPerPool * 1.5;
    //         if (m_setsPerPool > SET_LIMIT_PER_POOL) {
    //             m_setsPerPool = SET_LIMIT_PER_POOL;
    //         }
    //     }

    //     return newPool;
    // }

    // VkDescriptorPool DescriptorAllocator::createPool(U32 setCount, std::span<PoolSizeRatio> poolRatios) {
    //     std::vector<VkDescriptorPoolSize> poolSizes;
    //     for (PoolSizeRatio ratio : poolRatios) {
    //         poolSizes.push_back(VkDescriptorPoolSize{
    //                 .type = ratio.type,
    //                 .descriptorCount = U32(ratio.ratio * setCount)
    //         });
    //     }

    //     VkDescriptorPoolCreateInfo pool_info = {};
    //     pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    //     pool_info.flags = 0;
    //     pool_info.maxSets = setCount;
    //     pool_info.poolSizeCount = (U32)poolSizes.size();
    //     pool_info.pPoolSizes = poolSizes.data();

    //     VkDescriptorPool newPool = m_device->CreateDescriptorPool(pool_info);
    //     return newPool;
    // }

    // VkDescriptorSet DescriptorAllocator::allocate(VkDescriptorSetLayout layout) {
    //     VkDescriptorPool poolToUse = getPool();

    //     VkDescriptorSetAllocateInfo allocInfo = {};
    //     allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    //     allocInfo.descriptorPool = poolToUse;
    //     allocInfo.descriptorSetCount = 1;
    //     allocInfo.pSetLayouts = &layout;

    //     VkResult result;
    //     VkDescriptorSet ds = m_device->AllocateDescriptorSet(allocInfo, &result);

    //     //! Allocation failed. Try again but if not then we fucked up
    //     if (result == VK_ERROR_OUT_OF_POOL_MEMORY || result == VK_ERROR_FRAGMENTED_POOL) {
    //         m_fullPools.push_back(poolToUse);

    //         poolToUse = GetPool();
    //         allocInfo.descriptorPool = poolToUse;

    //         ds = m_device->AllocateDescriptorSet(allocInfo, &result);

    //         if ( VkCheck(result) ) {
    //             Log(Error, "Error allocating a descriptor set from the descriptor pool");
    //             ds = VK_NULL_HANDLE;
    //             return ds;
    //         }
    //     }

    //     m_readyPools.push_back(poolToUse);
    //     return ds;
    // }
}





























// 	VyDescriptorSetLayout::Builder& 
// 	VyDescriptorSetLayout::Builder::addBinding(
// 		BindingIndex       binding,
// 		VkDescriptorType   descriptorType,
// 		VkShaderStageFlags stageFlags, 
// 		U32                count)
// 	{
// 		VY_ASSERT(m_Bindings.count(binding) == 0, "Binding already in use");
		
// 		VkDescriptorSetLayoutBinding layoutBinding{};
// 		{
// 			layoutBinding.binding		  = binding;
// 			layoutBinding.descriptorType  = descriptorType;
// 			layoutBinding.descriptorCount = count;
// 			layoutBinding.stageFlags	  = stageFlags;
// 		}

// 		m_Bindings[binding] = layoutBinding;

// 		return *this;
// 	}


// 	Unique<VyDescriptorSetLayout> 
// 	VyDescriptorSetLayout::Builder::buildUnique() const
// 	{
// 		return std::make_unique<VyDescriptorSetLayout>(m_Bindings);
// 	}


// 	VyDescriptorSetLayout::VyDescriptorSetLayout(
// 		VyLayoutBindingMap bindings
// 	):
// 		m_Bindings{ bindings }
// 	{
// 		TVector<VkDescriptorSetLayoutBinding> setLayoutBindings;
// 		setLayoutBindings.reserve(m_Bindings.size());

// 		for (auto kv : m_Bindings) 
// 		{ 
// 			setLayoutBindings.push_back(kv.second); 
// 		}

// 		auto setLayoutInfo{ VKInit::descriptorSetLayoutCreateInfo() };
// 		{
// 			setLayoutInfo.bindingCount = static_cast<U32>(setLayoutBindings.size());
// 			setLayoutInfo.pBindings	   = setLayoutBindings.data();
// 		}

// 		VK_CHECK(vkCreateDescriptorSetLayout(VyContext::device(), &setLayoutInfo, nullptr, &m_SetLayout));
// 	}


// 	VyDescriptorSetLayout::~VyDescriptorSetLayout()
// 	{
// 		vkDestroyDescriptorSetLayout(VyContext::device(), m_SetLayout, nullptr);
// 	}



	
// 	VyDescriptorPool::Builder& 
// 	VyDescriptorPool::Builder::addPoolSize(VkDescriptorType descriptorType, U32 count)
// 	{
// 		m_PoolSizes.push_back({descriptorType, count});
// 		return *this;
// 	}


// 	VyDescriptorPool::Builder& 
// 	VyDescriptorPool::Builder::addPoolSizes(TSpan<VkDescriptorPoolSize> poolSizes)
// 	{
//         for (auto& poolSize : poolSizes) 
// 		{
//             m_PoolSizes.emplace_back(poolSize);
//         }

//         return *this;
// 	}

	
// 	VyDescriptorPool::Builder& 
// 	VyDescriptorPool::Builder::setPoolFlags(VkDescriptorPoolCreateFlags flags)
// 	{
// 		m_PoolFlags = flags;
// 		return *this;
// 	}

// 	VyDescriptorPool::Builder& 
// 	VyDescriptorPool::Builder::setMaxSets(U32 count)
// 	{
// 		m_MaxSets = count;
// 		return *this;
// 	}

// 	Unique<VyDescriptorPool> 
// 	VyDescriptorPool::Builder::buildUnique() const
// 	{
// 		return std::make_unique<VyDescriptorPool>(m_MaxSets, m_PoolFlags, m_PoolSizes);
// 	}


// 	VyDescriptorPool::VyDescriptorPool(
// 		U32                                  maxSets, 
// 		VkDescriptorPoolCreateFlags          poolFlags,
// 		const TVector<VkDescriptorPoolSize>& poolSizes)	
// 	{
// 		auto poolInfo{ VKInit::descriptorPoolCreateInfo() };
// 		{
// 			poolInfo.poolSizeCount = static_cast<U32>(poolSizes.size());
// 			poolInfo.pPoolSizes    = poolSizes.data();
// 			poolInfo.maxSets       = maxSets;
// 			poolInfo.flags         = poolFlags;
// 		}

// 		VK_CHECK(vkCreateDescriptorPool(VyContext::device(), &poolInfo, nullptr, &m_Pool));
// 	}


// 	VyDescriptorPool::~VyDescriptorPool()
// 	{
// 		vkDestroyDescriptorPool(VyContext::device(), m_Pool, nullptr);
// 	}


// 	bool VyDescriptorPool::allocateDescriptorSet(
// 		const VkDescriptorSetLayout setLayout,
// 		VkDescriptorSet&            descriptor) const
// 	{
// 		VkDescriptorSetAllocateInfo allocInfo{ VKInit::descriptorSetAllocateInfo() };
// 		{
// 			allocInfo.descriptorPool	 = m_Pool;
// 			allocInfo.pSetLayouts		 = &setLayout;
// 			allocInfo.descriptorSetCount = 1;
// 		}

// 		// Might want to create a "DescriptorPoolManager" class that handles this case, and builds
// 		// a new pool whenever an old pool fills up. But this is beyond our current scope
// 		VkResult result = vkAllocateDescriptorSets(VyContext::device(), &allocInfo, &descriptor);

// 		if (result == VK_ERROR_FRAGMENTED_POOL)
// 		{
//             VY_ERROR("Error: Fragmentated pool.");
// 			return false;
// 		}

// 		if (result == VK_ERROR_OUT_OF_POOL_MEMORY) 
// 		{
// 			VY_ERROR("Error: Descriptor pool ran out of memory.");
// 			return false;
// 		}

// 		if (result == VK_ERROR_FRAGMENTATION)
// 		{
// 			VY_ERROR("Error: Fragmentation of pool memory while allocating descriptor sets.");
// 			return false;
// 		}

// 		if (result == VK_ERROR_OUT_OF_DEVICE_MEMORY) 
// 		{
// 			VY_THROW_RUNTIME_ERROR("Failed to allocate descriptor set, device out of memory!");
// 		}

// 		if (result == VK_ERROR_OUT_OF_HOST_MEMORY) 
// 		{
// 			VY_THROW_RUNTIME_ERROR("Failed to allocate descriptor set, host out of memory!");
// 		}

// 		if (result != VK_SUCCESS) 
// 		{
// 			VY_THROW_RUNTIME_ERROR("Failed to allocate descriptor set!");
// 		}
	
//         return true;
// 	}


// 	void VyDescriptorPool::freeDescriptors(TVector<VkDescriptorSet>& descriptors) const
// 	{
// 		vkFreeDescriptorSets(VyContext::device(), m_Pool, static_cast<U32>(descriptors.size()), descriptors.data());
// 	}


// 	void VyDescriptorPool::resetPool()
// 	{
// 		vkResetDescriptorPool(VyContext::device(), m_Pool, 0);
// 	}


//     VyDescriptorWriter::VyDescriptorWriter(VyDescriptorSetLayout& setLayout) : 
// 		m_SetLayout{ setLayout } 
// 	{
// 	}


//     void VyDescriptorWriter::update(VkDescriptorSet& set) 
// 	{
//         for (auto& write : m_Writes) 
// 		{
//             write.dstSet = set;
//         }
    
// 		vkUpdateDescriptorSets(VyContext::device(), static_cast<U32>(m_Writes.size()), m_Writes.data(), 0, nullptr);
//     }
// }



















// namespace Vy
// {
// 	DescriptorSetLayout::Builder& DescriptorSetLayout::Builder::AddBinding(
// 		U32 binding,
// 		VkDescriptorType descriptorType,
// 		VkShaderStageFlags stageFlags,
// 		U32 count)
// 	{
// 		assert(std::ranges::find_if(
// 			m_Bindings, 
// 			[binding](const VkDescriptorSetLayoutBinding& layoutBinding)
// 			{
// 				return layoutBinding.binding == binding;
// 			}) == m_Bindings.end() && "Binding already in use");
		
// 		VkDescriptorSetLayoutBinding layoutBinding{};
// 		{
// 			layoutBinding.binding = binding;
// 			layoutBinding.descriptorType = descriptorType;
// 			layoutBinding.descriptorCount = count;
// 			layoutBinding.stageFlags = stageFlags;
// 		}

// 		m_Bindings.push_back(layoutBinding);

// 		return *this;
// 	}


// 	Shared<DescriptorSetLayout> 
// 	DescriptorSetLayout::Builder::build() const 
// 	{
// 		return m_Cache.createLayout(m_Bindings);
// 	}


// 	DescriptorSetLayout::DescriptorSetLayout(const TVector<VkDescriptorSetLayoutBinding>& bindings) : 
// 		m_Bindings{bindings}
// 	{
// 		VkDescriptorSetLayoutCreateInfo setLayoutInfo{};
// 		{
// 			setLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
// 			setLayoutInfo.bindingCount = static_cast<U32>(bindings.size());
// 			setLayoutInfo.pBindings = bindings.data();
// 		}

// 		if (vkCreateDescriptorSetLayout(
// 			VyContext::device(),
// 			&setLayoutInfo,
// 			nullptr,
// 			&m_SetLayout) != VK_SUCCESS) 
// 		{
// 			VY_THROW_RUNTIME_ERROR("failed to create descriptor set layout!");
// 		}
// 	}

// 	DescriptorSetLayout::~DescriptorSetLayout() 
// 	{
// 		vkDestroyDescriptorSetLayout(VyContext::device(), m_SetLayout, nullptr);
// 	}

// 	DescriptorSetLayout::DescriptorSetLayout(DescriptorSetLayout&& other) noexcept :
// 		m_SetLayout(other.m_SetLayout),
// 		m_Bindings(std::move(other.m_Bindings))
// 	{
// 		other.m_SetLayout = VK_NULL_HANDLE;
// 	}

// 	DescriptorSetLayout& DescriptorSetLayout::operator=(DescriptorSetLayout&& other) noexcept
// 	{
// 		m_SetLayout = other.m_SetLayout;
// 		m_Bindings = std::move(other.m_Bindings);
// 		return *this;
// 	}


// 	Shared<DescriptorSetLayout> DescriptorLayoutCache::createLayout(
// 		const TVector<VkDescriptorSetLayoutBinding>& bindings)
// 	{
// 		DescriptorLayoutInfo layoutInfo;
// 		layoutInfo.Bindings.reserve(bindings.size());
// 		bool isSorted = true;
// 		int32_t lastBinding = -1;
// 		for (const auto& binding : bindings)
// 		{
// 			layoutInfo.Bindings.push_back(binding);

// 			//check that the bindings are in strict increasing order
// 			if (static_cast<int32_t>(binding.binding) > lastBinding)
// 			{
// 				lastBinding = static_cast<int32_t>(binding.binding);
// 			}
// 			else
// 			{
// 				isSorted = false;
// 			}
// 		}

// 		if (!isSorted)
// 		{
// 			std::ranges::sort(
// 				layoutInfo.Bindings, 
// 				[](const VkDescriptorSetLayoutBinding& a, const VkDescriptorSetLayoutBinding& b)
// 				{
// 					return a.binding < b.binding;
// 				}
// 			);
// 		}

// 		const auto it = m_LayoutCache.find(layoutInfo);

// 		if (it != m_LayoutCache.end())
// 		{
// 			return it->second;
// 		}

// 		m_LayoutCache.insert({ layoutInfo, std::make_shared<DescriptorSetLayout>(layoutInfo.Bindings) });

// 		return m_LayoutCache.at(layoutInfo);
// 	}


// 	bool DescriptorLayoutCache::DescriptorLayoutInfo::operator==(const DescriptorLayoutInfo& other) const
// 	{
// 		if (other.Bindings.size() != Bindings.size())
// 			return false;
// 		for (size_t i = 0; i < Bindings.size(); i++) 
// 		{
// 			if (other.Bindings[i].binding != Bindings[i].binding)
// 				return false;
// 			if (other.Bindings[i].descriptorType != Bindings[i].descriptorType)
// 				return false;
// 			if (other.Bindings[i].descriptorCount != Bindings[i].descriptorCount)
// 				return false;
// 			if (other.Bindings[i].stageFlags != Bindings[i].stageFlags)
// 				return false;
// 		}
// 		return true;
// 	}

// 	size_t DescriptorLayoutCache::DescriptorLayoutInfo::hash() const
// 	{

// 		size_t result = std::hash<size_t>()(Bindings.size());

// 		for (const VkDescriptorSetLayoutBinding& b : Bindings)
// 		{
// 			//pack the binding data into a single int64. Not fully correct but its ok
// 			size_t binding_hash = b.binding | b.descriptorType << 8 | b.descriptorCount << 16 | b.stageFlags << 24;

// 			//shuffle the packed binding data and xor it with the main hash
// 			result ^= std::hash<size_t>()(binding_hash);
// 		}

// 		return result;
// 	}

	
// 	DescriptorPool::Builder& 
// 	DescriptorPool::Builder::addPoolSize(VkDescriptorType descriptorType, U32 count)
// 	{
// 		m_PoolSizes.push_back({descriptorType, count});
// 		return *this;
// 	}

// 	DescriptorPool::Builder&
// 	DescriptorPool::Builder::setPoolFlags(
// 		VkDescriptorPoolCreateFlags flags)
// 	{
// 		m_PoolFlags = flags;
// 		return *this;
// 	}

// 	DescriptorPool::Builder& 
// 	DescriptorPool::Builder::setMaxSets(U32 count) 
// 	{
// 		m_MaxSets = count;
// 		return *this;
// 	}

// 	DescriptorPool 
// 	DescriptorPool::Builder::build() const 
// 	{
// 		return {m_MaxSets, m_PoolFlags, m_PoolSizes};
// 	}


// 	DescriptorPool::DescriptorPool(
// 		U32 maxSets,
// 		VkDescriptorPoolCreateFlags poolFlags,
// 		const TVector<VkDescriptorPoolSize>& poolSizes)
// 	{
// 		VkDescriptorPoolCreateInfo poolInfo{};
// 		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
// 		poolInfo.poolSizeCount = static_cast<U32>(poolSizes.size());
// 		poolInfo.pPoolSizes = poolSizes.data();
// 		poolInfo.maxSets = maxSets;
// 		poolInfo.flags = poolFlags;

// 		if (vkCreateDescriptorPool(VyContext::device(), &poolInfo, nullptr, &m_Pool) != VK_SUCCESS) {
// 			VY_THROW_RUNTIME_ERROR("failed to create descriptor pool!");
// 		}
// 	}

// 	DescriptorPool::~DescriptorPool()
// 	{
// 		vkDestroyDescriptorPool(VyContext::device(), m_Pool, nullptr);
// 	}

// 	DescriptorPool::DescriptorPool(DescriptorPool&& other) noexcept
// 	{
// 		std::swap(m_Pool, other.m_Pool);
// 	}

// 	DescriptorPool& DescriptorPool::operator=(DescriptorPool&& other) noexcept
// 	{
// 		std::swap(m_Pool, other.m_Pool);
// 		return *this;
// 	}

// 	VkResult DescriptorPool::allocateDescriptor(
// 		const VkDescriptorSetLayout descriptorSetLayout, 
// 		VkDescriptorSet& descriptor) const
// 	{
// 		VkDescriptorSetAllocateInfo allocInfo{};
// 		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
// 		allocInfo.descriptorPool = m_Pool;
// 		allocInfo.descriptorSetCount = 1;
// 		allocInfo.pSetLayouts = &descriptorSetLayout;


// 		// Might want to create a "DescriptorPoolManager" class that handles this case, and builds
// 		// a new pool whenever an old pool fills up. But this is beyond our current scope
// 		return vkAllocateDescriptorSets(VyContext::device(), &allocInfo, &descriptor);
// 	}


// 	void DescriptorPool::freeDescriptors(TVector<DescriptorSet>& descriptors) const
// 	{
// 		TVector<VkDescriptorSet> vkDescriptors;

// 		std::ranges::transform(
// 			descriptors,
// 			std::back_inserter(vkDescriptors),
// 			[](DescriptorSet& descriptor)
// 			{
// 				const auto out = descriptor.m_Set;
// 				descriptor.m_Set = VK_NULL_HANDLE;
// 				return out;
// 			}
// 		);
		
// 		freeDescriptors(vkDescriptors);
// 	}

// 	void DescriptorPool::freeDescriptors(const TVector<VkDescriptorSet>& descriptors) const
// 	{
// 		vkFreeDescriptorSets(
// 			VyContext::device(),
// 			m_Pool,
// 			static_cast<U32>(descriptors.size()),
// 			descriptors.data()
// 		);
// 	}

// 	void DescriptorPool::resetPool() const
// 	{
// 		vkResetDescriptorPool(VyContext::device(), m_Pool, 0);
// 	}

// 	void DescriptorAllocator::resetPools()
// 	{
// 		for (auto& p : m_UsedPools)
// 		{
// 			p.resetPool();
// 		}

// 		m_FreePools = std::move(m_UsedPools);
// 		m_UsedPools.clear();
// 		m_CurrentPool = nullptr;
// 	}



// 	Optional<DescriptorSet> 
// 	DescriptorAllocator::allocate(const DescriptorSetLayout& layout)
// 	{
// 		if (!m_CurrentPool)
// 		{
// 			m_CurrentPool = &grabPool();
// 		}

// 		DescriptorSet descriptorSet(*m_CurrentPool);
// 		auto result = m_CurrentPool->allocateDescriptor(layout.handle(), descriptorSet.handle());

// 		switch (result) {
// 		case VK_SUCCESS:
// 			//all good, return
// 			return { std::move(descriptorSet) };
// 		case VK_ERROR_FRAGMENTED_POOL:
// 		case VK_ERROR_OUT_OF_POOL_MEMORY:
// 			//reallocate pool
// 			break;
// 		default:
// 			//unrecoverable error
// 			return {};
// 		}

// 		m_CurrentPool = &grabPool();
// 		descriptorSet = DescriptorSet(*m_CurrentPool);
// 		result = m_CurrentPool->allocateDescriptor(layout.handle(), descriptorSet.handle());

// 		if (result != VK_SUCCESS)
// 			return {};

// 		return { std::move(descriptorSet) };
// 	}

// 	DescriptorPool DescriptorAllocator::createPool(U32 count, VkDescriptorPoolCreateFlags flags) const
// 	{
// 		auto poolBuilder = DescriptorPool::Builder{};
// 		for (auto [type, factor] : m_PoolSizes)
// 		{
// 			poolBuilder.addPoolSize(type, static_cast<U32>(factor * static_cast<float>(count)));
// 		}

// 		poolBuilder.setMaxSets(count);
// 		poolBuilder.setPoolFlags(flags);

// 		return poolBuilder.build();
// 	}


// 	DescriptorPool& DescriptorAllocator::grabPool()
// 	{
// 		if (!m_FreePools.empty())
// 		{
// 			DescriptorPool& pool = m_FreePools.back();
// 			m_FreePools.pop_back();
// 			return pool;
// 		}
// 		else {
// 			m_UsedPools.push_back(createPool(1000, VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT));
// 			return m_UsedPools.back();
// 		}
// 	}


// 	DescriptorSet::Writer::Writer(DescriptorAllocator& allocator, DescriptorSetLayout& setLayout) : 
// 		m_Allocator{allocator}, 
// 		m_SetLayout{setLayout} 
// 	{
// 	}
	

// 	DescriptorSet::Writer& DescriptorSet::Writer::writeBuffer(U32 binding, const VkDescriptorBufferInfo* bufferInfo)
// 	{
// 		assert(std::ranges::count_if(
// 			m_SetLayout.m_Bindings, 
// 			[binding](const VkDescriptorSetLayoutBinding& layoutBinding)
// 			{
// 				return layoutBinding.binding == binding;
// 			}
// 		) == 1 && "Layout does not contain specified binding");

// 		const auto& bindingDescription = m_SetLayout.m_Bindings[binding];

// 		assert(
// 			bindingDescription.descriptorCount == 1 &&
// 			"Binding single descriptor info, but binding expects multiple"
// 		);

// 		VkWriteDescriptorSet write{};
// 		{
// 			write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
// 			write.descriptorType = bindingDescription.descriptorType;
// 			write.dstBinding = binding;
// 			write.pBufferInfo = bufferInfo;
// 			write.descriptorCount = 1;
// 		}

// 		m_Writes.push_back(write);
// 		return *this;
// 	}

// 	DescriptorSet::Writer& DescriptorSet::Writer::writeImage(U32 binding, const VkDescriptorImageInfo& imageInfo) 
// 	{
// 		assert(std::ranges::count_if(
// 			m_SetLayout.m_Bindings, 
// 			[binding](const VkDescriptorSetLayoutBinding& layoutBinding)
// 			{
// 				return layoutBinding.binding == binding;
// 			}
// 		) == 1 && "Layout does not contain specified binding");

// 		const auto& bindingDescription = m_SetLayout.m_Bindings[binding];

// 		assert(
// 			bindingDescription.descriptorCount == 1 &&
// 			"Binding single descriptor info, but binding expects multiple"
// 		);

// 		VkWriteDescriptorSet write{};
// 		{
// 			write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
// 			write.descriptorType = bindingDescription.descriptorType;
// 			write.dstBinding = binding;
// 			write.pImageInfo = &imageInfo;
// 			write.descriptorCount = 1;
// 		}

// 		m_Writes.push_back(write);
// 		return *this;
// 	}


// 	Unique<DescriptorSet> DescriptorSet::Writer::buildUnique() 
// 	{
// 		auto out = m_Allocator.allocate(m_SetLayout);
		
// 		if (!out.has_value()) 
// 		{
// 			return {};
// 		}
		
// 		overwrite(out.value());
		
// 		return std::make_unique<DescriptorSet>(std::move(out.value()));
// 	}

	
// 	void DescriptorSet::Writer::overwrite(const DescriptorSet& set) 
// 	{
// 		for (auto& write : m_Writes) 
// 		{
// 			write.dstSet = set.m_Set;
// 		}
		
// 		vkUpdateDescriptorSets(VyContext::device(), m_Writes.size(), m_Writes.data(), 0, nullptr);
// 	}


// 	DescriptorSet::~DescriptorSet()
// 	{
// 		m_Pool.freeDescriptors({m_Set});
// 	}


// 	DescriptorSet::DescriptorSet(DescriptorSet&& other) noexcept : 
// 		m_Pool(other.m_Pool)
// 	{
// 		m_Set = other.m_Set;
// 		other.m_Set = VK_NULL_HANDLE;
// 	}

// 	DescriptorSet& DescriptorSet::operator=(DescriptorSet&& other) noexcept
// 	{
// 		m_Set = other.m_Set;
// 		other.m_Set = VK_NULL_HANDLE;
// 		return *this;
// 	}
// }








	// VyDescriptorWriter::VyDescriptorWriter(
	// 	VyDescriptorSetLayout& setLayout, 
	// 	VyDescriptorPool&      pool
	// ) :
	// 	m_SetLayout{ setLayout }, 
	// 	m_Pool     { pool      }
	// {
	// }


	// VyDescriptorWriter& 
	// VyDescriptorWriter::writeBuffer(
	// 	BindingIndex            binding, 
	// 	VkDescriptorBufferInfo* bufferInfo, 
	// 	U32                     count)
	// {
	// 	VY_ASSERT(m_SetLayout.m_Bindings.count(binding) == 1, 
	// 		"Layout does not contain specified binding");

	// 	auto& bindingDesc = m_SetLayout.m_Bindings[binding];

	// 	if (!bindingDesc.descriptorCount == count)
	// 	{
	// 		VY_ERROR("Binding {} descriptor info, but binding expects {}", count, bindingDesc.descriptorCount);
	// 		VY_ASSERT(false);
	// 	}

	// 	VkWriteDescriptorSet write{ VKInit::writeDescriptorSet() };
	// 	{
	// 		write.descriptorType  = bindingDesc.descriptorType;
	// 		write.dstBinding	  = binding;
	// 		write.pBufferInfo	  = bufferInfo;
	// 		write.descriptorCount = count;
	// 	}

	// 	m_Writes.push_back(write);

	// 	return *this;
	// }


	// VyDescriptorWriter& 
	// VyDescriptorWriter::writeImage(
	// 	BindingIndex           binding, 
	// 	VkDescriptorImageInfo* imageInfo, 
	// 	U32                    count)
	// {
	// 	VY_ASSERT(m_SetLayout.m_Bindings.count(binding) == 1, 
	// 		"Layout does not contain specified binding");

	// 	auto& bindingDesc = m_SetLayout.m_Bindings[binding];

	// 	if (!bindingDesc.descriptorCount == count)
	// 	{
	// 		VY_ERROR("Binding {} descriptor info, but binding expects {}", count, bindingDesc.descriptorCount);
	// 		VY_ASSERT(false);
	// 	}

	// 	VkWriteDescriptorSet write{ VKInit::writeDescriptorSet() };
	// 	{
	// 		write.descriptorType  = bindingDesc.descriptorType;
	// 		write.dstBinding	  = binding;
	// 		write.pImageInfo	  = imageInfo;
	// 		write.descriptorCount = count;
	// 	}

	// 	m_Writes.push_back(write);

	// 	return *this;
	// }


	// bool VyDescriptorWriter::build(VkDescriptorSet& set)
	// {
	// 	bool success = m_Pool.allocateDescriptorSet(m_SetLayout.handle(), set);
		
	// 	if (!success) 
	// 	{
	// 		return false;
	// 	}
		
	// 	update(set);
		
	// 	return true;
	// }


	// void VyDescriptorWriter::update(VkDescriptorSet& set)
	// {
	// 	for (auto& write : m_Writes) 
	// 	{ 
	// 		write.dstSet = set; 
	// 	}
		
	// 	vkUpdateDescriptorSets(VyContext::device(), m_Writes.size(), m_Writes.data(), 0, nullptr);
	// }











































    // Shared<VyDescriptorPool> DescriptorAllocator::s_GlobalPool = nullptr;

    // /**
    //  * Init a global descriptor pool for all textures, materials, etc.
    //  * @param device
    //  */
    // void DescriptorAllocator::Init() 
	// {
    //     s_GlobalPool = VyDescriptorPool::Builder()
    //         .setMaxSets(1024)
    //         .setPoolFlags(VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT)
    //         .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1024)
    //         .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 128)
    //         .build();
    // }

    // bool DescriptorAllocator::allocate(const VkDescriptorSetLayout layout, VkDescriptorSet &set) 
	// {
    //     return s_GlobalPool->allocateDescriptorSet(layout, set);
    // }

    // Shared<VyDescriptorPool> DescriptorAllocator::sharedPool() 
	// {
    //     return s_GlobalPool;
    // }

    // void DescriptorAllocator::cleanup() 
	// {
    //     s_GlobalPool.reset();
    // }

    // void DescriptorAllocator::release(TVector<VkDescriptorSet>& sets) 
	// {
    //     s_GlobalPool->freeDescriptors(sets);
    // }


namespace Vy
{

	// VkGfxDescriptorSetLayout::Builder& VkGfxDescriptorSetLayout::Builder::addBinding(
	// 	BindingIndex       binding,
	// 	VkDescriptorType   descriptorType,
	// 	VkShaderStageFlags stageFlags,
	// 	U32                count,
	// 	bool               isBindless)
	// {
	// 	VY_ASSERT(BindingsMap.count(binding) == 0, "Binding already in use");

	// 	VkDescriptorSetLayoutBinding layoutBinding{};
	// 	{
	// 		layoutBinding.binding         = binding;
	// 		layoutBinding.descriptorType  = descriptorType;
	// 		layoutBinding.descriptorCount = count;
	// 		layoutBinding.stageFlags      = stageFlags;
	// 	}

	// 	BindingsMap.emplace(binding, layoutBinding);

	// 	if (isBindless)
	// 	{
	// 		BindingFlags.emplace(binding, VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT);
	// 	}
	// 	else
	// 	{
	// 		BindingFlags.emplace(binding, 0);
	// 	}

	// 	return *this;
	// }


	// VkGfxDescriptorSetLayout::Builder& 
	// VkGfxDescriptorSetLayout::Builder::setDebugName(const String& name)
	// {
	// 	DebugName = name;

	// 	return *this;
	// }


	// Unique<VkGfxDescriptorSetLayout> 
	// VkGfxDescriptorSetLayout::Builder::build()
	// {
	// 	U32 bindingsCount = BindingsMap.size();
		
	// 	TVector<VkDescriptorSetLayoutBinding> setLayoutBindings(bindingsCount);
	// 	TVector<VkDescriptorBindingFlags>     setBindingFlags(bindingsCount);
	// 	bool isBindless = false;

	// 	for (const auto& [ bindingKey, info ] : BindingsMap)
	// 	{
	// 		setLayoutBindings[ bindingKey ] = BindingsMap [ bindingKey ];
	// 		setBindingFlags  [ bindingKey ] = BindingFlags[ bindingKey ];

	// 		if (BindingFlags[ bindingKey ]) 
	// 		{
	// 			isBindless = true;
	// 		}
	// 	}

	// 	auto bindingFlagsInfo{ VKInit::descriptorSetLayoutBindingFlagsCreateInfo() };
	// 	{
	// 		bindingFlagsInfo.pNext         = nullptr;
	// 		bindingFlagsInfo.pBindingFlags = setBindingFlags.data();
	// 		bindingFlagsInfo.bindingCount  = setBindingFlags.size();
	// 	}

	// 	auto setLayoutInfo { VKInit::descriptorSetLayoutCreateInfo() };
	// 	{
	// 		setLayoutInfo.bindingCount = static_cast<U32>(setLayoutBindings.size());
	// 		setLayoutInfo.pBindings    = setLayoutBindings.data();
			
	// 		if (isBindless)
	// 		{
	// 			setLayoutInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
	// 			setLayoutInfo.pNext = &bindingFlagsInfo;
	// 		}
	// 	}

	// 	auto setLayout = MakeUnique<VkGfxDescriptorSetLayout>(BindingsMap);
	// 	VK_CHECK(vkCreateDescriptorSetLayout(VyContext::device(), &setLayoutInfo, nullptr, &setLayout->Layout));

	// 	return std::move(setLayout);
	// }

	// VkGfxDescriptorSetLayout::~VkGfxDescriptorSetLayout()
	// {
	// 	vkDestroyDescriptorSetLayout(VyContext::device(), Layout, nullptr);
	// }


	// VkGfxDescriptorPool::Builder& 
	// VkGfxDescriptorPool::Builder::addPoolSize(VkDescriptorType descriptorType, U32 count)
	// {
	// 	PoolSizes.push_back({ descriptorType, count });

	// 	return *this;
	// }

	// VkGfxDescriptorPool::Builder& 
	// VkGfxDescriptorPool::Builder::setDebugName(const String& name)
	// {
	// 	DebugName = name;
	
	// 	return *this;
	// }

	// Unique<VkGfxDescriptorPool> 
	// VkGfxDescriptorPool::Builder::build(U32 maxSets, VkDescriptorPoolCreateFlags poolFlags)
	// {
	// 	VkDescriptorPoolCreateInfo poolInfo { VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
	// 	poolInfo.poolSizeCount = static_cast<U32>(PoolSizes.size());
	// 	poolInfo.pPoolSizes    = PoolSizes.data();
	// 	poolInfo.maxSets       = maxSets;
	// 	poolInfo.flags         = poolFlags;

	// 	auto gfxDescriptorPool   = MakeUnique<VkGfxDescriptorPool>();
	// 	VK_CHECK(vkCreateDescriptorPool(VyContext::device(), &poolInfo, nullptr, &gfxDescriptorPool->Pool));

	// 	return std::move(gfxDescriptorPool);
	// }


	// VkGfxDescriptorPool::~VkGfxDescriptorPool()
	// {
	// 	vkDestroyDescriptorPool(VyContext::device(), Pool, nullptr);
	// }


	// Unique<VkGfxDescriptorSet> 
	// VkGfxDescriptorPool::allocateDescriptorSet(
	// 	VkGfxDescriptorSetLayout& descriptorSetLayout,
	// 	const char*               pDebugName)
	// {
	// 	VkDescriptorSetAllocateInfo allocInfo { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
	// 	allocInfo.descriptorPool     = Pool;
	// 	allocInfo.descriptorSetCount = 1;
	// 	allocInfo.pSetLayouts        = &descriptorSetLayout.Layout;

	// 	auto descriptorSet = MakeUnique<VkGfxDescriptorSet>(descriptorSetLayout);

	// 	VK_CHECK(vkAllocateDescriptorSets(VyContext::device(), &allocInfo, &descriptorSet->Set));

	// 	return std::move(descriptorSet);
	// }


	// void VkGfxDescriptorPool::freeDescriptorSets(TVector<VkDescriptorSet>& descriptorSets) const
	// {
	// 	vkFreeDescriptorSets(VyContext::device(), Pool, static_cast<U32>(descriptorSets.size()), descriptorSets.data());
	// }


	// void VkGfxDescriptorPool::resetPool()
	// {
	// 	VY_ASSERT(Pool, "invalid vulkan descriptor Pool");

	// 	vkResetDescriptorPool(VyContext::device(), Pool, 0);
	// }


	// VkGfxDescriptorSet& 
	// VkGfxDescriptorSet::configureBuffer(
	// 	BindingIndex            binding,
	// 	BindingIndex            dstIndex,
	// 	U32                     count,
	// 	VkDescriptorBufferInfo* bufferInfo)
	// {
	// 	// VY_ASSERT(BindingsMap.count(binding) == 0, "Binding already in use");
    //     VY_ASSERT(SetLayout.BindingsMap.count(binding) == 1, 
	// 		"Layout does not contain specified binding");

	// 	auto& bindingDesc = SetLayout.BindingsMap[binding];

	// 	VkWriteDescriptorSet write { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
	// 	write.descriptorType  = bindingDesc.descriptorType;
	// 	write.dstBinding      = binding;
	// 	write.pBufferInfo     = bufferInfo;
	// 	write.descriptorCount = count;
	// 	write.descriptorType  = bindingDesc.descriptorType;
	// 	write.dstArrayElement = dstIndex;

	// 	Writes.push_back(write);

	// 	return *this;
	// }

	
	// VkGfxDescriptorSet& 
	// VkGfxDescriptorSet::configureImage(
	// 	BindingIndex           binding,
	// 	BindingIndex           dstIndex,
	// 	U32               count,
	// 	VkDescriptorImageInfo* imageInfo)
	// {
    //     VY_ASSERT(SetLayout.BindingsMap.count(binding) == 1, 
	// 		"Layout does not contain specified binding");

	// 	auto& bindingDesc = SetLayout.BindingsMap[binding];

	// 	VkWriteDescriptorSet write { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
	// 	write.descriptorType  = bindingDesc.descriptorType;
	// 	write.dstBinding      = binding;
	// 	write.pImageInfo      = imageInfo;
	// 	write.descriptorCount = count;
	// 	write.descriptorType  = bindingDesc.descriptorType;
	// 	write.dstArrayElement = dstIndex;

	// 	Writes.push_back(write);

	// 	return *this;
	// }


	// void VkGfxDescriptorSet::applyConfiguration()
	// {
	// 	if (Writes.empty()) return;

	// 	for (auto& write : Writes)
	// 	{
	// 		write.dstSet = Set;
	// 	}

	// 	vkUpdateDescriptorSets(VyContext::device(), static_cast<U32>(Writes.size()), Writes.data(), 0, nullptr);

	// 	Writes.clear();
	// }
}