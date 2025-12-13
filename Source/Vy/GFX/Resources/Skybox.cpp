#include <Vy/GFX/Resources/Skybox.h>

#include <Vy/GFX/Context.h>


#include <stb_image.h>

namespace Vy
{
    Skybox::Skybox(const TArray<String, 6>& facePaths)
    {
        createCubemapImage(facePaths);
        createImageView();
        createSampler();
    }

    Skybox::~Skybox()
    {
    }


    Unique<Skybox> Skybox::loadFromFolder(const String& folderPath, const String& extension)
    {
        TArray<String, 6> facePaths = {
                folderPath + "/posx." + extension, // +X (right)
                folderPath + "/negx." + extension, // -X (left)
                folderPath + "/posy." + extension, // +Y (top)
                folderPath + "/negy." + extension, // -Y (bottom)
                folderPath + "/posz." + extension, // +Z (front)
                folderPath + "/negz." + extension, // -Z (back)
        };

        return std::make_unique<Skybox>(facePaths);
    }


    void Skybox::createCubemapImage(const TArray<String, 6>& facePaths)
    {
        // Load all 6 faces and determine size
        TArray<unsigned char*, 6> faceData{};
        
        int width = 0, height = 0, channels = 0;

        for (int i = 0; i < 6; i++)
        {
            faceData[i] = stbi_load(facePaths[i].c_str(), &width, &height, &channels, STBI_rgb_alpha);
            
            if (!faceData[i])
            {
                // Clean up already loaded faces
                for (int j = 0; j < i; j++)
                {
                    stbi_image_free(faceData[j]);
                }

                VY_THROW_RUNTIME_ERROR("Failed to load skybox face: " + facePaths[i]);
            }

            // Verify all faces are same size
            if (i == 0)
            {
                m_Size = width;

                if (width != height)
                {
                    stbi_image_free(faceData[i]);
                
                    VY_THROW_RUNTIME_ERROR("Skybox faces must be square: " + facePaths[i]);
                }
            }
            else if (width != m_Size || height != m_Size)
            {
                for (int j = 0; j <= i; j++)
                {
                    stbi_image_free(faceData[j]);
                }

                VY_THROW_RUNTIME_ERROR("All skybox faces must be same size: " + facePaths[i]);
            }
        }

        VkDeviceSize faceSize  = static_cast<VkDeviceSize>(m_Size) * m_Size * 4; // RGBA
        VkDeviceSize totalSize = faceSize * 6;

        // Create staging buffer with all face data
        VyBuffer stagingBuffer{ VyBuffer::stagingBuffer(faceSize, 6) };

        stagingBuffer.map();

        for (int i = 0; i < 6; i++)
        {
            stagingBuffer.writeToIndex(faceData[i], i);

            stbi_image_free(faceData[i]);
        }
        stagingBuffer.unmap();

        // Create cubemap image
        m_Image = VyImage::Builder{}
            .imageType  (VK_IMAGE_TYPE_2D)
            .format     (VK_FORMAT_R8G8B8A8_SRGB)
            .extent     (VkExtent2D{ static_cast<U32>(m_Size), static_cast<U32>(m_Size) })
            .mipLevels  (1)
            .arrayLayers(6)
            .tiling     (VK_IMAGE_TILING_OPTIMAL)
            .sampleCount(VK_SAMPLE_COUNT_1_BIT)
            .usage      (VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
            .memoryUsage(VMA_MEMORY_USAGE_AUTO)
            .createFlags(VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT)
        .build();

        // Transition to transfer destination
        transitionImageLayout(
            VK_IMAGE_LAYOUT_UNDEFINED, 
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
        );

        // Copy each face from staging buffer
        VkCommandBuffer commandBuffer = VyContext::device().beginSingleTimeCommands();
        {
            TArray<VkBufferImageCopy, 6> regions{};

            for (int i = 0; i < 6; i++)
            {
                regions[i].bufferOffset                    = i * faceSize;
                regions[i].bufferRowLength                 = 0;
                regions[i].bufferImageHeight               = 0;

                regions[i].imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
                regions[i].imageSubresource.mipLevel       = 0;
                regions[i].imageSubresource.baseArrayLayer = i;
                regions[i].imageSubresource.layerCount     = 1;
                
                regions[i].imageOffset                     = {0, 0, 0};
                regions[i].imageExtent                     = {static_cast<U32>(m_Size), static_cast<U32>(m_Size), 1};
            }

            vkCmdCopyBufferToImage(commandBuffer, stagingBuffer.handle(), m_Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 6, regions.data());
        }
        VyContext::device().endSingleTimeCommands(commandBuffer);

        // Transition to shader read
        transitionImageLayout(
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        );
    }


    void Skybox::createImageView()
    {
        m_ImageView = VyImageView::Builder{}
            .viewType   (VK_IMAGE_VIEW_TYPE_CUBE)
            .format     (VK_FORMAT_R8G8B8A8_SRGB)
            .aspectMask (VK_IMAGE_ASPECT_COLOR_BIT)
            .mipLevels  (0, 1)
            .arrayLayers(0, 6)
        .build(m_Image);
    }


    void Skybox::createSampler()
    {
        m_Sampler = VySampler::Builder{}
            .filters         (VK_FILTER_LINEAR)
            .mipmapMode      (VK_SAMPLER_MIPMAP_MODE_LINEAR)
            .addressMode     (VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE)
            .borderColor     (VK_BORDER_COLOR_INT_OPAQUE_BLACK)
            .enableAnisotropy(true)
            .lodRange        (0.0f, 0.0f)
            .mipLodBias      (0.0f)
        .build();
    }


    void Skybox::transitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout)
    {
        VkCommandBuffer commandBuffer = VyContext::device().beginSingleTimeCommands();
        {
            VkPipelineStageFlags sourceStage;
            VkPipelineStageFlags destinationStage;

            VkImageMemoryBarrier barrier{ VKInit::imageMemoryBarrier() };
            {
                barrier.oldLayout                       = oldLayout;
                barrier.newLayout                       = newLayout;
                barrier.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
                barrier.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
                barrier.image                           = m_Image;

                barrier.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
                barrier.subresourceRange.baseMipLevel   = 0;
                barrier.subresourceRange.levelCount     = 1;
                barrier.subresourceRange.baseArrayLayer = 0;
                barrier.subresourceRange.layerCount     = 6;

                if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && 
                    newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
                {
                    barrier.srcAccessMask = 0;
                    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

                    sourceStage      = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
                    destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
                }
                else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && 
                         newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
                {
                    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

                    sourceStage      = VK_PIPELINE_STAGE_TRANSFER_BIT;
                    destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                }
                else
                {
                    VY_THROW_RUNTIME_ERROR("Unsupported layout transition");
                }
            }
            vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
        }
        VyContext::device().endSingleTimeCommands(commandBuffer);
    }
}