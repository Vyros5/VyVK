#pragma once

#include <VyEngine/GFX/Resources/Mesh/Material.h>

namespace Vy
{
    struct MaterialComponent 
    {
        Shared<VyMaterial> Material;
        
        void setColor(const Vec3& color) 
        {
            if (Material) Material->setAlbedo(color); 
        }
        
        void setMetallic(float metallic) 
        { 
            if (Material) Material->setMetallic(metallic); 
        }
        
        void setRoughness(float roughness) 
        { 
            if (Material) Material->setRoughness(roughness); 
        }

        void setEmission(const Vec3& color, float strength) 
        {
            if (Material) { Material->setEmissionColor(color); Material->setEmissionStrength(strength); }
        }

        void setEmissionColor(const Vec3& color) 
        {
            if (Material) Material->setEmissionColor(color);
        }

        void setEmissionStrength(float strength) 
        {
            if (Material) Material->setEmissionStrength(strength);
        }

        void loadAlbedoTexture(const TString& filepath) 
        {
            if (Material) Material->loadAlbedoTexture(filepath); 
        }
        
        void loadNormalTexture(const TString& filepath) 
        {
            if (Material) Material->loadNormalTexture(filepath); 
        }
    };
}