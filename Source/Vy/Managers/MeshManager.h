#pragma once

#include <Vy/GFX/Resources/Mesh.h>
#include <Vy/GFX/Backend/Descriptors.h>
#include <Vy/GFX/Backend/Resources/Buffer.h>

namespace Vy
{
    using VyMeshID = U32;

    class VyMeshManager
    {
    public:
        VyMeshManager();
        ~VyMeshManager() = default;

        // Register a model and return its mesh ID
        VyMeshID registerModel(const Model* model);

        // Get the descriptor info for the global mesh buffer
        VkDescriptorBufferInfo descriptorBufferInfo() const;

        // Get the descriptor set layout binding for the mesh buffer
        static VkDescriptorSetLayoutBinding descriptorSetLayoutBinding();

    private:
        Unique<VyBuffer>                 m_MeshBuffer;
        TVector<MeshBuffers>             m_MeshInfos;
        THashMap<const Model*, VyMeshID> m_ModelToId;

        void updateBuffer();
    };
}