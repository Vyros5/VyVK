// #include <VyEngine/GFX/Resources/Misc/GBuffer.h>

// #include <VyEngine/VK/Context.h>

// namespace Vy
// {
// 	void GBuffer::create(U32 width, U32 height)
// 	{
//         m_Width  = width;
//         m_Height = height; 

//         m_PositionImage = MakeUnique<VyTexture>(
//             width, height,
//             POS_FORMAT,
//             VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
//             VK_IMAGE_ASPECT_COLOR_BIT
//         );

//         m_NormalImage = MakeUnique<VyTexture>(
//             width, height,
//             NORM_FORMAT,
//             VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
//             VK_IMAGE_ASPECT_COLOR_BIT
//         );

//         m_AlbedoImage = MakeUnique<VyTexture>(
//             width, height,
//             ALBEDO_FORMAT,
//             VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
//             VK_IMAGE_ASPECT_COLOR_BIT
//         );

//         m_MetalRoughImage = MakeUnique<VyTexture>(
//             width, height,
//             METALROUGH_FORMAT,
//             VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
//             VK_IMAGE_ASPECT_COLOR_BIT
//         );
        
// 		m_OcclusionImage = MakeUnique<VyTexture>(
//             width, height,
//             OCCLUSION_FORMAT, 
//             VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
//             VK_IMAGE_ASPECT_COLOR_BIT
//         );
        
        
//         m_DepthImage = MakeUnique<VyTexture>(
//             width, height,
//             DEPTH_FORMAT,
//             VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, 
//             VK_IMAGE_ASPECT_DEPTH_BIT
//         ); 
        
//         PositionLayout   = VK_IMAGE_LAYOUT_UNDEFINED;
//         NormalLayout     = VK_IMAGE_LAYOUT_UNDEFINED;
//         AlbedoLayout     = VK_IMAGE_LAYOUT_UNDEFINED; 
//         MetalRoughLayout = VK_IMAGE_LAYOUT_UNDEFINED;
//         OcclusionLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
//         DepthLayout      = VK_IMAGE_LAYOUT_UNDEFINED; 
// 	}


//     void GBuffer::cleanup() 
//     {
//         // Destroy each G-buffer attachment in turn:
//         if (m_PositionImage) 
//         {
//             m_PositionImage.reset();
//         }
//         if (m_NormalImage) 
//         {
//             m_NormalImage.reset();
//         }
//         if (m_AlbedoImage) 
//         {
//             m_AlbedoImage.reset();
//         }
//         if (m_DepthImage) 
//         {
//             m_DepthImage.reset();
//         }
//         if (m_MetalRoughImage)
//         {
//             m_MetalRoughImage.reset();
//         }
//         if (m_OcclusionImage)
//         {
//             m_OcclusionImage.reset();
//         }


//         // Reset layouts (optional, but keeps state clean):
//         PositionLayout   = VK_IMAGE_LAYOUT_UNDEFINED;
//         NormalLayout     = VK_IMAGE_LAYOUT_UNDEFINED;
//         AlbedoLayout     = VK_IMAGE_LAYOUT_UNDEFINED;
//         DepthLayout      = VK_IMAGE_LAYOUT_UNDEFINED;
//         MetalRoughLayout = VK_IMAGE_LAYOUT_UNDEFINED;
//         OcclusionLayout  = VK_IMAGE_LAYOUT_UNDEFINED;  
//     }
// }