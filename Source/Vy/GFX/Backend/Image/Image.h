#pragma once

#include <Vy/GFX/Backend/Device.h>
#include <Vy/GFX/Backend/Buffer/Buffer.h>

namespace Vy
{
	struct VyImageCreateInfo
	{
		// CString                  DebugName     = nullptr;
		VkImageType              ImageType     = VK_IMAGE_TYPE_2D;
		VkFormat                 Format        = VK_FORMAT_R8G8B8A8_SRGB;
		VkExtent3D               Extent 	   = { 1, 1, 1 };
		U32                      MipLevels     = 1;
		U32                      ArrayLayers   = 1;
		VkSampleCountFlagBits    Samples       = VK_SAMPLE_COUNT_1_BIT;
		VkImageTiling            Tiling        = VK_IMAGE_TILING_OPTIMAL;
		VkImageUsageFlags        Usage         = VK_IMAGE_USAGE_SAMPLED_BIT;
		VkSharingMode            SharingMode   = VK_SHARING_MODE_EXCLUSIVE;
		VkImageLayout            InitialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		VkImageCreateFlags       Flags         = 0;

		// Memory / Allocation
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

			Builder& imageLayout(VkImageLayout layout)
			{
				m_Info.InitialLayout= layout; 
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

            VyImage build() const 
            {
				VY_ASSERT(m_Info.Extent.width > 0 && m_Info.Extent.height > 0 && m_Info.Extent.depth > 0, 
					"Image extent dimensions must be greater than 0.");

                return VyImage{ m_Info };
            }

            Unique<VyImage> buildUnique() const 
            {
				VY_ASSERT(m_Info.Extent.width > 0 && m_Info.Extent.height > 0 && m_Info.Extent.depth > 0,
					"Image extent dimensions must be greater than 0.");

                return MakeUnique<VyImage>(m_Info);
            }

            Shared<VyImage> buildShared() const 
            {
				VY_ASSERT(m_Info.Extent.width > 0 && m_Info.Extent.height > 0 && m_Info.Extent.depth > 0,
					"Image extent dimensions must be greater than 0.");

                return MakeShared<VyImage>(m_Info);
            }

		private:
			VyImageCreateInfo m_Info{};
		};

	public:
		VyImage() = default;

		VyImage(const VyImageCreateInfo& desc);

		// VyImage(const VyImage&) = delete;
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

		void copyFrom(VkCommandBuffer cmdBuffer, const VyBuffer& srcBuffer);
		void copyFrom(const VyBuffer& srcBuffer);

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