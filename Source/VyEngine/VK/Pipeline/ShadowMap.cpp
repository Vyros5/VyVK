#include <VyEngine/VK/Pipeline/ShadowMap.h>

#include <VyEngine/VK/Context.h>

#include <VyEngine/Globals.h>

namespace Vy
{
    VyShadowMap::VyShadowMap(U32 width, U32 height) : 
        m_Width { width  }, 
        m_Height{ height }
    {
        createDepthResources();
        createRenderPass();
        createFramebuffer();
        createSampler();
    }


    VyShadowMap::~VyShadowMap()
    {
        if (m_Framebuffer != VK_NULL_HANDLE)
        {
            vkDestroyFramebuffer(VyContext::device(), m_Framebuffer, nullptr);
        }

        if (m_RenderPass != VK_NULL_HANDLE)
        {
            vkDestroyRenderPass(VyContext::device(), m_RenderPass, nullptr);
        }
    }


    void VyShadowMap::createDepthResources()
    {
        // Create depth image
        m_DepthImage = VyImage::Builder{}
            .setName       ("shadow_map_depth")
            .setImageType  (VK_IMAGE_TYPE_2D)
            .setFormat     (m_DepthFormat)
            .setExtent     (m_Width, m_Height)
            .setLevels     (1)
            .setLayers     (1)
            .setSamples    (VK_SAMPLE_COUNT_1_BIT)
            .setTiling     (VK_IMAGE_TILING_OPTIMAL)
            .setLayout     (VK_IMAGE_LAYOUT_UNDEFINED)
            .setSharing    (VK_SHARING_MODE_EXCLUSIVE)
            .setUsage      (VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
            .setMemoryUsage(VMA_MEMORY_USAGE_AUTO)
            .build();

        // Create image view
        m_DepthImageView = VyImageView::Builder{}
            .setName    ("shadow_map_depth")
            .setViewType(VK_IMAGE_VIEW_TYPE_2D)
            .setFormat  (m_DepthFormat)
            .setAspect  (VK_IMAGE_ASPECT_DEPTH_BIT)
            .setLevels  (0, 1)
            .setLayers  (0, 1)
            .build( m_DepthImage );

        // Transition to READ_ONLY_OPTIMAL initially so it's valid for binding
        m_DepthImage.transitionLayout(VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL);
    }


    void VyShadowMap::createRenderPass()
    {
        VkAttachmentDescription depthAttachment{};
        {
            depthAttachment.format         = m_DepthFormat;
            depthAttachment.samples        = VK_SAMPLE_COUNT_1_BIT;

            depthAttachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
            depthAttachment.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
            
            depthAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            
            depthAttachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
            depthAttachment.finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
        }

        VkAttachmentReference depthAttachmentRef{};
        {
            depthAttachmentRef.attachment = 0;
            depthAttachmentRef.layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        }

        VkSubpassDescription subpass{};
        {
            subpass.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
            subpass.colorAttachmentCount    = 0;
            subpass.pColorAttachments       = nullptr;
            subpass.pDepthStencilAttachment = &depthAttachmentRef;
        }

        // Dependency to ensure depth writes complete before sampling
        TArray<VkSubpassDependency, 2> dependencies{};
        {
            // [ 0 ]
            {
                dependencies[0].srcSubpass      = VK_SUBPASS_EXTERNAL;
                dependencies[0].dstSubpass      = 0;

                dependencies[0].srcStageMask    = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                dependencies[0].dstStageMask    = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
                
                dependencies[0].srcAccessMask   = VK_ACCESS_SHADER_READ_BIT;
                dependencies[0].dstAccessMask   = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                
                dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
            }
            
            // [ 1 ]
            {
                dependencies[1].srcSubpass      = 0;
                dependencies[1].dstSubpass      = VK_SUBPASS_EXTERNAL;

                dependencies[1].srcStageMask    = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
                dependencies[1].dstStageMask    = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                
                dependencies[1].srcAccessMask   = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                dependencies[1].dstAccessMask   = VK_ACCESS_SHADER_READ_BIT;
                
                dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
            }
        }

        VkRenderPassCreateInfo renderPassInfo{ VKInit::renderPassCreateInfo() };
        {
            renderPassInfo.attachmentCount = 1;
            renderPassInfo.pAttachments    = &depthAttachment;

            renderPassInfo.subpassCount    = 1;
            renderPassInfo.pSubpasses      = &subpass;
            
            renderPassInfo.dependencyCount = static_cast<U32>(dependencies.size());
            renderPassInfo.pDependencies   = dependencies.data();
        }

        VK_CHECK_SUCCESS(vkCreateRenderPass(VyContext::device(), &renderPassInfo, nullptr, &m_RenderPass), 
            "Failed to create shadow map render pass!");

        VKDbg::setObjectName(m_RenderPass, "shadow_map_renderpass");
    }


    void VyShadowMap::createFramebuffer()
    {
        VkFramebufferCreateInfo framebufferInfo{ VKInit::framebufferCreateInfo() };
        {
            framebufferInfo.renderPass      = m_RenderPass;

            framebufferInfo.attachmentCount = 1;
            framebufferInfo.pAttachments    = &m_DepthImageView.handle();
            
            framebufferInfo.width           = m_Width;
            framebufferInfo.height          = m_Height;
            
            framebufferInfo.layers          = 1;
        }

        VK_CHECK_SUCCESS(vkCreateFramebuffer(VyContext::device(), &framebufferInfo, nullptr, &m_Framebuffer), 
            "Failed to create shadow map framebuffer!");

        VKDbg::setObjectName(m_Framebuffer, "shadow_map_framebuffer");
    }


    void VyShadowMap::createSampler()
    {
        m_Sampler = VySampler::Builder{}
            .setName         ("shadow_map")
            .setFilters      (VK_FILTER_LINEAR)
            .setMipmapMode   (VK_SAMPLER_MIPMAP_MODE_LINEAR)
            .setWrap         (VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER)
            .setBorder       (VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE) // Outside shadow map = no shadow
			.enableAnisotropy(false, 1.0f)
            .enableCompare   (VK_COMPARE_OP_LESS_OR_EQUAL) // PCF comparison
            .setLodRange     (0.0f, 1.0f) 
            .setMipLodBias   (0.0f)
            .build();
    }


    void VyShadowMap::beginRenderPass(VkCommandBuffer cmdBuffer)
    {
        VkExtent2D extent{ m_Width, m_Height };

        VkClearValue clearValue{};
        {
            clearValue.depthStencil = { 1.0f, 0 };
        }

        VkRenderPassBeginInfo renderPassInfo{ VKInit::renderPassBeginInfo() };
        {
            renderPassInfo.renderPass        = m_RenderPass;
            renderPassInfo.framebuffer       = m_Framebuffer;

            renderPassInfo.renderArea.offset = { 0, 0 };
            renderPassInfo.renderArea.extent = extent;
            
            renderPassInfo.clearValueCount   = 1;
            renderPassInfo.pClearValues      = &clearValue;
        }
            
        vkCmdBeginRenderPass(cmdBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        // Set viewport and scissor rect.
        VKCmd::viewport(cmdBuffer, extent);
        VKCmd::scissor (cmdBuffer, extent);
    }


    void VyShadowMap::endRenderPass(VkCommandBuffer cmdBuffer)
    {
        vkCmdEndRenderPass(cmdBuffer);
    }
}






namespace Vy
{
    VyCubeShadowMap::VyCubeShadowMap(U32 size) : 
        m_Size{ size }
    {
        createDepthResources();
        createRenderPass();
        createFramebuffers();
        createSampler();
    }


    VyCubeShadowMap::~VyCubeShadowMap()
    {
        for (int i = 0; i < 6; i++)
        {
            vkDestroyFramebuffer(VyContext::device(), m_Framebuffers[ i ], nullptr);
        }

        vkDestroyRenderPass(VyContext::device(), m_RenderPass, nullptr);
    }


    void VyCubeShadowMap::createDepthResources()
    {
        // Create cube map image
        m_DepthImage = VyImage::Builder{}
            .setName       ("cube_shadow_map_depth")
            .setImageType  (VK_IMAGE_TYPE_2D)
            .setFormat     (m_DepthFormat)
            .setExtent     (m_Size, m_Size)
            .setLevels     (1)
            .setLayers     (6) // 6 faces for cubemap
            .setSamples    (VK_SAMPLE_COUNT_1_BIT)
            .setTiling     (VK_IMAGE_TILING_OPTIMAL)
            .setLayout     (VK_IMAGE_LAYOUT_UNDEFINED)
            .setSharing    (VK_SHARING_MODE_EXCLUSIVE)
            .setUsage      (VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
            .setFlags      (VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT) // cubemap image
            .setMemoryUsage(VMA_MEMORY_USAGE_AUTO)
            .build();

        // Create cube image view (for sampling in shader)
        m_CubeImageView = VyImageView::Builder{}
            .setName    ("cube_shadow_map_depth")
            .setViewType(VK_IMAGE_VIEW_TYPE_CUBE)
            .setFormat  (m_DepthFormat)
            .setAspect  (VK_IMAGE_ASPECT_DEPTH_BIT)
            .setLevels  (0, 1)
            .setLayers  (0, 6) // 6 layers for cubemap
            .build( m_DepthImage );

        // Create individual face image views (for rendering)
        for (int f = 0; f < 6; f++)
        {
            m_FaceImageViews[ f ] = VyImageView::Builder{}
                .setName    (std::format("cube_shadow_map_face_{}", f))
                .setViewType(VK_IMAGE_VIEW_TYPE_2D)
                .setFormat  (m_DepthFormat)
                .setAspect  (VK_IMAGE_ASPECT_DEPTH_BIT)
                .setLevels  (0, 1)
                .setLayers  (f, 1) // 1 layer per face
                .build( m_DepthImage );
        }

        // Transition to READ_ONLY_OPTIMAL initially so it's valid for binding
        m_DepthImage.transitionLayout(VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL);
    }


    void VyCubeShadowMap::createRenderPass()
    {
        VkAttachmentDescription depthAttachment{};
        {
            depthAttachment.format         = m_DepthFormat;
            depthAttachment.samples        = VK_SAMPLE_COUNT_1_BIT;

            depthAttachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
            depthAttachment.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
            
            depthAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            
            // Since we transition per-layer before and after, use same layout
            depthAttachment.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            depthAttachment.finalLayout   = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        }

        VkAttachmentReference depthAttachmentRef{};
        {
            depthAttachmentRef.attachment = 0;
            depthAttachmentRef.layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        }

        VkSubpassDescription subpass{};
        {
            subpass.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
            subpass.colorAttachmentCount    = 0;
            subpass.pDepthStencilAttachment = &depthAttachmentRef;
        }

        // Add subpass dependencies for proper synchronization,
        TArray<VkSubpassDependency, 2> dependencies;
        {
            // [ 0 ]
            {
                dependencies[0].srcSubpass      = VK_SUBPASS_EXTERNAL;
                dependencies[0].dstSubpass      = 0;
                
                dependencies[0].srcStageMask    = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                dependencies[0].dstStageMask    = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
                
                dependencies[0].srcAccessMask   = VK_ACCESS_SHADER_READ_BIT;
                dependencies[0].dstAccessMask   = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                
                dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
            }

            // [ 1 ]
            {
                dependencies[1].srcSubpass      = 0;
                dependencies[1].dstSubpass      = VK_SUBPASS_EXTERNAL;
                
                dependencies[1].srcStageMask    = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
                dependencies[1].dstStageMask    = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                
                dependencies[1].srcAccessMask   = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                dependencies[1].dstAccessMask   = VK_ACCESS_SHADER_READ_BIT;
                
                dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
            }
        }

        VkRenderPassCreateInfo renderPassInfo{ VKInit::renderPassCreateInfo() };
        {
            renderPassInfo.attachmentCount = 1;
            renderPassInfo.pAttachments    = &depthAttachment;

            renderPassInfo.subpassCount    = 1;
            renderPassInfo.pSubpasses      = &subpass;
            
            renderPassInfo.dependencyCount = static_cast<U32>(dependencies.size());
            renderPassInfo.pDependencies   = dependencies.data();
        }

        VK_CHECK_SUCCESS(vkCreateRenderPass(VyContext::device(), &renderPassInfo, nullptr, &m_RenderPass), 
            "Failed to create cube shadow map render pass!");

        VKDbg::setObjectName(m_RenderPass, "cube_shadow_map_renderpass");
    }


    void VyCubeShadowMap::createFramebuffers()
    {
        for (int i = 0; i < 6; i++)
        {
            VkFramebufferCreateInfo framebufferInfo{ VKInit::framebufferCreateInfo() };
            {
                framebufferInfo.renderPass      = m_RenderPass;

                framebufferInfo.attachmentCount = 1;
                framebufferInfo.pAttachments    = &m_FaceImageViews[i].handle();
                
                framebufferInfo.width           = m_Size;
                framebufferInfo.height          = m_Size;
                
                framebufferInfo.layers          = 1;
            }

            VK_CHECK_SUCCESS(vkCreateFramebuffer(VyContext::device(), &framebufferInfo, nullptr, &m_Framebuffers[i]), 
                "Failed to create cube shadow map framebuffer!");

            TString name = std::format("shadow_map_face_framebuffer_{}", i);

            VKDbg::setObjectName(m_Framebuffers[ i ], name.c_str());
        }
    }


    void VyCubeShadowMap::createSampler()
    {
        m_Sampler = VySampler::Builder{}
            .setName         ("cube_shadow_map")
            .setFilters      (VK_FILTER_LINEAR)
            .setMipmapMode   (VK_SAMPLER_MIPMAP_MODE_LINEAR)
            .setWrap         (VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE)
            .setBorder       (VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE)
			.enableAnisotropy(false, 1.0f)
            // No comparison - // We'll compare manually in shader.
            .setLodRange     (0.0f, 1.0f) 
            .setMipLodBias   (0.0f)
            .build();
    }


    Mat4 VyCubeShadowMap::faceViewMatrix(const Vec3& lightPos, int face)
    {
        // Face directions: +X, -X, +Y, -Y, +Z, -Z
        static const Vec3 targets[6] = {
            Vec3( 1.0f,  0.0f,  0.0f), // +X
            Vec3(-1.0f,  0.0f,  0.0f), // -X
            Vec3( 0.0f,  1.0f,  0.0f), // +Y
            Vec3( 0.0f, -1.0f,  0.0f), // -Y
            Vec3( 0.0f,  0.0f,  1.0f), // +Z
            Vec3( 0.0f,  0.0f, -1.0f), // -Z
        };

        static const Vec3 ups[6] = {
            Vec3( 0.0f, -1.0f,  0.0f), // +X
            Vec3( 0.0f, -1.0f,  0.0f), // -X
            Vec3( 0.0f,  0.0f,  1.0f), // +Y
            Vec3( 0.0f,  0.0f, -1.0f), // -Y
            Vec3( 0.0f, -1.0f,  0.0f), // +Z
            Vec3( 0.0f, -1.0f,  0.0f), // -Z
        };

        return glm::lookAt(lightPos, lightPos + targets[face], ups[face]);
    }


    Mat4 VyCubeShadowMap::projectionMatrix(float nearPlane, float farPlane)
    {
        // 90 degree FOV for cube faces
        Mat4 proj = glm::perspective(glm::radians(90.0f), 1.0f, nearPlane, farPlane);
        
        // Vulkan Y flip
        proj[1][1] *= -1;

        return proj;
    }


    void VyCubeShadowMap::beginRenderPass(VkCommandBuffer cmdBuffer, int face)
    {
        VkExtent2D extent{ m_Size, m_Size };

        VkClearValue clearValue{};
        {
            clearValue.depthStencil = { 1.0f, 0 };
        }

        VkRenderPassBeginInfo renderPassInfo{ VKInit::renderPassBeginInfo() };
        {
            renderPassInfo.renderPass        = m_RenderPass;
            renderPassInfo.framebuffer       = m_Framebuffers[ face ];

            renderPassInfo.renderArea.offset = { 0, 0 };
            renderPassInfo.renderArea.extent = extent;

            renderPassInfo.clearValueCount = 1;
            renderPassInfo.pClearValues    = &clearValue;
        }

        vkCmdBeginRenderPass(cmdBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        // Set viewport and scissor rect.
        VKCmd::viewport(cmdBuffer, extent);
        VKCmd::scissor (cmdBuffer, extent);
    }


    void VyCubeShadowMap::endRenderPass(VkCommandBuffer cmdBuffer)
    {
        vkCmdEndRenderPass(cmdBuffer);
    }


    void VyCubeShadowMap::transitionToAttachmentLayout(VkCommandBuffer cmdBuffer)
    {
        // Transition ALL 6 layers from shader read (or undefined) to attachment optimal
        VkImageMemoryBarrier barrier{};
        barrier.sType                           = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout                       = VK_IMAGE_LAYOUT_UNDEFINED; // Don't care about previous contents
        barrier.newLayout                       = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        barrier.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
        barrier.image                           = m_DepthImage.handle();
        barrier.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_DEPTH_BIT;
        barrier.subresourceRange.baseMipLevel   = 0;
        barrier.subresourceRange.levelCount     = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount     = 6; // All 6 faces

        barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        vkCmdPipelineBarrier(cmdBuffer,
                            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                            VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
                            0,
                            0,
                            nullptr,
                            0,
                            nullptr,
                            1,
                            &barrier);
    }


    void VyCubeShadowMap::transitionToShaderReadLayout(VkCommandBuffer cmdBuffer)
    {
        // Transition ALL 6 layers from attachment optimal to shader read
        VkImageMemoryBarrier barrier{};
        barrier.sType                           = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout                       = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        barrier.newLayout                       = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
        barrier.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
        barrier.image                           = m_DepthImage.handle();
        barrier.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_DEPTH_BIT;
        barrier.subresourceRange.baseMipLevel   = 0;
        barrier.subresourceRange.levelCount     = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount     = 6; // All 6 faces

        barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(cmdBuffer,
                            VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
                            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                            0,
                            0,
                            nullptr,
                            0,
                            nullptr,
                            1,
                            &barrier);
    }
}