#pragma once

#include <VyEngine/GFX/Data/FrameInfo.h>

#include <VyEngine/VK/Pipeline/Pipeline.h>
#include <VyEngine/VK/Descriptors/Descriptors.h>

#include <VyEngine/Scene/ECS/Components.h>

#define CASCADE_SHADOW_MAP_COUNT 4

namespace Vy
{
    class VyRenderSystem
    {
    public:

        enum PushConstantType
        {
            MAIN           = 0,
            POINTSHADOW    = 1,
            SPOTSHADOW     = 2,
            CASCADEDSHADOW = 3
        };

        struct FramebufferAttachment 
        {
            VyImage     Image;
            VyImageView View;
        };

        struct ShadowPass 
        {
            VkExtent2D            Extent;
            VkFramebuffer         Framebuffer;
            VkRenderPass          RenderPass;
            // FramebufferAttachment ShadowMapImage;
            // VkSampler             ShadowMapSampler;
            // VkDescriptorImageInfo Descriptor;
        };

        struct CascadedShadowPassUBO
        {
            TArray<Mat4,  CASCADE_SHADOW_MAP_COUNT> ViewProjMats;
            TArray<float, CASCADE_SHADOW_MAP_COUNT> SplitDepths;
        };

        struct Cascade
        {
            VkFramebuffer Framebuffer;
            VyImageView   View;
        };

        struct CascadedShadowPass
        {
            using CascadeArr = TArray<Cascade, CASCADE_SHADOW_MAP_COUNT>;

            VkRenderPass          RenderPass;
            CascadeArr            Cascades;
            CascadedShadowPassUBO UBO;
        };

        struct CascadedDepthMap
        {
            VyImage     Image;
            VyImageView View;
            VySampler   Sampler;
        };

        struct ShadowPassUBO
        {
            Mat4 LightProjection;
        };

        struct PointShadowPass 
        {
            using PointFramebuffers = TArray<TArray<VkFramebuffer, 6>, MAX_POINT_LIGHTS>;

            VkExtent2D            Extent;
            PointFramebuffers     Framebuffers;
            FramebufferAttachment Attachment;
            VkRenderPass          RenderPass;
            // VkSampler             PointShadowMapSampler;
            // VkDescriptorImageInfo Descriptor;
        };

        struct TextureArray 
        {
            VkExtent2D            Extent;
            FramebufferAttachment Attachment;
            VySampler             Sampler;
        };

        struct PointShadowPassViewMatrixUBO
        {
            TArray<Mat4, 6> FaceViewMatrix;
        };

        struct SpotShadowPass
        {
            VkExtent2D                             Extent;
            TArray<VkFramebuffer, MAX_SPOT_LIGHTS> Framebuffers;
            TArray<VkImageView, MAX_SPOT_LIGHTS>   ImageViews;
            FramebufferAttachment                  Attachment;
            VkRenderPass                           RenderPass;
            // VySampler                              Sampler;
        };

        struct SpotShadowLightProjectionsUBO
        {
            TArray<Mat4, MAX_SPOT_LIGHTS> LightProjections;
        };

        VyRenderSystem(
            VkRenderPass                   renderPass, 
            TVector<VkDescriptorSetLayout> setLayouts,
            VyDescriptorPool&              descriptorPool
        );
        
        ~VyRenderSystem();

        VyRenderSystem(const VyRenderSystem&) = delete;
        VyRenderSystem& operator=(const VyRenderSystem&) = delete;

        // void renderShadowPass(VyFrameInfo frameInfo, GlobalUbo& globalUBO);
        void renderCascadedShadowPass(VyFrameInfo frameInfo, GlobalUbo& globalUBO);
        void renderPointShadowPass(VyFrameInfo frameInfo, GlobalUbo& globalUBO);
        void renderSpotShadowPass(VyFrameInfo frameInfo, GlobalUbo& globalUBO);
        void renderMainPass(VyFrameInfo frameInfo);

        void renderObjects(VyFrameInfo frameInfo, PushConstantType type, int setCount, bool bRenderMaterial = true);

    private:
        void createPipeline(VkRenderPass renderpass, TVector<VkDescriptorSetLayout> setLayouts, VyDescriptorPool& descriptorPool);

        void prepareShadowPassUBO();

        void prepareShadowPassRenderpass();
        // void prepareShadowPassFramebuffer();
        void updateShadowPassBuffer(GlobalUbo& globalUBO);

        void prepareCascadeShadowPass();
        void updateCascades(GlobalUbo& globalUBO);

        void preparePointShadowCubeMaps();
        void preparePointShadowPassRenderPass();
        void preparePointShadowPassFramebuffers();

        void updateCubeFace(U32 faceIndex, VyFrameInfo frameInfo, GlobalUbo& globalUBO);

        void prepareSpotShadowMaps();
        void prepareSpotShadowPassRenderPass();
        void prepareSpotShadowPassFramebuffers();

        void updateSpotShadowMaps(U32 lightIndex, VyFrameInfo frameInfo, GlobalUbo& globalUBO);

        // Main Pipeline variables
        Unique<VyPipeline> m_MainPipeline;

        VkDescriptorSet m_ShadowMapDescriptorSet;

        CascadedDepthMap m_CascadedDepthMapObject;
        VkDescriptorSet m_CascadedShadowMapDescriptorSet;

        TextureArray m_PointShadowCubeMaps{};
        VkDescriptorSet m_PointShadowMapDescriptorSet;

        TextureArray m_SpotShadowMaps{};
        VkDescriptorSet m_SpotShadowMapDescriptorSet;

        SpotShadowLightProjectionsUBO m_SpotShadowLightProjectionsUBO{};
        Unique<VyBuffer> m_SpotShadowLightProjectionsBuffer;
        VkDescriptorSet m_SpotShadowLightProjectionsDescriptorSet;

        // Directional Shadow variables
        // Unique<VyPipeline> m_ShadowPassPipeline;

        const VkFormat m_ShadowPassImageFormat{ VK_FORMAT_D16_UNORM };
        // const U32 m_ShadowMapSize{ 4096 };

        ShadowPassUBO m_ShadowPassUBO;
        Unique<VyBuffer> m_ShadowPassBuffer;
        // VkDescriptorSet m_ShadowPassDescriptorSet;

        ShadowPass m_ShadowPass{};

        // Cascaded Shadow Map
        Unique<VyPipeline> m_CascadedShadowPassPipeline;

        const U32 m_CascadedShadowMapSize{4096};

        Unique<VyBuffer> m_CascadedShadowPassBuffer;
        VkDescriptorSet m_CascadedShadowPassDescriptorSet;

        CascadedShadowPass m_CascadedShadowPass{};

        int m_CascadeIndex = 0;

        //Point Shadow variables
        Unique<VyPipeline> m_PointShadowPassPipeline;
        // VkPipelineLayout   m_PointShadowPassPipelineLayout;

        const U32      m_PointShadowMapSize{ 1024 };
        const VkFormat m_PointShadowPassImageFormat{ VK_FORMAT_R32_SFLOAT };
        VkFormat       m_PointShadowPassDepthFormat{ VK_FORMAT_UNDEFINED };

        PointShadowPassViewMatrixUBO m_PointShadowPassUBO {};
        Unique<VyBuffer>             m_PointShadowPassBuffer;
        VkDescriptorSet              m_PointShadowPassDescriptorSet;

        PointShadowPass m_PointShadowPass{};
        TArray<TArray<VyImageView, 6>, MAX_POINT_LIGHTS> m_PointShadowCubeMapImageViews{};
        U32 m_PointLightCount = 0;
        int m_FaceCount = 0;

        //Spot Shadow variables
        Unique<VyPipeline> m_SpotShadowPassPipeline;
        // VkPipelineLayout m_SpotShadowPassPipelineLayout;

        const U32 m_SpotShadowMapSize{ 1024 };
        const VkFormat m_SpotShadowPassImageFormat{ VK_FORMAT_R32_SFLOAT };
        VkFormat m_SpotShadowPassDepthFormat{ VK_FORMAT_UNDEFINED };

        ShadowPassUBO m_SpotShadowPassUBO;
        Unique<VyBuffer> m_SpotShadowPassBuffer;
        VkDescriptorSet m_SpotShadowPassDescriptorSet;

        SpotShadowPass m_SpotShadowPass{};

        int m_SpotLightIndex = 0;
    };
}