// #pragma once

// #include <VyEngine/VK/Buffer/Buffer.h>
// #include <VyEngine/GFX/Resources/Texture/Texture.h>
// #include <VyEngine/VK/Descriptors/Descriptors.h>

// namespace Vy
// {
// 	class GBuffer final
// 	{
// 	public:
// 		// G-buffer attachment formats
// 		static constexpr VkFormat POS_FORMAT        = VK_FORMAT_R16G16B16A16_SFLOAT; 
// 		static constexpr VkFormat NORM_FORMAT       = VK_FORMAT_R8G8B8A8_UNORM;   
// 		static constexpr VkFormat ALBEDO_FORMAT     = VK_FORMAT_R8G8B8A8_SRGB; // USE THIS FORMAT FOR OTHER TEXTURES PLEASE
// 		static constexpr VkFormat METALROUGH_FORMAT = VK_FORMAT_R8G8B8A8_UNORM; //r metal, g roughness
// 		static constexpr VkFormat OCCLUSION_FORMAT  = VK_FORMAT_R8G8B8A8_SRGB; 
// 		static constexpr VkFormat DEPTH_FORMAT      = VK_FORMAT_D32_SFLOAT;

// 		void create(U32 width, U32 height);
// 		void cleanup();

// 		VkImageView positionView()      const { return m_PositionImage->imageViewHandle();  };
// 		VkImage     positionImage()     const { return m_PositionImage->imageHandle();  }
// 		VkSampler   positionSampler()   const { return m_PositionImage->samplerHandle(); }

// 		VkImageView normalView()        const { return  m_NormalImage->imageViewHandle();  };
// 		VkImage     normalImage()       const { return m_NormalImage->imageHandle();  }
// 		VkSampler   normalSampler()     const { return m_NormalImage->samplerHandle(); } 
		
//         VkImageView albedoSpecView()    const { return  m_AlbedoImage->imageViewHandle();  };
// 		VkImage     albedoSpecImage()   const { return m_AlbedoImage->imageHandle();  }
// 		VkSampler   albedoSpecSampler() const { return m_AlbedoImage->samplerHandle(); }
		
//         VkImageView depthView()         const { return  m_DepthImage->imageViewHandle();  };
// 		VkImage     depthImage()        const { return m_DepthImage->imageHandle();  }
// 		VkSampler   depthSampler()      const { return m_DepthImage->samplerHandle(); }

// 		VkImageView metalRoughView()    const { return m_MetalRoughImage->imageViewHandle(); }
// 		VkImage     metalRoughImage()   const { return m_MetalRoughImage->imageHandle(); }
// 		VkSampler   metalRoughSampler() const { return m_MetalRoughImage->samplerHandle(); }
        
// 		VkImageView occlusionView()     const { return m_OcclusionImage->imageViewHandle(); }
// 		VkImage     occlusionImage()    const { return m_OcclusionImage->imageHandle(); }
// 		VkSampler   occlusionSampler()  const { return m_OcclusionImage->samplerHandle(); }
        

// 		U32         width()  const { return m_Width;  }
// 		U32         height() const { return m_Height;  }
//         VkExtent2D  extent() const { return { m_Width, m_Height }; }

// 		VkImageLayout PositionLayout;
// 		VkImageLayout NormalLayout;
// 		VkImageLayout AlbedoLayout;
// 		VkImageLayout DepthLayout;
// 		VkImageLayout MetalRoughLayout; 
// 		VkImageLayout OcclusionLayout; 
//     private:

// 		Unique<VyTexture> m_PositionImage;
// 		Unique<VyTexture> m_NormalImage;
// 		Unique<VyTexture> m_AlbedoImage;
// 		Unique<VyTexture> m_DepthImage;
// 		Unique<VyTexture> m_MetalRoughImage;
// 		Unique<VyTexture> m_OcclusionImage;

// 		U32 m_Width;
//         U32 m_Height;
// 	};
// }