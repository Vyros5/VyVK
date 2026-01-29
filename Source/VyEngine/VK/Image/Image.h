#pragma once

#include <VyEngine/VK/Device/Device.h>
#include <VyEngine/VK/Buffer/Buffer.h>

#include <VyEngine/VK/Image/ImageView.h>

namespace Vy
{
	struct VyImageInfo
	{
		static constexpr U32 CALCULATE_MIP_LEVELS = 0;

		VkImageCreateFlags       Flags          = 0;
		VkImageType              ImageType      = VK_IMAGE_TYPE_2D;
		VkFormat                 Format         = VK_FORMAT_R8G8B8A8_UNORM;
		VkExtent3D               Extent         = { 1, 1, 1 };
		U32                      MipLevels      = CALCULATE_MIP_LEVELS;
		U32                      ArrayLayers    = 1;
		VkSampleCountFlagBits    Samples        = VK_SAMPLE_COUNT_1_BIT;
		VkImageUsageFlags        Usage          = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        VkImageTiling            Tiling         = VK_IMAGE_TILING_OPTIMAL;
        VkSharingMode            SharingMode    = VK_SHARING_MODE_EXCLUSIVE;
        VkImageLayout            InitialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;

		VmaAllocationCreateFlags AllocFlags     = 0;
		VmaMemoryUsage           MemoryUsage    = VMA_MEMORY_USAGE_AUTO;
	};

	class VyImage final
	{
	public:

		class  Builder;
		friend Builder;

	public:

		/**
		 * @brief Default constructor.
		 */
		VyImage() = default;

		explicit VyImage(const TString& name, const VyImageInfo& info);

        /**
         * @brief Deleted Copy constructor.
         */
		VyImage(const VyImage&) = delete;

        /**
         * @brief Move constructor.
         */
		VyImage(VyImage&& other) noexcept;
        
        /**
         * @brief Destructor for the VyImage class.
         *
         * Releases the Vulkan image.
         */
		~VyImage();

        /**
         * @brief Deleted Copy assignment operator.
         */
		VyImage& operator=(VyImage&) = delete;

        /**
         * @brief Move assignment operator.
         */
		VyImage& operator=(VyImage&& other) noexcept;

		operator VkImage() const { return m_Image; }
		operator VkImage&()      { return m_Image; }

		VY_NODISCARD VkImage               handle()      const { return m_Image; }
		VY_NODISCARD VmaAllocation         allocation()  const { return m_ImageMemory; }
		VY_NODISCARD VkFormat              format()      const { return m_Info.Format; }
		VY_NODISCARD U32                   width()       const { return m_Info.Extent.width; }
		VY_NODISCARD U32                   height()      const { return m_Info.Extent.height; }
		VY_NODISCARD U32                   depth()       const { return m_Info.Extent.depth; }
		VY_NODISCARD U32                   mipLevels()   const { return m_Info.MipLevels; }
		VY_NODISCARD U32                   layerCount()  const { return m_Info.ArrayLayers; }
		VY_NODISCARD VkImageLayout         layout()      const { return m_CurrentLayout; }
		VY_NODISCARD VkSampleCountFlagBits sampleCount() const { return m_Info.Samples; }
		VY_NODISCARD VkExtent3D            extent()      const { return m_Info.Extent; }
		VY_NODISCARD VkExtent2D            extent2D()    const { return VkExtent2D{ m_Info.Extent.width, m_Info.Extent.height }; }
		VY_NODISCARD bool                  valid()       const { return m_Image != VK_NULL_HANDLE; }
		VY_NODISCARD const TString&        debugName()   const { return m_DebugName; }
		
		void create(const VyImageInfo& info);
		// void create(const VkImageCreateInfo& info, const VmaAllocationCreateInfo& allocInfo);

		void upload(const VyBuffer& srcBuffer);
		void upload(const void* pData, VkDeviceSize size);

		void copyFrom(VkCommandBuffer cmdBuffer, const VyBuffer& srcBuffer, bool toShaderReadOnly = true);
		void copyFrom(const VyBuffer& srcBuffer, bool toShaderReadOnly = true);

		/**
		 * @brief Transitions the image to a new layout.
		 * @param cmdBuffer The Vulkan command buffer.
		 * @param newLayout The new image layout.
		 */
		void transitionLayout(VkCommandBuffer cmdBuffer, VkImageLayout newLayout);
		void transitionLayout(VkImageLayout newLayout);

		void generateMipmaps(VkCommandBuffer cmdBuffer, VkImageLayout finalLayout);
		void generateMipmaps(VkImageLayout finalLayout);

		void generateMipmaps(VkCommandBuffer cmdBuffer, U32 texW, U32 texH, U32 mipCount, U32 layerCount, VkImageLayout finalLayout);
		void generateMipmaps(U32 texW, U32 texH, U32 mipCount, U32 layerCount, VkImageLayout finalLayout);

	private:

		void destroy();

		void swap(VyImage& other);

		VkImage       m_Image      { VK_NULL_HANDLE };
		VmaAllocation m_ImageMemory{ VK_NULL_HANDLE };
		VyImageInfo   m_Info       {};

		VkImageLayout m_CurrentLayout{ VK_IMAGE_LAYOUT_UNDEFINED };
		TString       m_DebugName    { "unnamed" };
	};



	class VyImage::Builder
	{
	public:
		Builder();

		Builder& setName(const TString& name);
		
		Builder& setWidth(U32  width);
		Builder& setHeight(U32 height);
		Builder& setDepth(U32  depth);

		Builder& setExtent(VkExtent2D extent);
		Builder& setExtent(VkExtent3D extent);
		Builder& setExtent(U32 width, U32 height, U32 depth = 1);
		
		Builder& setImageType(VkImageType type);

		Builder& setLevels(U32 levels);
		Builder& setLayers(U32 layers);

		Builder& setFormat(VkFormat format);
		Builder& setLayout(VkImageLayout layout);
		Builder& setTiling(VkImageTiling tiling);
		Builder& setSamples(VkSampleCountFlagBits sampleCount);
		Builder& setSharing(VkSharingMode sharingMode);
		
		Builder& setUsage(VkImageUsageFlags flags);
		Builder& addUsage(VkImageUsageFlags flags);
		Builder& setFlags(VkImageCreateFlags flags);
		
		Builder& setMemoryUsage(VmaMemoryUsage usage);
		Builder& setAllocFlags(VmaAllocationCreateFlags flags);

		// Builder& initialData(void* data, U32 offset, U32 width, U32 height, U32 dataSize, VkImageLayout finalLayout);
		// Builder& setPreMadeImage(VkImage image);

		VyImage         build()    const;
		Unique<VyImage> buildPtr() const;

	private:
		// bool          m_UseInitialData;
		// void*         m_pData;
		// U32           m_InitDataSize;
		// U32           m_InitDataWidth;
		// U32           m_InitDataHeight;
		// U32           m_InitDataOffset;
		// VkImageLayout m_FinalLayout;
		// VkImage       m_PreMadeImage;

		TString        m_Name{ "unnamed" };
		VyImageInfo    m_Info{};
	};


}