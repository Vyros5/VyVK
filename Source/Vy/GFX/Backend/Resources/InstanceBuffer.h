#pragma once

#include <Vy/GFX/Backend/Resources/Buffer.h>

namespace Vy
{
    class VyInstanceBuffer //: public InstanceBuffer
    {

    public:
        VyInstanceBuffer(U32 numInstances);

        ~VyInstanceBuffer();

        VyInstanceBuffer(const VyInstanceBuffer&)            = delete;
        VyInstanceBuffer& operator=(const VyInstanceBuffer&) = delete;

        void setInstanceData(U32 index, const Mat4& mat4Global, const Mat4& normalMatrix);
        
        const Mat4& modelMatrix(U32 index);
        
        const Mat4& normalMatrix(U32 index);
        
        Shared<VyBuffer> buffer();
        
        void update();

    private:
        struct InstanceData
        {
            Mat4 ModelMatrix;
            Mat4 NormalMatrix;
        };

        U32                   m_NumInstances;
        bool                  m_Dirty;
        TVector<InstanceData> m_DataInstances;
        Shared<VyBuffer>      m_UBO;
    };
}