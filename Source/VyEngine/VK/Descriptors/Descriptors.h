#pragma once

#include <VyEngine/VK/Descriptors/Pool.h>
#include <VyEngine/VK/Descriptors/SetLayout.h>
#include <VyEngine/VK/Descriptors/Writer.h>

namespace Vy
{
    struct VyDescriptorSet
    {
        VkDescriptorSet               Set = VK_NULL_HANDLE;
        VyDescriptorSetLayout&        SetLayout;
        TVector<VkWriteDescriptorSet> Writes;

        VyDescriptorSet() = delete;

        explicit VyDescriptorSet(VyDescriptorSetLayout& layout) :
            SetLayout( layout )
        {
        }

        /**
         * @brief Configure buffer which is part of descriptor
         * @param binding number in the set
         * @param dstIndex index of the element which has to be configured
         * @param count number of descriptors to update
         * @param pBufferInfo contains buffer details for linking to the descriptor
         * @return gfx descriptor set
         */
        VyDescriptorSet& configureBuffer(
            BindingIndex            binding,
            U32                     dstIndex,
            U32                     count,
            VkDescriptorBufferInfo* pBufferInfo);

        /**
         * @brief Configure image which is part of descriptor
         * @param binding number in the set
         * @param dstIndex index of the element which has to be configured
         * @param count number of descriptors to update
         * @param pImageInfo contains image deatils for linking to the descriptor
         * @return gfx descriptor set
         */
        VyDescriptorSet& configureImage(
            BindingIndex           binding,
            U32                    dstIndex,
            U32                    count,
            VkDescriptorImageInfo* pImageInfo);

        /**
         * @brief apply buffer/image configurations on the set. It will overwrite any
         * existing configurations
         */
        void apply();
    };
} 
