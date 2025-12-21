#pragma once

#include <Vy/GFX/Backend/Device.h>
#include <Vy/GFX/Backend/Buffer/Buffer.h>

#include <Vy/GFX/Backend/Image/Image.h>
#include <Vy/GFX/Backend/Image/ImageView.h>
#include <Vy/GFX/Backend/Image/Sampler.h>

#include <Vy/GFX/Context.h>

namespace Vy
{
    class VySampledTexture
    {
    public:
        VySampledTexture(
            const String& filepath, 
            bool          bSRGB   = true, 
            bool          bFlipY  = false
        );

        ~VySampledTexture();

        VySampledTexture(const VySampledTexture&)            = delete;
        VySampledTexture& operator=(const VySampledTexture&) = delete;

        VySampledTexture(VySampledTexture&&)                 = delete;
        VySampledTexture& operator=(VySampledTexture&&)      = delete;

        // Create simple single-color textures (1x1 pixel)
        static Shared<VySampledTexture> createWhiteTexture();
        static Shared<VySampledTexture> createNormalTexture(); // Flat normal (0.5, 0.5, 1.0)

        VkImage     image()     const { return m_Image  .handle(); }
        VkImageView imageView() const { return m_View   .handle(); }
        VkSampler   sampler()   const { return m_Sampler.handle(); }

        VkDescriptorImageInfo descriptorImageInfo() const
        {
            return VkDescriptorImageInfo{
                .sampler     = m_Sampler.handle(),
                .imageView   = m_View   .handle(),
                .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            };
        }

        int width()     const { return m_Width;     }
        int height()    const { return m_Height;    }
        int mipLevels() const { return m_MipLevels; }

        // void setGlobalIndex(U32 index) { m_GlobalIndex = index; }
        // U32 globalIndex() const { return m_GlobalIndex; }

        /**
         * @brief Get approximate memory size of this texture
         * @return Memory size in bytes (includes mipmaps)
         */
        size_t memorySize() const;

        // Private constructor for creating textures from memory
        VySampledTexture(const unsigned char* pixels, int width, int height, VkFormat format);

    private:

        void createImageView(VkFormat format);

        void createSampler();
        
        VyImage     m_Image;
        VyImageView m_View;
        VySampler   m_Sampler;

        int m_Width       = 0;
        int m_Height      = 0;
        U32 m_MipLevels   = 1;
        // U32 m_GlobalIndex = 0;
    };
}