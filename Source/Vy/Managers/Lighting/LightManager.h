#pragma once

#include <Vy/GFX/Backend/Resources/Image.h>
#include <Vy/GFX/Backend/Resources/ImageView.h>

#include <Vy/GFX/Backend/Descriptors.h>
#include <Vy/GFX/Backend/Device.h>

#define SHADOW_MAP_CASCADE_COUNT 4

namespace Vy
{
    // 16B alignment
    struct GPULight 
    {
        Vec4 Color;     // rgb, a = intensity
        Vec4 Position;  // xyz, w = type (0/1/2)
        Vec4 Direction; // xyz, w = unused
        Vec4 Param;     // x = range, y = innerCos, z = outerCos, w = specPow
    };


    struct alignas(16) LightSSBOHeader 
    {
        U32 Count{ 0 };

        U32 pad1 { 0 };
        U32 pad2 { 0 }; 
        U32 pad3 { 0 };
    };


    class VyLightManager 
    {
    public:
        VyLightManager(U32 framesInFlight, U32 initialCapacity);

        ~VyLightManager();

        VyLightManager(const VyLightManager&)            = delete;
        VyLightManager& operator=(const VyLightManager&) = delete;

        // CPU side

        // call once per frame, before filling
        void clear();

        // return index
        U32 addLight(GPULight& light);

        // modify properties of light at a specific index
        void setLight(U32 index, GPULight& light);
        
        // optional manual grow
        void reserve(U32 minCapacity);

        // Upload SSBO to gpu at index frameIndex
        void upload(U32 frameIndex);  

        // void createOrthographicProjection();

        // Return descriptor index
        VkDescriptorSet       descriptorSet(U32 frameIndex) const { return m_LightSSBODescriptorSets[frameIndex]; }
        VkDescriptorSetLayout descriptorLayout()            const { return m_LightSSBOSetLayout->handle();        }

    private:
        struct PerFrame 
        {
            Unique<VyBuffer> Buffer; // single blob: Header + Lights[]
            VkDeviceSize     TotalSize{ 0 };
        };

        void allocateDescriptorSets();
        
        void createBuffers(U32 capacityLights);
        
        void destroyBuffers();

        void rewriteDescriptorSets(); // (re)point sets to the per-frame buffers

        U32 m_FramesInFlight;

        // CPU side Buffer
        TVector<GPULight> m_GPULights{};
        LightSSBOHeader   m_Header   {}; // Filled during upload

        // GPU side resources
        TVector<PerFrame> m_PerFrameLightBuffers;
        U32               m_Capacity{ 0 };

        Unique<VyDescriptorPool>      m_LightSSBOPool          {};
        Unique<VyDescriptorSetLayout> m_LightSSBOSetLayout     {};
        TVector<VkDescriptorSet>      m_LightSSBODescriptorSets{};
    };
}