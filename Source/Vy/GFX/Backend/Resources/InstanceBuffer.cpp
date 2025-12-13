#include <Vy/GFX/Backend/Resources/InstanceBuffer.h>

#include <Vy/GFX/Context.h>

namespace Vy
{
    VyInstanceBuffer::VyInstanceBuffer(U32 numInstances) : 
        m_NumInstances{ numInstances }, 
        m_Dirty       { true }
    {
        m_UBO = MakeShared<VyBuffer>( VyBuffer::uniformBuffer(sizeof(InstanceData)), false );

        m_UBO->map();
        m_DataInstances.resize(numInstances);
    }

    VyInstanceBuffer::~VyInstanceBuffer()
    {
    }

    void VyInstanceBuffer::setInstanceData(U32 index, const Mat4& mat4Global, const Mat4& normalMatrix)
    {
        VY_ASSERT(index < m_NumInstances, "out of bounds");

        m_DataInstances[index].ModelMatrix  = mat4Global;
        m_DataInstances[index].NormalMatrix = normalMatrix;

        m_Dirty = true;
    }
    
    const Mat4& VyInstanceBuffer::modelMatrix(U32 index)
    {
        return m_DataInstances[index].ModelMatrix;
    }
    
    const Mat4& VyInstanceBuffer::normalMatrix(U32 index)
    {
        return m_DataInstances[index].NormalMatrix;
    }
    
    Shared<VyBuffer> VyInstanceBuffer::buffer()
    {
        return m_UBO;
    }
    
    void VyInstanceBuffer::update()
    {
        if (m_Dirty)
        {
            // Update the UBO.
            m_UBO->writeToBuffer(m_DataInstances.data(), sizeof(InstanceData)/*  * m_NumInstances */, 0);

            m_Dirty = false;
        }
    }
}