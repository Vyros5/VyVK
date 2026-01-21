#include <VyEngine/VK/Descriptors/Descriptors.h>

#include <VyEngine/VK/Context.h>

namespace Vy
{
    VyDescriptorSet& VyDescriptorSet::configureBuffer(
        BindingIndex            binding,
        U32                     dstIndex,
        U32                     count,
        VkDescriptorBufferInfo* pBufferInfo)
    {
        size_t bindingCount = SetLayout.m_Bindings.count( binding );

        VY_ASSERT(bindingCount == 1, "Layout does not contain specified binding");

        auto& bindingDescription = SetLayout.m_Bindings[ binding ];

        VkWriteDescriptorSet write{ VKInit::writeDescriptorSet() };
        {
            write.descriptorType  = bindingDescription.descriptorType;
            write.dstBinding      = binding;
            write.pBufferInfo     = pBufferInfo;
            write.descriptorCount = count;
            write.dstArrayElement = dstIndex;
        }

        Writes.push_back( write );

        return *this;
    }


    VyDescriptorSet& VyDescriptorSet::configureImage(
        BindingIndex            binding,
        U32                     dstIndex,
        U32                     count,
        VkDescriptorImageInfo*  pImageInfo)
    {
        size_t bindingCount = SetLayout.m_Bindings.count( binding );

        VY_ASSERT(bindingCount == 1, "Layout does not contain specified binding");

        auto& bindingDescription = SetLayout.m_Bindings[ binding ];

        VkWriteDescriptorSet write{ VKInit::writeDescriptorSet() };
        {
            write.descriptorType  = bindingDescription.descriptorType;
            write.dstBinding      = binding;
            write.pImageInfo      = pImageInfo;
            write.descriptorCount = count;
            write.dstArrayElement = dstIndex;
        }

        Writes.push_back( write );

        return *this;
    }
    

    void VyDescriptorSet::apply()
    {
        if (Writes.empty()) return;

        for (auto& write : Writes)
        {
            write.dstSet = this->Set;
        }

        vkUpdateDescriptorSets(VyContext::device(), static_cast<U32>(Writes.size()), Writes.data(), 0, nullptr);

        Writes.clear();
    }
}