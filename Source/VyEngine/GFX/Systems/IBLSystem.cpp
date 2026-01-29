// #include <VyEngine/GFX/Systems/IBLSystem.h>

// #include <VyEngine/VK/Context.h>
// #include <VyEngine/Globals.h>

// namespace Vy
// {
//     struct PrefilterPushBlock
//     {
//         Mat4  MVP;
//         int   FaceIndex;
//         float Roughness;
//         U32   SampleCount;
//     };

//     struct IrradiancePushBlock
//     {
//         Mat4  MVP;
//         int   FaceIndex;
//         float SampleDelta;
//     };


//     VyIBLSystem::VyIBLSystem() 
//     {
//     }


//     VyIBLSystem::~VyIBLSystem()
//     {
//         cleanup();
//     }


//     void VyIBLSystem::cleanup()
//     {
//         // Destroy Irradiance Resources
//         if (m_IrradianceRenderPass)
//         {
//             vkDestroyRenderPass(VyContext::device(), m_IrradianceRenderPass, nullptr);

//             m_IrradianceRenderPass = VK_NULL_HANDLE;
//         }

//         // Destroy Prefilter Resources
//         if (m_PrefilterRenderPass)
//         {
//             vkDestroyRenderPass(VyContext::device(), m_PrefilterRenderPass, nullptr);

//             m_PrefilterRenderPass = VK_NULL_HANDLE;
//         }
//     }


//     VkDescriptorImageInfo VyIBLSystem::irradianceDescriptorImageInfo() const
//     {
//         return VkDescriptorImageInfo{
//             .sampler     = m_IrradianceSampler  .handle(),
//             .imageView   = m_IrradianceImageView.handle(),
//             .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
//         };
//     }


//     VkDescriptorImageInfo VyIBLSystem::prefilteredDescriptorImageInfo() const
//     {
//         return VkDescriptorImageInfo{
//             .sampler     = m_PrefilteredSampler  .handle(),
//             .imageView   = m_PrefilteredImageView.handle(),
//             .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
//         };
//     }


//     VkDescriptorImageInfo VyIBLSystem::brdfLUTDescriptorImageInfo() const
//     {
//         return VkDescriptorImageInfo{
//             .sampler     = m_BrdfLUTSampler  .handle(),
//             .imageView   = m_BrdfLUTImageView.handle(),
//             .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
//         };
//     }


//     void VyIBLSystem::updateSettings(const Settings& settings)
//     {
//         m_Settings = settings;
//     }


//     void VyIBLSystem::requestRegeneration(const Settings& newSettings, VySkybox& skybox)
//     {
//         m_NextSettings          = newSettings;
//         m_NextSkybox            = &skybox;
//         m_RegenerationRequested = true;
//     }


//     void VyIBLSystem::update()
//     {
//         if (m_RegenerationRequested && m_NextSkybox)
//         {
//             // Wait for device idle to ensure no resources are in use.
//             VyContext::waitIdle();

//             // Update settings.
//             m_Settings = m_NextSettings;

//             // Regenerate
//             generateFromSkybox( *m_NextSkybox );

//             // Reset flag
//             m_RegenerationRequested = false;
//             m_NextSkybox            = nullptr;
//         }
//     }


//     void VyIBLSystem::generateFromSkybox(VySkybox& skybox)
//     {
//         if (m_Generated)
//         {
//             cleanup();
//         }

//         createIrradianceMap();
//         createPrefilteredEnvMap();
//         createBRDFLUT();

//         createIrradianceResources();
//         generateIrradianceMap(skybox);

//         createPrefilterResources();
//         generatePrefilteredEnvMap(skybox);

//         createBRDFResources();
//         generateBRDFLUT();

//         m_Generated = true;

//         // Wait for everything to finish
//         VyContext::waitIdle();
//     }


//     // void VyIBLSystem::generateFromProcedural(VySkyboxRenderSystem& skyRenderSystem, const SkyboxSettings& settings)
//     // {
//     //     // 1. Create temporary Skybox (512x512)
//     //     VySkybox tempSkybox( 512 );
//     //
//     //     // 2. Create RenderPass
//     //     VkRenderPass renderPass;
//     //     {
//     //         VkAttachmentDescription attachment{};
//     //         {
//     //             attachment.format         = VK_FORMAT_R8G8B8A8_UNORM;
//     //             attachment.samples        = VK_SAMPLE_COUNT_1_BIT;
//     //             attachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
//     //             attachment.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
//     //             attachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
//     //             attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
//     //             attachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
//     //             attachment.finalLayout    = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
//     //         }
//     //
//     //         VkAttachmentReference colorReference{};
//     //         {
//     //             colorReference.attachment = 0;
//     //             colorReference.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
//     //         }
//     //
//     //         VkSubpassDescription subpass{};
//     //         {
//     //             subpass.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;
//     //             subpass.colorAttachmentCount = 1;
//     //             subpass.pColorAttachments    = &colorReference;
//     //         }
//     //
//     //         VkSubpassDependency dependency{};
//     //         {
//     //             dependency.srcSubpass    = VK_SUBPASS_EXTERNAL;
//     //             dependency.dstSubpass    = 0;
//     //             dependency.srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
//     //             dependency.srcAccessMask = 0;
//     //             dependency.dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
//     //             dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
//     //         }
//     //
//     //         VkRenderPassCreateInfo renderPassInfo{ VKInit::renderPassCreateInfo()};
//     //         {
//     //             renderPassInfo.attachmentCount = 1;
//     //             renderPassInfo.pAttachments    = &attachment;
//     //             renderPassInfo.subpassCount    = 1;
//     //             renderPassInfo.pSubpasses      = &subpass;
//     //             renderPassInfo.dependencyCount = 1;
//     //             renderPassInfo.pDependencies   = &dependency;
//     //         }
//     //
//     //         VK_CHECK_SUCCESS(vkCreateRenderPass(VyContext::device(), &renderPassInfo, nullptr, &renderPass), 
//     //             "Failed to create skybox generation render pass!");
//     //
//     //         VKDbg::setObjectName(VK_OBJECT_TYPE_RENDER_PASS, (U64)renderPass, "sky_gen_renderpass");
//     //     }
//     //
//     //     // 3. Create Framebuffers (one for each face)
//     //     TVector<VkImageView>   faceViews   ( 6 );
//     //     TVector<VkFramebuffer> framebuffers( 6 );
//     //
//     //     for (int i = 0; i < 6; i++)
//     //     {
//     //         VkImageViewCreateInfo viewInfo{};
//     //         viewInfo.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
//     //         viewInfo.image                           = tempSkybox.image();
//     //         viewInfo.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
//     //         viewInfo.format                          = VK_FORMAT_R8G8B8A8_UNORM;
//     //         viewInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
//     //         viewInfo.subresourceRange.baseMipLevel   = 0;
//     //         viewInfo.subresourceRange.levelCount     = 1;
//     //         viewInfo.subresourceRange.baseArrayLayer = i;
//     //         viewInfo.subresourceRange.layerCount     = 1;
//     //
//     //         vkCreateImageView(VyContext::device(), &viewInfo, nullptr, &faceViews[ i ]);
//     //
//     //         TString viewName = std::format("sky_gen_face_{}_image_view", i);
//     //         VKDbg::setObjectName(VK_OBJECT_TYPE_IMAGE_VIEW, (U64)faceViews.at( i ), viewName.c_str());
//     //
//     //         VkFramebufferCreateInfo framebufferInfo{};
//     //         framebufferInfo.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
//     //         framebufferInfo.renderPass      = renderPass;
//     //         framebufferInfo.attachmentCount = 1;
//     //         framebufferInfo.pAttachments    = &faceViews[ i ];
//     //         framebufferInfo.width           = 512;
//     //         framebufferInfo.height          = 512;
//     //         framebufferInfo.layers          = 1;
//     //
//     //         vkCreateFramebuffer(VyContext::device(), &framebufferInfo, nullptr, &framebuffers[ i ]);
//     //
//     //         TString fbName = std::format("sky_gen_face_{}_framebuffer", i);
//     //         VKDbg::setObjectName(VK_OBJECT_TYPE_FRAMEBUFFER, (U64)framebuffers.at( i ), fbName.c_str());
//     //     }
//     //
//     //     // 4. Render
//     //     VkCommandBuffer cmdBuffer = VyContext::beginCommands();
//     //
//     //     VyCamera cam;
//     //     cam.setPerspectiveParams(glm::radians(90.0f), 0.1f, 10.0f);
//     //     cam.setPerspective(1.0f);
//     //
//     //     // Standard cubemap faces
//     //     struct Face
//     //     {
//     //         Vec3 Target;
//     //         Vec3 Up;
//     //     };
//     //
//     //     Face faces[] = {
//     //         {{ 1.0f,  0.0f,  0.0f }, { 0.0f, -1.0f,  0.0f }}, // +X
//     //         {{-1.0f,  0.0f,  0.0f }, { 0.0f, -1.0f,  0.0f }}, // -X
//     //         {{ 0.0f, -1.0f,  0.0f }, { 0.0f,  0.0f, -1.0f }}, // +Y (Top) - Vulkan Y down, so -Y is up? No, +Y is down.
//     //         {{ 0.0f,  1.0f,  0.0f }, { 0.0f,  0.0f,  1.0f }}, // -Y (Bottom)
//     //         {{ 0.0f,  0.0f,  1.0f }, { 0.0f, -1.0f,  0.0f }}, // +Z
//     //         {{ 0.0f,  0.0f, -1.0f }, { 0.0f, -1.0f,  0.0f }}  // -Z
//     //     };
//     //     // Standard cubemap: +Y is Top. In Vulkan, if we render to an image, +Y is down in UV space.
//     //     // But 3D direction +Y is usually Up in world space. Let's stick to standard lookAt.
//     //
//     //     // Correct faces for Vulkan coordinate system (Y down)
//     //     // Actually, we want to render the world as seen from center.
//     //     // World +Y is Up.
//     //     // Camera Up should be -Y if we want the image to be upright?
//     //     // No, Camera Up is (0, -1, 0) means "Up" in camera space (which is -Y) aligns with World -Y?
//     //     // Let's use the standard GL cubemap targets, but flip Y in projection if needed.
//     //     // I already flipped Y in projection: captureProjection[1][1] *= -1;
//     //     // Wait, I didn't do that here yet.
//     //
//     //     // cam.setPerspectiveProjection handles the Vulkan clip space (Y down) if implemented correctly?
//     //     // Usually setPerspectiveProjection creates a standard GL matrix.
//     //     // If so, we need to flip Y.
//     //     // But let's check Camera::setPerspectiveProjection.
//     //
//     //     for (int i = 0; i < 6; ++i)
//     //     {
//     //         cam.setViewTarget( Vec3(0.0f), faces[ i ].Target, faces[ i ].Up );
//     //
//     //         VkClearValue clearValue = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
//     //
//     //         VkRenderPassBeginInfo renderPassInfo{ VKInit::renderPassBeginInfo() };
//     //         {
//     //             renderPassInfo.renderPass        = renderPass;
//     //             renderPassInfo.framebuffer       = framebuffers[ i ];
//     //             renderPassInfo.renderArea.offset = { 0, 0 };
//     //             renderPassInfo.renderArea.extent = { 512, 512 };
//     //
//     //             renderPassInfo.clearValueCount   = 1;
//     //             renderPassInfo.pClearValues      = &clearValue;
//     //         }
//     //
//     //         vkCmdBeginRenderPass(cmdBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
//     //         {
//     //             VyFrameInfo frameInfo{
//     //                 .FrameIndex    = 0, 
//     //                 .FrameTime     = 0.0f, 
//     //                 .CommandBuffer = cmdBuffer, 
//     //                 .Camera        = cam
//     //             };
//     //
//     //             skyRenderSystem.render( frameInfo, nullptr, settings );
//     //         }
//     //         vkCmdEndRenderPass(cmdBuffer);
//     //     }
//     //
//     //     VyContext::endCommands(cmdBuffer);
//     //
//     //     // Cleanup render pass resources
//     //     vkDestroyRenderPass(VyContext::device(), renderPass, nullptr);
//     //
//     //     for (auto fb : framebuffers)
//     //     {
//     //         vkDestroyFramebuffer(VyContext::device(), fb, nullptr);
//     //     }
//     //
//     //     for (auto view : faceViews)
//     //     {
//     //         vkDestroyImageView(VyContext::device(), view, nullptr);
//     //     }
//     //
//     //     // 5. Generate IBL maps
//     //     generateFromSkybox( tempSkybox );
//     // }


//     // Helper to transition image layout
//     void transitionImageLayoutHelper(
//         VkImage       image,
//         VkFormat      format,
//         VkImageLayout oldLayout,
//         VkImageLayout newLayout,
//         U32           mipLevels,
//         U32           layerCount = 1)
//     {
//         VkCommandBuffer cmdBuffer = VyContext::beginCommands();
//         {
//             VkPipelineStageFlags srcStage;
//             VkPipelineStageFlags dstStage;

//             VkImageMemoryBarrier barrier{ VKInit::imageMemoryBarrier() };
//             {
//                 barrier.image                           = image;

//                 barrier.oldLayout                       = oldLayout;
//                 barrier.newLayout                       = newLayout;

//                 barrier.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
//                 barrier.subresourceRange.baseMipLevel   = 0;
//                 barrier.subresourceRange.levelCount     = mipLevels;
//                 barrier.subresourceRange.baseArrayLayer = 0;
//                 barrier.subresourceRange.layerCount     = layerCount;

//                 if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && 
//                     newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
//                 {
//                     barrier.srcAccessMask = 0;
//                     barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

//                     srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
//                     dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
//                 }
//                 else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && 
//                          newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
//                 {
//                     barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
//                     barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

//                     srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
//                     dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
//                 }
//                 else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && 
//                          newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
//                 {
//                     barrier.srcAccessMask = 0;
//                     barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

//                     srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
//                     dstStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
//                 }
//                 else if (oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && 
//                          newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
//                 {
//                     barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
//                     barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

//                     srcStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
//                     dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
//                 }
//                 else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && 
//                          newLayout == VK_IMAGE_LAYOUT_GENERAL)
//                 {
//                     barrier.srcAccessMask = 0;
//                     barrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;

//                     srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
//                     dstStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
//                 }
//                 else if (oldLayout == VK_IMAGE_LAYOUT_GENERAL && 
//                          newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
//                 {
//                     barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
//                     barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

//                     srcStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
//                     dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
//                 }
//                 else
//                 {
//                     VY_THROW_INVALID_ARGUMENT("Unsupported layout transition!");
//                 }
//             }

//             vkCmdPipelineBarrier(cmdBuffer, 
//                 srcStage, 
//                 dstStage, 
//                 0, 
//                 0, nullptr, 
//                 0, nullptr, 
//                 1, &barrier
//             );
//         }
//         VyContext::endCommands(cmdBuffer);
//     }


// // =========================================================================================================================
// #pragma region [ RESOURCES ]
// // =========================================================================================================================

//     void VyIBLSystem::createIrradianceMap()
//     {
//         m_IrradianceImage = VyImage::Builder{}
//             .setName       ("ibl_irradiance")
//             .setImageType  (VK_IMAGE_TYPE_2D)
//             .setFormat     (VK_FORMAT_R32G32B32A32_SFLOAT)
//             .setExtent     (static_cast<U32>(m_Settings.IrradianceSize), static_cast<U32>(m_Settings.IrradianceSize))
//             .setLevels     (1)
// 			.setLayers     (6)
// 			.setSamples    (VK_SAMPLE_COUNT_1_BIT)
//             .setTiling     (VK_IMAGE_TILING_OPTIMAL)
// 			.setLayout     (VK_IMAGE_LAYOUT_UNDEFINED)
//             .setUsage      (VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
//             .setFlags      (VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT)
//             .setMemoryUsage(VMA_MEMORY_USAGE_AUTO)
//             .build();

//         m_IrradianceImageView = VyImageView::Builder{}
//             .setName    ("ibl_irradiance")
//             .setViewType(VK_IMAGE_VIEW_TYPE_CUBE)
//             .setFormat  (VK_FORMAT_R32G32B32A32_SFLOAT)
//             .setAspect  (VK_IMAGE_ASPECT_COLOR_BIT)
//             .setLevels  (0, 1)
//             .setLayers  (0, 6)
//             .build( m_IrradianceImage );

//         m_IrradianceSampler = VySampler::Builder{}
//             .setName         ("ibl_irradiance")
//             .setFilters      (VK_FILTER_LINEAR)
//             .setMipmapMode   (VK_SAMPLER_MIPMAP_MODE_LINEAR)
//             .setWrap         (VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE)
//             .setBorder       (VK_BORDER_COLOR_INT_OPAQUE_BLACK)
// 			.enableAnisotropy(true)
//             .setLodRange     (0.0f, 1.0f)
//             .setMipLodBias   (0.0f)
//             .build();

//         // Transition to color attachment optimal
//         transitionImageLayoutHelper(
//             m_IrradianceImage,
//             VK_FORMAT_R32G32B32A32_SFLOAT,
//             VK_IMAGE_LAYOUT_UNDEFINED,
//             VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
//             1,
//             6
//         );
//     }

//     // =====================================================================================================================

//     void VyIBLSystem::createPrefilteredEnvMap()
//     {
//         m_PrefilteredImage = VyImage::Builder{}
//             .setName       ("ibl_prefiltered")
//             .setImageType  (VK_IMAGE_TYPE_2D)
//             .setFormat     (VK_FORMAT_R16G16B16A16_SFLOAT)
//             .setExtent     (static_cast<U32>(m_Settings.PrefilterSize), static_cast<U32>(m_Settings.PrefilterSize))
//             .setLevels     (m_Settings.PrefilterMipLevels)
// 			.setLayers     (6)
// 			.setSamples    (VK_SAMPLE_COUNT_1_BIT)
//             .setTiling     (VK_IMAGE_TILING_OPTIMAL)
// 			.setLayout     (VK_IMAGE_LAYOUT_UNDEFINED)
//             .setUsage      (VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
//             .setFlags      (VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT)
//             .setMemoryUsage(VMA_MEMORY_USAGE_AUTO)
//             .build();

//         m_PrefilteredImageView = VyImageView::Builder{}
//             .setName    ("ibl_prefiltered")
//             .setViewType(VK_IMAGE_VIEW_TYPE_CUBE)
//             .setFormat  (VK_FORMAT_R16G16B16A16_SFLOAT)
//             .setAspect  (VK_IMAGE_ASPECT_COLOR_BIT)
//             .setLevels  (0, m_Settings.PrefilterMipLevels)
//             .setLayers  (0, 6)
//             .build( m_PrefilteredImage );

//         m_PrefilteredSampler = VySampler::Builder{}
//             .setName         ("ibl_prefiltered")
//             .setFilters      (VK_FILTER_LINEAR)
//             .setMipmapMode   (VK_SAMPLER_MIPMAP_MODE_LINEAR)
//             .setWrap         (VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE)
//             .setBorder       (VK_BORDER_COLOR_INT_OPAQUE_BLACK)
// 			.enableAnisotropy(true)
//             .setLodRange     (0.0f, static_cast<float>(m_Settings.PrefilterMipLevels))
//             .setMipLodBias   (0.0f)
//             .build();

//         // Transition to color attachment optimal
//         transitionImageLayoutHelper(
//             m_PrefilteredImage,
//             VK_FORMAT_R16G16B16A16_SFLOAT,
//             VK_IMAGE_LAYOUT_UNDEFINED,
//             VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
//             m_Settings.PrefilterMipLevels,
//             6
//         );
//     }

//     // =====================================================================================================================

//     void VyIBLSystem::createBRDFLUT()
//     {
//         m_BrdfLUTImage = VyImage::Builder{}
//             .setName       ("ibl_brdflut")
//             .setImageType  (VK_IMAGE_TYPE_2D)
//             .setFormat     (VK_FORMAT_R16G16_SFLOAT)
//             .setExtent     (static_cast<U32>(m_Settings.BrdfLUTSize), static_cast<U32>(m_Settings.BrdfLUTSize))
//             .setLevels     (1)
// 			.setLayers     (1)
// 			.setSamples    (VK_SAMPLE_COUNT_1_BIT)
//             .setTiling     (VK_IMAGE_TILING_OPTIMAL)
// 			.setLayout     (VK_IMAGE_LAYOUT_UNDEFINED)
//             .setUsage      (VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
//             .setMemoryUsage(VMA_MEMORY_USAGE_AUTO)
//             .build();

//         m_BrdfLUTImageView = VyImageView::Builder{}
//             .setName    ("ibl_brdflut")
//             .setViewType(VK_IMAGE_VIEW_TYPE_2D)
//             .setFormat  (VK_FORMAT_R16G16_SFLOAT)
//             .setAspect  (VK_IMAGE_ASPECT_COLOR_BIT)
//             .setLevels  (0, 1)
//             .setLayers  (0, 1)
//             .build( m_BrdfLUTImage );

//         m_BrdfLUTSampler = VySampler::Builder{}
//             .setName         ("ibl_brdflut")
//             .setFilters      (VK_FILTER_LINEAR)
//             .setMipmapMode   (VK_SAMPLER_MIPMAP_MODE_LINEAR)
//             .setWrap         (VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE)
//             .setBorder       (VK_BORDER_COLOR_INT_OPAQUE_BLACK)
// 			.enableAnisotropy(true)
//             .setLodRange     (0.0f, 1.0f)
//             .setMipLodBias   (0.0f)
//             .build();

//         // Transition to general layout for compute shader storage
//         transitionImageLayoutHelper(
//             m_BrdfLUTImage, 
//             VK_FORMAT_R16G16_SFLOAT, 
//             VK_IMAGE_LAYOUT_UNDEFINED, 
//             VK_IMAGE_LAYOUT_GENERAL,
//             1
//         );
//     }

// #pragma endregion RESOURCES


// // =========================================================================================================================
// #pragma region [ IRRADIANCE ]
// // =========================================================================================================================

//     void VyIBLSystem::createIrradianceResources()
//     {
//         // Color attachment
//         VkAttachmentDescription attachment{ VKInit::attachmentDescription() };
//         {
//             attachment.format         = VK_FORMAT_R32G32B32A32_SFLOAT;
//             attachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
//             attachment.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
//             attachment.initialLayout  = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
//             attachment.finalLayout    = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
//         }

//         VkAttachmentReference colorAttachmentRef{};
//         {
//             colorAttachmentRef.attachment = 0;
//             colorAttachmentRef.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
//         }

//         VkSubpassDescription subpass{};
//         {
//             subpass.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;
//             subpass.colorAttachmentCount = 1;
//             subpass.pColorAttachments    = &colorAttachmentRef;
//         }

//         VkRenderPassCreateInfo renderPassInfo{ VKInit::renderPassCreateInfo() };
//         {
//             renderPassInfo.attachmentCount = 1;
//             renderPassInfo.pAttachments    = &attachment;

//             renderPassInfo.subpassCount    = 1;
//             renderPassInfo.pSubpasses      = &subpass;
//         }

//         VK_CHECK_SUCCESS(vkCreateRenderPass(VyContext::device(), &renderPassInfo, nullptr, &m_IrradianceRenderPass), 
//             "Failed to create irradiance render pass!");

//         VKDbg::setObjectName(VK_OBJECT_TYPE_RENDER_PASS, (U64)m_IrradianceRenderPass, "irradiance_renderpass");

//         // Descriptor Set Layout
//         m_IrradianceDescSetLayout = VyDescriptorSetLayout::Builder{}
//             .addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
//             .buildPtr();

//         // Pipeline
//         m_IrradiancePipeline = VyPipeline::GraphicsBuilder{}
//             .setName               ("irradiance")
//             .addDescriptorSetLayout(m_IrradianceDescSetLayout->handle())
//             .addPushConstantRange  (VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(IrradiancePushBlock))
//             .addShaderStage        (VK_SHADER_STAGE_VERTEX_BIT,   "IrradianceConvolution.vert.spv")
//             .addShaderStage        (VK_SHADER_STAGE_FRAGMENT_BIT, "IrradianceConvolution.frag.spv")
//             .setCullMode           (VK_CULL_MODE_NONE) // No culling for full screen quad/cube
//             .addColorAttachment    (VK_FORMAT_R32G32B32A32_SFLOAT)
//             // .setDepthAttachment(VK_FORMAT_D32_SFLOAT)
//             // No vertex input needed (generated in shader)
//             .clearVertexDescriptions() // Clear default vertex bindings and attributes.
//             .setRenderPass(m_IrradianceRenderPass)
//             .build();

//         // Descriptor Pool
//         m_IrradianceDescPool = VyDescriptorPool::Builder{}
//             .setMaxSets (1)
//             .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1)
//             .buildPtr();

//         // Allocate Descriptor Set
//         if (!m_IrradianceDescPool->allocate( m_IrradianceDescSetLayout->handle(), m_IrradianceDescSet ))
//         {
//             VY_THROW_RUNTIME_ERROR("Failed to allocate irradiance descriptor set!");
//         }
//     }

//     // =====================================================================================================================

//     void VyIBLSystem::generateIrradianceMap(VySkybox& skybox)
//     {
//         // Combined image sampler.
//         VkDescriptorImageInfo imageInfo = skybox.descriptorImageInfo();
        
//         // Update descriptor set
//         VyDescriptorWriter{ *m_IrradianceDescSetLayout, *m_IrradianceDescPool }
//             .writeImage( 0, &imageInfo )
//             .update( m_IrradianceDescSet );

//         Mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
//         Mat4 captureViews[]    = {
//             glm::lookAt(Vec3(0.0f, 0.0f, 0.0f), Vec3( 1.0f,  0.0f,  0.0f), Vec3( 0.0f, -1.0f,  0.0f)),
//             glm::lookAt(Vec3(0.0f, 0.0f, 0.0f), Vec3(-1.0f,  0.0f,  0.0f), Vec3( 0.0f, -1.0f,  0.0f)),
//             glm::lookAt(Vec3(0.0f, 0.0f, 0.0f), Vec3( 0.0f,  1.0f,  0.0f), Vec3( 0.0f,  0.0f,  1.0f)), // Top
//             glm::lookAt(Vec3(0.0f, 0.0f, 0.0f), Vec3( 0.0f, -1.0f,  0.0f), Vec3( 0.0f,  0.0f, -1.0f)), // Bottom
//             glm::lookAt(Vec3(0.0f, 0.0f, 0.0f), Vec3( 0.0f,  0.0f,  1.0f), Vec3( 0.0f, -1.0f,  0.0f)),
//             glm::lookAt(Vec3(0.0f, 0.0f, 0.0f), Vec3( 0.0f,  0.0f, -1.0f), Vec3( 0.0f, -1.0f,  0.0f))
//         };

//         TVector<VkFramebuffer> framebuffers;
//         TVector<VkImageView>   imageViews;

//         VkCommandBuffer cmdBuffer = VyContext::beginCommands();
//         {
//             for (int i = 0; i < 6; ++i)
//             {
//                 TString faceName = std::format("irradiance_map_face_{}", i);

//                 // Create view for this face
//                 VyImageView faceView = VyImageView::Builder{}
//                     .setName    (faceName)
//                     .setViewType(VK_IMAGE_VIEW_TYPE_2D)
//                     .setFormat  (VK_FORMAT_R32G32B32A32_SFLOAT)
//                     .setAspect  (VK_IMAGE_ASPECT_COLOR_BIT)
//                     .setLevels  (0, 1)
//                     .setLayers  (i, 1)
//                     .build( m_IrradianceImage );

//                 imageViews.push_back( faceView.handle() );

//                 // Create framebuffer
//                 VkFramebuffer           framebuffer;
//                 VkFramebufferCreateInfo framebufferInfo{ VKInit::framebufferCreateInfo() };
//                 {
//                     framebufferInfo.renderPass      = m_IrradianceRenderPass;

//                     framebufferInfo.attachmentCount = 1;
//                     framebufferInfo.pAttachments    = &faceView.handle();
                    
//                     framebufferInfo.width           = m_Settings.IrradianceSize;
//                     framebufferInfo.height          = m_Settings.IrradianceSize;
//                     framebufferInfo.layers          = 1;
//                 }

//                 vkCreateFramebuffer(VyContext::device(), &framebufferInfo, nullptr, &framebuffer);

//                 TString fbName = std::format("irradiance_framebuffer_{}", i);
//                 VKDbg::setObjectName(VK_OBJECT_TYPE_FRAMEBUFFER, (U64)framebuffer, fbName.c_str());

//                 framebuffers.push_back( framebuffer );

//                 VkClearValue clearValue = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
                
//                 // Render Pass
//                 VkRenderPassBeginInfo renderPassInfo{ VKInit::renderPassBeginInfo() };
//                 {
//                     renderPassInfo.renderPass        = m_IrradianceRenderPass;
//                     renderPassInfo.framebuffer       = framebuffer;
                    
//                     renderPassInfo.renderArea.offset = {0, 0};
//                     renderPassInfo.renderArea.extent = {static_cast<U32>(m_Settings.IrradianceSize), static_cast<U32>(m_Settings.IrradianceSize)};
                    
//                     renderPassInfo.clearValueCount   = 1;
//                     renderPassInfo.pClearValues      = &clearValue;
//                 }

//                 vkCmdBeginRenderPass(cmdBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
//                 {
//                     VkExtent2D extent{
//                         static_cast<U32>(m_Settings.IrradianceSize), 
//                         static_cast<U32>(m_Settings.IrradianceSize) 
//                     };

//                     VKCmd::viewport(cmdBuffer, extent);
//                     VKCmd::scissor (cmdBuffer, extent);
                    
//                     m_IrradiancePipeline.bind( cmdBuffer );

//                     m_IrradiancePipeline.bindDescriptorSet(cmdBuffer, 
//                         0, 
//                         m_IrradianceDescSet
//                     );
                    
//                     // Fill push constant data.
//                     IrradiancePushBlock pushBlock;
//                     {
//                         pushBlock.MVP         = captureProjection * captureViews[ i ];
//                         pushBlock.FaceIndex   = i;
//                         pushBlock.SampleDelta = m_Settings.IrradianceSampleDelta;
//                     }

//                     m_IrradiancePipeline.pushConstants(cmdBuffer, 
//                         VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 
//                         &pushBlock, 
//                         sizeof(IrradiancePushBlock)
//                     );

//                     // Draw 36 vertices (12 triangles) for a cube.
//                     vkCmdDraw(cmdBuffer, 36, 1, 0, 0);
//                 }
//                 vkCmdEndRenderPass(cmdBuffer);
//             }
//         }
//         VyContext::endCommands(cmdBuffer);

//         for (auto framebuffer : framebuffers)
//         {
//             vkDestroyFramebuffer(VyContext::device(), framebuffer, nullptr);
//         }

//         // Transition to shader read
//         transitionImageLayoutHelper(
//             m_IrradianceImage,
//             VK_FORMAT_R32G32B32A32_SFLOAT,
//             VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
//             VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
//             1,
//             6
//         );
//     }

// #pragma endregion IRRADIANCE


// // =========================================================================================================================
// #pragma region [ PREFILTER ]
// // =========================================================================================================================

//     void VyIBLSystem::createPrefilterResources()
//     {
//         // Similar to Irradiance but different format and shader
//         VkAttachmentDescription attachment{ VKInit::attachmentDescription() };
//         {
//             attachment.format         = VK_FORMAT_R16G16B16A16_SFLOAT;
//             attachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
//             attachment.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
//             attachment.initialLayout  = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
//             attachment.finalLayout    = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
//         }

//         VkAttachmentReference colorAttachmentRef{};
//         {
//             colorAttachmentRef.attachment = 0;
//             colorAttachmentRef.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
//         }

//         VkSubpassDescription subpass{};
//         {
//             subpass.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;
//             subpass.colorAttachmentCount = 1;
//             subpass.pColorAttachments    = &colorAttachmentRef;
//         }

//         VkRenderPassCreateInfo renderPassInfo{ VKInit::renderPassCreateInfo() };
//         {
//             renderPassInfo.attachmentCount = 1;
//             renderPassInfo.pAttachments    = &attachment;
            
//             renderPassInfo.subpassCount    = 1;
//             renderPassInfo.pSubpasses      = &subpass;
//         }

//         VK_CHECK_SUCCESS(vkCreateRenderPass(VyContext::device(), &renderPassInfo, nullptr, &m_PrefilterRenderPass), 
//             "Failed to create prefilter render pass!");

//         VKDbg::setObjectName(VK_OBJECT_TYPE_RENDER_PASS, (U64)m_PrefilterRenderPass, "prefiltered_renderpass");

//         // Descriptor Set Layout
//         m_PrefilterDescSetLayout = VyDescriptorSetLayout::Builder{}
//             .addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
//             .buildPtr();

//         // Pipeline
//         m_PrefilterPipeline = VyPipeline::GraphicsBuilder{}
//             .setName               ("prefilter")
//             .addDescriptorSetLayout(m_PrefilterDescSetLayout->handle())
//             .addPushConstantRange  (VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(PrefilterPushBlock))
//             .addShaderStage        (VK_SHADER_STAGE_VERTEX_BIT,   "PrefilterEnvmap.vert.spv")
//             .addShaderStage        (VK_SHADER_STAGE_FRAGMENT_BIT, "PrefilterEnvmap.frag.spv")
//             .setCullMode           (VK_CULL_MODE_NONE)
//             .addColorAttachment    (VK_FORMAT_R16G16B16A16_SFLOAT)
//             .setDepthAttachment    (VK_FORMAT_D32_SFLOAT)
//             // No vertex input needed (generated in shader)
//             .clearVertexDescriptions() // Clear default vertex bindings and attributes.
//             .setRenderPass(m_PrefilterRenderPass)
//             .build();

//         // Descriptor Pool
//         m_PrefilterDescPool = VyDescriptorPool::Builder{}
//             .setMaxSets (1)
//             .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1)
//             .buildPtr();

//         // Allocate Descriptor Set
//         if (!m_PrefilterDescPool->allocate( m_PrefilterDescSetLayout->handle(), m_PrefilterDescSet ))
//         {
//             VY_THROW_RUNTIME_ERROR("Failed to allocate prefilter descriptor set!");
//         }
//     }

//     // =====================================================================================================================

//     void VyIBLSystem::generatePrefilteredEnvMap(VySkybox& skybox)
//     {
//         // Combined image sampler.
//         VkDescriptorImageInfo imageInfo = skybox.descriptorImageInfo();
        
//         // Update descriptor set
//         VyDescriptorWriter{ *m_PrefilterDescSetLayout, *m_PrefilterDescPool }
//             .writeImage( 0, &imageInfo )
//             .update( m_PrefilterDescSet );


//         Mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
//         Mat4 captureViews[]    = {
//             glm::lookAt(Vec3(0.0f, 0.0f, 0.0f), Vec3( 1.0f,  0.0f,  0.0f), Vec3( 0.0f, -1.0f,  0.0f)),
//             glm::lookAt(Vec3(0.0f, 0.0f, 0.0f), Vec3(-1.0f,  0.0f,  0.0f), Vec3( 0.0f, -1.0f,  0.0f)),
//             glm::lookAt(Vec3(0.0f, 0.0f, 0.0f), Vec3( 0.0f,  1.0f,  0.0f), Vec3( 0.0f,  0.0f,  1.0f)),
//             glm::lookAt(Vec3(0.0f, 0.0f, 0.0f), Vec3( 0.0f, -1.0f,  0.0f), Vec3( 0.0f,  0.0f, -1.0f)),
//             glm::lookAt(Vec3(0.0f, 0.0f, 0.0f), Vec3( 0.0f,  0.0f,  1.0f), Vec3( 0.0f, -1.0f,  0.0f)),
//             glm::lookAt(Vec3(0.0f, 0.0f, 0.0f), Vec3( 0.0f,  0.0f, -1.0f), Vec3( 0.0f, -1.0f,  0.0f))
//         };

//         TVector<VkFramebuffer> framebuffers;
//         TVector<VkImageView>   imageViews;

//         VkCommandBuffer cmdBuffer = VyContext::beginCommands();
//         {
//             for (int mip = 0; mip < m_Settings.PrefilterMipLevels; ++mip)
//             {
//                 U32 mipW = m_Settings.PrefilterSize * std::pow(0.5, mip);
//                 U32 mipH = m_Settings.PrefilterSize * std::pow(0.5, mip);
                
//                 float roughness = (float)mip / (float)(m_Settings.PrefilterMipLevels - 1);

//                 for (int i = 0; i < 6; ++i)
//                 {
//                     TString name = std::format("ibl_prefiltered_env_map_mip_{}_face_{}", mip, i);
                    
//                     VyImageView faceView = VyImageView::Builder{}
//                         .setName    (name)
//                         .setViewType(VK_IMAGE_VIEW_TYPE_2D)
//                         .setFormat  (VK_FORMAT_R16G16B16A16_SFLOAT)
//                         .setAspect  (VK_IMAGE_ASPECT_COLOR_BIT)
//                         .setLevels  (mip, 1)
//                         .setLayers  (i,   1)
//                         .build( m_PrefilteredImage );

//                     imageViews.push_back( faceView.handle() );

//                     VkFramebuffer           framebuffer;
//                     VkFramebufferCreateInfo framebufferInfo{ VKInit::framebufferCreateInfo() };
//                     {
//                         framebufferInfo.renderPass      = m_PrefilterRenderPass;

//                         framebufferInfo.attachmentCount = 1;
//                         framebufferInfo.pAttachments    = &faceView.handle();
                        
//                         framebufferInfo.width           = mipW;
//                         framebufferInfo.height          = mipH;
//                         framebufferInfo.layers          = 1;
//                     }

//                     vkCreateFramebuffer(VyContext::device(), &framebufferInfo, nullptr, &framebuffer);

//                     TString fbName = std::format("prefiltered_framebuffer_{}", i);
//                     VKDbg::setObjectName(VK_OBJECT_TYPE_FRAMEBUFFER, (U64)framebuffer, fbName.c_str());
                    
//                     framebuffers.push_back( framebuffer );

//                     VkClearValue clearValue = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
                    
//                     // Render Pass
//                     VkRenderPassBeginInfo renderPassInfo{ VKInit::renderPassBeginInfo() };
//                     {
//                         renderPassInfo.renderPass        = m_PrefilterRenderPass;
//                         renderPassInfo.framebuffer       = framebuffer;

//                         renderPassInfo.renderArea.offset =  {0, 0 };
//                         renderPassInfo.renderArea.extent = { mipW, mipH };
                        
//                         renderPassInfo.clearValueCount   = 1;
//                         renderPassInfo.pClearValues      = &clearValue;
//                     }

//                     vkCmdBeginRenderPass(cmdBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
//                     {
//                         VkExtent2D extent{ mipW, mipH };

//                         VKCmd::viewport(cmdBuffer, extent);
//                         VKCmd::scissor (cmdBuffer, extent);

//                         m_PrefilterPipeline.bind( cmdBuffer );

//                         // Bind Descriptor Set ( 0 )
//                         m_PrefilterPipeline.bindDescriptorSet(cmdBuffer, 
//                             0, 
//                             m_PrefilterDescSet
//                         );

//                         // Fill push constant data.
//                         PrefilterPushBlock pushBlock;
//                         {
//                             pushBlock.MVP         = captureProjection * captureViews[ i ];
//                             pushBlock.FaceIndex   = i;
//                             pushBlock.Roughness   = roughness;
//                             pushBlock.SampleCount = m_Settings.PrefilterSampleCount;
//                         }

//                         m_PrefilterPipeline.pushConstants(cmdBuffer, 
//                             VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 
//                             &pushBlock, 
//                             sizeof(PrefilterPushBlock)
//                         );

//                         // Draw 36 vertices (12 triangles) for a cube.
//                         vkCmdDraw(cmdBuffer, 36, 1, 0, 0);
//                     }
//                     vkCmdEndRenderPass(cmdBuffer);
//                 }
//             }
//         }
//         VyContext::endCommands(cmdBuffer);

//         for (auto framebuffer : framebuffers)
//         {
//             vkDestroyFramebuffer(VyContext::device(), framebuffer, nullptr);
//         }

//         transitionImageLayoutHelper(
//             m_PrefilteredImage,
//             VK_FORMAT_R16G16B16A16_SFLOAT,
//             VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
//             VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
//             m_Settings.PrefilterMipLevels,
//             6
//         );
//     }

// #pragma endregion PREFILTER


// // =========================================================================================================================
// #pragma region [ BRDFLUT ]
// // =========================================================================================================================

//     void VyIBLSystem::createBRDFResources()
//     {
//         // Descriptor Set Layout
//         m_BrdfDescSetLayout = VyDescriptorSetLayout::Builder{}
//             .addBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
//             .buildPtr();

//         // Pipeline
//         m_BrdfPipeline = VyPipeline::ComputeBuilder{}
//             .setName               ("brdf_lut")
//             .addDescriptorSetLayout(m_BrdfDescSetLayout->handle())
//             .setShaderStage        ("BrdfLUT.comp.spv")
//             .build();

//         // Descriptor Pool
//         m_BrdfDescPool = VyDescriptorPool::Builder{}
//             .setMaxSets (1)
//             .addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1)
//             .buildPtr();

//         // Allocate Descriptor Set
//         if (!m_BrdfDescPool->allocate( m_BrdfDescSetLayout->handle(), m_BrdfDescSet ))
//         {
//             VY_THROW_RUNTIME_ERROR("Failed to allocate BRDF descriptor set!");
//         }
//     }

//     // =====================================================================================================================

//     void VyIBLSystem::generateBRDFLUT()
//     {
//         VkDescriptorImageInfo storageImageInfo{};
//         {
//             storageImageInfo.sampler     = nullptr;
//             storageImageInfo.imageView   = m_BrdfLUTImageView.handle();
//             storageImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
//         }
        
//         // Update descriptor set
//         VyDescriptorWriter{ *m_BrdfDescSetLayout, *m_BrdfDescPool }
//             .writeImage(0, &storageImageInfo)
//             .update( m_BrdfDescSet );

//         // Dispatch
//         VkCommandBuffer cmdBuffer = VyContext::beginCommands();
//         {
//             m_BrdfPipeline.bind( cmdBuffer );
            
//             m_BrdfPipeline.bindDescriptorSet(cmdBuffer, 
//                 0, 
//                 m_BrdfDescSet
//             );

//             // Dispatch 
//             vkCmdDispatch(cmdBuffer, m_Settings.BrdfLUTSize / 16, m_Settings.BrdfLUTSize / 16, 1);
//         }
//         VyContext::endCommands(cmdBuffer);

//         // Transition to shader read only.
//         transitionImageLayoutHelper(
//             m_BrdfLUTImage, 
//             VK_FORMAT_R16G16_SFLOAT, 
//             VK_IMAGE_LAYOUT_GENERAL, 
//             VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 
//             1
//         );
//     }

// #pragma endregion BRDFLUT
// }