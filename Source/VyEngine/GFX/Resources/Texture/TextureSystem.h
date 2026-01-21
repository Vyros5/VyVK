// #pragma once

// #include <VyEngine/VK/Device/Device.h>
// #include <VyEngine/VK/Buffer/Buffer.h>

// #include <VyEngine/VK/Image/Image.h>
// #include <VyEngine/VK/Image/ImageView.h>
// #include <VyEngine/VK/Image/Sampler.h>

// #include <VyEngine/VK/Context.h>

// #include <VyEngine/GFX/Resources/Mesh/Model.h>

// namespace Vy
// {
//     struct TextureData
//     {
//         VmaAllocation AllocationMemory{ VK_NULL_HANDLE };	// Memory allocation handled by VMA
//         VkImage Image{ VK_NULL_HANDLE };					// The Vulkan image
//         VkImageView ImageView{ VK_NULL_HANDLE };			// The image view for shader access
//         VkSampler Sampler{ VK_NULL_HANDLE };				// The sampler for texture filtering
//         I32 Index{ -1 };
//     };


//     class TextureSystem final
//     {
//     private:
//         struct DefaultTextureType
//         {
//             TString        Name;
//             TArray<U8, 4u> Buffer;
//             VkFormat       Format;
//             I32            Width;
//             I32            Height;
//         };

//     public:
//         // Common default textures
//         static const DefaultTextureType NormalTexture;

//         // Phong default textures
//         static const DefaultTextureType DiffuseTexture;
//         static const DefaultTextureType SpecularTexture;
//         static const DefaultTextureType AmbientTexture;
//         static const DefaultTextureType AlphaTexture;

//         // PBR default textures
//         static const DefaultTextureType RoughnessTexture;
//         static const DefaultTextureType MetallicTexture;
//         static const DefaultTextureType SheenTexture;
//         static const DefaultTextureType EmissiveTexture;
//         static const DefaultTextureType BaseColorTexture;
//         static const DefaultTextureType AOTexture;

//     public:
//         static I32 GetBytesPerPixel(VkFormat _format);

//         template <typename T>
//         static void ValidateFormatCompatibility(VkFormat format)
//         {
//             // Handle uint8_t formats
//             if constexpr (std::is_same<T, uint8_t>::value)
//             {
//                 if (format != VK_FORMAT_R8_UNORM       && format != VK_FORMAT_R8_SNORM       &&
//                     format != VK_FORMAT_R8_UINT        && format != VK_FORMAT_R8_SRGB        &&
//                     format != VK_FORMAT_R8G8_UNORM     && format != VK_FORMAT_R8G8_SNORM     &&
//                     format != VK_FORMAT_R8G8_UINT      && format != VK_FORMAT_R8G8_SRGB      &&
//                     format != VK_FORMAT_R8G8B8_UNORM   && format != VK_FORMAT_R8G8B8_SNORM   &&
//                     format != VK_FORMAT_R8G8B8_UINT    && format != VK_FORMAT_R8G8B8_SRGB    &&
//                     format != VK_FORMAT_R8G8B8A8_UNORM && format != VK_FORMAT_R8G8B8A8_SNORM &&
//                     format != VK_FORMAT_R8G8B8A8_UINT  && format != VK_FORMAT_R8G8B8A8_SRGB)
//                 {
//                     VY_THROW_INVALID_ARGUMENT("Format is incompatible with uint8_t data type.");
//                 }
//             }
//             // Handle int8_t formats
//             else if constexpr (std::is_same<T, int8_t>::value)
//             {
//                 if (format != VK_FORMAT_R8_SINT     && format != VK_FORMAT_R8G8_SINT &&
//                     format != VK_FORMAT_R8G8B8_SINT && format != VK_FORMAT_R8G8B8A8_SINT)
//                 {
//                     VY_THROW_INVALID_ARGUMENT("Format is incompatible with int8_t data type.");
//                 }
//             }
//             // Handle float formats
//             else if constexpr (std::is_same<T, float>::value)
//             {
//                 if (format != VK_FORMAT_R16_SFLOAT       && format != VK_FORMAT_R16G16_SFLOAT       &&
//                     format != VK_FORMAT_R16G16B16_SFLOAT && format != VK_FORMAT_R16G16B16A16_SFLOAT &&
//                     format != VK_FORMAT_R32_SFLOAT       && format != VK_FORMAT_R32G32_SFLOAT       &&
//                     format != VK_FORMAT_R32G32B32_SFLOAT && format != VK_FORMAT_R32G32B32A32_SFLOAT)
//                 {
//                     VY_THROW_INVALID_ARGUMENT("Format is incompatible with float data type.");
//                 }
//             }
//             else if constexpr (std::is_same<T, uint16_t>::value)
//             {
//                 if (format != VK_FORMAT_R16_UNORM          && format != VK_FORMAT_R16_SNORM && 
//                     format != VK_FORMAT_R16_UINT           && format != VK_FORMAT_R16_SINT  && 
//                     format != VK_FORMAT_R16_SFLOAT         &&

//                     format != VK_FORMAT_R16G16_UNORM       && format != VK_FORMAT_R16G16_SNORM && 
//                     format != VK_FORMAT_R16G16_UINT        && format != VK_FORMAT_R16G16_SINT  && 
//                     format != VK_FORMAT_R16G16_SFLOAT      &&

//                     format != VK_FORMAT_R16G16B16_UNORM    && format != VK_FORMAT_R16G16B16_SNORM && 
//                     format != VK_FORMAT_R16G16B16_UINT     && format != VK_FORMAT_R16G16B16_SINT  && 
//                     format != VK_FORMAT_R16G16B16_SFLOAT   &&

//                     format != VK_FORMAT_R16G16B16A16_UNORM && format != VK_FORMAT_R16G16B16A16_SNORM && 
//                     format != VK_FORMAT_R16G16B16A16_UINT  && format != VK_FORMAT_R16G16B16A16_SINT  && 
//                     format != VK_FORMAT_R16G16B16A16_SFLOAT)
//                 {
//                     VY_THROW_INVALID_ARGUMENT("Format is incompatible with uint16_t data type.");
//                 }
//             }
//             // Handle int32_t formats
//             else if constexpr (std::is_same<T, int32_t>::value)
//             {
//                 if (format != VK_FORMAT_R32_SINT       && format != VK_FORMAT_R32G32_SINT &&
//                     format != VK_FORMAT_R32G32B32_SINT && format != VK_FORMAT_R32G32B32A32_SINT)
//                 {
//                     VY_THROW_INVALID_ARGUMENT("Format is incompatible with int32_t data type.");
//                 }
//             }
//             else
//             {
//                 VY_THROW_INVALID_ARGUMENT("Unsupported data type.");
//             }
//         }


//     public:
//         TextureSystem();
//         ~TextureSystem() = default;

//         TextureSystem(const TextureSystem&) = delete;
//         TextureSystem& operator=(const TextureSystem&) = delete;

//         // if _overrideFormat is different from VK_FORMAT_UNDEFINED, it uses whatever has been passed, 
//         // otherwise use channels definition to figure out what format to use
//         Shared<TextureData> loadTexture(const TString& _path, VkFormat _overrideFormat = VK_FORMAT_UNDEFINED);
//         Shared<TextureData> loadTexture(const TString& _name, VkFormat _format, const U8* pData, I32 _width, I32 _height);
//         Shared<TextureData> loadTexture(const DefaultTextureType& _defaultTexture);

//         Shared<TextureData> loadCubemap(const TArray<TString, 6>& _paths, VkFormat _overrideFormat = VK_FORMAT_UNDEFINED);
//         Shared<TextureData> loadCubemap(const TString& _hdrPath, VkFormat _overrideFormat = VK_FORMAT_UNDEFINED);
        
//         void Cleanup();

//         Shared<TextureData> generateBRDFLutTexture(const TString& _name, VkExtent2D _extent);
//         Shared<TextureData> generateIrradianceCubemap(const TString& _name, U32 _faceSize, Shared<TextureData> _environment);
//         Shared<TextureData> generatePreFilteredEnvironmentMap(const TString& _name, U32 _faceSize, Shared<TextureData> _environment);

//         VY_INLINE Shared<TextureData> getTexture(U32 index) const
//         {
//             assert(index >= 0 && index < m_Textures.size() && "Texture index is out of bound!");
            
//             return m_Textures[ index ];
//         }

//         VY_INLINE const TVector<Shared<TextureData>>& getTextures() const
//         {
//             return m_Textures;
//         }

//         VY_INLINE const I32 getTextureIndex(const Shared<TextureData>& pTexture) const
//         {
//             auto it = std::find(m_Textures.begin(), m_Textures.end(), pTexture);

//             if (it != m_Textures.end())
//             {
//                 return static_cast<I32>(std::distance(m_Textures.begin(), it));
//             }

//             return -1;
//         }

//     private:
//         U8* loadTextureData(const TString& _path, I32& _width, I32& _height, I32& _channels, I32 _desired_channels);
        
//         void freeTextureData(U8* pData);
        
//         void generateMipmaps(VkImage _image, VkFormat _format, I32 _width, I32 _height, U32 _mipLevels);

//         template <typename T>
//         void createTextureImage(
//             const T*           pData, 
//             I32                _width, 
//             I32                _height, 
//             VkFormat           _format, 
//             VkImage&           _image, 
//             VmaAllocation&     _allocation,
//             U32                _layerCount = 1, 
//             U32                _mipLevels  = 1, 
//             VkImageCreateFlags _flags      = 0)
//         {
//             static_assert(
//                 std::is_same<T, uint8_t>::value || 
//                 std::is_same<T, float>::value   || 
//                 std::is_same<T, uint16_t>::value, 
//                 "Unsupported data type. Only U8, uint16 and float are allowed.");

//     #ifdef _DEBUG
//             ValidateFormatCompatibility<T>(_format);
//     #endif

//             const I32 bpp = GetBytesPerPixel(_format);
//             assert(bpp != -1 && "Unsupported image format");

//             VkDeviceSize imageSize = _width * _height * bpp * _layerCount;

//             m_Buffer->upload( pData, imageSize );

//             // Create the Vulkan image
//             auto image = VyImage::Builder{}
//                 .setName       ("ddd")
//                 .setImageType  (VK_IMAGE_TYPE_2D)
//                 .setFormat     (_format)
//                 .setExtent     (static_cast<U32>(_width), static_cast<U32>(_height))
//                 .setLevels     (_mipLevels)
//                 .setLayers     (_layerCount)
//                 .setSamples    (VK_SAMPLE_COUNT_1_BIT)
//                 .setTiling     (VK_IMAGE_TILING_OPTIMAL)
//                 .setLayout     (VK_IMAGE_LAYOUT_UNDEFINED)
//                 .setUsage      (VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
//                 .setSharing    (VK_SHARING_MODE_EXCLUSIVE)
//                 .setMemoryUsage(VMA_MEMORY_USAGE_AUTO)
//                 .build();

//             image.upload( *m_Buffer );

//             _image      = image.handle();
//             _allocation = image.allocation();
//         }

//         // nullptr does not have a deduced type, so cannot specialize the template, 
//         // so I made an overload, but I keep in the header for reference!
//         void createTextureImage(
//             const std::nullptr_t*, 
//             I32                width, 
//             I32                height, 
//             VkFormat           format, 
//             VkImage&           image, 
//             VmaAllocation&     allocation,
//             U32                layerCount, 
//             U32                mipLevels, 
//             VkImageCreateFlags flags)
//         {
//             // Create the Vulkan image
//             auto _image = VyImage::Builder{}
//                 .setName       ("ddd2")
//                 .setImageType  (VK_IMAGE_TYPE_2D)
//                 .setFormat     (format)
//                 .setExtent     (static_cast<U32>(width), static_cast<U32>(height))
//                 .setLevels     (mipLevels)
//                 .setLayers     (layerCount)
//                 .setSamples    (VK_SAMPLE_COUNT_1_BIT)
//                 .setTiling     (VK_IMAGE_TILING_OPTIMAL)
//                 .setLayout     (VK_IMAGE_LAYOUT_UNDEFINED)
//                 // Allow rendering and copying into this image
//                 .setUsage      (VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
//                 .setSharing    (VK_SHARING_MODE_EXCLUSIVE)
//                 .setFlags      (flags)
//                 .setMemoryUsage(VMA_MEMORY_USAGE_AUTO)
//                 .build();

//             _image.transitionLayout( VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL );

//             image      = _image.handle();
//             allocation = _image.allocation();
//         }

//         VkImageView createImageView(VkImage _image, VkFormat _format, U32 _layerCount = 1, U32 _mipLevels = 1);
        
//         VkSampler createTextureSampler(float _maxLod = 1.0f);

//     private:
//         Unique<VyBuffer>             m_Buffer;
//         TVector<Shared<TextureData>> m_Textures;
//         THashMap<U32, I32>           m_TextureLookup;
//     };
// }