// #pragma once

// #include <VyEngine/GFX/Resources/Texture/Texture.h>
// #include <VyEngine/VK/Device/Device.h>
// #include <VyEngine/VK/Buffer/Buffer.h>
// #include <VyEngine/VK/Renderer/OffscreenRenderer.h>

// namespace Vy
// {
//     class VyTextureSystem final
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
//         static I32 getBytesPerPixel(VkFormat format);

//         template <typename T>
//         static void validateFormatCompatibility(VkFormat format)
//         {
//             // Handle uint8_t formats
//             if constexpr (std::is_same<T, uint8_t>::value)
//             {
//                 if (format != VK_FORMAT_R8_UNORM && format != VK_FORMAT_R8_SNORM &&
//                     format != VK_FORMAT_R8_UINT && format != VK_FORMAT_R8_SRGB &&
//                     format != VK_FORMAT_R8G8_UNORM && format != VK_FORMAT_R8G8_SNORM &&
//                     format != VK_FORMAT_R8G8_UINT && format != VK_FORMAT_R8G8_SRGB &&
//                     format != VK_FORMAT_R8G8B8_UNORM && format != VK_FORMAT_R8G8B8_SNORM &&
//                     format != VK_FORMAT_R8G8B8_UINT && format != VK_FORMAT_R8G8B8_SRGB &&
//                     format != VK_FORMAT_R8G8B8A8_UNORM && format != VK_FORMAT_R8G8B8A8_SNORM &&
//                     format != VK_FORMAT_R8G8B8A8_UINT && format != VK_FORMAT_R8G8B8A8_SRGB)
//                 {
//                     throw std::invalid_argument("Format is incompatible with uint8_t data type.");
//                 }
//             }
//             // Handle int8_t formats
//             else if constexpr (std::is_same<T, int8_t>::value)
//             {
//                 if (format != VK_FORMAT_R8_SINT && format != VK_FORMAT_R8G8_SINT &&
//                     format != VK_FORMAT_R8G8B8_SINT && format != VK_FORMAT_R8G8B8A8_SINT)
//                 {
//                     throw std::invalid_argument("Format is incompatible with int8_t data type.");
//                 }
//             }
//             // Handle float formats
//             else if constexpr (std::is_same<T, float>::value)
//             {
//                 if (format != VK_FORMAT_R16_SFLOAT && format != VK_FORMAT_R16G16_SFLOAT &&
//                     format != VK_FORMAT_R16G16B16_SFLOAT && format != VK_FORMAT_R16G16B16A16_SFLOAT &&
//                     format != VK_FORMAT_R32_SFLOAT && format != VK_FORMAT_R32G32_SFLOAT &&
//                     format != VK_FORMAT_R32G32B32_SFLOAT && format != VK_FORMAT_R32G32B32A32_SFLOAT)
//                 {
//                     throw std::invalid_argument("Format is incompatible with float data type.");
//                 }
//             }
//             else if constexpr (std::is_same<T, uint16_t>::value)
//             {
//                 if (format != VK_FORMAT_R16_UNORM && format != VK_FORMAT_R16_SNORM && format != VK_FORMAT_R16_UINT && format != VK_FORMAT_R16_SINT && format != VK_FORMAT_R16_SFLOAT &&
//                     format != VK_FORMAT_R16G16_UNORM && format != VK_FORMAT_R16G16_SNORM && format != VK_FORMAT_R16G16_UINT && format != VK_FORMAT_R16G16_SINT && format != VK_FORMAT_R16G16_SFLOAT &&
//                     format != VK_FORMAT_R16G16B16_UNORM && format != VK_FORMAT_R16G16B16_SNORM && format != VK_FORMAT_R16G16B16_UINT && format != VK_FORMAT_R16G16B16_SINT && format != VK_FORMAT_R16G16B16_SFLOAT &&
//                     format != VK_FORMAT_R16G16B16A16_UNORM && format != VK_FORMAT_R16G16B16A16_SNORM && format != VK_FORMAT_R16G16B16A16_UINT && format != VK_FORMAT_R16G16B16A16_SINT && format != VK_FORMAT_R16G16B16A16_SFLOAT)
//                 {
//                     throw std::invalid_argument("Format is incompatible with uint16_t data type.");
//                 }
//             }
//             // Handle int32_t formats
//             else if constexpr (std::is_same<T, int32_t>::value)
//             {
//                 if (format != VK_FORMAT_R32_SINT && format != VK_FORMAT_R32G32_SINT &&
//                     format != VK_FORMAT_R32G32B32_SINT && format != VK_FORMAT_R32G32B32A32_SINT)
//                 {
//                     throw std::invalid_argument("Format is incompatible with int32_t data type.");
//                 }
//             }
//             else
//             {
//                 throw std::invalid_argument("Unsupported data type.");
//             }
//         }

//     public:
//         VyTextureSystem();
//         ~VyTextureSystem() = default;

//         VyTextureSystem(const VyTextureSystem&) = delete;
//         VyTextureSystem& operator=(const VyTextureSystem&) = delete;

//         // if _overrideFormat is different from VK_FORMAT_UNDEFINED, it uses whatever has been passed, 
//         // otherwise use channels definition to figure out what format to use
//         Shared<VyTexture> loadTexture(const TString& path, VkFormat overrideFormat = VK_FORMAT_UNDEFINED);
//         Shared<VyTexture> loadTexture(const TString& name, VkFormat format, const U8* pData, I32 width, I32 height);
//         Shared<VyTexture> loadTexture(const DefaultTextureType& defaultTexture);
//         Shared<VyTexture> loadCubemap(const TArray<TString, 6>& paths, VkFormat overrideFormat = VK_FORMAT_UNDEFINED);
//         Shared<VyTexture> loadCubemap(const TString& hdrPath, VkFormat overrideFormat = VK_FORMAT_UNDEFINED);
        
//         Shared<VyTexture> generateBRDFLutTexture(const TString& name, VkExtent2D extent);
//         Shared<VyTexture> generateIrradianceCubemap(const TString& name, U32 faceSize, Shared<VyTexture> environment);
//         Shared<VyTexture> generatePreFilteredEnvironmentMap(const TString& name, U32 faceSize, Shared<VyTexture> environment);
        
//         void cleanup();
    

//         VY_INLINE Shared<VyTexture> getTexture(U32 index) const
//         {
//             VY_ASSERT(index >= 0 && index < m_Textures.size(), "Texture index is out of bound!");

//             return m_Textures[ index ];
//         }

//         VY_INLINE const TVector<Shared<VyTexture>>& getTextures() const
//         {
//             return m_Textures;
//         }

//         VY_INLINE const I32 getTextureIndex(const Shared<VyTexture>& texture) const
//         {
//             auto it = std::find(m_Textures.begin(), m_Textures.end(), texture);

//             if (it != m_Textures.end())
//             {
//                 return static_cast<I32>( std::distance( m_Textures.begin(), it ) );
//             }

//             return -1;
//         }

//     private:
    
//         U8* loadTextureData(const TString& path, I32& width, I32& height, I32& channels, I32 desiredChannels);
        
//         void freeTextureData(U8* pData);

//         void generateMipmaps(VkImage image, VkFormat format, I32 width, I32 height, U32 mipLevels);

//     private:
//         TVector<Shared<VyTexture>> m_Textures;
//         THashMap<U32, I32>         m_TextureLookup;
//     };
// }