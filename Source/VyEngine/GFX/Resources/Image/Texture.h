#pragma once


#include <VyEngine/VK/Core/VKCore.h>
#include <VyEngine/VK/Buffer/Buffer.h>

#include <VyEngine/VK/Image/Image.h>
#include <VyEngine/VK/Image/ImageView.h>
#include <VyEngine/VK/Image/Sampler.h>


namespace Vy
{
    class VyTextureObject 
    {
    private:

        struct TextureInitInfo
        {
            VyImage            Image;
            VyImageView        View;
            VySampler          Sampler;
            U32                Width;
            U32                Height;
            U32                MipLevels;
            U32                ArrayLayers;
            VkImageCreateFlags Flags;
        };

    public:
        explicit VyTextureObject();
        
        VyTextureObject(const VyTextureObject&)     = delete;
        VyTextureObject(VyTextureObject&&) noexcept = delete;
        
        ~VyTextureObject();
        
        VyTextureObject& operator=(const VyTextureObject&) = delete;
        VyTextureObject& operator=(VyTextureObject&&) noexcept = delete;

        

        // When the texture type changes
        void recreateImageView(bool bIsCubemap, bool bIsHDR);

        void updateSampler(VkFilter minFilter, VkFilter magFilter, VkSamplerAddressMode addressMode);

        // Getters
        VY_NODISCARD       VkImage      imageHandle()           { return m_Image  .handle(); }
        VY_NODISCARD       VkImage      imageHandle()     const { return m_Image  .handle(); }
        VY_NODISCARD const VyImage&     image()           const { return m_Image; }
        
        VY_NODISCARD       VkImageView  imageViewHandle()       { return m_View   .handle(); }
        VY_NODISCARD       VkImageView  imageViewHandle() const { return m_View   .handle(); }
        VY_NODISCARD const VyImageView& imageView()       const { return m_View; }

        VY_NODISCARD       VkSampler    samplerHandle()         { return m_Sampler.handle(); }
        VY_NODISCARD       VkSampler    samplerHandle()   const { return m_Sampler.handle(); }
        VY_NODISCARD const VySampler&   sampler()         const { return m_Sampler; }

        VY_NODISCARD       VkFormat     format()          const { return m_Image.format(); }
        VY_NODISCARD       VkExtent3D   extent()          const { return m_Image.extent(); }
        VY_NODISCARD       VkExtent2D   extent2D()        const { return m_Image.extent2D(); }

        U32         mipLevels() const { return m_MipLevel; }
        bool        loaded()    const { return m_IsLoaded; }


        // Descriptor helpers for writes.
        VY_NODISCARD VkDescriptorImageInfo descriptorImageInfo() const
        {
            return VkDescriptorImageInfo{
                .sampler     = m_Sampler.handle(),
                .imageView   = m_View   .handle(),
                .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            };
        }

        VY_NODISCARD VkDescriptorImageInfo descriptorImageInfo(VkImageLayout layoutOverride) const
        {
            return VkDescriptorImageInfo{
                .sampler     = m_Sampler.handle(),
                .imageView   = m_View   .handle(),
                .imageLayout = layoutOverride,
            };
        }

        // Private constructor used only by TextureUploader
        // VyTextureObject(TextureInitInfo& info);
        VyTextureObject(
            VyImage&           image, 
            VyImageView&       view, 
            VySampler&         sampler,
            U32                Width,
            U32                Height,
            U32                MipLevels,
            U32                ArrayLayers,
            VkImageCreateFlags Flags
        );

    private:


        // Helpers called by recreateImageView()
        // VkImageView createImageView(VkImage image,
        //     VkFormat format,
        //     U32 mipmapLevel,
        //     VkImageAspectFlagBits aspect,
        //     bool bIsCube) const;

        // VkImageView createCubemapView(VkFormat format) const;

        // Internal Vulkan resource creators (used by uploader only)
        void destroy();

    private:

        VyImage            m_Image;
        VyImageView        m_View;
        VySampler          m_Sampler;

        VkExtent3D         m_Extent{ 0, 0, 1 };
        U32                m_MipLevel           = 1;
        U32                m_CreatedArrayLayers = 1;
        VkImageCreateFlags m_CreatedImageFlags  = 0;

        bool m_IsLoaded = false;

        friend class TextureUploader;
        friend class TextureBuilder;
    };
}




// namespace Vy
// {
//     class TextureObject 
//     {
//     private:

//         struct TextureInitInfo
//         {
//             VyImage            Image;
//             VmaAllocation      Allocation;
//             VyImageView        View;
//             VySampler          Sampler;
//             U32                Width;
//             U32                Height;
//             U32                MipLevels;
//             U32                ArrayLayers;
//             VkImageCreateFlags Flags;
//         };

//     public:
//         explicit TextureObject();
        
//         TextureObject(const TextureObject&)     = delete;
//         TextureObject(TextureObject&&) noexcept = delete;
        
//         ~TextureObject();
        
//         TextureObject& operator=(const TextureObject&) = delete;
//         TextureObject& operator=(TextureObject&&) noexcept = delete;

//         // When the texture type changes
//         void recreateImageView(bool bIsCubemap, bool bIsHDR);

//         void updateSampler(VkFilter minFilter, VkFilter magFilter, VkSamplerAddressMode addressMode);

//         // Getters
//         VY_NODISCARD VkImage      imageHandle()      const { return m_Image;   }
//         VY_NODISCARD VkImageView  imageViewHandle()  const { return m_View;    }
//         VY_NODISCARD VkSampler    samplerHandle()    const { return m_Sampler; }

//         VY_NODISCARD VkExtent2D   extent()           const { return m_Extent; }

//         VY_NODISCARD U32          mipLevels()        const { return m_MipLevel; }
//         VY_NODISCARD bool         loaded()           const { return m_IsLoaded; }


//         // Descriptor helpers for writes.
//         VY_NODISCARD VkDescriptorImageInfo descriptorImageInfo() const
//         {
//             return VkDescriptorImageInfo{
//                 .sampler     = m_Sampler,
//                 .imageView   = m_View,
//                 .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
//             };
//         }

//         VY_NODISCARD VkDescriptorImageInfo descriptorImageInfo(VkImageLayout layoutOverride) const
//         {
//             return VkDescriptorImageInfo{
//                 .sampler     = m_Sampler,
//                 .imageView   = m_View,
//                 .imageLayout = layoutOverride,
//             };
//         }

//     private:
        
//         // Private constructor used only by TextureUploader
//         TextureObject(const TextureInitInfo& info);


//         // Helpers called by recreateImageView()
//         VkImageView createImageView(
//             VkImage               image,
//             VkFormat              format,
//             U32                   mipmapLevel,
//             VkImageAspectFlagBits aspect,
//             bool                  bIsCube
//         ) const;

//         VkImageView createCubemapView(VkFormat format) const;

//         // Internal Vulkan resource creators (used by uploader only)
//         void destroy();

//     private:

//         VkImage       m_Image     { VK_NULL_HANDLE };
//         VmaAllocation m_Allocation{ VK_NULL_HANDLE };
//         VkImageView   m_View      { VK_NULL_HANDLE };
//         VkSampler     m_Sampler   { VK_NULL_HANDLE };

//         VkExtent2D         m_Extent{ 0, 0 };
//         U32                m_MipLevel           = 1;
//         U32                m_CreatedArrayLayers = 1;
//         VkImageCreateFlags m_CreatedImageFlags  = 0;

//         bool m_IsLoaded = false;

//         friend class TextureUploader;
//         friend class TextureBuilder;
//     };
// }