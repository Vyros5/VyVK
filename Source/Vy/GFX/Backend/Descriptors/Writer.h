#pragma once

#include <Vy/GFX/Backend/Device.h>

#include <Vy/GFX/Backend/Descriptors/SetLayout.h>
#include <Vy/GFX/Backend/Descriptors/Pool.h>

#include <Vy/GFX/Backend/Buffer/Buffer.h>

namespace Vy
{
// ================================================================================================
#pragma region [ Writer ]
// ================================================================================================
    class VyDescriptorWriter 
    {
    public:

        VyDescriptorWriter(
            VyDescriptorSetLayout& setLayout, 
            VyDescriptorPool&      pool
        );

        /**
         * @brief Writes a single buffer descriptor to the specified binding.
         * 
         * @param binding     The binding index.
         * @param pBufferInfo Pointer to the buffer descriptor info.
         * 
         * @return Reference to the VyDescriptorWriter instance.
         */
        VyDescriptorWriter& writeBuffer(
            BindingIndex            binding, 
            VkDescriptorBufferInfo* pBufferInfo) 
        {
            return write(binding, pBufferInfo, 1);
        }
    
        /**
         * @brief Writes multiple buffer descriptors to the specified binding.
         * 
         * @param binding      The binding index.
         * @param pBuffersInfo Pointer to the array of buffer descriptor infos.
         * @param count        The number of descriptors.
         * 
         * @return Reference to the VyDescriptorWriter instance.
         */
        VyDescriptorWriter& writeBuffers(
            BindingIndex            binding, 
            VkDescriptorBufferInfo* pBuffersInfo, 
            U32                     count) 
        {
            return write(binding, pBuffersInfo, count);
        }
    
        /**
         * @brief Writes a single image descriptor to the specified binding.
         * 
         * @param binding    The binding index.
         * @param pImageInfo Pointer to the image descriptor info.
         * 
         * @return Reference to the VyDescriptorWriter instance.
         */
        VyDescriptorWriter& writeImage(
            BindingIndex           binding, 
            VkDescriptorImageInfo* pImageInfo) 
        {
            return write(binding, pImageInfo, 1);
        }
    
        /**
         * @brief Writes multiple image descriptors to the specified binding.
         * 
         * @param binding     The binding index.
         * @param pImagesInfo Pointer to the array of image descriptor infos.
         * @param count       The number of descriptors.
         * 
         * @return Reference to the VyDescriptorWriter instance.
         */
        VyDescriptorWriter& writeImages(
            BindingIndex           binding, 
            VkDescriptorImageInfo* pImagesInfo, 
            U32                    count) 
        {
            return write(binding, pImagesInfo, count);
        }

		/**
		 * @brief Writes a single acceleration structure descriptor to the specified binding.
		 *
		 * @param binding   The binding index.
		 * @param writeInfo Acceleration structure descriptor info.
		 *
		 * @return Reference to the VyDescriptorWriter instance.
		 */
        VyDescriptorWriter& writeTLAS(
            BindingIndex                                 binding, 
            VkWriteDescriptorSetAccelerationStructureKHR writeInfo) 
        {
			return write(binding, &writeInfo, 1);
        }

        /**
         * @brief Allocate the sets from the pool and update them with the stored writes.
         * 
         * @param set Reference to the descriptor set to be allocated and updated.
         */
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
         * @param pInfo   Pointer to descriptor info.
         * @param count   Number of descriptors.
         * 
         * @return Reference to the VyDescriptorWriter instance.
         */
        template <typename T>
        VyDescriptorWriter& write(U32 binding, T* pInfo, U32 count) 
        {
			size_t bindingCount = m_SetLayout.m_Bindings.count(binding);

            VY_ASSERT(bindingCount == 1, "Layout does not contain specified binding");
            
            auto& bindingDescription = m_SetLayout.m_Bindings[binding];
            
            VY_ASSERT(bindingDescription.descriptorCount == count, "Binding descriptor info count mismatch");
            
            VkWriteDescriptorSet write{ VKInit::writeDescriptorSet() };
            {
                write.descriptorType  = bindingDescription.descriptorType;
                write.dstBinding      = binding;
                write.descriptorCount = count;
                
                if constexpr (std::is_same_v<T, VkDescriptorBufferInfo>) 
                {
                    write.pBufferInfo = pInfo;
                } 
                else if constexpr (std::is_same_v<T, VkDescriptorImageInfo>) 
                {
                    write.pImageInfo  = pInfo;
                } 
                else if constexpr (std::is_same_v<T, VkWriteDescriptorSetAccelerationStructureKHR>) 
                {
                    write.pNext       = pInfo;
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
}