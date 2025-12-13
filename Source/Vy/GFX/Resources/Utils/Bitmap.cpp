// #include <Vy/GFX/Resources/Utils/Bitmap.h>

// namespace Vy
// {
//     VyBitmap::VyBitmap(int w, int h, int channels, VyBitmapFormat format) :
//         W(w), 
//         H(h), 
//         Channels(channels), 
//         Format(format), 
//         Data(w * h * channels * getBytesPerChannel(format))
//     {
//         initGetSetFuncs();
//     }


//     VyBitmap::VyBitmap(int w, int h, int depth, int channels, VyBitmapFormat format) :
//         W(w), 
//         H(h), 
//         Depth(depth), 
//         Channels(channels), 
//         Format(format), 
//         Data(w * h * depth * channels * getBytesPerChannel(format))
//     {
//         initGetSetFuncs();
//     }


//     VyBitmap::VyBitmap(int w, int h, int channels, VyBitmapFormat format, const void* ptr) :
//         W(w), 
//         H(h), 
//         Channels(channels), 
//         Format(format), 
//         Data(w * h * channels * getBytesPerChannel(format))
//     {
//         initGetSetFuncs();
//         memcpy(Data.data(), ptr, Data.size());
        
//         if(!Data.data())
//         {     
//             throw std::runtime_error("DEBUG: No data copied for Bitmap!");
//         }
//     }
// }





// namespace Vy
// {
//     ///////////////////////////////////////////////////////////////////////////////////////////
//     VySamplerManager::VySamplerManager()
//     {
//         // Find maximum sampler anistropy.
//         VkPhysicalDeviceProperties properties = VyContext::device().properties();

//         m_MaxSamplerAnisotropy = properties.limits.maxSamplerAnisotropy;

//         initDefault();
//     }


//     void VySamplerManager::initDefault()
//     {
//         // TextureGlobal
//         VkSampler SamplerTextureGlobal = build(SamplerType::TextureGlobal);
//         m_DefaultSamplers[SamplerType::TextureGlobal] = SamplerTextureGlobal;

//         // Skybox linear clamp
//         VkSampler SamplerSkyboxLinearClamp = build(SamplerType::SkyboxLinearClamp);
//         m_DefaultSamplers[SamplerType::SkyboxLinearClamp] = SamplerSkyboxLinearClamp;
//     }


//     void VySamplerManager::destroy()
//     {
//         // Destroy default samplers
//         for(auto& pair:m_DefaultSamplers)
//         {
//             VyContext::destroy(pair.second);
//         }
        
//         m_DefaultSamplers.clear();
        
//         // Destory customized samplers. dont need to check if it has something already
//         for(auto& pair: m_CustomSamplers)
//         {
//             VyContext::destroy(pair.second);   
//         }

//         m_CustomSamplers.clear();
//     }


//     void VySamplerManager::addSampler(const String& name, VkSampler newSampler)
//     {
//         m_CustomSamplers[ name ] = newSampler;
//     }


//     VkSampler VySamplerManager::build(SamplerType samplerType)
//     {
//         switch(samplerType)
//         {
//             case SamplerType::TextureGlobal :
//             {
//                 auto samplerInfo = SamplerCreateInfoBuilder()
//                     .maxAnisotropy(m_MaxSamplerAnisotropy)
//                     .getInfo();
                
//                 VkSampler retSampler;
//                 if (vkCreateSampler(VyContext::device(), &samplerInfo, nullptr, &retSampler) != VK_SUCCESS) 
//                 {
//                     VY_THROW_RUNTIME_ERROR("Failed to create TextureGlobal sampler!");
//                 }

//                 return retSampler;
//                 break;
//             }
            
//             case SamplerType::SkyboxLinearClamp :
//             {
//                 auto samplerInfo = SamplerCreateInfoBuilder()
//                     .addressMode  (VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE)
//                     .compareOp    (VK_COMPARE_OP_NEVER)
//                     .maxAnisotropy(m_MaxSamplerAnisotropy)
//                     .getInfo();
                
//                 VkSampler retSampler;
//                 if (vkCreateSampler(VyContext::device(), &samplerInfo, nullptr, &retSampler) != VK_SUCCESS) 
//                 {
//                     VY_THROW_RUNTIME_ERROR("Failed to create Skybox_LinearClamp sampler!");
//                 }
                
//                 return retSampler;
//                 break;
//             }
            
//             default:
//             {
//                 VY_THROW_RUNTIME_ERROR("Unknown SamplerType in VySamplerManager::build()");
//             }
//         }
//     }


//     VkSampler VySamplerManager::getSampler(const String& name)
//     {
//         auto pair = m_CustomSamplers.find(name);

//         if(pair!=m_CustomSamplers.end())
//         {
//             return pair->second;
//         }
        
//         VY_THROW_RUNTIME_ERROR("customized sampler:" + name + "not found!");
//     }


//     VkSampler VySamplerManager::getSampler(SamplerType samplerType)
//     {
//         auto pair = m_DefaultSamplers.find(samplerType);
        
//         if(pair != m_DefaultSamplers.end())
//         {
//             return pair->second;
//         }
        
//         VY_THROW_RUNTIME_ERROR("default sampler:" + std::to_string(static_cast<int>(samplerType)) + "not found!");
//     }

//     ///////////////////////////////////////////////////////////////////////////////////////////
//     ///////////////////////////////////////////////////////////////////////////////////////////
//     ///////////////////////////////////////////////////////////////////////////////////////////
//     ///////////////////////////////////////////////////////////////////////////////////////////
//     ///////////////////////////////////////////////////////////////////////////////////////////

//     //need to add one automatically read channels function 

//     //need to create texChannels based on the vkformat
//     VyTextureBase::VyTextureBase(VyTextureConfig& textureConfig) :
//         m_Config(textureConfig)
//     {
//         m_TexWidth    = m_Config.Extent.width;
//         m_TexHeight   = m_Config.Extent.height;
//         m_TexChannels = bytesPerPixel(m_Config.Format);


//         // Calculate mip levels if unprovided.
//         if(m_Config.MipLevels)
//         {
//             m_MipLevels = *m_Config.MipLevels;
//         }
//         else
//         {
//             m_MipLevels = static_cast<U32>(std::floor(std::log2(std::max(m_TexWidth, m_TexHeight))))+1;
//         }

//         createTextureBase();
//     }


//     VyTextureBase::~VyTextureBase()
//     {
//         if(m_CustomSampler)
//         {
//             VyContext::destroy(*m_CustomSampler);
//         }

//         VyContext::destroy(m_TextureBaseImageView);
//         VyContext::destroy(m_TextureBaseImage, m_TextureBaseAllocation);
//     }


//     void VyTextureBase::createCustomSampler(const VkSamplerCreateInfo& samplerInfo)
//     {
//         VkSampler tempSampler = VK_NULL_HANDLE;

//         if (vkCreateSampler(VyContext::device(), &samplerInfo, nullptr, &tempSampler) != VK_SUCCESS) 
//         {
//             VY_THROW_RUNTIME_ERROR("Failed to create customized sampler!");
//         }
        
//         m_CustomSampler = tempSampler;
//     }



//     VkImageView VyTextureBase::switchViewForMip(U32 selectMip, VkImageViewType vType)
//     {
//         auto viewInfo = ImageViewCreateInfoBuilder(m_TextureBaseImage)
//             .viewType   (vType)
//             .format     (m_Config.Format)
//             .mipLevels  (selectMip, 1)
//             .arrayLayers(0, m_Config.ArrayLayers)
//             .getInfo();

//         VkImageView currentView;
//         VyContext::device().createImageView(currentView, viewInfo);

//         return currentView;
//     }


//     void VyTextureBase::createTextureBase()
//     {
//         // Create Image and Allocation info.
//         auto imageInfo = ImageCreateInfoBuilder(m_TexWidth, m_TexHeight)
//             .imageType  (m_Config.ImageType)
//             .format     (m_Config.Format)
//             .arrayLayers(m_Config.ArrayLayers)
//             .mipLevels  (m_MipLevels)
//             .flags      (m_Config.CreateFlags)
//             .usage      (m_Config.UsageFlags)
//             .getInfo();

// 		VmaAllocationCreateInfo allocInfo{};
//         {
//             allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
//         }

//         if(VyContext::device().createImage(m_TextureBaseImage, m_TextureBaseAllocation, imageInfo, allocInfo) != VK_SUCCESS)
//         {
//             VY_THROW_RUNTIME_ERROR("Failed to create VkImage for Texture Base");
//         };

//         // Create image view
//         auto viewInfo = ImageViewCreateInfoBuilder(m_TextureBaseImage)
//             .viewType   (m_Config.ViewType)
//             .format     (m_Config.Format)
//             .mipLevels  (0, m_MipLevels)
//             .arrayLayers(0, m_Config.ArrayLayers)
//             .getInfo();

//         if(VyContext::device().createImageView(m_TextureBaseImageView, viewInfo)!=VK_SUCCESS)
//         {
//             VY_THROW_RUNTIME_ERROR("Failed to create VkImageView for Texture Base");
//         };

//         //T ransition image layout for all
//         auto cmdBuffer = VyContext::device().beginSingleTimeCommands();
//         {
//             VyContext::device().transitionImageLayout(cmdBuffer, 
//                 m_TextureBaseImage,
//                 VK_IMAGE_LAYOUT_UNDEFINED, 
//                 m_Config.NewLayout, 
//                 VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 
//                 VK_PIPELINE_STAGE_TRANSFER_BIT,
//                 VK_IMAGE_ASPECT_COLOR_BIT, 
//                 m_MipLevels, 
//                 m_Config.ArrayLayers
//             );
//         }
//         VyContext::device().endSingleTimeCommands(cmdBuffer);

//         if(m_Config.Data)
//         { 
//             // if user provide data
//             VkDeviceSize imageSize = m_TexWidth * m_TexHeight * bytesPerPixel(m_Config.Format) * m_Config.ArrayLayers;

//             VyBuffer stagingBuffer{ VyBuffer::stagingBuffer(imageSize) };

//             stagingBuffer.singleWrite(*m_Config.Data);
            
//             cmdBuffer = VyContext::device().beginSingleTimeCommands();
//             {
//                 copyBufferToImage(cmdBuffer, 
//                     stagingBuffer.handle(), 
//                     m_TextureBaseImage, 
//                     static_cast<U32>(m_TexWidth), 
//                     static_cast<U32>(m_TexHeight)
//                 );
//             }
//             VyContext::device().endSingleTimeCommands(cmdBuffer);

//             // Generate mipmaps.
//             generateMipmaps(m_TextureBaseImage, m_Config.Format , m_TexWidth, m_TexHeight, m_MipLevels);   
//         }
//     }

        

//     void VyTextureBase::copyBufferToImage(
//         VkCommandBuffer cmdBuffer,
//         VkBuffer        buffer, 
//         VkImage         image, 
//         U32             width, 
//         U32             height,
//         VkDeviceSize    bufferOffset, 
//         U32             layers) 
//     {
//         //include the single layer or multi layer cases
//         TVector<VkBufferImageCopy> regions(layers);

//         for(U32 i = 0 ; i<layers; ++i)
//         {
//             regions[i].bufferOffset      = bufferOffset * i;
//             regions[i].bufferRowLength   = 0;
//             regions[i].bufferImageHeight = 0;

//             regions[i].imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
//             regions[i].imageSubresource.mipLevel       = 0;
//             regions[i].imageSubresource.baseArrayLayer = i;
//             regions[i].imageSubresource.layerCount     = 1;

//             regions[i].imageOffset = { 0, 0, 0 };
//             regions[i].imageExtent = { (U32)m_TexWidth, (U32)m_TexHeight, 1 };
//         }
        
//         vkCmdCopyBufferToImage(
//             cmdBuffer,
//             buffer,
//             image,
//             VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
//             static_cast<U32>(regions.size()),
//             regions.data()
//         );
//     }



//     void VyTextureBase::generateMipmaps(
//         VkImage  image, 
//         VkFormat imageFormat, 
//         I32      texWidth, 
//         I32      texHeight, 
//         U32      mipLevels, 
//         U32      layerCount)
//     {
//         VkFormatProperties formatProperties;
//         vkGetPhysicalDeviceFormatProperties(VyContext::device().physicalDevice(), imageFormat, &formatProperties);

//         if(!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
//         {
//             VY_THROW_RUNTIME_ERROR("texture image format does not support linear blitting!"); }
        
//         auto cmdBuffer = VyContext::device().beginSingleTimeCommands();
//         {
//             VkImageMemoryBarrier barrier{ VKInit::imageMemoryBarrier() };
//             {
//                 barrier.image = image;

//                 barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
//                 barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                
//                 barrier.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
//                 barrier.subresourceRange.baseArrayLayer = 0;
//                 barrier.subresourceRange.layerCount     = layerCount;
//                 barrier.subresourceRange.levelCount     = 1;
//             }

//             I32 mipWidth  = texWidth;
//             I32 mipHeight = texHeight;

//             for (U32 i = 1; i < mipLevels; i++) 
//             {
//                 barrier.subresourceRange.baseMipLevel = i - 1;

//                 barrier.oldLayout     = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
//                 barrier.newLayout     = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
//                 barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
//                 barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

//                 vkCmdPipelineBarrier(cmdBuffer,
//                     VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
//                     0, nullptr,
//                     0, nullptr,
//                     1, &barrier
//                 );

//                 VkImageBlit blit{};
//                 {
//                     blit.srcOffsets[0] = { 0, 0, 0 };
//                     blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };

//                     blit.srcSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
//                     blit.srcSubresource.mipLevel       = i - 1;
//                     blit.srcSubresource.baseArrayLayer = 0;
//                     blit.srcSubresource.layerCount     = layerCount;

//                     blit.dstOffsets[0] = { 0, 0, 0 };
//                     blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };

//                     blit.dstSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
//                     blit.dstSubresource.mipLevel       = i;
//                     blit.dstSubresource.baseArrayLayer = 0;
//                     blit.dstSubresource.layerCount     = layerCount;
//                 }

//                 vkCmdBlitImage(cmdBuffer,
//                     image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
//                     image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
//                     1, &blit,
//                     VK_FILTER_LINEAR
//                 );

//                 barrier.oldLayout     = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
//                 barrier.newLayout     = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; 
//                 barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
//                 barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

//                 vkCmdPipelineBarrier(cmdBuffer,
//                     VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
//                     0, nullptr,
//                     0, nullptr,
//                     1, &barrier
//                 );

//                 if (mipWidth  > 1) mipWidth  /= 2;
//                 if (mipHeight > 1) mipHeight /= 2;
//             }

//             barrier.subresourceRange.baseMipLevel = mipLevels - 1;

//             barrier.oldLayout     = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
//             barrier.newLayout     = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
//             barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
//             barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

//             vkCmdPipelineBarrier(cmdBuffer, 
//                 VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
//                 0, nullptr,
//                 0, nullptr,
//                 1, &barrier 
//             );
//         }
//         VyContext::device().endSingleTimeCommands(cmdBuffer);
//     }


//     ///////////////////////////////////////////////////////////////////////////////////////////
//     ///////////////////////////////////////////////////////////////////////////////////////////
//     ///////////////////////////////////////////////////////////////////////////////////////////
//     ///////////////////////////////////////////////////////////////////////////////////////////
//     ///////////////////////////////////////////////////////////////////////////////////////////


//     // Create 2dTexture automatically from path
//     VyTexture2D::VyTexture2D(const String& path, VkFormat format) :
//         VyTextureBase()
//     {
//         m_Config = createConfig(path, format);

//         createTextureImage();
//         createTextureImageView();
//     }


//     VyTextureConfig VyTexture2D::createConfig(const String& path, VkFormat format)
//     {
//         //here can read data directly
//         m_Pixels = stbi_load(path.data(), &m_TexWidth, &m_TexHeight, &m_TexChannels, STBI_rgb_alpha);
        
//         m_MipLevels = static_cast<U32>(std::floor(std::log2(std::max(m_TexWidth, m_TexHeight)))) + 1;

//         if (!m_Pixels) 
//         {
//             VY_THROW_RUNTIME_ERROR("Failed to load texture image!");
//         }

//         VyTextureConfig config;
//         {
//             config.ImageType   = VK_IMAGE_TYPE_2D;
//             config.ViewType    = VK_IMAGE_VIEW_TYPE_2D;
//             config.Extent      = { static_cast<U32>(m_TexWidth), static_cast<U32>(m_TexHeight), 1 };
//             config.Format      = format;
//             config.ArrayLayers = 1;
//             config.UsageFlags  = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
//             config.NewLayout   = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
//         }

//         return config;
//     }


//     void VyTexture2D::createTextureImage()
//     {
//         VkDeviceSize imageSize = m_TexWidth * m_TexHeight * 4; // Always 4 channels due to STBI_rgb_alpha; 
//         //if i fill in texchannels not correct, fill in bytesperpixel(by vkformat) still not correct, need to investigate later
//         VyBuffer stagingBuffer{ VyBuffer::stagingBuffer(imageSize) };

//         stagingBuffer.singleWrite(m_Pixels);
        
//         stbi_image_free(m_Pixels);

//         auto imageInfo = ImageCreateInfoBuilder(m_TexWidth, m_TexHeight)
//             .mipLevels(m_MipLevels)
//             .format   (m_Config.Format)
//             .usage    (m_Config.UsageFlags )
//             .getInfo();

// 		VmaAllocationCreateInfo allocInfo{};
//         {
//             allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
//         }

//         if(VyContext::device().createImage(m_TextureBaseImage, m_TextureBaseAllocation, imageInfo, allocInfo)!=VK_SUCCESS)
//         {
//             VY_THROW_RUNTIME_ERROR("Failed to create VkImage for Texture2D");
//         };

//         auto cmdBuffer = VyContext::device().beginSingleTimeCommands();
//         {
//             VyContext::device().transitionImageLayout(cmdBuffer,
//                 m_TextureBaseImage,  
//                 VK_IMAGE_LAYOUT_UNDEFINED, 
//                 m_Config.NewLayout,
//                 VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 
//                 VK_PIPELINE_STAGE_TRANSFER_BIT,
//                 VK_IMAGE_ASPECT_COLOR_BIT, 
//                 m_MipLevels, 
//                 1
//             );
                
//             copyBufferToImage(cmdBuffer, 
//                 stagingBuffer.handle(), 
//                 m_TextureBaseImage, 
//                 static_cast<U32>(m_TexWidth), 
//                 static_cast<U32>(m_TexHeight)
//             );
//         }
//         VyContext::device().endSingleTimeCommands(cmdBuffer);

//         generateMipmaps(m_TextureBaseImage, m_Config.Format , m_TexWidth, m_TexHeight, m_MipLevels);   
//     }



//     void VyTexture2D::createTextureImageView()
//     {
//         auto viewInfo = ImageViewCreateInfoBuilder(m_TextureBaseImage)
//             .viewType (VK_IMAGE_VIEW_TYPE_2D)
//             .format   (m_Config.Format)
//             .mipLevels(0, m_MipLevels)
//             .getInfo();

//         if(VyContext::device().createImageView(m_TextureBaseImageView, viewInfo)!=VK_SUCCESS)
//         {
//             VY_THROW_RUNTIME_ERROR("Failed to create VkImageView for Texture2D");
//         };
//     }


//     VyTexture2D::~VyTexture2D()
//     {
//     }
    
    
//     ///////////////////////////////////////////////////////////////////////////////////////////
//     ///////////////////////////////////////////////////////////////////////////////////////////
//     ///////////////////////////////////////////////////////////////////////////////////////////
//     ///////////////////////////////////////////////////////////////////////////////////////////
//     ///////////////////////////////////////////////////////////////////////////////////////////



//     //create 2dTexture automatically from path
//     VyColorTexture::VyColorTexture(float r, float g, float b) :
//         VyTextureBase()
//     {
//         m_TexWidth    = 4;
//         m_TexHeight   = 4;
//         m_TexChannels = 4;
//         m_Config    = createConfig();

//         generateData(r, g, b);
//         createTextureImage();
//         createTextureImageView();
//     }

    
//     void VyColorTexture::generateData(float r, float g, float b)
//     {
//         //must be four channels, otherwise my gpu doesnt support three channels linear blitting
//         U8 red   = static_cast<U8>(r* 255.0f);
//         U8 green = static_cast<U8>(g* 255.0f);
//         U8 blue  = static_cast<U8>(b* 255.0f);

//         int totalPixels = m_TexWidth * m_TexHeight;
//         int totalBytes  = totalPixels * m_TexChannels;

//         m_Pixels = new U8[totalBytes];

//         for(int i = 0; i < totalPixels; ++i)
//         {
//             m_Pixels[i * m_TexChannels + 0] = red;
//             m_Pixels[i * m_TexChannels + 1] = green;
//             m_Pixels[i * m_TexChannels + 2] = blue;      
//             m_Pixels[i * m_TexChannels + 3] = 255.0f;
//         }
//     }


//     VyTextureConfig VyColorTexture::createConfig()
//     {
//         m_MipLevels = static_cast<U32>(std::floor(std::log2(std::max(m_TexWidth, m_TexHeight)))) + 1;

//         VyTextureConfig config;
//         {
//             config.ImageType   = VK_IMAGE_TYPE_2D;
//             config.ViewType    = VK_IMAGE_VIEW_TYPE_2D;
//             config.Extent      = { static_cast<U32>(4), static_cast<U32>(4), 1 };
//             config.Format      = VK_FORMAT_R8G8B8A8_UNORM;
//             config.ArrayLayers = 1;
//             config.UsageFlags  = VK_IMAGE_USAGE_TRANSFER_DST_BIT|VK_IMAGE_USAGE_TRANSFER_SRC_BIT|VK_IMAGE_USAGE_SAMPLED_BIT;
//             config.NewLayout   = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
//         }

//         return config;
//     }


//     void VyColorTexture::createTextureImage()
//     {
//         VkDeviceSize imageSize = m_TexWidth * m_TexHeight * m_TexChannels; // 4 channels, integer, so each channel 1 byte

//         VyBuffer stagingBuffer{ VyBuffer::stagingBuffer(imageSize) };

//         stagingBuffer.singleWrite(m_Pixels);

//         auto imageInfo = ImageCreateInfoBuilder(m_TexWidth, m_TexHeight)
//             .mipLevels(m_MipLevels)
//             .format   (m_Config.Format)
//             .usage    (m_Config.UsageFlags )
//             .getInfo();

// 		VmaAllocationCreateInfo allocInfo{};
//         {
//             allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
//         }

//         if(VyContext::device().createImage(m_TextureBaseImage, m_TextureBaseAllocation, imageInfo, allocInfo)!=VK_SUCCESS)
//         {
//             VY_THROW_RUNTIME_ERROR("Failed to create VkImage for Solid color");
//         };

//         auto cmdBuffer = VyContext::device().beginSingleTimeCommands();
//         {
//             VyContext::device().transitionImageLayout(cmdBuffer,
//                 m_TextureBaseImage,  
//                 VK_IMAGE_LAYOUT_UNDEFINED, 
//                 m_Config.NewLayout,
//                 VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 
//                 VK_PIPELINE_STAGE_TRANSFER_BIT,
//                 VK_IMAGE_ASPECT_COLOR_BIT, 
//                 m_MipLevels, 
//                 1
//             );

//             copyBufferToImage(cmdBuffer, 
//                 stagingBuffer.handle(), 
//                 m_TextureBaseImage, 
//                 static_cast<U32>(m_TexWidth), 
//                 static_cast<U32>(m_TexHeight)
//             );
//         }
//         VyContext::device().endSingleTimeCommands(cmdBuffer);

//         generateMipmaps(m_TextureBaseImage, m_Config.Format , m_TexWidth, m_TexHeight, m_MipLevels);   
//     }



//     void VyColorTexture::createTextureImageView()
//     {
//         auto viewInfo = ImageViewCreateInfoBuilder(m_TextureBaseImage)
//             .viewType (VK_IMAGE_VIEW_TYPE_2D)
//             .format   (m_Config.Format)
//             .mipLevels(0, m_MipLevels)
//             .getInfo();

//         if(VyContext::device().createImageView(m_TextureBaseImageView, viewInfo) != VK_SUCCESS)
//         {
//             VY_THROW_RUNTIME_ERROR("Failed to create VkImageView for Solid color");
//         };
//     }


//     VyColorTexture::~VyColorTexture()
//     {
//         delete[] m_Pixels;
//     }
// }

// namespace Vy
// {
//     //first when initialize, build all default solid color for all channels
//     //then when call setXXXtexture, overwrite the pimageInfo, 
//     //one vector for descriptor writes, one vector for pimageinfo, overwrite pimageinfo one

//     VyMaterial::VyMaterial
//     (
//         Shared<VyDescriptorAllocator> descriptorAllocator,
//         VkDescriptorSetLayout         descriptorSetLayout,
//         VySamplerManager&             samplerManager
//     ) :
//         m_DescriptorAllocator{ descriptorAllocator }, 
//         m_SamplerManager     { samplerManager      }
//     {
//         DefaultWhite  = MakeShared<VyColorTexture>(1.0f, 1.0f, 1.0f);
//         DefaultBlack  = MakeShared<VyColorTexture>(0.0f, 0.0f, 0.0f);
//         DefaultGrey   = MakeShared<VyColorTexture>(0.5f, 0.5f, 0.5f);
//         DefaultNormal = MakeShared<VyColorTexture>(0.5f, 0.5f, 1.0f);

//         // When create a new material (initialize a material)->allocate a desriptor set
//         m_DescriptorAllocator->allocate(descriptorSetLayout, m_MaterialDescriptorSet);

//         initDefault();
//         update();
//     }


//     VyMaterial::~VyMaterial()
//     {
//     }


//     void VyMaterial::initDefault()
//     {
//         m_ImageInfos.clear();
//         m_ImageInfos.reserve(4); 
//         m_ImageInfos.push_back(DefaultWhite-> descriptorImageInfo(m_SamplerManager.getSampler( SamplerType::TextureGlobal )));
//         m_ImageInfos.push_back(DefaultGrey->  descriptorImageInfo(m_SamplerManager.getSampler( SamplerType::TextureGlobal )));
//         m_ImageInfos.push_back(DefaultBlack-> descriptorImageInfo(m_SamplerManager.getSampler( SamplerType::TextureGlobal )));
//         m_ImageInfos.push_back(DefaultNormal->descriptorImageInfo(m_SamplerManager.getSampler( SamplerType::TextureGlobal )));

//         VkDescriptorType desType{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER };

//         /* 0 : albedo */
//         VkWriteDescriptorSet albedoWrite{ VKInit::writeDescriptorSet() };
//         {
//             albedoWrite.dstSet          = m_MaterialDescriptorSet;
//             albedoWrite.dstBinding      = 0;
//             albedoWrite.descriptorCount = 1;
//             albedoWrite.descriptorType  = desType;
//             albedoWrite.pImageInfo      = &m_ImageInfos[0];
//         }

//         m_DescriptorWrites.push_back(albedoWrite);

//         /* 1 : roughness */
//         VkWriteDescriptorSet roughnessWrite{ VKInit::writeDescriptorSet() };
//         {
//             roughnessWrite.dstSet          = m_MaterialDescriptorSet;
//             roughnessWrite.dstBinding      = 1;
//             roughnessWrite.descriptorCount = 1;
//             roughnessWrite.descriptorType  = desType;
//             roughnessWrite.pImageInfo      = &m_ImageInfos[1];
//         }

//         m_DescriptorWrites.push_back(roughnessWrite);

//         /* 2 : metallic */
//         VkWriteDescriptorSet metallicWrite{ VKInit::writeDescriptorSet() };
//         {
//             roughnessWrite.dstSet          = m_MaterialDescriptorSet;
//             roughnessWrite.dstBinding      = 2;
//             roughnessWrite.descriptorCount = 1;
//             roughnessWrite.descriptorType  = desType;
//             roughnessWrite.pImageInfo      = &m_ImageInfos[2];
//         }

//         m_DescriptorWrites.push_back(metallicWrite);


//         /* 3: normal */
//         VkWriteDescriptorSet normalWrite{ VKInit::writeDescriptorSet() };
//         {
//             roughnessWrite.dstSet          = m_MaterialDescriptorSet;
//             roughnessWrite.dstBinding      = 3;
//             roughnessWrite.descriptorCount = 1;
//             roughnessWrite.descriptorType  = desType;
//             roughnessWrite.pImageInfo      = &m_ImageInfos[3];
//         }

//         m_DescriptorWrites.push_back(normalWrite);
//     }


//     void VyMaterial::update()
//     {
//         vkUpdateDescriptorSets(VyContext::device(), m_DescriptorWrites.size(), m_DescriptorWrites.data(), 0, nullptr);
//     }


//     void VyMaterial::setAlbedoTexture(const VyTexture2D& albedoMap)
//     {
//         m_ImageInfos[0] = albedoMap.descriptorImageInfo(m_SamplerManager.getSampler(SamplerType::TextureGlobal));
//         update();
//     }


//     void VyMaterial::setRoughnessTexture(const VyTexture2D& roughnessMap)
//     {
//         m_ImageInfos[1] = roughnessMap.descriptorImageInfo(m_SamplerManager.getSampler(SamplerType::TextureGlobal));
//         update();
//     }


//     void VyMaterial::setMetallicTexture(const VyTexture2D& metallicMap)
//     {
//         m_ImageInfos[2] = metallicMap.descriptorImageInfo(m_SamplerManager.getSampler(SamplerType::TextureGlobal));
//         update();
//     }


//     void VyMaterial::setNormalTexture(const VyTexture2D& normalMap)
//     {
//         m_ImageInfos[3] = normalMap.descriptorImageInfo(m_SamplerManager.getSampler(SamplerType::TextureGlobal));
//         update();
//     }


//     void VyMaterial::bind(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout)
//     {
//         vkCmdBindDescriptorSets(commandBuffer, 
//             VK_PIPELINE_BIND_POINT_GRAPHICS, 
//             pipelineLayout,
//             2, /* set layout index */
//             1, /* descriptorSetCount */
//             &m_MaterialDescriptorSet, /* *pDescriptorSets */
//             0, 
//             nullptr 
//         );
//     }
// }


// ///////////////////////////////////////////////////////////////////////////////////////////
// ///////////////////////////////////////////////////////////////////////////////////////////
// ///////////////////////////////////////////////////////////////////////////////////////////
// ///////////////////////////////////////////////////////////////////////////////////////////
// ///////////////////////////////////////////////////////////////////////////////////////////
// //LEGACY class, exisit because of cube map, will change later


// // IVyTexture::IVyTexture(const String& path)  
// // {
// //     createTextureImage(path);
// //     createTextureImageView();
// //     createTextureSampler();
// //     createDescriptorInfo();
// // }


// // IVyTexture::~IVyTexture(){  //order is important
// //     vkDestroySampler(VyContext::device(), textureSampler_, nullptr);
// //     vkDestroyImageView(VyContext::device(), textureImageView_, nullptr);
// //     vkDestroyImage(VyContext::device(), m_TextureImage, nullptr);
// //     vkFreeMemory(VyContext::device(), textureImageMemory_, nullptr);

// // }

// // void IVyTexture::createDescriptorInfo()
// // {
// //     descriptorImageInfo_ = VkDescriptorImageInfo
// //     {
// //         textureSampler_,
// //         textureImageView_,
// //         VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
// //     };
// // }

// // void IVyTexture::createTextureImage(const String& path) 
// // {
// //     // int texWidth, texHeight, texChannels;
// //     stbi_uc* pixels = stbi_load(path.data(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
// //     VkDeviceSize imageSize = texWidth * texHeight * 4; // 4 channels, integer, so each channel 1 byte
// //     m_MipLevels = static_cast<U32>(std::floor(std::log2(std::max(texWidth, texHeight))))+1;

// //     if (!pixels) {
// //         VY_THROW_RUNTIME_ERROR("Failed to load texture image!");}


// //     JBuffer stagingBuffer(imageSize, 
// //                 VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
// //     void* data;
// //     vkMapMemory(VyContext::device(), stagingBuffer.bufferMemory(), 0, stagingBuffer.getSize(), 0, &data);
// //     memcpy(data, pixels, static_cast<size_t>(imageSize));
// //     vkUnmapMemory(VyContext::device(), stagingBuffer.bufferMemory());
// //     stbi_image_free(pixels);

// //     auto imageInfo = ImageCreateInfoBuilder(texWidth, texHeight)
// //                     .mipLevels(m_MipLevels)
// //                     .usage(VK_IMAGE_USAGE_TRANSFER_DST_BIT|VK_IMAGE_USAGE_TRANSFER_SRC_BIT|VK_IMAGE_USAGE_SAMPLED_BIT)
// //                     .getInfo();
// //     if(VyContext::device().createImage(imageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_TextureImage, textureImageMemory_)!=VK_SUCCESS){
// //         VY_THROW_RUNTIME_ERROR("Failed to create VkImage for texture(old one)");
// //     };

// //     JCommandBuffer cmdBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY);
// //     cmdBuffer.beginSingleTimeCommands();

// //     VyContext::device().transitionImageLayout(cmdBuffer ,m_TextureImage,  
// //         VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
// //         VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,   VK_PIPELINE_STAGE_TRANSFER_BIT,
// //         VK_IMAGE_ASPECT_COLOR_BIT, m_MipLevels, 1);

// //     copyBufferToImage(cmdBuffer, stagingBuffer.buffer(), m_TextureImage, 
// //         static_cast<U32>(texWidth), static_cast<U32>(texHeight));

// //     cmdBuffer.endSingleTimeCommands(VyContext::device().graphicsQueue());

// //     generateMipmaps(m_TextureImage, VK_FORMAT_R8G8B8A8_SRGB, texWidth, texHeight, m_MipLevels);

// // }


// // void IVyTexture::createTextureImageView()
// // {
// //     auto viewInfo = ImageViewCreateInfoBuilder(m_TextureImage)
// //                     .viewType(VK_IMAGE_VIEW_TYPE_2D)
// //                     .mipLevels(0, m_MipLevels)
// //                     .getInfo();
// //     VyContext::device().createImageView(viewInfo, textureImageView_);

// // }


// // void IVyTexture::copyBufferToImage(VkCommandBuffer cmdBuffer,VkBuffer buffer, VkImage image, U32 width, U32 height) 
// // {
// //     // VkCommandBuffer cmdBuffer = util::beginSingleTimeCommands(VyContext::device(), VyContext::device().getCommandPool());
// //     VkBufferImageCopy region{};
// //     region.bufferOffset = 0;
// //     region.bufferRowLength = 0;
// //     region.bufferImageHeight = 0;
// //     region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
// //     region.imageSubresource.mipLevel = 0;
// //     region.imageSubresource.baseArrayLayer = 0;
// //     region.imageSubresource.layerCount = 1;
// //     region.imageOffset = {0, 0, 0};
// //     region.imageExtent = {
// //         width,
// //         height,
// //         1
// //     };

// //     vkCmdCopyBufferToImage(cmdBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
// //     // util::endSingleTimeCommands(VyContext::device(), cmdBuffer, VyContext::device().getCommandPool(), VyContext::device().graphicsQueue());
// // }





// // void IVyTexture::createTextureSampler() 
// // {
// //     VkPhysicalDeviceProperties properties{};
// //     vkGetPhysicalDeviceProperties(VyContext::device().physicalDevice(), &properties);

// //     auto samplerInfo = SamplerCreateInfoBuilder()
// //                         .maxAnisotropy(properties.limits.maxSamplerAnisotropy)
// //                         .getInfo();


// //     if (vkCreateSampler(VyContext::device(), &samplerInfo, nullptr, &textureSampler_) != VK_SUCCESS) {
// //         VY_THROW_RUNTIME_ERROR("Failed to create texture sampler!");
// //     }
// // }



// // void IVyTexture::generateMipmaps(VkImage image, VkFormat imageFormat, 
// //                     I32 texWidth, I32 texHeight, 
// //                     U32 mipLevels, U32 layerCount){

// //     VkFormatProperties formatProperties;
// //     vkGetPhysicalDeviceFormatProperties(VyContext::device().physicalDevice(), imageFormat, &formatProperties);

// //     if(!(formatProperties.optimalTilingFeatures&VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)){
// //         VY_THROW_RUNTIME_ERROR("texture iamge format does not support linear blitting!"); }
    
// //     VkCommandBuffer cmdBuffer = util::beginSingleTimeCommands(VyContext::device(), VyContext::device().getCommandPool());
    
// //     VkImageMemoryBarrier barrier{};
// //     barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
// //     barrier.image = image;
// //     barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
// //     barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
// //     barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
// //     barrier.subresourceRange.baseArrayLayer = 0;
// //     barrier.subresourceRange.layerCount = layerCount;
// //     barrier.subresourceRange.levelCount = 1;

// //     I32 mipWidth = texWidth;
// //     I32 mipHeight = texHeight;

// //     for (U32 i = 1; i < mipLevels; i++) {
// //         barrier.subresourceRange.baseMipLevel = i - 1;
// //         barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;  // oldlayout用的是什么布局
// //         barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
// //         barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
// //         barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

// //         vkCmdPipelineBarrier(cmdBuffer,
// //             VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
// //             0, nullptr,
// //             0, nullptr,
// //             1, &barrier);

// //         VkImageBlit blit{};
// //         blit.srcOffsets[0] = {0, 0, 0};
// //         blit.srcOffsets[1] = {mipWidth, mipHeight, 1};
// //         blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
// //         blit.srcSubresource.mipLevel = i - 1;
// //         blit.srcSubresource.baseArrayLayer = 0;
// //         blit.srcSubresource.layerCount = layerCount;
// //         blit.dstOffsets[0] = {0, 0, 0};
// //         blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
// //         blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
// //         blit.dstSubresource.mipLevel = i;
// //         blit.dstSubresource.baseArrayLayer = 0;
// //         blit.dstSubresource.layerCount = layerCount;

// //         vkCmdBlitImage(cmdBuffer,
// //             image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
// //             image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
// //             1, &blit,
// //             VK_FILTER_LINEAR);

// //         barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
// //         barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; 
// //         barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
// //         barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

// //         vkCmdPipelineBarrier(cmdBuffer,
// //             VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
// //             0, nullptr,
// //             0, nullptr,
// //             1, &barrier);

// //         if (mipWidth > 1) mipWidth /= 2;
// //         if (mipHeight > 1) mipHeight /= 2;
// //     }

// //     barrier.subresourceRange.baseMipLevel = mipLevels - 1;
// //     barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
// //     barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
// //     barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
// //     barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

// //     vkCmdPipelineBarrier(cmdBuffer, 
// //             VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
// //             0, nullptr,
// //             0, nullptr,
// //             1, &barrier );

// //     util::endSingleTimeCommands(VyContext::device(), cmdBuffer, VyContext::device().getCommandPool(), VyContext::device().graphicsQueue());
// // }




// // ///////////////////////////////////////////////////////////////////////////////////////////
// // ///////////////////////////////////////////////////////////////////////////////////////////
// // ///////////////////////////////////////////////////////////////////////////////////////////
// // ///////////////////////////////////////////////////////////////////////////////////////////
// // ///////////////////////////////////////////////////////////////////////////////////////////

// // /*  ------------------

// //     ----Cube Map-----

// //     ------------------*/
// // VyCubemap::VyCubemap(const String& path) :
// //     IVyTexture()
// // {
// //     createCubemapImage(path);
// //     createCubemapImageView();
// //     createCubemapSampler();
// //     createDescriptorInfo();
// // }



// // VyCubemap::~VyCubemap(){

// // }



// // void VyCubemap::createCubemapImage(const String& path) 
// // {
// //     int width, height;
// //     const float* pixels = stbi_loadf(path.data(), &width, &height, nullptr, 4);
// //         if (!pixels) {
// //             String reason = stbi_failure_reason();
// //             VY_THROW_RUNTIME_ERROR("Failed to load cubemap image ' " + path + " ' : "+ reason);}
// //         if (width<=0||height<=0){
// //             VY_THROW_RUNTIME_ERROR("Loaded cubemap image has invalid dimensions!");
// //         }

    
// //     VyBitmap in(width, height, 4, VY_BITMAP_FORMAT_FLOAT, pixels); 
// //     //convert to vertical cross
// //     VyBitmap out = convertEquirectangularMapToVerticalCross(in);    
// //     stbi_image_free((void*)pixels);
    
// //     //write to disk to debug
// //     // stbi_write_hdr("./loaded_hdr_toVertical.hdr", out.W, out.H, out.Channels,
// //     //                 (const float*)out.Data.data());
    
// //     m_Cubemap = convertVerticalCrossToCubeMapFaces(out);  //create cubemap
// //     VkDeviceSize faceSize = m_Cubemap.W * m_Cubemap.H * m_Cubemap.Channels * VyBitmap::getBytesPerChannel(m_Cubemap.Format) ; //float, 4 channels
// //     VkDeviceSize totalSize = faceSize * 6;

// //     //find miplevels figure
// //     m_MipLevels = static_cast<U32>(std::floor(std::log2(std::max(m_Cubemap.W, m_Cubemap.H))))+1;
// //     //tex width is one m_Cubemap face width
// //     texWidth = m_Cubemap.W;
// //     texHeight = m_Cubemap.H;
// //     texChannels = m_Cubemap.Channels;


// //     // Debug checks
// //     std::cout << "DEBUG: m_Cubemap.Data.size() = " << m_Cubemap.Data.size() << std::endl;
// //     std::cout << "DEBUG: m_Cubemap.Depth = " << m_Cubemap.Depth << std::endl;
// //     std::cout << "DEBUG: cubemap dimensions: " << m_Cubemap.W << "x" << m_Cubemap.H << "x" << m_Cubemap.Channels << std::endl;

// //     // Verify data size matches expectation
// //     if (m_Cubemap.Data.size() != totalSize) {
// //         VY_THROW_RUNTIME_ERROR("Cubemap data size mismatch!");
// //     }
// //     if (m_Cubemap.Depth != 6) {
// //         VY_THROW_RUNTIME_ERROR("Cubemap depth is not 6!");
// //     }

// //     //Staging buffer
// //     JBuffer stagingBuffer(totalSize, 
// //                 VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
// //     void* data;
// //     vkMapMemory(VyContext::device(), stagingBuffer.bufferMemory(), 0, stagingBuffer.getSize(), 0, &data);
// //     memcpy(data, m_Cubemap.Data.data(), static_cast<size_t>(totalSize));
// //     vkUnmapMemory(VyContext::device(), stagingBuffer.bufferMemory());

// //     // image create info
// //     auto imageInfo = ImageCreateInfoBuilder(texWidth, texHeight)
// //                     .mipLevels(m_MipLevels)
// //                     .arrayLayers(m_Cubemap.Depth)
// //                     .format(m_Cubemap.getVkFormat())
// //                     .flags(VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT) // for m_Cubemap especially
// //                     .usage(VK_IMAGE_USAGE_TRANSFER_DST_BIT|VK_IMAGE_USAGE_TRANSFER_SRC_BIT|VK_IMAGE_USAGE_SAMPLED_BIT)
// //                     .getInfo();

// // 	VmaAllocationCreateInfo allocInfo{};
// //     {
// //         allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
// //     }

// // 	VyContext::device().createImage(m_Image, m_Allocation, imageInfo, allocInfo);

// //     VkResult result = VyContext::device().createImage(m_TextureImage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImageMemory_);
// //     if (result != VK_SUCCESS) {
// //         VY_THROW_RUNTIME_ERROR("Failed to create cubemap image! VkResult: " + std::to_string(result));
// //     }
    
// //     JCommandBuffer cmdBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY);
// //     cmdBuffer.beginSingleTimeCommands();

// //     VyContext::device().transitionImageLayout(cmdBuffer, m_TextureImage,
// //         VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
// //         VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
// //         VK_IMAGE_ASPECT_COLOR_BIT, m_MipLevels, 6);

    
// //     copyBufferToImage_multiple(cmdBuffer, 
// //         stagingBuffer.buffer(), m_TextureImage,
// //         (U32)texWidth, (U32)texHeight,
// //         faceSize, 6   );

// // // no transition again, egerate mipmaps will transfer everything back


// //     cmdBuffer.endSingleTimeCommands(VyContext::device().graphicsQueue());

// //     //generate mipmaps for cubemap (6 layers)
// //     generateMipmaps(m_TextureImage, m_Cubemap.getVkFormat(), texWidth, texHeight, m_MipLevels, 6);

// // }




// // void VyCubemap::createCubemapImageView()
// // {
// //     auto viewInfo = ImageViewCreateInfoBuilder(m_TextureImage)
// //         .mipLevels(0, m_MipLevels)
// //         .viewType(VK_IMAGE_VIEW_TYPE_CUBE)
// //         .format(m_Cubemap.getVkFormat())
// //         .arrayLayers(0, static_cast<U32>(m_Cubemap.Depth))
// //         .getInfo();

// //     VkResult result = VyContext::device().createImageView(textureImageView_, viewInfo);
// //     if (result != VK_SUCCESS) 
// //     {
// //         VY_THROW_RUNTIME_ERROR("Failed to create cubemap image view! VkResult: " + std::to_string(result));
// //     }
// // }




// // void VyCubemap::createCubemapSampler() 
// // {
// //     VkPhysicalDeviceProperties properties{};
// //     vkGetPhysicalDeviceProperties(VyContext::device().physicalDevice(), &properties);

// //     auto samplerInfo = SamplerCreateInfoBuilder()
// //                         .addressMode(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE)
// //                         .maxLod(m_MipLevels)
// //                         .compareOp(VK_COMPARE_OP_NEVER)
// //                         .maxAnisotropy(properties.limits.maxSamplerAnisotropy)
// //                         .getInfo();

// //     if (vkCreateSampler(VyContext::device(), &samplerInfo, nullptr, &textureSampler_) != VK_SUCCESS) {
// //         VY_THROW_RUNTIME_ERROR("Failed to create cubemap sampler!");
// //     }
// // }


// // void VyCubemap::copyBufferToImage_multiple(VkCommandBuffer cmdBuffer,
// //     VkBuffer buffer, VkImage image, 
// //     U32 imgWidth, U32 imgHeight, 
// //     VkDeviceSize bufferOffset, U32 layers) 
// // {
// //         TVector<VkBufferImageCopy> regions(layers);
// //         for(U32 i = 0 ; i<layers; ++i){
// //             regions[i].bufferOffset = bufferOffset * i;
// //             regions[i].bufferRowLength = 0;
// //             regions[i].bufferImageHeight = 0;
// //             regions[i].imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
// //             regions[i].imageSubresource.mipLevel = 0;
// //             regions[i].imageSubresource.baseArrayLayer = i; // 逐层
// //             regions[i].imageSubresource.layerCount = 1;
// //             regions[i].imageOffset = {0, 0, 0};
// //             regions[i].imageExtent = { (U32)texWidth, (U32)texHeight, 1 };
// //         }

// //         vkCmdCopyBufferToImage(
// //             cmdBuffer,
// //             buffer,
// //             m_TextureImage,
// //             VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
// //             static_cast<U32>(regions.size()),
// //             regions.data()
// //         );
// // }


// ///////////////////////////////////////////////////////////////////////////////////////////
// ///////////////////////////////////////////////////////////////////////////////////////////
// ///////////////////////////////////////////////////////////////////////////////////////////
// ///////////////////////////////////////////////////////////////////////////////////////////
// ///////////////////////////////////////////////////////////////////////////////////////////






// //     namespace TexUtils
// //     {

// //     void UploadKtxToTexture(JDevice& device,
// //                                 ktxTexture2* ktxTex,
// //                                 VyTextureBase& dstTex,
// //                                 bool isCube)
// //     {
// //         //ktx, has layers, face, mipmap . when comes to vulkan, face become layers.
// //         const ktx_size_t dataSize = ktxTex->dataSize; // raw image data size
// //         JBuffer stagingBuffer(dataSize,
// //                             VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
// //                             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

// //         void* mapped = nullptr;
// //         vkMapMemory(device.device(), stagingBuffer.bufferMemory(), 0, stagingBuffer.getSize(), 0, &mapped);
// //         memcpy(mapped, ktxTex->pData, dataSize);
// //         vkUnmapMemory(device.device(), stagingBuffer.bufferMemory());

// //         // Build copy regions for all levels and faces
// //         TVector<VkBufferImageCopy> regions;
// //         regions.reserve(ktxTex->numLevels * (isCube ? 6u : 1u));

// //         for (ktx_uint32_t level = 0; level < ktxTex->numLevels; ++level) {
// //             const ktx_size_t levelOffsetBase = 0; // offsets are absolute via ktxTexture_GetImageOffset
// //             const U32 w = std::max(1u, ktxTex->baseWidth >> level);
// //             const U32 h = std::max(1u, ktxTex->baseHeight >> level);

// //             const U32 faceCount = isCube ? 6u : 1u;


// //             for (U32 face = 0; face < faceCount; ++face) {
// //                 ktx_size_t offset = 0;
// //                 // layer = 0 for non-array textures
// //                 KTX_error_code result = ktxTexture2_GetImageOffset(ktxTex, level, /*layer*/0, /*faceSlice*/face, &offset);
// //                 if (result != KTX_SUCCESS) {
// //                     VY_THROW_RUNTIME_ERROR("ktxTexture_GetImageOffset Failed while preparing upload");
// //                 }

// //                 VkBufferImageCopy region{};
// //                 region.bufferOffset = levelOffsetBase + offset;
// //                 region.bufferRowLength = 0;
// //                 region.bufferImageHeight = 0;
// //                 region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
// //                 region.imageSubresource.mipLevel = level;
// //                 region.imageSubresource.baseArrayLayer = face; // each face into its layer
// //                 region.imageSubresource.layerCount = 1;
// //                 region.imageOffset = {0, 0, 0};
// //                 region.imageExtent = { w, h, 1 };
// //                 regions.push_back(region);        }
// //         }

// //         // Copy to image and transition to shader-read
// //         JCommandBuffer cmd(VK_COMMAND_BUFFER_LEVEL_PRIMARY);
// //         cmd.beginSingleTimeCommands();

// //         // Ensure image is in TRANSFER_DST for all mips/layers already (created that way)
// //         vkCmdCopyBufferToImage(cmd.getCommandBuffer(),
// //                             stagingBuffer.buffer(),
// //                             dstTex.textureImage(),
// //                             VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
// //                             static_cast<U32>(regions.size()),
// //                             regions.data());

// //         device.transitionImageLayout(cmd.getCommandBuffer(), dstTex.textureImage(),
// //                                     VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
// //                                     VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
// //                                     VK_IMAGE_ASPECT_COLOR_BIT, dstTex.getMipLevels(), isCube ? 6u : 1u);

// //         cmd.endSingleTimeCommands(device.graphicsQueue());
// //     }
// // }