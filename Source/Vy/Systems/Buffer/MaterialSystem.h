#pragma once

#include <Vy/Systems/Rendering/IRenderSystem.h>

#include <Vy/GFX/Backend/Descriptors.h>
#include <Vy/GFX/Backend/Device.h>

namespace Vy 
{
    class VyMaterialSystem //: public IRenderSystem
    {
    public:
        VyMaterialSystem() = default;
        
        VyMaterialSystem(const VyMaterialSystem&)            = delete;
        VyMaterialSystem& operator=(const VyMaterialSystem&) = delete;

        ~VyMaterialSystem();// override;

        Shared<VyDescriptorSetLayout> createMaterialSetLayout();

        void updateMaterials(const VyFrameInfo& frameInfo, VyDescriptorSetLayout& materialSetLayout, VyDescriptorPool& materialPool);
    };
}