#pragma once

#include <Vy/GFX/Backend/Image/Image.h>
#include <Vy/GFX/Backend/Image/ImageView.h>
#include <Vy/GFX/Backend/Image/Sampler.h>

namespace Vy
{
	struct VyImageAttachmentCreateInfo
	{
		VkFormat           Format     = VK_FORMAT_R8G8B8A8_SRGB;
		VkExtent2D         Extent 	  = { 1, 1 };
		VkImageUsageFlags  Usage      = VK_IMAGE_USAGE_SAMPLED_BIT;
        VkImageAspectFlags AspectMask = VyImageViewCreateInfo::kUseImageAspectMask;
    };


    class VyImageAttachment 
    {
    public:
		class Builder
		{
			friend class VyImageAttachment;
        
        public:

            Builder() = default;

		    Builder& extent(VkExtent2D exent)
		    {
		        m_Info.Extent = exent;
		        return *this;
		    }

		    Builder& format(VkFormat format)
			{
				m_Info.Format = format; 
				return *this;
			}
            
			Builder& usage(VkImageUsageFlags usage)
			{
				m_Info.Usage = usage; 
				return *this;
			}

			Builder& aspectMask(VkImageAspectFlags aspectMask)
			{
				m_Info.AspectMask = aspectMask; 
				return *this; 
			}

            VyImageAttachment build() const 
            {
				VY_ASSERT(m_Info.Extent.width > 1 && m_Info.Extent.height > 1, 
					"Image extent dimensions must be greater than 1.");

                return VyImageAttachment{ m_Info };
            }

            Unique<VyImageAttachment> buildUnique() const 
            {
				VY_ASSERT(m_Info.Extent.width > 1 && m_Info.Extent.height > 1, 
					"Image extent dimensions must be greater than 1.");

                return MakeUnique<VyImageAttachment>(m_Info);
            }

            Shared<VyImageAttachment> buildShared() const 
            {
				VY_ASSERT(m_Info.Extent.width > 1 && m_Info.Extent.height > 1, 
					"Image extent dimensions must be greater than 1.");

                return MakeShared<VyImageAttachment>(m_Info);
            }

		private:
			VyImageAttachmentCreateInfo m_Info{};
        };

    public:

        VyImageAttachment() = default;

        VyImageAttachment(const VyImageAttachmentCreateInfo& info)
        {
            m_Image = VyImage::Builder{}
                .imageType  (VK_IMAGE_TYPE_2D)
                .extent     (info.Extent)
                .mipLevels  (1)
                .arrayLayers(1)
                .format     (info.Format)
                .tiling     (VK_IMAGE_TILING_OPTIMAL)
                .sampleCount(VK_SAMPLE_COUNT_1_BIT)
                .sharingMode(VK_SHARING_MODE_EXCLUSIVE)
                .usage      (info.Usage)
                .memoryUsage(VMA_MEMORY_USAGE_AUTO)
            .build();

            m_View = VyImageView::Builder{}
                .viewType   (VK_IMAGE_VIEW_TYPE_2D)
                .format     (info.Format)
                .aspectMask (info.AspectMask)
                .mipLevels  (0, 1)
                .arrayLayers(0, 1)
            .build(m_Image);
        }

        // // Construct attachment with an existing VkImage handle
        // VyImageAttachment(
        //     VkExtent2D extent,
        //     VkImage image,
        //     VkFormat format,
        //     VkImageAspectFlags aspectMask
        // );

        ~VyImageAttachment()
        {

        }

        VkImage               image()         const { return m_Image.handle();    }
        VkImageView           imageView()     const { return m_View.handle();     }
        VyImageViewCreateInfo imageViewInfo() const { return m_View.createInfo(); }
        VkFormat              format()        const { return m_Image.format(); }
        VkExtent2D            extent()        const { return VkExtent2D{ m_Image.width(), m_Image.height() }; }

    private:
        VyImage     m_Image;
        VyImageView m_View;
    };
}