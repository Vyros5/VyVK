#pragma once

#include <Vy/Systems/Rendering/IRenderSystem.h>

#include <Vy/GFX/Backend/Descriptors.h>
#include <Vy/GFX/Backend/Device.h>

#define CASCADE_SHADOW_MAP_COUNT 4

namespace Vy 
{
    class VyRenderSystem : public IRenderSystem
    {
    public:
        VyRenderSystem(
            VkRenderPass                   renderPass, 
            TVector<VkDescriptorSetLayout> descSetLayouts
        );

        VyRenderSystem(const VyRenderSystem&)            = delete;
        VyRenderSystem& operator=(const VyRenderSystem&) = delete;

        ~VyRenderSystem() override;
        
        virtual void render(const VyFrameInfo& frameInfo) override;

        void createPipeline(VkRenderPass& renderPass, TVector<VkDescriptorSetLayout> descSetLayouts);

    private:
        Unique<VyPipeline> m_Pipeline;
    };
}



namespace Vy
{
    class SimpleRenderSystem : public IRenderSystem
    {
    public:

        enum class PushConstantType
        {
            MAIN           = 0,
            POINTSHADOW    = 1,
            SPOTSHADOW     = 2,
            CASCADEDSHADOW = 3
        };

        struct ShadowFrameBufferAttachment 
        {
            VyImage     Image;
            VyImageView View;
        };

        struct ShadowPass 
        {
            U32                         Width;
            U32                         Height;
            VkFramebuffer               Framebuffer;
            ShadowFrameBufferAttachment ShadowMapImage;
            VkRenderPass                RenderPass;
            VkSampler                   ShadowMapSampler;
            VkDescriptorImageInfo       Descriptor;
        };

        struct CascadedShadowPassUBO
        {
            TArray<Mat4,  CASCADE_SHADOW_MAP_COUNT> ViewProjMatrices;
            TArray<float, CASCADE_SHADOW_MAP_COUNT> SplitDepths;
        };

        struct Cascade
        {
            VkFramebuffer Framebuffer;
            VyImageView   View;
        };

        struct CascadedShadowPass
        {
            VkRenderPass                              RenderPass;
            TArray<Cascade, CASCADE_SHADOW_MAP_COUNT> Cascades;
            CascadedShadowPassUBO                     UBO;
        };

        struct CascadedDepthMap
        {
            VyImage        Image;
            VyImageView    View;
            VySampler      Sampler;
        };


        struct ShadowPassUBO
        {
            Mat4 LightProjection;
        };

        struct PointShadowPass 
        {
            using FBArray = TArray<TArray<VkFramebuffer, 6>, MAX_POINT_LIGHTS>;

            U32                         Width;
            U32                         Height;
            FBArray                     Framebuffers;
            ShadowFrameBufferAttachment PointShadowMapImage;
            VkRenderPass                RenderPass;
            VkSampler                   PointShadowMapSampler;
            VkDescriptorImageInfo       Descriptor;
        };

        struct TextureArray 
        {
            U32                         Width;
            U32                         Height;
            ShadowFrameBufferAttachment CubeMapImage;
            VySampler                   CubeMapSampler;
        };

        struct PointShadowPassViewMatrixUBO
        {
            TArray<Mat4, 6> FaceViewMatrices;
        };


        struct SpotShadowPass
        {
            U32                                    Width;
            U32                                    Height;
            TArray<VkFramebuffer, MAX_SPOT_LIGHTS> Framebuffers;
            TArray<VyImageView,   MAX_SPOT_LIGHTS> ImageViews;
            ShadowFrameBufferAttachment            SpotShadowMapImage;
            VkRenderPass                           RenderPass;
            VySampler                              Sampler;
        };

        struct SpotShadowLightProjectionsUBO
        {
            TArray<Mat4, MAX_SPOT_LIGHTS> LightProjections;
        };


        SimpleRenderSystem(
            VkRenderPass                   renderPass, 
            TVector<VkDescriptorSetLayout> setLayouts,
            VyDescriptorPool&              descriptorPool
        );
        
        ~SimpleRenderSystem();

        SimpleRenderSystem(const SimpleRenderSystem&) = delete;
        SimpleRenderSystem& operator=(const SimpleRenderSystem&) = delete;

        void renderShadowPass(VyFrameInfo frameInfo, GlobalUBO& globalUBO);
        void renderCascadedShadowPass(VyFrameInfo frameInfo, GlobalUBO& globalUBO);
        void renderPointShadowPass(VyFrameInfo frameInfo, GlobalUBO& globalUBO);
        void renderSpotShadowPass(VyFrameInfo frameInfo, GlobalUBO& globalUBO);
        void renderMainPass(VyFrameInfo frameInfo);

        void renderGameObjects(VyFrameInfo frameInfo, VkPipelineLayout pipelineLayout, PushConstantType type, int setCount, bool bRenderMaterial = true);

    private:

        void createPipelineLayout(TVector<VkDescriptorSetLayout> setLayouts, VyDescriptorPool& descriptorPool);
    
        void createPipeline(VkRenderPass renderpass);

        void createMainPipeline(VkRenderPass renderpass);
        void createShadowPassPipeline();
        void createCascadedShadowPassPipeline();
        void createPointShadowPassPipeline();
        void createSpotShadowPassPipeline();

        void prepareShadowPassUBO();

        void prepareShadowPassRenderpass();
        void prepareShadowPassFramebuffer();
        void updateShadowPassBuffer(GlobalUBO& globalUBO);

        void prepareCascadeShadowPass();
        void updateCascades(GlobalUBO& ubo);

        void preparePointShadowCubeMaps();
        void preparePointShadowPassRenderPass();
        void preparePointShadowPassFramebuffers();

        void updateCubeFace(U32 faceIndex, VyFrameInfo frameInfo, GlobalUBO& ubo);

        void prepareSpotShadowMaps();
        void prepareSpotShadowPassRenderPass();
        void prepareSpotShadowPassFramebuffers();

        void updateSpotShadowMaps(U32 lightIndex, VyFrameInfo frameInfo, GlobalUBO& ubo);

        TVector<VkDescriptorSetLayout> m_MainSetLayouts;
        // TVector<VkDescriptorSetLayout> m_ShadowPassSetLayouts;
        TVector<VkDescriptorSetLayout> m_PointShadowPassSetLayouts;
        TVector<VkDescriptorSetLayout> m_SpotShadowPassSetLayouts;
        TVector<VkDescriptorSetLayout> m_CascadedShadowPassSetLayouts;

        // Main Pipeline variables
        Unique<VyPipeline> m_MainPipeline;
        // VkPipelineLayout   m_MainPipelineLayout;

        // VkDescriptorSet    m_ShadowMapDescriptorSet;

        CascadedDepthMap   m_CascadedDepthMapObject;
        VkDescriptorSet    m_CascadedShadowMapDescriptorSet;

        TextureArray       m_PointShadowCubeMaps{};
        VkDescriptorSet    m_PointShadowMapDescriptorSet;

        TextureArray       m_SpotShadowMaps{};
        VkDescriptorSet    m_SpotShadowMapDescriptorSet;

        SpotShadowLightProjectionsUBO m_SpotShadowLightProjectionsUBO{};
        Unique<VyBuffer>              m_SpotShadowLightProjectionsBuffer;
        // VkDescriptorSet               m_SpotShadowLightProjectionsDescriptorSet;

        // Directional Shadow variables
        Unique<VyPipeline> m_ShadowPassPipeline;
        // VkPipelineLayout   m_ShadowPassPipelineLayout;

        const VkFormat m_ShadowPassImageFormat{ VK_FORMAT_D16_UNORM };
        const U32      m_ShadowMapSize{ 4096 };

        ShadowPassUBO    m_ShadowPassUBO;
        Unique<VyBuffer> m_ShadowPassBuffer;
        VkDescriptorSet  m_ShadowPassDescriptorSet;

        ShadowPass       m_ShadowPass{};

        // Cascaded Shadow Map
        Unique<VyPipeline> m_CascadedShadowPassPipeline;
        // VkPipelineLayout   m_CascadedShadowPassPipelineLayout;

        const U32 m_CascadedShadowMapSize{4096};

        Unique<VyBuffer> m_CascadedShadowPassBuffer;
        VkDescriptorSet  m_CascadedShadowPassDescriptorSet;

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

        PointShadowPass                                  m_PointShadowPass{};
        TArray<TArray<VkImageView, 6>, MAX_POINT_LIGHTS> m_PointShadowCubeMapImageViews{};
        U32                                              m_PointLightCount = 0;
        int                                              m_FaceCount = 0;

        //Spot Shadow variables
        Unique<VyPipeline> m_SpotShadowPassPipeline;
        // VkPipelineLayout   m_SpotShadowPassPipelineLayout;

        const U32      m_SpotShadowMapSize{ 1024 };
        const VkFormat m_SpotShadowPassImageFormat{ VK_FORMAT_R32_SFLOAT };
        VkFormat       m_SpotShadowPassDepthFormat{ VK_FORMAT_UNDEFINED };

        ShadowPassUBO    m_SpotShadowPassUBO;
        Unique<VyBuffer> m_SpotShadowPassBuffer;
        VkDescriptorSet  m_SpotShadowPassDescriptorSet;

        SpotShadowPass   m_SpotShadowPass{};

        int m_SpotLightIndex = 0;
    };
}