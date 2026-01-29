// #include <VyEngine/GFX/Resources/Misc/LightBuffer.h>

// #include <VyEngine/VK/Context.h>

// namespace Vy
// {
//     void LightBuffer::create(U32 width, U32 height) 
//     {
//         m_Width  = width;
//         m_Height = height;

//         m_Image = MakeUnique<VyTexture>(
//             width, height,
//             HDR_FORMAT,
//             VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
//             VK_IMAGE_ASPECT_COLOR_BIT
//         );

//         Layout = VK_IMAGE_LAYOUT_UNDEFINED;
//     }


//     void LightBuffer::cleanup() 
//     {
//         if (m_Image) 
//         {
//             m_Image.reset();
//         }
        
//         Layout = VK_IMAGE_LAYOUT_UNDEFINED;
//     }
// }