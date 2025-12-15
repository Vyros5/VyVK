#pragma once

#include <Vy/GFX/Backend/Device.h>
#include <Vy/GFX/Backend/Resources/Buffer.h>

namespace Vy
{
	struct VyImageCreateInfo
	{
		// static constexpr I32 kNoCascades = -1;

		// CString                  DebugName     = nullptr;
		VkImageType              ImageType     = VK_IMAGE_TYPE_2D;
		VkFormat                 Format        = VK_FORMAT_R8G8B8A8_SRGB;
		VkExtent3D               Extent 	   = { 1, 1, 1 };
		U32                      MipLevels     = 1;
		U32                      ArrayLayers   = 1;
		// I32                      Cascades      = kNoCascades;
		VkSampleCountFlagBits    Samples       = VK_SAMPLE_COUNT_1_BIT;
		VkImageTiling            Tiling        = VK_IMAGE_TILING_OPTIMAL;
		VkImageUsageFlags        Usage         = VK_IMAGE_USAGE_SAMPLED_BIT;
		VkSharingMode            SharingMode   = VK_SHARING_MODE_EXCLUSIVE;
		VkImageLayout            InitialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		VkImageCreateFlags       Flags         = 0;

		VmaAllocationCreateFlags AllocFlags    = 0;
		VmaMemoryUsage           MemoryUsage   = VMA_MEMORY_USAGE_AUTO;
	};


	class VyImage
	{
	public:

		class Builder
		{
			friend class VyImage;

		public:
			Builder() = default;

			// Builder& debugName(CString name)
			// {
			// 	m_Info.DebugName = name; 
			// 	return *this;
			// }
			
			Builder& width(U32 width)
			{
				m_Info.Extent.width = width; 
				return *this; 
			}
			
			Builder& height(U32 height)
			{
				m_Info.Extent.height = height; 
				return *this; 
			}
			
			Builder& depth(U32 depth)
			{
				m_Info.Extent.depth = depth; 
				return *this; 
			}

			// Builder& cascades(I32 cascades)
			// {
			// 	m_Info.Cascades = cascades; 
			// 	return *this; 
			// }

			Builder& extent(VkExtent2D exent)
			{
				m_Info.Extent.width  = exent.width;
				m_Info.Extent.height = exent.height;
				m_Info.Extent.depth  = 1;
				return *this;
			}

		    Builder& extent(VkExtent3D exent)
		    {
		        m_Info.Extent = exent;
		        return *this;
		    }

		    Builder& format(VkFormat format)
			{
				m_Info.Format = format; 
				return *this;
			}

			Builder& tiling(VkImageTiling tiling)
			{
				m_Info.Tiling = tiling; 
				return *this;
			}

			Builder& usage(VkImageUsageFlags usage)
			{
				m_Info.Usage = usage; 
				return *this;
			}

			Builder& memoryUsage(VmaMemoryUsage usage)
			{
				m_Info.MemoryUsage = usage; 
				return *this;
			}

			Builder& createFlags(VkImageCreateFlags flags)
			{
				m_Info.Flags = flags; 
				return *this;
			}

			Builder& allocFlags(VmaAllocationCreateFlags allocFlags)
			{
				m_Info.AllocFlags = allocFlags; 
				return *this;
			}

			Builder& mipLevels(U32 levels, bool useMipmaps = false)
			{
				m_Info.MipLevels = levels;

				if (useMipmaps)
				{
					m_Info.Usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
				}

				return *this;
			}

			Builder& arrayLayers(U32 layers)
			{
				m_Info.ArrayLayers = layers;
				return *this;
			}

			Builder& sampleCount(VkSampleCountFlagBits sampleCount)
			{
				m_Info.Samples = sampleCount;
				return *this;
			}

			Builder& sharingMode(VkSharingMode sharingMode)
			{
				m_Info.SharingMode = sharingMode;
				return *this;
			}

			Builder& imageType(VkImageType type)
			{
				m_Info.ImageType = type;
				return *this;
			}

			// Builder& initialData(void* pData, U32 offset, U32 width, U32 height, U32 dataSize, VkImageLayout finalLayout);
			// Builder& preMadeImage(VkImage image);

			// void build(Image& image) const;

            VyImage build() const 
            {
				VY_ASSERT(m_Info.Extent.width > 1 && m_Info.Extent.height > 1 && m_Info.Extent.depth > 0, 
					"Image extent dimensions must be greater than 0.");

                return VyImage{ m_Info };
            }

            Unique<VyImage> buildUnique() const 
            {
				VY_ASSERT(m_Info.Extent.width > 1 && m_Info.Extent.height > 1 && m_Info.Extent.depth > 0, 
					"Image extent dimensions must be greater than 0.");

                return MakeUnique<VyImage>(m_Info);
            }

            Shared<VyImage> buildShared() const 
            {
				VY_ASSERT(m_Info.Extent.width > 1 && m_Info.Extent.height > 1 && m_Info.Extent.depth > 0, 
					"Image extent dimensions must be greater than 0.");

                return MakeShared<VyImage>(m_Info);
            }

		private:
		// 	bool          m_UseInitialData;
		// 	void*         m_pData;
		// 	U32           m_InitDataSize;
		// 	U32           m_InitDataWidth;
		// 	U32           m_InitDataHeight;
		// 	U32           m_InitDataOffset;
		// 	VkImageLayout m_FinalLayout;

		// 	VkImage                 m_PreMadeImage;
		// 	VkImageCreateInfo       m_ImageInfo{};
		// 	VmaAllocationCreateInfo m_AllocInfo{};
			VyImageCreateInfo m_Info{};
		};

	public:
		VyImage() = default;

		VyImage(const VyImageCreateInfo& desc);

		VyImage(const VyImage&) = delete;
		VyImage(VyImage&& other) noexcept;
		
        ~VyImage();

		VyImage& operator=(const VyImage&) = delete;
		VyImage& operator=(VyImage&& other) noexcept;

		operator     VkImage()                           const { return m_Image;         }
		VY_NODISCARD VkImage               handle()      const { return m_Image;         }
		VY_NODISCARD VkFormat              format()      const { return m_Format;        }
		VY_NODISCARD VkExtent3D            extent()      const { return m_Extent;        }
		VY_NODISCARD U32                   width()       const { return m_Extent.width;  }
		VY_NODISCARD U32                   height()      const { return m_Extent.height; }
		VY_NODISCARD U32                   depth()       const { return m_Extent.depth;  }
		VY_NODISCARD U32                   mipLevels()   const { return m_MipLevels;     }
		VY_NODISCARD U32                   layerCount()  const { return m_LayerCount;    }
		VY_NODISCARD VkImageLayout         layout()      const { return m_Layout;        }
		VY_NODISCARD VkSampleCountFlagBits sampleCount() const { return m_SampleCount;   }

		void create(const VyImageCreateInfo& desc);

		void copyFrom(VkCommandBuffer cmdBuffer, const VyBuffer& src);

		// void resize(VkExtent3D extent, VkImageUsageFlags usage);

		void transitionLayout(VkImageLayout newLayout);
		void transitionLayout(VkCommandBuffer cmdBuffer, VkImageLayout newLayout);
		
		void generateMipmaps(VkCommandBuffer cmdBuffer, VkImageLayout finalLayout);
	private:

		void destroy();

		VkImage       m_Image      { VK_NULL_HANDLE };
		VmaAllocation m_Allocation { VK_NULL_HANDLE };

		VkImageCreateInfo     m_CreateInfo{};
		VkFormat              m_Format     = VK_FORMAT_UNDEFINED;
		VkExtent3D            m_Extent     = { 1, 1, 1 };
		U32                   m_MipLevels  = 1;
		U32                   m_LayerCount = 1;
		VkImageLayout         m_Layout     = VK_IMAGE_LAYOUT_UNDEFINED;
		VkSampleCountFlagBits m_SampleCount {};
	};
}

namespace Vy
{
	// //? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// //? ~~	  Image View	
	// //? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// class Image;

	// class ImageView final
	// {
	// public:
	// 	//--------------------------------------------------
	// 	//    Constructor & Destructor
	// 	//--------------------------------------------------
	// 	explicit ImageView() = default;
		
    //     ~ImageView() = default;
		
    //     void destroy() const;

	// 	//--------------------------------------------------
	// 	//    Accessors & Mutators
	// 	//--------------------------------------------------
	// 	const VkImageView& handle() const;
	// 	const Image&       image()  const;

	// private:
	// 	Image*                m_pOwnerImage{};
	// 	VkImageView           m_ImageView  { VK_NULL_HANDLE };
	// 	VkImageViewCreateInfo m_Info       {};

	// 	friend class Image;
	// };

	// //? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// //? ~~	  Image	
	// //? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// class Image final
	// {
	// public:
	// 	//--------------------------------------------------
	// 	//    Constructor & Destructor
	// 	//--------------------------------------------------
	// 	explicit Image() = default;

	// 	~Image() = default;
		
    //     Image(const Image& other) = delete;
	// 	Image(Image&& other) noexcept;
		
    //     Image& operator=(const Image& other) = delete;
	// 	Image& operator=(Image&& other) noexcept;
		
    //     void destroy();
	// 	void destroyAllViews();
	// 	void destroyViewsFrom(U32 firstViewToRemove);

	// 	//--------------------------------------------------
	// 	//    Helpers
	// 	//--------------------------------------------------
	// 	ImageView& CreateView(VkImageAspectFlags aspectFlags, VkImageViewType viewType, U32 baseMip, U32 mipCount, U32 baseLayer, U32 layerCount);

	// 	//--------------------------------------------------
	// 	//    Accessors & Mutators
	// 	//--------------------------------------------------
	// 	const VkImage&		handle()           const;
	// 	const ImageView&	view(U32 index = 0) const;
	// 	U32                 viewCount() const;
	// 	const TVector<ImageView>& allViews()			const;

	// 	U32			mipLevels()					const;
	// 	U32			layerCount()					const;

	// 	VkFormat			format()						const;
	// 	VkExtent3D			extent3D()					const;
	// 	VkExtent2D			extent2D()					const;
	// 	VkImageLayout		currentLayout()				const;

	// 	//--------------------------------------------------
	// 	//    Commands
	// 	//--------------------------------------------------
	// 	void transitionLayout(const VkCommandBuffer& cmdBuffer, VkImageLayout newLayout,
	// 						  VkAccessFlags2 srcAccess, VkPipelineStageFlags2 srcStage,
	// 						  VkAccessFlags2 dstAccess, VkPipelineStageFlags2 dstStage,
	// 						  U32 baseMip, U32 mipCount, U32 baseLayer, U32 layerCount);
	// 	void insertBarrier(const VkCommandBuffer& cmdBuffer,
	// 						  VkAccessFlags2 srcAccess, VkPipelineStageFlags2 srcStage,
	// 						  VkAccessFlags2 dstAccess, VkPipelineStageFlags2 dstStage);
	// 	void generateMipMaps(U32 texW, U32 texH, U32 mips, U32 layers, VkImageLayout finalLayout);
	// 	void generateMipMaps(const VkCommandBuffer& cmdBuffer, U32 texW, U32 texH, U32 mips, U32 layers, VkImageLayout finalLayout);

	// private:
	// 	VkImage            m_Image      { VK_NULL_HANDLE };
	// 	TVector<ImageView> m_vImageViews{};
	// 	VmaAllocation      m_ImageMemory{ VK_NULL_HANDLE };

	// 	VkImageLayout      m_CurrentLayout{ VK_IMAGE_LAYOUT_UNDEFINED };
	// 	VkImageCreateInfo  m_ImageInfo    {};

	// 	friend class ImageBuilder;
	// 	// friend class SwapChainBuilder;
	// };


	// //? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// //? ~~	  ImageBuilder	
	// //? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// class ImageBuilder final
	// {
	// public:
	// 	//--------------------------------------------------
	// 	//    Constructor & Destructor
	// 	//--------------------------------------------------
	// 	ImageBuilder();


	// 	//--------------------------------------------------
	// 	//    Builder
	// 	//--------------------------------------------------
	// 	ImageBuilder& debugName(const char* name);						
	// 	//! REQUIRED
	// 	ImageBuilder& width(U32 width);						
	// 	ImageBuilder& height(U32 height);
	// 	ImageBuilder& depth(U32 depth);
    //     ImageBuilder& extent(VkExtent3D exent);
    //     ImageBuilder& format(VkFormat format);
	// 	ImageBuilder& tiling(VkImageTiling tiling);
	// 	ImageBuilder& usageFlags(VkImageUsageFlags usage);
	// 	ImageBuilder& createFlags(VkImageCreateFlags flags);
	// 	ImageBuilder& memoryProperties(VkMemoryPropertyFlags properties);
	// 	ImageBuilder& mipLevels(U32 levels);
	// 	ImageBuilder& arrayLayers(U32 layers);
	// 	ImageBuilder& sampleCount(VkSampleCountFlagBits sampleCount);
	// 	ImageBuilder& sharingMode(VkSharingMode sharingMode);
	// 	ImageBuilder& imageType(VkImageType type);
	// 	ImageBuilder& initialData(void* pData, U32 offset, U32 width, U32 height, U32 dataSize, VkImageLayout finalLayout);
	// 	ImageBuilder& preMadeImage(VkImage image);

	// 	void build(Image& image) const;

	// private:
	// 	bool          m_UseInitialData;
	// 	void*         m_pData;
	// 	U32           m_InitDataSize;
	// 	U32           m_InitDataWidth;
	// 	U32           m_InitDataHeight;
	// 	U32           m_InitDataOffset;
	// 	VkImageLayout m_FinalLayout;

	// 	const char*   m_pName{};

	// 	VkImage                 m_PreMadeImage;
	// 	VkImageCreateInfo       m_ImageInfo{};
	// 	VmaAllocationCreateInfo m_AllocInfo{};
	// };


	// //? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// //? ~~	  Sampler	
	// //? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// class Sampler final
	// {
	// public:
	// 	//--------------------------------------------------
	// 	//    Constructor & Destructor
	// 	//--------------------------------------------------
	// 	explicit Sampler() = default;
		
    //     ~Sampler() = default;
		
    //     Sampler(const Sampler& other) = delete;
	// 	Sampler(Sampler&& other) noexcept = delete;
		
    //     Sampler& operator=(const Sampler& other) = delete;
	// 	Sampler& operator=(Sampler&& other) noexcept = delete;
		
    //     void destroy() const;

	// 	//--------------------------------------------------
	// 	//    Accessors & Mutators
	// 	//--------------------------------------------------
	// 	const VkSampler& handle() const;

	// private:
	// 	VkSampler m_Sampler;

	// 	friend class SamplerBuilder;
	// };


	// //? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// //? ~~	  SamplerBuilder	
	// //? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// class SamplerBuilder final
	// {
	// public:
	// 	//--------------------------------------------------
	// 	//    Constructor & Destructor
	// 	//--------------------------------------------------
	// 	SamplerBuilder();

	// 	//--------------------------------------------------
	// 	//    Builder
	// 	//--------------------------------------------------
	// 	// If not set, default of VK_FILTER_LINEAR is assumed
	// 	SamplerBuilder& filters(VkFilter magFilter, VkFilter minFilter);
		
    //     //! REQUIRED
	// 	SamplerBuilder& addressMode(VkSamplerAddressMode mode);
		
    //     // Anisotropy is VK_FALSE by default
	// 	SamplerBuilder& enableAnisotropy(float maxAnisotropy);
		
    //     // Black by default
	// 	SamplerBuilder& borderColor(VkBorderColor color);
		
    //     // Compare is VK_FALSE by default
	// 	SamplerBuilder& enableCompare(VkCompareOp op);
		
    //     // If not set, a default of VK_SAMPLER_MIPMAP_MODE_LINEAR is assumed
	// 	SamplerBuilder& mipmapMode(VkSamplerMipmapMode mode);

	// 	// Bias and min 0, max set to VK_LOD_CLAMP_NONE by default
	// 	SamplerBuilder& mipLevels(float bias, float minMip, float maxMip);

	// 	void build(Sampler& sampler) const;

	// private:
	// 	VkSamplerCreateInfo m_CreateInfo{};
	// };




    


//     ////////////////////////////////////////////////////////////////////////////////////////////////////
//     struct ImageViewCreateInfoBuilder
//     {
//         VkImageViewCreateInfo ViewInfo{};

//         ImageViewCreateInfoBuilder(VkImage image)
//         {
//             ViewInfo = VKInit::imageViewCreateInfo();
//             {
//                 ViewInfo.image    = image;

//                 ViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
//                 ViewInfo.format   = VK_FORMAT_R8G8B8A8_SRGB;

//                 ViewInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
//                 ViewInfo.subresourceRange.baseMipLevel   = 0;
//                 ViewInfo.subresourceRange.levelCount     = 1;
//                 ViewInfo.subresourceRange.baseArrayLayer = 0;
//                 ViewInfo.subresourceRange.layerCount     = 1;
//             }
//         }

//         ImageViewCreateInfoBuilder& viewType(VkImageViewType _type)
//         {
//             ViewInfo.viewType = _type; return *this; 
//         }

//         ImageViewCreateInfoBuilder& format(VkFormat _format)
//         {
//             ViewInfo.format = _format; return *this; 
//         }

//         ImageViewCreateInfoBuilder& aspectMask(VkImageAspectFlags _aspectMask)
//         {
//             ViewInfo.subresourceRange.aspectMask = _aspectMask; return *this; 
//         }
        
//         ImageViewCreateInfoBuilder& mipLevels(U32 _baseMipLevel, U32 _levelCount)
//         {
//             ViewInfo.subresourceRange.baseMipLevel = _baseMipLevel;
//             ViewInfo.subresourceRange.levelCount = _levelCount;
//             return *this; 
//         }

//         ImageViewCreateInfoBuilder& arrayLayers(U32 _baseArrayLayer, U32 _layerCount)
//         {
//             ViewInfo.subresourceRange.baseArrayLayer = _baseArrayLayer;
//             ViewInfo.subresourceRange.layerCount     = _layerCount;
//             return *this; 
//         }

//         ImageViewCreateInfoBuilder& componentsRGBA(VkComponentSwizzle r, VkComponentSwizzle g, VkComponentSwizzle b, VkComponentSwizzle a)
//         {
//             ViewInfo.components.r = r;
//             ViewInfo.components.g = g;
//             ViewInfo.components.b = b;
//             ViewInfo.components.a = a;
//             return *this; 
//         }

//         VkImageViewCreateInfo getInfo() const 
//         {
//             return ViewInfo;
//         }
//     };






    // struct VyImageDesc
    // {
    //     static constexpr U32 kCalculateMipLevels = 0;

    //     VkFormat           Format     = VK_FORMAT_R8G8B8A8_UNORM;
    //     VkExtent3D         Extent     = { 1, 1, 1 };
    //     U32                MipLevels  = kCalculateMipLevels;
    //     U32                LayerCount = 1;
    //     VkImageUsageFlags  Usage      = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    //     VkImageType        Type       = VK_IMAGE_TYPE_2D;
	// 	VkImageTiling      Tiling     = VK_IMAGE_TILING_OPTIMAL;
    //     VkImageCreateFlags Flags      = 0;
    // };

    // struct ImageCreateInfoBuilder
    // {
    //     VkImageCreateInfo ImageInfo{};

    //     ImageCreateInfoBuilder(U32 texWidth, U32 texHeight)
    //     {
    //         ImageInfo = VKInit::imageCreateInfo();
    //         {
    //             ImageInfo.imageType     = VK_IMAGE_TYPE_2D;
    //             ImageInfo.extent.width  = texWidth;
    //             ImageInfo.extent.height = texHeight;
    //             ImageInfo.extent.depth  = 1;
    //             ImageInfo.mipLevels     = 1;
    //             ImageInfo.arrayLayers   = 1;
    //             ImageInfo.format        = VK_FORMAT_R8G8B8A8_SRGB;
    //             ImageInfo.tiling        = VK_IMAGE_TILING_OPTIMAL;
    //             ImageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    //             ImageInfo.usage         = VK_IMAGE_USAGE_SAMPLED_BIT;
    //             ImageInfo.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;
    //             ImageInfo.samples       = VK_SAMPLE_COUNT_1_BIT; // For multisampling
    //             ImageInfo.flags         = 0;
    //         }
    //     }

    //     ImageCreateInfoBuilder& imageCreateFlags(VkImageCreateFlags _flags)
    //     {
    //         ImageInfo.flags = _flags; 
	// 		return *this; 
    //     }
        
    //     ImageCreateInfoBuilder& imageType(VkImageType _type)
    //     {
    //         ImageInfo.imageType = _type; 
	// 		return *this; 
    //     }

    //     ImageCreateInfoBuilder& extentDepth(U32 _depth)
    //     {
    //         ImageInfo.extent.depth = _depth; 
	// 		return *this; 
    //     }

    //     ImageCreateInfoBuilder& mipLevels(U32 _mipLevels)
    //     {
    //         ImageInfo.mipLevels = _mipLevels; 
	// 		return *this; 
    //     }

    //     ImageCreateInfoBuilder& arrayLayers(U32 _arrayLayers)
    //     {
    //         ImageInfo.arrayLayers = _arrayLayers; 
	// 		return *this; 
    //     }

    //     ImageCreateInfoBuilder& tiling(VkImageTiling _tiling)
    //     {
    //         ImageInfo.tiling = _tiling; 
	// 		return *this; 
    //     }

    //     ImageCreateInfoBuilder& usage(VkImageUsageFlags _usage)
    //     {
    //         ImageInfo.usage = _usage; 
	// 		return *this; 
    //     }

    //     ImageCreateInfoBuilder& sharingMode(VkSharingMode _sharingMode)
    //     {
    //         ImageInfo.sharingMode = _sharingMode; 
	// 		return *this; 
    //     }

    //     ImageCreateInfoBuilder& samples(VkSampleCountFlagBits _samples)
    //     {
    //         ImageInfo.samples = _samples; 
	// 		return *this; 
    //     }

    //     ImageCreateInfoBuilder& initialLayout(VkImageLayout _initialLayout)
    //     {
    //         ImageInfo.initialLayout = _initialLayout; 
	// 		return *this; 
    //     }

    //     ImageCreateInfoBuilder& format(VkFormat _format)
    //     {
    //         ImageInfo.format = _format; 
	// 		return *this; 
    //     }

    //     ImageCreateInfoBuilder& flags(VkImageCreateFlags _flags)
    //     {
    //         ImageInfo.flags = _flags; 
	// 		return *this; 
    //     }

    //     VkImageCreateInfo getInfo() const 
    //     {
    //         return ImageInfo;
    //     }
    // };
	
	// class VyImage
	// {
	// public:

	// 	VyImage() = default;

    //     VyImage(const VyImageDesc& desc);

	// 	VyImage(const VyImage&) = delete;
	// 	VyImage(VyImage&& other) noexcept;
		
    //     ~VyImage();

	// 	VyImage& operator=(const VyImage&) = delete;
	// 	VyImage& operator=(VyImage&& other) noexcept;

	// 	operator     VkImage()                  const { return m_Image;         }
	// 	VY_NODISCARD VkImage       handle()     const { return m_Image;         }
	// 	VY_NODISCARD VkFormat      format()     const { return m_Format;        }
	// 	VY_NODISCARD VkExtent3D    extent()     const { return m_Extent;        }
	// 	VY_NODISCARD U32           width()      const { return m_Extent.width;  }
	// 	VY_NODISCARD U32           height()     const { return m_Extent.height; }
	// 	VY_NODISCARD U32           depth()      const { return m_Extent.depth;  }
	// 	VY_NODISCARD U32           mipLevels()  const { return m_MipLevels;     }
	// 	VY_NODISCARD U32           layerCount() const { return m_LayerCount;    }
	// 	VY_NODISCARD VkImageLayout layout()     const { return m_Layout;        }

	// 	void upload(const VyBuffer& buffer);
	// 	void upload(const void* data, VkDeviceSize size);

	// 	void create(const VyImageDesc& desc);

	// 	void copyFrom(VkCommandBuffer cmdBuffer, const VyBuffer& src);

	// 	void resize(VkExtent3D extent, VkImageUsageFlags usage);

	// 	void transitionLayout(VkImageLayout newLayout);
	// 	void transitionLayout(VkCommandBuffer cmdBuffer, VkImageLayout newLayout);
		
	// 	VkImageMemoryBarrier transitionLayoutDeferred(VkImageLayout newLayout);

	// 	void generateMipmaps(VkCommandBuffer cmdBuffer, VkImageLayout finalLayout);

	// private:
	// 	void destroy();

	// 	VkImage       m_Image      = VK_NULL_HANDLE;
	// 	VmaAllocation m_Allocation = VK_NULL_HANDLE;
        
	// 	VkFormat      m_Format     = VK_FORMAT_UNDEFINED;
	// 	VkExtent3D    m_Extent     = { 1, 1, 1 };
	// 	U32           m_MipLevels  = 1;
	// 	U32           m_LayerCount = 1;
	// 	VkImageLayout m_Layout     = VK_IMAGE_LAYOUT_UNDEFINED;
	// };
}