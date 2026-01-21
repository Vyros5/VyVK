#pragma once

#include <VyEngine/VK/Buffer/Buffer.h>
#include <VyEngine/GFX/Resources/Texture/Texture.h>
#include <VyEngine/VK/Descriptors/Descriptors.h>

namespace Vy
{
    class LightBuffer 
    {
    public:

        static constexpr VkFormat HDR_FORMAT = VK_FORMAT_R32G32B32A32_SFLOAT; 

        void create(U32 width, U32 height); 
        void cleanup();

        VkImageView imageView() const { return m_Image->imageViewHandle(); }
        VkImage     image()     const { return m_Image->imageHandle(); }
        VkSampler   sampler()   const { return m_Image->samplerHandle(); }

        VkImageLayout Layout{ VK_IMAGE_LAYOUT_UNDEFINED };

		U32         width()  const { return m_Width;   }
		U32         height() const { return m_Height;  }
        VkExtent2D  extent() const { return { m_Width, m_Height }; }

    private:
        Unique<VyTexture> m_Image;
        U32 m_Width  = 0;
        U32 m_Height = 0;
    };
}