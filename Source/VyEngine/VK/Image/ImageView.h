#pragma once

#include <VyEngine/VK/Device/Device.h>

namespace Vy
{
    class VyImage;

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
			const TString&               name,
			const VkImageViewCreateInfo& info, 
			const VyImage&               image
		);

		VyImageView(
			const TString&         name,
			VyImage&               image,
			VkImageViewType        viewType = VK_IMAGE_VIEW_TYPE_2D 
		);

		VyImageView(
			const TString&          name,
			const VyImage&          image,
			VkImageViewType         viewType,
			VkFormat                format,
			VkComponentMapping      componentMapping,
			VkImageSubresourceRange subresourceRange
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
		// VY_NODISCARD const VyImage&     imageRef()   const { return m_pOwnerImage; }

		void destroy();

	private:
		// VyImage&              m_pOwnerImage;
		VkImageView           m_ImageView  { VK_NULL_HANDLE };
		VkImageViewCreateInfo m_Info       {};
		TString                m_DebugName { "unnamed" };

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
		TString               m_Name{ "unnamed" };
		VkImageViewCreateInfo m_Info{};
	};
}