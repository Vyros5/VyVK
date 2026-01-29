// #include <VyEngine/GFX/Resources/Mesh/Material.h>

// #include <VyEngine/Engine.h>

// #include <VyEngine/VK/Pipeline/Pipeline.h>
// #include <VyEngine/VK/Context.h>
// #include <VyEngine/Globals.h>

// #include <stb_image.h>

// namespace Vy
// {
//     VyMaterial::VyMaterial()
//     {
//         createDefaultTexture();
//     }


//     VyMaterial::~VyMaterial() 
//     {
//         // Clean up albedo texture
//         VyContext::destroy(m_AlbedoTextureSampler);
//         VyContext::destroy(m_AlbedoTextureImageView);    
//         VyContext::destroy(m_AlbedoTextureImage, m_AlbedoTextureImageAllocation);

//         // Clean up normal texture
//         VyContext::destroy(m_NormalTextureSampler);
//         VyContext::destroy(m_NormalTextureImageView);    
//         VyContext::destroy(m_NormalTextureImage, m_NormalTextureImageAllocation);

//         // Clean up roughness texture
//         VyContext::destroy(m_RoughnessTextureSampler);
//         VyContext::destroy(m_RoughnessTextureImageView);
//         VyContext::destroy(m_RoughnessTextureImage, m_RoughnessTextureImageAllocation);

//         // Clean up metallic texture
//         VyContext::destroy(m_MetallicTextureSampler);
//         VyContext::destroy(m_MetallicTextureImageView);
//         VyContext::destroy(m_MetallicTextureImage, m_MetallicTextureImageAllocation);

//         // Clean up default texture
//         VyContext::destroy(m_DefaultTextureSampler);
//         VyContext::destroy(m_DefaultTextureImageView);
//         VyContext::destroy(m_DefaultTextureImage, m_DefaultTextureImageAllocation);
//     }


//     void VyMaterial::createDefaultTexture() 
//     {
//         // Create a simple 1x1 white texture as default
//         const U32 texWidth    = 1;
//         const U32 texHeight   = 1;
//         const U32 texChannels = 4;

//         unsigned char pixels[4] = { 255, 255, 255, 255 }; // White pixel

//         VkDeviceSize imageSize = texWidth * texHeight * texChannels;

//         // Create staging buffer
//         VyBuffer stagingBuffer{ VyBuffer::stagingBuffer("default_mat_tex", imageSize) };

//         stagingBuffer.singleWrite(pixels);

//         // Create image
//         VkImageCreateInfo imageInfo{ VKInit::imageCreateInfo() };
//         {
//             imageInfo.imageType     = VK_IMAGE_TYPE_2D;
//             imageInfo.extent.width  = texWidth;
//             imageInfo.extent.height = texHeight;
//             imageInfo.extent.depth  = 1;
//             imageInfo.mipLevels     = 1;
//             imageInfo.arrayLayers   = 1;
//             imageInfo.format        = VK_FORMAT_R8G8B8A8_SRGB;
//             imageInfo.tiling        = VK_IMAGE_TILING_OPTIMAL;
//             imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
//             imageInfo.usage         = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
//             imageInfo.samples       = VK_SAMPLE_COUNT_1_BIT;
//             imageInfo.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;
//         }

// 		VmaAllocationCreateInfo allocInfo{};
//         {
//             allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
//         }

// 		VyContext::device().createImage(m_DefaultTextureImage, m_DefaultTextureImageAllocation, imageInfo, allocInfo);

//         // Transition image layout and copy buffer to image
//         VyContext::device().transitionImageLayout(
//             m_DefaultTextureImage, 
//             VK_FORMAT_R8G8B8A8_SRGB,
//             VK_IMAGE_LAYOUT_UNDEFINED, 
//             VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
//         );

//         VyContext::device().copyBufferToImage(
//             stagingBuffer, 
//             m_DefaultTextureImage, 
//             texWidth, 
//             texHeight, 
//             1
//         );
        
//         VyContext::device().transitionImageLayout(
//             m_DefaultTextureImage, 
//             VK_FORMAT_R8G8B8A8_SRGB,
//             VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
//             VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
//         );

//         // Create image view
//         createTextureImageView(m_DefaultTextureImage, m_DefaultTextureImageView);
        
//         // Create sampler
//         createTextureSampler(m_DefaultTextureSampler);

//         VY_INFO_TAG("VyMaterial", "Successfully created Default Texture.");
//     }


//     void VyMaterial::loadAlbedoTexture(const TString& filepath) 
//     {
//         try 
//         {
//             createTextureImage(filepath, m_AlbedoTextureImage, m_AlbedoTextureImageAllocation);
//             createTextureImageView(m_AlbedoTextureImage, m_AlbedoTextureImageView);
//             createTextureSampler(m_AlbedoTextureSampler);
            
//             m_HasAlbedoTexture = true;

//             VY_INFO_TAG("VyMaterial", "Successfully loaded Albedo Texture: {}", filepath);
//         } 
//         catch (const std::exception& e) 
//         {
//             VY_ERROR_TAG("VyMaterial", "Failed to load Albedo Texture: {}\n{}", filepath, e.what());

//             m_HasAlbedoTexture = false;

//             // Make the object neon pink if it cant load or find the material
//             m_FailedAlbedo = true;
//             m_Data.Albedo  = Vec3(1.0f, 0.0f, 1.0f); // pink
//         }
//     }


//     void VyMaterial::loadNormalTexture(const TString& filepath) 
//     {
//         try 
//         {
//             createTextureImage(filepath, m_NormalTextureImage, m_NormalTextureImageAllocation);
//             createTextureImageView(m_NormalTextureImage, m_NormalTextureImageView);
//             createTextureSampler(m_NormalTextureSampler);

//             m_HasNormalTexture = true;

//             VY_INFO_TAG("VyMaterial", "Successfully loaded Normal Texture: {}", filepath);
//         } 
//         catch (const std::exception& e) 
//         {
//             VY_ERROR_TAG("VyMaterial", "Failed to load Normal Texture: {}\n{}", filepath, e.what());

//             m_HasNormalTexture = false;
//         }
//     }

    
//     void VyMaterial::loadRoughnessMap(const TString& filepath) 
//     {
//         try 
//         {
//             createTextureImage(filepath, m_RoughnessTextureImage, m_RoughnessTextureImageAllocation);
//             createTextureImageView(m_RoughnessTextureImage, m_RoughnessTextureImageView);
//             createTextureSampler(m_RoughnessTextureSampler);

//             m_HasRoughnessTexture = true;

//             VY_INFO_TAG("VyMaterial", "Successfully loaded Roughness Texture: {}", filepath);
//         } 
//         catch (const std::exception& e) 
//         {
//             VY_ERROR_TAG("VyMaterial", "Failed to load Roughness Texture: {}\n{}", filepath, e.what());

//             m_HasRoughnessTexture = false;
//         }
//     }

    
//     void VyMaterial::loadMetallicMap(const TString& filepath) 
//     {
//         try 
//         {
//             createTextureImage(filepath, m_MetallicTextureImage, m_MetallicTextureImageAllocation);
//             createTextureImageView(m_MetallicTextureImage, m_MetallicTextureImageView);
//             createTextureSampler(m_MetallicTextureSampler);

//             m_HasMetallicTexture = true;

//             VY_INFO_TAG("VyMaterial", "Successfully loaded Metallic Texture: {}", filepath);
//         } 
//         catch (const std::exception& e) 
//         {
//             VY_ERROR_TAG("VyMaterial", "Failed to load Metallic Texture: {}\n{}", filepath, e.what());

//             m_HasMetallicTexture = false;
//         }
//     }


//     void VyMaterial::createTextureImage(const TString& filepath, VkImage& image, VmaAllocation& allocation) 
//     {
//         TString fullPath = ASSETS_DIR + filepath;
        
// 		int stbWidth    = 0;
// 		int stbHeight   = 0;
// 		int stbChannels = 0;

// 		// Load Texture data.
// 		auto pixels = stbi_load(fullPath.c_str(), &stbWidth, &stbHeight, &stbChannels, STBI_rgb_alpha);
		
// 		U32 width  = static_cast<U32>(stbWidth);
// 		U32 height = static_cast<U32>(stbHeight);

// 		VkDeviceSize imageSize = 4 * static_cast<VkDeviceSize>(stbWidth) * static_cast<VkDeviceSize>(stbHeight);

//         if (!pixels)
// 		{
// 			VY_THROW_RUNTIME_ERROR("Failed to load texture image: " + fullPath + " - " + stbi_failure_reason());
// 		}

//         // Create staging buffer
//         VyBuffer stagingBuffer{ VyBuffer::stagingBuffer("mat_tex", imageSize) };

//         stagingBuffer.singleWrite(pixels);

//         stbi_image_free(pixels);

//         // Create image
//         VkImageCreateInfo imageInfo{ VKInit::imageCreateInfo() };
//         {
//             imageInfo.imageType     = VK_IMAGE_TYPE_2D;
//             imageInfo.extent.width  = static_cast<U32>(stbWidth);
//             imageInfo.extent.height = static_cast<U32>(stbHeight);
//             imageInfo.extent.depth  = 1;
//             imageInfo.mipLevels     = 1;
//             imageInfo.arrayLayers   = 1;
//             imageInfo.format        = VK_FORMAT_R8G8B8A8_SRGB;
//             imageInfo.tiling        = VK_IMAGE_TILING_OPTIMAL;
//             imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
//             imageInfo.usage         = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
//             imageInfo.samples       = VK_SAMPLE_COUNT_1_BIT;
//             imageInfo.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;
//         }

// 		VmaAllocationCreateInfo allocInfo{};
//         {
//             allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
//         }

// 		VyContext::device().createImage(image, allocation, imageInfo, allocInfo);

//         // Transition image layout and copy buffer to image.
//         VyContext::device().transitionImageLayout(
//             image, 
//             VK_FORMAT_R8G8B8A8_SRGB,
//             VK_IMAGE_LAYOUT_UNDEFINED, 
//             VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
//         );
        
//         VyContext::device().copyBufferToImage(
//             stagingBuffer, 
//             image, 
//             width, 
//             height,
//             1
//         );

//         VyContext::device().transitionImageLayout(
//             image, 
//             VK_FORMAT_R8G8B8A8_SRGB,
//             VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
//             VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
//         );
//     }


//     void VyMaterial::createTextureImageView(VkImage image, VkImageView& imageView) 
//     {
//         VkImageViewCreateInfo viewInfo{ VKInit::imageViewCreateInfo() };
//         {
//             viewInfo.image    = image;
//             viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
//             viewInfo.format   = VK_FORMAT_R8G8B8A8_SRGB;

//             viewInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
//             viewInfo.subresourceRange.baseMipLevel   = 0;
//             viewInfo.subresourceRange.levelCount     = 1;
//             viewInfo.subresourceRange.baseArrayLayer = 0;
//             viewInfo.subresourceRange.layerCount     = 1;
//         }

//         if (vkCreateImageView(VyContext::device(), &viewInfo, nullptr, &imageView) != VK_SUCCESS) 
//         {
//             VY_THROW_RUNTIME_ERROR("Failed to create texture image view!");
//         }
//     }

    
//     void VyMaterial::createTextureSampler(VkSampler& sampler) 
//     {
//         VkPhysicalDeviceProperties properties = VyContext::device().properties();

//         VkSamplerCreateInfo samplerInfo{ VKInit::samplerCreateInfo() };
//         {
//             samplerInfo.magFilter               = VK_FILTER_LINEAR;
//             samplerInfo.minFilter               = VK_FILTER_LINEAR;
//             samplerInfo.addressModeU            = VK_SAMPLER_ADDRESS_MODE_REPEAT;
//             samplerInfo.addressModeV            = VK_SAMPLER_ADDRESS_MODE_REPEAT;
//             samplerInfo.addressModeW            = VK_SAMPLER_ADDRESS_MODE_REPEAT;
//             samplerInfo.anisotropyEnable        = VK_TRUE;
//             samplerInfo.maxAnisotropy           = properties.limits.maxSamplerAnisotropy;
//             samplerInfo.borderColor             = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
//             samplerInfo.unnormalizedCoordinates = VK_FALSE;
//             samplerInfo.compareEnable           = VK_FALSE;
//             samplerInfo.compareOp               = VK_COMPARE_OP_ALWAYS;
//             samplerInfo.mipmapMode              = VK_SAMPLER_MIPMAP_MODE_LINEAR;
//         }

//         if (vkCreateSampler(VyContext::device(), &samplerInfo, nullptr, &sampler) != VK_SUCCESS) 
//         {
//             VY_THROW_RUNTIME_ERROR("Failed to create texture sampler!");
//         }
//     }


//     void VyMaterial::updateDescriptorSet(VyDescriptorSetLayout& setLayout, VyDescriptorPool& pool) 
//     {
//         // Create descriptor set if not already created.
//         if (m_DescriptorSet == VK_NULL_HANDLE) 
//         {
//             if (!pool.allocate( setLayout.handle(), m_DescriptorSet )) 
//             {
//                 VY_THROW_RUNTIME_ERROR("Failed to allocate material descriptor set!");
//             }

//             VKDbg::setObjectName(VK_OBJECT_TYPE_DESCRIPTOR_SET, (U64)m_DescriptorSet, "material_desc_set");
//         }

//         VkDescriptorImageInfo albedoImageInfo{};
//         {
//             albedoImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
//             albedoImageInfo.imageView   = m_HasAlbedoTexture ? m_AlbedoTextureImageView : m_DefaultTextureImageView;
//             albedoImageInfo.sampler     = m_HasAlbedoTexture ? m_AlbedoTextureSampler   : m_DefaultTextureSampler;
//         }

//         VkDescriptorImageInfo normalImageInfo{};
//         {
//             normalImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
//             normalImageInfo.imageView   = m_HasNormalTexture ? m_NormalTextureImageView : m_DefaultTextureImageView;
//             normalImageInfo.sampler     = m_HasNormalTexture ? m_NormalTextureSampler   : m_DefaultTextureSampler;
//         }

//         VkDescriptorImageInfo roughnessImageInfo{};
//         {
//             roughnessImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
//             roughnessImageInfo.imageView   = m_HasRoughnessTexture ? m_RoughnessTextureImageView : m_DefaultTextureImageView;
//             roughnessImageInfo.sampler     = m_HasRoughnessTexture ? m_RoughnessTextureSampler   : m_DefaultTextureSampler;
//         }

//         VkDescriptorImageInfo metallicImageInfo{};
//         {
//             metallicImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
//             metallicImageInfo.imageView   = m_HasMetallicTexture ? m_MetallicTextureImageView : m_DefaultTextureImageView;
//             metallicImageInfo.sampler     = m_HasMetallicTexture ? m_MetallicTextureSampler   : m_DefaultTextureSampler;
//         }

//         VyDescriptorWriter{ setLayout, pool }
//             .writeImage(0, &albedoImageInfo)
//             .writeImage(1, &normalImageInfo)
//             .writeImage(2, &roughnessImageInfo)
//             .writeImage(3, &metallicImageInfo)
//             .update( m_DescriptorSet );
//     }
// }

// // namespace Vy
// // {
// //     VyMaterial::VyMaterial()
// //     {
// //         createDefaultTexture();
// //     }


// //     VyMaterial::~VyMaterial() 
// //     {
// //         // m_DefaultTexture.reset();
// //     }


// //     void VyMaterial::createDefaultTexture() 
// //     {
// //         m_DefaultTexture = VyTexture::createWhiteTexture();

// //         VY_INFO_TAG("VyMaterial", "Created default texture.");
// //     }


// //     void VyMaterial::loadAlbedoTexture(const TString& filepath) 
// //     {
// //         try 
// //         {
// //             m_AlbedoTexture = VyTexture::createFromFile( filepath );
// //         } 
// //         catch (const std::exception& e) 
// //         {
// //             VY_ERROR_TAG("VyMaterial", "Failed to load Albedo Texture: {}\n{}", filepath, e.what());

// //             // Make the object neon pink if it cant load or find the material
// //             m_FailedAlbedo = true;
// //             m_Data.Albedo  = Vec3(1.0f, 0.0f, 1.0f); // pink
// //         }
// //     }


// //     void VyMaterial::loadNormalTexture(const TString& filepath) 
// //     {
// //         try 
// //         {
// //             m_NormalTexture = VyTexture::createFromFile( filepath, false );
// //         } 
// //         catch (const std::exception& e) 
// //         {
// //             VY_ERROR_TAG("VyMaterial", "Failed to load Normal Texture: {}\n{}", filepath, e.what());
// //         }
// //     }

    
// //     void VyMaterial::loadRoughnessMap(const TString& filepath) 
// //     {
// //         try 
// //         {
// //             m_RoughnessTexture = VyTexture::createFromFile( filepath );
// //         } 
// //         catch (const std::exception& e) 
// //         {
// //             VY_ERROR_TAG("VyMaterial", "Failed to load Roughness Texture: {}\n{}", filepath, e.what());
// //         }
// //     }

    
// //     void VyMaterial::loadMetallicMap(const TString& filepath) 
// //     {
// //         try 
// //         {
// //             m_MetallicTexture = VyTexture::createFromFile( filepath );
// //         } 
// //         catch (const std::exception& e) 
// //         {
// //             VY_ERROR_TAG("VyMaterial", "Failed to load Metallic Texture: {}\n{}", filepath, e.what());
// //         }
// //     }


// //     void VyMaterial::updateDescriptorSet(VyDescriptorSetLayout& setLayout, VyDescriptorPool& pool) 
// //     {
// //         // Create descriptor set if not already created.
// //         if (m_DescriptorSet == VK_NULL_HANDLE)
// //         {
// //             if (!pool.allocate( setLayout.handle(), m_DescriptorSet )) 
// //             {
// //                 VY_THROW_RUNTIME_ERROR("Failed to allocate material descriptor set!");
// //             }
// //             else {
// //                 VY_INFO_TAG("VyMaterial", "Successdully allocated descriptor set");
// //             }
// //         }

// //         VkDescriptorImageInfo albedoImageInfo = m_AlbedoTexture 
// //             ? m_AlbedoTexture ->descriptorImageInfo() 
// //             : m_DefaultTexture->descriptorImageInfo();

// //         VkDescriptorImageInfo normalImageInfo = m_NormalTexture 
// //             ? m_NormalTexture ->descriptorImageInfo() 
// //             : m_DefaultTexture->descriptorImageInfo();

// //         VkDescriptorImageInfo roughnessImageInfo = m_RoughnessTexture 
// //             ? m_RoughnessTexture->descriptorImageInfo() 
// //             : m_DefaultTexture  ->descriptorImageInfo();

// //         VkDescriptorImageInfo metallicImageInfo = m_MetallicTexture 
// //             ? m_MetallicTexture->descriptorImageInfo() 
// //             : m_DefaultTexture ->descriptorImageInfo();

// //         VyDescriptorWriter{ setLayout, *VyContext::globalPool() }
// //             .writeImage( 0, &albedoImageInfo    )
// //             .writeImage( 1, &normalImageInfo    )
// //             .writeImage( 2, &roughnessImageInfo )
// //             .writeImage( 3, &metallicImageInfo  )
// //             .update( m_DescriptorSet );
// //     }
// // }