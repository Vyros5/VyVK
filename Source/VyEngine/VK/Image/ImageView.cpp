#include <VyEngine/VK/Image/ImageView.h>

#include <VyEngine/VK/Image/Image.h>
#include <VyEngine/VK/Context.h>

namespace Vy
{
    VyImageView::VyImageView(
        const TString&         name,
        const VyImageViewInfo& info, 
        const VyImage&         image
    ) :
        m_Info     { info                 },
        m_DebugName{ name + "_image_view" }
    {
        VY_ASSERT(image != VK_NULL_HANDLE);

        VkImageViewCreateInfo viewInfo{ VKInit::imageViewCreateInfo() };
        {
			viewInfo.image                           = image.handle();
			viewInfo.viewType                        = info.ViewType;
			viewInfo.format                          = image.format();
            viewInfo.subresourceRange.aspectMask     = VyContext::device().findAspectFlags(image.format());
            viewInfo.subresourceRange.baseMipLevel   = info.BaseMipLevel;
            viewInfo.subresourceRange.levelCount     = info.LevelCount == VyImageViewInfo::USE_IMAGE_MIP_LEVELS ? image.mipLevels() : info.LevelCount;
            viewInfo.subresourceRange.baseArrayLayer = info.BaseLayer;
            viewInfo.subresourceRange.layerCount     = info.LayerCount == VyImageViewInfo::USE_IMAGE_LAYERS ? image.layerCount() : info.LayerCount;
        }

        VK_CHECK_SUCCESS(vkCreateImageView(
            VyContext::device(), 
            &viewInfo, 
            nullptr, 
            &m_ImageView
        ), "Failed to create image view!");

        VKDbg::setObjectName(m_ImageView, m_DebugName.c_str());
    }


    VyImageView::~VyImageView() 
    {
        destroy();
    }

    
    VyImageView::VyImageView(VyImageView&& other) noexcept : 
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
        m_Info.ViewType = type; 

        return *this; 
    }

    VyImageView::Builder& 
    VyImageView::Builder::setFormat(VkFormat format)
    {
        m_Info.Format = format; 

        return *this; 
    }


    VyImageView::Builder& 
    VyImageView::Builder::setAspect(VkImageAspectFlags aspectMask)
    {
        m_Info.AspectMask = aspectMask; 

        return *this; 
    }

    VyImageView::Builder& 
    VyImageView::Builder::addAspect(VkImageAspectFlags aspectMask)
    {
        m_Info.AspectMask |= aspectMask; 

        return *this; 
    }
    

    VyImageView::Builder& 
    VyImageView::Builder::setLevels(U32 baseMipLevel, U32 levelCount /*= 1*/)
    {
        m_Info.BaseMipLevel = baseMipLevel;
        m_Info.LevelCount   = levelCount;

        return *this; 
    }

    VyImageView::Builder& 
    VyImageView::Builder::setBaseLevel(U32 baseMipLevel)
    {
        m_Info.BaseMipLevel = baseMipLevel;

        return *this; 
    }

    VyImageView::Builder& 
    VyImageView::Builder::setLevelCount(U32 levelCount)
    {
        m_Info.LevelCount = levelCount;

        return *this; 
    }


    VyImageView::Builder& 
    VyImageView::Builder::setLayers(U32 baseArrayLayer, U32 layerCount /*= 1*/)
    {
        m_Info.BaseLayer  = baseArrayLayer;
        m_Info.LayerCount = layerCount;

        return *this; 
    }

    VyImageView::Builder& 
    VyImageView::Builder::setBaseLayer(U32 baseArrayLayer)
    {
        m_Info.BaseLayer = baseArrayLayer;

        return *this; 
    }

    VyImageView::Builder& 
    VyImageView::Builder::setLayerCount(U32 layerCount)
    {
        m_Info.LayerCount = layerCount;

        return *this; 
    }

    VyImageView::Builder& 
    VyImageView::Builder::setComponents(VkComponentSwizzle swizzle)
    {
        m_Info.Components = { swizzle };

        return *this; 
    }

    VyImageView::Builder& 
    VyImageView::Builder::setComponentsRGBA(VkComponentSwizzle r, VkComponentSwizzle g, VkComponentSwizzle b, VkComponentSwizzle a)
    {
        m_Info.Components.r = r;
        m_Info.Components.g = g;
        m_Info.Components.b = b;
        m_Info.Components.a = a;

        return *this; 
    }

    VyImageView::Builder& 
    VyImageView::Builder::setMapping(VkComponentMapping mapping)
    {
        m_Info.Components = mapping;

        return *this; 
    }

    VyImageView::Builder& 
    VyImageView::Builder::setSubresourceRange(VkImageSubresourceRange subRange)
    {
        m_Info.AspectMask   = subRange.aspectMask;
        m_Info.BaseMipLevel = subRange.baseMipLevel;
        m_Info.LevelCount   = subRange.levelCount;
        m_Info.BaseLayer    = subRange.baseArrayLayer;
        m_Info.LayerCount   = subRange.layerCount;

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