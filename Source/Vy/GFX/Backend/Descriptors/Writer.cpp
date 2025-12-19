#include <Vy/GFX/Backend/Descriptors/Writer.h>

#include <Vy/GFX/Context.h>

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
}