// #pragma once

// #include <Vy/GFX/Backend/VK/VKCore.h>

// #include <Vy/GFX/Context.h>

// #include <iostream>
// // #define STB_IMAGE_IMPLEMENTATION
// #include <stb_image.h>
// #define STB_IMAGE_WRITE_IMPLEMENTATION
// #include <stb_image_write.h>

// namespace Vy
// {
//     enum VyBitmapType
//     {
//         VY_BITMAP_TYPE_2D,
//         VY_BITMAP_TYPE_CUBE,
//     };

//     enum VyBitmapFormat
//     {
//         VY_BITMAP_FORMAT_UBYTE,
//         VY_BITMAP_FORMAT_FLOAT,
//     };


//     struct VyBitmap
//     {
//         VyBitmap() = default;

//         VyBitmap(int w, int h,            int channels, VyBitmapFormat format);
//         VyBitmap(int w, int h, int depth, int channels, VyBitmapFormat format);
//         VyBitmap(int w, int h,            int channels, VyBitmapFormat format, const void* ptr);

//         int W          = 0;
//         int H          = 0;
//         int Depth      = 1;
//         int Channels   = 3;
        
//         VyBitmapFormat Format = VY_BITMAP_FORMAT_UBYTE;
//         VyBitmapType   Type   = VY_BITMAP_TYPE_2D;
//         TBlob          Data;


//     // Helpler function
//         static int getBytesPerChannel(VyBitmapFormat format)
//         {
//             if(format == VY_BITMAP_FORMAT_UBYTE) return 1; //unsigned byte is 1 byte (8bits)
//             if(format == VY_BITMAP_FORMAT_FLOAT)        return 4; //float is 4 bytes (32 bits)
//             return 0;
//         }
        
//         void setPixel(int x, int y, const Vec4& color)
//         {
//             (*this.*setPixelFunc)(x, y , color);
//         }
        
//         Vec4 getPixel(int x, int y) const
//         {
//             return ((*this.*getPixelFunc)(x, y));
//         }

//         VkFormat getVkFormat() const 
//         { 
//             return bitmapFormatToVkFormat(Format, Channels);
//         }

//     private:
//         using setPixel_t = void(VyBitmap::*)(int, int, const Vec4&); //for pointer who has (int, int , vec4) and no return 
//         using getPixel_t = Vec4(VyBitmap::*)(int, int) const;

//         setPixel_t setPixelFunc = &VyBitmap::setPixelUnsignedByte;;
//         getPixel_t getPixelFunc = &VyBitmap::getPixelUnsignedByte;

//         void initGetSetFuncs()
//         {
//             switch (Format)
//             {
//             case VY_BITMAP_FORMAT_UBYTE:
//                 setPixelFunc = &VyBitmap::setPixelUnsignedByte;
//                 getPixelFunc = &VyBitmap::getPixelUnsignedByte;
//                 break;
            
//             case VY_BITMAP_FORMAT_FLOAT:
//                 setPixelFunc = &VyBitmap::setPixelFloat;
//                 getPixelFunc = &VyBitmap::getPixelFloat;
//                 break;
//             }
//         }

//         void setPixelFloat(int x, int y, const Vec4& color)
//         {
//             if (!(x >= 0 && x < W && y >= 0 && y < H)) 
//             {
//                 std::cout << "setPixelFloat out of range: (" << x << ", " << y << ") size=("
//                         << W << ", " << H << ")" << std::endl;

//                 throw std::out_of_range("setPixelFloat out of range");
//             }
            
//             const int offset = Channels * (y * W + x);
//             float*    data   = reinterpret_cast<float*>(Data.data());

//             if(Channels > 0) { data[offset+0] = color.r; }
//             if(Channels > 1) { data[offset+1] = color.g; }
//             if(Channels > 2) { data[offset+2] = color.b; }
//             if(Channels > 3) { data[offset+3] = color.a; }
//         }

//         Vec4 getPixelFloat(int x, int y) const
//         {
//             // assert(x >= 0 && x < W && y >= 0 && y <H && "get pixel float out of range" ); 
//             if (!(x >= 0 && x < W && y >= 0 && y < H)) 
//             {
//                 std::cout << "getPixelFloat out of range: (" << x << ", " << y << ") size=("
//                         << W << ", " << H << ")" << std::endl;
            
//                 throw std::out_of_range("getPixelFloat out of range");
//             }
            
//             const int    offset = Channels * (y * W + x);
//             const float* data   = reinterpret_cast<const float*>(Data.data());
            
//             return Vec4(
//                 Channels > 0 ? data[offset + 0] : 0.0f,
//                 Channels > 1 ? data[offset + 1] : 0.0f,
//                 Channels > 2 ? data[offset + 2] : 0.0f,
//                 Channels > 3 ? data[offset + 3] : 0.0f
//             );
//         }

//         //rgbrgbrgb, not rrrgggbbb. stbi use interleaved (rgbrgbrgb)
//         void setPixelUnsignedByte(int x, int y, const Vec4& color)
//         {
//             const int offset = Channels * (y * W + x);

//             if (Channels > 0) { Data[offset + 0] = U8(color.r * 255.f); }
//             if (Channels > 1) { Data[offset + 1] = U8(color.g * 255.f); }
//             if (Channels > 2) { Data[offset + 2] = U8(color.b * 255.f); }
//             if (Channels > 3) { Data[offset + 3] = U8(color.a * 255.f); }
//         }

//         Vec4 getPixelUnsignedByte(int x, int y) const
//         {
//             const int offset = Channels * (y * W + x);

//             return Vec4(
//                 Channels > 0 ? float(Data[offset + 0]) / 255.f : 0.f,
//                 Channels > 1 ? float(Data[offset + 1]) / 255.f : 0.f,
//                 Channels > 2 ? float(Data[offset + 2]) / 255.f : 0.f,
//                 Channels > 3 ? float(Data[offset + 3]) / 255.f : 0.f  
//             );
//         }


//         VkFormat bitmapFormatToVkFormat(VyBitmapFormat format, int channel) const
//         {
//             switch (format) 
//             {
//             case VY_BITMAP_FORMAT_UBYTE:
//                 switch (channel) 
//                 {
//                 case 1: return VK_FORMAT_R8_UNORM;
//                 case 2: return VK_FORMAT_R8G8_UNORM;
//                 case 3: return VK_FORMAT_R8G8B8_UNORM;
//                 case 4: return VK_FORMAT_R8G8B8A8_UNORM;
//                 default: break;
//                 }
//                 break;

//             case VY_BITMAP_FORMAT_FLOAT:
//                 switch (channel) 
//                 {
//                 case 1: return VK_FORMAT_R32_SFLOAT;
//                 case 2: return VK_FORMAT_R32G32_SFLOAT;
//                 case 3: return VK_FORMAT_R32G32B32_SFLOAT;
//                 case 4: return VK_FORMAT_R32G32B32A32_SFLOAT;
//                 default: break;
//                 }
//                 break;
//         }

//         // if no compatibale
//         return VK_FORMAT_UNDEFINED;
//         }
//     };
// }






// namespace Vy
// {
//     ////////////////////////////////////////////////////////////////////////////////////////////////////
//     struct ImageViewCreateInfoBuilder
//     {
//         VkImageViewCreateInfo ViewInfo{};

//         ImageViewCreateInfoBuilder(VkImage image)
//         {
//             ViewInfo = VKInit::imageViewCreateInfo();
//             {
//                 ViewInfo.image    = image;

//                 ViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
//                 ViewInfo.format   = VK_FORMAT_R8G8B8A8_SRGB;

//                 ViewInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
//                 ViewInfo.subresourceRange.baseMipLevel   = 0;
//                 ViewInfo.subresourceRange.levelCount     = 1;
//                 ViewInfo.subresourceRange.baseArrayLayer = 0;
//                 ViewInfo.subresourceRange.layerCount     = 1;
//             }
//         }

//         ImageViewCreateInfoBuilder& viewType(VkImageViewType _type)
//         {
//             ViewInfo.viewType = _type; return *this; 
//         }

//         ImageViewCreateInfoBuilder& format(VkFormat _format)
//         {
//             ViewInfo.format = _format; return *this; 
//         }

//         ImageViewCreateInfoBuilder& aspectMask(VkImageAspectFlags _aspectMask)
//         {
//             ViewInfo.subresourceRange.aspectMask = _aspectMask; return *this; 
//         }
        
//         ImageViewCreateInfoBuilder& mipLevels(U32 _baseMipLevel, U32 _levelCount)
//         {
//             ViewInfo.subresourceRange.baseMipLevel = _baseMipLevel;
//             ViewInfo.subresourceRange.levelCount = _levelCount;
//             return *this; 
//         }

//         ImageViewCreateInfoBuilder& arrayLayers(U32 _baseArrayLayer, U32 _layerCount)
//         {
//             ViewInfo.subresourceRange.baseArrayLayer = _baseArrayLayer;
//             ViewInfo.subresourceRange.layerCount     = _layerCount;
//             return *this; 
//         }

//         ImageViewCreateInfoBuilder& componentsRGBA(VkComponentSwizzle r, VkComponentSwizzle g, VkComponentSwizzle b, VkComponentSwizzle a)
//         {
//             ViewInfo.components.r = r;
//             ViewInfo.components.g = g;
//             ViewInfo.components.b = b;
//             ViewInfo.components.a = a;
//             return *this; 
//         }

//         VkImageViewCreateInfo getInfo() const 
//         {
//             return ViewInfo;
//         }
//     };


//     struct ImageCreateInfoBuilder
//     {
//         VkImageCreateInfo ImageInfo{};

//         ImageCreateInfoBuilder(U32 texWidth, U32 texHeight)
//         {
//             ImageInfo = VKInit::imageCreateInfo();
//             {
//                 ImageInfo.imageType     = VK_IMAGE_TYPE_2D;
//                 ImageInfo.extent.width  = texWidth;
//                 ImageInfo.extent.height = texHeight;
//                 ImageInfo.extent.depth  = 1;
//                 ImageInfo.mipLevels     = 1;
//                 ImageInfo.arrayLayers   = 1;
//                 ImageInfo.format        = VK_FORMAT_R8G8B8A8_SRGB;
//                 ImageInfo.tiling        = VK_IMAGE_TILING_OPTIMAL;
//                 ImageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
//                 ImageInfo.usage         = VK_IMAGE_USAGE_SAMPLED_BIT;
//                 ImageInfo.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;
//                 ImageInfo.samples       = VK_SAMPLE_COUNT_1_BIT; // For multisampling
//                 ImageInfo.flags         = 0;
//             }
//         }

//         ImageCreateInfoBuilder& imageCreateFlags(VkImageCreateFlags _flags)
//         {
//             ImageInfo.flags = _flags; return *this; 
//         }
        
//         ImageCreateInfoBuilder& imageType(VkImageType _type)
//         {
//             ImageInfo.imageType = _type; return *this; 
//         }

//         ImageCreateInfoBuilder& extentDepth(U32 _depth)
//         {
//             ImageInfo.extent.depth = _depth; return *this; 
//         }

//         ImageCreateInfoBuilder& mipLevels(U32 _mipLevels)
//         {
//             ImageInfo.mipLevels = _mipLevels; return *this; 
//         }

//         ImageCreateInfoBuilder& arrayLayers(U32 _arrayLayers)
//         {
//             ImageInfo.arrayLayers = _arrayLayers; return *this; 
//         }

//         ImageCreateInfoBuilder& tiling(VkImageTiling _tiling)
//         {
//             ImageInfo.tiling = _tiling; return *this; 
//         }

//         ImageCreateInfoBuilder& usage(VkImageUsageFlags _usage)
//         {
//             ImageInfo.usage = _usage; return *this; 
//         }

//         ImageCreateInfoBuilder& sharingMode(VkSharingMode _sharingMode)
//         {
//             ImageInfo.sharingMode = _sharingMode; return *this; 
//         }

//         ImageCreateInfoBuilder& samples(VkSampleCountFlagBits _samples)
//         {
//             ImageInfo.samples = _samples; return *this; 
//         }

//         ImageCreateInfoBuilder& initialLayout(VkImageLayout _initialLayout)
//         {
//             ImageInfo.initialLayout = _initialLayout; return *this; 
//         }

//         ImageCreateInfoBuilder& format(VkFormat _format)
//         {
//             ImageInfo.format = _format; return *this; 
//         }

//         ImageCreateInfoBuilder& flags(VkImageCreateFlags _flags)
//         {
//             ImageInfo.flags = _flags; return *this; 
//         }

//         VkImageCreateInfo getInfo() const 
//         {
//             return ImageInfo;
//         }
//     };
// }








// namespace Vy
// {
//     inline int bytesPerPixel(VkFormat format)
//     {
//         switch(format)
//         {
//             case VK_FORMAT_R8_UNORM: 
//             case VK_FORMAT_R8_SRGB: 
//                 return 1;

//             case VK_FORMAT_R8G8_UNORM: 
//             case VK_FORMAT_R8G8_SRGB:
//                 return 2;

//             case VK_FORMAT_R8G8B8_UNORM:
//             case VK_FORMAT_R8G8B8_SRGB:
//                 return 3;

//             case VK_FORMAT_R8G8B8A8_UNORM:
//             case VK_FORMAT_R8G8B8A8_SRGB:
//                 return 4;

//             case VK_FORMAT_R16G16B16A16_SFLOAT:
//                 return 8;

//             case VK_FORMAT_R32G32B32A32_SFLOAT:
//                 return 16; // 32 bits × 4 channels

//             default:
//                 VY_THROW_RUNTIME_ERROR("bytesPerPixel() not implemented for this VkFormat");
        
//         }
//     }



//     ///sampler
//     //create global sampler

//     enum class SamplerType 
//     {
//         TextureGlobal,
//         SkyboxLinearClamp, //see in the viewport
//     };

//     //universe sampler, the miplevel is controlled by image view, not the sampler side

//     class VySamplerManager
//     {
//     public:
//         explicit VySamplerManager();

//         //including both customized sampler(later added) and default samplers

//         //init all universal ones
//         void initDefault();

//         void addSampler(const String& name, VkSampler newSampler);

//         VkSampler getSampler(const String& name);      // get customized
//         VkSampler getSampler(SamplerType samplerType); // get default

//         void destroy();  //need explicit call

//     private:
//         float m_MaxSamplerAnisotropy;

//         THashMap<String,      VkSampler> m_CustomSamplers;
//         THashMap<SamplerType, VkSampler> m_DefaultSamplers;

//         //individual build
//         VkSampler build(SamplerType samplerType);
//     };

//     //individual one just use the samplerInfoBuilder.. all been added to samplers_, in the end they all be destroyed


//     ///////////////////////////////////////////////////////////////////////////////////////////
//     ///////////////////////////////////////////////////////////////////////////////////////////
//     //CONFIG area

//     struct VyTextureConfig
//     {
//         // Common
//         VkFormat                Format;
//         U32                     Channels;

//         // Image
//         VkImageType             ImageType{VK_IMAGE_TYPE_2D};
//         VkImageUsageFlags       UsageFlags;
//         VkImageCreateFlags      CreateFlags{0}; //for cube compatibility
//         VkExtent3D              Extent; //dimension - w, h, 1. this is for 3D texture
//         Optional<U32>           MipLevels;

//         Optional<void*>         Data; //can provide, or just empty one

//         VkImageLayout           NewLayout{VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL};
        
//         // View
//         VkImageViewType         ViewType{VK_IMAGE_VIEW_TYPE_2D};  //2D or cube
//         U32                     ArrayLayers{1};
//     };



//     //when read general textures, it si from vk_layout_undefined -> vk_layout_transfer_dst -> vk_layout_shader_read_only_optimizal
//     inline VyTextureConfig cubeMapConfig(U32 width, U32 height, VkFormat format, U32 mipLevels)
//     {
//         VyTextureConfig config;
//         {
//             config.ImageType        = VK_IMAGE_TYPE_2D;
//             config.ViewType         = VK_IMAGE_VIEW_TYPE_CUBE;
            
//             config.Extent           = { width, height, 1 };
//             config.Format           = format;
//             config.ArrayLayers      = 6;
            
//             config.CreateFlags      = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
//             config.UsageFlags       = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
//             config.NewLayout        = VK_IMAGE_LAYOUT_GENERAL;
//         }

//         return config;
//     }

//     inline VyTextureConfig prefilterEnvMapConfig(U32 width, U32 height, VkFormat format)
//     {
//         U32 mips = static_cast<U32>(std::floor(std::log2(std::max(width, height)))) + 1;

//         VyTextureConfig config = cubeMapConfig(width, height, format, mips);
//         {
//             config.UsageFlags |= VK_IMAGE_USAGE_STORAGE_BIT;
//         }

//         return config;
//     }



//     ///////////////////////////////////////////////////////////////////////////////////////////
//     ///////////////////////////////////////////////////////////////////////////////////////////
//     ///////////////////////////////////////////////////////////////////////////////////////////
//     ///////////////////////////////////////////////////////////////////////////////////////////


//     //create empty texture. should be able to inheritance
//     //NO DATA WRITE IN
//     class VyTextureBase
//     {
//     public:
//         VyTextureBase(VyTextureConfig& textureConfig);

//         virtual ~VyTextureBase();

//         VkImage     textureImage() const { return m_TextureBaseImage;     }
//         VkImageView textureView()  const { return m_TextureBaseImageView; }
//         int         width()        const { return m_TexWidth;             }
//         int         height()       const { return m_TexHeight;            }
//         int         channels()     const { return m_TexChannels;          }
//         U32         mipLevels()    const { return m_MipLevels;            }

//         VkImageView switchViewForMip(U32 selectMip, VkImageViewType vType);

//         //only use when has customized sampler.. doing this for manage customized sampler inside this texture life cycle
//         void createCustomSampler(const VkSamplerCreateInfo& samplerInfo); //create sampler inside, so store in the class

//         VkDescriptorImageInfo descriptorImageInfo() const 
//         {   
//             if (!m_CustomSampler.has_value())
//             {
//                 VY_THROW_RUNTIME_ERROR("No sampler assigned when creating this texture object");
//             }
            
//             return VkDescriptorImageInfo{
//                 *m_CustomSampler,
//                 m_TextureBaseImageView,
//                 VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL 
//             };        
//         }

//     private:
//         void createTextureBase();

//     protected:
//         VyTextureConfig m_Config;    

//         int           m_TexWidth;
//         int           m_TexHeight;
//         int           m_TexChannels;
//         U32           m_MipLevels;
//         VkImage       m_TextureBaseImage;
//         VkImageView   m_TextureBaseImageView;
//         VmaAllocation m_TextureBaseAllocation;

//         Optional<VkSampler> m_CustomSampler; //for unique customized sampler store inside

//         //for use
//         void copyBufferToImage(
//             VkCommandBuffer cmdBuffer,
//             VkBuffer buffer, 
//             VkImage image, 
//             U32 width, 
//             U32 height,
//             VkDeviceSize bufferOffset = 0, 
//             U32 layers = 1
//         );

//         void generateMipmaps(
//             VkImage image, 
//             VkFormat imageFormat, 
//             I32 texWidth, 
//             I32 texHeight, 
//             U32 mipLevels, 
//             U32 layerCount = 1
//         );

//     protected:
//         // protected constructor for child class
//         VyTextureBase() : 
//             m_TexWidth(0), 
//             m_TexHeight(0), 
//             m_TexChannels(0),
//             m_MipLevels(0),
//             m_TextureBaseImage(VK_NULL_HANDLE),
//             m_TextureBaseImageView(VK_NULL_HANDLE),
//             m_TextureBaseAllocation(VK_NULL_HANDLE)
//         {
//         }
//     };    


    
//     class VyTexture2D: public VyTextureBase
//     {
//     public:
//         VyTexture2D(const String& path, VkFormat format);

//         ~VyTexture2D() override;

//         VkDescriptorImageInfo descriptorImageInfo(VkSampler sampler) const 
//         {
//             return  VkDescriptorImageInfo{
//                 sampler,
//                 m_TextureBaseImageView,
//                 VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,   
//             };        
//         }

//     private:
//         stbi_uc* m_Pixels;

//         VyTextureConfig createConfig(const String& path, VkFormat format);

//         void createTextureImage();
//         void createTextureImageView();
//     };

//     //---------------------------------------------------------------------------------------
//     class VyColorTexture: public VyTextureBase
//     {
//     public:
//         VyColorTexture(float r, float g, float b);

//         ~VyColorTexture() override;
    
//         VkDescriptorImageInfo descriptorImageInfo(VkSampler sampler) const 
//         {
//             return  VkDescriptorImageInfo{
//                 sampler,
//                 m_TextureBaseImageView,
//                 VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
//             };        
//         }
    
//     private:
//         U8* m_Pixels;

//         VyTextureConfig createConfig();

//         void generateData(float r, float g, float b);
    
//         void createTextureImage();
//         void createTextureImageView();
//     };

//     ///////////////////////////////////////////////////////////////////////////////////////////


//     /*  
//         0 : albedo
//         1 : roughness
//         2 : metallic
//         3 : normal     
//     */
//     class VyMaterial
//     {
//     public:

//         VyMaterial(
//             Shared<VyDescriptorAllocator> descriptorAllocator,
//             VkDescriptorSetLayout         descriptorSetLayout,
//             VySamplerManager&             samplerManager
//         );
        
//         ~VyMaterial();

//         // bind loaded in texture, with, the corresponding pbr set layout, and this material's descriptor set
//         void setAlbedoTexture   (const VyTexture2D& albedoMap);
//         void setRoughnessTexture(const VyTexture2D& roughnessMap);
//         void setMetallicTexture (const VyTexture2D& metallicMap);
//         void setNormalTexture   (const VyTexture2D& normalMap);

//         // build material after loading all textures
//         void update();


//         // bind with pipeline during command call
//         void bind(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout);

//     private:
//         VySamplerManager&             m_SamplerManager;
//         Shared<VyDescriptorAllocator> m_DescriptorAllocator;
//         VkDescriptorSet               m_MaterialDescriptorSet;

//         // base color
//         Shared<VyColorTexture> DefaultWhite;
//         Shared<VyColorTexture> DefaultBlack;
//         Shared<VyColorTexture> DefaultGrey;
//         Shared<VyColorTexture> DefaultNormal;
        

//         //descriptor writes, will flush in the end
//         std::vector<VkWriteDescriptorSet>  m_DescriptorWrites;
//         std::vector<VkDescriptorImageInfo> m_ImageInfos;

//         void initDefault();
//     };







//     //legacy one, create texture for asset
//     // class IVyTexture
//     // {
//     // public:
//     //     IVyTexture(const String& path);
//     //     virtual ~IVyTexture();
        
//     //     VkImageView textureImageView() const            {return textureImageView_;}
//     //     VkSampler textureSampler() const                {return textureSampler_;}
//     //     int getTextureWidth() const                     {return texWidth;}
//     //     int getTextureHeight() const                    {return texHeight;}
//     //     int getTextureChannels() const                  {return texChannels;}
//     //     U32 getMipLevels() const                   {return m_MipLevels;}
//     //     const VkDescriptorImageInfo& descriptorImageInfo() const {return descriptorImageInfo_;}

//     //     static void copyBufferToImage(VkCommandBuffer cmdBuffer, VkBuffer buffer, VkImage image, U32 width, U32 height) ;
        
//     // private:
//     //     void createTextureImage(const String& path);    
//     //     void createTextureImageView();   
//     //     void createTextureSampler() ;

//     // protected:

//     //     int                         texWidth;
//     //     int                         texHeight;
//     //     int                         texChannels;
//     //     U32                         m_MipLevels;
//     //     VkImage                     m_TextureImage;
//     //     VkImageView                 textureImageView_;
//     //     VmaAllocation              textureImageMemory_;
//     //     VkSampler                   textureSampler_;
//     //     VkDescriptorImageInfo       descriptorImageInfo_;

    
//     //     void createDescriptorInfo();
//     //     void generateMipmaps(VkImage image, VkFormat imageFormat, 
//     //         I32 texWidth, I32 texHeight, U32 mipLevels, U32 layerCount=1);


//     // protected:
//     //     // protected constructor for child class
//     //     IVyTexture() : 
//     //         texWidth(0), texHeight(0), texChannels(0),
//     //         m_MipLevels(0),
//     //         m_TextureImage(VK_NULL_HANDLE),
//     //         textureImageView_(VK_NULL_HANDLE),
//     //         textureImageMemory_(VK_NULL_HANDLE),
//     //         textureSampler_(VK_NULL_HANDLE)
//     //     {}

//     // };


//     // //---------------------------------------------------------

//     // class VyCubemap: public IVyTexture
//     // {
//     // public:
//     //     VyCubemap(const String& path);
//     //     ~VyCubemap() override;

//     // private:
//     //     VyBitmap m_Cubemap;

//     //     void createCubemapImage(const String& path);
//     //     void createCubemapImageView();
//     //     void createCubemapSampler();
//     //     void copyBufferToImage_multiple(VkCommandBuffer cmdBuffer,
//     //         VkBuffer buffer, VkImage image, 
//     //         U32 imgWidth, U32 imgHeight, 
//     //         VkDeviceSize bufferOffset, U32 layers);
//     // };


        
        






//     //---------------------------------------------------
//     struct SamplerCreateInfoBuilder
//     {
//         VkSamplerCreateInfo samplerInfo{};
        
//         SamplerCreateInfoBuilder()
//         {
//             samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
//             samplerInfo.magFilter = VK_FILTER_LINEAR;
//             samplerInfo.minFilter = VK_FILTER_LINEAR;
//             samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
//             samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
//             samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
//             samplerInfo.anisotropyEnable = VK_TRUE;
//             samplerInfo.maxAnisotropy = 1.0f;
//             samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
//             samplerInfo.unnormalizedCoordinates = VK_FALSE;
//             samplerInfo.compareEnable = VK_FALSE;
//             samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;  //when compareEnable is false, compareOP doesnt matter
//             samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
//             samplerInfo.mipLodBias = 0.0f;
//             samplerInfo.minLod = 0.0f;
//             samplerInfo.maxLod = VK_LOD_CLAMP_NONE;
//         }
        
//         SamplerCreateInfoBuilder& addressMode(VkSamplerAddressMode u, VkSamplerAddressMode v, VkSamplerAddressMode w)
//         {
//             samplerInfo.addressModeU = u;
//             samplerInfo.addressModeV = v;
//             samplerInfo.addressModeW = w;
//             return *this; 
//         }
        
//         SamplerCreateInfoBuilder& maxAnisotropy(float _maxAni)
//         {
//             samplerInfo.maxAnisotropy = _maxAni; return *this; 
//         }
        
//         SamplerCreateInfoBuilder& borderColor(VkBorderColor _color)
//         {
//             samplerInfo.borderColor = _color; return *this; 
//         }
        
//         SamplerCreateInfoBuilder& mipmapMode(VkSamplerMipmapMode _mipmapMode)
//         {
//             samplerInfo.mipmapMode = _mipmapMode; return *this; 
//         } 
        
//         SamplerCreateInfoBuilder& maxLod(float _maxLod)
//         {
//             samplerInfo.maxLod = _maxLod; return *this; 
//         }

//         SamplerCreateInfoBuilder& compareOp(VkCompareOp _compareOp)
//         {
//             samplerInfo.compareOp = _compareOp; return *this; 
//         } 

//         VkSamplerCreateInfo getInfo() const 
//         {
//             return samplerInfo;
//         }
//     };
// }




