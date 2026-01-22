#include <VyEngine/GFX/Resources/Texture/TextureSystem.h>
#include <VyEngine/VK/Context.h>

// #include <VyEngine/Globals.h>
// #include <VyLib/Util/String.h>
// #include <VyLib/Util/Hash.h>
// // #define STB_IMAGE_IMPLEMENTATION
// #include <stb_image.h>

// #include <iostream>

namespace Vy
{
    VyTextureManager::VyTextureManager(U32 maxTextures) : 
        m_MaxTextures( maxTextures ) 
    {
        m_TexturePool = VyDescriptorPool::Builder()
            .setMaxSets (1)
            .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, m_MaxTextures)
            .buildPtr();

        m_TextureSetLayout = VyDescriptorSetLayout::Builder()
            .addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, m_MaxTextures)
            .buildPtr();

        VyDescriptorWriter(*m_TextureSetLayout, *m_TexturePool)
            .build(m_TextureDescriptorSet);

        createDefaultAlbedoTexture();
        createDefaultNormalTexture();
        initializeDescriptorSet();
    }


    VyTextureManager::~VyTextureManager() 
    { 
    }


    void VyTextureManager::createDefaultAlbedoTexture() 
    {
        const TString path = "assets\\models\\checkerboard\\tiles_0059_color_1k.jpg";
        TString key = path;
        std::replace(key.begin(), key.end(), '\\', '/');

        m_DefaultAlbedoTexture = std::make_shared<VyTexture>(key, VK_FORMAT_R8G8B8A8_SRGB);
        m_Textures.push_back(m_DefaultAlbedoTexture);
        m_DefaultAlbedoIndex = static_cast<int>(m_Textures.size()) - 1;

        m_TexturesIndexMap[key] = m_DefaultAlbedoIndex;
    }

    
    void VyTextureManager::createDefaultNormalTexture() 
    {
        const TString path = "assets\\models\\checkerboard\\tiles_0059_normal_direct_1k.png";
        TString key = path;
        std::replace(key.begin(), key.end(), '\\', '/');

        m_DefaultNormalTexture = std::make_shared<VyTexture>(key, VK_FORMAT_R8G8B8A8_UNORM);
        m_Textures.push_back(m_DefaultNormalTexture);
        m_DefaultNormalIndex = static_cast<int>(m_Textures.size()) - 1;

        m_TexturesIndexMap[key] = m_DefaultNormalIndex;
    }

    
    void VyTextureManager::initializeDescriptorSet() 
    {
        m_ImageInfos.resize(m_MaxTextures);

        for (U32 i = 0; i < m_MaxTextures; i++) 
        {
            m_ImageInfos[i] = m_DefaultAlbedoTexture->descriptorImageInfo();
        }

        m_ImageInfos[ m_DefaultAlbedoIndex ] = m_DefaultAlbedoTexture->descriptorImageInfo();
        m_ImageInfos[ m_DefaultNormalIndex ] = m_DefaultNormalTexture->descriptorImageInfo();

        updateDescriptorSet();
    }

    
    int VyTextureManager::getOrLoadTexture(const TString& path, TextureSemantic semantic) 
    {
        // Normalize path
        TString key = path;
        std::replace(key.begin(), key.end(), '\\', '/');
        auto it = m_TexturesIndexMap.find(key);
        if (it != m_TexturesIndexMap.end()) 
        {
            return it->second; // Texture has already been loaded
        }

        VkFormat imgFormat{};

        if (semantic == TextureSemantic::BaseColor) 
        { 
            imgFormat = VK_FORMAT_R8G8B8A8_SRGB; 
        }
        else if (semantic == TextureSemantic::Normal) 
        { 
            imgFormat = VK_FORMAT_R8G8B8A8_UNORM; 
        }
        else 
        { 
            VY_THROW_INVALID_ARGUMENT("Unknown semantic");
        }

        auto tex = std::make_shared<VyTexture>(key, imgFormat);
        m_Textures.push_back(tex);

        int index = static_cast<int>(m_Textures.size()) - 1;
        m_TexturesIndexMap[ key ] = index;
        m_ImageInfos[ index ] = tex->descriptorImageInfo();

        updateDescriptorSet();
        return index;
    }

    
    void VyTextureManager::updateDescriptorSet() 
    {
        VyDescriptorWriter( *m_TextureSetLayout, *m_TexturePool )
            .writeImages( 0, m_ImageInfos.data(), m_ImageInfos.size() )
            .update( m_TextureDescriptorSet );
    }
}



// {
//     const TextureSystem::DefaultTextureType TextureSystem::NormalTexture =
//     {
//         "_NormalTexture_",
//         { 128, 128, 255, 255 },
//         VK_FORMAT_R8G8B8A8_UNORM,
//         1, 1
//     };

//     const TextureSystem::DefaultTextureType TextureSystem::DiffuseTexture =
//     {
//         "_DiffuseTexture_",
//         { 255, 255, 255, 255 },
//         VK_FORMAT_R8G8B8A8_SRGB,
//         1, 1
//     };

//     const TextureSystem::DefaultTextureType TextureSystem::SpecularTexture =
//     {
//         "_SpecularTexture_",
//         { 0, 0, 0, 255 },
//         VK_FORMAT_R8G8B8A8_UNORM,
//         1, 1
//     };

//     const TextureSystem::DefaultTextureType TextureSystem::AmbientTexture =
//     {
//             "_AmbientTexture_",
//             { 128, 128, 128, 255 },
//             VK_FORMAT_R8G8B8A8_SRGB,
//             1, 1
//     };

//     const TextureSystem::DefaultTextureType TextureSystem::AlphaTexture =
//     {
//             "_AlphaTexture_",
//             { 255, 255, 255, 255 },
//             VK_FORMAT_R8G8B8A8_UNORM,
//             1, 1
//     };

//     const TextureSystem::DefaultTextureType TextureSystem::RoughnessTexture =
//     {
//         "_RoughnessTexture_",
//         { 255, 255, 255, 255 },
//         VK_FORMAT_R8G8B8A8_UNORM,
//         1, 1
//     };

//     const TextureSystem::DefaultTextureType TextureSystem::MetallicTexture =
//     {
//         "_MetallicTexture_",
//         { 0, 0, 0, 255 },
//         VK_FORMAT_R8G8B8A8_UNORM,
//         1, 1
//     };

//     const TextureSystem::DefaultTextureType TextureSystem::SheenTexture =
//     {
//         "_SheenTexture_",
//         { 128, 128, 128, 255 },
//         VK_FORMAT_R8G8B8A8_UNORM,
//         1, 1
//     };

//     const TextureSystem::DefaultTextureType TextureSystem::EmissiveTexture =
//     {
//         "_EmissiveTexture_",
//         { 0, 0, 0, 255 },
//         VK_FORMAT_R8G8B8A8_SRGB,
//         1, 1
//     };

//     const TextureSystem::DefaultTextureType TextureSystem::BaseColorTexture =
//     {
//         "_BaseColorTexture_",
//         {255, 255, 255, 255},
//         VK_FORMAT_R8G8B8A8_SRGB,
//         1, 1 };

//     const TextureSystem::DefaultTextureType TextureSystem::AOTexture = 
//     {
//         "_AOTexture_",
//         {255, 255, 255, 255},
//         VK_FORMAT_R8G8B8A8_UNORM,
//         1, 1 };

//     I32 TextureSystem::GetBytesPerPixel(VkFormat _format)
//     {
//         switch (_format) {
//         case VK_FORMAT_R8_UNORM:
//         case VK_FORMAT_R8_SNORM:
//         case VK_FORMAT_R8_UINT:
//         case VK_FORMAT_R8_SINT:
//         case VK_FORMAT_R8_SRGB:
//             return 1; // 1 byte per pixel

//         case VK_FORMAT_R8G8_UNORM:
//         case VK_FORMAT_R8G8_SNORM:
//         case VK_FORMAT_R8G8_UINT:
//         case VK_FORMAT_R8G8_SINT:
//         case VK_FORMAT_R8G8_SRGB:
//             return 2; // 2 bytes per pixel

//         case VK_FORMAT_R8G8B8_UNORM:
//         case VK_FORMAT_R8G8B8_SNORM:
//         case VK_FORMAT_R8G8B8_UINT:
//         case VK_FORMAT_R8G8B8_SINT:
//         case VK_FORMAT_R8G8B8_SRGB:
//         case VK_FORMAT_B8G8R8_UNORM:
//         case VK_FORMAT_B8G8R8_SNORM:
//         case VK_FORMAT_B8G8R8_UINT:
//         case VK_FORMAT_B8G8R8_SINT:
//         case VK_FORMAT_B8G8R8_SRGB:
//             return 3; // 3 bytes per pixel

//         case VK_FORMAT_R8G8B8A8_UNORM:
//         case VK_FORMAT_R8G8B8A8_SNORM:
//         case VK_FORMAT_R8G8B8A8_UINT:
//         case VK_FORMAT_R8G8B8A8_SINT:
//         case VK_FORMAT_R8G8B8A8_SRGB:
//         case VK_FORMAT_B8G8R8A8_UNORM:
//         case VK_FORMAT_B8G8R8A8_SNORM:
//         case VK_FORMAT_B8G8R8A8_UINT:
//         case VK_FORMAT_B8G8R8A8_SINT:
//         case VK_FORMAT_B8G8R8A8_SRGB:
//             return 4; // 4 bytes per pixel

//         case VK_FORMAT_R16_UNORM:
//         case VK_FORMAT_R16_SNORM:
//         case VK_FORMAT_R16_UINT:
//         case VK_FORMAT_R16_SINT:
//         case VK_FORMAT_R16_SFLOAT:
//             return 2; // 2 bytes per pixel

//         case VK_FORMAT_R16G16_UNORM:
//         case VK_FORMAT_R16G16_SNORM:
//         case VK_FORMAT_R16G16_UINT:
//         case VK_FORMAT_R16G16_SINT:
//         case VK_FORMAT_R16G16_SFLOAT:
//             return 4; // 4 bytes per pixel

//         case VK_FORMAT_R16G16B16_UNORM:
//         case VK_FORMAT_R16G16B16_SNORM:
//         case VK_FORMAT_R16G16B16_UINT:
//         case VK_FORMAT_R16G16B16_SINT:
//         case VK_FORMAT_R16G16B16_SFLOAT:
//             return 6; // 6 bytes per pixel

//         case VK_FORMAT_R16G16B16A16_UNORM:
//         case VK_FORMAT_R16G16B16A16_SNORM:
//         case VK_FORMAT_R16G16B16A16_UINT:
//         case VK_FORMAT_R16G16B16A16_SINT:
//         case VK_FORMAT_R16G16B16A16_SFLOAT:
//             return 8; // 8 bytes per pixel

//         case VK_FORMAT_R32_SFLOAT:
//             return 4; // 4 bytes per pixel

//         case VK_FORMAT_R32G32_SFLOAT:
//             return 8; // 8 bytes per pixel

//         case VK_FORMAT_R32G32B32_SFLOAT:
//             return 12; // 12 bytes per pixel

//         case VK_FORMAT_R32G32B32A32_SFLOAT:
//             return 16; // 16 bytes per pixel

//         default:
//             return -1; // Unsupported format
//         }
//     }

//     TextureSystem::TextureSystem()
//     {
//         m_Buffer = MakeUnique<VyBuffer>();
//     }

//     Shared<TextureData> TextureSystem::loadTexture(const TString& _path, VkFormat _overrideFormat)
//     {
//         const U32 hash = Vy::Hash::hashString(_path.c_str());
            
//         auto it = m_TextureLookup.find(hash);
//         if (it != m_TextureLookup.end())
//         {
//             const I32 index = it->second;
//             return m_Textures[index];
//         }

//         auto texture = MakeShared<TextureData>();

//         // Step 1: Load texture data
//         I32 width, height, channels;
//         //U8* data = loadTextureData(_path, width, height, channels, STBI_rgb_alpha);
//         U8* data = stbi_load(_path.c_str(), &width, &height, &channels, 0);
//         if (!data)
//         {
//             throw std::runtime_error("Failed to load texture: " + _path);
//         }

//         // Step 2: Determine Vulkan format
//         VkFormat format = VK_FORMAT_R8G8B8A8_SRGB; // Default to SRGB 4 channels
//         if (_overrideFormat == VK_FORMAT_UNDEFINED)
//         {
//             if (channels == 1)
//             {
//                 format = VK_FORMAT_R8_UNORM; // Single channel (gray-scale)
//             }
//             else if (channels == 3)
//             {
//                 U8* rgbaData = new U8[width * height * 4];
//                 for (I32 i = 0; i < width * height; ++i) {
//                     rgbaData[i * 4 + 0] = data[i * 3 + 0];
//                     rgbaData[i * 4 + 1] = data[i * 3 + 1];
//                     rgbaData[i * 4 + 2] = data[i * 3 + 2];
//                     rgbaData[i * 4 + 3] = 255;
//                 }

//                 this->freeTextureData(data); // original RGB
//                 data = rgbaData;
//                 channels = 4;

//                 format = VK_FORMAT_R8G8B8A8_SRGB; // Convert RGB to RGBA
//             }
//         }
//         else
//         {
//             format = _overrideFormat;
//         }

//         const U32 mipLevels = static_cast<U32>(std::floor(std::log2(std::max(width, height)))) + 1;

//         // Step 3: Create Vulkan texture image with mipmaps
//         this->createTextureImage(data, width, height, format, texture->Image, texture->AllocationMemory, 1, mipLevels);

//         // Step 4: Create Vulkan image view
//         texture->ImageView = createImageView(texture->Image, format, 1, mipLevels);

//         // Step 5: Create Vulkan texture sampler
//         texture->Sampler = createTextureSampler(static_cast<float>(mipLevels));

//         // Free the loaded texture data
//         this->freeTextureData(data);

//         m_Textures.push_back(texture);

//         const I32 index = static_cast<I32>(m_Textures.size() - 1);
//         m_TextureLookup[hash] = index;

//         texture->Index = index;

//         return texture;
//     }


//     Shared<TextureData> TextureSystem::loadTexture(const TString& _name, VkFormat _format, const U8* _data, I32 _width, I32 _height)
//     {
//         const U32 hash = Vy::Hash::hashString(_name.c_str());

//         auto it = m_TextureLookup.find(hash);
//         if (it != m_TextureLookup.end())
//         {
//             const I32 index = it->second;
//             return m_Textures[index];
//         }

//         auto texture = MakeShared<TextureData>();

//         const U32 mipLevels = static_cast<U32>(std::floor(std::log2(std::max(_width, _height)))) + 1;

//         this->createTextureImage(_data, _width, _height, _format, texture->Image, texture->AllocationMemory, 1, mipLevels);
//         texture->ImageView = createImageView(texture->Image, _format, 1, mipLevels);
//         texture->Sampler = createTextureSampler(static_cast<float>(mipLevels));

//         m_Textures.push_back(texture);

//         const I32 index = static_cast<I32>(m_Textures.size() - 1);
//         m_TextureLookup[hash] = index;

//         texture->Index = index;

//         return texture;
//     }


//     Shared<TextureData> TextureSystem::loadTexture(const DefaultTextureType& _defaultTexture)
//     {
//         return loadTexture(_defaultTexture.Name, _defaultTexture.Format, _defaultTexture.Buffer.data(), _defaultTexture.Width, _defaultTexture.Height);
//     }


//     Shared<TextureData> TextureSystem::loadCubemap(const TArray<TString, 6>& _paths, VkFormat _overrideFormat)
//     {
//         TString hugePathNameForHash{""};
//         for (const TString& path : _paths)
//         {
//             hugePathNameForHash += path;
//         }

//         const U32 hash = Vy::Hash::hashString(hugePathNameForHash.c_str());

//         auto it = m_TextureLookup.find(hash);
//         if (it != m_TextureLookup.end())
//         {
//             const I32 index = it->second;
//             return m_Textures[index];
//         }

//         auto texture = MakeShared<TextureData>();

//         U32 totalFaceSize = 0;
//         U32 width = 0;
//         U32 height = 0;

//         TArray<U8*, 6> faceData;
//         TArray<I32, 6> faceWidths, faceHeights, faceChannels;
//         for (U32 i = 0; i < 6; ++i)
//         {
//             faceData[i] = loadTextureData(_paths[i], faceWidths[i], faceHeights[i], faceChannels[i], STBI_rgb_alpha);
//             if (!faceData[i])
//             {
//                 throw std::runtime_error("Failed to load texture for cubemap face: " + _paths[i]);
//             }

//             if (i == 0)
//             {
//                 width  = faceWidths[i];
//                 height = faceHeights[i];
//                 totalFaceSize = width * height * STBI_rgb_alpha; // Calculate size for one face
//             }
//             else if (width != faceWidths[i] || height != faceHeights[i])
//             {
//                 throw std::runtime_error("Cubemap faces have mismatched dimensions");
//             }
//         }

//         VkFormat format = VK_FORMAT_R8G8B8A8_SRGB; // Default to SRGB 4 channels
//         if (_overrideFormat == VK_FORMAT_UNDEFINED)
//         {
//             if (faceChannels[0] == 1)
//             {
//                 format = VK_FORMAT_R8_UNORM; // Single channel (gray-scale)
//             }
//             else if (faceChannels[0] == 3)
//             {
//                 format = VK_FORMAT_R8G8B8A8_SRGB; // Convert RGB to RGBA
//             }
//         }
//         else
//         {
//             format = _overrideFormat;
//         }

//         // Allocate a single continuous buffer
//         U8* continuousBuffer = new U8[totalFaceSize * 6];

//         // Copy all face data into the continuous buffer
//         for (U32 i = 0; i < 6; ++i)
//         {
//             std::memcpy(continuousBuffer + i * totalFaceSize, faceData[i], totalFaceSize);
//             this->freeTextureData(faceData[i]);
//             faceData[i] = continuousBuffer + i * totalFaceSize; // Optionally map pointers back to the single buffer
//         }

//         this->createTextureImage(continuousBuffer, width, height, format, texture->Image, texture->AllocationMemory, 6, 1, VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT);

//         texture->ImageView = createImageView(texture->Image, format, 6, 1);

//         texture->Sampler = createTextureSampler();

//         // free local buffer
//         delete[] continuousBuffer;

//         m_Textures.push_back(texture);

//         const I32 index = static_cast<I32>(m_Textures.size() - 1);
//         m_TextureLookup[hash] = index;

//         texture->Index = index;

//         return texture;
//     }


//     Shared<TextureData> TextureSystem::loadCubemap(const TString& _hdrPath, VkFormat _overrideFormat)
//     {
//         const U32 hash = Vy::Hash::hashString(_hdrPath.c_str());

//         auto it = m_TextureLookup.find(hash);
//         if (it != m_TextureLookup.end())
//         {
//             const I32 index = it->second;
//             return m_Textures[index];
//         }

//         auto texture = MakeShared<TextureData>();

//         // Load the HDR texture
//         I32 width, height, channels;
//         float* hdrData = stbi_loadf(_hdrPath.c_str(), &width, &height, &channels, STBI_rgb_alpha);
//         if (!hdrData)
//         {
//             throw std::runtime_error("Failed to load HDR file: " + _hdrPath);
//         }

//         VkFormat format = (_overrideFormat != VK_FORMAT_UNDEFINED) ? _overrideFormat : VK_FORMAT_R32G32B32A32_SFLOAT;
//         HDRProjectionType projection = HDRCubemapCPU::DetectHDRProjectionType(width, height);

//         U32 cubemapSize = 0;
//         switch (projection)
//         {
//         case HDRProjectionType::Cube:
//         {
//             bool vertical = (width / 3 == height / 4);
//             cubemapSize = vertical ? width / 3 : width / 4;
//             break;
//         }
//         case HDRProjectionType::Hemisphere:
//             cubemapSize = width / 2;
//             break;
//         case HDRProjectionType::Parabolic:
//         case HDRProjectionType::LatLongCubemap:
//         case HDRProjectionType::Equirectangular:
//         default:
//             cubemapSize = height / 2;
//             break;
//         }
//         cubemapSize = std::clamp(cubemapSize, 64u, 2048u);

//         TVector<U16>   cubemap16;
//         TVector<float> cubemap32;
        
//         if (format == VK_FORMAT_R16G16B16A16_SFLOAT)
//         {
//             cubemap16 = HDRCubemapCPU::generateFloat16Cubemap(hdrData, width, height, cubemapSize, projection);
//             this->createTextureImage(cubemap16.data(), cubemapSize, cubemapSize, format, texture->Image, texture->AllocationMemory, 6, 1, VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT);
//         }
//         else
//         {
//             cubemap32 = HDRCubemapCPU::generateFloat32Cubemap(hdrData, width, height, cubemapSize, projection);
//             this->createTextureImage(cubemap32.data(), cubemapSize, cubemapSize, format, texture->Image, texture->AllocationMemory, 6, 1, VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT);
//         }

//         texture->ImageView = createImageView(texture->Image, format, 6, 1);
//         texture->Sampler   = createTextureSampler();

//         stbi_image_free(hdrData);

//         m_Textures.push_back(texture);

//         const I32 index = static_cast<I32>(m_Textures.size() - 1);
//         m_TextureLookup[hash] = index;

//         texture->Index = index;

//         return texture;
//     }


//     Shared<TextureData> TextureSystem::generateBRDFLutTexture(const TString& _name, VkExtent2D _extent)
//     {
//         const U32 hash = Vy::Hash::hashString(_name.c_str());

//         auto it = m_TextureLookup.find(hash);
//         if (it != m_TextureLookup.end())
//         {
//             const I32 index = it->second;
//             return m_Textures[index];
//         }

//         Unique<OffscreenRenderer> offscreenRenderer = MakeUnique<OffscreenRenderer>(_extent, VK_FORMAT_R8G8B8A8_UNORM);
//         Unique<BRDFLUTGenerationSystem> m_brdfLutGenerationSystem = MakeUnique<BRDFLUTGenerationSystem>(offscreenRenderer->GetOffscreenSwapChainRenderPass());

//         auto cmdBuffer = offscreenRenderer->BeginFrame();
//         offscreenRenderer->BeginOffscreenSwapChainRenderPass(cmdBuffer);

//         m_brdfLutGenerationSystem->Generate(cmdBuffer, _extent.width, _extent.height);

//         offscreenRenderer->EndOffscreenSwapChainRenderPass(cmdBuffer);

//         BufferComponent stagingBuffer = offscreenRenderer->PrepareImageCopy(cmdBuffer);

//         offscreenRenderer->EndFrame();

//         TVector<U8> imageData = offscreenRenderer->FlushBufferToMemory(stagingBuffer);

//         return loadTexture(_name, VK_FORMAT_R8G8B8A8_UNORM, imageData.data(), _extent.width, _extent.height);
//     }


//     Shared<TextureData> TextureSystem::generateIrradianceCubemap(const TString& _name, U32 _faceSize, Shared<TextureData> _environment)
//     {
//         const U32 hash = Vy::Hash::hashString(_name.c_str());

//         auto it = m_TextureLookup.find(hash);
//         if (it != m_TextureLookup.end())
//         {
//             const I32 index = it->second;
//             return m_Textures[index];
//         }

//         VkExtent2D extent{ _faceSize, _faceSize };
//         Unique<OffscreenRenderer> offscreen = MakeUnique<OffscreenRenderer>(extent, VK_FORMAT_R8G8B8A8_UNORM);
//         Unique<IrradianceConvolutionGenerationSystem> system = MakeUnique<IrradianceConvolutionGenerationSystem>(offscreen->GetOffscreenSwapChainRenderPass());

//         VkDescriptorImageInfo envInfo{};
//         envInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
//         envInfo.imageView = _environment->ImageView;
//         envInfo.sampler = _environment->Sampler;

//         const Mat4 proj = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
//         const Vec3 dirs[6]{ {1,0,0},{-1,0,0},{0,1,0},{0,-1,0},{0,0,1},{0,0,-1} };
//         const Vec3 ups[6]{ {0,-1,0},{0,-1,0},{0,0,1},{0,0,-1},{0,-1,0},{0,-1,0} };

//         const U32 faceByteSize = _faceSize * _faceSize * 4;
//         TVector<U8> cubemapBuffer(faceByteSize * 6);

//         for (U32 face = 0; face < 6; ++face)
//         {
//             auto cmd = offscreen->BeginFrame();
//             offscreen->BeginOffscreenSwapChainRenderPass(cmd);

//             Mat4 view = glm::lookAt(Vec3(0.0f), dirs[face], ups[face]);
//             Mat4 vp = proj * view;
//             system->Generate(cmd, envInfo, vp);

//             offscreen->EndOffscreenSwapChainRenderPass(cmd);
//             BufferComponent staging = offscreen->PrepareImageCopy(cmd);
//             offscreen->EndFrame();

//             m_buffer->Map(staging);
//             std::memcpy(cubemapBuffer.data() + face * faceByteSize, staging.MappedMemory, faceByteSize);
//             m_buffer->Unmap(staging);
//             m_buffer->Destroy(staging);
//         }

//         auto texture = MakeShared<TextureData>();
//         this->createTextureImage(cubemapBuffer.data(), _faceSize, _faceSize, VK_FORMAT_R8G8B8A8_UNORM, texture->Image, texture->AllocationMemory, 6, 1, VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT);

//         texture->ImageView = createImageView(texture->Image, VK_FORMAT_R8G8B8A8_UNORM, 6, 1);
//         texture->Sampler = createTextureSampler();

//         m_Textures.push_back(texture);

//         const I32 index = static_cast<I32>(m_Textures.size() - 1);
//         m_TextureLookup[hash] = index;

//         texture->Index = index;

//         return texture;
//     }


//     Shared<TextureData> TextureSystem::generatePreFilteredEnvironmentMap(const TString& _name, U32 _faceSize, Shared<TextureData> _environment)
//     {
//         const U32 hash = Vy::Hash::hashString(_name.c_str());
//         auto it = m_TextureLookup.find(hash);
//         if (it != m_TextureLookup.end())
//         {
//             const I32 index = it->second;
//             return m_Textures[index];
//         }

//         const U32 mipLevels = static_cast<U32>(std::floor(std::log2(_faceSize))) + 1;

//         auto texture = MakeShared<TextureData>();
//         this->createTextureImage(nullptr, _faceSize, _faceSize, VK_FORMAT_R8G8B8A8_UNORM, texture->Image, texture->AllocationMemory, 6, mipLevels, VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT);
//         texture->ImageView = createImageView(texture->Image, VK_FORMAT_R8G8B8A8_UNORM, 6, mipLevels);
//         texture->Sampler   = createTextureSampler();

//         VkDescriptorImageInfo envInfo{};
//         envInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
//         envInfo.imageView = _environment->ImageView;
//         envInfo.sampler = _environment->Sampler;

//         const Mat4 proj = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
//         const Vec3 dirs[6]{ {1,0,0},{-1,0,0},{0,1,0},{0,-1,0},{0,0,1},{0,0,-1} };
//         const Vec3 ups[6]{ {0,-1,0},{0,-1,0},{0,0,1},{0,0,-1},{0,-1,0},{0,-1,0} };

//         VkCommandBuffer cmd = VyContext::beginCommands();
//         m_device.TransitionImageLayout(cmd, texture->Image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 0, 6, mipLevels);
//         VyContext::endCommands(cmd);

//         for (U32 mip = 0; mip < mipLevels; ++mip)
//         {
//             const U32 mipSize = std::max(1u, _faceSize >> mip);
//             Unique<OffscreenRenderer> offscreen = MakeUnique<OffscreenRenderer>(VkExtent2D{ mipSize, mipSize }, VK_FORMAT_R8G8B8A8_UNORM);
//             Unique<PreFilteredEnvironmentGenerationSystem> system = MakeUnique<PreFilteredEnvironmentGenerationSystem>(offscreen->GetOffscreenSwapChainRenderPass());

//             TVector<U8> cubemapData(mipSize * mipSize * 4 * 6);

//             float roughness = static_cast<float>(mip) / static_cast<float>(mipLevels - 1);

//             for (U32 face = 0; face < 6; ++face)
//             {
//                 auto cb = offscreen->BeginFrame();
//                 offscreen->BeginOffscreenSwapChainRenderPass(cb);

//                 Mat4 view = glm::lookAt(Vec3(0.0f), dirs[face], ups[face]);
//                 Mat4 vp = proj * view;
//                 system->Generate(cb, envInfo, vp, roughness);

//                 offscreen->EndOffscreenSwapChainRenderPass(cb);
//                 BufferComponent staging = offscreen->PrepareImageCopy(cb);
//                 offscreen->EndFrame();

//                 m_buffer->Map(staging);
//                 std::memcpy(cubemapData.data() + face * mipSize * mipSize * 4, staging.MappedMemory, mipSize * mipSize * 4);
//                 m_buffer->Unmap(staging);
//                 m_buffer->Destroy(staging);
//             }

//             BufferComponent upload = m_buffer->Create<BufferComponent>(cubemapData.size(), 1, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);
//             m_buffer->Map(upload);
//             m_buffer->WriteToBuffer(upload.MappedMemory, cubemapData.data(), cubemapData.size());
//             m_buffer->Unmap(upload);

//             VkCommandBuffer copyCmd = VyContext::beginCommands();
//             m_device.CopyBufferToImage(copyCmd, upload.Buffer, texture->Image, mipSize, mipSize, 6, mip + 1);
//             VyContext::endCommands(copyCmd);

//             m_buffer->Destroy(upload);
//         }

//         cmd = VyContext::beginCommands();
//         m_device.TransitionImageLayout(cmd, texture->Image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0, 6, mipLevels);
//         VyContext::endCommands(cmd);

//         m_Textures.push_back(texture);
//         const I32 index = static_cast<I32>(m_Textures.size() - 1);
//         m_TextureLookup[hash] = index;
//         texture->Index = index;

//         return texture;
//     }


//     U8* TextureSystem::loadTextureData(const TString& _path, I32& _width, I32& _height, I32& _channels, I32 _desired_channels)
//     {
//         U8* data = stbi_load(_path.c_str(), &_width, &_height, &_channels, _desired_channels);

//         if (!data)
//         {
//             throw std::runtime_error("Failed to load texture file: " + _path);
//         }

//         return data;
//     }


//     void TextureSystem::freeTextureData(U8* _data)
//     {
//         stbi_image_free(_data);
//     }


//     void TextureSystem::generateMipmaps(VkImage _image, VkFormat _format, I32 _width, I32 _height, U32 _mipLevels)
//     {
//         VkFormatProperties formatProperties;
//         vkGetPhysicalDeviceFormatProperties(m_device.GetPhysicalDevice(), _format, &formatProperties);

//         if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
//         {
//             throw std::runtime_error("Texture image format does not support linear blitting!");
//         }

//         VkCommandBuffer cmdBuffer = VyContext::beginCommands();

//         VkImageMemoryBarrier barrier{};
//         barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
//         barrier.image = _image;
//         barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
//         barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
//         barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//         barrier.subresourceRange.baseArrayLayer = 0;
//         barrier.subresourceRange.layerCount = 1;
//         barrier.subresourceRange.levelCount = 1;

//         I32 mipWidth = _width;
//         I32 mipHeight = _height;

//         for (U32 i = 1; i < _mipLevels; ++i)
//         {
//             barrier.subresourceRange.baseMipLevel = i - 1;
//             barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
//             barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
//             barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
//             barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

//             vkCmdPipelineBarrier(
//                 cmdBuffer,
//                 VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
//                 0,
//                 0, nullptr,
//                 0, nullptr,
//                 1, &barrier);

//             VkImageBlit blit{};
//             blit.srcOffsets[0] = { 0, 0, 0 };
//             blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
//             blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//             blit.srcSubresource.mipLevel = i - 1;
//             blit.srcSubresource.baseArrayLayer = 0;
//             blit.srcSubresource.layerCount = 1;
//             blit.dstOffsets[0] = { 0, 0, 0 };
//             blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
//             blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//             blit.dstSubresource.mipLevel = i;
//             blit.dstSubresource.baseArrayLayer = 0;
//             blit.dstSubresource.layerCount = 1;

//             vkCmdBlitImage(
//                 cmdBuffer,
//                 _image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
//                 _image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
//                 1, &blit,
//                 VK_FILTER_LINEAR);

//             barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
//             barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
//             barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
//             barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

//             vkCmdPipelineBarrier(
//                 cmdBuffer,
//                 VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
//                 0,
//                 0, nullptr,
//                 0, nullptr,
//                 1, &barrier);

//             if (mipWidth > 1) mipWidth /= 2;
//             if (mipHeight > 1) mipHeight /= 2;
//         }

//         barrier.subresourceRange.baseMipLevel = _mipLevels - 1;
//         barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
//         barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
//         barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
//         barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

//         vkCmdPipelineBarrier(
//             cmdBuffer,
//             VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
//             0,
//             0, nullptr,
//             0, nullptr,
//             1, &barrier);

//         VyContext::endCommands(cmdBuffer);
//     }


//     VkImageView TextureSystem::createImageView(VkImage _image, VkFormat _format, U32 _layerCount, U32 _mipLevels)
//     {
//         VkImageViewCreateInfo viewInfo{};
//         viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
//         viewInfo.image = _image;
//         viewInfo.viewType = [_layerCount]()
//             {
//                 if (_layerCount == 6)
//                 {
//                     return VK_IMAGE_VIEW_TYPE_CUBE;
//                 }
//                 else {
//                     return VK_IMAGE_VIEW_TYPE_2D;
//                 }
//             }();

//         viewInfo.format = _format;
//         viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
//         viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
//         viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
//         viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
//         viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//         viewInfo.subresourceRange.baseMipLevel = 0;
//         viewInfo.subresourceRange.levelCount = _mipLevels;
//         viewInfo.subresourceRange.baseArrayLayer = 0;
//         viewInfo.subresourceRange.layerCount = _layerCount;

//         VkImageView imageView;
//         if (vkCreateImageView(VyContext::device(), &viewInfo, nullptr, &imageView) != VK_SUCCESS)
//         {
//             throw std::runtime_error("Failed to create Vulkan image view");
//         }

//         return imageView;
//     }


//     VkSampler TextureSystem::createTextureSampler(float _maxLod)
//     {
//         VkSamplerCreateInfo samplerInfo{};
//         samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
//         samplerInfo.magFilter = VK_FILTER_LINEAR;
//         samplerInfo.minFilter = VK_FILTER_LINEAR;
//         samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;	// VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
//         samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;	// VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
//         samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;	// VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
//         samplerInfo.anisotropyEnable = VK_TRUE;
//         samplerInfo.maxAnisotropy = 16; // Adjust as per GPU capabilities
//         samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;	// VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
//         samplerInfo.unnormalizedCoordinates = VK_FALSE;
//         samplerInfo.compareEnable = VK_FALSE;
//         samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
//         samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
//         samplerInfo.minLod = 0.0f;
//         samplerInfo.maxLod = _maxLod;

//         VkSampler sampler;
//         if (vkCreateSampler(VyContext::device(), &samplerInfo, nullptr, &sampler) != VK_SUCCESS)
//         {
//             throw std::runtime_error("Failed to create texture sampler");
//         }

//         return sampler;
//     }

//     void TextureSystem::Cleanup()
//     {
//         for (const auto& texture : m_Textures)
//         {
//             vkDestroyImageView(VyContext::device(), texture->ImageView, nullptr);
//             vkDestroySampler(VyContext::device(), texture->Sampler, nullptr);
//             vmaDestroyImage(VyContext::allocator(), texture->Image, texture->AllocationMemory);
//         }

//         m_Textures.clear();
//         m_TextureLookup.clear();
//     }
// }