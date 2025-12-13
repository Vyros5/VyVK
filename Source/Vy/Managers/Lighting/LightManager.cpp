#include <Vy/Managers/Lighting/LightManager.h>


#include <Vy/GFX/Context.h>
#include <Vy/Globals.h>

namespace Vy
{
    VyLightManager::VyLightManager(U32 framesInFlight, U32 initialCapacity) :
        m_FramesInFlight(framesInFlight) 
    {
        m_LightSSBOPool = VyDescriptorPool::Builder{}
            .setMaxSets(framesInFlight)
            .addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, framesInFlight)
            .buildUnique();

        m_LightSSBOSetLayout = VyDescriptorSetLayout::Builder{}
            .addBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
            .buildUnique();

        m_LightSSBODescriptorSets.resize(framesInFlight);

        createBuffers(std::max(16u, initialCapacity));

        allocateDescriptorSets();
    }


    VyLightManager::~VyLightManager() 
    {
        destroyBuffers();
    }


    void VyLightManager::allocateDescriptorSets() 
    {
        for (int i = 0; i < m_LightSSBODescriptorSets.size(); i++) 
        {
            auto bufferInfo = m_PerFrameLightBuffers[i].Buffer->descriptorBufferInfo();

            VyDescriptorWriter{ *m_LightSSBOSetLayout, *m_LightSSBOPool }
                .writeBuffer(0, &bufferInfo)
                .build(m_LightSSBODescriptorSets[i]);
        }
    }

    
    void VyLightManager::rewriteDescriptorSets() 
    {
        for (int i = 0; i < m_LightSSBODescriptorSets.size(); i++) 
        {
            auto bufferInfo = m_PerFrameLightBuffers[i].Buffer->descriptorBufferInfo();
        
            VyDescriptorWriter{ *m_LightSSBOSetLayout, *m_LightSSBOPool }
                .writeBuffer(0, &bufferInfo)
                .update(m_LightSSBODescriptorSets[i]);
        }
    }


    
    void VyLightManager::createBuffers(U32 capacityLights) 
    {
        destroyBuffers();

        m_Capacity = capacityLights;

        m_PerFrameLightBuffers.resize(m_FramesInFlight);

        const VkDeviceSize totalSize = sizeof(LightSSBOHeader) + sizeof(GPULight) * static_cast<VkDeviceSize>(capacityLights);

        for (PerFrame& perFrame : m_PerFrameLightBuffers) 
        {
            perFrame.Buffer = MakeUnique<VyBuffer>( VyBuffer::storageBuffer(totalSize, 1, VK_BUFFER_USAGE_TRANSFER_SRC_BIT) );

            perFrame.Buffer->map();

            perFrame.TotalSize = totalSize;
        }
    }


    void VyLightManager::destroyBuffers() 
    {
        for (PerFrame& perFrame: m_PerFrameLightBuffers) 
        {
            if (perFrame.Buffer) 
            {
                perFrame.Buffer.reset();
            }
        
            perFrame.TotalSize = 0;
        }

        m_Capacity = 0;
    }

    
    void VyLightManager::clear() 
    {
        m_GPULights.clear();
    }

    
    U32 VyLightManager::addLight(GPULight& light) 
    {
        m_GPULights.push_back(light);

        return static_cast<U32>(m_GPULights.size() - 1);
    }


    void VyLightManager::setLight(U32 index, GPULight& light) 
    {
        if (index < m_GPULights.size()) 
        {
            m_GPULights[ index ] = light;
        }
    }

    
    void VyLightManager::reserve(U32 minCapacity) 
    {
        if (minCapacity <= m_Capacity) return;

        const U32 newCapacity = std::max( minCapacity, (m_Capacity > 0) ? (m_Capacity + m_Capacity / 2) : 128u );

        createBuffers(newCapacity);

        rewriteDescriptorSets();
    }


    void VyLightManager::upload(U32 frameIndex) 
    {
        if (m_GPULights.size() > m_Capacity) 
        {
            reserve(static_cast<U32>(m_GPULights.size()));
        }

        m_Header.Count = static_cast<U32>(m_GPULights.size());

        PerFrame& perFrame = m_PerFrameLightBuffers[ frameIndex ];

        // Write Header
        perFrame.Buffer->writeToBuffer(&m_Header, sizeof(m_Header), 0);

        // Write light positions
        if (!m_GPULights.empty()) 
        {
            perFrame.Buffer->writeToBuffer(m_GPULights.data(),
                static_cast<VkDeviceSize>(m_GPULights.size() * sizeof(GPULight)),
                sizeof(m_Header)
            );
        }
    }
}