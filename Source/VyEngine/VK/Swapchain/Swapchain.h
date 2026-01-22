#pragma once

#include <VyEngine/VK/Device/Device.h>

#include <VyEngine/VK/Image/Image.h>
#include <VyEngine/VK/Image/ImageView.h>
#include <VyEngine/VK/Image/Sampler.h>
#include <VyEngine/VK/Descriptors/Descriptors.h>

namespace Vy
{
    static constexpr int SHADOW_MAP_WIDTH = 1024;
    static constexpr int SHADOW_MAP_HEIGHT = 1024;

    static constexpr int NUM_CUBE_FACES = 6;
    static constexpr int MAPPINGS_ARRAY_LENGTH = 2;

    static constexpr VkFormat SHADOW_FB_COLOR_FORMAT = VK_FORMAT_R32_SFLOAT;
    static constexpr VkFormat DEFERRED_RESOURCES_FORMAT = VK_FORMAT_R32G32B32A32_SFLOAT;

    static constexpr Vec3 LIGHT_POSITION  = Vec3{ 1.0f, -4.0f, -4.0f };

    static constexpr Vec3 CAMERA_POSITION = Vec3{ 1.0f, -4.0f, -1.0f };

    static constexpr VkFilter DEFAULT_SHADOWMAP_FILTER = VK_FILTER_LINEAR;


    struct ShadowUbo {
        glm::mat4 projectionView[6];
        Vec3 lightPosition{ LIGHT_POSITION };
    };

    struct MappingsUbo {
        glm::mat4 projection{ 1.f };
        glm::mat4 view{ 1.f };
    };

    struct UVReflectionUbo {
        Vec3 viewPos;
        alignas(16) glm::mat4 projection{ 1.f };
        glm::mat4 view{ 1.f };
        glm::vec2 invResolution;
    };

    struct GBufferUbo {
        glm::mat4 projection{ 1.f };
        glm::mat4 view{ 1.f };
        Vec3 lightPosition{ LIGHT_POSITION };
    };

    struct CompositionUbo {
        Vec3 viewPos;
        alignas(16) glm::vec4 ambientLightColor{ 1.f, 1.f, 1.f, .15f }; //w is intensity
        Vec3 lightPosition{ LIGHT_POSITION };
        alignas(16) glm::vec4 lightColor{ .8f, 1.f, .2f, 1.f }; //w is light intensity
    };

    struct PostProcessingUbo {
        glm::vec2 invResolution;
    };

    struct FrameBufferAttachment
    {
        VyImage     Image;
        VyImageView View;
        VkFormat    Format;
        
        VkDescriptorImageInfo descriptorImageInfo(
            VkSampler     sampler     = VK_NULL_HANDLE, 
            VkImageLayout imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        );
    };

    struct ShadowMapDimension 
    {
        int width, height;
    };

    struct ImageSampler 
    {
        VySampler             Sampler;
        FrameBufferAttachment Attachment;
    };

    struct Samplers 
    {
        ImageSampler ShadowOmniMap;
        ImageSampler MappingsMap;
        ImageSampler UVReflectionMap;
        ImageSampler LightingMap;
    };

    struct Attachments 
    {
        FrameBufferAttachment Albedo;
        FrameBufferAttachment Normal;
        FrameBufferAttachment Depth;
        FrameBufferAttachment ShadowDepth;
        FrameBufferAttachment MappingsMapDepth;
        FrameBufferAttachment UVReflectionMapDepth;
    };




    /**
     * @brief The Swapchain class defines the creation and usage of a graphics swapchain. 
     * Swapchains define how images are rendered to screen. Generally, rendered objects are
     * drawn to images, which are contained within command buffers. Each frame, an image is drawn
     * to a command buffer which is then presented for rendering.
     * 
     * When an image takes longer than the given time for a frame, it runs the risk of overlapping
     * with the image being written in the following frame. This often results in screen tearing, 
     * where two images are overlain on top of each other. 
     * 
     * The swapchain is therefore responsible for buffering these images by writing to a new commandbuffer 
     * each frame. The new commandbuffer is then swapped out each frame depending on the method used. This
     * results in smooth image transitions and safer memory usage, as frames no longer compete to render the same data. 
     * 
     * Swapchains are essential for computer graphics processing. 
     */
    class VySwapchain 
    {
    public:

        /**
         * @brief Creates a new Swapchain object to buffer our images. 
         * 
         * @param windowExtent the window resolution. Images need to be scaled to an extent, 
         *                     this usually must follow the extents of the window. 
         */
        VySwapchain(VkExtent2D windowExtent);

        VySwapchain(VkExtent2D windowExtent, Shared<VySwapchain> previous);

        ~VySwapchain();

        
        VySwapchain(const VySwapchain&)            = delete;
        VySwapchain& operator=(const VySwapchain&) = delete;

        operator     VkSwapchainKHR()               { return m_Swapchain; }
		VY_NODISCARD VkSwapchainKHR handle()  const { return m_Swapchain; }

        VkImage       swapchainImage(int index)     { return m_SwapchainImages[index]; }
        VkImageView   swapchainImageView(int index) { return m_SwapchainImageViews[index]; }
        
        VkImage       depthImage(int index)         { return m_DepthImages[index]    .handle(); }
        VkImageView   depthImageView(int index)     { return m_DepthImageViews[index].handle(); }

        U32           width()                       { return m_SwapchainExtent.width;      }
        U32           height()                      { return m_SwapchainExtent.height;     }

        /**
         * @brief Returns the color format currently being used by the renderer.
         * 
         * @return VkFormat the format being used by the renderer. 
         */
        VkFormat swapchainColorFormat() const
        { 
            return m_SwapchainColorFormat; 
        }
        
        /**
         * @brief Returns the depth format currently being used by the renderer.
         * 
         * @return VkFormat the format being used by the renderer. 
         */
        VkFormat swapchainDepthFormat() const
        { 
            return m_SwapchainDepthFormat; 
        }

        /**
         * @brief Returns the image width and height that images are being rendered to. 
         * 
         * @return VkExtent2D a 2D representation of the height and width of the screen. 
         */
        VkExtent2D swapchainExtent() 
        { 
            return m_SwapchainExtent; 
        }
        
        /**
         * @brief Get a frame buffer object corresponding to the index.
         * 
         * The swapchain object stores multiple frame buffers equal to the number of images that can be held at once. 
         * 
         * @param index the index of the required framebuffer. 
         * 
         * @return The framebuffer at the index. 
         */
        VkFramebuffer frameBuffer(int index) const 
        { 
            return m_SwapchainFramebuffers[ index ]; 
        }

        /**
         * @brief Get the current render pass object. 
         * 
         * @return The raw vulkan render pass object.
         */
        VkRenderPass renderPass() const 
        { 
            return m_RenderPass; 
        }

        /**
         * @brief Get the number of images that can be active at once. 
         * 
         * @return size_t representing the number of possible active simultaneous images. 
         */
        size_t imageCount()
        { 
            return m_SwapchainImages.size(); 
        }

        /**
         * @brief Finds the depth format supported by this swapchain. 
         * 
         * @return The supported image format. 
         */
        VkFormat findDepthFormat();

        /**
         * @brief Loads in the next image to be written to in the Swapchain. 
         * 
         * @param pImageIndex The index of the next image. 
         * 
         * @return The result of acquiring the next image. 
         */
        VkResult acquireNextImage(U32* pImageIndex);

        /**
         * @brief Submits a command buffer for drawing.
         * 
         * @param pCmdBuffers An array of command buffers. 
         * @param pImageIndex The index of the image being drawn. 
         * 
         * @return The result of submitting the buffer. 
         */
        VkResult submitCommandBuffers(const VkCommandBuffer* pCmdBuffers, U32* pImageIndex);

        bool compareSwapFormats(const VySwapchain& swapchain) const 
        {
            return swapchain.m_SwapchainDepthFormat == m_SwapchainDepthFormat &&
                   swapchain.m_SwapchainColorFormat == m_SwapchainColorFormat;
        }
        
        float extentAspectRatio() 
        {
            return static_cast<float>(m_SwapchainExtent.width ) / 
                   static_cast<float>(m_SwapchainExtent.height);
        }

        void transitionSwapchainImageLayout(
            VkCommandBuffer cmdBuffer,
	        VkImageLayout   oldLayout, 
            VkImageLayout   newLayout, 
            U32             currentImageIndex
        );

    private:

        void init();

        void cleanup();

        /**
         * @brief Create the Vulkan swapchain object. 
         */
        void createSwapchain();

        /**
         * @brief Create image views for the images. 
         * 
         * An image view represents the subresources that an image must have to be valid. 
         * Must be created for each image being drawn.
         */
        void createImageViews();
        
        /**
         * @brief Creates a render pass. 
         * 
         * Render passes are objects that represent the state that rendering resources must be in to render to an image. 
         * All image rendering must occur within a render pass, while compute operations do not. 
         */
        void createRenderPass();
        
        /**
         * @brief Create a Depth Resources object. 
         * 
         * Depth resources represent how object depth is calculated. 
         * These resources are written to their own resource views. 
         */
        void createDepthResources();

        /**
         * @brief Creates the swapchain's frame buffers. 
         * 
         * A frame buffer represents all the data needed to be submitted to the GPU in a frame. 
         */
        void createFramebuffers();

        /**
         * @brief Initialises all thread synchronization objects. 
         * 
         * Vulkan is multi-threaded and submits images asynchronously. 
         * This can be dangerous when submitting images to the GPU since Vulkan does not guarantee safe execution. 
         * Therefore we must handle our own thread synchronization. 
         */
        void createSyncObjects();

        /**
         * @brief Specifies which color format we want images to be written to.
         * 
         * Accepts a set of formats and populates them with requisite data. 
         * 
         * @param availableFormats A array of possible formats. 
         * 
         * @return The chosen surface format.
         */
        VkSurfaceFormatKHR chooseSwapSurfaceFormat(const TVector<VkSurfaceFormatKHR>& availableFormats);

        /**
         * @brief Chooses the presentation mode that the window surface will use for images. 
         * 
         * @param availablePresentModes Array of possible presentation modes.
         *  
         * @return The chosen presentation mode. 
         */
        VkPresentModeKHR chooseSwapPresentMode(const TVector<VkPresentModeKHR>& availablePresentModes);

        /**
         * @brief Chooses an image size extent that's supported by the swapchain.
         * 
         * @param capabilities a reference to a struct containing our swapchain capabilities. 
         * 
         * @return The swapchain's supported extents. 
         */
        VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);


        U32 chooseImageCount(const VkSurfaceCapabilitiesKHR& capabilities) const;

        void destroySwapchain(VkSwapchainKHR swapchain);

        void printSwapchainRequestInfo(VkSwapchainCreateInfoKHR info);

    private:

        VkFormat   m_SwapchainColorFormat; // Format used by this swapchain's images.
        VkFormat   m_SwapchainDepthFormat; // Format used by this swapchain's depthbuffer.

        VkExtent2D m_SwapchainExtent; // Extent (size) of this swapchain's images.
        VkExtent2D m_WindowExtent;    // Extent (size) of the window this swapchain is rendering too.

        // Rendering
        VkRenderPass           m_RenderPass;            // Renderpass to be preformed on swapchain images.
        TVector<VkFramebuffer> m_SwapchainFramebuffers; // Framebuffer to each swapchain image.

        // Depth Image Resources
        TVector<VyImage>       m_DepthImages;           // Images that will be used to do depth tests.
        TVector<VyImageView>   m_DepthImageViews;       // Image views of the depth images.

        // Swapchain Image Resources
        TVector<VkImage>       m_SwapchainImages;          // All the images this swapchain is using for colors (rendering).
        TVector<VkImageView>   m_SwapchainImageViews; 

        // Swapchains
        VkSwapchainKHR         m_Swapchain{ VK_NULL_HANDLE }; // Swapchain object in vulkan.
        Shared<VySwapchain>    m_OldSwapchain;                // Previous swapchain (only exists if this is a recreated swapchain)

        // Synchronization Objects
        TVector<VkSemaphore>   m_ImageAvailableSemaphores;
        TVector<VkSemaphore>   m_RenderFinishedSemaphores;
        TVector<VkFence>       m_InFlightFences;
        TVector<VkFence>       m_ImagesInFlight;

        // Current frame in flight.
        size_t                 m_CurrentFrame{ 0 };

        // Present ID
        // VyPresentIdState       m_PresentIdState;

        bool m_UseMsaaSamples{ false };



    private:
        TVector<VkFramebuffer> m_ShadowFramebuffers;
        VkRenderPass           m_ShadowRenderPass;

        TVector<VkFramebuffer> m_MappingsFramebuffers;
        VkRenderPass           m_MappingsRenderPass;

        TVector<VkFramebuffer> m_UVReflectionFramebuffers;
        VkRenderPass           m_UVReflectionRenderPass;

        TVector<VkFramebuffer> m_LightingFramebuffers;
        VkRenderPass           m_LightingRenderPass;

        TVector<VkFramebuffer> m_PostProcessingFramebuffers;
        VkRenderPass           m_PostProcessingRenderPass;

        TVector<Samplers>             m_Samplers;
        TVector<Attachments>          m_Attachments;
        Unique<VyDescriptorSetLayout> m_GBufferSetLayout;
        Unique<VyDescriptorSetLayout> m_CompositionSetLayout;
        Unique<VyDescriptorSetLayout> m_ShadowSetLayout;
        Unique<VyDescriptorSetLayout> m_MappingsSetLayout;
        Unique<VyDescriptorSetLayout> m_UVReflectionSetLayout;
        Unique<VyDescriptorSetLayout> m_PostProcessingSetLayout;
        Unique<VyDescriptorPool>      m_GlobalPool;
        TVector<Unique<VyBuffer>>     m_GBufferUboBuffers;
        TVector<Unique<VyBuffer>>     m_CompositionBuffers;
        TVector<Unique<VyBuffer>>     m_ShadowBuffers;
        TVector<Unique<VyBuffer>>     m_MappingBuffers;
        TVector<Unique<VyBuffer>>     m_UVReflectionBuffers;
        TVector<Unique<VyBuffer>>     m_PostProcessingBuffers;
        TVector<VkDescriptorSet>      m_GBufferDescriptorSets;
        TVector<VkDescriptorSet>      m_CompositionSets;
        TVector<VkDescriptorSet>      m_ShadowSets;
        TVector<VkDescriptorSet>      m_MappingSets;
        TVector<VkDescriptorSet>      m_UVReflectionSets;
        TVector<VkDescriptorSet>      m_PostProcessingSets;

    public:
        VkFramebuffer shadowFrameBuffer(int index) { return m_ShadowFramebuffers[index]; }
        VkFramebuffer mappingsFrameBuffer(int index) { return m_MappingsFramebuffers[index]; }
        VkFramebuffer uvReflectionFrameBuffer(int index) { return m_UVReflectionFramebuffers[index]; }
        VkFramebuffer lightingFrameBuffer(int index) { return m_LightingFramebuffers[index]; }
        VkFramebuffer postProcessingFrameBuffer(int index) { return m_PostProcessingFramebuffers[index]; }

        VkRenderPass  shadowRenderPass() { return m_ShadowRenderPass; }
        VkRenderPass  mappingsRenderPass() { return m_MappingsRenderPass; }
        VkRenderPass  uvReflectionRenderPass() { return m_UVReflectionRenderPass; }
        VkRenderPass  lightingRenderPass() { return m_LightingRenderPass; }
        VkRenderPass  postProcessingRenderPass() { return m_PostProcessingRenderPass; }

        VkExtent2D shadowMapExtent() { return VkExtent2D{SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT}; }
        float shadowExtentAspectRatio() 
        {
            return static_cast<float>(shadowMapExtent().width) / static_cast<float>(shadowMapExtent().height);
        }
        VkDescriptorSetLayout shadowSetLayout() const { return m_ShadowSetLayout->handle(); };
        VkDescriptorSetLayout mappingsSetLayout() const { return m_MappingsSetLayout->handle(); };
        VkDescriptorSetLayout uvReflectionSetLayout() const { return m_UVReflectionSetLayout->handle(); };
        VkDescriptorSetLayout gBufferSetLayout() const { return m_GBufferSetLayout->handle(); };
        VkDescriptorSetLayout compositionSetLayout() const { return m_CompositionSetLayout->handle(); };
        VkDescriptorSetLayout postProcessingSetLayout() const { return m_PostProcessingSetLayout->handle(); };
        VkDescriptorSet currentShadowSet(int currentImageIndex) { return m_ShadowSets[currentImageIndex]; };
        VkDescriptorSet currentMappingsSet(int currentImageIndex) { return m_MappingSets[currentImageIndex]; };
        VkDescriptorSet currentUVReflectionSet(int currentImageIndex) { return m_UVReflectionSets[currentImageIndex]; };
        VkDescriptorSet currentGBufferSet(int currentImageIndex) { return m_GBufferDescriptorSets[currentImageIndex]; };
        VkDescriptorSet currentCompositionSet(int currentImageIndex) { return m_CompositionSets[currentImageIndex]; };
        VkDescriptorSet currentPostProcessingSet(int currentImageIndex) { return m_PostProcessingSets[currentImageIndex]; };
        void updateCurrentShadowUbo(void* data, int currentImageIndex);
        void updateCurrentMappingsUbo(void* data, int currentImageIndex);
        void updateCurrentUVReflectionUbo(void* data, int currentImageIndex);
        void updateCurrentGBufferUbo(void* data, int currentImageIndex);
        void updateCurrentCompositionUbo(void* data, int currentImageIndex);
        void updateCurrentPostProcessingUbo(void* data, int currentImageIndex);


    private:

        // void destroyAttachment(FrameBufferAttachment* pAttachment);
        // void destroySampler(ImageSampler* pSampler);
        void createDescriptorPool();
        void createUniformBuffers();

        void createAttachment(VkFormat format, VkImageUsageFlags usage, FrameBufferAttachment* pAttachment, VkExtent2D extent, VkImageViewType imageViewType = VK_IMAGE_VIEW_TYPE_2D, U32 arrayLayers = 1);
        void createSampler(VkFormat format, VkImageUsageFlags usage, ImageSampler* pSampler, VkExtent2D extent, VkImageViewType imageViewType = VK_IMAGE_VIEW_TYPE_2D, U32 arrayLayers = 1);
        
        void createShadowSampler();
        void createShadowRenderPass();
        void createShadowFramebuffers();
        void createMappingsSampler();
        void createMappingsRenderPass();
        void createMappingsFramebuffers();
        void createUVMapSampler();
        void createUVMapRenderPass();
        void createUVMapFramebuffers();
        void createDeferredResources();
        void createLightingRenderPass();
        void createLightingFramebuffers();
        void createPostProcessingRenderPass();
        void createPostProcessingFramebuffers();

    };
}