#pragma once

#include <VyEngine/GFX/Data/FrameInfo.h>

#include <VyEngine/VK/Pipeline/Pipeline.h>
#include <VyEngine/VK/Descriptors/Descriptors.h>
#include <VyEngine/VK/Buffer/Buffer.h>

namespace Vy
{
    class BRDFLUTGenerationSystem
    {
    public:
        BRDFLUTGenerationSystem(VkRenderPass renderPass);
        
        ~BRDFLUTGenerationSystem();

        // Non-copyable
        BRDFLUTGenerationSystem(const BRDFLUTGenerationSystem&)            = delete;
        BRDFLUTGenerationSystem& operator=(const BRDFLUTGenerationSystem&) = delete;

        void generate(VkCommandBuffer cmdBuffer, U32 width, U32 height);

    private:
        void createPipeline(VkRenderPass renderPass);

        Unique<VyPipeline> m_Pipeline;
        Unique<VyBuffer>   m_VertexBuffer;
    };
}