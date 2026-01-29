#include <VyEngine/GFX/Resources/Image/Texture.h>
#include <VyEngine/VK/Context.h>

#include <VyEngine/Globals.h>

namespace Vy
{
    VyTextureObject::VyTextureObject()
    {
    }


    VyTextureObject::~VyTextureObject() 
    {
        destroy();
    }

    VyTextureObject::VyTextureObject(
        VyImage&           image, 
        VyImageView&       view, 
        VySampler&         sampler,
        U32                width,
        U32                height,
        U32                mipLevels,
        U32                arrayLayers,
        VkImageCreateFlags flags
    ) :
        m_Image             { std::move(image)   },
        m_View              { std::move(view)    },
        m_Sampler           { std::move(sampler) },
        m_Extent            { width, height, 1   },
        m_MipLevel          { mipLevels          },
        m_CreatedArrayLayers{ arrayLayers        },
        m_CreatedImageFlags { flags              },
        m_IsLoaded          { true               }
    {

    }

    // Manually constructed by TextureUploader
    // VyTextureObject::VyTextureObject(TextureInitInfo& info) : 
    //     m_Image( std::move(info.Image) ),
    //     m_View( std::move(info.View) ),
    //     m_Sampler( std::move(info.Sampler) ),
    //     m_Extent{ info.Width, info.Height },
    //     m_MipLevel(info.MipLevels),
    //     m_CreatedArrayLayers(info.ArrayLayers),
    //     m_CreatedImageFlags(info.Flags),
    //     m_IsLoaded(true) 
    // {
    // }


    //// Destructor helper ////

    void VyTextureObject::destroy() 
    {
        if (!m_IsLoaded)
            return;

        // if (m_Sampler) {
        //     vkDestroySampler(logicalDevice, m_Sampler, nullptr);
        //     m_Sampler = VK_NULL_HANDLE;
        // }

        // if (m_View) {
        //     vkDestroyImageView(logicalDevice, m_View, nullptr);
        //     m_View = VK_NULL_HANDLE;
        // }

        // if (m_Image) {
        //     vkDestroyImage(logicalDevice, m_Image, nullptr);
        //     m_Image = VK_NULL_HANDLE;
        // }

        // if (m_Allocation) {
        //     vkFreeMemory(logicalDevice, m_Allocation, nullptr);
        //     m_Allocation = VK_NULL_HANDLE;
        // }

        m_IsLoaded = false;
    }


    // Recreate view (used when switching between type of views)
    void VyTextureObject::recreateImageView(bool bIsCubemap, bool bIsHDR) {
        // assert(m_Image != VK_NULL_HANDLE);

        // // Destroy old view
        // if (m_View) {
        //     vkDestroyImageView(VyContext::device(), m_View, nullptr);
        //     m_View = VK_NULL_HANDLE;
        // }

        // VkFormat format = bIsHDR ?
        //     VK_FORMAT_R16G16B16A16_SFLOAT :
        //     VK_FORMAT_R8G8B8A8_UNORM;

        // if (bIsCubemap) {
        //     m_View = createCubemapView(format);
        // }
        // else {
        //     m_View = createImageView(
        //         m_Image,
        //         format,
        //         m_MipLevel,
        //         VK_IMAGE_ASPECT_COLOR_BIT,
        //         false
        //     );
        // }
    }

    void VyTextureObject::updateSampler(VkFilter minFilter, VkFilter magFilter, VkSamplerAddressMode addressMode)
    {
        // if (m_Sampler) {
        //     vkDestroySampler(VyContext::device(), m_Sampler, nullptr);
        //     m_Sampler = VK_NULL_HANDLE;
        // }

        // VkSamplerCreateInfo samplerInfo{};
        // samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        // samplerInfo.magFilter = magFilter;
        // samplerInfo.minFilter = minFilter;
        // samplerInfo.addressModeU = addressMode;
        // samplerInfo.addressModeV = addressMode;
        // samplerInfo.addressModeW = addressMode;
        // samplerInfo.anisotropyEnable = VK_TRUE;
        // samplerInfo.maxAnisotropy = device.properties.limits.maxSamplerAnisotropy;
        // samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        // samplerInfo.unnormalizedCoordinates = VK_FALSE;
        // samplerInfo.compareEnable = VK_FALSE;
        // samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

        // samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        // samplerInfo.mipLodBias = 0.0f;
        // samplerInfo.minLod = 0.0f;
        // samplerInfo.maxLod = m_MipLevel;

        // if (vkCreateSampler(VyContext::device(), &samplerInfo, nullptr, &m_Sampler) != VK_SUCCESS) {
        //     throw std::runtime_error("failed to create texture sampler!");
        // }
    }

    //// Internal helpers ////

    // VkImageView VyTextureObject::createImageView(
    //     VkImage image,
    //     VkFormat format,
    //     U32 mipmapLevel,
    //     VkImageAspectFlagBits aspect,
    //     bool bIsCube
    // ) const
    // {
    //     VkImageViewCreateInfo viewInfo{};
    //     viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    //     viewInfo.image = image;
    //     viewInfo.viewType = bIsCube ? VK_IMAGE_VIEW_TYPE_CUBE : VK_IMAGE_VIEW_TYPE_2D;
    //     viewInfo.format = format;

    //     viewInfo.subresourceRange.aspectMask = aspect;
    //     viewInfo.subresourceRange.baseMipLevel = 0;
    //     viewInfo.subresourceRange.levelCount = mipmapLevel;
    //     viewInfo.subresourceRange.baseArrayLayer = 0;
    //     viewInfo.subresourceRange.layerCount = bIsCube ? 6u : 1u;

    //     VkImageView imageView = VK_NULL_HANDLE;
    //     if (vkCreateImageView(VyContext::device(), &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
    //         throw std::runtime_error("VyTextureObject: Failed to create image view");
    //     }

    //     return imageView;
    // }


    // VkImageView VyTextureObject::createCubemapView(VkFormat format) const {
    //     return createImageView(
    //         m_Image,
    //         format,
    //         m_MipLevel,
    //         VK_IMAGE_ASPECT_COLOR_BIT,
    //         true // bIsCube
    //     );
    // }
}




// namespace Vy
// {
//     TextureObject::TextureObject()
//     {
//     }


//     TextureObject::~TextureObject() 
//     {
//         destroy();
//     }


//     // Manually constructed by TextureUploader
//     TextureObject::TextureObject(const TextureInitInfo& info) : 
//         m_Image             (info.Image),
//         m_Allocation        (info.Allocation),
//         m_View              (info.View),
//         m_Sampler           (info.Sampler),
//         m_Extent            { info.Width, info.Height },
//         m_MipLevel          (info.MipLevels),
//         m_CreatedArrayLayers(info.ArrayLayers),
//         m_CreatedImageFlags (info.Flags),
//         m_IsLoaded          (true) 
//     {
//     }


//     //// Destructor helper ////

//     void TextureObject::destroy() 
//     {
//         if (!m_IsLoaded)
//             return;

//         if (m_Sampler) 
//         {
//             VyContext::destroy(m_Sampler);
//             m_Sampler = VK_NULL_HANDLE;
//         }

//         if (m_View) 
//         {
//             VyContext::destroy(m_View);
//             m_View = VK_NULL_HANDLE;
//         }

//         if (m_Image) 
//         {
//             VyContext::destroy(m_Image, m_Allocation);
//             m_Image      = VK_NULL_HANDLE;
//             m_Allocation = VK_NULL_HANDLE;
//         }

//         m_IsLoaded = false;
//     }




//     // Recreate view (used when switching between type of views)
//     void TextureObject::recreateImageView(bool bIsCubemap, bool bIsHDR) 
//     {
//         assert(m_Image != VK_NULL_HANDLE);

//         // Destroy old view
//         if (m_View) 
//         {
//             VyContext::destroy(m_View);

//             m_View = VK_NULL_HANDLE;
//         }

//         VkFormat format = bIsHDR 
//             ? VK_FORMAT_R16G16B16A16_SFLOAT 
//             : VK_FORMAT_R8G8B8A8_UNORM;

//         if (bIsCubemap) 
//         {
//             m_View = createCubemapView(format);
//         }
//         else 
//         {
//             m_View = createImageView(
//                 m_Image,
//                 format,
//                 m_MipLevel,
//                 VK_IMAGE_ASPECT_COLOR_BIT,
//                 false
//             );
//         }
//     }

//     void TextureObject::updateSampler(VkFilter minFilter, VkFilter magFilter, VkSamplerAddressMode addressMode)
//     {
//         if (m_Sampler) 
//         {
//             VyContext::destroy(m_Sampler);
//             m_Sampler = VK_NULL_HANDLE;
//         }

//         VkSamplerCreateInfo samplerInfo{ VKInit::samplerCreateInfo() };
//         {
//             samplerInfo.magFilter               = magFilter;
//             samplerInfo.minFilter               = minFilter;
//             samplerInfo.addressModeU            = addressMode;
//             samplerInfo.addressModeV            = addressMode;
//             samplerInfo.addressModeW            = addressMode;
//             samplerInfo.anisotropyEnable        = VK_TRUE;
//             samplerInfo.maxAnisotropy           = VyContext::device().limits().maxSamplerAnisotropy;
//             samplerInfo.borderColor             = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
//             samplerInfo.unnormalizedCoordinates = VK_FALSE;
//             samplerInfo.compareEnable           = VK_FALSE;
//             samplerInfo.compareOp               = VK_COMPARE_OP_ALWAYS;
            
//             samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
//             samplerInfo.mipLodBias = 0.0f;
//             samplerInfo.minLod    = 0.0f;
//             samplerInfo.maxLod    = m_MipLevel;
//         }

//         if (vkCreateSampler(VyContext::device(), &samplerInfo, nullptr, &m_Sampler) != VK_SUCCESS) 
//         {
//             throw std::runtime_error("TextureObject: Failed to create texture sampler!");
//         }
//     }

//     //// Internal helpers ////

//     VkImageView TextureObject::createImageView(
//         VkImage               image,
//         VkFormat              format,
//         U32                   mipmapLevel,
//         VkImageAspectFlagBits aspect,
//         bool                  bIsCube
//     ) const
//     {
//         VkImageViewCreateInfo viewInfo{ VKInit::imageViewCreateInfo() };
//         {
//             viewInfo.image    = image;
//             viewInfo.viewType = bIsCube ? VK_IMAGE_VIEW_TYPE_CUBE : VK_IMAGE_VIEW_TYPE_2D;
//             viewInfo.format   = format;
            
//             viewInfo.subresourceRange.aspectMask     = aspect;
//             viewInfo.subresourceRange.baseMipLevel   = 0;
//             viewInfo.subresourceRange.levelCount     = mipmapLevel;
//             viewInfo.subresourceRange.baseArrayLayer = 0;
//             viewInfo.subresourceRange.layerCount     = bIsCube ? 6u : 1u;
//         }

//         VkImageView imageView = VK_NULL_HANDLE;
//         if (vkCreateImageView(VyContext::device(), &viewInfo, nullptr, &imageView) != VK_SUCCESS) 
//         {
//             throw std::runtime_error("TextureObject: Failed to create image view");
//         }

//         return imageView;
//     }


//     VkImageView TextureObject::createCubemapView(VkFormat format) const 
//     {
//         return createImageView(
//             m_Image,
//             format,
//             m_MipLevel,
//             VK_IMAGE_ASPECT_COLOR_BIT,
//             true // bIsCube
//         );
//     }
// }