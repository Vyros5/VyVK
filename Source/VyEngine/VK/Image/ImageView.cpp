#include <VyEngine/VK/Image/ImageView.h>

#include <VyEngine/VK/Image/Image.h>
#include <VyEngine/VK/Context.h>

namespace Vy
{
    VyImageView::VyImageView(
        const TString&               name,
        const VkImageViewCreateInfo& info, 
        const VyImage&               image
    ) :
        m_Info     { info                 },
        m_DebugName{ name + "_image_view" }
    {
        VY_ASSERT(image != VK_NULL_HANDLE);

        // VkImageViewCreateInfo viewInfo = info;
        m_Info.image = image.handle();

        VK_CHECK_SUCCESS(vkCreateImageView(VyContext::device(), &m_Info, nullptr, &m_ImageView),
            "Failed to create image view!");

        VKDbg::setObjectName(m_ImageView, m_DebugName.c_str());
    }


    VyImageView::VyImageView(
        const TString&         name,
        VyImage&               image,
        VkImageViewType        viewType /*VK_IMAGE_VIEW_TYPE_2D*/
    ) :
        m_DebugName{ name + "_image_view" }
    {
        VkImageViewCreateInfo viewInfo{ VKInit::imageViewCreateInfo() };
        {
            viewInfo.image            = image.handle();
            viewInfo.format           = image.format();
            viewInfo.viewType         = viewType;
            viewInfo.subresourceRange = {
                .baseMipLevel   = 0,
                .levelCount     = image.mipLevels(),
                .baseArrayLayer = 0,
                .layerCount     = image.layerCount(),
            };
        }

        if (VKUtil::isDepthFormat(image.format()))
        {
            viewInfo.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_DEPTH_BIT;

            if (VKUtil::isDepthStencil(image.format()))
            {
                viewInfo.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
            }
        }
        else
        {
            viewInfo.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_COLOR_BIT;
        }

        m_Info = viewInfo;

        VK_CHECK_SUCCESS(vkCreateImageView(VyContext::device(), &viewInfo, nullptr, &m_ImageView),
            "Failed to create image view!");

        VKDbg::setObjectName(m_ImageView, m_DebugName.c_str());
    }

    

    VyImageView::VyImageView(
        const TString&          name,
        const VyImage&          image,
        VkImageViewType         viewType,
        VkFormat                format,
        VkComponentMapping      componentMapping,
        VkImageSubresourceRange subresourceRange
    ) :
        m_DebugName{ name + "_image_view" }
    {
		VY_ASSERT(subresourceRange.baseMipLevel   + subresourceRange.levelCount <= image.mipLevels(),  "Invalid mip level range");
		VY_ASSERT(subresourceRange.baseArrayLayer + subresourceRange.layerCount <= image.layerCount(), "Invalid layer range"    );

        VkImageViewCreateInfo viewInfo{ VKInit::imageViewCreateInfo() };
        {
            viewInfo.image            = image.handle();
            viewInfo.format           = format == VK_FORMAT_UNDEFINED ? image.format() : format;
            viewInfo.viewType         = viewType;
            viewInfo.subresourceRange = subresourceRange;
            viewInfo.components       = componentMapping;
        }

        m_Info = viewInfo;

        VK_CHECK_SUCCESS(vkCreateImageView(VyContext::device(), &viewInfo, nullptr, &m_ImageView),
            "Failed to create image view!");

        VKDbg::setObjectName(m_ImageView, m_DebugName.c_str());
    }


    VyImageView::~VyImageView() 
    {
        destroy();
    }

    
    VyImageView::VyImageView(VyImageView&& other) noexcept : 
        // m_pOwnerImage{ other.m_pOwnerImage }, 
        m_ImageView  { other.m_ImageView   },
        m_DebugName  { other.m_DebugName   },
        m_Info       { other.m_Info        } 
    {
        other.m_ImageView = VK_NULL_HANDLE;
    }


    VyImageView& VyImageView::operator=(VyImageView&& other) noexcept 
    {
        if (this != &other) 
        {
            destroy();

            // m_pOwnerImage = other.m_pOwnerImage;
            m_ImageView   = other.m_ImageView;
            m_DebugName   = other.m_DebugName;
            m_Info        = other.m_Info;
            
            other.m_ImageView = VK_NULL_HANDLE;
        }

        return *this;
    }


    void VyImageView::destroy()
    {
        if (m_ImageView)
        {
            VyContext::destroy( m_ImageView );
        }

        m_ImageView = VK_NULL_HANDLE;
    }




    VyImageView::Builder::Builder()
    {
        // Setup Default Values
        m_Info = VKInit::imageViewCreateInfo();
        {
            m_Info.flags      = 0;
            m_Info.image      = nullptr;
            m_Info.viewType   = VK_IMAGE_VIEW_TYPE_2D;
            m_Info.format     = VK_FORMAT_UNDEFINED;
            m_Info.components = { VK_COMPONENT_SWIZZLE_IDENTITY };
            
            m_Info.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
            m_Info.subresourceRange.baseMipLevel   = 0;
            m_Info.subresourceRange.levelCount     = 1;
            m_Info.subresourceRange.baseArrayLayer = 0;
            m_Info.subresourceRange.layerCount     = 1;
        }
    }

    VyImageView::Builder& 
    VyImageView::Builder::setName(const TString& name)
    {
        m_Name = name;
        
        return *this;
    }

    VyImageView::Builder& 
    VyImageView::Builder::setViewType(VkImageViewType type)
    {
        m_Info.viewType = type; 

        return *this; 
    }

    VyImageView::Builder& 
    VyImageView::Builder::setFormat(VkFormat format)
    {
        m_Info.format = format; 

        return *this; 
    }


    VyImageView::Builder& 
    VyImageView::Builder::setAspect(VkImageAspectFlags aspectMask)
    {
        m_Info.subresourceRange.aspectMask = aspectMask; 

        return *this; 
    }

    VyImageView::Builder& 
    VyImageView::Builder::addAspect(VkImageAspectFlags aspectMask)
    {
        m_Info.subresourceRange.aspectMask |= aspectMask; 

        return *this; 
    }
    

    VyImageView::Builder& 
    VyImageView::Builder::setLevels(U32 baseMipLevel, U32 levelCount /*= 1*/)
    {
        m_Info.subresourceRange.baseMipLevel = baseMipLevel;
        m_Info.subresourceRange.levelCount   = levelCount;

        return *this; 
    }

    VyImageView::Builder& 
    VyImageView::Builder::setBaseLevel(U32 baseMipLevel)
    {
        m_Info.subresourceRange.baseMipLevel = baseMipLevel;

        return *this; 
    }

    VyImageView::Builder& 
    VyImageView::Builder::setLevelCount(U32 levelCount)
    {
        m_Info.subresourceRange.levelCount = levelCount;

        return *this; 
    }


    VyImageView::Builder& 
    VyImageView::Builder::setLayers(U32 baseArrayLayer, U32 layerCount /*= 1*/)
    {
        m_Info.subresourceRange.baseArrayLayer = baseArrayLayer;
        m_Info.subresourceRange.layerCount     = layerCount;

        return *this; 
    }

    VyImageView::Builder& 
    VyImageView::Builder::setBaseLayer(U32 baseArrayLayer)
    {
        m_Info.subresourceRange.baseArrayLayer = baseArrayLayer;

        return *this; 
    }

    VyImageView::Builder& 
    VyImageView::Builder::setLayerCount(U32 layerCount)
    {
        m_Info.subresourceRange.layerCount = layerCount;

        return *this; 
    }

    VyImageView::Builder& 
    VyImageView::Builder::setComponents(VkComponentSwizzle swizzle)
    {
        m_Info.components = { swizzle };

        return *this; 
    }

    VyImageView::Builder& 
    VyImageView::Builder::setComponentsRGBA(VkComponentSwizzle r, VkComponentSwizzle g, VkComponentSwizzle b, VkComponentSwizzle a)
    {
        m_Info.components.r = r;
        m_Info.components.g = g;
        m_Info.components.b = b;
        m_Info.components.a = a;

        return *this; 
    }

    VyImageView::Builder& 
    VyImageView::Builder::setMapping(VkComponentMapping mapping)
    {
        m_Info.components = mapping;

        return *this; 
    }

    VyImageView::Builder& 
    VyImageView::Builder::setSubresourceRange(VkImageSubresourceRange subRange)
    {
        m_Info.subresourceRange = subRange;

        return *this; 
    }

    VyImageView 
    VyImageView::Builder::build(const VyImage& inImage) const 
    {
        return VyImageView{ m_Name, m_Info, inImage };
    }

    Unique<VyImageView> 
    VyImageView::Builder::buildPtr(const VyImage& inImage) const 
    {
        return MakeUnique<VyImageView>( m_Name, m_Info, inImage );
    }
}