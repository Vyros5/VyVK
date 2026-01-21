#include <VyEngine/VK/Descriptors/SetLayout.h>

#include <VyEngine/VK/Context.h>

#include <ranges>

namespace Vy
{
// ================================================================================================
#pragma region [ Builder ]
// ================================================================================================
	
	VyDescriptorSetLayout::Builder& 
	VyDescriptorSetLayout::Builder::setName(const TString& name)
	{
		m_Name = name;

		return *this;
	}


	VyDescriptorSetLayout::Builder& 
	VyDescriptorSetLayout::Builder::addBinding(
		BindingIndex             binding, 
		VkDescriptorType         descriptorType,
		VkShaderStageFlags       stageFlags, 
		U32                      count,
		VkDescriptorBindingFlags bindingFlags)
    {
        VY_ASSERT(m_Bindings.count( binding ) == 0, "Binding already in use");

		VkDescriptorSetLayoutBinding layoutBinding{};
		{
			layoutBinding.binding            = binding;
			layoutBinding.descriptorType     = descriptorType;
			layoutBinding.descriptorCount    = count;
			layoutBinding.stageFlags         = stageFlags;
			layoutBinding.pImmutableSamplers = nullptr;
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
        VY_ASSERT(m_Bindings.count( binding ) == 1, "Binding does not exist");

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
	VyDescriptorSetLayout::Builder::buildPtr() const
	{
		return std::make_unique<VyDescriptorSetLayout>( m_Name, m_Bindings, m_BindingFlags, m_LayoutFlags );
	}


	VyDescriptorSetLayout 
	VyDescriptorSetLayout::Builder::build() const
	{
		return VyDescriptorSetLayout{ m_Name, m_Bindings, m_BindingFlags, m_LayoutFlags };
	}


#pragma endregion [ Set Layout Builder ]


// ================================================================================================
#pragma region [ Set Layout ]
// ================================================================================================

	VyDescriptorSetLayout::VyDescriptorSetLayout(
		const TString&                   name,
		VyLayoutBindingMap               bindings, 
		VyLayoutBindingFlagsMap          bindingFlags, 
		VkDescriptorSetLayoutCreateFlags layoutFlags
    ) : 
        m_Bindings { bindings                  }, 
		m_DebugName{ name + "_desc_set_layout" }
    {
		TVector<VkDescriptorSetLayoutBinding> setLayoutBindings    {};
		TVector<VkDescriptorBindingFlags>     setLayoutBindingFlags{};

		setLayoutBindings.reserve( m_Bindings.size() );

		// Sort bindings by binding index to ensure consistent order.
		TVector<BindingIndex> keys;
		// keys.reserve( bindings.size() );

		for (const auto& [ binding, _ ] : bindings)
		{
			keys.push_back( binding );
		}
		std::sort( keys.begin(), keys.end() );

        for (BindingIndex binding : keys) 
        {
            setLayoutBindings.push_back( m_Bindings.at( binding ) );

			// Add flags for this binding (0 if not specified).
			if (bindingFlags.count( binding ))
			{
				setLayoutBindingFlags.push_back( bindingFlags.at( binding ) );
			}
			else
			{
				setLayoutBindingFlags.push_back( 0 );
			}
        }

        auto bindingFlagsInfo{ VKInit::descriptorSetLayoutBindingFlagsCreateInfo() };
		{
			bindingFlagsInfo.bindingCount  = static_cast<U32>(setLayoutBindingFlags.size());
			bindingFlagsInfo.pBindingFlags = setLayoutBindingFlags.data();
		}

        auto setLayoutInfo{ VKInit::descriptorSetLayoutCreateInfo() };
        {
            setLayoutInfo.bindingCount = static_cast<U32>(setLayoutBindings.size());
            setLayoutInfo.pBindings    = setLayoutBindings.data();

			// Only set pNext if we have binding flags.
			if (!bindingFlags.empty()) 
			{
				for (auto flag : setLayoutBindingFlags)
				{
					if (flag & VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT)
					{
						setLayoutInfo.flags |= VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
						break;
					}
				}
				
				setLayoutInfo.pNext = &bindingFlagsInfo;
			}
			// Check if we need UPDATE_AFTER_BIND_POOL_BIT
        }

		VK_CHECK_SUCCESS(vkCreateDescriptorSetLayout(VyContext::device(), &setLayoutInfo, nullptr, &m_SetLayout), 
			"Failed to create descriptor set layout!" );

		VKDbg::setObjectName(VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT, (U64)m_SetLayout, m_DebugName.c_str());
	}


	VyDescriptorSetLayout::VyDescriptorSetLayout(VyDescriptorSetLayout&& other) noexcept : 
		m_SetLayout{ other.m_SetLayout           }, 
		m_Bindings { std::move(other.m_Bindings) },
		m_DebugName{ other.m_DebugName           }
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
			m_DebugName = other.m_DebugName;

			other.m_SetLayout = VK_NULL_HANDLE;
		}

		return *this;
	}


	VkDescriptorSet 
	VyDescriptorSetLayout::allocate() const
	{
		VkDescriptorSet descriptorSet{ VK_NULL_HANDLE };

		VyContext::allocateSet( m_SetLayout, descriptorSet );

        return descriptorSet;
	}

#pragma endregion [ Set Layout ]


// ================================================================================================
}