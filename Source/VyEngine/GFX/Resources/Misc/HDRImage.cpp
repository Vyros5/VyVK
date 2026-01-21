#include <VyEngine/GFX/Resources/Misc/HDRImage.h>

#include <VyEngine/VK/Context.h>

#include <VyEngine/VK/Pipeline/Pipeline.h>

#include "stb_image.h"

namespace Vy
{
	HDRImage::HDRImage(const TString& filename)
	{
        // 1) Load the HDR pixels with stb_image
        if (!std::filesystem::exists(filename)) {
            throw std::runtime_error("File does not exist: " + filename);
        }
        if (!stbi_is_hdr(filename.c_str())) {
            throw std::runtime_error("File is not HDR format: " + filename);
        }

        int texWidth, texHeight, texChannels;
        float* pixels = stbi_loadf(filename.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        if (!pixels) {
            throw std::runtime_error("Failed to load HDR image: " + filename);
        }
        // m_EquirectMipLevels = 1; // only one mip level for now
        // m_EquirectExtent = { U32(texWidth), U32(texHeight) };
        // m_EquirectFormat = VK_FORMAT_R32G32B32A32_SFLOAT;

        VkDeviceSize imageSize = VkDeviceSize(texWidth) * texHeight * 4 * sizeof(float);

        // 2) Create a host?visible staging buffer
        VyBuffer stagingBuffer = VyBuffer::stagingBuffer("hdri", imageSize );

        // 3) Copy pixels into the staging buffer
        stagingBuffer.write(pixels);

        stbi_image_free(pixels);

        // 4) Create the equirectangular image (device?local)
        CreateEquirectImage(
            texWidth,
            texHeight,
            1,
            VK_FORMAT_R32G32B32A32_SFLOAT,
            VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT
        );
        // CreateEquirectTextureImageView();

        // 5) Transition image to TRANSFER_DST_OPTIMAL, copy, then to SHADER_READ_ONLY_OPTIMAL
        m_EquirectImage.copyFrom(stagingBuffer, true);

        // 7) Create the sampler
        CreateEquirectTextureSampler(VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);

        // 8) Build the cube?map from this equirectangular image
        CreateCubeMap();
        CreateIrradianceMap(); 
	}


    HDRImage::~HDRImage()
	{
    //     vkDestroySampler(VyContext::device(), m_EquirectSampler, nullptr);
    //     vkDestroyImageView(VyContext::device(), m_EquirectImageView, nullptr);
    //     vkDestroyImage(VyContext::device(), m_EquirectImage, nullptr);

        // vkDestroySampler(VyContext::device(), m_CubeMapSampler, nullptr);
        // vkDestroyImageView(VyContext::device(), m_CubeMapImageView, nullptr);
        for (auto& faceViews : m_CubeMapFaceViews)
            for (auto v : faceViews)
                vkDestroyImageView(VyContext::device(), v, nullptr);

        // Destroy irradiance map face views
        for (auto& view : m_IrradianceMapFaceViews) {
            if (view != VK_NULL_HANDLE) {
                vkDestroyImageView(VyContext::device(), view, nullptr);
            }
        }

		// vkDestroySampler(VyContext::device(), m_IrradianceMapSampler, nullptr);
        // vkDestroyImageView(VyContext::device(), m_IrradianceMapImageView, nullptr);
        // vkDestroyImage(VyContext::device(), m_IrradianceMapImage, nullptr);
        // vkDestroyImage(VyContext::device(), m_CubeMapImage, nullptr);
	}



    void HDRImage::CreateEquirectImage(
        U32               width,
        U32               height,
        U32               miplevels,
        VkFormat          format,
        VkImageUsageFlags usage)
    {
        m_EquirectImage = VyImage::Builder{}
			.setName       ("equirect")
            .setImageType  (VK_IMAGE_TYPE_2D)
            .setFormat     (format)
            .setExtent     (width, height)
            .setLevels     (1)
			.setLayers     (miplevels)
			.setSamples    (VK_SAMPLE_COUNT_1_BIT)
            .setTiling     (VK_IMAGE_TILING_OPTIMAL)
			.setLayout     (VK_IMAGE_LAYOUT_UNDEFINED)
            .setUsage      (usage)
            .setMemoryUsage(VMA_MEMORY_USAGE_AUTO)
        	.build();

        m_EquirectImageView = VyImageView::Builder{}
			.setName    ("equirect")
            .setViewType(VK_IMAGE_VIEW_TYPE_2D)
            .setFormat  (format)
            .setAspect  ( VKUtil::aspectFromFormat( format ))
            .setLevels  (0, miplevels)
            .setLayers  (0, 1)
        	.build( m_EquirectImage );
    }


    void HDRImage::CreateEquirectTextureSampler(VkFilter filter, VkSamplerAddressMode addressMode)
    {
        m_EquirectSampler = VySampler::Builder{}
            .setName         ("equirect")
            .setFilters      (filter)
            .setMipmapMode   (VK_SAMPLER_MIPMAP_MODE_LINEAR)
            .setWrap         (addressMode)
            .setBorder       (VK_BORDER_COLOR_INT_OPAQUE_BLACK)
            .enableAnisotropy(true)
            .setLodRange     (0.0f, 1.0f)
            .setMipLodBias   (0.0f)
            .build();
    }


    void HDRImage::CreateCubeMap()
    {
        // We only need one mip level for now
        U32 cubeMipLevels = 1;

        m_CubeMapImage = VyImage::Builder{}
			.setName       ("cubemap")
            .setImageType  (VK_IMAGE_TYPE_2D)
            .setFormat     (m_EquirectImage.format())
            .setExtent     (m_CubeMapExtent)
            .setLevels     (cubeMipLevels)
			.setLayers     (m_FACE_COUNT)
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
            .setFlags      (VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT)
        	.build();

        m_CubeMapImageView = VyImageView::Builder{}
			.setName    ("cubemap")
            .setViewType(VK_IMAGE_VIEW_TYPE_CUBE)
            .setFormat  (m_EquirectImage.format())
            .setAspect  (VK_IMAGE_ASPECT_COLOR_BIT)
            .setLevels  (0, 1)
            .setLayers  (0, m_FACE_COUNT)
        	.build( m_CubeMapImage );


        // 3. Create per?face 2D views (for rendering each face)
        for (U32 face = 0; face < m_FACE_COUNT; ++face) 
        {
            VyImageView faceView = VyImageView::Builder{}
                .setName    ("cubemap_face")
                .setViewType(VK_IMAGE_VIEW_TYPE_2D)
                .setFormat  (m_EquirectImage.format())
                .setAspect  (VK_IMAGE_ASPECT_COLOR_BIT)
                .setLevels  (0,    1)
                .setLayers  (face, 1)
                .build( m_CubeMapImage );

            m_CubeMapFaceViews[face].push_back(faceView);
        }

        // 4. Create the sampler
        m_CubeMapSampler = VySampler::Builder{}
            .setName         ("cubemap")
            .setFilters      (VK_FILTER_LINEAR)
            .setMipmapMode   (VK_SAMPLER_MIPMAP_MODE_LINEAR)
            .setWrap         (VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE)
            .setBorder       (VK_BORDER_COLOR_INT_OPAQUE_BLACK)
            .enableAnisotropy(true)
            .setLodRange     (0.0f, static_cast<float>(cubeMipLevels))
            .setMipLodBias   (0.0f)
            .build();

        // 5. Finally render into it
        renderToCubeMap(
            m_CubeMapExtent,
            cubeMipLevels,
            m_CubeVertPath,
            m_SkyFragPath,
            m_EquirectImage,
            m_EquirectImageView.handle(),
            m_EquirectSampler.handle(),
            m_CubeMapImage,
            m_CubeMapFaceViews
        );
    }

    
    void HDRImage::renderToCubeMap(
        const VkExtent2D&                extent, 
        U32                              mipLevels, 
        const TString&                   vertPath, 
        const TString&                   fragPath, 
        VyImage&                         inputImage, 
        const VkImageView&               inputImageView, 
        VkSampler                        inputSampler, 
        VkImage&                         outputCubeMapImage, 
        TArray<TVector<VkImageView>, 6>& outputCubeMapImageViews)
	{
        // 1) Transition the equirectangular input to SHADER_READ_ONLY_OPTIMAL if needed
        if (m_EquirectImage.layout() != VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) 
        {
            m_EquirectImage.transitionLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        }

        // 2) Begin one-off command buffer
        VkCommandBuffer cmdBuffer = VyContext::beginCommands();

        // 3) Transition the entire cube-map image into COLOR_ATTACHMENT_OPTIMAL
        {
            VkImageMemoryBarrier2 barrier{ VKInit::imageMemoryBarrier2() };
            {
                barrier.srcStageMask     = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
                barrier.srcAccessMask    = 0;
                barrier.dstStageMask     = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
                barrier.dstAccessMask    = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
                barrier.oldLayout        = VK_IMAGE_LAYOUT_UNDEFINED;
                barrier.newLayout        = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                barrier.image            = outputCubeMapImage;
                barrier.subresourceRange = { 
                    VK_IMAGE_ASPECT_COLOR_BIT, 
                    0, 1, 
                    0, m_FACE_COUNT 
                };
            }

            VkDependencyInfo dep{ VKInit::dependencyInfo() };
            {
                dep.imageMemoryBarrierCount = 1;
                dep.pImageMemoryBarriers = &barrier;
            }

            vkCmdPipelineBarrier2(cmdBuffer, &dep);
        }

        // 4) Build a one-off descriptor set to sample the equirectangular map
        VkDescriptorSetLayout descriptorLayout;
        {
            VkDescriptorSetLayoutBinding b{ 0,
                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                1,
                VK_SHADER_STAGE_FRAGMENT_BIT,
                nullptr
            };
            VkDescriptorSetLayoutCreateInfo li{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
            {
                li.bindingCount = 1;
                li.pBindings = &b;
            }

            vkCreateDescriptorSetLayout(VyContext::device(), &li, nullptr, &descriptorLayout);
        }

        VkDescriptorPool descriptorPool;
        {
            VkDescriptorPoolSize sz{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1 };
            VkDescriptorPoolCreateInfo pi{ VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
            pi.maxSets = 1;
            pi.poolSizeCount = 1;
            pi.pPoolSizes = &sz;
            vkCreateDescriptorPool(VyContext::device(), &pi, nullptr, &descriptorPool);
        }

        VkDescriptorSet descriptorSet;
        {
            VkDescriptorSetAllocateInfo ai{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
            ai.descriptorPool = descriptorPool;
            ai.descriptorSetCount = 1;
            ai.pSetLayouts = &descriptorLayout;
            vkAllocateDescriptorSets(VyContext::device(), &ai, &descriptorSet);

            VkDescriptorImageInfo ii{};
            {
                ii.sampler     = inputSampler;
                ii.imageView   = inputImageView;
                ii.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            }

            VkWriteDescriptorSet w{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
            w.dstSet = descriptorSet;
            w.dstBinding = 0;
            w.dstArrayElement = 0;
            w.descriptorCount = 1;
            w.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            w.pImageInfo = &ii;

            vkUpdateDescriptorSets(VyContext::device(), 1, &w, 0, nullptr);
        }

        // 5) Create pipeline layout with 2×mat4 push-constants + our single descriptor set
        auto builder = VyPipeline::GraphicsBuilder{};
        {
            builder.setName( "model" );
            
            builder.addDescriptorSetLayout( descriptorLayout );
            
            builder.addPushConstantRange(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(Mat4) * 2 /* view + proj*/);
            
            builder.addShaderStage(VK_SHADER_STAGE_VERTEX_BIT,   vertPath);
            builder.addShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT, fragPath);
            
            builder.addColorAttachment( inputImage.format() );
            builder.setDepthAttachment(VK_FORMAT_UNDEFINED);

            builder.clearVertexDescriptions();
        }

        auto pipeline = builder.buildPtr();

        // 7) Prepare capture projection / views
        const Mat4 captureProj = [] {
            Mat4 p = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
            return p;
        }();

        const Vec3 eye{ 0.0f, 0.0f, 0.0f };

        const std::array<Mat4, 6> captureViews = {
            glm::lookAt(eye, eye + Vec3(  1.0f,  0.0f,  0.0f ), Vec3( 0.0f, -1.0f,  0.0f )), // +X
            glm::lookAt(eye, eye + Vec3( -1.0f,  0.0f,  0.0f ), Vec3( 0.0f, -1.0f,  0.0f )), // -X
            glm::lookAt(eye, eye + Vec3(  0.0f,  1.0f,  0.0f ), Vec3( 0.0f,  0.0f,  1.0f )), // -Y
            glm::lookAt(eye, eye + Vec3(  0.0f, -1.0f,  0.0f ), Vec3( 0.0f,  0.0f, -1.0f )), // +Y
            glm::lookAt(eye, eye + Vec3(  0.0f,  0.0f,  1.0f ), Vec3( 0.0f, -1.0f,  0.0f )), // +Z
            glm::lookAt(eye, eye + Vec3(  0.0f,  0.0f, -1.0f ), Vec3( 0.0f, -1.0f,  0.0f ))  // -Z
        };

        // 8) Render each of the six faces
        for (U32 face = 0; face < m_FACE_COUNT; ++face) 
        {
            // (a) barrier for this layer is already handled above by the 6-layer barrier

            // (b) dynamic rendering begin
            VkRenderingAttachmentInfo colorAtt{ VKInit::renderingAttachmentInfo() };
            {
                colorAtt.imageView        = outputCubeMapImageViews[face][0];
                colorAtt.imageLayout      = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                colorAtt.loadOp           = VK_ATTACHMENT_LOAD_OP_CLEAR;
                colorAtt.storeOp          = VK_ATTACHMENT_STORE_OP_STORE;
                colorAtt.clearValue.color = { 0,0,0,1 };
            }

            VkRenderingInfo ri{ VKInit::renderingInfo() };
            {
                ri.renderArea.extent    = extent;
                ri.layerCount           = 1;
                ri.colorAttachmentCount = 1;
                ri.pColorAttachments    = &colorAtt;
            }

            vkCmdBeginRendering(cmdBuffer, &ri);

            // (c) set viewport & scissor
            VkViewport vp{ 0,0, float(extent.width), float(extent.height), 0,1 };
            VkRect2D  sc{ {0,0}, extent };
            vkCmdSetViewport(cmdBuffer, 0, 1, &vp);
            vkCmdSetScissor(cmdBuffer, 0, 1, &sc);

            // (d) bind pipeline + descriptor
            pipeline->bind(cmdBuffer);

            pipeline->bindDescriptorSet(cmdBuffer, 0, descriptorSet);
            // vkCmdBindDescriptorSets(
            //     cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
            //     pipelineLayout, 0, 1, &descriptorSet,
            //     0, nullptr
            // );

            // (e) push constants: view then proj
            pipeline->pushConstants(cmdBuffer, VK_SHADER_STAGE_VERTEX_BIT, &captureViews[face], sizeof(Mat4));
            // vkCmdPushConstants(cmdBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT,
            //     0, sizeof(Mat4), &captureViews[face]);
            pipeline->pushConstants(cmdBuffer, VK_SHADER_STAGE_VERTEX_BIT, &captureViews[face], sizeof(Mat4), sizeof(Mat4));
            // vkCmdPushConstants(cmdBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT,
            //     sizeof(Mat4), sizeof(Mat4), &captureProj);

            // (f) draw a 36-vertex cube (generated in your vert shader)
            vkCmdDraw(cmdBuffer, 36, 1, 0, 0);

            vkCmdEndRendering(cmdBuffer);
        }

        // 9) finally transition the cube to SHADER_READ_ONLY_OPTIMAL
        {
            VkImageMemoryBarrier2 barrier{ VKInit::imageMemoryBarrier2() };
            {
                barrier.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
                barrier.srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
                barrier.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
                barrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
                barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                barrier.image = outputCubeMapImage;
                barrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, m_FACE_COUNT };
            } 

            VkDependencyInfo dep{ VKInit::dependencyInfo() };
            {
                dep.imageMemoryBarrierCount = 1;
                dep.pImageMemoryBarriers = &barrier;
            }

            vkCmdPipelineBarrier2(cmdBuffer, &dep);
        }

        // 10) End and submit
        VyContext::endCommands(cmdBuffer);

        // 11) Clean up
        // vkDestroyPipelineLayout(VyContext::device(), pipelineLayout, nullptr);
        vkDestroyDescriptorPool(VyContext::device(), descriptorPool, nullptr);
        vkDestroyDescriptorSetLayout(VyContext::device(), descriptorLayout, nullptr);
	}



    void HDRImage::CreateIrradianceMap()
    {
        U32 irradianceMipLevels = 1;

        // 1. Create image
        {
            m_IrradianceMapImage = VyImage::Builder{}
                .setName       ("irradiance")
                .setImageType  (VK_IMAGE_TYPE_2D)
                .setFormat     (m_EquirectImage.format())
                .setExtent     (m_IrradianceMapExtent)
                .setLevels     (irradianceMipLevels)
                .setLayers     (m_FACE_COUNT)
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
                .setFlags      (VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT)
                .build();
        }

        // 2. Create view
        {
            m_IrradianceMapImageView = VyImageView::Builder{}
                .setName    ("irradiance")
                .setViewType(VK_IMAGE_VIEW_TYPE_CUBE)
                .setFormat  (m_EquirectImage.format())
                .setAspect  (VK_IMAGE_ASPECT_COLOR_BIT)
                .setLevels  (0, irradianceMipLevels)
                .setLayers  (0, m_FACE_COUNT)
                .build( m_IrradianceMapImage );
        }
        {
			m_IrradianceMapSampler = VySampler::Builder{}
				.setName         ("irradiance")
				.setFilters      (VK_FILTER_LINEAR)
				.setMipmapMode   (VK_SAMPLER_MIPMAP_MODE_LINEAR)
				.setWrap         (VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE)
				.setBorder       (VK_BORDER_COLOR_INT_OPAQUE_BLACK)
				.setLodRange     (0.0f, 1.0f)
				.setMipLodBias   (0.0f)
				.build();
        }

        // 3. Create face views
        {
            for (U32 face = 0; face < m_FACE_COUNT; ++face)
            {
                m_IrradianceMapFaceViews[ face ] = VyImageView::Builder{}
                    .setName    ("irradiance_face")
                    .setViewType(VK_IMAGE_VIEW_TYPE_2D)
                    .setFormat  (m_EquirectImage.format())
                    .setAspect  (VK_IMAGE_ASPECT_COLOR_BIT)
                    .setLevels  (0,    irradianceMipLevels)
                    .setLayers  (face, 1)
                    .build( m_IrradianceMapImage ).handle();
            }
        }

        // 4. Render to irradiance map
        {
            TArray<TVector<VkImageView>, 6> faceViews;

            for (int i = 0; i < 6; ++i)
            {
                faceViews[i].push_back( m_IrradianceMapFaceViews[ i ] );
            }

            renderToCubeMap(
                m_IrradianceMapExtent, 
                irradianceMipLevels,
                m_CubeVertPath, 
                m_IBLFragPath, 
                m_CubeMapImage,
                m_CubeMapImageView.handle(), 
                m_EquirectSampler.handle(), 
                m_IrradianceMapImage, 
                faceViews
            );

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