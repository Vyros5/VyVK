#pragma once

#include <VyEngine/VK/Buffer/Buffer.h>
#include <VyEngine/GFX/Resources/Texture/Texture.h>
#include <VyEngine/VK/Descriptors/Descriptors.h>

namespace Vy
{
	class HDRImage final
	{
	public: 
		HDRImage(const TString& filename);
		~HDRImage();


		HDRImage(const HDRImage&) = delete;
		HDRImage& operator=(const HDRImage&) = delete;
		HDRImage(HDRImage&&) = delete;
		HDRImage& operator=(HDRImage&&) = delete;


		const VkImageView& cubeMapView()       const { return m_CubeMapImageView.handle();  }
		const VkSampler&   cubeMapSampler()    const { return m_CubeMapSampler.handle();  }
		const VkImageView& irradianceView()    const { return m_IrradianceMapImageView; }
		const VkSampler&   irradianceSampler() const { return m_IrradianceMapSampler; }

	private: 

		void renderToCubeMap(
            const VkExtent2D&                extent, 
            U32                              mipLevels, 
            const TString&                   vertPath, 
            const TString&                   fragPath, 
            VyImage&                         inputImage, 
            const VkImageView&               inputImageView, 
            VkSampler                        inputSampler, 
            VkImage&                         outputCubeMapImage, 
            TArray<TVector<VkImageView>, 6>& outputCubeMapImageViews);

		void CreateIrradianceMap();

		// void TransitionImageLayout(VkImage image, VkFormat, VkImageLayout oldLayout, VkImageLayout newLayout, U32 mipLevels);
		void CreateCubeMap();
		void CreateEquirectImage(U32 width, U32 height, U32 miplevels, VkFormat format, VkImageUsageFlags usage);
		// void CreateEquirectTextureImageView();
		void CreateEquirectTextureSampler(VkFilter filter, VkSamplerAddressMode addressMode);

		static constexpr int m_FACE_COUNT = 6;

		// IMAGES
		VyImage        m_EquirectImage;
		VyImageView    m_EquirectImageView;
		VySampler      m_EquirectSampler;

		// U32            m_EquirectMipLevels{};
		// VkFormat       m_EquirectFormat = VK_FORMAT_UNDEFINED;
		// VkExtent2D     m_EquirectExtent{ 0, 0 };
		// VkImageLayout  m_EquirectImageLayout{ VK_IMAGE_LAYOUT_UNDEFINED };

		VyImage        m_CubeMapImage;
		VyImageView    m_CubeMapImageView;
		VySampler      m_CubeMapSampler;

		VkExtent2D     m_CubeMapExtent{ 1024, 1024 };
		TArray<TVector<VkImageView>, m_FACE_COUNT> m_CubeMapFaceViews;

		VyImage        m_IrradianceMapImage;
		VyImageView    m_IrradianceMapImageView;
		VySampler      m_IrradianceMapSampler;

		VkExtent2D     m_IrradianceMapExtent{ 32, 32 };
		TArray<VkImageView, m_FACE_COUNT> m_IrradianceMapFaceViews;

		const TString m_CubeVertPath = "Cube.vert.spv";
		const TString m_SkyFragPath  = "Sky.frag.spv";
		const TString m_IBLFragPath  = "ImageBasedLighting.frag.spv";
	};
}