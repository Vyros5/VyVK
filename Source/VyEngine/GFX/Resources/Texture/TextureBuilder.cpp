#include <VyEngine/GFX/Resources/Texture/Texture.h>
#include <VyEngine/VK/Context.h>

namespace Vy
{
    VyTexture2::Builder::Builder()
    {
        // Setup Default Values
        m_Name = { "unnamed" };

        m_ImageInfo = VKInit::imageCreateInfo();
        {
            m_ImageInfo.imageType     = VK_IMAGE_TYPE_2D;
            m_ImageInfo.extent.width  = 0;
            m_ImageInfo.extent.height = 0;
            m_ImageInfo.extent.depth  = 1;
            m_ImageInfo.mipLevels     = 1;
            m_ImageInfo.arrayLayers   = 1;
            m_ImageInfo.format        = VK_FORMAT_UNDEFINED;
            m_ImageInfo.tiling        = VK_IMAGE_TILING_OPTIMAL;
            m_ImageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            m_ImageInfo.usage         = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
            m_ImageInfo.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;
            m_ImageInfo.samples       = VK_SAMPLE_COUNT_1_BIT;
            m_ImageInfo.flags         = 0;
        }

        m_AllocInfo = {};
        {
            m_AllocInfo.usage = VMA_MEMORY_USAGE_AUTO;
            m_AllocInfo.flags = 0;
        }


        m_ViewInfo = VKInit::imageViewCreateInfo();
        {
            m_ViewInfo.flags      = 0;
            m_ViewInfo.image      = nullptr;
            m_ViewInfo.viewType   = VK_IMAGE_VIEW_TYPE_2D;
            m_ViewInfo.format     = VK_FORMAT_UNDEFINED;
            m_ViewInfo.components = { VK_COMPONENT_SWIZZLE_IDENTITY };
            
            m_ViewInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
            m_ViewInfo.subresourceRange.baseMipLevel   = 0;
            m_ViewInfo.subresourceRange.levelCount     = 1;
            m_ViewInfo.subresourceRange.baseArrayLayer = 0;
            m_ViewInfo.subresourceRange.layerCount     = 1;
        }

		m_SamplerInfo = VKInit::samplerCreateInfo();
		{
			m_SamplerInfo.magFilter               = VK_FILTER_LINEAR;
			m_SamplerInfo.minFilter               = VK_FILTER_LINEAR;
			m_SamplerInfo.addressModeU            = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			m_SamplerInfo.addressModeV            = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			m_SamplerInfo.addressModeW            = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			m_SamplerInfo.anisotropyEnable        = VK_FALSE;
			m_SamplerInfo.maxAnisotropy           = 0;
			m_SamplerInfo.borderColor             = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
			m_SamplerInfo.unnormalizedCoordinates = VK_FALSE;
			m_SamplerInfo.compareEnable           = VK_FALSE;
			m_SamplerInfo.compareOp               = VK_COMPARE_OP_ALWAYS;
			m_SamplerInfo.mipmapMode              = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			m_SamplerInfo.mipLodBias              = 0.0f;
			m_SamplerInfo.minLod                  = 0.0f;
			m_SamplerInfo.maxLod                  = VK_LOD_CLAMP_NONE;
		}
    }


    VyTexture2::Builder& 
    VyTexture2::Builder::setName(const TString& name)
    {
        m_Name = name;

        return *this;
    }


    VyTexture2::Builder& 
    VyTexture2::Builder::setWidth(U32 width)
    {
        m_ImageInfo.extent.width = width;

        return *this;
    }
    
    VyTexture2::Builder& 
    VyTexture2::Builder::setHeight(U32 height)
    {
        m_ImageInfo.extent.height = height;

        return *this;
    }
    
    VyTexture2::Builder& 
    VyTexture2::Builder::setDepth(U32 depth)
    {
        m_ImageInfo.extent.depth = depth;

        return *this;
    }

    VyTexture2::Builder& 
    VyTexture2::Builder::setExtent2D(U32 extent)
    {
        m_ImageInfo.extent = VkExtent3D{ extent, extent, 1 };

        return *this;
    }
    
    VyTexture2::Builder& 
    VyTexture2::Builder::setExtent(VkExtent2D extent)
    {
        m_ImageInfo.extent = VkExtent3D{ extent.width, extent.height, 1 };

        return *this;
    }
    
    VyTexture2::Builder& 
    VyTexture2::Builder::setExtent(VkExtent3D extent)
    {
        m_ImageInfo.extent = extent;

        return *this;
    }
    
    VyTexture2::Builder& 
    VyTexture2::Builder::setExtent(U32 width, U32 height, U32 depth)
    {
        m_ImageInfo.extent = VkExtent3D{ width, height, depth };

        return *this;
    }

    VyTexture2::Builder& 
    VyTexture2::Builder::setImageType(VkImageType type)
    {
        m_ImageInfo.imageType = type;

        return *this;
    }

    VyTexture2::Builder& 
    VyTexture2::Builder::setViewType(VkImageViewType type)
    {
        m_ViewInfo.viewType = type; 

        return *this; 
    }

    VyTexture2::Builder& 
    VyTexture2::Builder::setLevels(U32 levelCount)
    {
        m_ImageInfo.mipLevels                  = levelCount;
        m_ViewInfo.subresourceRange.levelCount = levelCount;

        return *this;
    }

    VyTexture2::Builder& 
    VyTexture2::Builder::setLevels(U32 baseMipLevel, U32 levelCount /*= 1*/)
    {
        m_ViewInfo.subresourceRange.baseMipLevel = baseMipLevel;
        m_ViewInfo.subresourceRange.levelCount   = levelCount;
        m_ImageInfo.mipLevels                    = levelCount;

        return *this; 
    }

    VyTexture2::Builder& 
    VyTexture2::Builder::setLevelCount(U32 levelCount)
    {
        m_ViewInfo.subresourceRange.levelCount = levelCount;
        m_ImageInfo.mipLevels                  = levelCount;

        return *this; 
    }

    VyTexture2::Builder& 
    VyTexture2::Builder::setBaseLevel(U32 baseMipLevel)
    {
        m_ViewInfo.subresourceRange.baseMipLevel = baseMipLevel;

        return *this; 
    }
    
    VyTexture2::Builder& 
    VyTexture2::Builder::setLayers(U32 layerCount)
    {
        m_ImageInfo.arrayLayers                = layerCount;
        m_ViewInfo.subresourceRange.layerCount = layerCount;

        return *this;
    }

    VyTexture2::Builder& 
    VyTexture2::Builder::setLayers(U32 baseArrayLayer, U32 layerCount /*= 1*/)
    {
        m_ViewInfo.subresourceRange.baseArrayLayer = baseArrayLayer;
        m_ViewInfo.subresourceRange.layerCount     = layerCount;
        m_ImageInfo.arrayLayers                    = layerCount;

        return *this; 
    }

    VyTexture2::Builder& 
    VyTexture2::Builder::setLayerCount(U32 layerCount)
    {
        m_ViewInfo.subresourceRange.layerCount = layerCount;
        m_ImageInfo.arrayLayers                = layerCount;

        return *this; 
    }

    VyTexture2::Builder& 
    VyTexture2::Builder::setBaseLayer(U32 baseArrayLayer)
    {
        m_ViewInfo.subresourceRange.baseArrayLayer = baseArrayLayer;

        return *this; 
    }

    VyTexture2::Builder& 
    VyTexture2::Builder::setFormat(VkFormat format)
    {
        m_ImageInfo.format = format;

        return *this;
    }
    
    VyTexture2::Builder& 
    VyTexture2::Builder::setLayout(VkImageLayout layout)
    {
        m_ImageInfo.initialLayout = layout;

        return *this;
    }
    
    VyTexture2::Builder& 
    VyTexture2::Builder::setTiling(VkImageTiling tiling)
    {
        m_ImageInfo.tiling = tiling;

        return *this;
    }
    
    VyTexture2::Builder& 
    VyTexture2::Builder::setSamples(VkSampleCountFlagBits sampleCount)
    {
        m_ImageInfo.samples = sampleCount;

        return *this;
    }
    
    VyTexture2::Builder& 
    VyTexture2::Builder::setSharing(VkSharingMode sharingMode)
    {
        m_ImageInfo.sharingMode = sharingMode;

        return *this;
    }
    
    VyTexture2::Builder& 
    VyTexture2::Builder::setUsage(VkImageUsageFlags flags)
    {
        m_ImageInfo.usage = flags;

        return *this;
    }

    VyTexture2::Builder& 
    VyTexture2::Builder::addUsage(VkImageUsageFlags flags)
    {
        m_ImageInfo.usage |= flags;

        return *this;
    }
    
    VyTexture2::Builder& 
    VyTexture2::Builder::setFlags(VkImageCreateFlags flags)
    {
        m_ImageInfo.flags = flags;

        return *this;
    }
    
    VyTexture2::Builder& 
    VyTexture2::Builder::setMemoryUsage(VmaMemoryUsage usage)
    {
        m_AllocInfo.usage = usage;

        return *this;
    }
    
    VyTexture2::Builder& 
    VyTexture2::Builder::setAllocFlags(VmaAllocationCreateFlags flags)
    {
        m_AllocInfo.flags = flags;

        return *this;
    }


    VyTexture2::Builder& 
    VyTexture2::Builder::setAspect(VkImageAspectFlags aspectMask)
    {
        m_ViewInfo.subresourceRange.aspectMask = aspectMask; 

        return *this; 
    }

    VyTexture2::Builder& 
    VyTexture2::Builder::addAspect(VkImageAspectFlags aspectMask)
    {
        m_ViewInfo.subresourceRange.aspectMask |= aspectMask; 

        return *this; 
    }

    VyTexture2::Builder& 
    VyTexture2::Builder::setComponents(VkComponentSwizzle swizzle)
    {
        m_ViewInfo.components = { swizzle };

        return *this; 
    }

    VyTexture2::Builder& 
    VyTexture2::Builder::setComponents(VkComponentSwizzle r, VkComponentSwizzle g, VkComponentSwizzle b, VkComponentSwizzle a)
    {
        m_ViewInfo.components.r = r;
        m_ViewInfo.components.g = g;
        m_ViewInfo.components.b = b;
        m_ViewInfo.components.a = a;

        return *this; 
    }

    VyTexture2::Builder& 
    VyTexture2::Builder::setMapping(VkComponentMapping mapping)
    {
        m_ViewInfo.components = mapping;

        return *this; 
    }

    VyTexture2::Builder& 
    VyTexture2::Builder::setSubresourceRange(VkImageSubresourceRange subRange)
    {
        m_ViewInfo.subresourceRange = subRange;

        return *this; 
    }


	VyTexture2::Builder& 
	VyTexture2::Builder::setFilters(VkFilter magFilter, VkFilter minFilter)
	{
		m_SamplerInfo.magFilter = magFilter; 
		m_SamplerInfo.minFilter = minFilter; 

		return *this; 
	}

	VyTexture2::Builder& 
	VyTexture2::Builder::setFilters(VkFilter filters)
	{
		m_SamplerInfo.magFilter = filters; 
		m_SamplerInfo.minFilter = filters; 

		return *this; 
	}

	VyTexture2::Builder& 
	VyTexture2::Builder::setWrap(VkSamplerAddressMode u, VkSamplerAddressMode v, VkSamplerAddressMode w)
	{
		m_SamplerInfo.addressModeU = u; 
		m_SamplerInfo.addressModeV = v;
		m_SamplerInfo.addressModeW = w;  

		return *this;
	}

	VyTexture2::Builder& 
	VyTexture2::Builder::setWrap(VkSamplerAddressMode mode)
	{
		m_SamplerInfo.addressModeU = mode; 
		m_SamplerInfo.addressModeV = mode;
		m_SamplerInfo.addressModeW = mode;  

		return *this; 
	}

	VyTexture2::Builder& 
	VyTexture2::Builder::enableAnisotropy(bool enable, float maxAnisotropy /* = -1.0f*/)
	{
		m_SamplerInfo.anisotropyEnable = enable;
		m_SamplerInfo.maxAnisotropy = maxAnisotropy;


		return *this; 
	}

	VyTexture2::Builder& 
	VyTexture2::Builder::setBorder(VkBorderColor color)
	{
		m_SamplerInfo.borderColor = color; 

		return *this; 
	}

	VyTexture2::Builder& 
	VyTexture2::Builder::enableCompare(VkCompareOp op)
	{
		m_SamplerInfo.compareEnable = true; 
		m_SamplerInfo.compareOp     = op; 

		return *this; 
	}

	VyTexture2::Builder& 
	VyTexture2::Builder::setMipmapMode(VkSamplerMipmapMode mode)
	{
		m_SamplerInfo.mipmapMode = mode; 

		return *this; 
	}

	VyTexture2::Builder& 
	VyTexture2::Builder::setMipLodBias(float bias)
	{
		m_SamplerInfo.mipLodBias = bias; 

		return *this; 
	}

	VyTexture2::Builder& 
	VyTexture2::Builder::setLodRange(float minLod, float maxLod)
	{
		m_SamplerInfo.minLod = minLod; 
		m_SamplerInfo.maxLod = maxLod; 

		return *this; 
	}
}