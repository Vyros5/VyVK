#pragma once

#include <VyEngine/VK/Buffer/Buffer.h>
#include <VyEngine/GFX/Resources/Texture/Texture.h>
#include <VyEngine/VK/Descriptors/Descriptors.h>

namespace Vy
{
	class HDRImage final
	{
	public: 
		HDRImage();
		~HDRImage();


		HDRImage(const HDRImage&) = delete;
		HDRImage& operator=(const HDRImage&) = delete;

		HDRImage(HDRImage&&) = delete;
		HDRImage& operator=(HDRImage&&) = delete;

		void loadHDR(const TString& filename);

        void RenderToCubemap(
			VyImage&               inputImage, 
			const VkImageView&     inputView, 
			VkSampler              inputSampler,
			VkImage&               outputImage, 
			TArray<VkImageView, 6> outPutaceViews, 
			U32                    size,
			const TString&         vertPath, 
			const TString&         fragPath
		);

		// const VkImage&     equirectImage()      const { return m_Equirect.Image.handle(); }
		// const VkImageView& equirectImageView()  const { return m_Equirect.ImageView.handle(); }
		// const VkSampler&   equirectSampler()    const { return m_Equirect.Sampler.handle(); }

		// const TArray<TVector<VkImageView>, 6>& cubeMapFaceViews() const { return m_CubeMap.FaceViews; }
		
		// const VkImage&     cubeMapImage()       const { return m_CubeMap.Image.handle(); }
		// const VkImageView& cubeMapView()        const { return m_CubeMap.ImageView.handle();  }
		// const VkSampler&   cubeMapSampler()     const { return m_CubeMap.Sampler.handle();  }
		
		// const VkImage&     irradianceMapImage() const { return m_IrradianceMap.Image.handle(); }
		// const VkImageView& irradianceView()     const { return m_IrradianceMap.ImageView.handle(); }
		// const VkSampler&   irradianceSampler()  const { return m_IrradianceMap.Sampler.handle(); }

	private: 

		void createCubeMap();
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

		void createIrradianceMap();

		void createEquirectImage(U32 width, U32 height, U32 miplevels, VkFormat format, VkImageUsageFlags usage);
		// void createEquirectImage(U32 width, U32 height, U32 miplevels, VkFormat format, VkImageUsageFlags usage, VmaMemoryUsage memoryUsage);
		void createEquirectTextureSampler(VkFilter filter, VkSamplerAddressMode addressMode);
		// void generateMipmaps(VkImage image, VkFormat format, U32 width, U32 height, U32 mipLevels, U32 arrayLevels) const;

		void generatePrefilteredEnvMap();
		// void TransitionImageLayout(VkImage image, VkFormat, VkImageLayout oldLayout, VkImageLayout newLayout, U32 mipLevels);

		// static constexpr int m_FACE_COUNT = 6;

		// IMAGES
		// VyImage        m_EquirectImage;
		// VyImageView    m_EquirectImageView;
		// VySampler      m_EquirectSampler;

		// // U32            m_EquirectMipLevels{};
		// // VkFormat       m_EquirectFormat = VK_FORMAT_UNDEFINED;
		// // VkExtent2D     m_EquirectExtent{ 0, 0 };
		// // VkImageLayout  m_EquirectImageLayout{ VK_IMAGE_LAYOUT_UNDEFINED };

		// VyImage        m_CubeMapImage;
		// VyImageView    m_CubeMapImageView;
		// VySampler      m_CubeMapSampler;

		// VkExtent2D     m_CubeMapExtent{ 1024, 1024 };
		// TArray<TVector<VkImageView>, m_FACE_COUNT> m_CubeMapFaceViews;

		// VyImage        m_IrradianceMapImage;
		// VyImageView    m_IrradianceMapImageView;
		// VySampler      m_IrradianceMapSampler;

		// VkExtent2D     m_IrradianceMapExtent{ 32, 32 };
		// TArray<VkImageView, m_FACE_COUNT> m_IrradianceMapFaceViews;

		const TString m_CubeVertPath = "Cube.vert.spv";
		const TString m_SkyFragPath  = "Sky.frag.spv";
		const TString m_IBLFragPath  = "ImageBasedLighting.frag.spv";

		static constexpr int m_FACE_COUNT = 6;
		// IMAGES
		// struct Equirect
		// {
		// 	VyImage       Image;
		// 	VyImageView   ImageView;
		// 	VySampler     Sampler;
		// 	U32           MipLevels{};
		// 	VkFormat      Format = VK_FORMAT_UNDEFINED;
		// 	VkExtent2D    Extent{ 0, 0 };
		// 	VkImageLayout ImageLayout{ VK_IMAGE_LAYOUT_UNDEFINED };

		// } m_Equirect;

		// struct Cubemap
		// {
		// 	VyImage                                    Image;
		// 	VyImageView                                ImageView;
		// 	VySampler                                  Sampler;
		// 	VkExtent2D                                 Extent{ 512, 512 };
		// 	TArray<TVector<VkImageView>, m_FACE_COUNT> FaceViews;

		// } m_CubeMap;

		// struct IrradianceMap
		// {
		// 	VyImage                           Image;
		// 	VyImageView                       ImageView;
		// 	VySampler                         Sampler;
		// 	VkExtent2D                        Extent{ 32, 32 };
		// 	TArray<VkImageView, m_FACE_COUNT> FaceViews;

		// } m_IrradianceMap;


        // HDRI image
		struct HDR
		{
			VyImage             Image;
			// VyImageView  View;
			Unique<VyImageView> View   { nullptr };
			Unique<VySampler>   Sampler{ nullptr };
			// VkExtent2D          Extent{ 1024, 1024 };

		} m_HDR;

        // Sampler m_sampler{m_device, VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, 1};

        // Cube map (skybox)
		struct Cubemap
		{
			using ViewArray = TArray<VkImageView, m_FACE_COUNT>;

			VyImage      Image;
			VyImageView  View;
			// VySampler    Sampler;
			// VkExtent2D   Extent{ 1024, 1024 }; 
			U32          Size{ 1024u }; // Default size for cube map

			ViewArray FaceViews;

		} m_CubeMap;

		struct DiffuseIrradianceMap
		{
			using ViewArray = TArray<VkImageView, m_FACE_COUNT>;

			VyImage      Image;
			VyImageView  View;
			// VySampler    Sampler;
			// VkExtent2D   Extent{ 256, 256 }; 
			U32          Size{ 256u }; // Default size for diffuse irradiance map

			ViewArray FaceViews{};

		} m_IrradianceMap;




		// CUBE
		// const Vec3 m_EYE = Vec3(0.0f);
		
        const TArray<Mat4, 6> m_ViewMatrices = {
            // POSITIVE_X
            glm::lookAt(Vec3(0.0f), Vec3( 1.0f,  0.0f,  0.0f), Vec3( 0.0f, -1.0f,  0.0f)), // +X
            // NEGATIVE_X
            glm::lookAt(Vec3(0.0f), Vec3(-1.0f,  0.0f,  0.0f), Vec3( 0.0f, -1.0f,  0.0f)), // -X
            // POSITIVE_Y
            glm::lookAt(Vec3(0.0f), Vec3( 0.0f,  1.0f,  0.0f), Vec3( 0.0f,  0.0f,  1.0f)), // +Y
            // NEGATIVE_Y
            glm::lookAt(Vec3(0.0f), Vec3( 0.0f, -1.0f,  0.0f), Vec3( 0.0f,  0.0f, -1.0f)), // -Y
            // POSITIVE_Z
            glm::lookAt(Vec3(0.0f), Vec3( 0.0f,  0.0f,  1.0f), Vec3( 0.0f, -1.0f,  0.0f)), // +Z
            // NEGATIVE_Z
            glm::lookAt(Vec3(0.0f), Vec3( 0.0f,  0.0f, -1.0f), Vec3( 0.0f, -1.0f,  0.0f))  // -Z
        };

		Mat4 m_Projection{};

		// const Mat4 m_CAPTURE_VIEWS[ m_FACE_COUNT ] =
		// {
		// 	glm::lookAt(m_EYE, m_EYE + Vec3( 1.0f,  0.0f,  0.0f), Vec3( 0.0f, -1.0f,  0.0f)), // +X
		// 	glm::lookAt(m_EYE, m_EYE + Vec3(-1.0f,  0.0f,  0.0f), Vec3( 0.0f, -1.0f,  0.0f)), // -X
		// 	glm::lookAt(m_EYE, m_EYE + Vec3( 0.0f, -1.0f,  0.0f), Vec3( 0.0f,  0.0f, -1.0f)), // -Y
		// 	glm::lookAt(m_EYE, m_EYE + Vec3( 0.0f,  1.0f,  0.0f), Vec3( 0.0f,  0.0f,  1.0f)), // +Y
		// 	glm::lookAt(m_EYE, m_EYE + Vec3( 0.0f,  0.0f,  1.0f), Vec3( 0.0f, -1.0f,  0.0f)), // +Z
		// 	glm::lookAt(m_EYE, m_EYE + Vec3( 0.0f,  0.0f, -1.0f), Vec3( 0.0f, -1.0f,  0.0f))  // -Z
		// };
		
		// Mat4 m_CAPTURE_PROJECTION;
	};
}