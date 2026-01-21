#include <VyEngine/VK/Descriptors/Pool.h>

#include <VyEngine/VK/Descriptors/SetLayout.h>

#include <VyEngine/VK/Context.h>

#include <ranges>

namespace Vy
{
// ================================================================================================
#pragma region [ Pool Builder ]
// ================================================================================================

	VyDescriptorPool::Builder& 
	VyDescriptorPool::Builder::setName(const TString& name)
	{
		m_Name = name;

		return *this;
	}

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
            m_PoolSizes.emplace_back( poolSize );
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
	VyDescriptorPool::Builder::buildPtr() const
	{
		return std::make_unique<VyDescriptorPool>( m_Name, m_MaxSets, m_PoolFlags, m_PoolSizes );
	}


	VyDescriptorPool 
	VyDescriptorPool::Builder::build() const
	{
		return VyDescriptorPool{ m_Name, m_MaxSets, m_PoolFlags, m_PoolSizes };
	}

#pragma endregion [ Pool Builder ]


// ================================================================================================
#pragma region [ Pool ]
// ================================================================================================

    VyDescriptorPool::VyDescriptorPool(
		const TString&                       name,
		U32                                  maxSets, 
		VkDescriptorPoolCreateFlags          poolFlags,
		const TVector<VkDescriptorPoolSize>& poolSizes
	) : 
		m_DebugName{ name + "_desc_pool" }
    {
		auto poolInfo{ VKInit::descriptorPoolCreateInfo() };
		{
			poolInfo.poolSizeCount = static_cast<U32>(poolSizes.size());
			poolInfo.pPoolSizes    = poolSizes.data();

			poolInfo.maxSets       = maxSets;
			poolInfo.flags         = poolFlags;
		}

		VK_CHECK_SUCCESS(vkCreateDescriptorPool(VyContext::device(), &poolInfo, nullptr, &m_Pool), 
			"Failed to create descriptor pool!");

		VKDbg::setObjectName(VK_OBJECT_TYPE_DESCRIPTOR_POOL, (U64)m_Pool, m_DebugName.c_str());
    }


	VyDescriptorPool::VyDescriptorPool(VyDescriptorPool&& other) noexcept : 
		m_Pool{ other.m_Pool }
	{
		other.m_Pool = VK_NULL_HANDLE;
	}


	VyDescriptorPool::~VyDescriptorPool()
	{
		destroy();
	}


	void VyDescriptorPool::destroy()
	{
		if (m_Pool)
		{
			vkDestroyDescriptorPool(VyContext::device(), m_Pool, nullptr);
		}

		m_Pool = VK_NULL_HANDLE;
	}


	VyDescriptorPool& 
	VyDescriptorPool::operator=(VyDescriptorPool&& other) noexcept
	{
		if (this != &other)
		{
			m_Pool = other.m_Pool;

			other.m_Pool = VK_NULL_HANDLE;
		}

		return *this;
	}


	bool VyDescriptorPool::allocate(
		VkDescriptorSetAllocateInfo allocInfo, 
		TVector<VkDescriptorSet>&   descriptors) const
	{
		VkResult result = vkAllocateDescriptorSets(VyContext::device(), &allocInfo, descriptors.data());

		return this->checkResult( result );
    }
	

    bool VyDescriptorPool::allocate(
		const VkDescriptorSetLayout setLayout, 
		VkDescriptorSet&            descriptor,
		const void*                 pNext,                  /* nullptr */
		U32                         variableDescriptorCount /* 0 */) const
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

		// Optional variable descriptor count extension.
		auto countInfo{ VKInit::descriptorSetVariableDescriptorCountAllocateInfo() };

		if (variableDescriptorCount > 0) 
		{
			countInfo.descriptorSetCount = 1;
			countInfo.pDescriptorCounts  = &variableDescriptorCount;

			allocInfo.pNext = &countInfo; // chain if used.
		}

		VkResult result = vkAllocateDescriptorSets(VyContext::device(), &allocInfo, &descriptor);

		return this->checkResult( result );
    }


	bool VyDescriptorPool::allocate(
		const VkDescriptorSetLayout setLayout, 
		TVector<VkDescriptorSet>&   descriptors,
		const void*                 pNext /* nullptr */) const
    {
		auto allocInfo{ VKInit::descriptorSetAllocateInfo() };
		{
			allocInfo.descriptorPool     = m_Pool;
			allocInfo.pSetLayouts        = &setLayout;
			allocInfo.descriptorSetCount = static_cast<U32>(descriptors.size());
			allocInfo.pNext              = pNext;
		}

		return this->allocate( allocInfo, descriptors );
    }


	bool VyDescriptorPool::allocate(
		const VkDescriptorSetLayout* pSetLayouts, 
		TVector<VkDescriptorSet>&    descriptors,
		const void*                  pNext /* nullptr */) const
	{
		auto allocInfo{ VKInit::descriptorSetAllocateInfo() };
		{
			allocInfo.descriptorPool     = m_Pool;
			allocInfo.pSetLayouts        = pSetLayouts;
			allocInfo.descriptorSetCount = static_cast<U32>(descriptors.size());
			allocInfo.pNext              = pNext;
		}

		return this->allocate( allocInfo, descriptors );
	}


	bool VyDescriptorPool::allocate(
		const VkDescriptorSetLayout* pSetLayouts, 
		U32                          count, 
		VkDescriptorSet*             pDescriptors)
	{
		auto allocInfo{ VKInit::descriptorSetAllocateInfo() };
		{
			allocInfo.descriptorPool     = m_Pool;
			allocInfo.pSetLayouts        = pSetLayouts;
			allocInfo.descriptorSetCount = count;
		}

		VkResult result = vkAllocateDescriptorSets(VyContext::device(), &allocInfo, pDescriptors);

        return this->checkResult( result );
	}


    TVector<VkDescriptorSet> 
	VyDescriptorPool::allocateSets(
		const VyDescriptorSetLayout& layout, 
		const U32                    count)
    {
        const TVector<VkDescriptorSetLayout> layouts( count, layout.handle() );

		auto allocInfo{ VKInit::descriptorSetAllocateInfo() };
		{
			allocInfo.descriptorPool     = m_Pool;
			allocInfo.pSetLayouts        = layouts.data();
			allocInfo.descriptorSetCount = count;
		}

        TVector<VkDescriptorSet> sets( count );
		VkResult result = vkAllocateDescriptorSets(VyContext::device(), &allocInfo, sets.data());
        
		VY_ASSERT( checkResult( result ), "Cannot create descriptor sets!" );

        return sets;
    }


	void VyDescriptorPool::freeDescriptors(TVector<VkDescriptorSet>& descriptors) const
	{
		vkFreeDescriptorSets(VyContext::device(), m_Pool, static_cast<U32>(descriptors.size()), descriptors.data());
	}


	void VyDescriptorPool::resetPool()
	{
		vkResetDescriptorPool(VyContext::device(), m_Pool, 0);
	}


	bool VyDescriptorPool::checkResult(VkResult result) const
	{
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

        return result == VK_SUCCESS;
    }

#pragma endregion [ Pool ]

// ================================================================================================
}