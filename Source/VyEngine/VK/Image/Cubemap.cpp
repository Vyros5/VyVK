// #include <VyEngine/VK/Image/Cubemap.h>

// #include <VyEngine/VK/Context.h>

// #include <VyEngine/GFX/Resources/Texture/Texture.h>

// #include <VyEngine/VK/Descriptors/Descriptors.h>
// #include <VyEngine/VK/Pipeline/Pipeline.h>
// #include <VyEngine/Globals.h>

// #include <stb_image.h>

// namespace Vy
// {
//     Cubemap::Cubemap(const TString& folderPath, const TString& extension)
//     {
//         createTexture(folderPath, extension);
//     }

//     void Cubemap::createCubemap(const TArray<TString, 6>& facePaths)
//     {
//         // Load all 6 faces and determine size.
//         TArray<U8*, 6> pFaceData{};

//         int texW{0};
//         int texH{0};

//         for (int i = 0; i < 6; i++)
//         {
//             // Load all faces.
//             pFaceData[ i ] = VyTexture::loadImage(facePaths[ i ], texW, texH, STBI_rgb_alpha);
            
//             if ( !pFaceData[ i ] )
//             {
//                 // Cleanup previously loaded images before throwing.
//                 for (int j = 0; j < i; j++)
//                 {
//                     VyTexture::freeImageData( pFaceData[ j ] );
//                 }

//                 VY_THROW_RUNTIME_ERROR("Failed to load skybox texture face: " + 
// 					facePaths[ i ] + " - " + stbi_failure_reason());
//             }

//             // Verify all faces are same size.
//             if ( i == 0 )
//             {
//                 m_Texture.Size = texW;

//                 if (texW != texH)
//                 {
//                     // Cleanup
//                     for (int j = 0; j <= i; ++j) 
// 					{
// 						if (pFaceData[ j ]) 
// 						{
//                             VyTexture::freeImageData( pFaceData[ j ] );
// 						}
// 					}

//                     VY_THROW_RUNTIME_ERROR("Skybox faces must be square. Face 0 (" + facePaths[0] + ") is " + 
// 						std::to_string(texW) + "x" + std::to_string(texH));
//                 }
//             }
//             else if ( texW != m_Texture.Size || texH != m_Texture.Size )
//             {
//                 // Cleanup
//                 for (int j = 0; j <= i; j++)
//                 {
//                     if (pFaceData[ j ]) 
//                     {
//                         VyTexture::freeImageData( pFaceData[ j ] );
//                     }
//                 }

//                 VY_THROW_RUNTIME_ERROR("Skybox faces must have consistent dimensions. Face " + 
//                     std::to_string(i) + " (" + facePaths[ i ] + ") is " +
//                     std::to_string(texW)  + "x" + std::to_string(texH) + ", expected " + 
//                     std::to_string(m_Texture.Size) + "x" + std::to_string(m_Texture.Size)
//                 );
//             }
//         }

//         m_Texture.MipLevels = static_cast<U32>(std::floor(std::log2(m_Texture.Size))) + 1;

//         VkDeviceSize faceSize  = static_cast<VkDeviceSize>(m_Texture.Size) * static_cast<VkDeviceSize>(m_Texture.Size) * 4; // RGBA

//         // Create staging buffer with all face data.
//         VyBuffer stagingBuffer{ VyBuffer::stagingBuffer("cubemap", faceSize, 6, 0, false /*not persistent*/) };

//         stagingBuffer.map();

//         for (int i = 0; i < 6; i++)
//         {
//             // Write each face into the buffer.
//             stagingBuffer.writeToIndex( pFaceData[ i ], i );

//             // Free CPU-side image data.
//             VyTexture::freeImageData( pFaceData[ i ] );
        
// 			// Avoid double free.
//             pFaceData[ i ] = nullptr;
//         }

//         stagingBuffer.unmap();

//         // Create cubemap image
//         m_Texture.Image = VyImage::Builder{}
//             .setName       ("cubemap")
//             .setImageType  (VK_IMAGE_TYPE_2D)
//             .setFormat     (VK_FORMAT_R8G8B8A8_SRGB)
//             .setExtent     (static_cast<U32>(m_Texture.Size), static_cast<U32>(m_Texture.Size))
//             .setLevels     (m_Texture.MipLevels)
// 			.setLayers     (6) // 6 faces
// 			.setSamples    (VK_SAMPLE_COUNT_1_BIT)
//             .setTiling     (VK_IMAGE_TILING_OPTIMAL)
// 			.setLayout     (VK_IMAGE_LAYOUT_UNDEFINED)
//             .setUsage      (VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
//             .setFlags      (VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT) // Cubemap
//             .setMemoryUsage(VMA_MEMORY_USAGE_AUTO)
//             .build();

//         VkCommandBuffer cmdBuffer = VyContext::beginCommands();
//         {
//             // Transition to transfer destination.
//             m_Texture.Image.transitionLayout(cmdBuffer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
            
//             // Copy each face from staging buffer.
//             TArray<VkBufferImageCopy, 6> regions{};

//             for (int i = 0; i < 6; i++)
//             {
//                 regions[ i ].bufferOffset                    = i * faceSize;
//                 regions[ i ].bufferRowLength                 = 0;
//                 regions[ i ].bufferImageHeight               = 0;
//                 regions[ i ].imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
//                 regions[ i ].imageSubresource.mipLevel       = m_Texture.MipLevels;
//                 regions[ i ].imageSubresource.baseArrayLayer = i;
//                 regions[ i ].imageSubresource.layerCount     = 1;
//                 regions[ i ].imageOffset                     = { 0, 0, 0 };
//                 regions[ i ].imageExtent                     = { static_cast<U32>(m_Texture.Size), static_cast<U32>(m_Texture.Size), 1 };
//             }

//             vkCmdCopyBufferToImage(cmdBuffer, 
//                 stagingBuffer  .handle(), 
//                 m_Texture.Image.handle(), 
//                 VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
//                 6, regions.data()
//             );
        
//             // Transition to shader read
//             m_Texture.Image.transitionLayout(cmdBuffer, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
//         }
//         VyContext::endCommands(cmdBuffer);

//         // Create view.
//         m_Texture.View = VyImageView::Builder{}
//             .setName    ("cubemap")
//             .setViewType(VK_IMAGE_VIEW_TYPE_CUBE)
//             .setFormat  (VK_FORMAT_R8G8B8A8_SRGB)
//             .setAspect  (VK_IMAGE_ASPECT_COLOR_BIT)
//             .setLevels  (0, m_Texture.MipLevels)
//             .setLayers  (0, 6) // 6 faces
//             .build( m_Texture.Image );

//         // Create Sampler
//         m_Texture.Sampler = VySampler::Builder{}
//             .setName         ("cubemap")
//             .setFilters      (VK_FILTER_LINEAR)
//             .setMipmapMode   (VK_SAMPLER_MIPMAP_MODE_LINEAR)
//             .setWrap         (VK_SAMPLER_ADDRESS_MODE_REPEAT)
//             .setBorder       (VK_BORDER_COLOR_INT_OPAQUE_BLACK)
// 			.enableAnisotropy(true)
//             .setLodRange     (0.0f, 0.0f)
//             .setMipLodBias   (0.0f)
//             .build();

//         // Generate
//         generateIBL();
//     }

//     void Cubemap::createTexture(const TString& folderPath, const TString& extension)
//     {
//         // TString names[] = {"right", "left", "bottom", "top", "front", "back"};

//         // Texture::Builder builder(device);
//         // builder.isCubemap(true)
//         //     .format(VK_FORMAT_R8G8B8A8_SRGB)
//         //     .addressMode(VK_SAMPLER_ADDRESS_MODE_REPEAT)
//         //     .useMipmaps(true);

//         // for (int i = 0; i < 6; i++) { builder.addLayer(FileTextureSource(folderPath + names[i] + "." + extension)); }

//         // texture = builder.build();
//         // GenerateIBL();
//     }

//     void Cubemap::loadHDR(const TString& filepath)
//     {
//         if (!std::filesystem::exists(filepath)) 
//         {
//             spdlog::warn("Skybox HDR not found at: {}. IBL will be disabled.", filepath);
//             return;
//         }
//         spdlog::info("Loading HDR environment map: {}", filepath);
        
//         int texW{0};
//         int texH{0};
//         int texC{0};

//         stbi_set_flip_vertically_on_load(true);

//         float* pData = stbi_loadf(filepath.c_str(), &texW, &texH, &texC, STBI_rgb_alpha);

//         stbi_set_flip_vertically_on_load(false);

//         if (!pData) 
//         {
//             spdlog::error("Failed to load HDR image: {}", filepath);
//             return;
//         }

//         m_Equirect.Image = VyImage::Builder{}
//             .setName       ("equirect")
//             .setImageType  (VK_IMAGE_TYPE_2D)
//             .setFormat     (VK_FORMAT_R32G32B32A32_SFLOAT)
//             .setExtent     (static_cast<U32>(texW), static_cast<U32>(texH))
//             .setLevels     (1)
// 			.setLayers     (1) 
// 			.setSamples    (VK_SAMPLE_COUNT_1_BIT)
//             .setTiling     (VK_IMAGE_TILING_OPTIMAL)
// 			.setLayout     (VK_IMAGE_LAYOUT_UNDEFINED)
//             .setUsage      (VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
//             .setMemoryUsage(VMA_MEMORY_USAGE_AUTO)
//             .build();

//         m_Equirect.Image.upload(pData, static_cast<U32>(texW) * static_cast<U32>(texH) * 4 * sizeof(float));

//         stbi_image_free( pData );

//         m_Equirect.View = VyImageView::Builder{}
//             .setName    ("equirect")
//             .setViewType(VK_IMAGE_VIEW_TYPE_2D)
//             .setFormat  (VK_FORMAT_R32G32B32A32_SFLOAT)
//             .setAspect  (VK_IMAGE_ASPECT_COLOR_BIT)
//             .setLevels  (0, 1)
//             .setLayers  (0, 1)
//             .build( m_Equirect.Image );

//         m_Equirect.Sampler = VySampler::Builder{}
//             .setName         ("equirect")
//             .setFilters      (VK_FILTER_LINEAR)
//             .setMipmapMode   (VK_SAMPLER_MIPMAP_MODE_LINEAR)
//             .setWrap         (VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE)
//             .setBorder       (VK_BORDER_COLOR_INT_OPAQUE_BLACK)
// 			// .enableAnisotropy(true)
//             .setLodRange     (0.0f, 0.0f)
//             .setMipLodBias   (0.0f)
//             .build();

//         // Convert to Cubemap
//         convertEquirectToCube();
//     }


//     void Cubemap::convertEquirectToCube()
//     {
//         uint32_t cubemapSize = 1024;

//         m_Skybox.Image = VyImage::Builder{}
//             .setName       ("skybox")
//             .setImageType  (VK_IMAGE_TYPE_2D)
//             .setFormat     (VK_FORMAT_R16G16B16A16_SFLOAT)
//             .setExtent     (cubemapSize, cubemapSize)
//             .setLevels     (1)
// 			.setLayers     (6) 
// 			.setSamples    (VK_SAMPLE_COUNT_1_BIT)
//             .setTiling     (VK_IMAGE_TILING_OPTIMAL)
// 			.setLayout     (VK_IMAGE_LAYOUT_UNDEFINED)
//             .setUsage      (VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT)
//             .setMemoryUsage(VMA_MEMORY_USAGE_AUTO)
//             .build();

//         m_Skybox.View = VyImageView::Builder{}
//             .setName    ("skybox")
//             .setViewType(VK_IMAGE_VIEW_TYPE_CUBE)
//             .setFormat  (VK_FORMAT_R16G16B16A16_SFLOAT)
//             .setAspect  (VK_IMAGE_ASPECT_COLOR_BIT)
//             .setLevels  (0, 1)
//             .setLayers  (0, 6)
//             .build( m_Skybox.Image );

//         m_Skybox.Sampler = VySampler::Builder{}
//             .setName         ("skybox")
//             .setFilters      (VK_FILTER_LINEAR)
//             .setMipmapMode   (VK_SAMPLER_MIPMAP_MODE_LINEAR)
//             .setWrap         (VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE)
//             .setBorder       (VK_BORDER_COLOR_INT_OPAQUE_BLACK)
// 			// .enableAnisotropy(true)
//             .setLodRange     (0.0f, 0.0f)
//             .setMipLodBias   (0.0f)
//             .build();

//         auto descriptorPool = VyDescriptorPool::Builder()
//             .setMaxSets (1)
//             .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1)
//             .addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1)
//             .buildPtr();

//         auto setLayout = VyDescriptorSetLayout::Builder()
//             .addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_COMPUTE_BIT)
//             .addBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
//             .buildPtr();

//         auto equirectInfo = m_Equirect.descriptorImageInfo();
//         auto skyboxInfo   = m_Skybox  .descriptorImageInfo();

//         VkDescriptorSet set;
//         VyDescriptorWriter(*setLayout, *descriptorPool)
//             .writeImage(0, &equirectInfo)
//             .writeImage(1, &skyboxInfo)
//             .build( set );

//         // Create pipeline
//         auto builder = VyPipeline::ComputeBuilder{};
//         {
//             builder.setName( "cubemap" );
            
//             builder.addDescriptorSetLayout( setLayout->handle() );

//             // builder.addPushConstantRange(VK_SHADER_STAGE_COMPUTE_BIT, sizeof(uint32_t) * 2);
            
//             builder.setShaderStage("EquirectToCube.comp.spv");
//         }

//         auto pipeline = builder.buildPtr();

//         // Begin one-time command buffer
//         VkCommandBuffer cmdBuffer = VyContext::beginCommands();
//         {
//             m_Skybox.Image.transitionLayout(cmdBuffer, VK_IMAGE_LAYOUT_GENERAL);

//             pipeline->bind(cmdBuffer);
            
//             pipeline->bindDescriptorSet(cmdBuffer, 0, set);
            
//             vkCmdDispatch(cmdBuffer, 
//                 cubemapSize / 16, 
//                 cubemapSize / 16,
//                 6
//             );
        
//             // Barrier for skybox generation
//             VkImageMemoryBarrier barrier{ VKInit::imageMemoryBarrier() };
//             {
//                 barrier.oldLayout                   = VK_IMAGE_LAYOUT_GENERAL;
//                 barrier.newLayout                   = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
//                 barrier.srcAccessMask               = VK_ACCESS_SHADER_WRITE_BIT;
//                 barrier.dstAccessMask               = VK_ACCESS_SHADER_READ_BIT;
//                 barrier.image                       = m_Skybox.Image.handle();
//                 barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//                 barrier.subresourceRange.levelCount = 1;
//                 barrier.subresourceRange.layerCount = 6;

//                 vkCmdPipelineBarrier(cmdBuffer, 
//                     VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
//                     VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 
//                     0, 
//                     0, nullptr, 
//                     0, nullptr, 
//                     1, &barrier
//                 );
//             }
//         }
//         VyContext::endCommands(cmdBuffer);
//     }

//     void Cubemap::createFromHdri(const TString& filepath, U32 resolution)
//     {
//         float* pData{};

//         int texW{0};
//         int texH{0};
//         int texC{0};

//         pData = stbi_loadf(filepath.c_str(), &texW, &texH, &texC, STBI_rgb_alpha);

//         m_Texture.MipLevels = static_cast<U32>(std::floor(std::max(static_cast<U32>(texW), static_cast<U32>(texH)))) + 1;

//         m_Texture.Size = std::max(static_cast<U32>(texW), static_cast<U32>(texH));

//         VkDeviceSize imageSize  = static_cast<VkDeviceSize>(texW) * static_cast<VkDeviceSize>(texH) * 4; // RGBA

//         // Create staging buffer.
//         VyBuffer stagingBuffer{ VyBuffer::stagingBuffer("hdri_cubemap", imageSize, 1, 0, false /*not persistent*/) };

//         stagingBuffer.map();

//         // Write image into the buffer.
//         stagingBuffer.write( pData );

//         // Free CPU-side image data.
//         stbi_image_free( pData );

//         stagingBuffer.unmap();

//         // Create hdri image
//         m_Texture.Image = VyImage::Builder{}
//             .setName       ("hdri_cubemap")
//             .setImageType  (VK_IMAGE_TYPE_2D)
//             .setFormat     (VK_FORMAT_R32G32B32A32_SFLOAT)
//             .setExtent     (static_cast<U32>(m_Texture.Size), static_cast<U32>(m_Texture.Size))
//             .setLevels     (m_Texture.MipLevels)
// 			.setLayers     (6) // 6 faces
// 			.setSamples    (VK_SAMPLE_COUNT_1_BIT)
//             .setTiling     (VK_IMAGE_TILING_OPTIMAL)
// 			.setLayout     (VK_IMAGE_LAYOUT_UNDEFINED)
//             .setUsage      (VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT)
//             .setFlags      (VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT) // Cubemap
//             .setMemoryUsage(VMA_MEMORY_USAGE_AUTO)
//             .build();

// 		// Transition image layout and copy buffer to image.
// 		m_Texture.Image.copyFrom( stagingBuffer, false /*toShaderReadOnly*/ );

// 		// Generate mipmaps (this also transitions to SHADER_READ_ONLY_OPTIMAL)
// 		m_Texture.Image.generateMipmaps( VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL );

//         // Create view.
//         m_Texture.View = VyImageView::Builder{}
//             .setName    ("hdri_cubemap")
//             .setViewType(VK_IMAGE_VIEW_TYPE_CUBE)
//             .setFormat  (VK_FORMAT_R32G32B32A32_SFLOAT)
//             .setAspect  (VK_IMAGE_ASPECT_COLOR_BIT)
//             .setLevels  (0, m_Texture.MipLevels)
//             .setLayers  (0, 6) // 6 faces
//             .build( m_Texture.Image );

//         // Create Sampler
//         m_Texture.Sampler = VySampler::Builder{}
//             .setName         ("hdri_cubemap")
//             .setFilters      (VK_FILTER_LINEAR)
//             .setMipmapMode   (VK_SAMPLER_MIPMAP_MODE_LINEAR)
//             .setWrap         (VK_SAMPLER_ADDRESS_MODE_REPEAT)
//             .setBorder       (VK_BORDER_COLOR_INT_OPAQUE_BLACK)
// 			.enableAnisotropy(true)
//             .setLodRange     (0.0f, 0.0f)
//             .setMipLodBias   (0.0f)
//             .build();

//         // Generate
//         equirect2Cubemap(filepath);

//         generateIBL();
//     }


//     void Cubemap::equirect2Cubemap(const TString& filepath)
//     {
//         float* pData{};

//         int texW{0};
//         int texH{0};
//         int texC{0};

//         pData = stbi_loadf(filepath.c_str(), &texW, &texH, &texC, STBI_rgb_alpha);

//         m_HDRI.MipLevels = static_cast<U32>(std::floor(std::max(static_cast<U32>(texW), static_cast<U32>(texH)))) + 1;

//         m_HDRI.Extent = { static_cast<U32>(texW), static_cast<U32>(texH) };

//         VkDeviceSize imageSize  = static_cast<VkDeviceSize>(texW) * static_cast<VkDeviceSize>(texH) * 4; // RGBA

//         // Create staging buffer.
//         VyBuffer stagingBuffer{ VyBuffer::stagingBuffer("hdri", imageSize, 1, 0, false /*not persistent*/) };

//         stagingBuffer.map();

//         // Write image into the buffer.
//         stagingBuffer.write( pData );

//         // Free CPU-side image data.
//         stbi_image_free( pData );

//         stagingBuffer.unmap();

//         // Create hdri image
//         m_HDRI.Image = VyImage::Builder{}
//             .setName       ("hdri_cubemap")
//             .setImageType  (VK_IMAGE_TYPE_2D)
//             .setFormat     (VK_FORMAT_R32G32B32A32_SFLOAT)
//             .setExtent     (m_HDRI.Extent)
//             .setLevels     (m_HDRI.MipLevels)
// 			.setLayers     (6) // 6 faces
// 			.setSamples    (VK_SAMPLE_COUNT_1_BIT)
//             .setTiling     (VK_IMAGE_TILING_OPTIMAL)
// 			.setLayout     (VK_IMAGE_LAYOUT_UNDEFINED)
//             .setUsage      (VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT)
//             .setFlags      (VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT) // Cubemap
//             .setMemoryUsage(VMA_MEMORY_USAGE_AUTO)
//             .build();

// 		// Transition image layout and copy buffer to image.
// 		m_HDRI.Image.copyFrom( stagingBuffer, false /*toShaderReadOnly*/ );

// 		// Generate mipmaps (this also transitions to SHADER_READ_ONLY_OPTIMAL)
// 		m_HDRI.Image.generateMipmaps( VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL );

//         // Create view.
//         m_HDRI.View = VyImageView::Builder{}
//             .setName    ("hdri_cubemap")
//             .setViewType(VK_IMAGE_VIEW_TYPE_CUBE)
//             .setFormat  (VK_FORMAT_R32G32B32A32_SFLOAT)
//             .setAspect  (VK_IMAGE_ASPECT_COLOR_BIT)
//             .setLevels  (0, m_HDRI.MipLevels)
//             .setLayers  (0, 6) // 6 faces
//             .build( m_HDRI.Image );

//         // Create Sampler
//         m_HDRI.Sampler = VySampler::Builder{}
//             .setName         ("hdri_cubemap")
//             .setFilters      (VK_FILTER_LINEAR)
//             .setMipmapMode   (VK_SAMPLER_MIPMAP_MODE_LINEAR)
//             .setWrap         (VK_SAMPLER_ADDRESS_MODE_REPEAT)
//             .setBorder       (VK_BORDER_COLOR_INT_OPAQUE_BLACK)
// 			.enableAnisotropy(true)
//             .setLodRange     (0.0f, 0.0f)
//             .setMipLodBias   (0.0f)
//             .build();


//         auto descriptorPool = VyDescriptorPool::Builder()
//             .setMaxSets(1)
//             .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1)
//             .addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1)
//             .buildPtr();

//         auto setLayout = VyDescriptorSetLayout::Builder()
//             .addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_COMPUTE_BIT)
//             .addBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
//             .buildPtr();

//         auto hdriInfo	 = m_HDRI   .descriptorImageInfo();
//         auto textureInfo = m_Texture.descriptorImageInfo();

//         VkDescriptorSet set;
//         VyDescriptorWriter(*setLayout, *descriptorPool)
//             .writeImage(0, &hdriInfo)
//             .writeImage(1, &textureInfo)
//             .build( set );

//         // Create pipeline
//         auto builder = VyPipeline::ComputeBuilder{};
//         {
//             builder.setName( "cubemap" );
            
//             builder.addDescriptorSetLayout( setLayout->handle() );
            
//             builder.setShaderStage("equirect2cube.comp");
//         }

//         auto pipeline = builder.buildPtr();

//         // render

//         // Begin one-time command buffer
//         VkCommandBuffer cmdBuffer = VyContext::beginCommands();
//         {
//             pipeline->bind(cmdBuffer);
            
//             pipeline->bindDescriptorSet(cmdBuffer, 0, set);
            
//             const glm::ivec3 shaderLocalSize {32, 32, 1};
            
//             vkCmdDispatch(cmdBuffer, 
//                 m_Texture.Image.width()      / shaderLocalSize.x, 
//                 m_Texture.Image.height()     / shaderLocalSize.y,
//                 m_Texture.Image.layerCount() / shaderLocalSize.z
//             );
//         }
//         VyContext::endCommands(cmdBuffer);
//     }


//     void Cubemap::generateIBL(U32 irradianceResolution)
//     {
//         generateIrradiance(irradianceResolution);
//         prefilterMap();
//     }


//     void Cubemap::generateIrradiance(U32 resolution)
//     {
//         uint32_t irradianceSize = 32;

//         m_Irradiance.Image = VyImage::Builder{}
//             .setName       ("irradiance")
//             .setImageType  (VK_IMAGE_TYPE_2D)
//             .setFormat     (VK_FORMAT_R16G16B16A16_SFLOAT)
//             .setExtent     (irradianceSize, irradianceSize)
//             .setLevels     (1)
// 			.setLayers     (6) 
// 			.setSamples    (VK_SAMPLE_COUNT_1_BIT)
//             .setTiling     (VK_IMAGE_TILING_OPTIMAL)
// 			.setLayout     (VK_IMAGE_LAYOUT_UNDEFINED)
//             .setUsage      (VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT)
//             .setMemoryUsage(VMA_MEMORY_USAGE_AUTO)
//             .build();

//         m_Irradiance.View = VyImageView::Builder{}
//             .setName    ("irradiance")
//             .setViewType(VK_IMAGE_VIEW_TYPE_CUBE)
//             .setFormat  (VK_FORMAT_R16G16B16A16_SFLOAT)
//             .setAspect  (VK_IMAGE_ASPECT_COLOR_BIT)
//             .setLevels  (0, 1)
//             .setLayers  (0, 6)
//             .build( m_Irradiance.Image );

//         m_Irradiance.Sampler = VySampler::Builder{}
//             .setName         ("irradiance")
//             .setFilters      (VK_FILTER_LINEAR)
//             .setMipmapMode   (VK_SAMPLER_MIPMAP_MODE_LINEAR)
//             .setWrap         (VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE)
//             .setBorder       (VK_BORDER_COLOR_INT_OPAQUE_BLACK)
// 			// .enableAnisotropy(true)
//             .setLodRange     (0.0f, 0.0f)
//             .setMipLodBias   (0.0f)
//             .build();

//         auto descriptorPool = VyDescriptorPool::Builder()
//             .setMaxSets (1)
//             .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1)
//             .addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,          1)
//             .buildPtr();

//         auto setLayout = VyDescriptorSetLayout::Builder()
//             .addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_COMPUTE_BIT)
//             .addBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,          VK_SHADER_STAGE_COMPUTE_BIT)
//             .buildPtr();

//         auto irradianceInfo = m_Irradiance.descriptorImageInfo();
//         auto skyboxInfo     = m_Skybox  .descriptorImageInfo();

//         VkDescriptorSet set;
//         VyDescriptorWriter(*setLayout, *descriptorPool)
//             .writeImage(0, &irradianceInfo)
//             .writeImage(1, &skyboxInfo)
//             .build( set );

//         // Create pipeline
//         auto builder = VyPipeline::ComputeBuilder{};
//         {
//             builder.setName( "cubemap" );
            
//             builder.addDescriptorSetLayout( setLayout->handle() );

//             // builder.addPushConstantRange(VK_SHADER_STAGE_COMPUTE_BIT, sizeof(uint32_t) * 2);
            
//             builder.setShaderStage("EquirectToCube.comp.spv");
//         }

//         auto pipeline = builder.buildPtr();

//         Texture::Builder builder(device);
//         builder.isCubemap(true)
//             .format(VK_FORMAT_R32G32B32A32_SFLOAT)
//             .addressMode(VK_SAMPLER_ADDRESS_MODE_REPEAT)
//             .addUsageFlag(VK_IMAGE_USAGE_STORAGE_BIT)
//             .layout(VK_IMAGE_LAYOUT_GENERAL);

//         for (int i = 0; i < 6; i++) { builder.addLayer(FloatSolidTextureSource(glm::vec4 {1.0f}, resolution, resolution)); }
//         irradiance = builder.build();



//         auto textureInfo	= texture->ImageInfo();
//         auto irradianceInfo = irradiance->ImageInfo();

//         VkDescriptorSet set;
//         DescriptorWriter(*setLayout, *descriptorPool).WriteImage(0, &textureInfo).WriteImage(1, &irradianceInfo).Build(set);

//         // create pipeline
//         std::unique_ptr<ComputePipeline> pipeline;
//         VkPipelineLayout pipelineLayout;

//         std::vector<VkDescriptorSetLayout> descripotorSetLayouts {setLayout->GetDescriptorSetLayout()};

//         VkPipelineLayoutCreateInfo pipelineLayoutInfo {};
//         pipelineLayoutInfo.sType				  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
//         pipelineLayoutInfo.setLayoutCount		  = descripotorSetLayouts.size();
//         pipelineLayoutInfo.pSetLayouts			  = descripotorSetLayouts.data();
//         pipelineLayoutInfo.pushConstantRangeCount = 0;
//         pipelineLayoutInfo.pPushConstantRanges	  = VK_NULL_HANDLE;

//         vkCreatePipelineLayout(device.VulkanDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout);

//         pipeline =
//             std::make_unique<ComputePipeline>(device, SHADER_BINARY_DIR "irradianceGenerator.comp.spv", pipelineLayout);

//         // render
//         auto cmdBuffer = device.BeginSingleTimeCommands();

//         pipeline->Bind(cmdBuffer);

//         vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout, 0, 1, &set, 0, nullptr);

//         const glm::ivec3 shaderLocalSize {16, 16, 1};
//         vkCmdDispatch(cmdBuffer, texture->width() / shaderLocalSize.x, texture->height() / shaderLocalSize.y,
//                     texture->layers() / shaderLocalSize.z);

//         device.EndSingleTimeCommands(cmdBuffer);
//         vkDestroyPipelineLayout(device.VulkanDevice(), pipelineLayout, nullptr);
//     }

//     void Cubemap::PrefilterMap()
//     {
//         U32 levels		= texture->mipMaps();
//         auto descriptorPool = DescriptorPool::Builder(device)
//                                 .SetMaxSets(1)
//                                 .AddPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1)
//                                 .AddPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, levels - 1)
//                                 .Build();

//         auto setLayout = DescriptorSetLayout::Builder(device)
//                             .AddBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_COMPUTE_BIT)
//                             .AddBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT, levels - 1)
//                             .Build();

//         auto textureInfo = texture->ImageInfo();
//         std::vector<VkDescriptorImageInfo> mipMapsImageInfos;

//         VkImageViewCreateInfo createInfo {};
//         createInfo.sType						   = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
//         createInfo.image						   = texture->image;
//         createInfo.viewType						   = VK_IMAGE_VIEW_TYPE_CUBE;
//         createInfo.format						   = texture->format_;
//         createInfo.subresourceRange.aspectMask	   = VK_IMAGE_ASPECT_COLOR_BIT;
//         createInfo.subresourceRange.levelCount	   = 1;
//         createInfo.subresourceRange.baseArrayLayer = 0;
//         createInfo.subresourceRange.layerCount	   = texture->layers_;

//         VkDescriptorImageInfo imageInfo {};
//         imageInfo.imageLayout = texture->layout_;
//         imageInfo.sampler	  = nullptr;

//         for (int i = 1; i < levels; i++) {
//             createInfo.subresourceRange.baseMipLevel = i;

//             VkImageView imageView;
//             auto code = vkCreateImageView(device.VulkanDevice(), &createInfo, nullptr, &imageView);
//             MVE_ASSERT(code == VK_SUCCESS, "Failed to create image view");

//             imageInfo.imageView = imageView;

//             mipMapsImageInfos.push_back(imageInfo);
//         }

//         VkDescriptorSet set;
//         DescriptorWriter(*setLayout, *descriptorPool)
//             .WriteImage(0, &textureInfo)
//             .WriteImage(1, mipMapsImageInfos.data(), levels - 1)
//             .Build(set);

//         struct
//         {
//             int level;
//             float roughness;
//         } pushConstants;

//         // create pipeline
//         std::unique_ptr<ComputePipeline> pipeline;
//         VkPipelineLayout pipelineLayout;

//         std::vector<VkDescriptorSetLayout> descripotorSetLayouts {setLayout->GetDescriptorSetLayout()};

//         VkPushConstantRange pushRange {};
//         pushRange.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
//         pushRange.offset	 = 0;
//         pushRange.size		 = sizeof(pushConstants);

//         VkPipelineLayoutCreateInfo pipelineLayoutInfo {};
//         pipelineLayoutInfo.sType				  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
//         pipelineLayoutInfo.setLayoutCount		  = descripotorSetLayouts.size();
//         pipelineLayoutInfo.pSetLayouts			  = descripotorSetLayouts.data();
//         pipelineLayoutInfo.pushConstantRangeCount = 1;
//         pipelineLayoutInfo.pPushConstantRanges	  = &pushRange;

//         const U32 specializationData[] = {levels - 1};
//         VkSpecializationMapEntry specializationMap {};
//         specializationMap.constantID = 0;
//         specializationMap.offset	 = 0;
//         specializationMap.size		 = sizeof(U32);

//         VkSpecializationInfo specializationInfo {};
//         specializationInfo.mapEntryCount = 1;
//         specializationInfo.pData		 = specializationData;
//         specializationInfo.pMapEntries	 = &specializationMap;
//         specializationInfo.dataSize		 = sizeof(specializationData);

//         vkCreatePipelineLayout(device.VulkanDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout);

//         pipeline = std::make_unique<ComputePipeline>(device, SHADER_BINARY_DIR "prefilterSkybox.comp.spv", pipelineLayout,
//                                                     &specializationInfo);

//         // render
//         auto cmdBuffer = device.BeginSingleTimeCommands();
//         pipeline->Bind(cmdBuffer);

//         vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout, 0, 1, &set, 0, nullptr);

//         U32 width	= texture->width() / 2;
//         U32 height = texture->height() / 2;
//         for (int level = 1; level < levels; level++) {
//             pushConstants.level		= level - 1;
//             pushConstants.roughness = (float)level / (levels - 1);

//             vkCmdPushConstants(cmdBuffer, pipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(pushConstants),
//                             &pushConstants);

//             const glm::ivec3 shaderLocalSize {16, 16, 1};
//             vkCmdDispatch(cmdBuffer, width / shaderLocalSize.x, height / shaderLocalSize.y,
//                         texture->layers() / shaderLocalSize.z);

//             width  = std::max<U32>(width / 2, shaderLocalSize.x);
//             height = std::max<U32>(height / 2, shaderLocalSize.y);
//         }

//         device.EndSingleTimeCommands(cmdBuffer);
//         vkDestroyPipelineLayout(device.VulkanDevice(), pipelineLayout, nullptr);

//         for (auto& info : mipMapsImageInfos) { vkDestroyImageView(device.VulkanDevice(), info.imageView, nullptr); }
//     }
// }