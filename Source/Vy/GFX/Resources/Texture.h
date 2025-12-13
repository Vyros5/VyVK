#pragma once

#include <Vy/Asset/Asset.h>

#include <Vy/GFX/Backend/Device.h>
#include <Vy/GFX/Backend/Resources/Buffer.h>

#include <Vy/GFX/Backend/Resources/Image.h>
#include <Vy/GFX/Backend/Resources/ImageView.h>
#include <Vy/GFX/Backend/Resources/Sampler.h>

#include <Vy/GFX/Context.h>

namespace Vy
{
    class VySampledTexture
    {
    public:
        VySampledTexture(const String& filepath, bool srgb = true, bool flipY = false);

        ~VySampledTexture();

        VySampledTexture(const VySampledTexture&)            = delete;
        VySampledTexture& operator=(const VySampledTexture&) = delete;

        VySampledTexture(VySampledTexture&&)                 = delete;
        VySampledTexture& operator=(VySampledTexture&&)      = delete;

        // Create simple single-color textures (1x1 pixel)
        static Shared<VySampledTexture> createWhiteTexture();
        static Shared<VySampledTexture> createNormalTexture(); // Flat normal (0.5, 0.5, 1.0)

        VkImage     image()     const { return m_Image.handle();   }
        VkImageView imageView() const { return m_View.handle();    }
        VkSampler   sampler()   const { return m_Sampler.handle(); }

        VkDescriptorImageInfo descriptorImageInfo() const
        {
            return VkDescriptorImageInfo{
                    .sampler     = m_Sampler,
                    .imageView   = m_View,
                    .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            };
        }

        int width()     const { return m_Width; }
        int height()    const { return m_Height; }
        int mipLevels() const { return m_MipLevels; }

        void setGlobalIndex(U32 index) { m_GlobalIndex = index; }
        U32 globalIndex() const { return m_GlobalIndex; }

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
        void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, U32 mipLevels);
        void copyBufferToImage(VkBuffer buffer, VkImage image, U32 width, U32 height);
        void generateMipmaps(VkImage image, VkFormat format, I32 width, I32 height, U32 mipLevels);

        VyImage     m_Image;
        VyImageView m_View;
        VySampler   m_Sampler;

        int m_Width       = 0;
        int m_Height      = 0;
        U32 m_MipLevels   = 1;
        U32 m_GlobalIndex = 0;
    };
}



namespace Vy
{
//     struct VyTextureCreateInfo
//     {
//         static VyTextureCreateInfo texture2D(U32 width, U32 height, VkFormat format);
//         static VyTextureCreateInfo cubemap  (U32 width, U32 height, VkFormat format);

//         VyImageCreateInfo     Image;
//         VyImageViewCreateInfo View;
//         VySamplerCreateInfo   Sampler;
//     };

//     class Texture
//     {
//     public:
//         class Builder
//         {
//         public:
//             explicit Builder() = default;

//         };
//     public:
// 		static constexpr int NUMBER_OF_CUBEMAP_IMAGES = 6;

//         static Shared<Texture> load(const Path& file, VkFormat format = VK_FORMAT_R8G8B8A8_UNORM);
// 		// static Shared<Texture> load(const TVector<Path>& files, bool flip = true);

// 		// static Shared<Texture> loadCubemap(const TVector<Path>& files, bool flip = true);
		
// 		static Shared<Texture> loadTexture2D(const Path& file, VkFormat format);

//     public:
    
// 		// Texture() = default;

//         Texture(const VyTextureCreateInfo& desc);

// 		Texture(const Texture&) = delete;
// 		Texture(Texture&&) noexcept;

// 		~Texture();

// 		Texture& operator=(const Texture&) = delete;
// 		Texture& operator=(Texture&&) noexcept;

// 		VY_NODISCARD       VyImage&     image()          { return m_Image;   }
// 		VY_NODISCARD const VyImage&     image()    const { return m_Image;   }

// 		VY_NODISCARD       VyImageView& view()           { return m_View;    }
// 		VY_NODISCARD const VyImageView& view()     const { return m_View;    }

// 		VY_NODISCARD       VySampler&   sampler()        { return m_Sampler; }
// 		VY_NODISCARD const VySampler&   sampler()  const { return m_Sampler; }

// 		VY_NODISCARD       VkExtent3D   extent()   const { return m_Image.extent(); }
// 		VY_NODISCARD       VkExtent2D   extent2D() const { return VkExtent2D{ m_Image.width(), m_Image.height() }; }

// 		VY_NODISCARD VkDescriptorImageInfo descriptorImageInfo() const
// 		{
// 			return descriptorImageInfo(m_Image.layout());
// 		}

// 		VY_NODISCARD VkDescriptorImageInfo descriptorImageInfo(VkImageLayout layoutOverride) const
// 		{
// 			VkDescriptorImageInfo info{};
// 			{
// 				info.sampler     = m_Sampler.handle();
// 				info.imageView   = m_View.handle();
// 				info.imageLayout = layoutOverride;
// 			}

// 			return info;
// 		}

// 		void resize(VkExtent3D newSize, VkImageUsageFlags usage);

// // 		// static VyAssetType getStaticType()               { return VyAssetType::Image; }
// // 		// VyAssetType        getAssetType() const override { return getStaticType(); }

//     private:
// 		void destroy();

// 		VyImage     m_Image;
// 		VyImageView m_View;
// 		VySampler   m_Sampler;

//         Unique<VyDescriptorPool>      m_DescriptorPool;
//         Unique<VyDescriptorSetLayout> m_DescriptorSetLayout;
//         VkDescriptorSet               m_DescriptorSet = VK_NULL_HANDLE;
//     };


    class VyTexture 
	{
    public:
        class Builder 
		{
        public:
            explicit Builder() = default;

            Builder& size(U32 width, U32 height) 
			{
                m_Width  = width;
                m_Height = height;
                return *this;
            }

            Builder& format(VkFormat fmt) 
			{
                m_Format = fmt;
                return *this;
            }

            Builder& usage(VkImageUsageFlags use) 
			{
                m_Usage = use;
                return *this;
            }

            Builder& sampleCount(VkSampleCountFlagBits samples)
			{
                m_Samples = samples;
                return *this;
            }

            Builder& filepath(const String& path) 
			{
                m_Filepath = path;
                return *this;
            }

            Builder& asCubemap() 
			{
                m_UseCubemap = true;
                return *this;
            }

            Builder& asHDR()
			{
                m_UseHDR = true;
                return *this;
            }

            VY_INLINE Shared<VyTexture> build() const 
			{
                if (m_UseCubemap && m_UseHDR) 
				{
                    throw std::runtime_error("Cannot build a texture as both HDR and Cubemap. Choose only one.");
                }

                auto texture = MakeShared<VyTexture>();

                if (!m_Filepath.empty()) 
				{
                    texture->createTexture(m_Filepath, m_Format);
                }
				else 
				{
                    if (m_UseCubemap) 
					{
                        texture->createCubeMap(m_Width, m_Height, m_Format, m_Usage);
                    } 
                    else if (m_UseHDR) 
					{
                        if (!m_Filepath.empty())
						{
							texture->createHDRTexture(m_Filepath);
						}
                        else
						{
							throw std::runtime_error("HDR texture requires a filepath.");
						}
                    } 
                    else 
					{
                        texture->createTexture(m_Width, m_Height, m_Format, m_Usage, m_Samples);
                    }
                }

                return texture;
            }

        private:
            bool                  m_UseCubemap = false;
            bool                  m_UseHDR     = false;
            U32                   m_Width      = 512;
            U32                   m_Height     = 512;
            VkFormat              m_Format     = VK_FORMAT_R8G8B8A8_UNORM;
            VkImageUsageFlags     m_Usage      = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
            VkSampleCountFlagBits m_Samples    = VK_SAMPLE_COUNT_1_BIT;
            String                m_Filepath;
        };

        enum class TextureType 
		{
            Albedo,
            Normal,
            Emissive,
            AO,
            MetallicRoughness,
            Cubemap
        };

	public:

        VY_INLINE static Shared<VyTexture> create() 
		{
            return MakeShared<VyTexture>();
        }

        void generateMipmap(
            VkImage  image, 
            VkFormat format, 
            I32       width, 
            I32       height, 
            U32       mipLevels
        );

        void createHDRTexture(
            const String& filepath, 
            VkFormat      format = VK_FORMAT_R32G32B32A32_SFLOAT
        );

        void createCubeMap(const TArray<String, 6>& paths);

        void createCubeMap(
            U32               width, 
            U32               height, 
            VkFormat          format, 
            VkImageUsageFlags usage
        );
        
        void createMipMappedCubemap(
            U32               width, 
            U32               height, 
            VkFormat          format, 
            VkImageUsageFlags usage
        );
        
        void createTexture(
            const String& filepath, 
            VkFormat      format = VK_FORMAT_R8G8B8A8_UNORM
        );

        void createTexture(
            U32                   width, 
            U32                   height, 
            VkFormat              format  = VK_FORMAT_R8G8B8A8_UNORM,
            VkImageUsageFlags     usage   = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT
        );
        
        void UseFallbackTextures(TextureType type);

        // Inline Getters
        VY_INLINE VkImage         textureImage()       const { return m_TextureImage; }
        VY_INLINE VkImageView     textureImageView()   const { return m_TextureView; }
        VY_INLINE VkSampler       textureSampler()     const { return m_TextureSampler; }
        VY_INLINE VmaAllocation   textureImageMemory() const { return m_TextureAllocation; }
        VY_INLINE VkDescriptorSet descriptorSet()      const { return m_DescriptorSet; }
        VY_INLINE U32             mipLevels()          const { return m_MipLevels; }
        VY_INLINE VkFormat        textureFormat()      const { return m_Format; }

        VY_INLINE bool hasImage()      const { return m_TextureImage      != VK_NULL_HANDLE; }
        VY_INLINE bool hasView()       const { return m_TextureView       != VK_NULL_HANDLE; }
        VY_INLINE bool hasSampler()    const { return m_TextureSampler    != VK_NULL_HANDLE; }
        VY_INLINE bool hasAllocation() const { return m_TextureAllocation != VK_NULL_HANDLE; }

        VY_INLINE VkDescriptorImageInfo descriptorImageInfo() const;

        void destroyAll();
        void destroy();

        void createImage(
			U32               width, 
			U32               height, 
			VkFormat          format, 
			VkImageTiling     tiling,
			VkImageUsageFlags usage, 
			VkImage&          image, 
			VmaAllocation&    allocation
		);

        // Rule of five
        VyTexture();
        ~VyTexture();

        VyTexture(const VyTexture&)            = delete;
        VyTexture& operator=(const VyTexture&) = delete;

        VyTexture(VyTexture&&) noexcept            = default;
        VyTexture& operator=(VyTexture&&) noexcept = delete;

    private:

        // HDR
        void createHDRTextureImage(const String& filepath);
        void createHDRTextureImageView(VkFormat format);
        void createHDRTextureSampler();

        // Cubemap
        void createCubemapImage(U32 width, U32 height, VkFormat format, VkImageUsageFlags usage);
        void createCubemapImageView(VkFormat format);
        void createCubemapSampler();
        void createSolidColorCubemap(Vec4 color);

        // LDR
        void createTextureImage(const String& filepath);
        void createTextureImage(U32 width, U32 height, VkFormat format, VkImageUsageFlags usage, VkSampleCountFlagBits samples);
        void createTextureImageView(VkFormat format);
        void createTextureSampler();
        void copyBufferToImage(VkBuffer buffer, VkImage image, U32 width, U32 height) const;
        void writeToDescriptorSet();
        void createSolidColorTexture(Vec4 color);

        void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, U32 layers = 1);


        U32 m_MipLevels = 1;
        U32 m_Size = 0;

        VkFormat m_Format = VK_FORMAT_R8G8B8A8_UNORM;

        VkImage       m_TextureImage      = VK_NULL_HANDLE;
        VkSampler     m_TextureSampler    = VK_NULL_HANDLE;
        VkImageView   m_TextureView       = VK_NULL_HANDLE;
        VmaAllocation m_TextureAllocation = VK_NULL_HANDLE;

        Unique<VyDescriptorPool>      m_DescriptorPool;
        Unique<VyDescriptorSetLayout> m_DescriptorSetLayout;
        VkDescriptorSet               m_DescriptorSet = VK_NULL_HANDLE;
    };
}


// namespace Vy
// {
// 	enum class TextureMode
// 	{
// 		Texture2D,
// 		CubeMap
// 	};

// 	struct TextureCreateInfo
// 	{
// 		VkExtent2D         Extent      { 1, 1 };
// 		VkFormat           Format      { VK_FORMAT_R8G8B8A8_UNORM };
// 		VkImageUsageFlags  UsageFlags  { VK_IMAGE_USAGE_TRANSFER_DST_BIT };
// 		VkImageAspectFlags AspectFlags { VK_IMAGE_ASPECT_COLOR_BIT };
// 		TextureMode        Mode        { TextureMode::Texture2D };
// 		void*              Data        { nullptr };
// 	};


// 	class Texture final : public VyAsset
// 	{
// 	public:
// 		class Builder
// 		{
// 		public:
// 			explicit Builder() = default;

// 			Builder& assetHandle(VyAssetHandle assetHandle) 
// 			{ 
// 				m_AssetHandle = std::move(assetHandle);

// 				return *this; 
// 			}
			
// 			Builder& textureMode(TextureMode textureMode) 
// 			{ 
// 				m_TextureCreateInfo.Mode = textureMode;

// 				return *this; 
// 			}
			
// 			Builder& format(VkFormat format) 
// 			{ 
// 				m_TextureCreateInfo.Format = format;

// 				return *this; 
// 			}
			
// 			Builder& extent(const VkExtent2D& size) 
// 			{ 
// 				m_TextureCreateInfo.Extent = size;

// 				return *this; 
// 			}
			
// 			Builder& usageFlags(VkImageUsageFlags usageFlags) 
// 			{ 
// 				m_TextureCreateInfo.UsageFlags = usageFlags;

// 				return *this; 
// 			}
			
// 			Builder& aspectFlags(VkImageAspectFlags aspectFlags) 
// 			{ 
// 				m_TextureCreateInfo.AspectFlags = aspectFlags;

// 				return *this; 
// 			}
			
// 			Builder& data(void* data)
// 			{ 
// 				m_TextureCreateInfo.Data = data;

// 				return *this; 
// 			}
			
// 			Unique<Texture> buildUnique() const;

// 		private:
// 			VyAssetHandle     m_AssetHandle = VyAssetHandle::Invalid();
// 			TextureCreateInfo m_TextureCreateInfo{};
// 		};

// 	public:
// 		void reload() override;

// 		bool checkForDirtyInDependencies() override { return false; }

// 		Texture(const Texture& other)            = delete;
// 		Texture& operator=(const Texture& other) = delete;

// 		Texture(Texture&& other) noexcept;
// 		Texture& operator=(Texture&& other) noexcept;
		
// 		Texture(TextureCreateInfo info, VyAssetHandle assetHandle = VyAssetHandle::Invalid());

// 		~Texture() override;

// 		U32 width()  const { return m_CreateInfo.Extent.width;  }
// 		U32 height() const { return m_CreateInfo.Extent.height; }

// 		VkSampler   sampler() const { return m_Sampler;   }
// 		VkImageView view()    const { return m_ImageView; }

// 		VkDescriptorImageInfo descriptorImageInfo() const;

// 	private:

// 		void initFromPath();
		
// 		void destroy();

// 		void createImage(VkFormat format, VkImageUsageFlags usageFlags, VkImageAspectFlags aspectFlags);
		
// 		void transitionImageLayout(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, VkImageLayout oldLayout, VkImageLayout newLayout) const;
		
// 		void createImageView(VkFormat format, VkImageAspectFlags aspectFlags);
		
// 		void createTextureSampler();

// 		TextureCreateInfo m_CreateInfo{};

// 		VkImage       m_Image{};
// 		VmaAllocation m_Allocation{};
// 		VkImageView   m_ImageView{};
// 		VkSampler     m_Sampler{};

// 		U32            m_LayerCount{ 1 };
// 	};
// }














//     struct VyTextureDesc
//     {
//         static VyTextureDesc texture2D(U32 width, U32 height, VkFormat format);
//         static VyTextureDesc cubemap  (U32 width, U32 height, VkFormat format);

//         VyImageDesc     Image;
//         VyImageViewDesc View;
//         VySamplerDesc   Sampler;
//     };



// 	class VyTexture //: public VyAsset
//     {
// 		static constexpr int NUMBER_OF_CUBEMAP_IMAGES = 6;

// 	public:
//         static Shared<VyTexture> load(const Path& file, VkFormat format = VK_FORMAT_R8G8B8A8_UNORM);
// 		// static Shared<VyTexture> load(const TVector<Path>& files, bool flip = true);

// 		// static Shared<VyTexture> loadCubemap(const TVector<Path>& files, bool flip = true);
		
// 		static Shared<VyTexture> loadTexture2D(const Path& file, VkFormat format);

// 		// VyTexture() = default;

		

// 		// template <typename T>
// 		// static void validateFormatCompatibility(VkFormat format);

// 	public:
//         VyTexture(const VyTextureDesc& desc);

// 		VyTexture(const VyTexture&) = delete;
// 		VyTexture(VyTexture&&) noexcept;

// 		~VyTexture();

// 		VyTexture& operator=(const VyTexture&) = delete;
// 		VyTexture& operator=(VyTexture&&) noexcept;

// 		VY_NODISCARD       VyImage&     image()          { return m_Image;   }
// 		VY_NODISCARD const VyImage&     image()    const { return m_Image;   }

// 		VY_NODISCARD       VyImageView& view()           { return m_View;    }
// 		VY_NODISCARD const VyImageView& view()     const { return m_View;    }

// 		VY_NODISCARD       VySampler&   sampler()        { return m_Sampler; }
// 		VY_NODISCARD const VySampler&   sampler()  const { return m_Sampler; }

// 		VY_NODISCARD       VkExtent3D   extent()   const { return m_Image.extent(); }
// 		VY_NODISCARD       VkExtent2D   extent2D() const { return VkExtent2D{ m_Image.width(), m_Image.height() }; }

// 		VY_NODISCARD VkDescriptorImageInfo descriptorImageInfo() const
// 		{
// 			return descriptorImageInfo(m_Image.layout());
// 		}

// 		VY_NODISCARD VkDescriptorImageInfo descriptorImageInfo(VkImageLayout layoutOverride) const
// 		{
// 			VkDescriptorImageInfo info{};
// 			{
// 				info.sampler     = m_Sampler.handle();
// 				info.imageView   = m_View.handle();
// 				info.imageLayout = layoutOverride;
// 			}

// 			return info;
// 		}

// 		void resize(VkExtent3D newSize, VkImageUsageFlags usage);

// 		// static VyAssetType getStaticType()               { return VyAssetType::Image; }
// 		// VyAssetType        getAssetType() const override { return getStaticType(); }

// 	private:

//         void destroy();

// 		VyImage     m_Image;
// 		VyImageView m_View;
// 		VySampler   m_Sampler;
// 	};
// }




















// namespace Vy
// {
// 	/**
// 	 * @class VulkanImage
// 	 * @brief Represents a Vulkan image and its associated resources.
// 	 *
// 	 * This class encapsulates the creation and management of a Vulkan image, including its view and sampler.
// 	 * It is a generic class that can be used for different types of images (e.g., 2D, 3D, cubeMaps).
// 	 * 
// 	 * It can be extended to create specific types of images (e.g., Texture2D, Texture3D, etc.).
// 	 */
// 	class VulkanImage : public VyImageAsset 
// 	{
// 	public:
// 		VulkanImage(const VyImageInfo& info, const DataBuffer& buffer);
// 		VulkanImage(const VkImageCreateInfo& imageInfo, VmaAllocationCreateFlags allocFlags = 0);

// 		~VulkanImage() override;

// 		VulkanImage(const VulkanImage&) = delete;
// 		VulkanImage& operator=(const VulkanImage&) = delete;
// 		VulkanImage(VulkanImage&&) = delete;
// 		VulkanImage& operator=(VulkanImage&&) = delete;

// 		U32 getWidth() override 
// 		{
// 			return m_Info.Width;
// 		}

// 		U32 getHeight() override 
// 		{
// 			return m_Info.Height;
// 		}

// 		U16 getChannels() override 
// 		{
// 			return m_Info.Channels;
// 		}

// 		VkFormat getFormat() override 
// 		{
// 			return m_Info.Format;
// 		}

// 		VyAssetType getAssetType() const override 
// 		{
// 			return VyAssetType::Image;
// 		}

// 		VkDescriptorImageInfo descriptorImageInfo(bool useSampler = true) 
// 		{
// 			return VkDescriptorImageInfo{
// 				.sampler     = useSampler ? m_Sampler : VK_NULL_HANDLE,
// 				.imageView   = m_View,
// 				.imageLayout = m_CurrentLayout
// 			};
// 		}

// 		VkExtent2D getExtent2D() const 
// 		{
// 			return { m_Info.Width, m_Info.Height };
// 		}

// 		float getAspectRatio() const 
// 		{
// 			return static_cast<float>(m_Info.Width) / static_cast<float>(m_Info.Height);
// 		}

// 		VkImage             handle()       { return m_Image; }
// 		const VkImageView   view()         { return m_View; }
// 		const VkSampler     sampler()      { return m_Sampler; }
// 		const VkFormat      format()       { return m_imageFormat; }
// 		const VkImageLayout layout() const { return m_CurrentLayout; }
		
// 		const void setImageSampler(const VkSampler sampler)      { m_Sampler = sampler; }
// 		void       setImageLayout(const VkImageLayout newLayout) { m_CurrentLayout = newLayout; }

// 		VulkanImage& createView(const VkImageViewCreateInfo& viewInfo);
// 		VulkanImage& createSampler(const VkSamplerCreateInfo& samplerInfo);

// 		/**
// 		 * @brief Transitions the layout of an image.
// 		 *
// 		 * This function transitions the layout of an image, which is required when changing the way the image is accessed.
// 		 * 
// 		 * @param oldLayout The old layout of the image.
// 		 * @param newLayout The new layout of the image.
// 		 * @param subresourceRange The subresource range of the image.
// 		 * @param sourceStage The source pipeline stage. If not specified, it will be set to VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, which is less efficient.
// 		 * @param destinationStage The destination pipeline stage. If not specified, it will be set to VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, which is less efficient.
// 		 */
// 		void transitionImageLayout(VkImageLayout newLayout, VkPipelineStageFlags sourceStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VkPipelineStageFlags destinationStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, std::optional<VkImageSubresourceRange> subresourceRange = std::nullopt);

// 		/**
// 		 * @brief Transitions the layout of an image.
// 		 *
// 		 * This function transitions the layout of an image, which is required when changing the way the image is accessed.
// 		 *
// 		 * @param cmdBuffer The command buffer handle.
// 		 * @param oldLayout The old layout of the image.
// 		 * @param newLayout The new layout of the image.
// 		 * @param subresourceRange The subresource range of the image.
// 		 * @param sourceStage The source pipeline stage. If not specified, it will be set to VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, which is less efficient.
// 		 * @param destinationStage The destination pipeline stage. If not specified, it will be set to VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, which is less efficient.
// 		 */
// 		void transitionImageLayout(VkCommandBuffer cmdBuffer, VkImageLayout newLayout, VkPipelineStageFlags sourceStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VkPipelineStageFlags destinationStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, std::optional<VkImageSubresourceRange> subresourceRange = std::nullopt);


// 	protected:

// 		VkFormat m_imageFormat;

// 		VyImageInfo m_Info;
// 		VkImage m_Image; // the raw image pixels
// 		VmaAllocation m_Allocation; // the memory occupied by the image
// 		VkImageView m_View; // an abstraction to view the same raw image in different "ways"
// 		VkSampler m_Sampler; // an abstraction (and tool) to help fragment shader pick the right color and apply useful transformations (e.g. bilinear filtering, anisotropic filtering etc.)

// 		VkImageLayout m_CurrentLayout = VK_IMAGE_LAYOUT_UNDEFINED;
// 	};



// 	/**
// 	 * @class VyTexture2D
// 	 * @brief Represents a Vulkan Texture and its associated resources.
// 	 *
// 	 * This class encapsulates the creation and management of a Vulkan texture, including its view and sampler.
// 	 * It extends the Image class to provide specific functionality for 2D textures.
// 	 */
// 	class VyTexture2D : public VulkanImage 
// 	{
// 	public:
// 		static Unique<VyTexture2D> create(const VyImageInfo& info, const DataBuffer& buffer);

// 		VyTexture2D(const VyImageInfo& info, const DataBuffer& buffer);

// 	private:

// 		/**
// 		 * @brief Creates a texture image.
// 		 *
// 		 * This function creates a Vulkan image and copies the pixel data from the provided buffer to the image.
// 		 * It also transitions the image layout to be used as a texture.
// 		 *
// 		 * @param info The texture information, including width, height, channels
// 		 * @param buffer The buffer containing the pixel data
// 		 */
// 		void createTextureImage(const DataBuffer& buffer);

// 		/**
// 		 * @brief Creates a Vulkan image.
// 		 */
// 		void createImage(U32 width, U32 height, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VmaAllocationCreateFlags allocFlags = 0);

// 		/**
// 		 * @brief Creates an image view.
// 		 *
// 		 * An image view is a way to interpret the image data.
// 		 * It describes how to access the image and which part of the image to access.
// 		 */
// 		void createTextureView();

// 		/**
// 		 * @brief Creates a texture sampler.
// 		 *
// 		 * A texture sampler is a set of parameters that control how textures are read and sampled by the GPU.
// 		 */
// 		void createTextureSampler();
// 	};
// }













// namespace Vy
// {
// 	class TextureSource
// 	{
// 		friend class Texture;

// 	public:
// 		virtual U32 width()        { return m_Width; }
// 		virtual U32 height()       { return m_Height; }
// 		virtual U32 bitsPerPixel() { return m_BitsPerPixel; }

// 	protected:
// 		TextureSource() {}

// 	protected:
// 		TBlob m_Pixels;
// 		U32   m_Width;
// 		U32   m_Height;

// 		/**
// 		 * 
// 		 * BitsPerChannel * NumOfChannels = BitsPerPixel. 
// 		 * 
// 		 * 24-bit: Usually an RGB image with 8 bits per color channel (8 * 3 = 24)
// 		 * 32-bit: Adds an alpha channel to a 24-bit RGB image, which is used for transparency.
// 		 * 
// 		 */
// 		U32 m_BitsPerPixel;
// 	};

// 	class FileTextureSource : public TextureSource
// 	{
// 	public:
// 		FileTextureSource(const String& filepath);
// 	};

// 	class SolidTextureSource : public TextureSource
// 	{
// 	public:
// 		SolidTextureSource(Vec4 color, U32 width = 1, U32 height = 1);
// 	};

// 	class FloatFileTextureSource : public TextureSource
// 	{
// 	public:
// 		FloatFileTextureSource(const String& filepath);
// 	};

// 	class FloatSolidTextureSource : public TextureSource
// 	{
// 	public:
// 		FloatSolidTextureSource(Vec4 color, U32 width = 1, U32 height = 1);
// 	};


// 	class Texture
// 	{
// 		friend class Cubemap;

// 	public:
// 		class Builder
// 		{
// 		public:
// 			Builder() {};

// 			Builder& addLayer(TextureSource&& source);

// 			Builder& setFormat(VkFormat format)
// 			{
// 				m_Format = format;
// 				return *this;
// 			}

// 			Builder& setFilter(VkFilter filter)
// 			{
// 				m_MinMagFilter = filter;
// 				return *this;
// 			}

// 			Builder& setAddressMode(VkSamplerAddressMode mode)
// 			{
// 				m_AddressMode = mode;
// 				return *this;
// 			}
			
// 			Builder& setCubemap(bool value)
// 			{
// 				m_IsCubemap = value;
// 				return *this;
// 			}

// 			Builder& useMipmaps(bool use, VkSamplerMipmapMode mode = VK_SAMPLER_MIPMAP_MODE_LINEAR)
// 			{
// 				m_UseMipmaps = use;
// 				m_MipmapMode = mode;
// 				return *this;
// 			}

// 			Builder& addUsageFlag(VkImageUsageFlagBits flag)
// 			{
// 				m_Usage |= flag;
// 				return *this;
// 			}

// 			Builder& setLayout(VkImageLayout layout)
// 			{
// 				m_Layout = layout;
// 				return *this;
// 			}

// 			Unique<Texture> build();

// 		private:
// 			void        generateMipmaps();
// 			VkImageView createImageView();
// 			VkSampler   createSampler();

// 		private:
// 			Unique<Texture> m_Texture;
// 			TVector<TBlob> m_Layers;
// 			U32 m_Width	         = -1;
// 			U32 m_Height         = -1;
// 			U32 m_BitsPerPixel	 = -1;

// 			VkFormat             m_Format       = VK_FORMAT_R8G8B8A8_SRGB;
// 			VkImageLayout        m_Layout       = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
// 			VkFilter             m_MinMagFilter = VK_FILTER_LINEAR;
// 			VkSamplerAddressMode m_AddressMode  = VK_SAMPLER_ADDRESS_MODE_REPEAT;
// 			bool                 m_IsCubemap    = false;
// 			bool                 m_UseMipmaps   = false;
// 			VkSamplerMipmapMode  m_MipmapMode   = VK_SAMPLER_MIPMAP_MODE_LINEAR;
// 			int                  m_MipmapCount  = 1;
// 			VkImageUsageFlags    m_Usage        = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
// 		};

// 	public:
// 		Texture() = default;

// 		~Texture();

// 		VkImageView           view()    const { return m_View; }
// 		VkSampler             sampler() const { return m_Sampler; }

// 		VkDescriptorImageInfo descriptorImageInfo() const;

// 		U32      layers()       const { return m_Layers; }
// 		U32      width()        const { return m_Width; }
// 		U32      height()       const { return m_Height; }
// 		U32      bitsPerPixel() const { return m_BitsPerPixel; }
// 		U32      mipMaps()      const { return m_MipMapsLevels; }
// 		VkFormat Format()       const { return m_Format; }

// 	public:
// 		void transitionImageLayout(VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, U32 layerCount, U32 mipmapCount);

// 	private:
// 		VkImage       m_Image;
// 		VmaAllocation m_Allocation;
// 		VkImageView   m_View;
// 		VkSampler     m_Sampler;
// 		VkImageLayout m_Layout;
// 		VkFormat      m_Format;

// 		U32 m_Layers;
// 		U32 m_Width;
// 		U32 m_Height;
// 		U32 m_BitsPerPixel;
// 		U32 m_MipMapsLevels;
// 	};