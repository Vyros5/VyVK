#include <VyEngine/GFX/Resources/Texture/Skybox.h>
#include <VyEngine/GFX/Resources/Texture/Texture.h>
#include <VyEngine/VK/Context.h>
#include <VyEngine/Globals.h>

#include <stb_image.h>

namespace Vy
{
    VySkybox::VySkybox(const TArray<TString, 6>& facePaths)
    {
        createCubemapImage(facePaths);
        createImageView();
        createSampler();
    }


    VySkybox::VySkybox(U32 size) : 
        m_Size{ static_cast<int>(size) }
    {
        // Create cubemap image for rendering
        m_Image = VyImage::Builder{}
            .setName       ("skybox_cubemap")
            .setImageType  (VK_IMAGE_TYPE_2D)
            .setFormat     (VK_FORMAT_R8G8B8A8_UNORM) // UNORM to store gamma-corrected values from shader
            .setExtent     (static_cast<U32>(m_Size), static_cast<U32>(m_Size))
            .setLevels     (1)
			.setLayers     (6) // 6 faces
			.setSamples    (VK_SAMPLE_COUNT_1_BIT)
            .setTiling     (VK_IMAGE_TILING_OPTIMAL)
			.setLayout     (VK_IMAGE_LAYOUT_UNDEFINED)
            .setUsage      (VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT)
            .setFlags      (VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT)
            .setMemoryUsage(VMA_MEMORY_USAGE_AUTO)
            .build();

        createImageView();
        createSampler();
    }


    VySkybox::~VySkybox()
    {
    }


    Unique<VySkybox> VySkybox::loadFromFolder(const TString& folderPath, const TString& extension)
    {
        TArray<TString, 6> facePaths = {
            folderPath + "/posx." + extension, // +X (right)
            folderPath + "/negx." + extension, // -X (left)
            folderPath + "/posy." + extension, // +Y (top)
            folderPath + "/negy." + extension, // -Y (bottom)
            folderPath + "/posz." + extension, // +Z (front)
            folderPath + "/negz." + extension, // -Z (back)
        };

        return MakeUnique<VySkybox>( facePaths );
    }


    void VySkybox::createCubemapImage(const TArray<TString, 6>& facePaths)
    {
        // Load all 6 faces and determine size
        TArray<U8*, 6> pFaceData{};
        
        int width = 0, height = 0;//, channels = 0;

        for (int i = 0; i < 6; i++)
        {
            // Load all faces.
            pFaceData[ i ] = VyTexture::loadImage(facePaths[ i ], width, height, STBI_rgb_alpha);
            // stbi_load(facePaths[ i ].c_str(), &width, &height, &channels, STBI_rgb_alpha);
            
            if ( !pFaceData[ i ] )
            {
                // Cleanup previously loaded images before throwing.
                for (int j = 0; j < i; j++)
                {
                    VyTexture::freeImageData( pFaceData[ j ] );
                }

                VY_THROW_RUNTIME_ERROR("Failed to load skybox texture face: " + 
					facePaths[ i ] + " - " + stbi_failure_reason());
            }

            // Verify all faces are same size.
            if ( i == 0 )
            {
                m_Size = width;

                if (width != height)
                {
                    // Cleanup
                    for (int j = 0; j <= i; ++j) 
					{
						if (pFaceData[ j ]) 
						{
                            VyTexture::freeImageData( pFaceData[ j ] );
						}
					}

                    VY_THROW_RUNTIME_ERROR("Skybox faces must be square. Face 0 (" + facePaths[0] + ") is " + 
						std::to_string(width) + "x" + std::to_string(height));
                }
            }
            else if ( width != m_Size || height != m_Size )
            {
                // Cleanup
                for (int j = 0; j <= i; j++)
                {
                    if (pFaceData[ j ]) 
                    {
                        VyTexture::freeImageData( pFaceData[ j ] );
                    }
                }

                VY_THROW_RUNTIME_ERROR("Skybox faces must have consistent dimensions. Face " + 
                    std::to_string(i) + " (" + facePaths[ i ] + ") is " +
                    std::to_string(width)  + "x" + std::to_string(height) + ", expected " + 
                    std::to_string(m_Size) + "x" + std::to_string(m_Size)
                );
            }
        }

        VkDeviceSize faceSize  = static_cast<VkDeviceSize>(m_Size) * static_cast<VkDeviceSize>(m_Size) * 4; // RGBA

        // Create staging buffer with all face data.
        VyBuffer stagingBuffer{ VyBuffer::stagingBuffer("skybox", faceSize, 6, 0, false /*not persistent*/) };

        stagingBuffer.map();

        for (int i = 0; i < 6; i++)
        {
            // Write each face into the buffer.
            stagingBuffer.writeToIndex( pFaceData[ i ], i );

            // Free CPU-side image data.
            VyTexture::freeImageData( pFaceData[ i ] );
        
			// Avoid double free.
            pFaceData[ i ] = nullptr;
        }
        
        stagingBuffer.unmap();

        // Create cubemap image
        m_Image = VyImage::Builder{}
            .setName       ("skybox_cubemap")
            .setImageType  (VK_IMAGE_TYPE_2D)
            .setFormat     (VK_FORMAT_R8G8B8A8_SRGB)
            .setExtent     (static_cast<U32>(m_Size), static_cast<U32>(m_Size))
            .setLevels     (1)
			.setLayers     (6) // 6 faces
			.setSamples    (VK_SAMPLE_COUNT_1_BIT)
            .setTiling     (VK_IMAGE_TILING_OPTIMAL)
			.setLayout     (VK_IMAGE_LAYOUT_UNDEFINED)
            .setUsage      (VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
            .setFlags      (VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT)
            .setMemoryUsage(VMA_MEMORY_USAGE_AUTO)
            .build();

        // Transition to transfer destination.
        transitionImageLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        // Copy each face from staging buffer.
        VkCommandBuffer cmdBuffer = VyContext::beginCommands();
        {
            TArray<VkBufferImageCopy, 6> regions{};

            for (int i = 0; i < 6; i++)
            {
                regions[ i ].bufferOffset                    = i * faceSize;
                regions[ i ].bufferRowLength                 = 0;
                regions[ i ].bufferImageHeight               = 0;
                regions[ i ].imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
                regions[ i ].imageSubresource.mipLevel       = 0;
                regions[ i ].imageSubresource.baseArrayLayer = i;
                regions[ i ].imageSubresource.layerCount     = 1;
                regions[ i ].imageOffset                     = { 0, 0, 0 };
                regions[ i ].imageExtent                     = { static_cast<U32>(m_Size), static_cast<U32>(m_Size), 1 };
            }

            vkCmdCopyBufferToImage(cmdBuffer, 
                stagingBuffer.handle(), 
                m_Image      .handle(), 
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
                6, regions.data()
            );
        }
        VyContext::endCommands(cmdBuffer);

        // Transition to shader read
        transitionImageLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    }


    void VySkybox::createImageView()
    {
        m_ImageView = VyImageView::Builder{}
            .setName    ("skybox_cubemap")
            .setViewType(VK_IMAGE_VIEW_TYPE_CUBE)
            .setFormat  (VK_FORMAT_R8G8B8A8_SRGB)
            .setAspect  (VK_IMAGE_ASPECT_COLOR_BIT)
            .setLevels  (0, 1)
            .setLayers  (0, 6) // 6 faces
            .build( m_Image );
    }


    void VySkybox::createSampler()
    {
        m_Sampler = VySampler::Builder{}
            .setName         ("skybox_cubemap")
            .setFilters      (VK_FILTER_LINEAR)
            .setMipmapMode   (VK_SAMPLER_MIPMAP_MODE_LINEAR)
            .setWrap         (VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE)
            .setBorder       (VK_BORDER_COLOR_INT_OPAQUE_BLACK)
			.enableAnisotropy(true)
            .setLodRange     (0.0f, 0.0f)
            .setMipLodBias   (0.0f)
            .build();
    }


    void VySkybox::transitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout)
    {
        VkCommandBuffer cmdBuffer = VyContext::beginCommands();
        {
            VkImageMemoryBarrier barrier{ VKInit::imageMemoryBarrier() };
            {
                barrier.image                           = m_Image.handle();

                barrier.oldLayout                       = oldLayout;
                barrier.newLayout                       = newLayout;

                barrier.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
                barrier.subresourceRange.baseMipLevel   = 0;
                barrier.subresourceRange.levelCount     = 1;
                barrier.subresourceRange.baseArrayLayer = 0;
                barrier.subresourceRange.layerCount     = 6;
            }

            VkPipelineStageFlags srcStage;
            VkPipelineStageFlags dstStage;

            if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && 
                newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
            {
                barrier.srcAccessMask = 0;
                barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

                srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
                dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            }
            else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && 
                     newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
            {
                barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

                srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
                dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            }
            else
            {
                VY_THROW_RUNTIME_ERROR("Unsupported layout transition");
            }

            vkCmdPipelineBarrier(cmdBuffer, 
                srcStage, dstStage, 
                0, 
                0, nullptr, 
                0, nullptr, 
                1, &barrier
            );
        }
        VyContext::endCommands(cmdBuffer);
    }
}