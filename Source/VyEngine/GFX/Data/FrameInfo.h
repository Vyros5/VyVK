#pragma once

#include <VyEngine/Scene/Scene.h>
#include <VyEngine/Scene/ECS/Components/LightComponent.h>
#include <VyEngine/Scene/ECS/Entity.h>
#include <VyEngine/Scene/Camera.h>

#include <VyEngine/GFX/Data/UBO.h>

namespace Vy 
{
    struct VyFrameInfo 
    {
        int              FrameIndex         { 0 };              // Index of the current frame.
        float            FrameTime          { 0.0f };           // Time between frames.
        VkCommandBuffer  CommandBuffer      { VK_NULL_HANDLE }; // Main command buffer.
        VkDescriptorSet  GlobalSet          { VK_NULL_HANDLE }; // Global descriptor set for the current frame.
        VkDescriptorSet  ShadowSet          { VK_NULL_HANDLE };
        VkDescriptorSet  MappingsSet        { VK_NULL_HANDLE };
        VkDescriptorSet  UVReflectionSet    { VK_NULL_HANDLE };
        VkDescriptorSet  GBufferSet         { VK_NULL_HANDLE };
        VkDescriptorSet  CompositionSet     { VK_NULL_HANDLE };
        VkDescriptorSet  PostProcessingSet  { VK_NULL_HANDLE };
        Shared<VyScene>& Scene              ;        // Active Scene.
        VyCamera&        Camera;                                // Active Camera to update the UBOs.
        // U32              SelectedObjectId; // ID of currently selected object (0 = camera)
        // EntityHandle     SelectedEntity;   // Selected entity handle
        // EntityHandle     CameraEntity;     // Camera entity handle
        // VkExtent2D       Extent;           // Screen extent
    };

    struct VyFrameInfo2 
    {
        int              FrameIndex         { 0 };              // Index of the current frame.
        float            FrameTime          { 0.0f };           // Time between frames.
        VkCommandBuffer  CommandBuffer      { VK_NULL_HANDLE }; // Main command buffer.
        VkDescriptorSet  GlobalSet          { VK_NULL_HANDLE }; // Global descriptor set for the current frame.
        Shared<VyScene>& Scene              ;                   // Active Scene.
        VyCamera&        Camera;                                // Active Camera to update the UBOs.
        // U32              SelectedObjectId; // ID of currently selected object (0 = camera)
        // EntityHandle     SelectedEntity;   // Selected entity handle
        // EntityHandle     CameraEntity;     // Camera entity handle
        // VkExtent2D       Extent;           // Screen extent
    };
}