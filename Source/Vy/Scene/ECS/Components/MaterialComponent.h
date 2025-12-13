#pragma once

#include <Vy/GFX/Resources/Material.h>
// #include <VyVK/Asset/Types/MaterialAsset.h>

namespace Vy
{
   struct MaterialComponent 
    {
        Shared<VyMaterial> Material;
        
        void setColor(const Vec3& color) 
        { 
            if (Material) 
            {
                Material->setAlbedo(color); 
            }
        }
        
        void loadAlbedoTexture(const String& filepath) 
        { 
            if (Material) 
            {
                Material->loadAlbedoTexture(filepath); 
            }
        }
        
        void loadNormalTexture(const String& filepath) 
        {
            if (Material) 
            {
                Material->loadNormalTexture(filepath); 
            }
        }
        
        void setMetallic(float metallic) 
        { 
            if (Material) 
            {
                Material->setMetallic(metallic); 
            }
        }
        
        void setRoughness(float roughness) 
        { 
            if (Material) 
            {
                Material->setRoughness(roughness); 
            }
        }

        void setEmission(const Vec3& color, float strength) 
        {
            if (Material) 
            { 
                Material->setEmissionColor(color); 
                Material->setEmissionStrength(strength); 
            }
        }

        void setEmissionColor(const Vec3& color) 
        {
            if (Material) 
            {
                Material->setEmissionColor(color);
            }
        }

        void setEmissionStrength(float strength) 
        {
            if (Material) 
            {
                Material->setEmissionStrength(strength);
            }
        }
    };
}
// struct MaterialComponent 
// {
// 	Shared<VyMaterialAsset> Material;
// 	float                   TilingFactor{ 1.0f };
// 	Vec3                    Tint        { 1.0f };

// 	MaterialComponent() = default;

// 	MaterialComponent(const MaterialComponent&) = default;

// 	MaterialComponent(const Shared<VyMaterialAsset>& material, float tilingFactor, const Vec3& tint) : 
// 		Material(material), 
// 		TilingFactor(tilingFactor), 
// 		Tint(tint) 
// 	{
// 	}

// 	struct Builder 
// 	{
// 		Shared<VyMaterialAsset> Material;
// 		float                   TilingFactor{ 1.0f };
// 		Vec3                    Tint        { 1.0f };

// 		Builder& setMaterial(const Shared<VyMaterialAsset>& material) 
// 		{
// 			Material = material;
// 			return *this;
// 		}

// 		Builder& setTilingFactor(float tilingFactor) 
// 		{
// 			TilingFactor = tilingFactor;
// 			return *this;
// 		}

// 		Builder& setTint(const Vec3& tint) 
// 		{
// 			Tint = tint;
// 			return *this;
// 		}

// 		MaterialComponent build()
// 		{
// 			return MaterialComponent(Material, TilingFactor, Tint);
// 		}
// 	};

// };