#pragma once

#include <VyEngine/VK/Device/Device.h>
#include <VyEngine/VK/Buffer/Buffer.h>

#include <VyEngine/VK/Image/Image.h>
#include <VyEngine/VK/Image/ImageView.h>
#include <VyEngine/VK/Image/Sampler.h>

namespace Vy
{
    // class TextureSource
    // {
    //     friend class VyTexture;

    // public:
    //     virtual U32 width()  { return m_Width;  }
    //     virtual U32 height() { return m_Height; }
    //     virtual U32 bpp()    { return m_BPP;    }

    // protected:
    //     TextureSource() {}

    // protected:
    //     TVector<U8> m_Pixels;
    //     U32         m_Width;
    //     U32         m_Height;
    //     U32         m_BPP;
    // };

    // class FileTextureSource : public TextureSource
    // {
    // public:
    //     FileTextureSource(const TString& filepath);
    // };

    // class SolidTextureSource : public TextureSource
    // {
    // public:
    //     SolidTextureSource(Vec4 color, U32 width = 1, U32 height = 1);
    // };

    // class FloatFileTextureSource : public TextureSource
    // {
    // public:
    //     FloatFileTextureSource(const TString& filepath);
    // };

    // class FloatSolidTextureSource : public TextureSource
    // {
    // public:
    //     FloatSolidTextureSource(Vec4 color, U32 width = 1, U32 height = 1);
    // };


    struct VyTextureConfig
    {
        TString Name{ "texture" };

        static VyTextureConfig texture2D(U32 width, U32 height, VkFormat format);
        static VyTextureConfig cubeMap(U32 size, VkFormat format);

        VyImageInfo     Image;
        VyImageViewInfo View;
        VySamplerInfo   Sampler;
    };

    class VyTexture
    {
    public:

        class Builder;
        friend Builder;

        /**
         * @brief Loads image data using STB library.
         * 
         * @param path           Path to the image file
         * @param width          Width of the loaded image to be assigned
         * @param height         Height of the loaded image to be assigned
         * @param componentCount Required number of components to be read
         * 
         * @note Read 'componentCount' pixel data, should be freed after use.
         */
        static U8* loadImage(const TString& path, int& width, int& height, int componentCount = 4);
        static float* loadImageFloat(const TString& path, int& width, int& height, int componentCount = 4);

        // @brief 

        /**
         * @brief Frees loaded data using STB library.
         * 
         * @param pData Data to be freed.
         */
        static void freeImageData(U8* pData);
        static void freeImageData(float* pData);

        VyTexture() = default;

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
        // VyTexture(
        //     U32                width,
        //     U32                height,
        //     VkFormat           format,
        //     VkImageUsageFlags  usage,
        //     VkImageAspectFlags aspectMask,
            
        //     bool               bCreateSampler = true
        // );

        //-----------------------------------------------------------------------------------------
        // CTor for creating from config.

        VyTexture(const VyTextureConfig& config);

        //-----------------------------------------------------------------------------------------
        // CTors for creating from memory.

        VyTexture(const void* pData, size_t size, bool bSRGB = true, bool bMipmapped = true);

        VyTexture(const void* pData, U32 width, U32 height, bool bSRGB = true, bool bMipmapped = true);

        //-----------------------------------------------------------------------------------------

        ~VyTexture();

        VyTexture(const VyTexture&)            = delete;
        VyTexture& operator=(const VyTexture&) = delete;

        VyTexture(VyTexture&&)                 = delete;
        VyTexture& operator=(VyTexture&&)      = delete;

        // Create simple single-color textures (1x1 pixel)
        static Shared<VyTexture> createWhiteTexture();
        static Shared<VyTexture> createNormalTexture(); // Flat normal (0.5, 0.5, 1.0)

		static Shared<VyTexture> loadFromFile(const TPath& file, VkFormat format = VK_FORMAT_R8G8B8A8_UNORM);
		static Shared<VyTexture> loadTexture2D(const TPath& file, VkFormat format);
		static Shared<VyTexture> loadCubemap(const TPath& file);

        static Shared<VyTexture> loadFromMemory(const std::byte* pData, size_t size, VkFormat format);

        static Shared<VyTexture> createSolidColor(Vec4 color);
        static Shared<VyTexture> createSolidColorCube(Vec4 color);


        static Shared<VyTexture> createFromFilepath(const TString& filepath, bool bSRGB = true)
        {
            return MakeShared<VyTexture>( filepath, bSRGB );
        }

        //-----------------------------------------------------------------------------------------

        static Unique<VyTexture> createFromFile(const TString& filepath, bool bSRGB = true)
        {
            return MakeUnique<VyTexture>( filepath, bSRGB );
        }

        static Unique<VyTexture> createFromFile(const TString& filepath, VkFormat format)
        {
            return MakeUnique<VyTexture>( filepath, format );
        }

        //-----------------------------------------------------------------------------------------

        static Unique<VyTexture> createFromMemory(const void* pData, size_t size, bool bSRGB = true, bool bMipmapped = true)
        {
            return MakeUnique<VyTexture>( pData, size, bSRGB, bMipmapped );
        }

        static Unique<VyTexture> createFromMemory(const void* pData, U32 width, U32 height, bool bSRGB = true, bool bMipmapped = true)
        {
            return MakeUnique<VyTexture>( pData, width, height, bSRGB, bMipmapped );
        }

        //-----------------------------------------------------------------------------------------

        static Unique<VyTexture> createFromConfig(const VyTextureConfig& config)
        {
            return MakeUnique<VyTexture>( config );
        }

        //-----------------------------------------------------------------------------------------

		static Shared<VyTexture> irradianceMap(const Shared<VyTexture>& skybox);
		static Shared<VyTexture> prefilteredMap(const Shared<VyTexture>& skybox);
		static Shared<VyTexture> BRDFLUT();

        VY_NODISCARD       VkImage      imageHandle()           { return m_Image  .handle(); }
        VY_NODISCARD       VkImage      imageHandle()     const { return m_Image  .handle(); }
        VY_NODISCARD       VyImage&     image()                 { return m_Image; }
        VY_NODISCARD const VyImage&     image()           const { return m_Image; }
        
        VY_NODISCARD       VkImageView  imageViewHandle()       { return m_View   .handle(); }
        VY_NODISCARD       VkImageView  imageViewHandle() const { return m_View   .handle(); }
        VY_NODISCARD       VyImageView& imageView()             { return m_View; }
        VY_NODISCARD const VyImageView& imageView()       const { return m_View; }

        VY_NODISCARD       VkSampler    samplerHandle()         { return m_Sampler.handle(); }
        VY_NODISCARD       VkSampler    samplerHandle()   const { return m_Sampler.handle(); }
        VY_NODISCARD       VySampler&   sampler()               { return m_Sampler; }
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


    class VyTexture2
    {
    public:

        class Builder;
        friend Builder;

        VyTexture2(
            const TString&                 name, 
            const VkImageCreateInfo&       imageInfo, 
            const VmaAllocationCreateInfo& allocInfo,
            const VkImageViewCreateInfo&   viewInfo,
            const VkSamplerCreateInfo&     samplerInfo
        );

        ~VyTexture2();

        VyTexture2(const VyTexture2&)            = delete;
        VyTexture2& operator=(const VyTexture2&) = delete;

        VyTexture2(VyTexture2&&)                 = delete;
        VyTexture2& operator=(VyTexture2&&)      = delete;

	private:

		void destroy();

		void swap(VyImage& other);

		VkImage                 m_Image        { VK_NULL_HANDLE };
		VkImageCreateInfo       m_ImageInfo    {};
        
		VmaAllocation           m_ImageMemory  { VK_NULL_HANDLE };
		VmaAllocationCreateInfo m_AllocInfo    {};
        
		VkImageView             m_ImageView    { VK_NULL_HANDLE };
        VkImageViewCreateInfo   m_ViewInfo     {};

        VkSampler               m_Sampler      { VK_NULL_HANDLE };
		VkSamplerCreateInfo     m_SamplerInfo  {};

		VkImageLayout           m_CurrentLayout{ VK_IMAGE_LAYOUT_UNDEFINED };
		TString                 m_DebugName    { "unnamed" };
    };


    /**
     * @brief Builder for Texture
     */
    class VyTexture2::Builder 
    {
    public:
		Builder();

		Builder& setName(const TString& name);
		
        // ----------------------------------------------------------------------------------------
        // SIZE

		Builder& setWidth(U32  width);
		Builder& setHeight(U32 height);
		Builder& setDepth(U32  depth);

        Builder& setExtent2D(U32 extent);
		Builder& setExtent(VkExtent2D extent);
		Builder& setExtent(VkExtent3D extent);
		Builder& setExtent(U32 width, U32 height, U32 depth = 1);
		
        // ----------------------------------------------------------------------------------------
        // TYPES

		Builder& setImageType(VkImageType type);
        Builder& setViewType(VkImageViewType type);

        // ----------------------------------------------------------------------------------------
        // MIP / LAYER

		Builder& setLevels(U32 levelCount);
        Builder& setLevels(U32 baseMipLevel, U32 levelCount);
        Builder& setLevelCount(U32 levelCount);
        Builder& setBaseLevel(U32 baseMipLevel);
        
		Builder& setLayers(U32 layerCount);
        Builder& setLayers(U32 baseArrayLayer, U32 layerCount);
		Builder& setLayerCount(U32 layerCount);
		Builder& setBaseLayer(U32 baseArrayLayer);

        // ----------------------------------------------------------------------------------------
        // PROPERTIES

		Builder& setFormat(VkFormat format);
		Builder& setLayout(VkImageLayout layout);
		Builder& setTiling(VkImageTiling tiling);
		Builder& setSamples(VkSampleCountFlagBits sampleCount);
		Builder& setSharing(VkSharingMode sharingMode);
        
        // ----------------------------------------------------------------------------------------
        // FLAGS

		Builder& setUsage(VkImageUsageFlags flags);
		Builder& addUsage(VkImageUsageFlags flags);
		Builder& setFlags(VkImageCreateFlags flags);
		
		Builder& setMemoryUsage(VmaMemoryUsage usage);
		Builder& setAllocFlags(VmaAllocationCreateFlags flags);

        // ----------------------------------------------------------------------------------------
        // IMAGE VIEW SPECIFIC

		Builder& setAspect(VkImageAspectFlags aspectMask);
		Builder& addAspect(VkImageAspectFlags aspectMask);
		Builder& setComponents(VkComponentSwizzle swizzle);
		Builder& setComponents(VkComponentSwizzle r, VkComponentSwizzle g, VkComponentSwizzle b, VkComponentSwizzle a);
		Builder& setMapping(VkComponentMapping mapping);

		Builder& setSubresourceRange(VkImageSubresourceRange subRange);
		
        // ----------------------------------------------------------------------------------------
        // SAMPLER SPECIFIC

		Builder& setFilters(VkFilter filters);
		Builder& setFilters(VkFilter magFilter, VkFilter minFilter);
		Builder& setWrap(VkSamplerAddressMode mode);
		Builder& setWrap(VkSamplerAddressMode u, VkSamplerAddressMode v, VkSamplerAddressMode w);
		Builder& enableAnisotropy(bool enable, float maxAnisotropy = -1.0f);
		Builder& setBorder(VkBorderColor color);
		Builder& enableCompare(VkCompareOp op);
		Builder& setMipmapMode(VkSamplerMipmapMode mode);
		Builder& setMipLodBias(float bias);
		Builder& setLodRange(float minLod, float maxLod);

        // ----------------------------------------------------------------------------------------

        /// Enable mipmap generation (default: false)
        // Builder& isMipmapped(bool enable = true);

        /// Use sRGB format for gamma-correct rendering (default: false)
        // Builder& isSRGB(bool enable = true);

        VyTexture2         build();
        Unique<VyTexture2> buildPtr();

    private:
        friend class VyTexture2;

		TString               m_Name{ "unnamed" };

		VkImageCreateInfo       m_ImageInfo  {};
		VmaAllocationCreateInfo m_AllocInfo  {};
		VkImageViewCreateInfo   m_ViewInfo   {};
        VkSamplerCreateInfo     m_SamplerInfo{};

        // Options
        bool m_GenerateMipmaps = false;
        bool m_SRGB            = false;
    };
}