#pragma once

#include <Vy/GFX/Backend/Device.h>
#include <Vy/GFX/Backend/Resources/Framebuffer.h>

namespace Vy
{
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
         *      this usually must follow the extents of the window. 
         */
        VySwapchain(VkExtent2D windowExtent);

        VySwapchain(VkExtent2D windowExtent, Shared<VySwapchain> previous);

        ~VySwapchain();

        
        VySwapchain(const VySwapchain&)            = delete;
        VySwapchain& operator=(const VySwapchain&) = delete;

        VkImageView   imageView(int index)   { return m_SwapchainImageViews[index]; }
        U32           width()                { return m_SwapchainExtent.width;      }
        U32           height()               { return m_SwapchainExtent.height;     }
        
        
        /**
         * @brief Returns the format currently being used by the renderer.
         * 
         * @return VkFormat the format being used by the renderer. 
         */
        VkFormat swapchainImageFormat() { return m_SwapchainImageFormat; }
        
        /**
         * @brief Returns the image width and height that images are being rendered to. 
         * 
         * @return VkExtent2D a 2D representation of the height and width of the screen. 
         */
        VkExtent2D swapchainExtent() { return m_SwapchainExtent; }
        
        /**
         * @brief Get a frame buffer object corresponding to the index.
         * 
         * The swapchain object stores multiple frame buffers equal to the number of images that can be held at once. 
         * 
         * @param index the index of the required framebuffer. 
         * 
         * @return VkFramebuffer the framebuffer at the index. 
         */
        const VyFramebuffer& frameBuffer(int index) const { return m_SwapchainFramebuffers[ index ]; }

        // VkFramebuffer frameBuffer(int index) { return m_SwapchainFramebuffers[ index ]; }
        
        /**
         * @brief Get the current render pass object. 
         * 
         * @return The raw vulkan render pass object.
         */
        VyRenderPass& renderPass() const { return *m_RenderPass; }

        // VkRenderPass renderPass() { return m_RenderPass; }

        /**
         * @brief Get the number of images that can be active at once. 
         * 
         * @return size_t representing the number of possible active simultaneous images. 
         */
        size_t imageCount() { return m_SwapchainImages.size(); }

        /**
         * @brief Finds the depth format supported by this swapchain. 
         * 
         * @return The supported image format. 
         */
        VkFormat findDepthFormat();

        /**
         * @brief Loads in the next image to be written to in the Swapchain. 
         * 
         * @param imageIndex The index of the next image. 
         * 
         * @return The result of acquiring the next image. 
         */
        VkResult acquireNextImage(U32* imageIndex);

        /**
         * @brief Submits a command buffer for drawing.
         * 
         * @param buffers    An array of command buffers. 
         * @param imageIndex The index of the image being drawn. 
         * 
         * @return The result of submitting the buffer. 
         */
        VkResult submitCommandBuffers(const VkCommandBuffer* cmdBuffers, U32* imageIndex);


        bool compareSwapFormats(const VySwapchain& swapchain) const 
        {
            return swapchain.m_SwapchainDepthFormat == m_SwapchainDepthFormat &&
                   swapchain.m_SwapchainImageFormat == m_SwapchainImageFormat;
        }

        float extentAspectRatio() 
        {
            return static_cast<float>(m_SwapchainExtent.width ) / 
                   static_cast<float>(m_SwapchainExtent.height);
        }

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

    private:
    
        VkFormat   m_SwapchainImageFormat; // Format used by this swapchain's images.
        VkFormat   m_SwapchainDepthFormat; // Format used by this swapchain's depthbuffer.

        VkExtent2D m_SwapchainExtent; // Extent (size) of this swapchain's images.
        VkExtent2D m_WindowExtent;    // Extent (size) of the window this swapchain is rendering too.

        // TVector<VkFramebuffer>  m_SwapchainFramebuffers; // Framebuffer to each swapchain image.
        // VkRenderPass            m_RenderPass;            // Renderpass to be preformed on swapchain images.
        
        TVector<VkImage>        m_DepthImages;           // Images that will be used to do depth tests.
        TVector<VmaAllocation>  m_DepthImageAllocations; // Memory allocations of the depth images.
        TVector<VkImageView>    m_DepthImageViews;       // Image views of the depth images.

        TVector<VkImage>        m_SwapchainImages;     // All the images this swapchain is using for colors (rendering).
        TVector<VkImageView>    m_SwapchainImageViews; // All the image views for each image, which are descriptors for the images (stuff like if 2d or 3d, how many layers, ect.)

        VkSwapchainKHR          m_Swapchain{ VK_NULL_HANDLE }; // Swapchain object in vulkan.
        Shared<VySwapchain>     m_OldSwapchain;                // Old swapchain that existed before this one (only exists if this is a recreated swapchain)

        // Synchronization Objects
        TVector<VkSemaphore> m_ImageAvailableSemaphores;
        TVector<VkSemaphore> m_RenderFinishedSemaphores;
        TVector<VkFence>     m_InFlightFences;
        TVector<VkFence>     m_ImagesInFlight;

        size_t               m_CurrentFrame = 0;

        Unique<VyRenderPass>   m_RenderPass{};
        TVector<VyFramebuffer> m_SwapchainFramebuffers;

        VyPresentIdState m_PresentIdState;
    };
}


namespace Vy
{

}