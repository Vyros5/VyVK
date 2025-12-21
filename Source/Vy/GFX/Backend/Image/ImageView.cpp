#include <Vy/GFX/Backend/Image/ImageView.h>

#include <Vy/GFX/Backend/Image/Image.h>
#include <Vy/GFX/Context.h>

namespace Vy
{
    VyImageView::VyImageView(const VyImageViewCreateInfo& desc, const VyImage& image)
    {
		VY_ASSERT(desc.BaseMipLevel + desc.LevelCount <= image.mipLevels(),  "Invalid mip level range");
		VY_ASSERT(desc.BaseLayer    + desc.LayerCount <= image.layerCount(), "Invalid layer range"    );

        VkImageViewCreateInfo viewInfo{ VKInit::imageViewCreateInfo() };
        {
            viewInfo.image    = image.handle();
            viewInfo.format   = VyImageViewCreateInfo::kUseImageFormat ? image.format() : desc.Format;
            viewInfo.viewType = desc.ViewType;

            viewInfo.subresourceRange.aspectMask     = VyImageViewCreateInfo::kUseImageAspectMask ? VyContext::device().findAspectFlags(image.format()) : desc.AspectMask;
            viewInfo.subresourceRange.baseMipLevel   = desc.BaseMipLevel;
            viewInfo.subresourceRange.baseArrayLayer = desc.BaseLayer;
			viewInfo.subresourceRange.levelCount     = desc.LevelCount == VyImageViewCreateInfo::kUseImageMipLevels ? image.mipLevels()  : desc.LevelCount;
			viewInfo.subresourceRange.layerCount     = desc.LayerCount == VyImageViewCreateInfo::kUseImageLayers    ? image.layerCount() : desc.LayerCount;

            viewInfo.components = desc.Components;
        }

        VK_CHECK(vkCreateImageView(VyContext::device(), &viewInfo, nullptr, &m_ImageView));

        // Store create info.
        m_Info = desc;
    
		if (desc.DebugName != "")
		{
			setName(desc.DebugName);
		}
    }


    VyImageView::VyImageView(VyImageView&& other) noexcept : 
        m_ImageView{ other.m_ImageView },
        m_Info     { other.m_Info      }
    {
        other.m_ImageView = VK_NULL_HANDLE;
    }


    VyImageView::~VyImageView()
    {
        destroy();
    }


    VyImageView& VyImageView::operator=(VyImageView&& other) noexcept
    {
        if (this != &other)
        {
            destroy();

            m_ImageView = other.m_ImageView;
            m_Info      = other.m_Info;
            
            other.m_ImageView = VK_NULL_HANDLE;
        }

        return *this;
    }


	void VyImageView::setName(const String& name) const
	{
		VyDebugLabel::setObjectName(reinterpret_cast<U64>(m_ImageView), VK_OBJECT_TYPE_IMAGE_VIEW, name.c_str());
	}


    void VyImageView::destroy()
    {
        VyContext::destroy(m_ImageView);

        m_ImageView = VK_NULL_HANDLE;
    }
}
