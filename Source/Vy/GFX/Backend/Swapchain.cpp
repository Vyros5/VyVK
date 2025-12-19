#include <Vy/GFX/Backend/Swapchain.h>

#include <Vy/GFX/Context.h>

#include <Vy/GFX/Backend/VK/vk_enum_str.h>
#include <Vy/GFX/Backend/Resources/RenderPass.h>
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


    VySwapchain::VySwapchain(VkExtent2D extent) : 
        m_WindowExtent{ extent }
    {
        m_PresentIdState.Enabled = VyContext::device().supportsPresentId();

        init();
    }


    VySwapchain::VySwapchain(VkExtent2D extent, Shared<VySwapchain> previous) : 
        m_WindowExtent{ extent   }, 
        m_OldSwapchain{ previous }
    {
        m_PresentIdState.Enabled = VyContext::device().supportsPresentId();

        init();

        // Clean up old swap chain since it's no longer being used.
        m_OldSwapchain = nullptr;
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

        createSyncObjects();
    }

    // ---------------------------------------------------------------------------------------------------------------------

    VkResult VySwapchain::acquireNextImage(U32* pImageIndex) 
    {
        // Wait for the fence of the current frame to have completed (Waits until command buffer has completed).
        vkWaitForFences(VyContext::device(), 1, &m_InFlightFences[ m_CurrentFrame ], VK_TRUE, UINT64_MAX);

        // Get next image to draw to in the swap chain.
        VkResult result = vkAcquireNextImageKHR(
            VyContext::device(),                          // Device swap chain is on.
            m_Swapchain,                                  // Swapchain being used.
            UINT64_MAX,                                   // Use max so no timeout.
            m_ImageAvailableSemaphores[ m_CurrentFrame ], // Give semaphore to be triggered when image is ready for rendering (Must not be a signaled semaphore).
            VK_NULL_HANDLE,                               // No fences in use for this.
            pImageIndex                                   // Gets set to image index to use.
        );

        return result;
    }

    // ---------------------------------------------------------------------------------------------------------------------

    VkResult VySwapchain::submitCommandBuffers(const VkCommandBuffer* pCmdBuffers, U32* pImageIndex) 
    {
        if (m_ImagesInFlight[ *pImageIndex ] != VK_NULL_HANDLE)
        {
            // Wait for all fences in the array to be signaled.
            vkWaitForFences(VyContext::device(), 1, &m_ImagesInFlight[ *pImageIndex ], VK_TRUE, UINT64_MAX);
        }

        // Mark the image as now being in use by this frame.
        // Associate the current frame's fence with the acquired swapchain image.
        m_ImagesInFlight[ *pImageIndex ] = m_InFlightFences[ m_CurrentFrame ];

        // The RenderFinishedSemaphores for the current frame to be submitted as the command buffer complete signal semaphore.
        VkSemaphore signalSemaphores[] = { m_RenderFinishedSemaphores[ m_CurrentFrame ] };

        // Specify the semaphore to wait on before execution begins and in which stage of the pipeline to wait.
        VkSemaphore          waitSemaphores[]   = { m_ImageAvailableSemaphores[ m_CurrentFrame ] };
        VkPipelineStageFlags waitStages[]       = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        
        // [ Submit ]
        VkSubmitInfo submitInfo{ VKInit::submitInfo() };
        {
            // Semaphore(s) to wait before the command buffers for this batch begin execution.
            submitInfo.waitSemaphoreCount   = 1;
            submitInfo.pWaitSemaphores      = waitSemaphores;
            // Pipeline stages at which each corresponding semaphore wait will occur.
            submitInfo.pWaitDstStageMask    = waitStages;
            
            // Command buffers to submit for execution.
            submitInfo.commandBufferCount   = 1;
            submitInfo.pCommandBuffers      = pCmdBuffers;
            
            // Semaphore(s) to signal once execution of the command buffer has completed.
            submitInfo.signalSemaphoreCount = 1;
            submitInfo.pSignalSemaphores    = signalSemaphores;
        }

        // Restore fences from signaled to unsignaled state.
        vkResetFences(VyContext::device(), 1, &m_InFlightFences[ m_CurrentFrame ]);
        
        // Submit command buffer to the graphics queue.
        // Pass in a fence to signal when the command buffer being submitted has finished executing.
		VK_CHECK(vkQueueSubmit(VyContext::device().graphicsQueue(), 1, &submitInfo, m_InFlightFences[ m_CurrentFrame ]));

        // Set this swapchain as the swapchain to use for presentation.
        VkSwapchainKHR swapchains[] = { m_Swapchain };

        // [ Present ]
        VkPresentInfoKHR presentInfo{ VKInit::presentInfoKHR() };
        {
            // Submit the signalSemaphores from the graphics queue completion as the wait, this is so
            // it will wait to present onto the screen until the command buffer has finished executing on the GPU.

            // Wait semaphores being used.
            presentInfo.waitSemaphoreCount = 1;
            presentInfo.pWaitSemaphores    = signalSemaphores;
            
            // Swapchains to present images to.
            presentInfo.swapchainCount     = 1;
            presentInfo.pSwapchains        = swapchains; 
            
            // Index of the image to present.
            presentInfo.pImageIndices      = pImageIndex;
        }

        // https://docs.vulkan.org/refpages/latest/refpages/source/VkPresentIdKHR.html
        VkPresentIdKHR presentIdInfo{};
        {
            presentIdInfo.sType          = VK_STRUCTURE_TYPE_PRESENT_ID_KHR;
            presentIdInfo.pNext          = nullptr;

            // The number of swapchains being presented to the vkQueuePresentKHR command.
            presentIdInfo.swapchainCount = 0;

            // Pointer to an array of U64 with swapchainCount entries. 
            // Each non-zero value in pPresentIds specifies the present id to be associated with 
            // the presentation of the swapchain with the same index in the vkQueuePresentKHR call.
            presentIdInfo.pPresentIds    = nullptr;
        }

        U64 presentIdValue = 0;

        if (m_PresentIdState.Enabled)
        {
            // Tag each present so validation can correlate semaphore ownership.
            {
                presentIdValue               = m_PresentIdState.Next++;
                presentIdInfo.swapchainCount = 1;
                presentIdInfo.pPresentIds    = &presentIdValue;
            }

            // Chain prsentIdInfo to presentInfo.
            presentInfo.pNext = &presentIdInfo;
        }

        // Send image to be presented to the display.
        auto result = vkQueuePresentKHR(VyContext::device().presentQueue(), &presentInfo);

        // Advance to the next frame.
        m_CurrentFrame = (m_CurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

        return result;
    }

    // ---------------------------------------------------------------------------------------------------------------------

#pragma region [ Creation ]

    void VySwapchain::createSwapchain() 
    {
        // Get the swap chain support details of the current device.
        SwapchainSupportDetails swapchainSupport = VyContext::device().querySwapchainSupport();

        // Get the format to use for this swapchain's surface.
        VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapchainSupport.Formats);

        // Get present mode to use for this swapchain.
        VkPresentModeKHR presentMode = chooseSwapPresentMode(swapchainSupport.PresentModes);

        // Gets the extent (size) to use for this swapchain.
        VkExtent2D extent = chooseSwapExtent(swapchainSupport.Capabilities);

        // Get the minimum image count to use.
		U32 imageCount = chooseImageCount(swapchainSupport.Capabilities);

        VkSwapchainCreateInfoKHR createInfo{ VKInit::swapchainCreateInfoKHR() };
        {
            // Surface onto which the swapchain will present images. 
            createInfo.surface = VyContext::device().surface();
            
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
		    VY_ASSERT(indices.isComplete(), "Queue family indices are not complete.");
		
            TArray<U32, 2> queueFamilyIndices{ indices.GraphicsFamily.value(), indices.PresentFamily.value() };

            // Check if the two queues are not the same (i.e. if the device does not use one queue for both).
            if (indices.GraphicsFamily != indices.PresentFamily) 
            {
                // Set sharing mode to concurrent so multiple queue families can access this swapchain.
                createInfo.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;

                // Set number of queue families accessing this and their indices.
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
            createInfo.preTransform     = preTransform;

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

            // Set compositeAlpha to opaque, which means the compositing alpha is always treated as 1.0 (i.e. not using alpha currently)
            createInfo.compositeAlpha   = compositeAlpha;

            // Set present mode.
            createInfo.presentMode      = presentMode;

            // Dont render to non-visable pixels (like if covered by another window).
            createInfo.clipped          = VK_TRUE;

            // Dont provide old swapchain if it doesnt exist, else provide it so it can resuse memory when possible.
            createInfo.oldSwapchain     = m_OldSwapchain == nullptr ? VK_NULL_HANDLE : m_OldSwapchain->m_Swapchain;
        }

        // Create swapchain.
		VK_CHECK(vkCreateSwapchainKHR(VyContext::device(), &createInfo, nullptr, &m_Swapchain));

        // destroySwapchain(createInfo.oldSwapchain);

        // We only specified a minimum number of images in the swap chain (chooseImageCount()), 
        // so the implementation is allowed to create a swap chain with more. 

        // That's why we'll first query the final number of images with vkGetSwapchainImagesKHR, 
        // then resize the container and finally call it again to retrieve the handles.

        vkGetSwapchainImagesKHR(VyContext::device(), m_Swapchain, &imageCount, nullptr);

        m_SwapchainImages.resize( imageCount );
        vkGetSwapchainImagesKHR(VyContext::device(), m_Swapchain, &imageCount, m_SwapchainImages.data());

        // Set format and extent member varibles.
        m_SwapchainImageFormat = surfaceFormat.format;
        m_SwapchainExtent      = extent;

		// At the beginning of a frame all swapchain images should be in `present` layout.
		// for (const auto& image : m_SwapchainImages)
		// {
		// 	VyContext::device().transitionImageLayout(
		// 		image, 
		// 		m_SwapchainImageFormat, 
		// 		VK_IMAGE_LAYOUT_UNDEFINED, 
		// 		VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
		// 	);
		// }
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
				viewInfo.image    = m_SwapchainImages[i];   // The image that this view is referring to.
				viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;  // Set so that this image is 2D.
				viewInfo.format   = m_SwapchainImageFormat; // Set the format to saved format found earlier.

                // Specifiy that the color compontent is what is to be rendered (compared to the depth bit, etc.)
				viewInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
				viewInfo.subresourceRange.baseMipLevel   = 0; // Dont scale down the image by defualt.
				viewInfo.subresourceRange.levelCount     = 1; // Only allow defualt scale level (no other mip levels).
				viewInfo.subresourceRange.baseArrayLayer = 0; // Make so the base accessed level into the image is 0.
				viewInfo.subresourceRange.layerCount     = 1; // Make so the image only has one level.
			}

            // Create Image View.
			VK_CHECK(vkCreateImageView(VyContext::device(), &viewInfo, nullptr, &m_SwapchainImageViews[i]));
        }
    }

    // ---------------------------------------------------------------------------------------------------------------------
    // MARK: RenderPass

    void VySwapchain::createRenderPass() 
    {
        // RenderPass::Builder rpBuilder;

        // auto swapchainFormat = swapchainImageFormat();
        // auto depthFormat     = findDepthFormat();

        // RenderPass rp = rpBuilder
        //     // 0
        //     .beginAttachment(VY_ATTACHMENT_COLOR)
        //         .setFormat(swapchainFormat)
        //         .setSamples(VK_SAMPLE_COUNT_1_BIT)
        //         .setLoadOpClear()
        //         .setStoreOpStore()
        //         .setStencilLoadOpDontCare()
        //         .setStencilStoreOpDontCare()
        //         .setInitialLayout(VK_IMAGE_LAYOUT_UNDEFINED)
        //         .setFinalLayoutPresentSrc()
        //     .endAttachment()
        //     // 1
        //     .beginAttachment(VY_ATTACHMENT_DEPTH)
        //         .setFormat(depthFormat)
        //         .setSamples(VK_SAMPLE_COUNT_1_BIT)
        //         .setLoadOpClear()
        //         .setStoreOpDontCare()
        //         .setStencilLoadOpDontCare()
        //         .setStencilStoreOpDontCare()
        //         .setInitialLayout(VK_IMAGE_LAYOUT_UNDEFINED)
        //         .setFinalLayoutDepthStencilAttachment()
        //     .endAttachment()

        //     .beginSubpass(VK_PIPELINE_BIND_POINT_GRAPHICS)
        //         .addColorAttachment       ( 0 )
        //         .setDepthStencilAttachment( 1 )
        //     .endSubpass()

        //     .beginDependency(VK_SUBPASS_EXTERNAL, 0)
        //         .setSrcAccessMask(0)
        //         .setDstAccessMask(VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT)
        //         .setSrcStageMask(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT)
        //         .setDstStageMask(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT)
        //     .endDependency()

        // .build();

        // [ Depth Attachment ]
        VkAttachmentDescription depthAttachment{};
        {
            // Find and set image format to use for this depth buffer.
            depthAttachment.format         = findDepthFormat(); 

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
            colorAttachment.format         = swapchainImageFormat(); 

            // // One sample per pixel (more samples used for multisampling).
            colorAttachment.samples        = VK_SAMPLE_COUNT_1_BIT; 

			// VK_ATTACHMENT_LOAD_OP_LOAD:      Preserve the existing contents of the attachment.
			// VK_ATTACHMENT_LOAD_OP_CLEAR:     Clear the values to a constant at the start.
			// VK_ATTACHMENT_LOAD_OP_DONT_CARE: Existing contents are undefined.

            // Tells the colorbuffer attachment to clear each time it is loaded.
            colorAttachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR; 

			// VK_ATTACHMENT_STORE_OP_STORE:     Rendered contents will be stored in memory and can be read later.
			// VK_ATTACHMENT_STORE_OP_DONT_CARE: Contents of the framebuffer will be undefined after the rendering operation.

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

        // //A subpass in Vulkan is a phase of rendering that can read from and write to certain framebuffer attachments (color, depth, and stencil buffers).
        VkSubpassDescription subpass{};
        {
            // Define that this subpass is for graphics output.
            subpass.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;

            // Number of color attachments used.
            subpass.colorAttachmentCount    = 1;

            // Refenece to the attachment index and layout for the color attachment.
            subpass.pColorAttachments       = &colorAttachmentRef; 

            // Refenece to the attachment index and layout for the depth attachment.
            subpass.pDepthStencilAttachment = &depthAttachmentRef; 
        }

        // Declare a dependency for the subpass (forces thread sync between source and destination).
        VkSubpassDependency dependency{};
        {
            // Sets the source of this dependency to be before the subpasses start.
            dependency.srcSubpass    = VK_SUBPASS_EXTERNAL;

            // No specific memory access is being waited on from the source operation.
            dependency.srcAccessMask = 0;

            // The render pass should wait until all color attachment writes and early fragment (depth) tests outside of the render pass have been completed.
            dependency.srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;

            // Sets the destination of this dependency to be the first subpass.
            dependency.dstSubpass    = 0;

            // These access masks ensure that the operations described by dstStageMask (color and depth/stencil attachment writes) 
            // will wait for the completion of the stages in the srcStageMask.
            dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

            // Subpass 0's color attachment output and early fragment tests will only begin after the external stages 
            // (color attachment output and early fragment tests) are complete.
            dependency.dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        }

        // Create RenderPass.
		VyRenderPassDesc renderPassDesc{};
        {
			renderPassDesc.AttachmentDescriptions = { colorAttachment, depthAttachment };
			renderPassDesc.SubpassDescription     = subpass;
			renderPassDesc.SubpassDependency      = dependency;
		}

        m_RenderPass = MakeUnique<VyRenderPass>(renderPassDesc);
    }


    // void VySwapchain::createShadowMapRenderPass()
    // {
    //     VkAttachmentDescription depthAttachment{};
    //     {
    //         depthAttachment.format         = VK_FORMAT_D32_SFLOAT;
    //         depthAttachment.samples        = VK_SAMPLE_COUNT_1_BIT;
    //         depthAttachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
    //         depthAttachment.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
    //         depthAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    //         depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    //         depthAttachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    //         depthAttachment.finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
    //     }

    //     VkAttachmentReference depthReference{};
    //     {
    //         depthReference.attachment = 0;
    //         depthReference.layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    //     }

    //     VkSubpassDescription subpass{};
    //     {
    //         subpass.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
    //         subpass.colorAttachmentCount    = 0;
    //         subpass.pDepthStencilAttachment = &depthReference;
    //     }

    //     TArray<VkSubpassDependency, 2> dependencies;
    //     {
    //         dependencies[0].srcSubpass      = VK_SUBPASS_EXTERNAL;
    //         dependencies[0].dstSubpass      = 0;
    //         dependencies[0].srcStageMask    = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    //         dependencies[0].dstStageMask    = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    //         dependencies[0].srcAccessMask   = VK_ACCESS_SHADER_READ_BIT;
    //         dependencies[0].dstAccessMask   = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    //         dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
            
    //         dependencies[1].srcSubpass      = 0;
    //         dependencies[1].dstSubpass      = VK_SUBPASS_EXTERNAL;
    //         dependencies[1].srcStageMask    = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    //         dependencies[1].dstStageMask    = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    //         dependencies[1].srcAccessMask   = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    //         dependencies[1].dstAccessMask   = VK_ACCESS_SHADER_READ_BIT;
    //         dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
    //     }

    //     VkRenderPassCreateInfo renderPassInfo{ VKInit::renderPassCreateInfo() };
    //     {
    //         renderPassInfo.attachmentCount = 1;
    //         renderPassInfo.pAttachments    = &depthAttachment;

    //         renderPassInfo.subpassCount    = 1;
    //         renderPassInfo.pSubpasses      = &subpass;
            
    //         renderPassInfo.dependencyCount = static_cast<U32>(dependencies.size());
    //         renderPassInfo.pDependencies   = dependencies.data();
    //     }

    //     // Create RenderPass.
    //     m_ShadowRenderPass = MakeUnique<VyRenderPass>(renderPassInfo);
    // }

    // ---------------------------------------------------------------------------------------------------------------------
    // MARK: Framebuffers

    void VySwapchain::createFramebuffers() 
    {
        // Resize framebuffers to match swapchain image count.
        m_SwapchainFramebuffers.reserve(imageCount());
        
        // Iterate over all the framebuffers.
        for (size_t i = 0; i < imageCount(); i++) 
        {
            // Get color and depth attachment images for current index.
            const std::vector attachments = { m_SwapchainImageViews[i], m_DepthImageViews[i].handle() };

            // Get the size of the swapchain.
            VkExtent2D swapchainExtent = this->swapchainExtent();

			VyFramebufferDesc framebufferDesc{};
            {
                // Set size to match swapchain size.
				framebufferDesc.Width       = swapchainExtent.width;
				framebufferDesc.Height      = swapchainExtent.height;

                // Attachments
				framebufferDesc.Attachments = attachments;

                // Only one layer.
                framebufferDesc.Layers      = 1;

                // Flags
                framebufferDesc.Flags       = 0;
			}

            // Renderpass to use this framebuffer with.
			m_SwapchainFramebuffers.emplace_back( *m_RenderPass, framebufferDesc );
        }
    }


    // void VySwapchain::createShadowMapFramebuffer()
    // {
    //     VY_ASSERT(m_ShadowRenderPass     != VK_NULL_HANDLE, "m_ShadowRenderPass is invalid!");
    //     VY_ASSERT(m_ShadowDepthImageView != VK_NULL_HANDLE, "m_ShadowDepthImageView is invalid!");

    //     const std::vector attachments = { m_ShadowDepthImageView.handle() };

    //     VyFramebufferDesc framebufferDesc{};
    //     {
    //         // Set size to match shadow map size.
    //         framebufferDesc.Width       = m_ShadowMapExtent.width;
    //         framebufferDesc.Height      = m_ShadowMapExtent.height;

    //         // Attachments
    //         framebufferDesc.Attachments = attachments;

    //         // Only one layer.
    //         framebufferDesc.Layers      = 1;

    //         // Flags
    //         framebufferDesc.Flags       = 0;
    //     }

    //     m_ShadowMapFramebuffer = VyFramebuffer{ *m_ShadowRenderPass, framebufferDesc };
    // }

    // ---------------------------------------------------------------------------------------------------------------------
    // MARK: Depth Res.

    void VySwapchain::createDepthResources() 
    {
        // Get the format of the image being used in this swapchain's renderpass's depth attachment.
        VkFormat depthFormat   = findDepthFormat();
        m_SwapchainDepthFormat = depthFormat;

        // Get the size of the swapchain output.
        VkExtent2D swapchainExtent = this->swapchainExtent();

        // Resize all vectors of depth images/images data to equal the number of images the swapchain is using.
        m_DepthImages    .resize( imageCount() );
        m_DepthImageViews.resize( imageCount() );

        // Iterate over each depth image and create an image view for it.
        for (int i = 0; i < m_DepthImages.size(); i++) 
        {
            // Depth Images
            auto builder = VyImage::Builder{};
            {
                builder.imageType(VK_IMAGE_TYPE_2D);
                builder.extent(swapchainExtent);
                builder.mipLevels(1);
                builder.arrayLayers(1);
                
                // Set the format to match the format of the depth attachment on the renderpass.
                builder.format(depthFormat);
                
                // Set tiling settings to be optimal.
                builder.tiling(VK_IMAGE_TILING_OPTIMAL);
                
                // Undefined layout
                builder.imageLayout(VK_IMAGE_LAYOUT_UNDEFINED);
                
                // Set the image to be used as a depth/stencil attachment.
                builder.usage(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
                
                // No multisampling.
                builder.sampleCount(VK_SAMPLE_COUNT_1_BIT);
                
                // Set so image can only be accessed by one command buffer queue family at a time.
                builder.sharingMode(VK_SHARING_MODE_EXCLUSIVE);
                
                builder.memoryUsage(VMA_MEMORY_USAGE_AUTO);
            }

            m_DepthImages[i] = builder.build();

            // Depth ImageView Framebuffer Attachments.
            auto viewBuilder = VyImageView::Builder{};
            {
                viewBuilder.viewType(VK_IMAGE_VIEW_TYPE_2D);
                viewBuilder.format(depthFormat);
                viewBuilder.aspectMask(VK_IMAGE_ASPECT_DEPTH_BIT);
                viewBuilder.arrayLayers(0, 1);
                viewBuilder.mipLevels(0, 1);
            }
            
            m_DepthImageViews[i] = viewBuilder.build(m_DepthImages[i]);
        }
    }


    // void VySwapchain::createShadowDepthImage()
    // {
    //     m_ShadowImage = VyImage::Builder{}
    //         .imageType(VK_IMAGE_TYPE_2D)
    //         .extent(m_ShadowMapExtent)
    //         .mipLevels(1)
    //         .arrayLayers(1)
    //         .format(VK_FORMAT_D32_SFLOAT)
    //         .tiling(VK_IMAGE_TILING_OPTIMAL)
    //         .usage(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
    //         .sampleCount(VK_SAMPLE_COUNT_1_BIT)
    //         .sharingMode(VK_SHARING_MODE_EXCLUSIVE)
    //         .memoryUsage(VMA_MEMORY_USAGE_AUTO)
    //     .build();

    //     m_ShadowDepthImageView = VyImageView::Builder{}
    //         .viewType(VK_IMAGE_VIEW_TYPE_2D)
    //         .format(VK_FORMAT_D32_SFLOAT)
    //         .aspectMask(VK_IMAGE_ASPECT_DEPTH_BIT)
    //         .arrayLayers(0, 1)
    //         .mipLevels(0, 1)
    //     .build(m_ShadowImage);

    //     m_ShadowSampler = VySampler::Builder{}
    //         .filters(VK_FILTER_LINEAR)
    //         .mipmapMode(VK_SAMPLER_MIPMAP_MODE_LINEAR)
    //         .addressMode(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER)
    //         .borderColor(VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE)
    //         .enableAnisotropy(true)
    //         .lodRange(0.0f, 100.0f)
    //         .mipLodBias(0.0f)
    //     .build();
    // }

    // ---------------------------------------------------------------------------------------------------------------------
    // MARK: Sync Objects

    void VySwapchain::createSyncObjects() 
    {
        // m_RenderFinishedSemaphores.assign( MAX_FRAMES_IN_FLIGHT, VK_NULL_HANDLE );
        m_ImageAvailableSemaphores.assign( MAX_FRAMES_IN_FLIGHT, VK_NULL_HANDLE );
        m_RenderFinishedSemaphores.assign( imageCount(),         VK_NULL_HANDLE );
        m_InFlightFences          .assign( MAX_FRAMES_IN_FLIGHT, VK_NULL_HANDLE );
        m_ImagesInFlight          .assign( imageCount(),         VK_NULL_HANDLE );

        VkSemaphoreCreateInfo semaphoreInfo{ VKInit::semaphoreCreateInfo() };

        // Specify a signaled fence.
        VkFenceCreateInfo fenceInfo{ VKInit::fenceCreateInfo() };
        {
            fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        }

        // Create sync objects for each frame.
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			VK_CHECK(vkCreateSemaphore(VyContext::device(), &semaphoreInfo, nullptr, &m_ImageAvailableSemaphores[i]));
			VK_CHECK(vkCreateSemaphore(VyContext::device(), &semaphoreInfo, nullptr, &m_RenderFinishedSemaphores[i]));
			VK_CHECK(vkCreateFence    (VyContext::device(), &fenceInfo,     nullptr, &m_InFlightFences[i]          ));
		}
    }

#pragma endregion Creation

    // ---------------------------------------------------------------------------------------------------------------------

    void VySwapchain::cleanup()
    {
        m_SwapchainFramebuffers.clear();

        // Cleanup Swapchain (Color) Image Views.
		for (auto imageView : m_SwapchainImageViews)
		{
            if (imageView != VK_NULL_HANDLE)
            {
                VyContext::destroy(imageView);
            }
		}

        m_SwapchainImageViews.clear();

        // Cleanup Depth Resources.
        // for (int i = 0; i < m_DepthImages.size(); i++) 
        // {
        //     if (m_DepthImageViews[i] != VK_NULL_HANDLE)
        //     {
        //         VyContext::destroy(m_DepthImageViews[i]);
        //     }

        //     if (m_DepthImages[i] != VK_NULL_HANDLE)
        //     {
        //         VyContext::destroy(m_DepthImages[i], m_DepthImageAllocations[i]);
        //     }
        // }

        m_DepthImageViews      .clear();
        m_DepthImages          .clear();


        // Cleanup Render Pass.
        // if (m_RenderPass != VK_NULL_HANDLE)
        // {
        //     vkDestroyRenderPass(VyContext::device(), m_RenderPass, nullptr);

        //     m_RenderPass = VK_NULL_HANDLE;
        // }

        // Cleanup Swapchain.
        if (m_Swapchain != VK_NULL_HANDLE) 
        {
            vkDestroySwapchainKHR(VyContext::device(), m_Swapchain, nullptr);
        
            m_Swapchain = VK_NULL_HANDLE;
        }

        m_SwapchainImages.clear();

        // Cleanup Synchronization objects.
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) 
        {
            vkDestroySemaphore(VyContext::device(), m_RenderFinishedSemaphores[i], nullptr);
            vkDestroySemaphore(VyContext::device(), m_ImageAvailableSemaphores[i], nullptr);
            vkDestroyFence    (VyContext::device(), m_InFlightFences[i],           nullptr);
        }

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
                VY_INFO_TAG("VySwapchain", "Selected {} with {}.", 
                    STR_VK_FORMAT(availableFormat.format), 
                    STR_VK_COLOR_SPACE_KHR(availableFormat.colorSpace)
                );

                return availableFormat;
            }
        }

        for (const auto& availableFormat : availableFormats) 
        {
            if (availableFormat.format     == VK_FORMAT_R8G8B8A8_SRGB &&
                availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) 
            {
                VY_INFO_TAG("VySwapchain", "Selected {} with {}.", 
                    STR_VK_FORMAT(availableFormat.format), 
                    STR_VK_COLOR_SPACE_KHR(availableFormat.colorSpace)
                );
                
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
                VY_INFO_TAG("VySwapchain", "Selected {} with {}.", 
                    STR_VK_FORMAT(availableFormat.format), 
                    STR_VK_COLOR_SPACE_KHR(availableFormat.colorSpace)
                );
                
                return availableFormat;
            }
        }
        for (const auto& availableFormat : availableFormats) 
        {
            if (availableFormat.format     == VK_FORMAT_R8G8B8A8_UNORM &&
                availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) 
            { 
                // Still prefer sRGB color space if possible
                VY_INFO_TAG("VySwapchain", "Selected {} with {}.", 
                    STR_VK_FORMAT(availableFormat.format), 
                    STR_VK_COLOR_SPACE_KHR(availableFormat.colorSpace)
                );
                
                return availableFormat;
            }
        }

        VY_INFO_TAG("VySwapchain", "Selected first format available (only one supported): {} with {}.", 
            STR_VK_FORMAT(availableFormats[0].format), 
            STR_VK_COLOR_SPACE_KHR(availableFormats[0].colorSpace)
        );
        
        return availableFormats[0];
    }

    // ---------------------------------------------------------------------------------------------------------------------

    VkPresentModeKHR VySwapchain::chooseSwapPresentMode(const TVector<VkPresentModeKHR> &availablePresentModes) 
    {
        for (const auto& availablePresentMode : availablePresentModes) 
        {
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) 
            {
                VY_INFO_TAG("VySwapchain", "Present mode: Mailbox");
                return availablePresentMode;
            }
        }

#ifdef USE_IMMEDIATE_PRESENT_MODE
		for (const auto& availablePresentMode : availablePresentModes) 
        {
			if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) 
            {
                VY_INFO_TAG("VySwapchain", "Present mode: Immediate");
				return availablePresentMode;
			}
		}
#endif

        VY_INFO_TAG("VySwapchain", "Present mode: V-Sync (FIFO)");
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
                actualExtent.width  = std::max(capabilities.minImageExtent.width,  std::min(capabilities.maxImageExtent.width,  actualExtent.width ));
                actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));
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

#pragma endregion Support
}