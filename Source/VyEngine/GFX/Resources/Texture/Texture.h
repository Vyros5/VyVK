#pragma once

#include <VyEngine/VK/Device/Device.h>
#include <VyEngine/VK/Buffer/Buffer.h>

#include <VyEngine/VK/Image/Image.h>
#include <VyEngine/VK/Image/ImageView.h>
#include <VyEngine/VK/Image/Sampler.h>

namespace Vy
{
    class VyTexture
    {
    public:
        static constexpr bool USE_SRGB  = true;
        static constexpr bool USE_UNORM = false;

        VyTexture(
            const TString& filepath, 
            bool           bSRGB   = true, 
            bool           bFlipY  = false
        );

        VyTexture(
            const TString& filepath, 
            VkFormat       format 
        );

        //ctor for render target / G buffer attachments
        VyTexture(
            U32                width,
            U32                height,
            VkFormat           format,
            VkImageUsageFlags  usage,
            VkImageAspectFlags aspectMask,
            bool               bCreateSampler = true
        );

        VyTexture(const void* data, size_t size, bool bSRGB = true);


        ~VyTexture();

        VyTexture(const VyTexture&)            = delete;
        VyTexture& operator=(const VyTexture&) = delete;

        VyTexture(VyTexture&&)                 = delete;
        VyTexture& operator=(VyTexture&&)      = delete;

        // Create simple single-color textures (1x1 pixel)
        static Shared<VyTexture> createWhiteTexture();
        static Shared<VyTexture> createNormalTexture(); // Flat normal (0.5, 0.5, 1.0)

        static Shared<VyTexture> createFromFilepath(const TString& filepath, bool bSRGB = true)
        {
            return MakeShared<VyTexture>( filepath, bSRGB );
        }

        static Unique<VyTexture> createFromFile(const TString& filepath, bool bSRGB = true)
        {
            return MakeUnique<VyTexture>( filepath, bSRGB );
        }

        static Unique<VyTexture> createFromFile(const TString& filepath, VkFormat format)
        {
            return MakeUnique<VyTexture>( filepath, format );
        }

        static Unique<VyTexture> createFromMemory(const void* data, size_t size, bool bSRGB = true)
        {
            return MakeUnique<VyTexture>( data, size, bSRGB );
        }

        static void bind(VkCommandBuffer& cmdBuffer, VkPipelineLayout& pipelineLayout);

        static void initBindless(U32 maxTextures);
        static void cleanupBindless(); 
        static void updateBindless(void* pData);

    	static VkDescriptorSetLayout s_BindlessSetLayout;
        static VkDescriptorPool      s_BindlessPool;
        static VkDescriptorSet       s_BindlessDescriptorSet;


        VY_NODISCARD       VkImage      imageHandle()           { return m_Image  .handle(); }
        VY_NODISCARD       VkImage      imageHandle()     const { return m_Image  .handle(); }
        VY_NODISCARD const VyImage&     image()           const { return m_Image; }
        
        VY_NODISCARD       VkImageView  imageViewHandle()       { return m_View   .handle(); }
        VY_NODISCARD       VkImageView  imageViewHandle() const { return m_View   .handle(); }
        VY_NODISCARD const VyImageView& imageView()       const { return m_View; }

        VY_NODISCARD       VkSampler    samplerHandle()         { return m_Sampler.handle(); }
        VY_NODISCARD       VkSampler    samplerHandle()   const { return m_Sampler.handle(); }
        VY_NODISCARD const VySampler&   sampler()         const { return m_Sampler; }

        VY_NODISCARD int                width()           const { return m_Width;     }
        VY_NODISCARD int                height()          const { return m_Height;    }
        VY_NODISCARD int                mipLevels()       const { return m_MipLevels; }
        VY_NODISCARD VkFormat           format()          const { return m_Image.format(); }
        VY_NODISCARD const TString&     filepath()        const { return m_Filepath; }

        VY_NODISCARD VkDescriptorImageInfo descriptorImageInfo() const
        {
            return VkDescriptorImageInfo{
                .sampler     = m_Sampler.handle(),
                .imageView   = m_View   .handle(),
                .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            };
        }

        VY_NODISCARD VkDescriptorImageInfo descriptorImageInfo(VkImageLayout layoutOverride) const
        {
            return VkDescriptorImageInfo{
                .sampler     = m_Sampler.handle(),
                .imageView   = m_View   .handle(),
                .imageLayout = layoutOverride,
            };
        }

        void setGlobalIndex(U32 index) { m_GlobalIndex = index; }
        U32  getGlobalIndex()    const { return m_GlobalIndex; }

        void transitionImageLayout(VkCommandBuffer cmdBuffer, VkImageLayout newLayout);

        /**
         * @brief Get approximate memory size of this texture
         * @return Memory size in bytes (includes mipmaps)
         */
        size_t memorySize() const;

        // Private constructor for creating textures from memory
        VyTexture(const unsigned char* pixels, int width, int height, VkFormat format);
    private:
    
        void createImageView(VkFormat format);

        void createSampler();
        
        VyImage     m_Image;
        VyImageView m_View;
        VySampler   m_Sampler;

        int m_Width       = 0;
        int m_Height      = 0;
        U32 m_MipLevels   = 1;
        U32 m_GlobalIndex = 0;

        TString m_Filepath;
    };
}