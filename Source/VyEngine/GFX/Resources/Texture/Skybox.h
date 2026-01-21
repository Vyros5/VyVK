#pragma once

#include <VyEngine/VK/Image/Image.h>
#include <VyEngine/VK/Image/ImageView.h>
#include <VyEngine/VK/Image/Sampler.h>

#include <VyEngine/VK/Descriptors/Descriptors.h>

namespace Vy
{
	/**
	 * @brief Cubemap texture for skybox rendering
	 *
	 * Loads 6 face textures (right, left, top, bottom, front, back) into a Vulkan cubemap.
	 * Supports JPG, PNG, and other formats via stb_image.
	 */
	class VySkybox
    {
	public:

		/**
		 * @brief Create an empty skybox for rendering (e.g. procedural sky)
		 * @param size Resolution of each face (e.g. 1024)
		 */
		VySkybox(U32 size);

		/**
		 * @brief Load skybox from 6 separate face images
		 * @param facePaths Array of 6 paths: [+X, -X, +Y, -Y, +Z, -Z]
		 *                  (right, left, top, bottom, front, back)
		 */
		VySkybox(const TArray<TString, 6>& paths);

		/**
		 * @brief Load skybox from folder with standard naming
		 * @param folderPath Folder containing posx.jpg, negx.jpg, etc.
		 * @param extension  File extension (default: "jpg")
		 */
		static Unique<VySkybox> loadFromFolder(const TString& folderPath, const TString& extension = "jpg");

		~VySkybox();

		VySkybox(const VySkybox&)            = delete;
		VySkybox& operator=(const VySkybox&) = delete;
		VySkybox(VySkybox&&)                 = delete;
		VySkybox& operator=(VySkybox&&)      = delete;

		int size() const { return m_Size; }

		VkImageView imageView() const { return m_ImageView.handle(); }
		VkSampler   sampler()   const { return m_Sampler.handle(); }
		VkImage     image()     const { return m_Image.handle(); }

		VkDescriptorImageInfo descriptorImageInfo() const
		{
			return VkDescriptorImageInfo{
				.sampler     = m_Sampler  .handle(),
				.imageView   = m_ImageView.handle(),
				.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			};
		}

	private:

		void createCubemapImage(const TArray<TString, 6>& facePaths);
		void createImageView();
		void createSampler();

		void transitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout);

		int         m_Size = 0;
        
		VyImage     m_Image;
		VyImageView m_ImageView;
		VySampler   m_Sampler;
	};
}