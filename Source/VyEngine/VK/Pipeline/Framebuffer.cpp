#include <VyEngine/VK/Pipeline/Framebuffer.h>

#include <VyEngine/VK/Context.h>

#include <VyEngine/Globals.h>
#include "stb_image.h"
namespace Vy
{
//     VyTexture2D::VyTexture2D()
//     {
//         m_DescriptorSetLayout = VyDescriptorSetLayout::Builder{}
//             .addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
//             .buildPtr();

//         // DescriptorAllocator::Allocate(m_DescriptorSetLayout->GetDescriptorSetLayout(), descriptorSet);
//     };

    
//     VyTexture2D::~VyTexture2D() 
//     {
//         destroyAll();
//     }


//     void VyTexture2D::destroyAll() 
//     {
//         // if (textureImageView != VK_NULL_HANDLE) {
//         //     vkDestroyImageView(m_Device.GetDevice(), textureImageView, nullptr);
//         //     textureImageView = VK_NULL_HANDLE;
//         // }
//         // if (textureImage != VK_NULL_HANDLE) {
//         //     vkDestroyImage(m_Device.GetDevice(), textureImage, nullptr);
//         //     textureImage = VK_NULL_HANDLE;
//         // }
//         // if (textureSampler != VK_NULL_HANDLE) {
//         //     vkDestroySampler(m_Device.GetDevice(), textureSampler, nullptr);
//         //     textureSampler = VK_NULL_HANDLE;
//         // }
//         // if (textureImageMemory != VK_NULL_HANDLE) {
//         //     vkFreeMemory(m_Device.GetDevice(), textureImageMemory, nullptr);
//         //     textureImageMemory = VK_NULL_HANDLE;
//         // }
//     }

//     void VyTexture2D::destroy() {
//         // VkDevice device = m_Device.GetDevice();
//         // if (textureSampler != VK_NULL_HANDLE) {
//         //     vkDestroySampler(device, textureSampler, nullptr);
//         //     textureSampler = VK_NULL_HANDLE;
//         // }
//         // if (textureImageView != VK_NULL_HANDLE) {
//         //     vkDestroyImageView(device, textureImageView, nullptr);
//         //     textureImageView = VK_NULL_HANDLE;
//         // }
//         // if (textureImage != VK_NULL_HANDLE) {
//         //     vkDestroyImage(device, textureImage, nullptr);
//         //     textureImage = VK_NULL_HANDLE;
//         // }
//         // if (textureImageMemory != VK_NULL_HANDLE) {
//         //     vkFreeMemory(device, textureImageMemory, nullptr);
//         //     textureImageMemory = VK_NULL_HANDLE;
//         // }

//         // if (descriptorSet != VK_NULL_HANDLE) {
//         //     std::vector<VkDescriptorSet> sets{ descriptorSet };
//         //     DescriptorAllocator::Release(sets);
//         //     descriptorSet = VK_NULL_HANDLE;
//         // }

//         // if (m_DescriptorSetLayout) {
//         //     m_DescriptorSetLayout.reset();
//         // }
//     }


//     void VyTexture2D::createHDRTexture(const TString &filepath, VkFormat format) 
//     {
//         createHDRTextureImage(filepath);
//         createHDRTextureImageView(format);
//         createHDRTextureSampler();
//         writeToDescriptorSet();
//     }

//     void VyTexture2D::createCubeMap(U32 width, U32 height, VkFormat format, VkImageUsageFlags usage) 
//     {
//         createCubemapImage(width, height, format, usage);
//         createCubemapImageView(format);
//         createCubemapSampler();
//         writeToDescriptorSet();
//     }

//     void VyTexture2D::GenerateMipmap(VkImage image, VkFormat imageFormat, int32_t texW, int32_t texH, U32 mipLevels)
//     {
//         // VkFormatProperties formatProperties;
//         // vkGetPhysicalDeviceFormatProperties(m_Device.GetPhysicalDevice(), imageFormat, &formatProperties);
//         // if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) 
//         // {
//         //     throw std::runtime_error("Texture image format does not support linear blitting!");
//         // }

//         // VkCommandBuffer commandBuffer = m_Device.BeginSingleTimeCommands();

//         // VkImageMemoryBarrier barrier{};
//         // barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
//         // barrier.image = image;
//         // barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
//         // barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
//         // barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//         // barrier.subresourceRange.baseArrayLayer = 0;
//         // barrier.subresourceRange.layerCount = 1;
//         // barrier.subresourceRange.levelCount = 1;

//         // int32_t mipWidth = texW;
//         // int32_t mipHeight = texH;

//         // for (U32 i = 1; i < mipLevels; i++) {
//         //     barrier.subresourceRange.baseMipLevel = i - 1;
//         //     barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
//         //     barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
//         //     barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
//         //     barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

//         //     vkCmdPipelineBarrier(commandBuffer,
//         //         VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
//         //         0, nullptr,
//         //         0, nullptr,
//         //         1, &barrier);

//         //     VkImageBlit blit{};
//         //     blit.srcOffsets[0] = {0, 0, 0};
//         //     blit.srcOffsets[1] = {mipWidth, mipHeight, 1};
//         //     blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//         //     blit.srcSubresource.mipLevel = i - 1;
//         //     blit.srcSubresource.baseArrayLayer = 0;
//         //     blit.srcSubresource.layerCount = 1;
//         //     blit.dstOffsets[0] = {0, 0, 0};
//         //     blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
//         //     blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//         //     blit.dstSubresource.mipLevel = i;
//         //     blit.dstSubresource.baseArrayLayer = 0;
//         //     blit.dstSubresource.layerCount = 1;

//         //     vkCmdBlitImage(commandBuffer,
//         //         image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
//         //         image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
//         //         1, &blit,
//         //         VK_FILTER_LINEAR);

//         //     barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
//         //     barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
//         //     barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
//         //     barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

//         //     vkCmdPipelineBarrier(commandBuffer,
//         //         VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
//         //         0, nullptr,
//         //         0, nullptr,
//         //         1, &barrier);

//         //     if (mipWidth > 1) mipWidth /= 2;
//         //     if (mipHeight > 1) mipHeight /= 2;
//         // }

//         // barrier.subresourceRange.baseMipLevel = mipLevels - 1;
//         // barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
//         // barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
//         // barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
//         // barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

//         // vkCmdPipelineBarrier(commandBuffer,
//         //     VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
//         //     0, nullptr,
//         //     0, nullptr,
//         //     1, &barrier);

//         // m_Device.EndSingleTimeCommands(commandBuffer);
//     }

//     // void VyTexture2D::createMipMappedCubemap(U32 width, U32 height, VkFormat format, VkImageUsageFlags usage) 
//     // {
//     //     m_MipLevels = static_cast<U32>(std::floor(std::log2(std::max(width, height)))) + 1;
//     //     m_Format = format;

//     //     VkImageCreateInfo imageInfo{};
//     //     imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
//     //     imageInfo.imageType = VK_IMAGE_TYPE_2D;
//     //     imageInfo.format = VK_FORMAT_R32G32B32A32_SFLOAT;
//     //     imageInfo.extent.width = width;
//     //     imageInfo.extent.height = height;
//     //     imageInfo.extent.depth = 1;
//     //     imageInfo.mipLevels = m_MipLevels;
//     //     imageInfo.arrayLayers = 6;
//     //     imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
//     //     imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
//     //     imageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
//     //     imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
//     //     imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

//     //     if (vkCreateImage(m_Device.GetDevice(), &imageInfo, nullptr, &textureImage) != VK_SUCCESS) {
//     //         throw std::runtime_error("Failed to create cubemap image!");
//     //     }

//     //     VkMemoryRequirements memRequirements;
//     //     vkGetImageMemoryRequirements(m_Device.GetDevice(), textureImage, &memRequirements);

//     //     VkMemoryAllocateInfo allocInfo{};
//     //     allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
//     //     allocInfo.allocationSize = memRequirements.size;
//     //     allocInfo.memoryTypeIndex = m_Device.FindMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

//     //     if (vkAllocateMemory(m_Device.GetDevice(), &allocInfo, nullptr, &textureImageMemory) != VK_SUCCESS) {
//     //         throw std::runtime_error("Failed to allocate cubemap memory!");
//     //     }

//     //     vkBindImageMemory(m_Device.GetDevice(), textureImage, textureImageMemory, 0);

//     //     VkImageViewCreateInfo viewInfo{};
//     //     viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
//     //     viewInfo.image = textureImage;
//     //     viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
//     //     viewInfo.format = VK_FORMAT_R32G32B32A32_SFLOAT;
//     //     viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//     //     viewInfo.subresourceRange.baseMipLevel = 0;
//     //     viewInfo.subresourceRange.levelCount = m_MipLevels;
//     //     viewInfo.subresourceRange.baseArrayLayer = 0;
//     //     viewInfo.subresourceRange.layerCount = 6;

//     //     if (vkCreateImageView(m_Device.GetDevice(), &viewInfo, nullptr, &textureImageView) != VK_SUCCESS) {
//     //         throw std::runtime_error("Failed to create cubemap image view!");
//     //     }

//     //     VkSamplerCreateInfo samplerInfo{};
//     //     samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
//     //     samplerInfo.magFilter = VK_FILTER_LINEAR;
//     //     samplerInfo.minFilter = VK_FILTER_LINEAR;
//     //     samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
//     //     samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
//     //     samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
//     //     samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
//     //     samplerInfo.minLod = 0.0f;
//     //     samplerInfo.maxLod = static_cast<float>(m_MipLevels);
//     //     samplerInfo.mipLodBias = 0.0f;
//     //     samplerInfo.anisotropyEnable = VK_FALSE;

//     //     if (vkCreateSampler(m_Device.GetDevice(), &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS) {
//     //         throw std::runtime_error("Failed to create cubemap sampler!");
//     //     }

//     //     writeToDescriptorSet();

//     // }


//     void VyTexture2D::createTexture(const TString& filepath, VkFormat format) 
//     {
//         createTextureImage(filepath);
//         createTextureImageView(format);
//         createTextureSampler();
//         writeToDescriptorSet();
//     }

//     void VyTexture2D::createTexture(const U32 width, const U32 height, const VkFormat format, const VkImageUsageFlags usage, const VkSampleCountFlagBits samples) 
//     {
//         m_Image = VyImage::Builder{}
//             .setImageType  (VK_IMAGE_TYPE_2D)
//             .setFormat     (format)
//             .setExtent     (width, height)
//             .setLevels     (m_MipLevels)
// 			.setLayers     (1)
// 			.setSamples    (samples)
//             .setTiling     (VK_IMAGE_TILING_OPTIMAL)
// 			.setLayout     (VK_IMAGE_LAYOUT_UNDEFINED)
//             .setSharing    (VK_SHARING_MODE_EXCLUSIVE)
//             .setUsage      (usage | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
//             .setMemoryUsage(VMA_MEMORY_USAGE_AUTO)
//         	.build();

//         createTextureImageView(format);
//         createTextureSampler();
//         writeToDescriptorSet();
//     }


//     void VyTexture2D::writeToDescriptorSet() 
//     {
//         VkDescriptorImageInfo imageInfo{};
//         imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
//         imageInfo.imageView = m_View.handle();
//         imageInfo.sampler   = m_Sampler.handle();

//         VyDescriptorWriter writer( *m_DescriptorSetLayout, *VyContext::globalPool() );
//         writer.writeImage(0, &imageInfo);
//         writer.update( m_DescriptorSet );
//     }

//     VkDescriptorImageInfo VyTexture2D::descriptorImageInfo() const 
//     {
//         VkDescriptorImageInfo imageInfo{};
//         imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
//         imageInfo.imageView = m_View.handle();
//         imageInfo.sampler   = m_Sampler.handle();
//         return imageInfo;
//     }


//     void VyTexture2D::createCubemapImage(U32 width, U32 height, VkFormat format, VkImageUsageFlags usage) 
//     {
//         m_Format = format;

//         m_Image = VyImage::Builder{}
//             .setImageType  (VK_IMAGE_TYPE_2D)
//             .setFormat     (format)
//             .setExtent     (width, height)
//             .setLevels     (1)
// 			.setLayers     (6) // Faces
// 			.setSamples    (VK_SAMPLE_COUNT_1_BIT)
//             .setTiling     (VK_IMAGE_TILING_OPTIMAL)
// 			.setLayout     (VK_IMAGE_LAYOUT_UNDEFINED)
//             .setSharing    (VK_SHARING_MODE_EXCLUSIVE)
//             .setUsage      (usage | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
//             .setMemoryUsage(VMA_MEMORY_USAGE_AUTO)
//             .setFlags      (VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT)
//         	.build();

//         m_Image.transitionLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
//     }


//     void VyTexture2D::createCubemapImageView(VkFormat format) 
//     {
//         m_View = VyImageView::Builder{}
//             .setViewType(VK_IMAGE_VIEW_TYPE_CUBE)
//             .setFormat  (format)
//             .setAspect  (VKUtil::aspectFlags(format))
//             .setLevels  (0, 1)
//             .setLayers  (0, 6) // 6 Faces
//         	.build( m_Image );
//     }


//     void VyTexture2D::createCubemapSampler() 
//     {
//         m_Sampler = VySampler::Builder{}
//             .setFilters      (VK_FILTER_LINEAR)
//             .setMipmapMode   (VK_SAMPLER_MIPMAP_MODE_LINEAR)
//             .setWrap         (VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE)
//             .setBorder       (VK_BORDER_COLOR_INT_OPAQUE_BLACK)
// 			.enableAnisotropy(true)
//         	.build();
//     }


//     void VyTexture2D::createTextureImageView(VkFormat format) 
//     {
//         m_Format = format;

//         m_View = VyImageView::Builder{}
//             .setViewType(VK_IMAGE_VIEW_TYPE_2D)
//             .setFormat  (format)
//             .setAspect  (VKUtil::aspectFlags(format))
//             .setLevels  (0, 1)
//             .setLayers  (0, 1)
//         	.build( m_Image );
//     }


//     void VyTexture2D::createTextureSampler() 
//     {
//         m_Sampler = VySampler::Builder{}
//             .setFilters      (VK_FILTER_LINEAR)
//             .setMipmapMode   (VK_SAMPLER_MIPMAP_MODE_LINEAR)
//             .setWrap         (VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE)
//             .setBorder       (VK_BORDER_COLOR_INT_OPAQUE_BLACK)
// 			.enableAnisotropy(true)
//             .setLodRange     (0.0f, 1.0f)
//             .setMipLodBias   (0.0f)
//         	.build();
//     }


//     void VyTexture2D::createSolidColorCubemap(Vec4 color)
//     {
//         const int texSize = 1;

//         U8 pixelData[4] = {
//             static_cast<U8>( color.r * 255.0f ),
//             static_cast<U8>( color.g * 255.0f ),
//             static_cast<U8>( color.b * 255.0f ),
//             static_cast<U8>( color.a * 255.0f )
//         };

//         VkDeviceSize imageSize = sizeof(pixelData) * 6;

//         VyBuffer stagingBuffer{ VyBuffer::stagingBuffer("solid_color_cubemap", imageSize ) };

//         for (int i = 0; i < 6; ++i) 
//         {
//             stagingBuffer.singleWrite( pixelData );
//         }

//         m_Format = VK_FORMAT_R32G32B32A32_SFLOAT;

//         m_Image = VyImage::Builder{}
//             .setImageType  (VK_IMAGE_TYPE_2D)
//             .setFormat     (m_Format)
//             .setExtent     (static_cast<U32>(texSize), static_cast<U32>(texSize))
//             .setLevels     (1)
// 			.setLayers     (6) // Faces
// 			.setSamples    (VK_SAMPLE_COUNT_1_BIT)
//             .setTiling     (VK_IMAGE_TILING_OPTIMAL)
// 			.setLayout     (VK_IMAGE_LAYOUT_UNDEFINED)
//             .setSharing    (VK_SHARING_MODE_EXCLUSIVE)
//             .setUsage      (VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
//             .setMemoryUsage(VMA_MEMORY_USAGE_AUTO)
//             .setFlags      (VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT)
//         	.build();

//         m_Image.transitionLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

//         m_Image.copyFrom(stagingBuffer, true);

//         createCubemapImageView(VK_FORMAT_R32G32B32A32_SFLOAT);
//         createCubemapSampler();
//     }


//     void VyTexture2D::createSolidColorTexture(Vec4 color) 
//     {
//         U8 pixelData[4] = {
//             static_cast<U8>( color.r * 255.0f ),
//             static_cast<U8>( color.g * 255.0f ),
//             static_cast<U8>( color.b * 255.0f ),
//             static_cast<U8>( color.a * 255.0f )
//         };

//         VkDeviceSize imageSize = sizeof(pixelData);

//         VyBuffer stagingBuffer{ VyBuffer::stagingBuffer("solid_color_texture", imageSize ) };

//         stagingBuffer.write( pixelData );

//         m_Image = VyImage::Builder{}
//             .setImageType  (VK_IMAGE_TYPE_2D)
//             .setFormat     (VK_FORMAT_R8G8B8A8_UNORM)
//             .setExtent     (1, 1)
//             .setLevels     (m_MipLevels)
// 			.setLayers     (1)
// 			.setSamples    (VK_SAMPLE_COUNT_1_BIT)
//             .setTiling     (VK_IMAGE_TILING_OPTIMAL)
// 			.setLayout     (VK_IMAGE_LAYOUT_UNDEFINED)
//             .setSharing    (VK_SHARING_MODE_EXCLUSIVE)
//             .setUsage      (VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
//             .setMemoryUsage(VMA_MEMORY_USAGE_AUTO)
//         	.build();

//         m_Image.transitionLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

//         m_Image.copyFrom(stagingBuffer, true);

//         createTextureImageView(VK_FORMAT_R8G8B8A8_UNORM);
//         createTextureSampler();
//     }


//     void VyTexture2D::useFallbackTextures(ETextureType type) 
//     {
//         switch (type) {
//             case ETextureType::Albedo:
//                 createSolidColorTexture({1.0f, 1.0f, 1.0f, 1.0f});
//                 break;
//             case ETextureType::Normal:
//                 createSolidColorTexture({0.5f, 0.5f, 1.0f, 1.0f});
//                 break;
//             case ETextureType::Emissive:
//                 createSolidColorTexture({1.0f, 1.0f, 1.0f, 1.0f});
//                 break;
//             case ETextureType::MetallicRoughness:
//                 createSolidColorTexture({1.0f, 0.5f, 0.0f, 1.0f});
//                 break;
//             case ETextureType::AO:
//                 createSolidColorTexture({1.0f, 1.0f, 1.0f, 1.0f});
//                 break;
//             case ETextureType::Cubemap:
//                 createSolidColorCubemap({1.0f,1.0f,1.0f,1.0f});
//             default:
//                 break;
//         }

//         writeToDescriptorSet();
//     }



//     void VyTexture2D::createHDRTextureImage(const TString& filepath) 
//     {
//         stbi_set_flip_vertically_on_load(false);  // Disable automatic flip by stb_image

//         int texW, texH, texChannels;
//         float* pPixels = stbi_loadf(filepath.c_str(), &texW, &texH, &texChannels, 4);
//         if (!pPixels) {
//             throw std::runtime_error("failed to load HDR texture image!");
//         }

//         // fixes the stbi_set_flip_vertically_on_load flipping my UVs
//         float* pFlippedPixels = new float[ texW * texH * 4 ];

//         for (int y = 0; y < texH; ++y) 
//         {
//             for (int x = 0; x < texW; ++x) 
//             {
//                 int originalIdx = (y * texW + x) * 4;
//                 int flippedIdx  = ((texH - 1 - y) * texW + x) * 4;

//                 pFlippedPixels[flippedIdx    ] = pPixels[originalIdx    ]; // R
//                 pFlippedPixels[flippedIdx + 1] = pPixels[originalIdx + 1]; // G
//                 pFlippedPixels[flippedIdx + 2] = pPixels[originalIdx + 2]; // B
//                 pFlippedPixels[flippedIdx + 3] = pPixels[originalIdx + 3]; // A
//             }
//         }

//         VkDeviceSize imageSize = texW * texH * 4 * sizeof(float);

//         VyBuffer stagingBuffer{ VyBuffer::stagingBuffer("hdri_texture", imageSize ) };

//         stagingBuffer.write( pFlippedPixels );

//         delete[] pFlippedPixels;

//         stbi_image_free(pPixels);

//         m_Image = VyImage::Builder{}
//             .setImageType  (VK_IMAGE_TYPE_2D)
//             .setFormat     (VK_FORMAT_R32G32B32A32_SFLOAT)
//             .setExtent     (static_cast<U32>(texW), static_cast<U32>(texH))
//             .setLevels     (m_MipLevels)
// 			.setLayers     (1)
// 			.setSamples    (VK_SAMPLE_COUNT_1_BIT)
//             .setTiling     (VK_IMAGE_TILING_OPTIMAL)
// 			.setLayout     (VK_IMAGE_LAYOUT_UNDEFINED)
//             .setSharing    (VK_SHARING_MODE_EXCLUSIVE)
//             .setUsage      (VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
//             .setMemoryUsage(VMA_MEMORY_USAGE_AUTO)
//         	.build();

//         m_Image.upload(stagingBuffer);

//         // m_Image.transitionLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

//         // m_Image.copyFrom(stagingBuffer, true);

//         // GenerateMipmap(textureImage, VK_FORMAT_R32G32B32A32_SFLOAT, texW, texH, m_MipLevels);
//     }

//     void VyTexture2D::createHDRTextureImageView(VkFormat format) 
//     {
//         m_Format = format;

//         m_View = VyImageView::Builder{}
//             .setViewType(VK_IMAGE_VIEW_TYPE_2D)
//             .setFormat  (format)
//             .setAspect  (VK_IMAGE_ASPECT_COLOR_BIT)
//             .setLevels  (0, 1)
//             .setLayers  (0, 1)
//         	.build( m_Image );
//     }

//     void VyTexture2D::createHDRTextureSampler() 
//     {
//         m_Sampler = VySampler::Builder{}
//             .setFilters      (VK_FILTER_LINEAR)
//             .setMipmapMode   (VK_SAMPLER_MIPMAP_MODE_LINEAR)
//             .setWrap         (VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE)
//             .setBorder       (VK_BORDER_COLOR_INT_OPAQUE_BLACK)
// 			.enableAnisotropy(true)
//             .setLodRange     (0.0f, 1.0f)
//             .setMipLodBias   (0.0f)
//         	.build();
//     }

    
//     void VyTexture2D::createTextureImage(const TString& filepath) 
//     {
//         int texW, texH, texChannels;
//         stbi_uc* pPixels = stbi_load(filepath.c_str(), &texW, &texH, &texChannels, STBI_rgb_alpha);
        
//         if (!pPixels) {
//             throw std::runtime_error("failed to load texture image!");
//         }
        
//         VkDeviceSize imageSize = texW * texH * 4;

//         VyBuffer stagingBuffer{ VyBuffer::stagingBuffer("hdri_texture", imageSize ) };

//         stagingBuffer.write( pPixels );

//         stbi_image_free(pPixels);

//         m_Image = VyImage::Builder{}
//             .setImageType  (VK_IMAGE_TYPE_2D)
//             .setFormat     (VK_FORMAT_R8G8B8A8_UNORM)
//             .setExtent     (static_cast<U32>(texW), static_cast<U32>(texH))
//             .setLevels     (m_MipLevels)
// 			.setLayers     (1)
// 			.setSamples    (VK_SAMPLE_COUNT_1_BIT)
//             .setTiling     (VK_IMAGE_TILING_OPTIMAL)
// 			.setLayout     (VK_IMAGE_LAYOUT_UNDEFINED)
//             .setSharing    (VK_SHARING_MODE_EXCLUSIVE)
//             .setUsage      (VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
//             .setMemoryUsage(VMA_MEMORY_USAGE_AUTO)
//         	.build();

//         m_Image.upload(stagingBuffer);

//         // m_Image.transitionLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

//         // m_Image.copyFrom(stagingBuffer, true);
    
//         // GenerateMipmap(textureImage, VK_FORMAT_R8G8B8A8_UNORM, texW, texH, m_MipLevels);
//     }
// }


// namespace Vy
// {
//     Renderpass::~Renderpass() {}

//     VkDescriptorImageInfo Renderpass::descriptorImageInfo(VkImageLayout layout) const 
//     {
//         VkDescriptorImageInfo info{};

//         if (m_ColorAttachment && m_ColorAttachment->hasView() && m_ColorAttachment->hasSampler()) 
//         {
//             info.imageView = m_ColorAttachment->view();
//             info.sampler   = m_ColorAttachment->sampler();
//         } 
//         else if (m_DepthAttachment && m_DepthAttachment->hasView() && m_DepthAttachment->hasSampler()) 
//         {
//             info.imageView = m_DepthAttachment->view();
//             info.sampler   = m_DepthAttachment->sampler();
//         }

//         info.imageLayout = layout;
        
//         return info;
//     }

//     void Renderpass::writeToDescriptorSet() 
//     {
//         VkDescriptorImageInfo descriptorInfo{};

//         if (m_ColorAttachment && m_ColorAttachment->hasView()) 
//         {
//             descriptorInfo.sampler     = m_ColorAttachment->sampler();
//             descriptorInfo.imageView   = m_ColorAttachment->view();
//             descriptorInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;  // For color
//         } 
//         else if (m_DepthAttachment && m_DepthAttachment->hasView()) 
//         {
//             descriptorInfo.sampler     = m_DepthAttachment->sampler();
//             descriptorInfo.imageView   = m_DepthAttachment->view();
//             descriptorInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;  // For depth
//         }

//         VyDescriptorWriter writer(*m_DescriptorSetLayout, *VyContext::globalPool() );
//         writer.writeImage(0, &descriptorInfo);
//         writer.update( m_DescriptorSet );
//     }
// }






// namespace Vy
// {
//     VyFramebuffer::VyFramebuffer(VkExtent2D extent, U32 frameCount, bool bUseMipmaps) : 
//         m_Extent    { extent      }, 
//         m_FrameCount{ frameCount  }, 
//         m_UseMipmaps{ bUseMipmaps }
//     {
//         createRenderPass();
//         createImages();
//         createFramebuffers();
//     }


//     VyFramebuffer::~VyFramebuffer()
//     {
//         cleanup();
    
//         vkDestroyRenderPass(VyContext::device(), m_RenderPass, nullptr);
//     }


//     void VyFramebuffer::cleanup()
//     {
//         for (auto framebuffer : m_Framebuffers)
//         {
//             vkDestroyFramebuffer(VyContext::device(), framebuffer, nullptr);
//         }

//         m_DepthMipImageViews.clear();
//         m_HZBMipImageViews  .clear();
//     }


//     void VyFramebuffer::resize(VkExtent2D newExtent)
//     {
//         m_Extent = newExtent;
        
//         cleanup();
//         createImages();
//         createFramebuffers();
//     }


//     void VyFramebuffer::createRenderPass()
//     {
//         auto depthFormat = VyContext::device().findSupportedFormat(
//             { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
//             VK_IMAGE_TILING_OPTIMAL,
//             VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
//         );

//         VkAttachmentDescription m_ColorAttachment{};
//         {
//             m_ColorAttachment.format         = VK_FORMAT_R16G16B16A16_SFLOAT;
//             m_ColorAttachment.samples        = VK_SAMPLE_COUNT_1_BIT;

//             m_ColorAttachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
//             m_ColorAttachment.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
            
//             m_ColorAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
//             m_ColorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            
//             m_ColorAttachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
            
//             if (m_UseMipmaps)
//             {
//                 m_ColorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
//             }
//             else
//             {
//                 m_ColorAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
//             }
//         }

//         VkAttachmentReference colorAttachmentRef{};
//         {
//             colorAttachmentRef.attachment = 0;
//             colorAttachmentRef.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
//         }

//         VkAttachmentDescription depthAttachment{};
//         {
//             depthAttachment.format         = depthFormat;
//             depthAttachment.samples        = VK_SAMPLE_COUNT_1_BIT;

//             depthAttachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
//             depthAttachment.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
            
//             depthAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
//             depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            
//             depthAttachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
//             depthAttachment.finalLayout    = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
//         }

//         VkAttachmentReference depthAttachmentRef{};
//         {
//             depthAttachmentRef.attachment = 1;
//             depthAttachmentRef.layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
//         }

//         VkSubpassDescription subpass{};
//         {
//             subpass.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
//             subpass.colorAttachmentCount    = 1;
//             subpass.pColorAttachments       = &colorAttachmentRef;
//             subpass.pDepthStencilAttachment = &depthAttachmentRef;
//         }

//         TArray<VkSubpassDependency, 2> dependencies;
//         {
//             // [ 0 ]
//             {
//                 dependencies[0].srcSubpass      = VK_SUBPASS_EXTERNAL;
//                 dependencies[0].dstSubpass      = 0;

//                 dependencies[0].srcStageMask    = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
//                 dependencies[0].dstStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;

//                 dependencies[0].srcAccessMask   = VK_ACCESS_SHADER_READ_BIT;
//                 dependencies[0].dstAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

//                 dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
//             }

//             // [ 1 ]
//             {
//                 dependencies[1].srcSubpass      = 0;
//                 dependencies[1].dstSubpass      = VK_SUBPASS_EXTERNAL;

//                 dependencies[1].srcStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
//                 dependencies[1].dstStageMask    = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;

//                 dependencies[1].srcAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
//                 dependencies[1].dstAccessMask   = VK_ACCESS_SHADER_READ_BIT;

//                 dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
//             }
//         }

//         TArray<VkAttachmentDescription, 2> attachments = { m_ColorAttachment, depthAttachment };

//         VkRenderPassCreateInfo renderPassInfo{ VKInit::renderPassCreateInfo() };
//         {
//             renderPassInfo.attachmentCount = static_cast<U32>(attachments.size());
//             renderPassInfo.pAttachments    = attachments.data();

//             renderPassInfo.subpassCount    = 1;
//             renderPassInfo.pSubpasses      = &subpass;
            
//             renderPassInfo.dependencyCount = static_cast<U32>(dependencies.size());
//             renderPassInfo.pDependencies   = dependencies.data();
//         }

//         VK_CHECK_SUCCESS(vkCreateRenderPass(VyContext::device(), &renderPassInfo, nullptr, &m_RenderPass), 
//             "Failed to create framebuffer render pass!");

//         VKDbg::setObjectName(m_RenderPass, "framebuffer_renderpass");
//     }


//     void VyFramebuffer::createImages()
//     {
//         if (m_UseMipmaps)
//         {
//             m_MipLevels = static_cast<U32>(std::floor(std::log2(std::max(m_Extent.width, m_Extent.height)))) + 1;
//         }
//         else
//         {
//             m_MipLevels = 1;
//         }

//         // Color
//         m_ColorImages              .resize( m_FrameCount );
//         m_ColorImageViews          .resize( m_FrameCount );
//         m_ColorAttachmentImageViews.resize( m_FrameCount );

//         // Depth
//         m_DepthImages              .resize( m_FrameCount );
//         m_DepthImageViews          .resize( m_FrameCount );

//         // HZB
//         m_HZBImages                .resize( m_FrameCount );
//         m_HZBImageViews            .resize( m_FrameCount );
//         m_HZBMipImageViews         .resize( m_FrameCount );

//         VkFormat colorFormat = VK_FORMAT_R16G16B16A16_SFLOAT;
//         VkFormat depthFormat = VyContext::device().findSupportedFormat(
//             { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
//             VK_IMAGE_TILING_OPTIMAL,
//             VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT
//         );

//         for (U32 i = 0; i < m_FrameCount; i++)
//         {
//             // Create Color Image
//             auto colorImgBuilder  = VyImage::Builder{}
//                 .setName       ("framebuffer_color_" + i)
//                 .setImageType  (VK_IMAGE_TYPE_2D)
//                 .setFormat     (colorFormat)
//                 .setExtent     (m_Extent)
//                 .setLevels     (m_MipLevels)
//                 .setLayers     (1)
//                 .setSamples    (VK_SAMPLE_COUNT_1_BIT)
//                 .setTiling     (VK_IMAGE_TILING_OPTIMAL)
//                 .setLayout     (VK_IMAGE_LAYOUT_UNDEFINED)
//                 .setSharing    (VK_SHARING_MODE_EXCLUSIVE)
//                 .setUsage      (VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
//                 .setMemoryUsage(VMA_MEMORY_USAGE_AUTO);

//                 if (m_UseMipmaps)
//                 {
//                     colorImgBuilder.addUsage(VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
//                 }

//             m_ColorImages[ i ] = colorImgBuilder.build();

//             // Create Color Image View
//             auto colorViewBuilder = VyImageView::Builder{}
//                 .setName    ("framebuffer_color_" + i)
//                 .setViewType(VK_IMAGE_VIEW_TYPE_2D)
//                 .setFormat  (colorFormat)
//                 .setAspect  (VK_IMAGE_ASPECT_COLOR_BIT)
//                 .setLevels  (0, m_MipLevels)
//                 .setLayers  (0, 1);

//             m_ColorImageViews[ i ] = colorViewBuilder.build( m_ColorImages[ i ] );

//             // Create Color Attachment Image View (Mip Level 0 only)
//             colorViewBuilder.setName(std::format("framebuffer_color_attachment_{}", i));
//             colorViewBuilder.setLevelCount(1);

//             m_ColorAttachmentImageViews[ i ] = colorViewBuilder.build( m_ColorImages[ i ] );

//             // Create Depth Image
//             auto depthImgBuilder = VyImage::Builder{}
//                 .setName       (std::format("framebuffer_depth_{}", i))
//                 .setImageType  (VK_IMAGE_TYPE_2D)
//                 .setFormat     (depthFormat)
//                 .setExtent     (m_Extent)
//                 .setLevels     (m_MipLevels)
//                 .setLayers     (1)
//                 .setSamples    (VK_SAMPLE_COUNT_1_BIT)
//                 .setTiling     (VK_IMAGE_TILING_OPTIMAL)
//                 .setLayout     (VK_IMAGE_LAYOUT_UNDEFINED)
//                 .setSharing    (VK_SHARING_MODE_EXCLUSIVE)
//                 .setUsage      (VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT)
//                 .setMemoryUsage(VMA_MEMORY_USAGE_AUTO);

//                 if (m_UseMipmaps)
//                 {
//                     depthImgBuilder.addUsage(VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
//                 }

//             m_DepthImages[ i ] = depthImgBuilder.build();

//             auto depthViewBuilder = VyImageView::Builder{}
//                 .setName    (std::format("framebuffer_depth_{}", i))
//                 .setViewType(VK_IMAGE_VIEW_TYPE_2D)
//                 .setFormat  (depthFormat)
//                 .setAspect  (VK_IMAGE_ASPECT_DEPTH_BIT)
//                 .setLevels  (0, 1) // Framebuffer needs single mip level
//                 .setLayers  (0, 1);

//             m_DepthImageViews[ i ] = depthViewBuilder.build( m_DepthImages[ i ] );

//             // Create per-mip views for HZB
//             m_DepthMipImageViews     .resize( m_FrameCount );
//             m_DepthMipImageViews[ i ].resize( m_MipLevels  );

//             for (U32 mip = 0; mip < m_MipLevels; mip++)
//             {
//                 depthViewBuilder.setName  (std::format("framebuffer_depth_mip_{}_frame_{}", mip, i));
//                 depthViewBuilder.setLevels(mip, 1);
//                 depthViewBuilder.setLayers(0,   1);

//                 m_DepthMipImageViews[ i ][ mip ] = depthViewBuilder.build( m_DepthImages[ i ] );
//             }

//             // Create HZB Images (R32_SFLOAT)
//             auto hzbImgBuilder  = VyImage::Builder{}
//                 .setName       (std::format("framebuffer_hzb_{}", i))
//                 .setImageType  (VK_IMAGE_TYPE_2D)
//                 .setFormat     (VK_FORMAT_R32_SFLOAT)
//                 .setExtent     (m_Extent)
//                 .setLevels     (m_MipLevels)
//                 .setLayers     (1)
//                 .setSamples    (VK_SAMPLE_COUNT_1_BIT)
//                 .setTiling     (VK_IMAGE_TILING_OPTIMAL)
//                 .setLayout     (VK_IMAGE_LAYOUT_UNDEFINED)
//                 .setSharing    (VK_SHARING_MODE_EXCLUSIVE)
//                 .setUsage      (
//                     VK_IMAGE_USAGE_SAMPLED_BIT      | 
//                     VK_IMAGE_USAGE_STORAGE_BIT      | 
//                     VK_IMAGE_USAGE_TRANSFER_DST_BIT | 
//                     VK_IMAGE_USAGE_TRANSFER_SRC_BIT
//                 )
//                 .setMemoryUsage(VMA_MEMORY_USAGE_AUTO);

//             m_HZBImages[ i ] = hzbImgBuilder.build();

//             // Full View
//             auto hzbViewBuilder = VyImageView::Builder{}
//                 .setName    (std::format("framebuffer_hzb_{}", i))
//                 .setViewType(VK_IMAGE_VIEW_TYPE_2D)
//                 .setFormat  (VK_FORMAT_R32_SFLOAT)
//                 .setAspect  (VK_IMAGE_ASPECT_COLOR_BIT)
//                 .setLevels  (0, m_MipLevels) 
//                 .setLayers  (0, 1);

//             m_HZBImageViews[ i ] = hzbViewBuilder.build( m_HZBImages[ i ] );

//             // Per-Mip Views
//             m_HZBMipImageViews[ i ].resize( m_MipLevels );

//             for (U32 mip = 0; mip < m_MipLevels; mip++)
//             {
//                 hzbViewBuilder.setName  (std::format("framebuffer_hzb_mip_{}_frame_{}", mip, i));
//                 hzbViewBuilder.setLevels(mip, 1);

//                 m_HZBMipImageViews[ i ][ mip ] = hzbViewBuilder.build( m_HZBImages[ i ] );
//             }
//         }

//         // Create Sampler
//         auto samplerBuilder = VySampler::Builder{}
//             .setName         ("framebuffer_color")
//             .setFilters      (VK_FILTER_LINEAR)
//             .setMipmapMode   (VK_SAMPLER_MIPMAP_MODE_LINEAR)
//             .setWrap         (VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE)
//             .setBorder       (VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE)
// 			.enableAnisotropy(false, 1.0f)
//             .setLodRange     (0.0f, static_cast<float>(m_MipLevels))
//             .setMipLodBias   (0.0f);
        
//         m_Sampler = samplerBuilder.build();

//         // Create Depth Sampler (Reduction Mode Min/Max if supported, otherwise Linear)
//         // For HZB we ideally want a reduction sampler, but for now we'll use a separate sampler for depth
//         // Use NEAREST for depth to avoid interpolation issues during HZB generation if we do it manually
//         // But for sampling in shader, we might want LINEAR if we do PCF or similar.
//         // For Occlusion Culling, we want conservative depth.
//         {
//             samplerBuilder.setName      ("framebuffer_depth");
//             samplerBuilder.setFilters   (VK_FILTER_NEAREST);
//             samplerBuilder.setMipmapMode(VK_SAMPLER_MIPMAP_MODE_NEAREST);
            
//             m_DepthSampler = samplerBuilder.build();
//         }

//         // Create HZB Sampler
//         {
//             samplerBuilder.setName      ("framebuffer_hzb");
//             samplerBuilder.setFilters   (VK_FILTER_NEAREST);
//             samplerBuilder.setMipmapMode(VK_SAMPLER_MIPMAP_MODE_NEAREST);
            
//             m_HZBSampler = samplerBuilder.build();
//         }
//     }
    


//     void VyFramebuffer::createFramebuffers()
//     {
//         m_Framebuffers.resize( m_FrameCount );

//         for (size_t i = 0; i < m_FrameCount; i++)
//         {
//             TArray<VkImageView, 2> attachments = {
//                 m_ColorAttachmentImageViews[ i ].handle(), 
//                 m_DepthImageViews          [ i ].handle()
//             };

//             VkFramebufferCreateInfo framebufferInfo{ VKInit::framebufferCreateInfo() };
//             {
//                 framebufferInfo.renderPass      = m_RenderPass;

//                 framebufferInfo.attachmentCount = static_cast<U32>(attachments.size());
//                 framebufferInfo.pAttachments    = attachments.data();
                
//                 framebufferInfo.width           = m_Extent.width;
//                 framebufferInfo.height          = m_Extent.height;
                
//                 framebufferInfo.layers          = 1;
//             }

//             VK_CHECK_SUCCESS(vkCreateFramebuffer(VyContext::device(), &framebufferInfo, nullptr, &m_Framebuffers[ i ]), 
//                 "Failed to create framebuffer!");

//             TString name = std::format("framebuffer_{}", i);
//             VKDbg::setObjectName(m_Framebuffers.at( i ), name.c_str());
//         }
//     }


//     VkDescriptorImageInfo VyFramebuffer::descriptorImageInfo(int index) const
//     {
//         return VkDescriptorImageInfo{
//             .sampler     = m_Sampler                 .handle(),
//             .imageView   = m_ColorImageViews[ index ].handle(),
//             .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
//         };
//     }


//     void VyFramebuffer::beginRenderPass(VkCommandBuffer cmdBuffer, int frameIndex)
//     {
//         TArray<VkClearValue, 2> clearValues{};
//         {
//             clearValues[0].color        = {{ 0.01f, 0.01f, 0.01f, 1.0f }};
//             clearValues[1].depthStencil = { 1.0f, 0 };
//         }

//         VkRenderPassBeginInfo renderPassInfo{ VKInit::renderPassBeginInfo() };
//         {
//             renderPassInfo.renderPass        = m_RenderPass;
//             renderPassInfo.framebuffer       = m_Framebuffers[ frameIndex ];

//             renderPassInfo.renderArea.offset = { 0, 0 };
//             renderPassInfo.renderArea.extent = m_Extent;
            
//             renderPassInfo.clearValueCount = static_cast<U32>(clearValues.size());
//             renderPassInfo.pClearValues    = clearValues.data();
//         }

//         vkCmdBeginRenderPass(cmdBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
//     }


//     void VyFramebuffer::endRenderPass(VkCommandBuffer cmdBuffer) const
//     {
//         vkCmdEndRenderPass(cmdBuffer);
//     }


//     void VyFramebuffer::generateMipmaps(VkCommandBuffer cmdBuffer, int frameIndex)
//     {
//         if (!m_UseMipmaps) return;

//         VkImage image = m_ColorImages[frameIndex].handle();
//         I32     mipW  = m_Extent.width;
//         I32     mipH  = m_Extent.height;

//         VkImageMemoryBarrier barrier{ VKInit::imageMemoryBarrier() };
//         {
//             barrier.image                           = image;
//             barrier.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
//             barrier.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
//             barrier.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
//             barrier.subresourceRange.baseArrayLayer = 0;
//             barrier.subresourceRange.layerCount     = 1;
//             barrier.subresourceRange.levelCount     = 1;
//         }

//         // Transition Mip 0 to TRANSFER_SRC
//         {
//             barrier.subresourceRange.baseMipLevel = 0;
//             barrier.oldLayout                     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
//             barrier.newLayout                     = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
//             barrier.srcAccessMask                 = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
//             barrier.dstAccessMask                 = VK_ACCESS_TRANSFER_READ_BIT;
            
//             vkCmdPipelineBarrier(cmdBuffer, 
//                 VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 
//                 VK_PIPELINE_STAGE_TRANSFER_BIT, 
//                 0, 
//                 0, nullptr, 
//                 0, nullptr, 
//                 1, &barrier
//             );
//         }

//         for (U32 i = 1; i < m_MipLevels; i++)
//         {
//             // Transition Mip i to TRANSFER_DST
//             {
//                 barrier.subresourceRange.baseMipLevel = i;
//                 barrier.oldLayout                     = VK_IMAGE_LAYOUT_UNDEFINED;
//                 barrier.newLayout                     = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
//                 barrier.srcAccessMask                 = 0;
//                 barrier.dstAccessMask                 = VK_ACCESS_TRANSFER_WRITE_BIT;
                
//                 vkCmdPipelineBarrier(cmdBuffer, 
//                     VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 
//                     VK_PIPELINE_STAGE_TRANSFER_BIT, 
//                     0, 
//                     0, nullptr, 
//                     0, nullptr, 
//                     1, &barrier
//                 );
//             }

//             VkImageBlit blit{};
//             {
//                 blit.srcOffsets[0]                 = {0, 0, 0};
//                 blit.srcOffsets[1]                 = {mipW, mipH, 1};
//                 blit.srcSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
//                 blit.srcSubresource.mipLevel       = i - 1;
//                 blit.srcSubresource.baseArrayLayer = 0;
//                 blit.srcSubresource.layerCount     = 1;
                
//                 blit.dstOffsets[0]                 = {0, 0, 0};
//                 blit.dstOffsets[1]                 = {mipW > 1 ? mipW / 2 : 1, mipH > 1 ? mipH / 2 : 1, 1};
//                 blit.dstSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
//                 blit.dstSubresource.mipLevel       = i;
//                 blit.dstSubresource.baseArrayLayer = 0;
//                 blit.dstSubresource.layerCount     = 1;
                
//                 vkCmdBlitImage(cmdBuffer, 
//                     image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, 
//                     image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
//                     1, &blit, 
//                     VK_FILTER_LINEAR
//                 );
//             }

//             // Transition Mip i-1 to SHADER_READ_ONLY
//             {
//                 barrier.subresourceRange.baseMipLevel = i - 1;
//                 barrier.oldLayout                     = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
//                 barrier.newLayout                     = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
//                 barrier.srcAccessMask                 = VK_ACCESS_TRANSFER_READ_BIT;
//                 barrier.dstAccessMask                 = VK_ACCESS_SHADER_READ_BIT;
                
//                 vkCmdPipelineBarrier(cmdBuffer, 
//                     VK_PIPELINE_STAGE_TRANSFER_BIT, 
//                     VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 
//                     0, 
//                     0, nullptr, 
//                     0, nullptr, 
//                     1, &barrier
//                 );
//             }

//             // Transition Mip i to TRANSFER_SRC (for next loop)
//             if (i < m_MipLevels - 1)
//             {
//                 barrier.subresourceRange.baseMipLevel = i;
//                 barrier.oldLayout                     = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
//                 barrier.newLayout                     = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
//                 barrier.srcAccessMask                 = VK_ACCESS_TRANSFER_WRITE_BIT;
//                 barrier.dstAccessMask                 = VK_ACCESS_TRANSFER_READ_BIT;

//                 vkCmdPipelineBarrier(cmdBuffer, 
//                     VK_PIPELINE_STAGE_TRANSFER_BIT, 
//                     VK_PIPELINE_STAGE_TRANSFER_BIT, 
//                     0, 
//                     0, nullptr, 
//                     0, nullptr, 
//                     1, &barrier
//                 );
//             }

//             if (mipW > 1) mipW /= 2;
//             if (mipH > 1) mipH /= 2;
//         }

//         // Transition Last Mip to SHADER_READ_ONLY
//         {
//             barrier.subresourceRange.baseMipLevel = m_MipLevels - 1;
//             barrier.oldLayout                     = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
//             barrier.newLayout                     = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
//             barrier.srcAccessMask                 = VK_ACCESS_TRANSFER_WRITE_BIT;
//             barrier.dstAccessMask                 = VK_ACCESS_SHADER_READ_BIT;
            
//             vkCmdPipelineBarrier(cmdBuffer, 
//                 VK_PIPELINE_STAGE_TRANSFER_BIT, 
//                 VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 
//                 0, 
//                 0, nullptr, 
//                 0, nullptr, 
//                 1, &barrier
//             );
//         }
//     }
}