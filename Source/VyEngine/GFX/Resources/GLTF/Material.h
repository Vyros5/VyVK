#pragma once

#include <VyEngine/GFX/Resources/Mesh/Vertex.h>
#include <VyEngine/GFX/Resources/Texture/Texture.h>

#include <VyEngine/VK/Buffer/Buffer.h>
#include <VyEngine/VK/Descriptors/Descriptors.h>
#include <VyEngine/VK/Pipeline/Pipeline.h>
#include <VyEngine/VK/Swapchain/Swapchain.h>

namespace Vy
{
    enum struct EMaterialType 
    {
        PBR,
        NPR,
        UNLIT,
        SCROLLING_UV,
        SHADOW_MAP
    };

    // Material Data loaded from the model file
    struct PbrMetallicRoughness 
    {
        glm::vec4 baseColorFactor{1.0f, 1.0f, 1.0f, 1.0f};
        float metallicFactor{1.0f};
        float roughnessFactor{1.0f};

        Shared<VyTexture> baseColorTexture;
        Shared<VyTexture> metallicRoughnessTexture;

        int baseColorTexCoord{0};
        int metallicRoughnessTexCoord{0};
    };

    enum class EAlphaMode 
    {
        Opaque,
        Mask,
        Blend
    };

    struct MaterialData 
    {
        TString name;

        PbrMetallicRoughness pbrMetallicRoughness;

        glm::vec2 uvScale{1.0f, 1.0f};
        glm::vec2 uvOffset{0.0f, 0.0f};
        float uvRotation{0.0f};

        Shared<VyTexture> normalTexture;

        float normalScale{1.0f};

        EAlphaMode alphaMode{EAlphaMode::Opaque};
        float alphaCutoff{0.5f};

        bool doubleSided{false};

        int normalTexCoord{0};
    };

    // Material shader
    class VyMaterial 
    {
    public:
        struct Config 
        {
            EMaterialType type;
            TString vertexShader;
            TString fragmentShader;
            VkCullModeFlags cullMode = VK_CULL_MODE_BACK_BIT;
            bool bAlphaBlending = false;
            bool depthWrite = true;
        };

        VyMaterial(const Config& config, 
            VkDescriptorSetLayout globalSetLayout,
            VkDescriptorSetLayout modelSetLayout,
            VkDescriptorSetLayout boneSetLayout,
            VySwapchain&          swapchain
        );

        ~VyMaterial();

        void bind(VkCommandBuffer commandBuffer);

        VkPipeline       pipeline()       const { return m_Pipeline->handle(); }
        VkPipelineLayout pipelineLayout() const { return m_Pipeline->layout(); }
        EMaterialType    type()           const { return m_Type; }

    private:
        EMaterialType m_Type;
        std::unique_ptr<VyPipeline> m_Pipeline;
        // VkPipelineLayout pipelineLayout;
    };

    class VyMaterialLibrary 
    {
    public:
        VyMaterialLibrary(
            VkDescriptorSetLayout globalSetLayout,
            VkDescriptorSetLayout modelSetLayout,
            VkDescriptorSetLayout boneSetLayout,
            VySwapchain&          swapchain
        );

        // Get material by type (or create one if it doesn't exit)
        Shared<VyMaterial> getMaterial(EMaterialType type);

        void registerMaterial(EMaterialType type, const VyMaterial::Config& config);

    private:
        VkDescriptorSetLayout globalSetLayout;
        VkDescriptorSetLayout modelSetLayout;
        VkDescriptorSetLayout boneSetLayout;
        VySwapchain& swapChain;
        THashMap<EMaterialType, Shared<VyMaterial>> materials;

        void createDefaultMaterials();
    };
}