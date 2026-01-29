#pragma once

#include <VyEngine/GFX/Resources/Image/Texture.h>

namespace Vy
{
    struct VyDecodedImage;
    struct VyDecodedCubemap;

    class TextureUploader 
    {
    public:
        // Upload a standard 2D image (8-bit or float)
        static Unique<VyTextureObject> upload2D(const VyDecodedImage& img, bool bUseMipmap,bool bSRGB);

        // Upload a cubemap (6 faces)
        static Unique<VyTextureObject> uploadCubemap(const VyDecodedCubemap& cubeMap);

        static Unique<VyTextureObject> uploadCompressed2D(const VyDecodedImage& imageData);

    private:
        static constexpr U32 CUBEMAP_FACE_COUNT = 6;
    
        /// <summary>
        /// Creates a VkImage and allocates its device memory 
        /// </summary>
        // static VkImage createImage(
        //     U32 width, 
        //     U32 height, 
        //     VkFormat format, 
        //     VkImageTiling tiling, 
        //     VkImageUsageFlags usage, 
        //     VkMemoryPropertyFlags properties, 
        //     VmaAllocation& imageMemory, 
        //     U32 arrayLayer, 
        //     VkImageCreateFlags flags, 
        //     VkImageType imageType, 
        //     U32 mipLevels
        // );

        // /// <summary>
        // /// Creates a VkImageView for the specified Vulkan image
        // /// </summary>
        // /// <returns> VkImageView handle representing the created image view for the given image and parameters. The caller is responsible for destroying the view when no longer needed</returns>
        // static VkImageView createImageView(
        //     VkImage image, 
        //     VkFormat format, 
        //     U32 mipmapLevel, 
        //     VkImageAspectFlagBits aspectFlag, 
        //     VkImageViewType viewType, 
        //     U32 layerCount
        // );

        // /// <summary>
        // /// Creates and returns VkSampler configured for the given device and number of mipmap levels
        // /// </summary>
        // static VkSampler createSampler(
        //     U32 mipLevels
        // );

        /// <summary>
        /// Generates mipmaps for a Vulkan image by recording a single-time command buffer that blits from higher-resolution mip levels to lower ones and transitions image layouts as required
        /// </summary>
        static void generateMipmaps(
            VkImage image, 
            int texWidth, 
            int texHeight, 
            U32 mipLevels, 
            U32 layers
        );
    };
}