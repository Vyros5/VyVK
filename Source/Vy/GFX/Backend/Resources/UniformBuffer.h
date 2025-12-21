#pragma once

#include <Vy/GFX/Backend/Buffer/Buffer.h>

#include <Vy/GFX/Context.h>


namespace Vy
{
    template <typename T>
    class VyUniformBuffer 
    {
    public:
        
        explicit VyUniformBuffer();


        void update(int frameIndex, const T& data);


        VY_NODISCARD VyBuffer* getBuffer(int frameIndex) const;


        VY_NODISCARD VkDescriptorBufferInfo descriptorBufferInfo(U32 frameIndex) const;


        VY_NODISCARD const TVector<Unique<VyBuffer>>& getBuffers() const 
        {
            return m_Buffers;
        }


        VY_NODISCARD static size_t getSize() 
        {
            return sizeof(T);
        }


        VyBuffer* operator[](int frameIndex) const 
        {
            if (frameIndex < 0 || frameIndex >= MAX_FRAMES_IN_FLIGHT) 
            {
                throw std::out_of_range("Frame index out of range");
            }
            
            return m_Buffers[frameIndex].get();
        }

    private:

        void createBuffers();

        TVector<Unique<VyBuffer>> m_Buffers;
    };


    template <typename T>
    VyUniformBuffer<T>::VyUniformBuffer()
    {
        createBuffers();
    }


    template <typename T>
    void VyUniformBuffer<T>::update(int frameIndex, const T& data) 
    {
        if (frameIndex < 0 || frameIndex >= MAX_FRAMES_IN_FLIGHT) 
        {
            throw std::out_of_range("Frame index out of range");
        }
        
        m_Buffers[frameIndex]->writeToBuffer(&data, sizeof(T));
        m_Buffers[frameIndex]->flush();
    }


    template <typename T>
    VyBuffer* VyUniformBuffer<T>::getBuffer(int frameIndex) const 
    {
        return m_Buffers[ frameIndex ].get();
    }


    template <typename T>
    VkDescriptorBufferInfo VyUniformBuffer<T>::descriptorBufferInfo(U32 frameIndex) const 
    {
        VkDescriptorBufferInfo bufferInfo{};
        {
            bufferInfo.buffer = m_Buffers[frameIndex]->getBuffer();
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(T);
        }

        return bufferInfo;
    }


    template <typename T>
    void VyUniformBuffer<T>::createBuffers() 
    {
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) 
        {
            m_Buffers.emplace_back( MakeUnique<VyBuffer>( VyBuffer::uniformBuffer(sizeof(T)), false ));
        }
    }
}