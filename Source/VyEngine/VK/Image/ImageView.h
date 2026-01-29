#pragma once

#include <VyEngine/VK/Device/Device.h>

namespace Vy
{
    class VyImage;

	struct VyImageViewInfo
	{
		static constexpr U32 USE_IMAGE_MIP_LEVELS = 0;
		static constexpr U32 USE_IMAGE_LAYERS     = 0;

		VkImageViewType    ViewType     = VK_IMAGE_VIEW_TYPE_2D;
		VkFormat           Format       = VK_FORMAT_UNDEFINED;
		
		VkImageAspectFlags AspectMask   = VK_IMAGE_ASPECT_COLOR_BIT;
		U32                BaseMipLevel = 0;
		U32                LevelCount   = USE_IMAGE_MIP_LEVELS;
		U32                BaseLayer    = 0;
		U32                LayerCount   = USE_IMAGE_LAYERS;

		VkComponentMapping Components   = { VK_COMPONENT_SWIZZLE_IDENTITY };
	};

	/**
	 * @brief Vulkan Image View wrapper 
	 */
	class VyImageView final
	{
	public:
		class  Builder;
		friend Builder;
		
		/**
		 * @brief Default constructor.
		 */
		VyImageView() = default;

		explicit VyImageView(
			const TString&         name,
			const VyImageViewInfo& info, 
			const VyImage&         image
		);
        
		/**
         * @brief Deleted Copy constructor.
         */
		VyImageView(const VyImageView&) = delete;

        /**
         * @brief Move constructor.
         */
		VyImageView(VyImageView&& other) noexcept;

        /**
         * @brief Destructor for the VyImageView class.
         *
         * Releases the Vulkan image view.
         */
		~VyImageView();

        /**
         * @brief Deleted Copy assignment operator.
         */
		VyImageView& operator=(VyImageView&) = delete;

        /**
         * @brief Move assignment operator.
         */
		VyImageView& operator=(VyImageView&& other) noexcept;
		

		operator     const VkImageView&()            const { return m_ImageView; }
		VY_NODISCARD const VkImageView& handle()     const { return m_ImageView; }
		VY_NODISCARD bool               valid()      const { return m_ImageView != VK_NULL_HANDLE; }

		void destroy();

	private:
		VkImageView     m_ImageView  { VK_NULL_HANDLE };
		VyImageViewInfo m_Info       {};
		TString         m_DebugName { "unnamed" };

		friend class VyImage;
	};




	class VyImageView::Builder
	{
	public:
		Builder();

		Builder& setName(const TString& name);

		Builder& setViewType(VkImageViewType type);
		Builder& setFormat(VkFormat format);

		Builder& setAspect(VkImageAspectFlags aspectMask);
		Builder& addAspect(VkImageAspectFlags aspectMask);

		Builder& setLevels(U32 baseMipLevel, U32 levelCount = 1);
		Builder& setBaseLevel(U32 baseMipLevel);
		Builder& setLevelCount(U32 levelCount);

		Builder& setLayers(U32 baseArrayLayer, U32 layerCount = 1);
		Builder& setBaseLayer(U32 baseArrayLayer);
		Builder& setLayerCount(U32 layerCount);

		Builder& setComponents(VkComponentSwizzle swizzle);
		Builder& setComponentsRGBA(VkComponentSwizzle r, VkComponentSwizzle g, VkComponentSwizzle b, VkComponentSwizzle a);
		Builder& setMapping(VkComponentMapping mapping);

		Builder& setSubresourceRange(VkImageSubresourceRange subRange);
		
		VyImageView         build   (const VyImage& inImage) const;
		Unique<VyImageView> buildPtr(const VyImage& inImage) const;

	private:
		TString         m_Name{ "unnamed" };
		VyImageViewInfo m_Info{};
	};
}