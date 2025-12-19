#include <Vy/GFX/Backend/Descriptors/SetLayout.h>

#include <Vy/GFX/Context.h>

#include <ranges>

namespace Vy
{
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
	VyDescriptorPool::Builder::buildUnique() const
	{
		return std::make_unique<VyDescriptorPool>( m_MaxSets, m_PoolFlags, m_PoolSizes );
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
}