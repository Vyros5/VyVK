#include <VyEngine/VK/Descriptors/Writer.h>

#include <VyEngine/VK/Context.h>

#include <ranges>

namespace Vy
{
// ================================================================================================
#pragma region [ Writer ]
// ================================================================================================

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
        bool bSuccess = m_Pool.allocate( m_SetLayout.handle(), set );
        
		if (!bSuccess) 
		{
            return false;
        }
        
		update( set );
        
		return true;
	}


    void VyDescriptorWriter::update(VkDescriptorSet& set) 
	{
        for (auto& write : m_Writes) 
		{
            write.dstSet = set;
        }

        vkUpdateDescriptorSets(VyContext::device(), static_cast<U32>(m_Writes.size()), m_Writes.data(), 0, nullptr);
    }

// ================================================================================================
}