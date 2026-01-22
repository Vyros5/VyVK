#include <VyEngine/VK/Swapchain/Swapchain.h>

#include <VyEngine/VK/Context.h>
#include <VyLib/Common/AnsiColor.h>
#include <iostream>

#define USE_IMMEDIATE_PRESENT_MODE 0

namespace Vy
{
    /**
     * @class VySwapchain
     * 
     * @brief Manages Vulkan swapchain, image views, framebuffers, render pass,
     * depth resources, and synchronization objects.
     *
     * The SwapChain class encapsulates all logic for creating and managing the
     * Vulkan swapchain and its associated resources. It handles:
     *   - Swapchain creation and destruction
     *   - Image view creation for swapchain images
     *   - Render pass setup
     *   - Depth buffer resources
     *   - Framebuffer creation
     *   - Synchronization primitives (semaphores, fences)
     *   - Image acquisition and presentation
     *   - Frame synchronization for multiple frames in flight
     *
     * Usage:
     *   - Construct with a valid Device and window extent
     *   - Call acquireNextImage() before rendering each frame
     *   - Call submitCommandBuffers() to submit rendering and present the image
     *
     * @note This class is designed for onboarding and learning Vulkan best
     * practices. All resource management is automatic.
     */

// =========================================================================================================================
#pragma region [ Swapchain ]
// =========================================================================================================================

    VySwapchain::VySwapchain(VkExtent2D extent) : 
        m_WindowExtent{ extent }
    {
        init();
    }


    VySwapchain::VySwapchain(VkExtent2D extent, Shared<VySwapchain> previous) : 
        m_WindowExtent{ extent   }, 
        m_OldSwapchain{ previous }
    {
        init();

        // Clean up old swapchain since it's no longer being used.
        if (m_OldSwapchain != nullptr)
        {
            m_OldSwapchain.reset();
        }
    }


    VySwapchain::~VySwapchain() 
    {
        cleanup();
    }

    // ---------------------------------------------------------------------------------------------------------------------

    void VySwapchain::init() 
    {
        createSwapchain();
        createImageViews();
        createRenderPass();
        createDepthResources();
        createFramebuffers();
        // createSyncObjects();

        createShadowSampler();
        createShadowRenderPass();
        createShadowFramebuffers();
        createMappingsSampler();
        createMappingsRenderPass();
        createMappingsFramebuffers();
        createUVMapSampler();
        createUVMapRenderPass();
        createUVMapFramebuffers();
        createDeferredResources();
        createLightingRenderPass();
        createLightingFramebuffers();
        createPostProcessingRenderPass();
        createPostProcessingFramebuffers();
        createSyncObjects();
        createDescriptorPool();
        createUniformBuffers();
    }

#pragma endregion Swapchain


// =========================================================================================================================
#pragma region [ Acquisition ]
// =========================================================================================================================

    VkResult VySwapchain::acquireNextImage(U32* pImageIndex) 
    {
        // Wait for the fence of the current frame to have completed (Waits until command buffer has completed).
        vkWaitForFences(VyContext::device(), 1, &m_InFlightFences[ m_CurrentFrame ], VK_TRUE, UINT64_MAX);

        // Get next image to draw to in the swapchain.
        VkResult result = vkAcquireNextImageKHR(
            VyContext::device(),                          // Device swapchain is on.
            m_Swapchain,                                  // Swapchain being used.
            UINT64_MAX,                                   // Use max so no timeout.
            m_ImageAvailableSemaphores[ m_CurrentFrame ], // Semaphore to be triggered when image is ready for rendering (Must not be a signaled semaphore).
            VK_NULL_HANDLE,                               // No fences in use for this.
            pImageIndex                                   // Gets set to image index to use.
        );

        // if ((result == VK_SUCCESS          || 
        //      result == VK_SUBOPTIMAL_KHR)  && 
        //      m_ImagesInFlight[ *pImageIndex ] != VK_NULL_HANDLE)
        // {
        //     vkWaitForFences(VyContext::device(), 1, &m_ImagesInFlight[ *pImageIndex ], VK_TRUE, UINT64_MAX);
        // }

        return result;
    }

#pragma endregion 


// =========================================================================================================================
#pragma region [ Submission ]
// =========================================================================================================================

    VkResult VySwapchain::submitCommandBuffers(const VkCommandBuffer* pCmdBuffers, U32* pImageIndex) 
    {
        // Check if a previous frame is using this image (i.e. there is a fence to wait on).
        if (m_ImagesInFlight[ *pImageIndex ] != VK_NULL_HANDLE)
        {
            // Wait for all fences in the array to be signaled.
            vkWaitForFences(VyContext::device(), 1, &m_ImagesInFlight[ *pImageIndex ], VK_TRUE, UINT64_MAX);
        }

        // Mark the image as now being in use by this frame.
        m_ImagesInFlight[ *pImageIndex ] = m_InFlightFences[ m_CurrentFrame ];

        // -----------------------------------------------------------------------------------------------------------------

        // The RenderFinishedSemaphores for the current frame to be submitted as the command buffer complete signal semaphore.
        VkSemaphore signalSemaphores[]        = { m_RenderFinishedSemaphores[ *pImageIndex ] };

        // Specify the semaphore to wait on before execution begins and in which stage of the pipeline to wait.
        VkSemaphore          waitSemaphores[] = { m_ImageAvailableSemaphores[ m_CurrentFrame ] };
        VkPipelineStageFlags waitStages    [] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        
        // -----------------------------------------------------------------------------------------------------------------
        // [ SUBMIT ]

        VkSubmitInfo submitInfo{ VKInit::submitInfo() };
        {
            // Semaphore(s) to wait before the command buffers for this batch begin execution.
            submitInfo.pWaitSemaphores      = waitSemaphores;
            submitInfo.waitSemaphoreCount   = 1;

            // Pipeline stages at which each corresponding semaphore wait will occur.
            submitInfo.pWaitDstStageMask    = waitStages;
            
            // Command buffers to submit for execution.
            submitInfo.pCommandBuffers      = pCmdBuffers;
            submitInfo.commandBufferCount   = 1;
            
            // Semaphore(s) to signal once execution of the command buffer has completed.
            submitInfo.pSignalSemaphores    = signalSemaphores;
            submitInfo.signalSemaphoreCount = 1;
        }

        // Restore fences from signaled to unsignaled state.
        vkResetFences(VyContext::device(), 1, &m_InFlightFences[ m_CurrentFrame ]);
        
        // Submit command buffer to the graphics queue.
        // Pass in a fence to signal when the command buffer being submitted has finished executing.
		VK_CHECK_SUCCESS(vkQueueSubmit(
                VyContext::device().graphicsQueue(), 
                1, 
                &submitInfo, 
                m_InFlightFences[ m_CurrentFrame ]
            ),
            "Failed to submit queue!"
        );

        // -----------------------------------------------------------------------------------------------------------------
        // [ PRESENT ] -- Wait specifically on the render-finished semaphore for this image.

        // Set this swapchain as the swapchain to use for presentation.
        VkSwapchainKHR swapchains[] = { m_Swapchain };
        
        VkPresentInfoKHR presentInfo{ VKInit::presentInfoKHR() };
        {
            // Submit the signalSemaphores from the graphics queue completion as the wait, this is so
            // it will wait to present onto the screen until the command buffer has finished executing on the GPU.
            
            // Wait semaphores being used.
            presentInfo.pWaitSemaphores    = signalSemaphores;
            presentInfo.waitSemaphoreCount = 1;

            // Swapchains to present images to.
            presentInfo.pSwapchains        = swapchains; 
            presentInfo.swapchainCount     = 1;
            
            // Index of the image to present.
            presentInfo.pImageIndices      = pImageIndex;
        }

        // Send image to be presented to the display.
        auto result = vkQueuePresentKHR(VyContext::device().presentQueue(), &presentInfo);

        // Advance to the next frame. (frame in flight)
        m_CurrentFrame = (m_CurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

        return result;
    }

#pragma endregion Submission


// =========================================================================================================================
#pragma region [ Creation ]
// =========================================================================================================================

    void VySwapchain::createSwapchain() 
    {
        // Get the swapchain support details of the current device.
        SwapchainSupportDetails swapchainSupport = VyContext::device().querySwapchainSupport();

        // Get the format to use for this swapchain's surface.
        VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapchainSupport.Formats);

        // Get present mode to use for this swapchain.
        VkPresentModeKHR presentMode = chooseSwapPresentMode(swapchainSupport.PresentModes);

        // Gets the extent (size) to use for this swapchain.
        VkExtent2D extent = chooseSwapExtent(swapchainSupport.Capabilities);

        // Get the minimum image count to use.
		U32 imageCount = chooseImageCount(swapchainSupport.Capabilities);


        // [ Create Info ]
        VkSwapchainCreateInfoKHR createInfo{ VKInit::swapchainCreateInfoKHR() };
        {
            // Surface onto which the swapchain will present images. 
            createInfo.surface          = VyContext::device().surface();
            
            // The minimum number of presentable images that the application needs.
            createInfo.minImageCount    = imageCount;

            // The format the swapchain image(s) will be created with.
            createInfo.imageFormat      = surfaceFormat.format;

            // The way the swapchain interprets image data.
            createInfo.imageColorSpace  = surfaceFormat.colorSpace;

            // The size (in pixels) of the swapchain image(s).
            createInfo.imageExtent      = extent;

            // The number of views in a multiview/stereo surface. For non-stereoscopic-3D applications, this value is 1.
            createInfo.imageArrayLayers = 1;

            // The intended usage of the (acquired) swapchain images.
            // VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT - Image will be used to render directly to (so as a color attachment).
            // VK_IMAGE_USAGE_TRANSFER_DST_BIT     - Needed in order to copy the ray traced output to a swapchain image.
            createInfo.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; // Color (For Rendering)
            
            // Find submission queues of the device and get their indices.
            QueueFamilyIndices indices = VyContext::device().findQueueFamilies();
		    VY_ASSERT(indices.hasValidQueueSupport(), "Queue family indices are not complete.");
		
            TArray<U32, 2> queueFamilyIndices{ 
                indices.GraphicsFamily.value(), 
                indices.PresentFamily .value() 
            };

            // Check if the two queues are not the same (i.e. if the device does not use one queue for both).
            if (indices.GraphicsFamily != indices.PresentFamily) 
            {
                // Set sharing mode to concurrent so multiple queue families can access this swapchain.
                createInfo.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;

                // Set number of queue families accessing this and their indices. (queues to share images between)
                createInfo.queueFamilyIndexCount = static_cast<U32>(queueFamilyIndices.size());
                createInfo.pQueueFamilyIndices   = queueFamilyIndices.data();
            } 
            else 
            {
                // Set sharing mode to exclusive since it will only be submitted to one queue familiy. 
                createInfo.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;

                // Not required with mode exclusive, so just set to defualts.
                createInfo.queueFamilyIndexCount = 0;      
                createInfo.pQueueFamilyIndices   = nullptr;
            }

            // Ensure we use a supported transform; fall back to identity if needed.
            const VkSurfaceTransformFlagsKHR supportedTransforms = swapchainSupport.Capabilities.supportedTransforms;
            VkSurfaceTransformFlagBitsKHR    preTransform        = swapchainSupport.Capabilities.currentTransform;
            
            if (!(supportedTransforms & preTransform))
            {
                preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
            }

            // Set preTransform to be applied before presenting to the device's currentTransform,
            // so it will always be transformed to what the device's surface requires (like if it was a tablet and the screen rotated).
            createInfo.preTransform = preTransform;

            // Pick a composite alpha the surface supports, prefer opaque.
            VkCompositeAlphaFlagsKHR    supportedAlpha = swapchainSupport.Capabilities.supportedCompositeAlpha;
            VkCompositeAlphaFlagBitsKHR compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

            if (!(supportedAlpha & compositeAlpha))
            {
                const VkCompositeAlphaFlagBitsKHR candidates[] = {
                    VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
                    VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
                    VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR
                };
                
                for (auto candidate : candidates)
                {
                    if (supportedAlpha & candidate)
                    {
                        compositeAlpha = candidate;

                        break;
                    }
                }
            }

            // Set compositeAlpha to opaque, which means the compositing alpha is always 
            // treated as 1.0 (i.e. not using alpha currently).
            createInfo.compositeAlpha   = compositeAlpha;

            // Set present mode.
            createInfo.presentMode      = presentMode;

            // Dont render to non-visable pixels (like if covered by another window).
            createInfo.clipped          = VK_TRUE;

            // Dont provide old swapchain if it doesnt exist, 
            // else provide it so it can resuse memory when possible.
            createInfo.oldSwapchain     = m_OldSwapchain == nullptr ? VK_NULL_HANDLE : m_OldSwapchain->m_Swapchain;
        }

        printSwapchainRequestInfo(createInfo);

        // Create swapchain.
		VK_CHECK_SUCCESS(vkCreateSwapchainKHR(VyContext::device(), &createInfo, nullptr, &m_Swapchain), 
            "Failed to create swapchain!");

        // We only specified a minimum number of images in the swapchain (chooseImageCount()), 
        // so the implementation is allowed to create a swapchain with more. 

        // That's why we'll first query the final number of images with vkGetSwapchainImagesKHR, 
        // then resize the container and finally call it again to retrieve the handles.

        U32 actualImageCount = 0;
        VK_CHECK_SUCCESS(vkGetSwapchainImagesKHR(VyContext::device(), m_Swapchain, &actualImageCount, nullptr), 
            "Failed to query swapchain images!");

        m_SwapchainImages.resize( actualImageCount );
        VK_CHECK_SUCCESS(vkGetSwapchainImagesKHR(VyContext::device(), m_Swapchain, &actualImageCount, m_SwapchainImages.data()), 
            "Failed to retrieve swapchain images!");

        // std::cout << "ImageCount: " << imageCount << ", ActualImageCount: " << actualImageCount << std::endl;

        // Set format and extent member varibles.
        m_SwapchainColorFormat = surfaceFormat.format;
        m_SwapchainExtent      = extent;

        m_Samplers   .resize( actualImageCount );
        m_Attachments.resize( actualImageCount );
    }

#pragma endregion Creation

    void VySwapchain::printSwapchainRequestInfo(VkSwapchainCreateInfoKHR createInfo)
    {
        std::stringstream ss;
        ss  << '\n'
            << "--------------------------------------------------------------------------"                      << '\n'
            << "[" << MAGENTA "Swapchain Create Request" RESET "] "                                              << '\n'
            << " - MinImageCount   : " << createInfo.minImageCount                                               << '\n'
            << " - ImageFormat     : " << STR_VK_FORMAT(createInfo.imageFormat)                                  << '\n'
            << " - ImageColorSpace : " << STR_VK_COLOR_SPACE_KHR(createInfo.imageColorSpace)                     << '\n'
            << " - Extent          : " << createInfo.imageExtent.width << " x " << createInfo.imageExtent.height << '\n'
            << " - PreTransform    : " << STR_VK_SURFACE_TRANSFORM_FLAG_BITS_KHR(createInfo.preTransform)        << '\n'
            << " - CompositeAlpha  : " << STR_VK_COMPOSITE_ALPHA_FLAG_BITS_KHR(createInfo.compositeAlpha)        << '\n'
            << " - PresentMode     : " << STR_VK_PRESENT_MODE_KHR(createInfo.presentMode)                        << '\n'
            << " - ImageUsage      : " << STR_VK_IMAGE_USAGE_FLAG_BITS(createInfo.imageUsage)                    << '\n'
            << "--------------------------------------------------------------------------"                      << '\n'
        ;

        std::cout << ss.str() << std::endl;
    }


// =========================================================================================================================
#pragma region [ Res. Creation ]
// =========================================================================================================================
    

    // =========================================================================================================================
    #pragma region [ Create: RenderPass ]
    // =========================================================================================================================

        void VySwapchain::createRenderPass() 
        {
           // [ Depth Attachment ]
            VkAttachmentDescription depthAttachment{};
            {
                // Find and set image format to use for this depth buffer.
                depthAttachment.format         = this->findDepthFormat(); 

                // One sample per pixel (more samples used for multisampling).
                depthAttachment.samples        = VK_SAMPLE_COUNT_1_BIT;

                // Tells the depthbuffer attachment to clear each time it is loaded.
                depthAttachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;

                // Specifies the contents within the depth buffer render area are not needed after rendering.
                depthAttachment.storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE;

                // Specifies that the previous contents within the stencil need not be preserved and will be undefined.
                depthAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;

                // Specifies the contents within the stencil render area are not needed after rendering.
                depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

                // Inital image contents does not matter since we are clearing them on load.
                depthAttachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;

                // Set so the layout after rendering allows read and write access as a depth/stencil attachment.
                depthAttachment.finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            }

            // Defines the attachment index and the layout while rendering for the subpass (given to subpass below).
            VkAttachmentReference depthAttachmentRef{};
            {
                depthAttachmentRef.attachment  = 1;                                                // Index of this attachment in the swapchain.
                depthAttachmentRef.layout      = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL; // What this attachment is laid out to support.
            }
            
           // [ Color Attachment ]
            VkAttachmentDescription colorAttachment{};
            {
                // Set image format to match what is already being used by the swapchain.
                colorAttachment.format         = this->swapchainColorFormat(); 

                // // One sample per pixel (more samples used for multisampling).
                colorAttachment.samples        = VK_SAMPLE_COUNT_1_BIT;

                // VK_ATTACHMENT_LOAD_OP_LOAD:      Preserve the existing contents of the attachment.
                // VK_ATTACHMENT_LOAD_OP_CLEAR:     Clear the values to a constant at the start.
                // VK_ATTACHMENT_LOAD_OP_DONT_CARE: Existing contents are undefined.

                // VK_ATTACHMENT_STORE_OP_STORE:     Rendered contents will be stored in memory and can be read later.
                // VK_ATTACHMENT_STORE_OP_DONT_CARE: Contents of the framebuffer will be undefined after the rendering operation.

                // Tells the colorbuffer attachment to clear each time it is loaded.
                colorAttachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR; 

                // Specifies that the contents generated during the `render pass` and within the `render area` are written to memory.
                colorAttachment.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;

                // Specifies that the previous contents within the `stencil` don't need to be preserved and will be undefined.
                colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

                // Specifies that the contents within the `stencil render area` are not needed after rendering.
                colorAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;

                // Inital image contents does not matter since we are clearing them on load.
                // initialLayout specifies which layout the image will have before the render pass begins.
                // VK_IMAGE_LAYOUT_UNDEFINED for initialLayout means that we don't care what previous layout the image was in.
                // However, this means that the contents of the image are not guaranteed to be preserved, 
                // but that doesn't matter since we're going to clear it anyway.
                colorAttachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED; 

                // Layout used for presenting to the screen.
                // The layout to automatically transition to when the render pass finishes.
                // Uses VK_IMAGE_LAYOUT_PRESENT_SRC_KHR so the image will be ready for presentation using the swapchain after rendering.
                colorAttachment.finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
            }

            // Defines the attachment index and the layout while rendering for the subpass (given to subpass below).
            VkAttachmentReference colorAttachmentRef{};
            {
                colorAttachmentRef.attachment  = 0;                                        // Index of this attachment in the swapchain.
                colorAttachmentRef.layout      = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // What this attachment is laid out to support.
            }

            // //A subpass in Vulkan is a phase of rendering that can read from and write to 
            // certain framebuffer attachments (color, depth, and stencil buffers).
            VkSubpassDescription subpass{};
            {
                subpass.flags                   = 0;

                // Define that this subpass is for graphics output.
                subpass.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;

                subpass.inputAttachmentCount    = 0;
                subpass.pInputAttachments       = nullptr;
                
                // Number of color attachments used.
                subpass.colorAttachmentCount    = 1;

                // Refenece to the attachment index and layout for the color attachment.
                subpass.pColorAttachments       = &colorAttachmentRef;
                
                // Refenece to the attachment index and layout for the depth attachment.
                subpass.pDepthStencilAttachment = &depthAttachmentRef;
                subpass.pResolveAttachments     = nullptr;
                
                subpass.preserveAttachmentCount = 0;
                subpass.pPreserveAttachments    = nullptr;
            }

            // Declare a dependency for the subpass (forces thread sync between source and destination).
            VkSubpassDependency dependency{};
            {
                // [ Source ]
                {
                    // Sets the source of this dependency to be before the subpasses start.
                    dependency.srcSubpass    = VK_SUBPASS_EXTERNAL;
                    
                    // No specific memory access is being waited on from the source operation.
                    dependency.srcAccessMask = 0;
                    
                    // The render pass should wait until all color attachment writes and early fragment (depth) tests outside 
                    // of the render pass have been completed.
                    dependency.srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
                }

                // [ Destination ]
                {
                    // Sets the destination of this dependency to be the first subpass.
                    dependency.dstSubpass    = 0;
                    
                    // These access masks ensure that the operations described by dstStageMask (color and depth/stencil attachment writes) 
                    // will wait for the completion of the stages in the srcStageMask.
                    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

                    // Subpass 0's color attachment output and early fragment tests will only begin after the external stages 
                    // (color attachment output and early fragment tests) are complete.
                    dependency.dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
                }
            }

            // Create RenderPass.
            TArray<VkAttachmentDescription, 2> attachments = { 
                colorAttachment, // 0
                depthAttachment  // 1
            };

            VkRenderPassCreateInfo renderPassInfo{ VKInit::renderPassCreateInfo() };
            {
                // Attachments
                renderPassInfo.attachmentCount = static_cast<U32>(attachments.size());
                renderPassInfo.pAttachments    = attachments.data();

                // Subpasses
                renderPassInfo.subpassCount    = 1;
                renderPassInfo.pSubpasses      = &subpass;
                
                // Dependencies
                renderPassInfo.dependencyCount = 1;
                renderPassInfo.pDependencies   = &dependency;
            }

            VK_CHECK_SUCCESS(vkCreateRenderPass(VyContext::device(), &renderPassInfo, nullptr, &m_RenderPass), 
                "Failed to create swapchain render pass!"
            );

            VKDbg::setObjectName(m_RenderPass, "swapchain_renderpass");
        }

    #pragma endregion RenderPass


    // =========================================================================================================================
    #pragma region [ Create: Framebuffers ]
    // =========================================================================================================================

        void VySwapchain::createFramebuffers() 
        {
            // Resize framebuffers to match swapchain image count.
            m_SwapchainFramebuffers.resize( this->imageCount() );
            
            // Iterate over all the framebuffers.
            for (size_t i = 0; i < m_SwapchainFramebuffers.size(); i++) 
            {
                // Get color and depth attachment images for current index.
                const std::vector attachments = { 

                    m_SwapchainImageViews[ i ],         // 0
                    m_DepthImageViews    [ i ].handle() // 1
                };

                // Get the size of the swapchain.
                VkExtent2D swapchainExtent = this->swapchainExtent();

                VkFramebufferCreateInfo framebufferInfo{ VKInit::framebufferCreateInfo() };
                {
                    // Renderpass to use this framebuffer with.
                    framebufferInfo.renderPass      = m_RenderPass;
                    
                    // Attachments
                    framebufferInfo.pAttachments    = attachments.data();
                    framebufferInfo.attachmentCount = static_cast<U32>( attachments.size() );

                    // Set size to match swapchain size.
                    framebufferInfo.width           = swapchainExtent.width;
                    framebufferInfo.height          = swapchainExtent.height;

                    // Only one layer.
                    framebufferInfo.layers          = 1;

                    // No Flags
                    framebufferInfo.flags           = 0;
                }

                VK_CHECK_SUCCESS(vkCreateFramebuffer(VyContext::device(), &framebufferInfo, nullptr, &m_SwapchainFramebuffers[ i ]), 
                    "Failed to create swapchain framebuffer!"
                );

                TString name = std::format("swapchain_framebuffer_{}", i);

                VKDbg::setObjectName(m_SwapchainFramebuffers[ i ], name.c_str());
            }
        }

    #pragma endregion Framebuffers


    // =========================================================================================================================
    #pragma region [ Create: Image Resources ]
    // =========================================================================================================================


        // MARK: Depth

        void VySwapchain::createDepthResources() 
        {
            // Get the format of the image being used in this swapchain's renderpass's depth attachment.
            VkFormat depthFormat   = this->findDepthFormat();
            m_SwapchainDepthFormat = depthFormat;

            // Get the size of the swapchain output.
            VkExtent2D swapchainExtent = this->swapchainExtent();

            // Resize all vectors of depth images/images data to equal the number of images the swapchain is using.
            m_DepthImages    .resize( this->imageCount() );
            m_DepthImageViews.resize( this->imageCount() );

            // Iterate over each depth image and create an image view for it.
            for (int i = 0; i < m_DepthImages.size(); i++) 
            {
                TString depthName = std::format("swapchain_depth_image_{}", i);

                // Depth Images
                auto builder = VyImage::Builder{};
                {
                    builder.setName     (depthName);
                    builder.setImageType(VK_IMAGE_TYPE_2D);
                    builder.setExtent   (swapchainExtent);
                    builder.setLevels   (1);
                    builder.setLayers   (1);
                    
                    // Set the format to match the format of the depth attachment on the renderpass.
                    builder.setFormat(depthFormat);
                    
                    // Set tiling settings to be optimal.
                    builder.setTiling(VK_IMAGE_TILING_OPTIMAL);
                    
                    // Undefined layout
                    builder.setLayout(VK_IMAGE_LAYOUT_UNDEFINED);
                    
                    // Set the image to be used as a depth/stencil attachment.
                    builder.setUsage(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);

                    builder.setSamples(VK_SAMPLE_COUNT_1_BIT);
                    
                    // Set so image can only be accessed by one command buffer queue family at a time.
                    builder.setSharing(VK_SHARING_MODE_EXCLUSIVE);
                    
                    builder.setMemoryUsage(VMA_MEMORY_USAGE_AUTO);
                }

                m_DepthImages[ i ] = builder.build();

                TString depthViewName = std::format("swapchain_depth_image_view_{}", i);

                // Depth ImageView Framebuffer Attachments.
                auto viewBuilder = VyImageView::Builder{};
                {
                    viewBuilder.setName    (depthViewName);
                    viewBuilder.setViewType(VK_IMAGE_VIEW_TYPE_2D);
                    viewBuilder.setFormat  (depthFormat);
                    viewBuilder.setAspect  (VK_IMAGE_ASPECT_DEPTH_BIT);
                    viewBuilder.setLayers  (0, 1);
                    viewBuilder.setLevels  (0, 1);
                }
                
                m_DepthImageViews[ i ] = viewBuilder.build( m_DepthImages[ i ] );
            }
        }


        // MARK: Views

        void VySwapchain::createImageViews() 
        {
            // Resize image views to fit a view for all images in SwapChainImages.
            m_SwapchainImageViews.resize( m_SwapchainImages.size() );

            // Iterate over every image.
            for (size_t i = 0; i < m_SwapchainImages.size(); i++) 
            {
                // Create info for an image view, which is basicly a descriptor for an image (Ex. 2D or 3D, layers, etc.)
                VkImageViewCreateInfo viewInfo{ VKInit::imageViewCreateInfo() };
                {
                    viewInfo.image    = m_SwapchainImages[ i ]; // The image that this view is referring to.
                    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;  // Set so that this image is 2D.
                    viewInfo.format   = m_SwapchainColorFormat; // Set the format to saved format found earlier.

                    // Specifiy that the color compontent is what is to be rendered (compared to the depth bit, etc.)
                    viewInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
                    viewInfo.subresourceRange.baseMipLevel   = 0; // Dont scale down the image by defualt.
                    viewInfo.subresourceRange.levelCount     = 1; // Only allow defualt scale level (no other mip levels).
                    viewInfo.subresourceRange.baseArrayLayer = 0; // Make so the base accessed level into the image is 0.
                    viewInfo.subresourceRange.layerCount     = 1; // Make so the image only has one level.
                }

                // Create Image View.
                VK_CHECK(vkCreateImageView(VyContext::device(), &viewInfo, nullptr, &m_SwapchainImageViews[ i ]));

                TString viewName = std::format("swapchain_image_view_{}", i);

                VKDbg::setObjectName(m_SwapchainImageViews[ i ], viewName.c_str());
            }
        }

    #pragma endregion Depth Resources


    // =========================================================================================================================
    #pragma region [ Create: Sync ]
    // =========================================================================================================================

        void VySwapchain::createSyncObjects() 
        {
            // per-frame semaphores/fences.
            m_ImageAvailableSemaphores.assign( MAX_FRAMES_IN_FLIGHT, VK_NULL_HANDLE );
            m_InFlightFences          .assign( MAX_FRAMES_IN_FLIGHT, VK_NULL_HANDLE );

            // render-finished semaphores: one per swapchain image (prevents reuse while present is in-flight).
            m_RenderFinishedSemaphores.assign( this->imageCount(),   VK_NULL_HANDLE );
            m_ImagesInFlight          .assign( this->imageCount(),   VK_NULL_HANDLE );

            VkSemaphoreCreateInfo semaphoreInfo{ VKInit::semaphoreCreateInfo() };

            // Specify a signaled fence so the first frame doesn't wait forever.
            VkFenceCreateInfo fenceInfo{ VKInit::fenceCreateInfo( VK_FENCE_CREATE_SIGNALED_BIT ) };

            // Create per-frame image-available semaphores and fences.
            for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
            {
                VK_CHECK(vkCreateSemaphore(VyContext::device(), &semaphoreInfo, nullptr, &m_ImageAvailableSemaphores[ i ]));
                VK_CHECK(vkCreateFence    (VyContext::device(), &fenceInfo,     nullptr, &m_InFlightFences          [ i ]));

                TString semaphoreName = std::format("image_available_semaphore_{}", i);
                TString fenceName     = std::format("in_flight_fence_{}",           i);

                VKDbg::setObjectName(m_ImageAvailableSemaphores[ i ], semaphoreName.c_str());
                VKDbg::setObjectName(m_InFlightFences          [ i ], fenceName    .c_str());
            }
            
            // Create per-image render-finished semaphore.
            for (size_t i = 0; i < this->imageCount(); i++)
            {
                VK_CHECK(vkCreateSemaphore(VyContext::device(), &semaphoreInfo, nullptr, &m_RenderFinishedSemaphores[ i ]));

                TString semaphoreName = std::format("render_finished_semaphore_{}", i);

                VKDbg::setObjectName(m_RenderFinishedSemaphores[ i ], semaphoreName.c_str());
            }
        }

    #pragma endregion Sync

#pragma endregion Res. Creation

    // ---------------------------------------------------------------------------------------------------------------------

    void VySwapchain::cleanup()
    {
        for (auto framebuffer : m_SwapchainFramebuffers) 
        {
            vkDestroyFramebuffer(VyContext::device(), framebuffer, nullptr);
        }

        m_SwapchainFramebuffers.clear();


        for (auto framebuffer : m_PostProcessingFramebuffers) {
            vkDestroyFramebuffer(VyContext::device(), framebuffer, nullptr);
        }

        for (auto framebuffer : m_LightingFramebuffers) {
            vkDestroyFramebuffer(VyContext::device(), framebuffer, nullptr);
        }

        for (auto framebuffer : m_MappingsFramebuffers) {
            vkDestroyFramebuffer(VyContext::device(), framebuffer, nullptr);
        }

        for (auto framebuffer : m_UVReflectionFramebuffers) {
            vkDestroyFramebuffer(VyContext::device(), framebuffer, nullptr);
        }

        for (auto framebuffer : m_ShadowFramebuffers) {
            vkDestroyFramebuffer(VyContext::device(), framebuffer, nullptr);
        }

        vkDestroyRenderPass(VyContext::device(), m_PostProcessingRenderPass, nullptr);
        vkDestroyRenderPass(VyContext::device(), m_LightingRenderPass, nullptr);
        vkDestroyRenderPass(VyContext::device(), m_UVReflectionRenderPass, nullptr);
        vkDestroyRenderPass(VyContext::device(), m_MappingsRenderPass, nullptr);
        vkDestroyRenderPass(VyContext::device(), m_ShadowRenderPass, nullptr);


        vkDestroyRenderPass(VyContext::device(), m_RenderPass, nullptr);

        // Cleanup Swapchain Image Views.
		for (auto imageView : m_SwapchainImageViews)
		{
            if (imageView != VK_NULL_HANDLE)
            {
                VyContext::destroy( imageView );
            }
		}

        m_SwapchainImageViews.clear();

        m_DepthImageViews.clear();
        m_DepthImages    .clear();

        // Cleanup Swapchain.
        if (m_Swapchain != VK_NULL_HANDLE) 
        {
            vkDestroySwapchainKHR(VyContext::device(), m_Swapchain, nullptr);
        
            m_Swapchain = VK_NULL_HANDLE;
        }

        // Cleanup Synchronization objects.
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) 
        {
            vkDestroySemaphore(VyContext::device(), m_ImageAvailableSemaphores[ i ], nullptr);
            vkDestroyFence    (VyContext::device(), m_InFlightFences          [ i ], nullptr);
        }
        
        for (size_t i = 0; i < this->imageCount(); i++)
        {
            vkDestroySemaphore(VyContext::device(), m_RenderFinishedSemaphores[ i ], nullptr);
        }

        m_SwapchainImages.clear();

        m_RenderFinishedSemaphores.clear();
        m_ImageAvailableSemaphores.clear();

        m_InFlightFences.clear();
        m_ImagesInFlight.clear();
    }

    // ---------------------------------------------------------------------------------------------------------------------

#pragma region [ Support ]

	void VySwapchain::destroySwapchain(VkSwapchainKHR swapchain)
	{
		if (swapchain)
		{
			vkDestroySwapchainKHR(VyContext::device(), swapchain, nullptr);
		}

        swapchain = nullptr;
	}

    // ---------------------------------------------------------------------------------------------------------------------

    VkSurfaceFormatKHR VySwapchain::chooseSwapSurfaceFormat(const TVector<VkSurfaceFormatKHR> &availableFormats) 
    {
        // Common & Desirable: sRGB 8-bit per channel (B8G8R8A8_SRGB or R8G8B8A8_SRGB).
        // This is usually the preferred format for standard monitors, providing correct
        // sRGB gamma correction for visually accurate output. B8G8R8A8 is more common
        // on Windows, R8G8B8A8 on other platforms, but both are widely supported.
        
        for (const auto& availableFormat : availableFormats) 
        {
            if (availableFormat.format     == VK_FORMAT_B8G8R8A8_SRGB &&
                availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) 
            {
                // VY_INFO_TAG("VySwapchain", "Selected {} with {}.", 
                //     STR_VK_FORMAT(availableFormat.format), 
                //     STR_VK_COLOR_SPACE_KHR(availableFormat.colorSpace)
                // );

                return availableFormat;
            }
        }

        for (const auto& availableFormat : availableFormats) 
        {
            if (availableFormat.format     == VK_FORMAT_R8G8B8A8_SRGB &&
                availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) 
            {
                // VY_INFO_TAG("VySwapchain", "Selected {} with {}.", 
                //     STR_VK_FORMAT(availableFormat.format), 
                //     STR_VK_COLOR_SPACE_KHR(availableFormat.colorSpace)
                // );
                
                return availableFormat;
            }
        }

        // Good Fallback: Linear 8-bit per channel (B8G8R8A8_UNORM or R8G8B8A8_UNORM).
        // If sRGB isn't available, UNORM is a common fallback. It doesn't apply sRGB
        // gamma correction automatically, so your rendering might appear too dark.
        // However, it's widely supported.
        for (const auto& availableFormat : availableFormats) 
        {
            if (availableFormat.format     == VK_FORMAT_B8G8R8A8_UNORM &&
                availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) 
            { 
                // Still prefer sRGB color space if possible
                // VY_INFO_TAG("VySwapchain", "Selected {} with {}.", 
                //     STR_VK_FORMAT(availableFormat.format), 
                //     STR_VK_COLOR_SPACE_KHR(availableFormat.colorSpace)
                // );
                
                return availableFormat;
            }
        }
        for (const auto& availableFormat : availableFormats) 
        {
            if (availableFormat.format     == VK_FORMAT_R8G8B8A8_UNORM &&
                availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) 
            { 
                // Still prefer sRGB color space if possible
                // VY_INFO_TAG("VySwapchain", "Selected {} with {}.", 
                //     STR_VK_FORMAT(availableFormat.format), 
                //     STR_VK_COLOR_SPACE_KHR(availableFormat.colorSpace)
                // );
                
                return availableFormat;
            }
        }

        // VY_INFO_TAG("VySwapchain", "Selected first format available (only one supported): {} with {}.", 
        //     STR_VK_FORMAT(availableFormats[0].format), 
        //     STR_VK_COLOR_SPACE_KHR(availableFormats[0].colorSpace)
        // );
        
        return availableFormats[0];
    }

    // ---------------------------------------------------------------------------------------------------------------------

    VkPresentModeKHR VySwapchain::chooseSwapPresentMode(const TVector<VkPresentModeKHR> &availablePresentModes) 
    {
        for (const auto& availablePresentMode : availablePresentModes) 
        {
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) 
            {
                // VY_INFO_TAG("VySwapchain", "Present mode: Mailbox");
                return availablePresentMode;
            }
        }

#ifdef USE_IMMEDIATE_PRESENT_MODE
		for (const auto& availablePresentMode : availablePresentModes) 
        {
			if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) 
            {
                // VY_INFO_TAG("VySwapchain", "Present mode: Immediate");
				return availablePresentMode;
			}
		}
#endif

        // VY_INFO_TAG("VySwapchain", "Present mode: V-Sync (FIFO)");
        return VK_PRESENT_MODE_FIFO_KHR;
    }

    // ---------------------------------------------------------------------------------------------------------------------

    VkExtent2D VySwapchain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) 
    {
        if (capabilities.currentExtent.width != std::numeric_limits<U32>::max()) 
        {
            return capabilities.currentExtent;
        } 
        else 
        {
            VkExtent2D actualExtent = m_WindowExtent;
            {
                actualExtent.width  = std::max(
                    capabilities.minImageExtent.width, 
                    std::min(
                        capabilities.maxImageExtent.width,  
                        actualExtent.width
                    )
                );
                actualExtent.height = std::max(
                    capabilities.minImageExtent.height, 
                    std::min(
                        capabilities.maxImageExtent.height, 
                        actualExtent.height
                    )
                );
            }

            return actualExtent;
        }
    }

    // ---------------------------------------------------------------------------------------------------------------------

	U32 VySwapchain::chooseImageCount(const VkSurfaceCapabilitiesKHR& capabilities) const
	{
        // Set the image count to use to be one more than the minimum.
		U32 imageCount = capabilities.minImageCount + 1;

         // If the max exists and is smaller than current imagecount, set image count to max image count.
		if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount)
		{
			imageCount = capabilities.maxImageCount;
		}

		return imageCount;
	}

    // ---------------------------------------------------------------------------------------------------------------------

    VkFormat VySwapchain::findDepthFormat() 
    {
        return VyContext::device().findSupportedFormat(
            {
                VK_FORMAT_D32_SFLOAT, 
                VK_FORMAT_D32_SFLOAT_S8_UINT, 
                VK_FORMAT_D24_UNORM_S8_UINT
            },
            VK_IMAGE_TILING_OPTIMAL,
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
        );
    }

    // ---------------------------------------------------------------------------------------------------------------------

    void VySwapchain::transitionSwapchainImageLayout(
        VkCommandBuffer cmdBuffer,
        VkImageLayout   oldLayout, 
        VkImageLayout   newLayout, 
        U32             currentImageIndex)
    {
        VkImageMemoryBarrier barrier{ VKInit::imageMemoryBarrier() };
        {
            // Image being accessed and modified as part of barrier.
            barrier.image = m_SwapchainImages[ currentImageIndex ];

            barrier.oldLayout            = oldLayout; // Layout to transition from
            barrier.newLayout            = newLayout; // Layout to transition to

            barrier.srcQueueFamilyIndex  = VK_QUEUE_FAMILY_IGNORED;  // Queue family to transition from
            barrier.dstQueueFamilyIndex  = VK_QUEUE_FAMILY_IGNORED;  // Queue family to transition to

            // Aspect of image being altered.
            barrier.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
            barrier.subresourceRange.baseMipLevel   = 0;  // First mip level to start alterations on
            barrier.subresourceRange.levelCount     = 1;  // Number of mip levels to alter starting from baseMipLevel
            barrier.subresourceRange.baseArrayLayer = 0;  // First layer to start alterations on
            barrier.subresourceRange.layerCount     = 1;  // Number of layers to alter stating from baseArrayLayer
        }

        VkPipelineStageFlags srcStage{};
        VkPipelineStageFlags dstStage{};

        // If transitioning from color attachment to shader readable...
        if (oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) 
        {
            barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            srcStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }
        // If transitioning from shader read only to present source...
        else if (oldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) 
        {
            barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
            barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;

            srcStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            dstStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        } else {
            VY_ERROR("Unsupported Layout Transition in Swapchain!");
            return;
        }

        vkCmdPipelineBarrier(cmdBuffer,
            srcStage, dstStage,  // Pipeline stages(match to src and dst AccessMasks)
            0,                   // Dependency flags
            0, nullptr,          // Memory Barrier count + data
            0, nullptr,          // Buffer Memory Barrier count + data
            1, &barrier          // Image Memory Barrier count + data
        );
    }


#pragma endregion Support


    void VySwapchain::createShadowSampler() 
    {
        VkExtent2D shadowMapExtent = this->shadowMapExtent();

        for (auto& samplers : m_Samplers) 
        {
            createSampler(
                SHADOW_FB_COLOR_FORMAT, 
                VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 
                &samplers.ShadowOmniMap, 
                shadowMapExtent, 
                VK_IMAGE_VIEW_TYPE_CUBE, 
                NUM_CUBE_FACES
            );
        }

        for (auto& attachments : m_Attachments) 
        {
            createAttachment(
                findDepthFormat(), 
                VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, 
                &attachments.ShadowDepth, 
                shadowMapExtent, 
                VK_IMAGE_VIEW_TYPE_CUBE, 
                NUM_CUBE_FACES
            );
        }
    }


    void VySwapchain::createShadowRenderPass() 
    {
        TArray<VkAttachmentDescription, 2> attachments{};
        {
            // Position attachment (shadow)
            attachments[0].format         = SHADOW_FB_COLOR_FORMAT;
            attachments[0].samples        = VK_SAMPLE_COUNT_1_BIT;
            attachments[0].loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
            attachments[0].storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
            attachments[0].stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachments[0].initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
            attachments[0].finalLayout    = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            attachments[0].flags          = 0;
            
            // Depth attachment (shadow)
            attachments[1].format         = findDepthFormat();
            attachments[1].samples        = VK_SAMPLE_COUNT_1_BIT;
            attachments[1].loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
            attachments[1].storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
            attachments[1].stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachments[1].initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
            attachments[1].finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            attachments[1].flags          = 0;
        }

        VkAttachmentReference colorReferences[1];
        VkAttachmentReference depthReference;
        {
            colorReferences[0] = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
            depthReference     = { 1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };
        }
        
        // One subpass
        TArray<VkSubpassDescription, 1> subpassDescriptions{};
        {
            subpassDescriptions[0].pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
            subpassDescriptions[0].colorAttachmentCount    = 1;
            subpassDescriptions[0].pColorAttachments       = colorReferences;
            subpassDescriptions[0].pDepthStencilAttachment = &depthReference;
        }

        // Subpass dependencies for layout transitions
        TArray<VkSubpassDependency, 2> dependencies;
        {
            dependencies[0].srcSubpass      = VK_SUBPASS_EXTERNAL;
            dependencies[0].dstSubpass      = 0;
            dependencies[0].srcStageMask    = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
            dependencies[0].dstStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
            dependencies[0].srcAccessMask   = 0;
            dependencies[0].dstAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
            
            dependencies[1].srcSubpass      = 0;
            dependencies[1].dstSubpass      = VK_SUBPASS_EXTERNAL;
            dependencies[1].srcStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
            dependencies[1].dstStageMask    = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            dependencies[1].srcAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            dependencies[1].dstAccessMask   = VK_ACCESS_SHADER_READ_BIT;
            dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
        }

        VkRenderPassCreateInfo renderPassInfo{ VKInit::renderPassCreateInfo() };
        {
            renderPassInfo.attachmentCount = static_cast<U32>(attachments.size());
            renderPassInfo.pAttachments    = attachments.data();
            renderPassInfo.subpassCount    = subpassDescriptions.size();
            renderPassInfo.pSubpasses      = subpassDescriptions.data();
            renderPassInfo.dependencyCount = dependencies.size();
            renderPassInfo.pDependencies   = dependencies.data();
        }

        U32 viewAndCorrelationMask = 0b00111111; //6 faces

        VkRenderPassMultiviewCreateInfo renderPassMultiviewInfo{ VKInit::renderPassMultiviewCreateInfo() };
        {
            renderPassMultiviewInfo.subpassCount         = 1;
            renderPassMultiviewInfo.pViewMasks           = &viewAndCorrelationMask;
            renderPassMultiviewInfo.correlationMaskCount = 1;
            renderPassMultiviewInfo.pCorrelationMasks    = &viewAndCorrelationMask;
        }

        renderPassInfo.pNext = &renderPassMultiviewInfo;

        VK_CHECK_SUCCESS(vkCreateRenderPass(VyContext::device(), &renderPassInfo, nullptr, &m_ShadowRenderPass), 
            "Failed to create shadow render pass!"
        );

        VKDbg::setObjectName(m_ShadowRenderPass, "shadow_renderpass");
    }


    void VySwapchain::createShadowFramebuffers() 
    {
        m_ShadowFramebuffers.resize( this->imageCount() );

        VkExtent2D shadowMapExtent = this->shadowMapExtent();

        for (size_t i = 0; i < this->imageCount(); i++) 
        {
            TArray<VkImageView, 2> attachments = { 
                m_Samplers   [ i ].ShadowOmniMap.Attachment.View.handle(), 
                m_Attachments[ i ].ShadowDepth.View.handle() 
            };

            VkFramebufferCreateInfo framebufferInfo{ VKInit::framebufferCreateInfo() };
            {
                framebufferInfo.renderPass      = m_ShadowRenderPass;
                framebufferInfo.attachmentCount = static_cast<U32>(attachments.size());
                framebufferInfo.pAttachments    = attachments.data();
                framebufferInfo.width           = shadowMapExtent.width;
                framebufferInfo.height          = shadowMapExtent.height;
                framebufferInfo.layers          = 1;
            }

            VK_CHECK_SUCCESS(vkCreateFramebuffer(VyContext::device(), &framebufferInfo, nullptr, &m_ShadowFramebuffers[ i ]), 
                "Failed to create shadow framebuffer!"
            );

            TString name = std::format("shadow_framebuffer_{}", i);
            VKDbg::setObjectName(m_ShadowFramebuffers[ i ], name.c_str());
        }
    }


    void VySwapchain::createMappingsSampler() 
    {
        VkExtent2D extent = this->swapchainExtent();

        for (auto& samplers : m_Samplers) 
        {
            createSampler(
                DEFERRED_RESOURCES_FORMAT, 
                VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 
                &samplers.MappingsMap, 
                extent, 
                VK_IMAGE_VIEW_TYPE_2D_ARRAY, 
                MAPPINGS_ARRAY_LENGTH
            );
        }

        for (auto& attachments : m_Attachments) 
        {
            createAttachment(
                findDepthFormat(), 
                VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, 
                &attachments.MappingsMapDepth, 
                extent, 
                VK_IMAGE_VIEW_TYPE_2D_ARRAY, 
                MAPPINGS_ARRAY_LENGTH
            );
        }
    }


    void VySwapchain::createMappingsRenderPass() 
    {
        TArray<VkAttachmentDescription, 2> attachments{};
        {
            // View space position % normals attachment
            attachments[0].format         = DEFERRED_RESOURCES_FORMAT;
            attachments[0].samples        = VK_SAMPLE_COUNT_1_BIT;
            attachments[0].loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
            attachments[0].storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
            attachments[0].stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachments[0].initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
            attachments[0].finalLayout    = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            attachments[0].flags          = 0;
            
            // Depth attachment (shadow)
            attachments[1].format         = findDepthFormat();
            attachments[1].samples        = VK_SAMPLE_COUNT_1_BIT;
            attachments[1].loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
            attachments[1].storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
            attachments[1].stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachments[1].initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
            attachments[1].finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            attachments[1].flags          = 0;
        }

        VkAttachmentReference colorReferences[1];
        VkAttachmentReference depthReference;
        {
            colorReferences[0] = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
            depthReference     = { 1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };
        }

        // One subpass
        TArray<VkSubpassDescription, 1> subpassDescriptions{};
        {
            subpassDescriptions[0].pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
            subpassDescriptions[0].colorAttachmentCount    = 1;
            subpassDescriptions[0].pColorAttachments       = colorReferences;
            subpassDescriptions[0].pDepthStencilAttachment = &depthReference;
        }

        // Subpass dependencies for layout transitions
        TArray<VkSubpassDependency, 2> dependencies;
        {
            dependencies[0].srcSubpass      = VK_SUBPASS_EXTERNAL;
            dependencies[0].dstSubpass      = 0;
            dependencies[0].srcStageMask    = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
            dependencies[0].dstStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
            dependencies[0].srcAccessMask   = 0;
            dependencies[0].dstAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
            
            dependencies[1].srcSubpass      = 0;
            dependencies[1].dstSubpass      = VK_SUBPASS_EXTERNAL;
            dependencies[1].srcStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
            dependencies[1].dstStageMask    = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            dependencies[1].srcAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            dependencies[1].dstAccessMask   = VK_ACCESS_SHADER_READ_BIT;
            dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
        }

        VkRenderPassCreateInfo renderPassInfo{ VKInit::renderPassCreateInfo() };
        {
            renderPassInfo.attachmentCount = static_cast<U32>(attachments.size());
            renderPassInfo.pAttachments    = attachments.data();
            renderPassInfo.subpassCount    = subpassDescriptions.size();
            renderPassInfo.pSubpasses      = subpassDescriptions.data();
            renderPassInfo.dependencyCount = dependencies.size();
            renderPassInfo.pDependencies   = dependencies.data();
        }

        U32 viewAndCorrelationMask = 0b00000011; //2 maps

        VkRenderPassMultiviewCreateInfo renderPassMultiviewInfo{ VKInit::renderPassMultiviewCreateInfo() };
        {
            renderPassMultiviewInfo.subpassCount         = 1;
            renderPassMultiviewInfo.pViewMasks           = &viewAndCorrelationMask;
            renderPassMultiviewInfo.correlationMaskCount = 1;
            renderPassMultiviewInfo.pCorrelationMasks    = &viewAndCorrelationMask;
        }

        renderPassInfo.pNext = &renderPassMultiviewInfo;

        VK_CHECK_SUCCESS(vkCreateRenderPass(VyContext::device(), &renderPassInfo, nullptr, &m_MappingsRenderPass), 
            "Failed to create mappings render pass!"
        );

        VKDbg::setObjectName(m_MappingsRenderPass, "mappings_renderpass");
    }

    
    void VySwapchain::createMappingsFramebuffers() 
    {
        m_MappingsFramebuffers.resize( this->imageCount() );
        
        VkExtent2D extent = this->swapchainExtent();

        for (size_t i = 0; i < this->imageCount(); i++) 
        {
            TArray<VkImageView, 2> attachments = { 
                m_Samplers   [ i ].MappingsMap.Attachment.View.handle(), 
                m_Attachments[ i ].MappingsMapDepth.View.handle() 
            };

            VkFramebufferCreateInfo framebufferInfo{ VKInit::framebufferCreateInfo() };
            {
                framebufferInfo.renderPass      = m_MappingsRenderPass;
                framebufferInfo.attachmentCount = static_cast<U32>(attachments.size());
                framebufferInfo.pAttachments    = attachments.data();
                framebufferInfo.width           = extent.width;
                framebufferInfo.height          = extent.height;
                framebufferInfo.layers          = 1;
            }

            VK_CHECK_SUCCESS(vkCreateFramebuffer(VyContext::device(), &framebufferInfo, nullptr, &m_MappingsFramebuffers[ i ]), 
                "Failed to create mappings framebuffer!"
            );

            TString name = std::format("mappings_framebuffer_{}", i);
            VKDbg::setObjectName(m_MappingsFramebuffers[ i ], name.c_str());
        }
    }


    void VySwapchain::createUVMapSampler() 
    {
        VkExtent2D extent = this->swapchainExtent();

        for (auto& samplers : m_Samplers) 
        {
            createSampler(
                DEFERRED_RESOURCES_FORMAT, 
                VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 
                &samplers.UVReflectionMap, 
                extent
            );
        }

        for (auto& attachments : m_Attachments) 
        {
            createAttachment(
                findDepthFormat(), 
                VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, 
                &attachments.UVReflectionMapDepth, 
                extent
            );
        }
    }


    void VySwapchain::createUVMapRenderPass() 
    {
        TArray<VkAttachmentDescription, 2> attachments{};
        {
            // View space position & normals attachment
            attachments[0].format         = DEFERRED_RESOURCES_FORMAT;
            attachments[0].samples        = VK_SAMPLE_COUNT_1_BIT;
            attachments[0].loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
            attachments[0].storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
            attachments[0].stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachments[0].initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
            attachments[0].finalLayout    = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            attachments[0].flags          = 0;
            
            // Depth attachment (shadow)
            attachments[1].format         = findDepthFormat();
            attachments[1].samples        = VK_SAMPLE_COUNT_1_BIT;
            attachments[1].loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
            attachments[1].storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
            attachments[1].stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachments[1].initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
            attachments[1].finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            attachments[1].flags          = 0;
        }

        VkAttachmentReference colorReferences[1];
        VkAttachmentReference depthReference;
        {
            colorReferences[0] = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
            depthReference     = { 1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };
        }

        // One subpass
        TArray<VkSubpassDescription, 1> subpassDescriptions{};
        {
            subpassDescriptions[0].pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
            subpassDescriptions[0].colorAttachmentCount    = 1;
            subpassDescriptions[0].pColorAttachments       = colorReferences;
            subpassDescriptions[0].pDepthStencilAttachment = &depthReference;
        }

        // Subpass dependencies for layout transitions
        TArray<VkSubpassDependency, 2> dependencies;
        {
            dependencies[0].srcSubpass      = VK_SUBPASS_EXTERNAL;
            dependencies[0].dstSubpass      = 0;
            dependencies[0].srcStageMask    = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
            dependencies[0].dstStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
            dependencies[0].srcAccessMask   = 0;
            dependencies[0].dstAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
            
            dependencies[1].srcSubpass      = 0;
            dependencies[1].dstSubpass      = VK_SUBPASS_EXTERNAL;
            dependencies[1].srcStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
            dependencies[1].dstStageMask    = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            dependencies[1].srcAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            dependencies[1].dstAccessMask   = VK_ACCESS_SHADER_READ_BIT;
            dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
        }

        VkRenderPassCreateInfo renderPassInfo{ VKInit::renderPassCreateInfo() };
        {
            renderPassInfo.attachmentCount = static_cast<U32>(attachments.size());
            renderPassInfo.pAttachments    = attachments.data();
            renderPassInfo.subpassCount    = subpassDescriptions.size();
            renderPassInfo.pSubpasses      = subpassDescriptions.data();
            renderPassInfo.dependencyCount = dependencies.size();
            renderPassInfo.pDependencies   = dependencies.data();
        }

        VK_CHECK_SUCCESS(vkCreateRenderPass(VyContext::device(), &renderPassInfo, nullptr, &m_UVReflectionRenderPass), 
            "Failed to create UV reflection render pass!"
        );

        VKDbg::setObjectName(m_UVReflectionRenderPass, "uv_reflection_renderpass");
    }


    void VySwapchain::createUVMapFramebuffers() 
    {
        m_UVReflectionFramebuffers.resize( this->imageCount() );

        VkExtent2D extent = this->swapchainExtent();

        for (size_t i = 0; i < this->imageCount(); i++) 
        {
            TArray<VkImageView, 2> attachments = { 
                m_Samplers   [ i ].UVReflectionMap.Attachment.View.handle(), 
                m_Attachments[ i ].UVReflectionMapDepth.View.handle() 
            };

            VkFramebufferCreateInfo framebufferInfo{ VKInit::framebufferCreateInfo() };
            {
                framebufferInfo.renderPass      = m_UVReflectionRenderPass;
                framebufferInfo.attachmentCount = static_cast<U32>(attachments.size());
                framebufferInfo.pAttachments    = attachments.data();
                framebufferInfo.width           = extent.width;
                framebufferInfo.height          = extent.height;
                framebufferInfo.layers          = 1;
            }

            VK_CHECK_SUCCESS(vkCreateFramebuffer(VyContext::device(), &framebufferInfo, nullptr, &m_UVReflectionFramebuffers[ i ]), 
                "Failed to create uv map framebuffer!"
            );

            TString name = std::format("uv_map_framebuffer_{}", i);
            VKDbg::setObjectName(m_UVReflectionFramebuffers[ i ], name.c_str());
        }
    }


    void VySwapchain::createDeferredResources() 
    {
        VkExtent2D extent = this->swapchainExtent();

        for (auto& samplers : m_Samplers) 
        {
            createSampler(
                DEFERRED_RESOURCES_FORMAT, 
                VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 
                &samplers.LightingMap, 
                extent
            );
            
            //Check if depth is necessary
        }

        for (auto& attachments : m_Attachments) 
        {
            createAttachment(
                DEFERRED_RESOURCES_FORMAT, 
                VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT, 
                &attachments.Normal, 
                extent
            );

            createAttachment(
                DEFERRED_RESOURCES_FORMAT, 
                VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT, 
                &attachments.Albedo, 
                extent
            );

            createAttachment(
                findDepthFormat(), 
                VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT, 
                &attachments.Depth, 
                extent
            );
        }
    }

    
    void VySwapchain::createLightingRenderPass() 
    {
        TArray<VkAttachmentDescription, 4> attachments {};

        // Color attachment
        attachments[0].format         = DEFERRED_RESOURCES_FORMAT;
        attachments[0].samples        = VK_SAMPLE_COUNT_1_BIT;
        attachments[0].loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachments[0].storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
        attachments[0].stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments[0].initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
        attachments[0].finalLayout    = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        // Deferred attachments
        // Normals
        attachments[1].format         = DEFERRED_RESOURCES_FORMAT;
        attachments[1].samples        = VK_SAMPLE_COUNT_1_BIT;
        attachments[1].loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachments[1].storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments[1].stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments[1].initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
        attachments[1].finalLayout    = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        // Albedo
        attachments[2].format         = DEFERRED_RESOURCES_FORMAT;
        attachments[2].samples        = VK_SAMPLE_COUNT_1_BIT;
        attachments[2].loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachments[2].storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments[2].stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachments[2].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments[2].initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
        attachments[2].finalLayout    = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        // Depth attachment
        attachments[3].format         = findDepthFormat();
        attachments[3].samples        = VK_SAMPLE_COUNT_1_BIT;
        attachments[3].loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachments[3].storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments[3].stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachments[3].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments[3].initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
        attachments[3].finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        // Two subpasses
        TArray<VkSubpassDescription, 2> subpassDescriptions{};

        // First subpass: Fill G-Buffer components
        // ----------------------------------------------------------------------------------------
        VkAttachmentReference colorReferences[2];
        VkAttachmentReference depthReference;
        {
            colorReferences[0] = { 1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
            colorReferences[1] = { 2, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
            depthReference     = { 3, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };
        }

        {
            subpassDescriptions[0].pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
            
            subpassDescriptions[0].colorAttachmentCount    = 2;
            subpassDescriptions[0].pColorAttachments       = colorReferences;

            subpassDescriptions[0].pDepthStencilAttachment = &depthReference;
        }

        // Second subpass: Final composition (by using previous G-Buffer components)
        // ----------------------------------------------------------------------------------------
        
        VkAttachmentReference inputReferences[3];
        VkAttachmentReference colorReference;
        {
            colorReference     = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
            inputReferences[0] = { 1, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
            inputReferences[1] = { 2, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
            inputReferences[2] = { 3, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
        }

        {
            subpassDescriptions[1].pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;
            
            subpassDescriptions[1].colorAttachmentCount = 1;
            subpassDescriptions[1].pColorAttachments    = &colorReference;

            subpassDescriptions[1].inputAttachmentCount = 3;
            subpassDescriptions[1].pInputAttachments    = inputReferences;
        }

        // Subpass dependencies for layout transitions
        TArray<VkSubpassDependency, 3> dependencies;
        {
            dependencies[0].srcSubpass      = VK_SUBPASS_EXTERNAL;
            dependencies[0].dstSubpass      = 0;
            dependencies[0].srcStageMask    = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
            dependencies[0].dstStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
            dependencies[0].srcAccessMask   = 0;
            dependencies[0].dstAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
            
            // This dependency transitions the input attachment from color attachment to shader read
            dependencies[1].srcSubpass      = 0;
            dependencies[1].dstSubpass      = 1;
            dependencies[1].srcStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
            dependencies[1].dstStageMask    = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            dependencies[1].srcAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            dependencies[1].dstAccessMask   = VK_ACCESS_SHADER_READ_BIT;
            dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
            
            dependencies[2].srcSubpass      = 1;
            dependencies[2].dstSubpass      = VK_SUBPASS_EXTERNAL;
            dependencies[2].srcStageMask    = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            dependencies[2].dstStageMask    = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
            dependencies[2].srcAccessMask   = VK_ACCESS_SHADER_WRITE_BIT;
            dependencies[2].dstAccessMask   = VK_ACCESS_MEMORY_READ_BIT;
            dependencies[2].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
        }

        VkRenderPassCreateInfo renderPassInfo{ VKInit::renderPassCreateInfo() };
        {
            renderPassInfo.attachmentCount = static_cast<U32>(attachments.size());
            renderPassInfo.pAttachments    = attachments.data();
            renderPassInfo.subpassCount    = subpassDescriptions.size();
            renderPassInfo.pSubpasses      = subpassDescriptions.data();
            renderPassInfo.dependencyCount = dependencies.size();
            renderPassInfo.pDependencies   = dependencies.data();
        }

        VK_CHECK_SUCCESS(vkCreateRenderPass(VyContext::device(), &renderPassInfo, nullptr, &m_LightingRenderPass), 
            "Failed to create lighting render pass!"
        );

        VKDbg::setObjectName(m_LightingRenderPass, "lighting_renderpass");
    }


    void VySwapchain::createLightingFramebuffers() 
    {
        m_LightingFramebuffers.resize( this->imageCount() );

        for (size_t i = 0; i < this->imageCount(); i++) 
        {
            TArray<VkImageView, 4> attachments = { 
                m_Samplers   [ i ].LightingMap.Attachment.View.handle(), 
                m_Attachments[ i ].Normal.View.handle(), 
                m_Attachments[ i ].Albedo.View.handle(), 
                m_Attachments[ i ].Depth.View.handle() 
            };
            
            VkExtent2D extent = this->swapchainExtent();

            VkFramebufferCreateInfo framebufferInfo{ VKInit::framebufferCreateInfo() };
            {
                framebufferInfo.renderPass      = m_LightingRenderPass;
                framebufferInfo.attachmentCount = static_cast<U32>(attachments.size());
                framebufferInfo.pAttachments    = attachments.data();
                framebufferInfo.width           = extent.width;
                framebufferInfo.height          = extent.height;
                framebufferInfo.layers          = 1;
            }

            VK_CHECK_SUCCESS(vkCreateFramebuffer(VyContext::device(), &framebufferInfo, nullptr, &m_LightingFramebuffers[ i ]), 
                "Failed to create lighting framebuffer!"
            );

            TString name = std::format("lighting_framebuffer_{}", i);
            VKDbg::setObjectName(m_LightingFramebuffers[ i ], name.c_str());
        }
    }


    void VySwapchain::createPostProcessingRenderPass() 
    {
        TArray<VkAttachmentDescription, 1> attachments{};
        {
            // Color attachment (swap chain)
            attachments[0].format         = this->swapchainColorFormat();
            attachments[0].samples        = VK_SAMPLE_COUNT_1_BIT;
            attachments[0].loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
            attachments[0].storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
            attachments[0].stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachments[0].initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
            attachments[0].finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        }
        
        VkAttachmentReference colorReference = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
        
        TArray<VkSubpassDescription, 1> subpassDescriptions{};
        {
            subpassDescriptions[0].pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;
            subpassDescriptions[0].colorAttachmentCount = 1;
            subpassDescriptions[0].pColorAttachments    = &colorReference;
        }

        // Subpass dependencies for layout transitions
        TArray<VkSubpassDependency, 2> dependencies;
        {
            dependencies[0].srcSubpass      = VK_SUBPASS_EXTERNAL;
            dependencies[0].dstSubpass      = 0;
            dependencies[0].srcStageMask    = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
            dependencies[0].dstStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
            dependencies[0].srcAccessMask   = 0;
            dependencies[0].dstAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
            
            dependencies[1].srcSubpass      = 0;
            dependencies[1].dstSubpass      = VK_SUBPASS_EXTERNAL;
            dependencies[1].srcStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
            dependencies[1].dstStageMask    = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            dependencies[1].srcAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            dependencies[1].dstAccessMask   = VK_ACCESS_SHADER_READ_BIT;
            dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
        }

        VkRenderPassCreateInfo renderPassInfo{ VKInit::renderPassCreateInfo() };
        {
            renderPassInfo.attachmentCount = static_cast<U32>(attachments.size());
            renderPassInfo.pAttachments    = attachments.data();
            renderPassInfo.subpassCount    = subpassDescriptions.size();
            renderPassInfo.pSubpasses      = subpassDescriptions.data();
            renderPassInfo.dependencyCount = dependencies.size();
            renderPassInfo.pDependencies   = dependencies.data();
        }

        VK_CHECK_SUCCESS(vkCreateRenderPass(VyContext::device(), &renderPassInfo, nullptr, &m_PostProcessingRenderPass), 
            "Failed to create post processing render pass!"
        );

        VKDbg::setObjectName(m_PostProcessingRenderPass, "post_proc_renderpass");
    }

    
    void VySwapchain::createPostProcessingFramebuffers() 
    {
        m_PostProcessingFramebuffers.resize( this->imageCount() );

        for (size_t i = 0; i < this->imageCount(); i++) 
        {
            TArray<VkImageView, 1> attachments = { m_SwapchainImageViews[i] };

            VkExtent2D extent = this->swapchainExtent();

            VkFramebufferCreateInfo framebufferInfo{ VKInit::framebufferCreateInfo() };
            {
                framebufferInfo.renderPass      = m_PostProcessingRenderPass;
                framebufferInfo.attachmentCount = static_cast<U32>(attachments.size());
                framebufferInfo.pAttachments    = attachments.data();
                framebufferInfo.width           = extent.width;
                framebufferInfo.height          = extent.height;
                framebufferInfo.layers          = 1;
            }

            VK_CHECK_SUCCESS(vkCreateFramebuffer(VyContext::device(), &framebufferInfo, nullptr, &m_PostProcessingFramebuffers[ i ]), 
                "Failed to create post porcessing framebuffer!"
            );

            TString name = std::format("post_proc_framebuffer_{}", i);
            VKDbg::setObjectName(m_PostProcessingFramebuffers[ i ], name.c_str());
        }
    }

    // Returns if a given format support LINEAR filtering
    static VkBool32 formatIsFilterable(VkFormat format, VkImageTiling tiling)
    {
        VkFormatProperties formatProps;
        vkGetPhysicalDeviceFormatProperties(VyContext::physicalDevice(), format, &formatProps);

        if (tiling == VK_IMAGE_TILING_OPTIMAL)
            return formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT;

        if (tiling == VK_IMAGE_TILING_LINEAR)
            return formatProps.linearTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT;

        return false;
    }

    void VySwapchain::createSampler(
        VkFormat          format, 
        VkImageUsageFlags usage, 
        ImageSampler*     pSampler, 
        VkExtent2D        extent, 
        VkImageViewType   imageViewType, 
        U32               arrayLayers) 
    {
        VkFilter shadowMapFilter = formatIsFilterable(format, VK_IMAGE_TILING_OPTIMAL) 
            ? DEFAULT_SHADOWMAP_FILTER 
            : VK_FILTER_NEAREST;

        pSampler->Sampler = VySampler::Builder{}
            .setName         ("attachment")
            .setFilters      (!(imageViewType & VK_IMAGE_VIEW_TYPE_2D) ? VK_FILTER_LINEAR : shadowMapFilter)
            .setMipmapMode   (VK_SAMPLER_MIPMAP_MODE_LINEAR)
            .setWrap         (VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE)
            .setBorder       (VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE)
            .setLodRange     (0.0f, 1.0f)
            .setMipLodBias   (0.0f)
            .build();

        createAttachment(format, usage, &pSampler->Attachment, extent, imageViewType, arrayLayers);
    }

    void VySwapchain::createAttachment(
        VkFormat               format, 
        VkImageUsageFlags      usage, 
        FrameBufferAttachment* pAttachment, 
        VkExtent2D             extent, 
        VkImageViewType        imageViewType, 
        U32                    arrayLayers) 
    {
        pAttachment->Format = format;

        VkComponentMapping componentMapping{};

        auto imgBuilder = VyImage::Builder{};
        {
            imgBuilder.setName("attachment");
            
            if (imageViewType == VK_IMAGE_VIEW_TYPE_CUBE)
            {
                imgBuilder.setFlags( VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT );
                componentMapping = { VK_COMPONENT_SWIZZLE_R };
            }
            
            imgBuilder.setImageType(VK_IMAGE_TYPE_2D);
            imgBuilder.setFormat     (format);
            imgBuilder.setExtent     (extent);
            imgBuilder.setLevels     (1);
            imgBuilder.setLayers     (arrayLayers);
            imgBuilder.setSamples    (VK_SAMPLE_COUNT_1_BIT);
            imgBuilder.setTiling     (VK_IMAGE_TILING_OPTIMAL);
            imgBuilder.setLayout     (VK_IMAGE_LAYOUT_UNDEFINED);
            imgBuilder.setUsage      (usage);
            imgBuilder.setMemoryUsage(VMA_MEMORY_USAGE_AUTO);
        }

        pAttachment->Image = imgBuilder.build();

        auto viewBuilder = VyImageView::Builder{};
        {
            viewBuilder.setName("attachment");

            viewBuilder.setViewType(imageViewType);
            viewBuilder.setFormat  (format);

            if (usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) 
            {
                viewBuilder.setAspect(VK_IMAGE_ASPECT_DEPTH_BIT);
            }
            else {
                viewBuilder.setAspect(VK_IMAGE_ASPECT_COLOR_BIT);
            }

            viewBuilder.setLevels  (0, 1);
            viewBuilder.setLayers  (0, arrayLayers);

            viewBuilder.setMapping(componentMapping);
        }

        pAttachment->View = viewBuilder.build( pAttachment->Image );
    }






    // void VySwapchain::destroyAttachment(FrameBufferAttachment* pAttachment) 
    // {
    //     vkDestroyImageView(VyContext::device(), pAttachment->view, nullptr);
    //     allocator.destroyImage(pAttachment->image, pAttachment->memory);
    // }

    // void VySwapchain::destroySampler(ImageSampler* sampler) {
    //     destroyAttachment(&sampler->pAttachment);
    //     vkDestroySampler(VyContext::device(), sampler->sampler, nullptr);
    // }

    VkDescriptorImageInfo FrameBufferAttachment::descriptorImageInfo(VkSampler sampler, VkImageLayout imageLayout) 
    {
        return VkDescriptorImageInfo{
                sampler,
                View.handle(),
                imageLayout
        };
    }

    void VySwapchain::createDescriptorPool() 
    {
        m_GlobalPool = VyDescriptorPool::Builder{}
            .setMaxSets(6 * this->imageCount())
            .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 6 * this->imageCount())
            .addPoolSize(VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 3 * this->imageCount())
            .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4 * this->imageCount())
            .buildPtr();
    }

    void VySwapchain::createUniformBuffers() 
    {
        m_ShadowBuffers        .clear();
        m_MappingBuffers       .clear();
        m_UVReflectionBuffers  .clear();
        m_GBufferUboBuffers    .clear();
        m_CompositionBuffers   .clear();
        m_PostProcessingBuffers.clear();

        m_ShadowBuffers        .resize( this->imageCount() );
        m_MappingBuffers       .resize( this->imageCount() );
        m_UVReflectionBuffers  .resize( this->imageCount() );
        m_GBufferUboBuffers    .resize( this->imageCount() );
        m_CompositionBuffers   .resize( this->imageCount() );
        m_PostProcessingBuffers.resize( this->imageCount() );

        for (int i = 0; i < this->imageCount(); i++) 
        {
            m_ShadowBuffers        [i] = MakeUnique<VyBuffer>( VyBuffer::uniformBuffer( "shadow",        sizeof(ShadowUbo),         1 ) );
            m_MappingBuffers       [i] = MakeUnique<VyBuffer>( VyBuffer::uniformBuffer( "mappings",      sizeof(MappingsUbo),       1 ) );
            m_UVReflectionBuffers  [i] = MakeUnique<VyBuffer>( VyBuffer::uniformBuffer( "uv_reflection", sizeof(UVReflectionUbo),   1 ) );
            m_GBufferUboBuffers    [i] = MakeUnique<VyBuffer>( VyBuffer::uniformBuffer( "gbuffer",       sizeof(GBufferUbo),        1 ) );
            m_CompositionBuffers   [i] = MakeUnique<VyBuffer>( VyBuffer::uniformBuffer( "composition",   sizeof(CompositionUbo),    1 ) );
            m_PostProcessingBuffers[i] = MakeUnique<VyBuffer>( VyBuffer::uniformBuffer( "post_proc",     sizeof(PostProcessingUbo), 1 ) );
        }

        m_ShadowSetLayout = VyDescriptorSetLayout::Builder()
            .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
            .buildPtr();

        m_ShadowSets.clear();
        m_ShadowSets.resize( this->imageCount() );

        for (int i = 0; i < m_ShadowSets.size(); i++) 
        {
            auto bufferInfo = m_ShadowBuffers[i]->descriptorBufferInfo();
        
            VyDescriptorWriter(*m_ShadowSetLayout, *m_GlobalPool)
                .writeBuffer(0, &bufferInfo)
                .build(m_ShadowSets[i]);
        }

        m_MappingsSetLayout = VyDescriptorSetLayout::Builder()
            .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
            .buildPtr();

        m_MappingSets.clear();
        m_MappingSets.resize( this->imageCount() );

        for (int i = 0; i < m_MappingSets.size(); i++) 
        {
            auto bufferInfo = m_MappingBuffers[i]->descriptorBufferInfo();

            VyDescriptorWriter(*m_MappingsSetLayout, *m_GlobalPool)
                .writeBuffer(0, &bufferInfo)
                .build(m_MappingSets[i]);
        }

        m_UVReflectionSetLayout = VyDescriptorSetLayout::Builder{}
            .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
            .addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
            .buildPtr();

        m_UVReflectionSets.clear();
        m_UVReflectionSets.resize( this->imageCount() );

        for (int i = 0; i < m_UVReflectionSets.size(); i++) 
        {
            auto bufferInfo  = m_UVReflectionBuffers[i]->descriptorBufferInfo();
            auto mappingsMap = m_Samplers[i].MappingsMap.Attachment.descriptorImageInfo(m_Samplers[i].MappingsMap.Sampler);
        
            VyDescriptorWriter(*m_UVReflectionSetLayout, *m_GlobalPool)
                .writeBuffer(0, &bufferInfo)
                .writeImage (1, &mappingsMap)
                .build(m_UVReflectionSets[i]);
        }

        m_GBufferSetLayout = VyDescriptorSetLayout::Builder{}
            .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
            .buildPtr();

        m_GBufferDescriptorSets.clear();
        m_GBufferDescriptorSets.resize( this->imageCount() );
        
        for (int i = 0; i < m_GBufferDescriptorSets.size(); i++) 
        {
            auto bufferInfo = m_GBufferUboBuffers[i]->descriptorBufferInfo();

            VyDescriptorWriter(*m_GBufferSetLayout, *m_GlobalPool)
                .writeBuffer(0, &bufferInfo)
                .build(m_GBufferDescriptorSets[i]);
        }

        m_CompositionSetLayout = VyDescriptorSetLayout::Builder{}
            .addBinding(0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,       VK_SHADER_STAGE_FRAGMENT_BIT)
            .addBinding(1, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,       VK_SHADER_STAGE_FRAGMENT_BIT)
            .addBinding(2, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,       VK_SHADER_STAGE_FRAGMENT_BIT)
            .addBinding(3, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
            .addBinding(4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
            .buildPtr();

        m_CompositionSets.clear();
        m_CompositionSets.resize( this->imageCount() );

        for (int i = 0; i < m_CompositionSets.size(); i++) 
        {
            auto normalInfo = m_Attachments[i].Normal.descriptorImageInfo();
            auto albedoInfo = m_Attachments[i].Albedo.descriptorImageInfo();
            auto depthInfo  = m_Attachments[i].Depth.descriptorImageInfo();
            auto bufferInfo = m_CompositionBuffers[i]->descriptorBufferInfo();
            auto shadowOmni = m_Samplers[i].ShadowOmniMap.Attachment.descriptorImageInfo( m_Samplers[i].ShadowOmniMap.Sampler.handle() );

            VyDescriptorWriter(*m_CompositionSetLayout, *m_GlobalPool)
                .writeImage (0, &normalInfo)
                .writeImage (1, &albedoInfo)
                .writeImage (2, &depthInfo)
                .writeBuffer(3, &bufferInfo)
                .writeImage (4, &shadowOmni)
                .build(m_CompositionSets[i]);
        }

        m_PostProcessingSetLayout = VyDescriptorSetLayout::Builder{}
            .addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
            .addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
            .addBinding(2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT)
            .buildPtr();

        m_PostProcessingSets.clear();
        m_PostProcessingSets.resize( this->imageCount() );

        for (int i = 0; i < m_PostProcessingSets.size(); i++) 
        {
            auto uvReflection  = m_Samplers[i].UVReflectionMap.Attachment.descriptorImageInfo(m_Samplers[i].UVReflectionMap.Sampler);
            auto lightingImage = m_Samplers[i].LightingMap.Attachment.descriptorImageInfo(m_Samplers[i].LightingMap.Sampler);
            auto bufferInfo    = m_PostProcessingBuffers[i]->descriptorBufferInfo();
        
            VyDescriptorWriter(*m_PostProcessingSetLayout, *m_GlobalPool)
                .writeImage (0, &uvReflection)
                .writeImage (1, &lightingImage)
                .writeBuffer(2, &bufferInfo)
                .build(m_PostProcessingSets[i]);
        }
    }

    void VySwapchain::updateCurrentShadowUbo(void* data, int currentImageIndex) 
    {
        m_ShadowBuffers[currentImageIndex]->write(data, sizeof(ShadowUbo), 0);
        // m_ShadowBuffers[currentImageIndex]->flush();
    }
    void VySwapchain::updateCurrentMappingsUbo(void* data, int currentImageIndex) 
    {
        m_MappingBuffers[currentImageIndex]->write(data, sizeof(MappingsUbo), 0);
        // m_MappingBuffers[currentImageIndex]->flush();
    }
    void VySwapchain::updateCurrentUVReflectionUbo(void* data, int currentImageIndex) 
    {
        m_UVReflectionBuffers[currentImageIndex]->write(data, sizeof(UVReflectionUbo), 0);
        // m_UVReflectionBuffers[currentImageIndex]->flush();
    }
    void VySwapchain::updateCurrentGBufferUbo(void* data, int currentImageIndex) 
    {
        m_GBufferUboBuffers[currentImageIndex]->write(data, sizeof(GBufferUbo), 0);
        // m_GBufferUboBuffers[currentImageIndex]->flush();
    }
    void VySwapchain::updateCurrentCompositionUbo(void* data, int currentImageIndex) 
    {
        m_CompositionBuffers[currentImageIndex]->write(data, sizeof(CompositionUbo), 0);
        // m_CompositionBuffers[currentImageIndex]->flush();
    }
    void VySwapchain::updateCurrentPostProcessingUbo(void* data, int currentImageIndex) 
    {
        m_PostProcessingBuffers[currentImageIndex]->write(data, sizeof(PostProcessingUbo), 0);
        // m_PostProcessingBuffers[currentImageIndex]->flush();
    }
}










