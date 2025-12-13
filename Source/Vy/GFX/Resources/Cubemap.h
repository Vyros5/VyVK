#pragma once

#include <Vy/GFX/Backend/Resources/Image.h>
#include <Vy/GFX/Backend/Resources/ImageView.h>
#include <Vy/GFX/Backend/Resources/Sampler.h>
#include <Vy/GFX/Backend/Descriptors.h>

namespace Vy
{
	class VyCubemap //: public VulkanImage 
    {
	public:
		VyCubemap(
            U32               size, 
            VkFormat          format,
            VkImageUsageFlags usageFlags
        ) :
            m_Size       { size       },
            m_ImageFormat{ format     },
            m_UsageFlags { usageFlags }
        {
            m_Image = VyImage::Builder{}
                .extent( { size, size, 1 } )
                .format( format )
                .mipLevels( 1 )
                .arrayLayers( 6 )
                .usage( usageFlags )
                .createFlags( VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT )
                .tiling( VK_IMAGE_TILING_OPTIMAL )
                .memoryUsage( VMA_MEMORY_USAGE_AUTO )
                .build();

            // VyImageDesc imageDesc{
            //     .Format     = m_ImageFormat,
            //     .Extent     = { m_Size, m_Size, 1 },
            //     .MipLevels  = 1,
            //     .LayerCount = 6,
            //     .Usage      = m_UsageFlags,
            //     .Type       = VK_IMAGE_TYPE_2D,
            //     .Tiling     = VK_IMAGE_TILING_OPTIMAL,
            //     .Flags      = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT
            // };

            // m_Image = VyImage{ imageDesc };

            // VyImageViewDesc viewDesc{
            //     .BaseMipLevel = 0,
            //     .LevelCount   = 1,
            //     .BaseLayer    = 0,
            //     .LayerCount   = 6,
            //     .Type         = VK_IMAGE_VIEW_TYPE_CUBE
            // }; // VyImageView{ viewDesc, m_Image };

            // this is the image view for the whole cube map
            m_ImageView = VyImageView::Builder{}
                .viewType( VK_IMAGE_VIEW_TYPE_CUBE )
                .format( format )
                .aspectMask( VK_IMAGE_ASPECT_COLOR_BIT )
                .mipLevels( 0, 1 )
                .arrayLayers( 0, 6 )
                .build( m_Image );

            // now we create the image views for each face of the cube map
            // viewDesc.Type       = VK_IMAGE_VIEW_TYPE_2D;
            // viewDesc.LayerCount = 1;
        
            for (U32 i = 0; i < 6; i++) // VyImageView{ viewDesc, m_Image };
            {
                // viewDesc.BaseLayer = i;

                m_CubeFaceViews[i] = VyImageView::Builder{}
                    .viewType( VK_IMAGE_VIEW_TYPE_2D )
                    .format( format )
                    .aspectMask( VK_IMAGE_ASPECT_COLOR_BIT )
                    .mipLevels( 0, 1 )
                    .arrayLayers( i, 1 )
                    .build( m_Image );
            }


            // VySamplerDesc samplerDesc{
            //     .MagFilter   = VK_FILTER_LINEAR,
            //     .MinFilter   = VK_FILTER_LINEAR,
            //     .AddressMode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
            //     .MipmapMode  = VK_SAMPLER_MIPMAP_MODE_LINEAR,
            //     .MaxLod      = 1.0f
            // };

            m_Sampler = VySampler::Builder{}
                .filters( VK_FILTER_LINEAR, VK_FILTER_LINEAR )
                .addressMode( VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER )
                .mipmapMode( VK_SAMPLER_MIPMAP_MODE_LINEAR )
                .lodRange( 0.0f, 1.0f )
                .build(); 
            
            
            
            //VySampler{ samplerDesc };
        }


		~VyCubemap()
        {
        }

        // void copyFrom(VkCommandBuffer cmdBuffer, const VyBuffer& src)
        // {
        //     m_Image.copyFrom(cmdBuffer, src);
        // }

		const VyImageView& faceImageView(U32 faceIndex) const { return m_CubeFaceViews[ faceIndex ]; }

        const VyImage& image() const { return m_Image; }
        const VySampler& sampler() const { return m_Sampler; }
        const VyImageView& view() const { return m_ImageView; }
    private:
		U32 m_Size; // Size of the cube map faces

		// void createImage();
		// void createImageViews();
		// void createSampler();

        VyImage  m_Image;
        VySampler m_Sampler;
		VkFormat          m_ImageFormat;
		VkImageUsageFlags m_UsageFlags;

        VyImageView m_ImageView;

		TArray<VyImageView, 6> m_CubeFaceViews;
	};


	class VySkybox //: public Skybox 
    {
	public:
		static Unique<VySkybox> create(const TArray<String, 6>& paths);

		VySkybox(const TArray<String, 6>& paths);

		~VySkybox()  = default;

		void createDescriptorSet();

		VkDescriptorImageInfo descriptorImageInfo() const;
		VkDescriptorSet       descriptorSet()       const { return m_SkyboxDescriptorSet; }
		VkDescriptorSetLayout descriptorSetLayout() const { return m_SkyboxDescriptorSetLayout->handle(); }

	private:
		void loadTextures(const TArray<String, 6>& paths);

		
		U32 m_Size = 0;
		Unique<VyCubemap> m_Cubemap;

		VkDescriptorSet               m_SkyboxDescriptorSet;
		Unique<VyDescriptorSetLayout> m_SkyboxDescriptorSetLayout;
	};
}