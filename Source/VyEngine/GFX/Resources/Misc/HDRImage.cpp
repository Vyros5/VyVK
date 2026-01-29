#include <VyEngine/GFX/Resources/Misc/HDRImage.h>

#include <VyEngine/VK/Context.h>

#include <VyEngine/VK/Pipeline/Pipeline.h>
#include <VyEngine/VK/Descriptors/Descriptors.h>

#include "stb_image.h"

namespace Vy
{
	HDRImage::HDRImage() :
        m_Projection(glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f))
	{
        // m_Projection[1][1] *= -1.0f; 
    }


    void HDRImage::loadHDR(const TString& filename)
    {
        const VkFormat hdrFormat = VK_FORMAT_R32G32B32A32_SFLOAT;

        // Load the HDR pixels with stb_image
        if (!std::filesystem::exists(filename)) 
        {
            throw std::runtime_error("File does not exist: " + filename);
        }
        if (!stbi_is_hdr(filename.c_str())) 
        {
            throw std::runtime_error("File is not HDR format: " + filename);
        }

        int texW, texH, texChannels;
        float* pPixels = stbi_loadf(filename.c_str(), &texW, &texH, &texChannels, STBI_rgb_alpha);
        
        if (!pPixels) 
        {
            throw std::runtime_error("Failed to load HDR image: " + filename);
        }

        const VkDeviceSize imageSize = static_cast<VkDeviceSize>(texW * texH * 4 * sizeof(float));

        // Create host visible staging buffer
        VyBuffer stagingBuffer = VyBuffer::stagingBuffer("hdri", imageSize );

        // Copy pixels into the staging buffer
        stagingBuffer.write(pPixels);

        stbi_image_free(pPixels);

        // Create the equirectangular image
        m_HDR.Image = VyImage::Builder{}
			.setName       ("hdr")
            .setImageType  (VK_IMAGE_TYPE_2D)
            .setFormat     (hdrFormat)
            .setExtent     (static_cast<U32>(texW), static_cast<U32>(texH))
            .setLevels     (1)
			.setLayers     (1)
			.setSamples    (VK_SAMPLE_COUNT_1_BIT)
            .setTiling     (VK_IMAGE_TILING_OPTIMAL)
			.setLayout     (VK_IMAGE_LAYOUT_UNDEFINED)
            .setUsage      (VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
            .setMemoryUsage(VMA_MEMORY_USAGE_AUTO)
        	.build();

        m_HDR.View = VyImageView::Builder{}
			.setName    ("hdr")
            .setViewType(VK_IMAGE_VIEW_TYPE_2D)
            .setFormat  (hdrFormat)
            .setAspect  ( VKUtil::aspectFromFormat( hdrFormat ))
            .setLevels  (0, 1)
            .setLayers  (0, 1)
        	.buildPtr( m_HDR.Image );


        // Transition image to TRANSFER_DST_OPTIMAL, copy, then to SHADER_READ_ONLY_OPTIMAL
        m_HDR.Image.copyFrom(stagingBuffer, true);

        // Create the sampler
        m_HDR.Sampler = VySampler::Builder{}
            .setName         ("equirect")
            .setFilters      (VK_FILTER_LINEAR)
            .setMipmapMode   (VK_SAMPLER_MIPMAP_MODE_LINEAR)
            .setWrap         (VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE)
            .setBorder       (VK_BORDER_COLOR_INT_OPAQUE_BLACK)
            .enableAnisotropy(true)
            .setLodRange     (0.0f, 1.0f)
            .setMipLodBias   (0.0f)
            .buildPtr();
	}


    HDRImage::~HDRImage()
	{
    //     vkDestroySampler(VyContext::device(), m_EquirectSampler, nullptr);
    //     vkDestroyImageView(VyContext::device(), m_EquirectImageView, nullptr);
    //     vkDestroyImage(VyContext::device(), m_EquirectImage, nullptr);

        // vkDestroySampler(VyContext::device(), m_CubeMapSampler, nullptr);
        // vkDestroyImageView(VyContext::device(), m_CubeMapImageView, nullptr);
        // for (auto& faceViews : m_CubeMap.FaceViews)
        // {
        //     for (auto v : faceViews)
        //     {
        //         vkDestroyImageView(VyContext::device(), v, nullptr);
        //     }
        // }

        // // Destroy irradiance map face views
        // for (auto& view : m_IrradianceMap.FaceViews) 
        // {
        //     if (view != VK_NULL_HANDLE) 
        //     {
        //         vkDestroyImageView(VyContext::device(), view, nullptr);
        //     }
        // }

		// vkDestroySampler(VyContext::device(), m_IrradianceMapSampler, nullptr);
        // vkDestroyImageView(VyContext::device(), m_IrradianceMapImageView, nullptr);
        // vkDestroyImage(VyContext::device(), m_IrradianceMapImage, nullptr);
        // vkDestroyImage(VyContext::device(), m_CubeMapImage, nullptr);
	}



    // BASE IMAGE CREATION

    // void HDRImage::createEquirectImage(
    //     U32               width,
    //     U32               height,
    //     U32               miplevels,
    //     VkFormat          format,
    //     VkImageUsageFlags usage)
    // {
    //     m_Equirect.Image = VyImage::Builder{}
	// 		.setName       ("equirect")
    //         .setImageType  (VK_IMAGE_TYPE_2D)
    //         .setFormat     (format)
    //         .setExtent     (width, height)
    //         .setLevels     (1)
	// 		.setLayers     (miplevels)
	// 		.setSamples    (VK_SAMPLE_COUNT_1_BIT)
    //         .setTiling     (VK_IMAGE_TILING_OPTIMAL)
	// 		.setLayout     (VK_IMAGE_LAYOUT_UNDEFINED)
    //         .setUsage      (usage)
    //         .setMemoryUsage(VMA_MEMORY_USAGE_AUTO)
    //     	.build();

    //     m_Equirect.ImageView = VyImageView::Builder{}
	// 		.setName    ("equirect")
    //         .setViewType(VK_IMAGE_VIEW_TYPE_2D)
    //         .setFormat  (format)
    //         .setAspect  ( VKUtil::aspectFromFormat( format ))
    //         .setLevels  (0, miplevels)
    //         .setLayers  (0, 1)
    //     	.build( m_Equirect.Image );
    // }


    // void HDRImage::createEquirectTextureSampler(VkFilter filter, VkSamplerAddressMode addressMode)
    // {
    //     m_Equirect.Sampler = VySampler::Builder{}
    //         .setName         ("equirect")
    //         .setFilters      (filter)
    //         .setMipmapMode   (VK_SAMPLER_MIPMAP_MODE_LINEAR)
    //         .setWrap         (addressMode)
    //         .setBorder       (VK_BORDER_COLOR_INT_OPAQUE_BLACK)
    //         .enableAnisotropy(true)
    //         .setLodRange     (0.0f, 1.0f)
    //         .setMipLodBias   (0.0f)
    //         .build();
    // }

    void HDRImage::generatePrefilteredEnvMap()
    {

    }


    void HDRImage::createCubeMap()
    {
        // We only need one mip level for now
        const U32 kMipLevels = static_cast<U32>(std::floor(std::log2(m_CubeMap.Size))) + 1;
        // const U32 kMipLevels = 1;

        m_CubeMap.Image = VyImage::Builder{}
			.setName       ("cubemap")
            .setImageType  (VK_IMAGE_TYPE_2D)
            .setFormat     (m_HDR.Image.format())
            .setExtent     (m_CubeMap.Size, m_CubeMap.Size)
            .setLevels     (kMipLevels)
			.setLayers     (m_FACE_COUNT) // 6 Faces
			.setSamples    (VK_SAMPLE_COUNT_1_BIT)
            .setTiling     (VK_IMAGE_TILING_OPTIMAL)
			.setLayout     (VK_IMAGE_LAYOUT_UNDEFINED)
            .setUsage      (
                  VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
                | VK_IMAGE_USAGE_SAMPLED_BIT
                | VK_IMAGE_USAGE_TRANSFER_DST_BIT
                | VK_IMAGE_USAGE_TRANSFER_SRC_BIT
            )
            .setMemoryUsage(VMA_MEMORY_USAGE_AUTO)
            .setFlags      (VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT) // Create a cubemap
        	.build();

        m_CubeMap.View = VyImageView::Builder{}
			.setName    ("cubemap")
            .setViewType(VK_IMAGE_VIEW_TYPE_CUBE)
            .setFormat  (m_HDR.Image.format())
            .setAspect  (VK_IMAGE_ASPECT_COLOR_BIT)
            .setLevels  (0, 1)
            .setLayers  (0, m_FACE_COUNT)
        	.build( m_CubeMap.Image );


        // Create per-face 2D views (for rendering each face)
        for (U32 face = 0; face < m_FACE_COUNT; ++face) 
        {
            VyImageView faceView = VyImageView::Builder{}
                .setName    (std::format("cubemap_face_{}", face))
                .setViewType(VK_IMAGE_VIEW_TYPE_2D)
                .setFormat  (m_HDR.Image.format())
                .setAspect  (VK_IMAGE_ASPECT_COLOR_BIT)
                .setLevels  (0,    1)
                .setLayers  (face, 1)
                .build( m_CubeMap.Image );

            m_CubeMap.FaceViews[ face ] = faceView.handle();
        }

        // Create the sampler
        // m_CubeMap.Sampler = VySampler::Builder{}
        //     .setName         ("cubemap")
        //     .setFilters      (VK_FILTER_LINEAR)
        //     .setMipmapMode   (VK_SAMPLER_MIPMAP_MODE_LINEAR)
        //     .setWrap         (VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE)
        //     .setBorder       (VK_BORDER_COLOR_INT_OPAQUE_BLACK)
        //     .enableAnisotropy(true)
        //     .setLodRange     (0.0f, static_cast<float>(kMipLevels))
        //     .setMipLodBias   (0.0f)
        //     .build();

        // Finally render into it
        RenderToCubemap(
            m_HDR.Image,
            m_HDR.View->handle(),
            m_HDR.Sampler->handle(),
            m_CubeMap.Image,
            m_CubeMap.FaceViews,
            m_CubeMap.Size,
            m_CubeVertPath,
            m_SkyFragPath
        );
    }


    void HDRImage::RenderToCubemap(
        VyImage&               inputImage, 
        const VkImageView&     inputView, 
        VkSampler              inputSampler,
        VkImage&               outputImage, 
        TArray<VkImageView, 6> outputFaceViews, 
        U32                    size,
        const TString&         vertPath, 
        const TString&         fragPath
    )
    {
        // Transition the equirectangular input to SHADER_READ_ONLY_OPTIMAL if needed
        if (m_HDR.Image.layout() != VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) 
        {
            m_HDR.Image.transitionLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        }

        // Begin one-time command buffer
        VkCommandBuffer cmdBuffer = VyContext::beginCommands();

        // Transition the entire cube-map image into COLOR_ATTACHMENT_OPTIMAL
        {
            VkImageMemoryBarrier barrier{ VKInit::imageMemoryBarrier() };
            {
                barrier.image                           = outputImage; // Cubemap Image
                barrier.oldLayout                       = VK_IMAGE_LAYOUT_UNDEFINED;
                barrier.newLayout                       = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                barrier.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
                barrier.subresourceRange.baseMipLevel   = 0;
                barrier.subresourceRange.levelCount     = 1;
                barrier.subresourceRange.baseArrayLayer = 0;
                barrier.subresourceRange.layerCount     = m_FACE_COUNT;
                barrier.srcAccessMask                   = 0;
                barrier.dstAccessMask                   = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            }

            vkCmdPipelineBarrier(cmdBuffer,
                VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                0,
                0, nullptr,
                0, nullptr,
                1, &barrier
            );
        }

        // Build a one-time descriptor set to sample the equirectangular map.
        Unique<VyDescriptorSetLayout> setLayout = VyDescriptorSetLayout::Builder{}
            .setName   ("hdri")
            .addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
            .buildPtr();

        Unique<VyDescriptorPool> descPool = VyDescriptorPool::Builder{}
            .setName    ("hdri")
            .setMaxSets (1)
            .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1)
            .buildPtr();

        struct PushConstants 
        {
            Mat4 View;
            Mat4 Projection;
        };

        // Create pipeline
        auto builder = VyPipeline::GraphicsBuilder{};
        {
            builder.setName( "cubemap" );
            
            builder.addDescriptorSetLayout( setLayout->handle() );
            
            builder.addPushConstantRange(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(PushConstants));
            
            builder.addShaderStage(VK_SHADER_STAGE_VERTEX_BIT,   vertPath);
            builder.addShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT, fragPath);
            
            builder.addColorAttachment( inputImage.format() );
            // builder.setDepthAttachment(VK_FORMAT_UNDEFINED);

            builder.setCullMode(VK_CULL_MODE_NONE);

            builder.clearVertexDescriptions();
        }

        auto pipeline = builder.buildPtr();

        // Write descriptor image.
        VkDescriptorImageInfo inputInfo{};
        {
            inputInfo.sampler     = inputSampler;
            inputInfo.imageView   = inputView;
            inputInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        }

        VkDescriptorSet descriptorSet = setLayout->allocate();

        VyDescriptorWriter{ *setLayout, *descPool }
            .writeImage( 0, &inputInfo )
            .update( descriptorSet );

        VkExtent2D extent = { size, size };

        // Render each of the six faces
        for (U32 face = 0; face < m_FACE_COUNT; ++face) 
        {
            // The barrier for this layer is already handled above by the 6-layer barrier.

            PushConstants push{};
            {
                push.View       = m_ViewMatrices[ face ];
                push.Projection = m_Projection;
            }

            // Dynamic rendering begin
            VkRenderingAttachmentInfoKHR colorAttachment{ VKInit::renderingAttachmentInfoKHR() };
            {
                colorAttachment.imageView        = outputFaceViews[ face ];
                colorAttachment.imageLayout      = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                colorAttachment.loadOp           = VK_ATTACHMENT_LOAD_OP_CLEAR;
                colorAttachment.storeOp          = VK_ATTACHMENT_STORE_OP_STORE;
                colorAttachment.clearValue.color = { 0.0f, 0.0f, 0.0f, 1.0f };
            }

            VkRenderingInfoKHR renderingInfo{ VKInit::renderingInfoKHR() };
            {
                renderingInfo.renderArea.extent    = extent;
                renderingInfo.renderArea.offset    = { 0, 0 };
                renderingInfo.layerCount           = 1;
                renderingInfo.colorAttachmentCount = 1;
                renderingInfo.pColorAttachments    = &colorAttachment;
            }

            vkCmdBeginRenderingKHR( cmdBuffer, &renderingInfo );
            {
                // Set viewport & scissor
                VKCmd::viewport(cmdBuffer, extent);
                VKCmd::scissor (cmdBuffer, extent);

                // Bind pipeline + descriptor
                pipeline->bind( cmdBuffer );

                // Set 0
                pipeline->bindDescriptorSet(cmdBuffer, 0, descriptorSet);

                // Push constants
                pipeline->pushConstants(cmdBuffer, VK_SHADER_STAGE_VERTEX_BIT, &push, sizeof(PushConstants));

                // Draw a 36-vertex cube (generated in vert shader)
                vkCmdDraw(cmdBuffer, 36, 1, 0, 0);
            }
            vkCmdEndRenderingKHR( cmdBuffer );
        }

        // Finally transition the cube to SHADER_READ_ONLY_OPTIMAL
        {
            VkImageMemoryBarrier barrier{ VKInit::imageMemoryBarrier() };
            {
                barrier.oldLayout                       = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                barrier.newLayout                       = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                barrier.srcAccessMask                   = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                barrier.dstAccessMask                   = VK_ACCESS_SHADER_READ_BIT;
                barrier.image                           = outputImage;
                barrier.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
                barrier.subresourceRange.baseMipLevel   = 0;
                barrier.subresourceRange.levelCount     = 1;
                barrier.subresourceRange.baseArrayLayer = 0;
                barrier.subresourceRange.layerCount     = m_FACE_COUNT;
            } 

            vkCmdPipelineBarrier(cmdBuffer,
                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                0, 
                0, nullptr, 
                0, nullptr, 
                1, &barrier
            );
        }

        // End and submit
        VyContext::endCommands(cmdBuffer);
    }

    
    // void HDRImage::renderToCubeMap(
    //     const VkExtent2D&                extent, 
    //     U32                              mipLevels, 
    //     const TString&                   vertPath, 
    //     const TString&                   fragPath, 
    //     VyImage&                         inputImage, 
    //     const VkImageView&               inputImageView, 
    //     VkSampler                        inputSampler, 
    //     VkImage&                         outputCubeMapImage, 
    //     TArray<TVector<VkImageView>, 6>& outputCubeMapImageViews)
	// {
    //     // Transition the equirectangular input to SHADER_READ_ONLY_OPTIMAL if needed
    //     if (m_Equirect.Image.layout() != VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) 
    //     {
    //         m_Equirect.Image.transitionLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    //     }

    //     // Begin one-off command buffer
    //     VkCommandBuffer cmdBuffer = VyContext::beginCommands();

    //     // Transition the entire cube-map image into COLOR_ATTACHMENT_OPTIMAL
    //     {
    //         VkImageMemoryBarrier barrier{ VKInit::imageMemoryBarrier() };
    //         {
    //             barrier.image                           = outputCubeMapImage;
    //             barrier.oldLayout                       = VK_IMAGE_LAYOUT_UNDEFINED;
    //             barrier.newLayout                       = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    //             barrier.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    //             barrier.subresourceRange.baseMipLevel   = 0;
    //             barrier.subresourceRange.levelCount     = 1;
    //             barrier.subresourceRange.baseArrayLayer = 0;
    //             barrier.subresourceRange.layerCount     = m_FACE_COUNT;
    //             barrier.srcAccessMask                   = 0;
    //             barrier.dstAccessMask                   = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    //         }

    //         vkCmdPipelineBarrier(cmdBuffer,
    //             VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
    //             VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
    //             0,
    //             0, nullptr,
    //             0, nullptr,
    //             1, &barrier
    //         );
    //     }

    //     // Build a one-off descriptor set to sample the equirectangular map.
    //     Unique<VyDescriptorSetLayout> setLayout = VyDescriptorSetLayout::Builder{}
    //         .addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
    //         .buildPtr();

    //     Unique<VyDescriptorPool> descPool = VyDescriptorPool::Builder{}
    //         .setMaxSets (1)
    //         .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1)
    //         .buildPtr();

    //     VkDescriptorImageInfo inputInfo{};
    //     {
    //         inputInfo.sampler     = inputSampler;
    //         inputInfo.imageView   = inputImageView;
    //         inputInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    //     }

    //     VkDescriptorSet descriptorSet = setLayout->allocate();

    //     VyDescriptorWriter{ *setLayout, *descPool }
    //         .writeImage( 0, &inputInfo )
    //         .update( descriptorSet );

    //     // Create pipeline layout with 2×mat4 push-constants + our single descriptor set
    //     auto builder = VyPipeline::GraphicsBuilder{};
    //     {
    //         builder.setName( "cubemap" );
            
    //         builder.addDescriptorSetLayout( setLayout->handle() );
            
    //         builder.addPushConstantRange(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(Mat4) * 2 /* view + proj*/);
            
    //         builder.addShaderStage(VK_SHADER_STAGE_VERTEX_BIT,   vertPath);
    //         builder.addShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT, fragPath);
            
    //         builder.addColorAttachment( inputImage.format() );
    //         builder.setDepthAttachment(VK_FORMAT_UNDEFINED);

    //         builder.setCullMode(VK_CULL_MODE_NONE);

    //         builder.clearVertexDescriptions();
    //     }

    //     auto pipeline = builder.buildPtr();

    //     // Render each of the six faces
    //     for (U32 face = 0; face < m_FACE_COUNT; ++face) 
    //     {
    //         // The barrier for this layer is already handled above by the 6-layer barrier

    //         // Dynamic rendering begin
    //         VkRenderingAttachmentInfo colorAttachment{ VKInit::renderingAttachmentInfo() };
    //         {
    //             colorAttachment.imageView        = outputCubeMapImageViews[face][0];
    //             colorAttachment.imageLayout      = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    //             colorAttachment.loadOp           = VK_ATTACHMENT_LOAD_OP_CLEAR;
    //             colorAttachment.storeOp          = VK_ATTACHMENT_STORE_OP_STORE;
    //             colorAttachment.clearValue.color = { 0, 0, 0, 1 };
    //         }

    //         VkRenderingInfo renderingInfo{ VKInit::renderingInfo() };
    //         {
    //             renderingInfo.renderArea.extent    = extent;
    //             renderingInfo.layerCount           = 1;
    //             renderingInfo.colorAttachmentCount = 1;
    //             renderingInfo.pColorAttachments    = &colorAttachment;
    //         }

    //         vkCmdBeginRendering(cmdBuffer, &renderingInfo);
    //         {
    //             // Set viewport & scissor
    //             VKCmd::viewport(cmdBuffer, extent);
    //             VKCmd::scissor (cmdBuffer, extent);

    //             // Bind pipeline + descriptor
    //             pipeline->bind(cmdBuffer);

    //             pipeline->bindDescriptorSet(cmdBuffer, 0, descriptorSet);

    //             // Push constants: view then proj
    //             pipeline->pushConstants(cmdBuffer, VK_SHADER_STAGE_VERTEX_BIT, &m_CAPTURE_PROJECTION, sizeof(Mat4));
    //             pipeline->pushConstants(cmdBuffer, VK_SHADER_STAGE_VERTEX_BIT, &m_CAPTURE_PROJECTION, sizeof(Mat4), sizeof(Mat4));

    //             // Draw a 36-vertex cube (generated in your vert shader)
    //             vkCmdDraw(cmdBuffer, 36, 1, 0, 0);
    //         }
    //         vkCmdEndRendering(cmdBuffer);
    //     }

    //     // Finally transition the cube to SHADER_READ_ONLY_OPTIMAL
    //     {
    //         VkImageMemoryBarrier barrier{ VKInit::imageMemoryBarrier() };
    //         {
    //             barrier.oldLayout        = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    //             barrier.newLayout        = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    //             barrier.srcAccessMask    = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    //             barrier.dstAccessMask    = VK_ACCESS_SHADER_READ_BIT;
    //             barrier.image            = outputCubeMapImage;
    //             barrier.subresourceRange = { 
    //                 VK_IMAGE_ASPECT_COLOR_BIT, 
    //                 0, mipLevels, 
    //                 0, m_FACE_COUNT 
    //             };
    //         } 

    //         vkCmdPipelineBarrier(cmdBuffer,
    //             VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
    //             VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
    //             0, 
    //             0, nullptr, 
    //             0, nullptr, 
    //             1, &barrier
    //         );
    //     }

    //     // End and submit
    //     VyContext::endCommands(cmdBuffer);
	// }



    void HDRImage::createIrradianceMap()
    {
        U32 irradianceMipLevels = 1;

        // Create the irradiance cubemap (lower resolution)
        {
            m_IrradianceMap.Image = VyImage::Builder{}
                .setName       ("irradiance")
                .setImageType  (VK_IMAGE_TYPE_2D)
                .setFormat     (m_HDR.Image.format())
                .setExtent     (m_IrradianceMap.Size, m_IrradianceMap.Size)
                .setLevels     (irradianceMipLevels)
                .setLayers     (m_FACE_COUNT) // 6 Faces
                .setSamples    (VK_SAMPLE_COUNT_1_BIT)
                .setTiling     (VK_IMAGE_TILING_OPTIMAL)
                .setLayout     (VK_IMAGE_LAYOUT_UNDEFINED)
                .setUsage      (
                    VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
                    | VK_IMAGE_USAGE_SAMPLED_BIT
                    // | VK_IMAGE_USAGE_TRANSFER_DST_BIT
                    // | VK_IMAGE_USAGE_TRANSFER_SRC_BIT
                )
                .setMemoryUsage(VMA_MEMORY_USAGE_AUTO)
                .setFlags      (VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT) // Cubemap
                .build();
        }

        // Create the cubemap view for the irradiance map
        {
            m_IrradianceMap.View = VyImageView::Builder{}
                .setName    ("irradiance")
                .setViewType(VK_IMAGE_VIEW_TYPE_CUBE)
                .setFormat  (m_HDR.Image.format())
                .setAspect  (VK_IMAGE_ASPECT_COLOR_BIT)
                .setLevels  (0, irradianceMipLevels)
                .setLayers  (0, m_FACE_COUNT)
                .build( m_IrradianceMap.Image );
        }
        // {
		// 	m_IrradianceMap.Sampler = VySampler::Builder{}
		// 		.setName         ("irradiance")
		// 		.setFilters      (VK_FILTER_LINEAR)
		// 		.setMipmapMode   (VK_SAMPLER_MIPMAP_MODE_LINEAR)
		// 		.setWrap         (VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE)
		// 		.setBorder       (VK_BORDER_COLOR_INT_OPAQUE_BLACK)
		// 		.setLodRange     (0.0f, 1.0f)
		// 		.setMipLodBias   (0.0f)
		// 		.build();
        // }

        // Create image views for each face
        {
            for (U32 face = 0; face < m_FACE_COUNT; ++face)
            {
                m_IrradianceMap.FaceViews[ face ] = VyImageView::Builder{}
                    .setName    ("irradiance_face")
                    .setViewType(VK_IMAGE_VIEW_TYPE_2D)
                    .setFormat  (m_HDR.Image.format())
                    .setAspect  (VK_IMAGE_ASPECT_COLOR_BIT)
                    .setLevels  (0,    irradianceMipLevels)
                    .setLayers  (face, 1)
                    .build( m_IrradianceMap.Image ).handle();
            }
        }

        // Render to irradiance map
        {
            TArray<VkImageView, 6> faceViews{};

            // for (int i = 0; i < 6; ++i)
            // {
            //     faceViews[i].push_back( m_IrradianceMap.FaceViews[ i ] );
            // }

            RenderToCubemap(
                m_CubeMap.Image, 
                m_CubeMap.View.handle(), 
                m_HDR.Sampler->handle(),
                m_IrradianceMap.Image, 
                faceViews, 
                m_IrradianceMap.Size,
                "Cubemap.vert.spv", 
                "DiffuseIrradiance.frag.spv"
            );
            
            // Clean up temporary face views
            for (const auto& view : faceViews) 
            {
                vkDestroyImageView(VyContext::device(), view, nullptr);
            }

            // renderToCubeMap(
            //     m_IrradianceMap.Extent, 
            //     irradianceMipLevels,
            //     m_CubeVertPath, 
            //     m_IBLFragPath, 
            //     m_CubeMap.Image,
            //     m_CubeMap.View.handle(), 
            //     m_Equirect.Sampler.handle(), 
            //     m_IrradianceMap.Image, 
            //     faceViews
            // );

            //GenerateMipmaps(m_IrradianceMapImage, m_EquirectFormat,
            //	m_IrradianceMapExtent.width, m_IrradianceMapExtent.height,
            //	irradianceMipLevels, m_FACE_COUNT);

        }
    }

    // void HDRImage::TransitionImageLayout(
    //     VkImage image,
    //     VkFormat /*format*/,
    //     VkImageLayout oldLayout,
    //     VkImageLayout newLayout,
    //     U32 mipLevels)
    // {
    //     VkCommandBuffer cmdBuffer = VyContext::beginCommands();

    //     VkImageMemoryBarrier2 barrier{};
    //     barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
    //     barrier.oldLayout = oldLayout;
    //     barrier.newLayout = newLayout;
    //     barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    //     barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    //     barrier.image = image;
    //     barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    //     barrier.subresourceRange.baseMipLevel = 0;
    //     barrier.subresourceRange.levelCount = mipLevels;
    //     barrier.subresourceRange.baseArrayLayer = 0;
    //     barrier.subresourceRange.layerCount = 1;

    //     VkPipelineStageFlags2 srcStage;
    //     VkPipelineStageFlags2 dstStage;

    //     if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
    //         newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
    //         barrier.srcAccessMask = 0;
    //         barrier.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
    //         srcStage = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
    //         dstStage = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
    //     }
    //     else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
    //         newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
    //         barrier.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
    //         barrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
    //         srcStage = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
    //         dstStage = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
    //     }
    //     else {
    //         throw std::invalid_argument("Unsupported layout transition!");
    //     }

    //     barrier.srcStageMask = srcStage;
    //     barrier.dstStageMask = dstStage;

    //     VkDependencyInfo depInfo{};
    //     depInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    //     depInfo.imageMemoryBarrierCount = 1;
    //     depInfo.pImageMemoryBarriers = &barrier;

    //     vkCmdPipelineBarrier2(cmdBuffer, &depInfo);

    //     VyContext::endCommands(cmdBuffer);
    // }
}