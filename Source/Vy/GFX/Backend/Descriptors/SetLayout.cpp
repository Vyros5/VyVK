#include <Vy/GFX/Backend/Descriptors/SetLayout.h>

#include <Vy/GFX/Context.h>

#include <ranges>

namespace Vy
{
// ================================================================================================
#pragma region [ Builder ]
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
	VyDescriptorSetLayout::Builder::build() const
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

		VyContext::allocateSet(m_SetLayout, descriptorSet);
		
		// VyContext::descriptorPool().allocateDescriptorSet(m_SetLayout, descriptorSet);
		
        return descriptorSet;
	}

#pragma endregion [ Set Layout ]


// ================================================================================================
}