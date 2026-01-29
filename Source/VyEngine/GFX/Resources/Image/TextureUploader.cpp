#include <VyEngine/GFX/Resources/Image/TextureUploader.h>

#include <VyEngine/GFX/Resources/Image/Texture.h>
#include <VyEngine/GFX/Resources/Image/Decoder/ImageDecoder.h>

#include <VyEngine/VK/Context.h>
#include <VyLib/Util/String.h>
namespace Vy
{
    namespace
    {
        /// <summary>
        /// Calculates the number of mipmap levels required for a texture of the given dimensions
        /// </summary>
        U32 calculateMipLevels(int width, int height) 
        {
            return static_cast<U32>(std::floor(std::log2(std::max(width, height)))) + 1;
        }


        /// <summary>
        /// Selects a Vulkan VkFormat for a texture based on if it uses float components and if it should be sRGB
        /// </summary>
        VkFormat selectFormat(bool bIsFloat, bool bSRGB)
        {
            if (bIsFloat) return VK_FORMAT_R32G32B32A32_SFLOAT;

            return bSRGB ? VK_FORMAT_R8G8B8A8_SRGB : VK_FORMAT_R8G8B8A8_UNORM;
        }
        

        /// <summary>
        /// Compute the size in bytes of the decoded image pixel data
        /// </summary>
        /// <returns>number of bytes required to hold the image pixel data</returns>
        size_t calculatePixelSize(const VyDecodedImage& img)
        {
            return img.IsFloat
                ? img.Pixels32.size() * sizeof(float)
                : img.Pixels8.size();
        }

        /// <summary>
        /// Calculates the total pixel data size in bytes for a cubemap made of six faces
        /// </summary>
        /// <param name="faces">An array of six VyDecodedImage objects representing the cubemap faces</param>
        /// <returns>The total size in bytes of the pixel data for all six faces</returns>
        size_t calculateCubemapPixelSize(const TArray<VyDecodedImage, 6>& faces)
        {
            const size_t faceSize = faces[0].CompressedData.size();

            // Ensure all faces match.
            for (int i = 1; i < 6; ++i) 
            {
                if (faces[i].CompressedData.size() != faceSize) 
                {
                    VY_THROW_RUNTIME_ERROR("calculateCubemapPixelSize - cubemap faces have mismatched data sizes");
                }
            }

            return faceSize * 6;
        }


        /// <summary>
        /// Validates that the six cubemap faces have identical dimensions and the same pixel format
        /// </summary>
        /// <param name="faces">Array of six VyDecodedImage objects representing the cubemap faces</param>
        void validateCubemapFaces(const TArray<VyDecodedImage, 6>& faces)
        {
            const int  w        = faces[0].Width;
            const int  h        = faces[0].Height;
            const bool bIsFloat = faces[0].IsFloat;

            for (int i = 1; i < 6; i++) 
            {
                if (faces[i].Width  != w ||
                    faces[i].Height != h)
                {
                    VY_THROW_RUNTIME_ERROR("All cubemap faces must have identical dimensions");
                }

                if (faces[i].IsFloat != bIsFloat) 
                {
                    VY_THROW_RUNTIME_ERROR("All cubemap faces must have the same pixel format");
                }
            }
        }


        /// <summary>
        /// validates that the six cubemap faces have valid compressed data
        /// </summary>
        void validateCubemapFacesData(const TArray<VyDecodedImage, 6>& faces)
        {
            const size_t faceSize = faces[0].CompressedData.size();

            for (U32 i = 0; i < 6; i++) 
            {
                if (!faces[i].IsCompressed) 
                {
                    VY_THROW_RUNTIME_ERROR("copyCubemapToMemory: face is not compressed");
                }
                
                if (faces[i].CompressedData.empty()) 
                {
                    VY_THROW_RUNTIME_ERROR("copyCubemapToMemory: CompressedData is empty");
                }
                
                if (faces[i].CompressedData.size() != faceSize) 
                {
                    VY_THROW_RUNTIME_ERROR("copyCubemapToMemory: inconsistent face sizes");
                }
            }
        }


        /// <summary>
        /// Validates a decoded image for correctness, format, and integrity
        /// </summary>
        /// <param name="img">The decoded image to validate</param>
        void validateImage(const VyDecodedImage& img)
        {
            if (img.Width <= 0 || img.Height <= 0)
            {
                VY_THROW_RUNTIME_ERROR("Image dimensions must be positive");
            }

            if (!img.IsFloat && img.Pixels8.empty())
            {
                VY_THROW_RUNTIME_ERROR("Pixels8 data missing for 8-bit image");
            }

            if (img.IsFloat && img.Pixels32.empty())
            {
                VY_THROW_RUNTIME_ERROR("Pixels32 data missing for float image");
            }
        }

        /// <summary>
        /// Validates a decoded image for correctness, format, and integrity
        /// </summary>
        /// <param name="img">The decoded image to validate</param>
        void validateCompressedImage(const VyDecodedImage& img)
        {
            if (!img.IsCompressed) 
            {
                VY_THROW_RUNTIME_ERROR("uploadCompressed2D: VyDecodedImage is not compressed");
            }

            if (img.MipLevels == 0) 
            {
                VY_THROW_RUNTIME_ERROR("uploadCompressed2D: Invalid mip level count");
            }
        }



        /// <summary>
        /// Copies six cubemap face pixel data into staging memory
        /// </summary>
        /// <param name="faces">array of six VyDecodedImage objects representing the cubemap faces</param>
        // void copyCubemapToMemory(
        //     VmaAllocation stagingMemory,
        //     const TArray<VyDecodedImage, 6>& faces)
        // {

        //     uint8_t* dst = nullptr;
        //     vkMapMemory(VyContext::device(), stagingMemory, 0, VK_WHOLE_SIZE, 0, (void**)&dst);

        //     size_t faceSize = calculateCubemapPixelSize({ faces }) / 6;

        //     for (U32 i = 0; i < 6; i++) {
        //         const uint8_t* src = faces[i].CompressedData.data();
        //         std::memcpy(dst + i * faceSize, src, faceSize);
        //     }

        //     vkUnmapMemory(VyContext::device(), stagingMemory);
        // }

        /// <summary>
        /// copies provided data into mapped Vulkan device memory
        /// </summary>
        /// <param name="device">Reference to the Device wrapper used to obtain the VkDevice for vkMapMemory/vkUnmapMemory operations</param>
        /// <param name="memory">VkDeviceMemory handle that identifies the device memory to map and write to</param>
        /// <param name="size">Number of bytes to copy into the mapped memory (the mapping range length)</param>
        /// <param name="data">Pointer to the source buffer containing at least size bytes to be copied into device memory</param>
        // void copyToMemory(VkDeviceMemory memory, size_t size, const void* data) {
        //     void* mapped = nullptr;
        //     vkMapMemory(VyContext::device(), memory, 0, size, 0, &mapped);
        //     std::memcpy(mapped, data, size);
        //     vkUnmapMemory(VyContext::device(), memory);
        // }

    }

    /// <summary>
    /// Creates a VkImage and allocates its device memory 
    /// </summary>
    // VkImage TextureUploader::createImage(
    //     U32 width, 
    //     U32 height,
    //     VkFormat format, 
    //     VkImageTiling tiling,
    //     VkImageUsageFlags usage, 
    //     VkMemoryPropertyFlags properties,
    //     VkDeviceMemory& imageMemory,
    //     U32 arrayLayer, 
    //     VkImageCreateFlags flags, 
    //     VkImageType imageType, 
    //     U32 mipLevels)
    // {
    //     assert(arrayLayer > 0 && "array layer cannot be zero");


    //     VkImageCreateInfo imageInfo = VKImageInfoBuilder{}

    //     .getInfo();
    //     imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    //     imageInfo.imageType = imageType;

    //     imageInfo.mipLevels = mipLevels;
    //     imageInfo.arrayLayers = arrayLayer;
    //     imageInfo.format = format;
    //     imageInfo.tiling = tiling;
    //     imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    //     imageInfo.usage = usage;
    //     imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    //     imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    //     imageInfo.extent.width = width;
    //     imageInfo.extent.height = height;
    //     imageInfo.extent.depth = 1;

    //     imageInfo.flags = flags;

    //     VkImage image;
    //     device.createImageWithInfo(imageInfo, properties, image, imageMemory);

    //     return image;
    // }

    // / <summary>
    // / Creates a VkImageView for the specified Vulkan image
    // / </summary>
    // / <returns> VkImageView handle representing the created image view for the given image and parameters. The caller is responsible for destroying the view when no longer needed</returns>
    // VkImageView TextureUploader::createImageView(
    //     VkImage image, VkFormat format, 
    //     U32 mipmapLevel, 
    //     VkImageAspectFlagBits aspectFlag, 
    //     VkImageViewType viewType, 
    //     U32 layerCount
    // )
    // {
    //     assert(mipmapLevel > 0 && "miplevel cannot be zero");

    //     VkImageViewCreateInfo viewInfo{};
    //     viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    //     viewInfo.image = image;
    //     viewInfo.viewType = viewType;
    //     viewInfo.format = format;
    //     viewInfo.subresourceRange.aspectMask = aspectFlag;
    //     viewInfo.subresourceRange.baseMipLevel = 0;
    //     viewInfo.subresourceRange.levelCount = mipmapLevel;
    //     viewInfo.subresourceRange.baseArrayLayer = 0;
    //     viewInfo.subresourceRange.layerCount = layerCount;

    //     VkImageView imageView;
    //     if (vkCreateImageView(VyContext::device(), &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
    //         VY_THROW_RUNTIME_ERROR("failed to create texture image view!");
    //     }

    //     return imageView;
    // }

    /// <summary>
    /// Creates and returns a standard VkSampler configured for the given device and number of mipmap levels
    /// </summary>
    // VkSampler TextureUploader::createSampler(U32 mipLevels) 
    // {
    //     VkSamplerCreateInfo info{};
    //     info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    //     info.magFilter = VK_FILTER_LINEAR;
    //     info.minFilter = VK_FILTER_LINEAR;
    //     info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    //     info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    //     info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    //     info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    //     info.minLod = 0.0f;
    //     info.maxLod = static_cast<float>(mipLevels);
    //     info.maxAnisotropy = VyContext::device().limits().maxSamplerAnisotropy;
    //     info.anisotropyEnable = VK_TRUE;

    //     VkSampler sampler;
    //     if (vkCreateSampler(VyContext::device(), &info, nullptr, &sampler) != VK_SUCCESS)
    //         VY_THROW_RUNTIME_ERROR("Failed to create sampler");

    //     return sampler;
    // }

    //// MipMap generation ////

    /// <summary>
    /// Generates mipmaps for a Vulkan image by recording a single-time command buffer that blits from higher-resolution mip levels to lower ones and transitions image layouts as required
    /// </summary>
    /// <param name="device">Reference to the Device used to begin and end the single-time command buffer and to submit the recorded commands</param>
    /// <param name="image">VkImage to generate mipmaps for</param>
    /// <param name="mipLevels">number of mipmap levels to generate</param>
    /// <param name="layers">Number of array layers in the image (defaults to 1)</param>
    void TextureUploader::generateMipmaps(
        VkImage image,
        int     texWidth,
        int     texHeight,
        U32     mipLevels,
        U32     layers = 1)
    {
        VkCommandBuffer cmd = VyContext::device().beginSingleTimeCommands();

        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.image = image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = layers;
        barrier.subresourceRange.levelCount = 1;

        int mipW = texWidth;
        int mipH = texHeight;

        for (U32 i = 1; i < mipLevels; i++) 
        {
            barrier.subresourceRange.baseMipLevel = i - 1;
            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

            vkCmdPipelineBarrier(
                cmd,
                VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                0,
                0, nullptr,
                0, nullptr,
                1, &barrier
            );

            VkImageBlit blit{};
            blit.srcOffsets[0] = { 0, 0, 0 };
            blit.srcOffsets[1] = { mipW, mipH, 1 };
            blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            blit.srcSubresource.mipLevel = i - 1;
            blit.srcSubresource.baseArrayLayer = 0;
            blit.srcSubresource.layerCount = layers;

            blit.dstOffsets[0] = { 0, 0, 0 };
            blit.dstOffsets[1] = { std::max(1, mipW / 2), std::max(1, mipH / 2), 1 };
            blit.dstSubresource = blit.srcSubresource;
            blit.dstSubresource.mipLevel = i;

            vkCmdBlitImage(
                cmd,
                image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                1, &blit,
                VK_FILTER_LINEAR
            );

            // Previous mip now shader readable
            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            vkCmdPipelineBarrier(
                cmd,
                VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                0,
                0, nullptr,
                0, nullptr,
                1, &barrier
            );

            mipW = std::max(1, mipW / 2);
            mipH = std::max(1, mipH / 2);
        }

        // Transition last mip level
        barrier.subresourceRange.baseMipLevel = mipLevels - 1;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(
            cmd,
            VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier
        );

        VyContext::device().endSingleTimeCommands(cmd);
    }

    //// Texture Uploading ////

    Unique<VyTextureObject> TextureUploader::upload2D(
        const VyDecodedImage& img,
        bool                  bUseMipmap,
        bool                  bSRGB)
    {
        const bool     IsFloat   = img.IsFloat;
        const U32      w         = img.Width;
        const U32      h         = img.Height;
        const U32      mipLevels = bUseMipmap ? calculateMipLevels(w, h):1;
        const VkFormat format    = selectFormat(IsFloat, bSRGB);
        const size_t   pixelSize = calculatePixelSize(img);

        // staging buffer
        VyBuffer stagingBuffer{ VyBuffer::stagingBuffer("tex_upload_2d", pixelSize) };

        const void* srcPixels = IsFloat 
            ? (void*)img.Pixels32.data() 
            : (void*)img.Pixels8 .data();

        stagingBuffer.write(srcPixels);

        // GPU image
        VyImage image = VyImage::Builder{}
			.setName       ("tex_upload_2d")
            .setImageType  (VK_IMAGE_TYPE_2D)
            .setFormat     (format)
            .setExtent     (w, h)
            .setLevels     (mipLevels)
			.setLayers     (1)
			.setSamples    (VK_SAMPLE_COUNT_1_BIT)
            .setTiling     (VK_IMAGE_TILING_OPTIMAL)
			.setLayout     (VK_IMAGE_LAYOUT_UNDEFINED)
            .setUsage      (VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
            .setMemoryUsage(VMA_MEMORY_USAGE_AUTO)
        	.build();

		// Transition image layout and copy buffer to image.
		image.copyFrom(stagingBuffer, false /*toShaderReadOnly*/);

        if (bUseMipmap) 
        {
            // Generate mipmaps (this also transitions to SHADER_READ_ONLY_OPTIMAL)
            image.generateMipmaps(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        }
        else
        {
            image.transitionLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        }

        // create view and sampler
        VyImageView view  = VyImageView::Builder{}
			.setName    ("tex_upload_2d")
            .setViewType(VK_IMAGE_VIEW_TYPE_2D)
            .setFormat  (format)
            .setAspect  (VK_IMAGE_ASPECT_COLOR_BIT)
            .setLevels  (0, mipLevels)
            .setLayers  (0, 1)
        	.build( image );


        VySampler sampler = VySampler::Builder{}
            .setName         ("tex_upload_2d")
            .setFilters      (VK_FILTER_LINEAR)
            .setMipmapMode   (VK_SAMPLER_MIPMAP_MODE_LINEAR)
            .setWrap         (VK_SAMPLER_ADDRESS_MODE_REPEAT)
            .setBorder       (VK_BORDER_COLOR_INT_OPAQUE_BLACK)
            .enableAnisotropy(true)
            .setLodRange     (0.0f, static_cast<float>(mipLevels))
            .setMipLodBias   (0.0f)
            .build();

        return MakeUnique<VyTextureObject>(
            image,
            view,
            sampler,
            w, 
            h,
            mipLevels,
            1,
            0
        );
    }


    Unique<VyTextureObject> TextureUploader::uploadCubemap(const VyDecodedCubemap& cubeMap)
    {
        validateCubemapFaces(cubeMap.Faces);
        validateCubemapFacesData(cubeMap.Faces);

        const int      width     = static_cast<U32>(cubeMap.Faces[0].Width);
        const int      height    = static_cast<U32>(cubeMap.Faces[0].Height);
        const bool     IsFloat   = cubeMap.Faces[0].IsFloat;
        const U32      mipLevels = cubeMap.Faces[0].MipLevels;
        const VkFormat format    = cubeMap.Faces[0].Format;
        const size_t   totalSize = calculateCubemapPixelSize(cubeMap.Faces);

        // staging buffer
        VyBuffer stagingBuffer{ VyBuffer::stagingBuffer("tex_upload_cubemap", totalSize) };

        // Copy all 6 images into one contiguous block
        size_t layerSize     = calculateCubemapPixelSize({ cubeMap.Faces }) / 6;
        size_t currentOffset = 0;
        for (U32 i = 0; i < 6; i++) 
        {
            stagingBuffer.singleWrite(cubeMap.Faces[i].CompressedData.data(), layerSize, currentOffset);

            currentOffset += layerSize;
        }

        VyImage image = VyImage::Builder{}
			.setName       ("tex_upload_cubemap")
            .setImageType  (VK_IMAGE_TYPE_2D)
            .setFormat     (format)
            .setExtent     (width, height)
            .setLevels     (mipLevels)
			.setLayers     (CUBEMAP_FACE_COUNT)
			.setSamples    (VK_SAMPLE_COUNT_1_BIT)
            .setTiling     (VK_IMAGE_TILING_OPTIMAL)
			.setLayout     (VK_IMAGE_LAYOUT_UNDEFINED)
            .setUsage      (VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
            .setFlags      (VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT)
            .setMemoryUsage(VMA_MEMORY_USAGE_AUTO)
        	.build();

        // create copy regions
        const size_t faceSize = totalSize / CUBEMAP_FACE_COUNT;

        TVector<VkBufferImageCopy> bufferCopyRegions;
        bufferCopyRegions.reserve(static_cast<size_t>(CUBEMAP_FACE_COUNT) * mipLevels);

        for (U32 face = 0; face < CUBEMAP_FACE_COUNT; ++face) 
        {
            const VyDecodedImage& f = cubeMap.Faces[face];

            const VkDeviceSize faceBase = static_cast<VkDeviceSize>(face) * static_cast<VkDeviceSize>(faceSize);

            for (U32 mip = 0; mip < mipLevels; ++mip) 
            {
                U32 w = std::max(1, width  >> mip);
                U32 h = std::max(1, height >> mip);

                VkBufferImageCopy region{};
                {
                    region.bufferOffset      = faceBase + static_cast<VkDeviceSize>(f.MipOffsets[mip]);
                    region.bufferRowLength   = 0;
                    region.bufferImageHeight = 0;
                    
                    region.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
                    region.imageSubresource.mipLevel       = mip;
                    region.imageSubresource.baseArrayLayer = face;
                    region.imageSubresource.layerCount     = 1;
                    
                    region.imageOffset = { 0, 0, 0 };
                    region.imageExtent = { w, h, 1 };
                }

                // Optional: verify bufferOffset + mipSizes[mip] <= totalSize
                VkDeviceSize endByte = region.bufferOffset + static_cast<VkDeviceSize>(f.MipSizes[mip]);
                
                if (endByte > static_cast<VkDeviceSize>(totalSize)) 
                {
                    VY_THROW_RUNTIME_ERROR("uploadCubemap: region exceeds staging buffer (face " +
                        std::to_string(face) + " mip " + std::to_string(mip) + ")");
                }

                bufferCopyRegions.push_back(region);
            }
        }

        // image.transitionLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        image.copyFrom(stagingBuffer, true);

        VyImageView view  = VyImageView::Builder{}
			.setName    ("tex_upload_cubemap")
            .setViewType(VK_IMAGE_VIEW_TYPE_CUBE)
            .setFormat  (format)
            .setAspect  (VK_IMAGE_ASPECT_COLOR_BIT)
            .setLevels  (0, mipLevels)
            .setLayers  (0, 1)
        	.build( image );

        VySampler sampler = VySampler::Builder{}
            .setName         ("tex_upload_cubemap")
            .setFilters      (VK_FILTER_LINEAR)
            .setMipmapMode   (VK_SAMPLER_MIPMAP_MODE_LINEAR)
            .setWrap         (VK_SAMPLER_ADDRESS_MODE_REPEAT)
            .setBorder       (VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE)
            .enableAnisotropy(true)
            .setLodRange     (0.0f, static_cast<float>(mipLevels))
            .setMipLodBias   (0.0f)
            .build();

        return MakeUnique<VyTextureObject>(
            image,
            view,
            sampler,
            static_cast<U32>(width), 
            static_cast<U32>(height),
            mipLevels,
            CUBEMAP_FACE_COUNT,
            VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT
        );

        // VyTextureObject::TextureInitInfo info{
        //     .Image       = std::move(image),
        //     .View        = std::move(view),
        //     .Sampler     = std::move(sampler),
        //     .Width       = static_cast<U32>(width), 
        //     .Height      = static_cast<U32>(height),
        //     .MipLevels   = mipLevels,
        //     .ArrayLayers = CUBEMAP_FACE_COUNT,
        //     .Flags       = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT
        // };

        // return MakeUnique<VyTextureObject>(info);
    }


    Unique<VyTextureObject> TextureUploader::uploadCompressed2D(const VyDecodedImage& imageData)
    {
        validateCompressedImage(imageData);

        const U32      width     = imageData.Width;
        const U32      height    = imageData.Height;
        const U32      mipLevels = imageData.MipLevels;
        const VkFormat format    = imageData.Format;
        const size_t   dataSize  = imageData.DataSize;

        // staging buffer
        VyBuffer stagingBuffer{ VyBuffer::stagingBuffer("tex_upload_compressed_2d", dataSize) };

        stagingBuffer.write(imageData.CompressedData.data());

        VyImage image = VyImage::Builder{}
			.setName       ("tex_upload_compressed_2d")
            .setImageType  (VK_IMAGE_TYPE_2D)
            .setFormat     (format)
            .setExtent     (width, height)
            .setLevels     (mipLevels)
			.setLayers     (1)
			.setSamples    (VK_SAMPLE_COUNT_1_BIT)
            .setTiling     (VK_IMAGE_TILING_OPTIMAL)
			.setLayout     (VK_IMAGE_LAYOUT_UNDEFINED)
            .setUsage      (VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
            .setMemoryUsage(VMA_MEMORY_USAGE_AUTO)
        	.build();

        image.transitionLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        TVector<VkBufferImageCopy> regions(imageData.MipLevels);

        for (U32 mip = 0; mip < imageData.MipLevels; mip++)
        {
            regions[mip].bufferOffset = imageData.MipOffsets[mip];
            regions[mip].bufferRowLength = 0;
            regions[mip].bufferImageHeight = 0;

            U32 w = std::max(1, imageData.Width  >> mip);
            U32 h = std::max(1, imageData.Height >> mip);

            regions[mip].imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            regions[mip].imageSubresource.mipLevel = mip;
            regions[mip].imageSubresource.baseArrayLayer = 0;
            regions[mip].imageSubresource.layerCount = 1;

            regions[mip].imageOffset = { 0, 0, 0 };
            regions[mip].imageExtent = { w, h, 1 };
        }

        image.copyFrom(stagingBuffer, true);
        // VkCommandBuffer commandBuffer = VyContext::beginCommands(); 
        // vkCmdCopyBufferToImage(
        //     commandBuffer,
        //     stagingBuffer.handle(),
        //     image.handle(),
        //     VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        //     regions.size(),
        //     regions.data()
        // );

        // VyContext::endCommands(commandBuffer);

        // image.transitionLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        // create view and sampler
        VyImageView view;
        if (imageData.IsCubemap)
        {
            view = VyImageView::Builder{}
                .setName    ("tex_upload_compressed_2d_cubemap")
                .setViewType(VK_IMAGE_VIEW_TYPE_CUBE)
                .setFormat  (format)
                .setAspect  (VK_IMAGE_ASPECT_COLOR_BIT)
                .setLevels  (0, mipLevels)
                .setLayers  (0, CUBEMAP_FACE_COUNT)
                .build( image );
        }
        else
        {
            view = VyImageView::Builder{}
                .setName    ("tex_upload_compressed_2d")
                .setViewType(VK_IMAGE_VIEW_TYPE_2D)
                .setFormat  (format)
                .setAspect  (VK_IMAGE_ASPECT_COLOR_BIT)
                .setLevels  (0, mipLevels)
                .setLayers  (0, 1)
                .build( image );
        }

        VySampler sampler = VySampler::Builder{}
            .setName         ("tex_upload_compressed_2d")
            .setFilters      (VK_FILTER_LINEAR)
            .setMipmapMode   (VK_SAMPLER_MIPMAP_MODE_LINEAR)
            .setWrap         (VK_SAMPLER_ADDRESS_MODE_REPEAT)
            .setBorder       (VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE)
            .enableAnisotropy(true)
            .setLodRange     (0.0f, static_cast<float>(mipLevels))
            .setMipLodBias   (0.0f)
            .build();

        // VyTextureObject::TextureInitInfo info{
        //     .Image       = std::move(image),
        //     .View        = std::move(view),
        //     .Sampler     = std::move(sampler),
        //     .Width       = width, 
        //     .Height      = height,
        //     .MipLevels   = mipLevels,
        //     .ArrayLayers = imageData.IsCubemap ? CUBEMAP_FACE_COUNT : 1,
        //     .Flags       = imageData.IsCubemap ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : 0
        // };

        return MakeUnique<VyTextureObject>(
            image,
            view,
            sampler,
            width, 
            height,
            mipLevels,
            imageData.IsCubemap ? CUBEMAP_FACE_COUNT : 1,
            imageData.IsCubemap ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : 0
        );
    }
}