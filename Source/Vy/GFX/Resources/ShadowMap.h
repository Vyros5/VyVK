#pragma once

#include <Vy/GFX/Backend/Device.h>
#include <Vy/GFX/Backend/Resources/Image.h>
#include <Vy/GFX/Backend/Resources/ImageView.h>
#include <Vy/GFX/Backend/Resources/Sampler.h>

namespace Vy
{
    /**
     * @brief Shadow map for depth-only rendering from light's perspective
     *
     * Creates a depth-only framebuffer and render pass for shadow mapping.
     * Supports directional, point, and spot lights.
     */
    class VyShadowMap
    {
    public:
        VyShadowMap(U32 width = 2048, U32 height = 2048);
        
        ~VyShadowMap();

        VyShadowMap(const VyShadowMap&)            = delete;
        VyShadowMap& operator=(const VyShadowMap&) = delete;

        VkRenderPass  renderPass()  const { return m_RenderPass; }
        VkFramebuffer framebuffer() const { return m_Framebuffer; }
        const VyImageView&  imageView()   { return m_DepthImageView; }
        const VySampler&    sampler()     { return m_Sampler; }

        U32 width()  const { return m_Width; }
        U32 height() const { return m_Height; }

        VkDescriptorImageInfo descriptorImageInfo() const
        {
            return VkDescriptorImageInfo{
                    .sampler     = m_Sampler.handle(),
                    .imageView   = m_DepthImageView.handle(),
                    .imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
            };
        }

        /**
         * @brief Begin shadow map render pass
         */
        void beginRenderPass(VkCommandBuffer commandBuffer);

        /**
         * @brief End shadow map render pass
         */
        void endRenderPass(VkCommandBuffer commandBuffer);

    private:
        void createDepthResources();
        void createRenderPass();
        void createFramebuffer();
        void createSampler();

        U32 m_Width;
        U32 m_Height;

        VyImage        m_DepthImage;
        VyImageView    m_DepthImageView;
        VySampler      m_Sampler;
        VkRenderPass   m_RenderPass  = VK_NULL_HANDLE;
        VkFramebuffer  m_Framebuffer = VK_NULL_HANDLE;
        VkFormat       m_DepthFormat = VK_FORMAT_D32_SFLOAT;
    };


    // ============================================================================================


    /**
     * @brief Cube shadow map for omnidirectional point light shadows
     *
     * Creates a depth cube map (6 faces) for point light shadow mapping.
     * Each face captures depth from the light's position in one direction.
     */
    class VyCubeShadowMap
    {
    public:
        VyCubeShadowMap(U32 size = 1024);

        ~VyCubeShadowMap();

        VyCubeShadowMap(const VyCubeShadowMap&)            = delete;
        VyCubeShadowMap& operator=(const VyCubeShadowMap&) = delete;

        VkRenderPass renderPass()    const { return m_RenderPass; }
        const VyImageView& cubeImageView()  { return m_CubeImageView; }
        const VySampler&   sampler()        { return m_Sampler; }

        U32 size() const { return m_Size; }

        /**
         * @brief Get framebuffer for a specific cube face
         * @param face Face index (0-5: +X, -X, +Y, -Y, +Z, -Z)
         */
        VkFramebuffer framebuffer(int face) const { return m_Framebuffers[face]; }

        /**
         * @brief Get view matrix for a specific cube face
         * @param lightPos Position of the point light
         * @param face Face index (0-5)
         */
        static Mat4 faceViewMatrix(const Vec3& lightPos, int face);

        /**
         * @brief Get projection matrix for cube shadow map
         * @param nearPlane Near plane distance
         * @param farPlane Far plane distance (light range)
         */
        static Mat4 projectionMatrix(float nearPlane, float farPlane);

        VkDescriptorImageInfo descriptorImageInfo() const
        {
            return VkDescriptorImageInfo{
                    .sampler     = m_Sampler,
                    .imageView   = m_CubeImageView,
                    .imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
            };
        }

        /**
         * @brief Transition all faces to attachment optimal before rendering
         * Call this ONCE before rendering all 6 faces
         */
        void transitionToAttachmentLayout(VkCommandBuffer commandBuffer);

        /**
         * @brief Transition all faces to shader read layout after rendering
         * Call this ONCE after rendering all 6 faces
         */
        void transitionToShaderReadLayout(VkCommandBuffer commandBuffer);

        /**
         * @brief Begin render pass for a specific cube face
         */
        void beginRenderPass(VkCommandBuffer commandBuffer, int face);

        /**
         * @brief End render pass
         */
        void endRenderPass(VkCommandBuffer commandBuffer);

    private:
        void createDepthResources();
        void createRenderPass();
        void createFramebuffers();
        void createSampler();

        U32 m_Size;

        VyImage        m_DepthImage;
        VyImageView    m_CubeImageView;     // View for the entire cube
        VyImageView    m_FaceImageViews[6]; // Views for each face
        VySampler      m_Sampler;

        VkRenderPass   m_RenderPass        = VK_NULL_HANDLE;
        VkFramebuffer  m_Framebuffers[6]   = {VK_NULL_HANDLE};
        VkFormat       m_DepthFormat       = VK_FORMAT_D32_SFLOAT;

        VkImage depthImage() const { return m_DepthImage; }
    };
}