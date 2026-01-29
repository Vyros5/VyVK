// #pragma once

// #include <VyEngine/VK/Image/Image.h>
// #include <VyEngine/VK/Image/ImageView.h>
// #include <VyEngine/GFX/Data/FrameInfo.h>

// namespace Vy
// {
//     class VyOffscreenRenderer final
//     {
//     public:
//         VyOffscreenRenderer(VkExtent2D extent, VkFormat format, U32 imageLayerCount = 1);
//         ~VyOffscreenRenderer();

//         VyOffscreenRenderer(const VyOffscreenRenderer&) = delete;
//         VyOffscreenRenderer& operator=(const VyOffscreenRenderer&) = delete;

//         VY_INLINE const VkFramebuffer frameBuffer() const { return m_Framebuffer; }
//         VY_INLINE VkRenderPass        renderPass()  const { return m_RenderPass; }

//         VY_INLINE float aspectRatio() const 
//         { 
//             return static_cast<float>(m_ImageExtent.width) / static_cast<float>(m_ImageExtent.height); 
//         };

//         VY_INLINE const U32 width()  const { return m_ImageExtent.width; }
//         VY_INLINE const U32 height() const { return m_ImageExtent.height; }
//         VY_INLINE const VkExtent2D extent() const { return m_ImageExtent; }
        
//         VY_INLINE bool isFrameInProgress() const 
//         { 
//             return m_IsFrameStarted; 
//         }
        
//         VY_INLINE VkImage offscreenImageHandle() const 
//         { 
//             return m_OffscreenImage.handle(); 
//         }
        
//         VY_INLINE const VyImage& offscreenImage() const 
//         { 
//             return m_OffscreenImage; 
//         }

//         VY_INLINE VkFormat format() const 
//         { 
//             return m_ImageFormat; 
//         }

//         VY_INLINE VkCommandBuffer currentCommandBuffer() const 
//         {
//             VY_ASSERT(m_IsFrameStarted, "Cannot get command buffer when frame not in progress");

//             return m_CommandBuffer;
//         }

//     public:
//         VkCommandBuffer beginFrame();
//         void endFrame();

//         void beginOffscreenPass(VkCommandBuffer cmdBuffer);
//         void endOffscreenPass(VkCommandBuffer cmdBuffer);

//         // VyBuffer prepareImageCopy(VkCommandBuffer cmdBuffer);

//         // TVector<U8> flushBufferToMemory(VyBuffer& stagingBuffer);

//     private:
//         // Renderer-Related
//         void createCommandBuffer();
//         void freeCommandBuffer();

//         // Swapchain-Related
//         void createOffscreenImage(U32 imageLayerCount);
//         void createRenderPass();
//         void createFramebuffer();

//     private:
//         VkFormat   m_ImageFormat;
//         VkExtent2D m_ImageExtent;

//         VkRenderPass  m_RenderPass;           
//         VkFramebuffer m_Framebuffer;

//         VyImage       m_OffscreenImage;    
//         VyImageView   m_OffscreenView;

//         VkCommandBuffer m_CommandBuffer;

//         // Unique<VyBuffer> m_Buffer;

//         bool m_IsFrameStarted = false;
//     };
// }