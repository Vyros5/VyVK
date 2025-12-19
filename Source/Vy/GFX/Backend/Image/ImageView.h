#pragma once

#include <Vy/GFX/Backend/Device.h>

namespace Vy
{
	class VyImage;

	struct VyImageViewCreateInfo
	{
        static constexpr U32                kUseImageMipLevels  = 0;
        static constexpr U32                kUseImageLayers     = 0;
		static constexpr VkFormat           kUseImageFormat     = VK_FORMAT_UNDEFINED;
		static constexpr VkImageAspectFlags kUseImageAspectMask = 0;

		VkImageViewType    ViewType     = VK_IMAGE_VIEW_TYPE_2D;
		VkFormat           Format       = kUseImageFormat;

		VkImageAspectFlags AspectMask   = kUseImageAspectMask;
		U32                BaseMipLevel = 0;
		U32                LevelCount   = kUseImageMipLevels;
		U32                BaseLayer    = 0;
		U32                LayerCount   = kUseImageLayers;
		
		VkComponentMapping Components   = {
			.r = VK_COMPONENT_SWIZZLE_IDENTITY,
			.g = VK_COMPONENT_SWIZZLE_IDENTITY,
			.b = VK_COMPONENT_SWIZZLE_IDENTITY,
			.a = VK_COMPONENT_SWIZZLE_IDENTITY
		};
	};

	
	class VyImageView
	{
	public:
		class Builder
		{
			friend class VyImageView;

		public:
			Builder() = default;

			Builder& viewType(VkImageViewType type)
			{
				m_Info.ViewType = type; 

				return *this; 
			}

			Builder& format(VkFormat format)
			{
				m_Info.Format = format; 

				return *this; 
			}

			Builder& aspectMask(VkImageAspectFlags aspectMask)
			{
				m_Info.AspectMask = aspectMask; 

				return *this; 
			}
			
			Builder& mipLevels(U32 baseMipLevel, U32 levelCount = VyImageViewCreateInfo::kUseImageMipLevels)
			{
				m_Info.BaseMipLevel = baseMipLevel;
				m_Info.LevelCount   = levelCount;

				return *this; 
			}

			Builder& arrayLayers(U32 baseArrayLayer, U32 layerCount = VyImageViewCreateInfo::kUseImageLayers)
			{
				m_Info.BaseLayer  = baseArrayLayer;
				m_Info.LayerCount = layerCount;

				return *this; 
			}

			Builder& componentsRGBA(VkComponentSwizzle r, VkComponentSwizzle g, VkComponentSwizzle b, VkComponentSwizzle a)
			{
				m_Info.Components.r = r;
				m_Info.Components.g = g;
				m_Info.Components.b = b;
				m_Info.Components.a = a;

				return *this; 
			}

			Builder& subresourceRange(VkImageSubresourceRange subRange)
			{
				m_Info.AspectMask   = subRange.aspectMask;
				m_Info.LayerCount   = subRange.layerCount;
				m_Info.BaseLayer    = subRange.baseArrayLayer;
				m_Info.LevelCount   = subRange.levelCount;
				m_Info.BaseMipLevel = subRange.baseMipLevel;

				return *this; 
			}

			VyImageView build(const VyImage& image) const 
			{
				return VyImageView{ m_Info, image };
			}

			Unique<VyImageView> buildUnique(const VyImage& image) const 
			{
				return MakeUnique<VyImageView>(m_Info, image);
			}

			Shared<VyImageView> buildShared(const VyImage& image) const 
			{
				return MakeShared<VyImageView>(m_Info, image);
			}

		private:
			VyImageViewCreateInfo m_Info{};
		};
	public:

		VyImageView() = default;

		explicit VyImageView(const VyImageViewCreateInfo& desc, const VyImage& image);

		VyImageView(const VyImageView&) = delete;
		VyImageView(VyImageView&& other) noexcept;

		~VyImageView();

		VyImageView& operator=(const VyImageView&) = delete;
		VyImageView& operator=(VyImageView&& other) noexcept;

		operator           VkImageView()             const { return m_ImageView; }
		VY_NODISCARD       VkImageView  handle()     const { return m_ImageView; }
		VY_NODISCARD const VkImageView& handleRef()        { return m_ImageView; }
		VyImageViewCreateInfo           createInfo() const { return m_Info; }
		
	private:
		void destroy();

		VkImageView           m_ImageView = VK_NULL_HANDLE;
		VyImageViewCreateInfo m_Info;
	};
}