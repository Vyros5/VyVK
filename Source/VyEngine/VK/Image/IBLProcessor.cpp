#include <VyEngine/VK/Image/IBLProcessor.h>

#include <VyEngine/VK/Context.h>

#include <VyEngine/Globals.h>
#include <VyLib/Util/String.h>
// #define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <iostream>

namespace Vy
{
    IBLProcessor::~IBLProcessor() 
    {
        // cleanup();
    }

    bool IBLProcessor::initialize() 
    {
        if (!createCubemapImages()) return false;
        if (!createDescriptors()) return false;
        if (!createComputePipelines()) return false;

        // Create samplers
        auto samplerBuilder = VySampler::Builder{};
        {
            samplerBuilder.setName         ("cubemap");
            samplerBuilder.setFilters      (VK_FILTER_LINEAR);
            samplerBuilder.setMipmapMode   (VK_SAMPLER_MIPMAP_MODE_LINEAR);
            samplerBuilder.setWrap         (VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
            samplerBuilder.setBorder       (VK_BORDER_COLOR_INT_OPAQUE_BLACK);
            samplerBuilder.setLodRange     (0.0f, static_cast<float>(PREFILTER_MIP_LEVELS));
            samplerBuilder.setMipLodBias   (0.0f);
            
            m_CubemapSampler = samplerBuilder.build();
        }

        {
            samplerBuilder.setName("brdf_lut");
            samplerBuilder.setLodRange(0.0f, 0.0f);
            
            m_BRDFSampler = samplerBuilder.build();
        }

        return true;
    }


    bool IBLProcessor::loadHDRImage(const TString& path, TVector<float>& pixels, U32& width, U32& height) 
    {
        int w, h, channels;
        stbi_set_flip_vertically_on_load(true);
        float* pData = stbi_loadf(path.c_str(), &w, &h, &channels, 4);

        if (!pData) {
            return false;
        }

        width  = static_cast<U32>(w);
        height = static_cast<U32>(h);
        pixels.resize(width * height * 4);
        memcpy(pixels.data(), pData, width * height * 4 * sizeof(float));

        stbi_image_free(pData);
        return true;
    }


    bool IBLProcessor::loadEnvironmentMap(const TString& hdrPath) 
    {
        TVector<float> pixels;
        U32 width, height;

        if (!loadHDRImage(hdrPath, pixels, width, height)) 
        {
            std::cerr << "Failed to load HDR image: " << hdrPath << std::endl;
            return false;
        }

        if (!createEquirectTexture(pixels, width, height)) 
        {
            std::cerr << "Failed to create equirect texture" << std::endl;
            return false;
        }

        // Process the environment map
        executeEquirectToCubemap();
        executeIrradianceConvolution();
        executePrefilterEnvironment();

        std::cout << "IBL environment map loaded: " << hdrPath << std::endl;
        return true;
    }

    bool IBLProcessor::generateBRDFLUT() 
    {
        if (m_BRDF.Image.handle() == VK_NULL_HANDLE) 
        {
            // Create BRDF LUT image
            m_BRDF.Image = VyImage::Builder{}
                .setName       ("brdf_lut")
                .setImageType  (VK_IMAGE_TYPE_2D)
                .setFormat     (VK_FORMAT_R16G16_SFLOAT)
                .setExtent     (BRDF_LUT_SIZE, BRDF_LUT_SIZE)
                .setLevels     (1)
                .setLayers     (1) 
                .setSamples    (VK_SAMPLE_COUNT_1_BIT)
                .setTiling     (VK_IMAGE_TILING_OPTIMAL)
                .setLayout     (VK_IMAGE_LAYOUT_UNDEFINED)
                .setUsage      (VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
                .setMemoryUsage(VMA_MEMORY_USAGE_AUTO)
                .build();

            m_BRDF.View = VyImageView::Builder{}
                .setName    ("brdf_lut")
                .setViewType(VK_IMAGE_VIEW_TYPE_2D)
                .setFormat  (VK_FORMAT_R16G16_SFLOAT)
                .setAspect  (VK_IMAGE_ASPECT_COLOR_BIT)
                .setLevels  (0, 1)
                .setLayers  (0, 1)
                .build( m_BRDF.Image );
        }

        executeBRDFIntegration();

        std::cout << "BRDF LUT generated" << std::endl;
        
        return true;
    }


    bool IBLProcessor::createEquirectTexture(const TVector<float>& pixels, U32 width, U32 height) 
    {
        VkDeviceSize imageSize = width * height * 4 * sizeof(float);

        // Create staging buffer
        VyBuffer stagingBuffer{ VyBuffer::stagingBuffer("equirect", imageSize, 1, 0, false /*not persistent*/) };

        stagingBuffer.map();
        {
            // Write image into the buffer.
            stagingBuffer.write( &pixels );
        }
        stagingBuffer.unmap();

        // Create equirect image
        m_Equirect.Image = VyImage::Builder{}
            .setName       ("equirect")
            .setImageType  (VK_IMAGE_TYPE_2D)
            .setFormat     (VK_FORMAT_R32G32B32A32_SFLOAT)
            .setExtent     (width, height)
            .setLevels     (1)
			.setLayers     (1)
			.setSamples    (VK_SAMPLE_COUNT_1_BIT)
            .setTiling     (VK_IMAGE_TILING_OPTIMAL)
			.setLayout     (VK_IMAGE_LAYOUT_UNDEFINED)
            .setUsage      (VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
            .setMemoryUsage(VMA_MEMORY_USAGE_AUTO)
            .build();

		// Transition image layout and copy buffer to image.
		m_Equirect.Image.copyFrom( stagingBuffer, true /*toShaderReadOnly*/ );

        // Create view
        m_Equirect.View = VyImageView::Builder{}
            .setName    ("equirect")
            .setViewType(VK_IMAGE_VIEW_TYPE_2D)
            .setFormat  (VK_FORMAT_R32G32B32A32_SFLOAT)
            .setAspect  (VK_IMAGE_ASPECT_COLOR_BIT)
            .setLevels  (0, 1)
            .setLayers  (0, 1)
            .build( m_Equirect.Image );

        return true;
    }



    bool IBLProcessor::createCubemapImages() 
    {
        auto createCubemap = [this](
            TString           name,
            VyImage&          image, 
            VyImageView&      view,
            U32               size, 
            U32               mipLevels, 
            VkFormat          format,
            VkImageUsageFlags usage) -> bool 
        {
            image = VyImage::Builder{}
                .setName       (name)
                .setImageType  (VK_IMAGE_TYPE_2D)
                .setFormat     (format)
                .setExtent     (size, size)
                .setLevels     (mipLevels)
                .setLayers     (6) // 6 faces
                .setSamples    (VK_SAMPLE_COUNT_1_BIT)
                .setTiling     (VK_IMAGE_TILING_OPTIMAL)
                .setLayout     (VK_IMAGE_LAYOUT_UNDEFINED)
                .setUsage      (usage)
                .setFlags      (VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT) // Cubemap
                .setMemoryUsage(VMA_MEMORY_USAGE_AUTO)
                .build();

            view = VyImageView::Builder{}
                .setName    (name)
                .setViewType(VK_IMAGE_VIEW_TYPE_CUBE)
                .setFormat  (format)
                .setAspect  (VK_IMAGE_ASPECT_COLOR_BIT)
                .setLevels  (0, mipLevels)
                .setLayers  (0, 6) // 6 faces
                .build( image );

            return true;
        };

        VkFormat          hdrFormat    = VK_FORMAT_R16G16B16A16_SFLOAT;
        VkImageUsageFlags cubemapUsage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

        // Environment cubemap
        if (!createCubemap(
            "env_cubemap", 
            m_EnvCubemap.Image, 
            m_EnvCubemap.View,
            ENV_CUBEMAP_SIZE, 
            1, 
            hdrFormat, 
            cubemapUsage)) 
        {
            return false;
        }

        // Irradiance cubemap
        if (!createCubemap(
            "irradiance_cubemap", 
            m_Irradiance.Image, 
            m_Irradiance.View,
            IRRADIANCE_SIZE, 
            1, 
            hdrFormat, 
            cubemapUsage)) 
        {
            return false;
        }

        // Prefiltered cubemap with mip chain
        if (!createCubemap(
            "prefiltered_cubemap", 
            m_Prefiltered.Image, 
            m_Prefiltered.View,
            PREFILTER_SIZE, 
            PREFILTER_MIP_LEVELS, 
            hdrFormat, 
            cubemapUsage)) 
        {
            return false;
        }

        // Create per-mip views for prefilter
        m_Prefiltered.MipViews.resize( PREFILTER_MIP_LEVELS );
        
        for (U32 mip = 0; mip < PREFILTER_MIP_LEVELS; ++mip) 
        {
            m_Prefiltered.MipViews[ mip ] = VyImageView::Builder{}
                .setName    (std::format("prefiltered_cubemap_mip_{}", mip))
                .setViewType(VK_IMAGE_VIEW_TYPE_CUBE)
                .setFormat  (hdrFormat)
                .setAspect  (VK_IMAGE_ASPECT_COLOR_BIT)
                .setLevels  (mip, 1)
                .setLayers  (0,   6) // 6 faces
                .build( m_Prefiltered.Image );
        }

        return true;
    }

    
    bool IBLProcessor::createComputePipelines() 
    {
        // Create pipeline
        {
            auto builder = VyPipeline::ComputeBuilder{};
            {
                builder.setName( "equirect" );
                
                builder.addDescriptorSetLayout( m_DescriptorLayout->handle() );
                builder.addPushConstantRange(VK_SHADER_STAGE_COMPUTE_BIT, sizeof(float) * 4); // roughness, mipLevel, etc.
                
                builder.setShaderStage("ComputeEquirectToCubemap.comp.spv");
            }

            m_EquirectToCubemapPipeline = builder.buildPtr();

            if (!m_EquirectToCubemapPipeline)
            {
                return false;
            }
        }

        {
            // Note: We're reusing the prefilter shader for irradiance with roughness=1.0
            auto builder = VyPipeline::ComputeBuilder{};
            {
                builder.setName( "prefilter" );
                
                builder.addDescriptorSetLayout( m_DescriptorLayout->handle() );
                builder.addPushConstantRange(VK_SHADER_STAGE_COMPUTE_BIT, sizeof(float) * 4); // roughness, mipLevel, etc.
                
                builder.setShaderStage("ComputePrefilterEnvmap.comp.spv");
            }
            
            m_PrefilterPipeline  = builder.buildPtr();

            {
                builder.setName("irradiance");
            }

            m_IrradiancePipeline = builder.buildPtr(); // Same pipeline, different params

            if (!m_PrefilterPipeline || !m_IrradiancePipeline)
            {
                return false;
            }
        }

        {
            auto builder = VyPipeline::ComputeBuilder{};
            {
                builder.setName( "brdf_lut" );
                
                builder.addDescriptorSetLayout( m_DescriptorLayout->handle() );
                builder.addPushConstantRange(VK_SHADER_STAGE_COMPUTE_BIT, sizeof(float) * 4); // roughness, mipLevel, etc.
                
                builder.setShaderStage("ComputeBRDFLUT.comp.spv");
            }

            m_BRDFPipeline = builder.buildPtr();

            if (!m_BRDFPipeline)
            {
                return false;
            }
        }

        return true;
    }

    
    bool IBLProcessor::createDescriptors() 
    {
        // Layout: input sampler + output storage image
        m_DescriptorLayout = VyDescriptorSetLayout::Builder()
            .addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_COMPUTE_BIT)
            .addBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,          VK_SHADER_STAGE_COMPUTE_BIT)
            .buildPtr();

        // Pool - need enough for all our descriptor sets
        m_DescriptorPool = VyDescriptorPool::Builder()
            .setMaxSets (20)
            .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 20) // More for prefilter mip levels
            .addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,          20)
            .buildPtr();

        // Allocate descriptor sets
        TVector<VkDescriptorSet> sets = m_DescriptorPool->allocateSets( *m_DescriptorLayout, 4);

        // TArray<VkDescriptorSetLayout, 4> layouts = {
        //     m_DescriptorLayout->handle(), 
        //     m_DescriptorLayout->handle(), 
        //     m_DescriptorLayout->handle(), 
        //     m_DescriptorLayout->handle()
        // };

        // VkDescriptorSetAllocateInfo allocInfo{};
        // allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        // allocInfo.descriptorPool = m_DescriptorPool;
        // allocInfo.descriptorSetCount = 4;
        // allocInfo.pSetLayouts = layouts.data();

        // TArray<VkDescriptorSet, 4> sets;
        // if (vkAllocateDescriptorSets(VyContext::device(), &allocInfo, sets.data()) != VK_SUCCESS) {
        //     return false;
        // }

        m_EquirectDescSet   = sets[0];
        m_IrradianceDescSet = sets[1];
        m_PrefilterDescSet  = sets[2];
        m_BRDFDescSet       = sets[3];

        return true;
    }

    void IBLProcessor::updateDescriptorSet(
        VkDescriptorSet descSet, 
        VkImageView     inputView,
        VkSampler       sampler, 
        VkImageView     outputView) 
    {
        VkDescriptorImageInfo inputInfo{};
        {
            inputInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            inputInfo.imageView   = inputView;
            inputInfo.sampler     = sampler;
        }

        VkDescriptorImageInfo outputInfo{};
        {
            outputInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
            outputInfo.imageView   = outputView;
        }

        TArray<VkWriteDescriptorSet, 2> writes{};
        {
            writes[0] = VKInit::writeDescriptorSet();
            writes[0].dstSet          = descSet;
            writes[0].dstBinding      = 0;
            writes[0].descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            writes[0].descriptorCount = 1;
            writes[0].pImageInfo      = &inputInfo;
            
            writes[1] = VKInit::writeDescriptorSet();
            writes[1].dstSet          = descSet;
            writes[1].dstBinding      = 1;
            writes[1].descriptorType  = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
            writes[1].descriptorCount = 1;
            writes[1].pImageInfo      = &outputInfo;
        }

        vkUpdateDescriptorSets(VyContext::device(), static_cast<U32>(writes.size()), writes.data(), 0, nullptr);
    }

    
    void IBLProcessor::executeEquirectToCubemap() 
    {
        // Update descriptor set: equirect input -> cubemap output
        updateDescriptorSet(
            m_EquirectDescSet, 
            m_Equirect.View.handle(),  // input view
            m_CubemapSampler, 
            m_EnvCubemap.View.handle() // output view
        );

        VkCommandBuffer cmdBuffer = VyContext::beginCommands();
        {
            // Transition cubemap to general for writing.
            m_EnvCubemap.Image.transitionLayout(cmdBuffer, VK_IMAGE_LAYOUT_GENERAL); // VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
            
            m_EquirectToCubemapPipeline->bind(cmdBuffer);

            m_EquirectToCubemapPipeline->bindDescriptorSet(cmdBuffer, 0, m_EquirectDescSet);
            
            // Dispatch: 8x8 workgroups, 6 faces
            vkCmdDispatch(cmdBuffer, 
                (ENV_CUBEMAP_SIZE + 7) / 8, 
                (ENV_CUBEMAP_SIZE + 7) / 8, 
                6
            );

            // Transition to shader read.
            m_EnvCubemap.Image.transitionLayout(cmdBuffer, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        }
        VyContext::endCommands(cmdBuffer);
    }


    void IBLProcessor::executeIrradianceConvolution() 
    {
        // Update descriptor: environment cubemap input -> irradiance output
        updateDescriptorSet(
            m_IrradianceDescSet, 
            m_EnvCubemap.View.handle(), // input view
            m_CubemapSampler, 
            m_Irradiance.View.handle()  // output view
        );

        VkCommandBuffer cmdBuffer = VyContext::beginCommands();
        {
            // Transition irradiance cubemap to general for writing
            m_Irradiance.Image.transitionLayout(cmdBuffer, VK_IMAGE_LAYOUT_GENERAL);
            
            m_IrradiancePipeline->bind(cmdBuffer);

            m_IrradiancePipeline->bindDescriptorSet(cmdBuffer, 0, m_IrradianceDescSet);
            
            // Push roughness = 1.0 for full diffuse convolution
            struct PushConstants 
            {
                float roughness;
                U32   mipLevel;
                U32   faceSize;
                U32   padding;
            
            } pc = { 1.0f, 0, IRRADIANCE_SIZE, 0 };

            m_IrradiancePipeline->pushConstants(cmdBuffer, VK_SHADER_STAGE_COMPUTE_BIT, &pc, sizeof(pc));

            // Dispatch: 8x8 workgroups, 6 faces
            vkCmdDispatch(cmdBuffer, 
                (IRRADIANCE_SIZE + 7) / 8, 
                (IRRADIANCE_SIZE + 7) / 8, 
                6
            );

            // Transition to shader read.
            m_Irradiance.Image.transitionLayout(cmdBuffer, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        }
        VyContext::endCommands(cmdBuffer);
    }


    void IBLProcessor::executePrefilterEnvironment() 
    {
        VkCommandBuffer cmdBuffer = VyContext::beginCommands();
        {
            // Transition entire prefiltered cubemap to general for writing
            VkImageMemoryBarrier barrier{ VKInit::imageMemoryBarrier() };
            {
                barrier.oldLayout                   = VK_IMAGE_LAYOUT_UNDEFINED;
                barrier.newLayout                   = VK_IMAGE_LAYOUT_GENERAL;
                barrier.srcAccessMask               = 0;
                barrier.dstAccessMask               = VK_ACCESS_SHADER_WRITE_BIT;
                barrier.image                       = m_Prefiltered.Image.handle();
                barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                barrier.subresourceRange.levelCount = PREFILTER_MIP_LEVELS;
                barrier.subresourceRange.layerCount = 6;

                vkCmdPipelineBarrier(cmdBuffer, 
                    VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                    VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 
                    0, 
                    0, nullptr, 
                    0, nullptr, 
                    1, &barrier
                );
            }

            m_PrefilterPipeline->bind(cmdBuffer);

            // Process each mip level with increasing roughness
            for (U32 mip = 0; mip < PREFILTER_MIP_LEVELS; ++mip) 
            {
                float roughness = static_cast<float>(mip) / static_cast<float>(PREFILTER_MIP_LEVELS - 1);
                U32   mipSize   = PREFILTER_SIZE >> mip;

                // Update descriptor for this mip level's view
                updateDescriptorSet(
                    m_PrefilterDescSet, 
                    m_EnvCubemap.View.handle(), // input view
                    m_CubemapSampler, 
                    m_Prefiltered.MipViews[ mip ].handle()  // output view
                );

                m_PrefilterPipeline->bindDescriptorSet(cmdBuffer, 0, m_PrefilterDescSet);

                // Push roughness constant
                struct PushConstants 
                {
                    float roughness;
                    U32   mipLevel;
                    U32   faceSize;
                    U32   padding;
                
                } pc = { roughness, mip, mipSize, 0 };

                m_PrefilterPipeline->pushConstants(cmdBuffer, VK_SHADER_STAGE_COMPUTE_BIT, &pc, sizeof(pc));

                // Dispatch for this mip level
                vkCmdDispatch(cmdBuffer, 
                    (mipSize + 7) / 8, 
                    (mipSize + 7) / 8, 
                    6
                );

                // Memory barrier between mip levels
                if (mip < PREFILTER_MIP_LEVELS - 1) 
                {
                    VkMemoryBarrier memBarrier{ VKInit::memoryBarrier() };
                    {
                        memBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
                        memBarrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
                    }

                    vkCmdPipelineBarrier(cmdBuffer, 
                        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 
                        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                        0, 
                        1, &memBarrier, 
                        0, nullptr, 
                        0, nullptr
                    );
                }
            }

            // Transition to shader read
            {
                barrier.oldLayout     = VK_IMAGE_LAYOUT_GENERAL;
                barrier.newLayout     = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
                barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            }

            vkCmdPipelineBarrier(cmdBuffer, 
                VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 
                VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                0, 
                0, nullptr, 
                0, nullptr, 
                1, &barrier
            );
        }
        VyContext::endCommands(cmdBuffer);
    }

    
    void IBLProcessor::executeBRDFIntegration() 
    {
        // For BRDF LUT, we only need the output storage image (no input)
        // Update descriptor with a dummy input (we'll ignore it in the shader)
        // The BRDF LUT shader only uses the output storage image
        VkDescriptorImageInfo outputInfo{};
        {
            outputInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
            outputInfo.imageView   = m_BRDF.View;
        }

        VkWriteDescriptorSet write{ VKInit::writeDescriptorSet() };
        {
            write.dstSet          = m_BRDFDescSet;
            write.dstBinding      = 1;  // Output binding
            write.descriptorType  = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
            write.descriptorCount = 1;
            write.pImageInfo      = &outputInfo;
        }

        vkUpdateDescriptorSets(VyContext::device(), 1, &write, 0, nullptr);

        VkCommandBuffer cmdBuffer = VyContext::beginCommands();
        {
            // Transition BRDF LUT to general for writing
            m_BRDF.Image.transitionLayout(cmdBuffer, VK_IMAGE_LAYOUT_GENERAL);

           m_BRDFPipeline->bind(cmdBuffer);

            m_BRDFPipeline->bindDescriptorSet(cmdBuffer, 0, m_BRDFDescSet);
            
            // Dispatch: compute entire BRDF LUT
            vkCmdDispatch(cmdBuffer, 
                (BRDF_LUT_SIZE + 7) / 8, 
                (BRDF_LUT_SIZE + 7) / 8, 
                1
            );

            // Transition to shader read.
            m_BRDF.Image.transitionLayout(cmdBuffer, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        }
        VyContext::endCommands(cmdBuffer);
    }

}