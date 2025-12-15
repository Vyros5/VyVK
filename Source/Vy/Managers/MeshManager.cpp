#include <Vy/Managers/MeshManager.h>

#include <Vy/GFX/Context.h>
#include <Vy/Globals.h>

namespace Vy
{
    VyMeshManager::VyMeshManager()
    {
        // Initialize with a dummy entry at index 0 so ID 0 can be "invalid" or "default"
        m_MeshInfos.push_back({ 0, 0 });
        
        updateBuffer();
    }


    VyMeshID VyMeshManager::registerModel(const Model* model)
    {
        if (m_ModelToId.find(model) != m_ModelToId.end())
        {
            return m_ModelToId[model];
        }

        VyMeshID id = static_cast<VyMeshID>(m_MeshInfos.size());

        MeshBuffers info{};
        {
            info.VertexBufferAddress = model->vertexBufferAddress();
            info.IndexBufferAddress  = model->indexBufferAddress();
        }

        m_MeshInfos.push_back(info);
        m_ModelToId[model] = id;

        updateBuffer();

        VY_INFO_TAG("VyMeshManager", "Registered model with ID {0} (VertexAddress: {1}, IndexAddress: {2})", 
            id, info.VertexBufferAddress, info.IndexBufferAddress);
        // std::cout << "[" << GREEN << "VyMeshManager" << RESET << "] Registered model with ID " << id << " (VA: " << info.vertexBufferAddress
                // << ", IA: " << info.indexBufferAddress << ")" << std::endl;
// 
        return id;
    }


    void VyMeshManager::updateBuffer()
    {
        VkDeviceSize bufferSize = sizeof(MeshBuffers) * m_MeshInfos.size();

        // Create a staging buffer
        VyBuffer stagingBuffer{ VyBuffer::stagingBuffer(sizeof(MeshBuffers), static_cast<U32>(m_MeshInfos.size())) };

        stagingBuffer.map();
        stagingBuffer.writeToBuffer(m_MeshInfos.data());

        // Create or resize the GPU buffer
        // Note: In a real engine, you might want to allocate a larger buffer upfront to avoid frequent reallocations
        m_MeshBuffer = std::make_unique<VyBuffer>( VyBuffer::storageBuffer(sizeof(MeshBuffers), static_cast<U32>(m_MeshInfos.size())) );

        VyContext::device().copyBuffer(
            stagingBuffer.handle(),
            m_MeshBuffer->handle(),
            bufferSize
        );
        // VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
        // VK_ACCESS_SHADER_READ_BIT);
    }


    VkDescriptorBufferInfo VyMeshManager::descriptorBufferInfo() const
    {
        return m_MeshBuffer->descriptorBufferInfo();
    }


    VkDescriptorSetLayoutBinding VyMeshManager::descriptorSetLayoutBinding()
    {
        VkDescriptorSetLayoutBinding binding{};
        {
            binding.binding            = 1; // Binding 1 in Set 0
            binding.descriptorType     = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            binding.descriptorCount    = 1;
            binding.stageFlags         = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_COMPUTE_BIT;
            binding.pImmutableSamplers = nullptr;
        }

        return binding;
    }
}