#pragma once

#include <VyEngine/VK/Device/Device.h>

#include <VyEngine/VK/Image/Image.h>
#include <VyEngine/VK/Image/ImageView.h>
#include <VyEngine/VK/Image/Sampler.h>
#include <VyEngine/GFX/Data/FrameInfo.h>

namespace Vy
{
    enum class AttachmentType
    {
        Color,
        Depth,
        DepthStencil,
        Resolve
    };

    struct AttachmentDescription
    {
        AttachmentType        Type{};
        VkFormat              Format{};
        VkSampleCountFlagBits Samples        = VK_SAMPLE_COUNT_1_BIT;
        VkAttachmentLoadOp    LoadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
        VkAttachmentStoreOp   StoreOp        = VK_ATTACHMENT_STORE_OP_STORE;
        VkAttachmentLoadOp    StencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        VkAttachmentStoreOp   StencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        VkImageLayout         InitialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
        VkImageLayout         FinalLayout    = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        VkClearValue          ClearValue{};
    };

    struct SubpassDescription
    {
        TVector  <U32> ColorAttachments;
        TOptional<U32> DepthStencilAttachment;
        TVector  <U32> InputAttachments;
        TVector  <U32> ResolveAttachments;
        TVector  <U32> PreserveAttachments;
    };

    class VyRenderPass
    {
    public:
        explicit VyRenderPass(TString debugName);
        ~VyRenderPass();

        void addAttachment(const AttachmentDescription& attachment);
        
        void addSubpass(const SubpassDescription& subpass);
        
        void addDependency(
            U32                  srcSubpass, 
            U32                  dstSubpass,
            VkPipelineStageFlags srcStageMask, 
            VkPipelineStageFlags dstStageMask,
            VkAccessFlags        srcAccessMask, 
            VkAccessFlags        dstAccessMask,
            VkDependencyFlags    dependencyFlags
        );

        void addDependency(const VkSubpassDependency& dependency);

        void build();

        void beginPass(VkCommandBuffer cmdBuffer, VkRenderPassBeginInfo beginInfo, VkExtent2D extent);
        void endPass(VkCommandBuffer cmdBuffer);

        VkRenderPass handle() const { return m_RenderPass; }
        U32 colorAttachmentCount();

        const TVector<AttachmentDescription>& attachmentDescriptions() const { return m_Attachments; }

    private:
        TString                        m_DebugName;
        TVector<AttachmentDescription> m_Attachments;
        TVector<SubpassDescription>    m_Subpasses;
        TVector<VkSubpassDependency>   m_Dependencies;
        TVector<VkClearValue>          m_AttachmentClearValues;
        VkRenderPass                   m_RenderPass = VK_NULL_HANDLE;

        void createRenderPass();
    };


class VyDeferredRenderer //: public IRenderer
{
public:
    explicit VyDeferredRenderer();// VyRenderer* renderer
    ~VyDeferredRenderer() = default;

    void Initialize();
    void Shutdown();
    void Render(VyFrameInfo& frameInfo);
    void Resize(uint32_t width, uint32_t height);
    // void RegisterGameObject(GameObject& gameObjectRef);

private:
	void RecordGBufferCommandBuffer(VyFrameInfo& frameInfo);
	void RecordLightingPassCommandBuffer(VyFrameInfo& frameInfo);
	void RecordCompositionPassCommandBuffer(VyFrameInfo& frameInfo);
	void SubmitRenderPasses(uint32_t frameIndex);

    void InvalidateGBufferPass();
    void CreateGBufferTextures();
    void CreateGBufferRenderPass();
    void CreateGBufferPipeline();
    void CreateGBufferFramebuffers();

    void InvalidateLightingPass();
    void CreateLightingTextures();
    void CreateLightingRenderPass();
    void CreateLightingPipeline();
    void CreateLightingFramebuffers();

    void InvalidateCompositionPass();
    void CreateCompositionRenderPass();
    void CreateCompositionPipeline();
    void CreateCompositionFramebuffers();

	void CreateCommandBuffers();
	void CreateSynchronizationPrimitives();
	void CreateShaders();
	void CreateMaterials();
	void CreateAttachmentTextures();

private:
    // VyRenderer *m_Renderer;

    /*
     * Per Frame Resources - Double/Triple Buffered
     */
    // std::vector<std::vector<std::shared_ptr<VyTexture2D>>> m_GBufferTextures{VySwapchain::MAX_FRAMES_IN_FLIGHT};
    // std::vector<std::shared_ptr<VyTexture2D>> m_LightingTextures{VySwapchain::MAX_FRAMES_IN_FLIGHT};

	std::vector<VkCommandBuffer> m_GBufferCommandBuffers;
	std::vector<VkCommandBuffer> m_LightingCommandBuffers;

	std::vector<VkSemaphore> m_GBufferCompleteSemaphores;
	std::vector<VkSemaphore> m_LightingCompleteSemaphores;
	std::vector<VkSemaphore> m_CompositionRenderCompleteSemaphores;

    // These framebuffers will be resized on creation or resize.
    // std::vector<std::unique_ptr<VyFramebuffer>> m_GBufferFramebuffers;
    // std::vector<std::unique_ptr<VyFramebuffer>> m_LightingFramebuffers;
    /*
     * Needs to be allocated based on the number swapchain images - NOT the number of frames in flight
     */
    // std::vector<std::unique_ptr<VyFramebuffer>> m_CompositionFramebuffers;

    /*
     * Single Use Resources
    */
    std::unique_ptr<VyRenderPass> m_GBufferPass;
    std::unique_ptr<VyRenderPass> m_LightingPass;
    std::unique_ptr<VyRenderPass> m_CompositionPass;

    // std::unique_ptr<VyGraphicsPipeline> m_GBufferPipeline;
    // std::unique_ptr<VyGraphicsPipeline> m_LightingPipeline;
    // std::unique_ptr<VyGraphicsPipeline> m_CompositionPipeline;

    // std::shared_ptr<VyShader> m_GBufferVertexShader;
    // std::shared_ptr<VyShader> m_GBufferFragmentShader;

    // std::shared_ptr<VyShader> m_FullScreenQuadVertexShader;
    // std::shared_ptr<VyShader> m_LightingFragmentShader;
    // std::shared_ptr<VyShader> m_CompositionFragmentShader;

    // std::shared_ptr<VyMaterialLayout> m_GBufferMaterialLayout;
    // std::shared_ptr<VyMaterial> m_GBufferBaseMaterial;

    // std::shared_ptr<VyMaterialLayout> m_LightingMaterialLayout;
    // std::shared_ptr<VyMaterial> m_LightingMaterial;

    // std::shared_ptr<VyMaterialLayout> m_CompositionMaterialLayout;
    // std::shared_ptr<VyMaterial> m_CompositionMaterial;

	// std::shared_ptr<VyTexture2D> m_SimpleTextureA;
};
}