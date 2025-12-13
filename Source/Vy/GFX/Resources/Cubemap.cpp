#include <Vy/GFX/Resources/Cubemap.h>

#include <Vy/GFX/Context.h>
#include <Vy/Globals.h>

#include <stb_image.h>

namespace Vy
{
    Unique<VySkybox> VySkybox::create(const TArray<String, 6>& paths)
    {
        return MakeUnique<VySkybox>(paths);
    }


    VySkybox::VySkybox(const TArray<String, 6>& paths)
    {
        loadTextures(paths);

    }


    void VySkybox::createDescriptorSet()
    {
        m_SkyboxDescriptorSetLayout = VyDescriptorSetLayout::Builder()
            .addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
            .buildUnique();

        VyContext::allocateSet(m_SkyboxDescriptorSetLayout->handle(), m_SkyboxDescriptorSet);

        // Get the VkDescriptorImageInfo from the Skybox object
        VkDescriptorImageInfo skyboxImageInfo = descriptorImageInfo();

        VyDescriptorWriter{ *m_SkyboxDescriptorSetLayout, *VyContext::globalPool() }
            .writeImage(0, &skyboxImageInfo)
            .update(m_SkyboxDescriptorSet);
    }


    VkDescriptorImageInfo VySkybox::descriptorImageInfo() const 
    {
        VkDescriptorImageInfo imageInfo{};
        {
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.sampler     = m_Cubemap->sampler().handle();
            imageInfo.imageView   = m_Cubemap->view().handle();
        }

        return imageInfo;
    }


	void VySkybox::loadTextures(const TArray<String, 6>& paths)
	{
		VkFormat format = VK_FORMAT_R8G8B8A8_SRGB;

		int texWidth, texHeight, texChannels;

		U8* pixels[6] = { nullptr };

        for (int i = 0; i < 6; ++i) 
		{
            // Load all faces.
            pixels[i] = stbi_load(paths[i].c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

            if (!pixels[i])
			{
                // Cleanup previously loaded images before throwing.
                for (int j = 0; j < i; ++j) 
				{
                    if (pixels[j]) 
					{
						stbi_image_free(pixels[j]);
					}
                }

                VY_THROW_RUNTIME_ERROR("Failed to load skybox texture face: " + 
					paths[i] + " - " + stbi_failure_reason());
            }

            if (i == 0) 
			{
                m_Size = static_cast<U32>(texWidth);
                
				if (texWidth != texHeight) 
				{
                    // Cleanup
                    for (int j = 0; j <= i; ++j) 
					{
						if (pixels[j]) 
						{
							stbi_image_free(pixels[j]);
						}
					}

                    VY_THROW_RUNTIME_ERROR("Skybox faces must be square. Face 0 (" + paths[0] + ") is " + 
						std::to_string(texWidth) + "x" + std::to_string(texHeight));
                }
            } 
			else {
                if (static_cast<U32>(texWidth)  != m_Size || 
					static_cast<U32>(texHeight) != m_Size) 
				{
                    // Cleanup
                    for (int j = 0; j <= i; ++j) 
					{
						if (pixels[j]) 
						{
							stbi_image_free(pixels[j]);
						}
					}

                    VY_THROW_RUNTIME_ERROR("Skybox faces must have consistent dimensions. Face " + 
						std::to_string(i) + " (" + paths[i] + ") is " +
                        std::to_string(texWidth) + "x" + std::to_string(texHeight) + ", expected " + 
						std::to_string(m_Size) + "x" + std::to_string(m_Size)
					);
                }
            }
        }

		VkDeviceSize layerSize = m_Size * m_Size * 4;
        VkDeviceSize imageSize = layerSize * 6;

        m_Cubemap = MakeUnique<VyCubemap>(
            m_Size, 
            format, 
            VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT
        );

		VyBuffer stagingBuffer{ VyBuffer::stagingBuffer(imageSize) };
		stagingBuffer.map();

        VkDeviceSize currentOffset = 0;
        
        for (int i = 0; i < 6; ++i) 
		{
            // Write each layer into the buffer.
            stagingBuffer.write(pixels[i], layerSize, currentOffset);

			// Free CPU-side image data.
            stbi_image_free(pixels[i]);

			// Avoid double free.
            pixels[i] = nullptr;

            // Advance offset,
            currentOffset += layerSize;
        }

        stagingBuffer.unmap();
        // VkImageSubresourceRange cubemapSubresourceRange{};
        // cubemapSubresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        // cubemapSubresourceRange.baseMipLevel = 0;
        // cubemapSubresourceRange.levelCount = 1;
        // cubemapSubresourceRange.baseArrayLayer = 0;
        // cubemapSubresourceRange.layerCount = 6;

        // auto cmdBuffer = VyContext::device().beginSingleTimeCommands();
        // {
        //     m_Cubemap->copyFrom(cmdBuffer, stagingBuffer);
        // }
        // VyContext::device().endSingleTimeCommands(cmdBuffer);

        VyContext::device().transitionImageLayout(
            m_Cubemap->image(),
            format,
            m_Cubemap->image().layout(),
            // VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            0,
            6
        );

        // m_Cubemap->(
        //     VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        //     VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        //     VK_PIPELINE_STAGE_TRANSFER_BIT,
        //     cubemapSubresourceRange
        // );

        VyContext::device().copyBufferToImage(
            stagingBuffer.handle(), 
            m_Cubemap->image().handle(), 
            m_Size, 
            m_Size, 
            6
        );

        VyContext::device().transitionImageLayout(
            m_Cubemap->image(),
            format,
            // m_Cubemap->image().layout(),
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            0,
            6
        );
        // m_Cubemap->transitionImageLayoutSingleTimeCmd(
        //     VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        //     VK_PIPELINE_STAGE_TRANSFER_BIT,
        //     VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        //     cubemapSubresourceRange
        // );
	}
}