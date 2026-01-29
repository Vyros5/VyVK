#include <VyEngine/GFX/Resources/Texture/Texture.h>
#include <VyEngine/VK/Context.h>

#include <VyEngine/VK/Pipeline/Pipeline.h>
#include <VyEngine/VK/Descriptors/Descriptors.h>

#include <VyEngine/Globals.h>
#include <VyLib/Util/String.h>
// #define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <VyEngine/GFX/Resources/Mesh/Model.h>
#include <iostream>


// namespace Vy
// {
// 	FileTextureSource::FileTextureSource(const TString& filepath)
// 	{
// 		int texW, texH, channels;
// 		stbi_uc* pPixels = stbi_load(filepath.c_str(), &texW, &texH, &channels, STBI_rgb_alpha);
		
// 		if (!pPixels)
// 		{
// 			VY_THROW_RUNTIME_ERROR("Failed to load texture image: " + filepath);
// 		}

// 		m_Width  = texW;
// 		m_Height = texH;
// 		m_BPP    = 4;

// 		U32 size = m_Width * m_Height * m_BPP;

// 		m_Pixels.reserve(size);
// 		m_Pixels.insert(m_Pixels.end(), pPixels, pPixels + size);

// 		stbi_image_free(pPixels);
// 	}


// 	FloatFileTextureSource::FloatFileTextureSource(const TString& filepath)
// 	{
// 		int texW, texH, channels;
// 		float* pPixels = stbi_loadf(filepath.c_str(), &texW, &texH, &channels, STBI_rgb_alpha);
		
// 		if (!pPixels)
// 		{
// 			VY_THROW_RUNTIME_ERROR("Failed to load texture image: " + filepath);
// 		}

// 		m_Width  = texW;
// 		m_Height = texH;
// 		m_BPP    = 4 * sizeof(float);

// 		U32 size = m_Width * m_Height * m_BPP;

// 		m_Pixels.reserve(size);
// 		m_Pixels.insert(m_Pixels.end(), (uint8_t*)pPixels, (uint8_t*)pPixels + size);

// 		stbi_image_free(pPixels);
// 	}


// 	SolidTextureSource::SolidTextureSource(Vec4 color, U32 width, U32 height)
// 	{
// 		m_Width	 = width;
// 		m_Height = height;
// 		m_BPP    = 4;

// 		m_Pixels.resize(width * height * m_BPP);

// 		for (int i = 0; i < width; i++) 
// 		{
// 			for (int j = 0; j < height; j++) 
// 			{
// 				m_Pixels[(i + j * width) * 4 + 0] = color.r * 255;
// 				m_Pixels[(i + j * width) * 4 + 1] = color.g * 255;
// 				m_Pixels[(i + j * width) * 4 + 2] = color.b * 255;
// 				m_Pixels[(i + j * width) * 4 + 3] = color.a * 255;
// 			}
// 		}
// 	}


// 	FloatSolidTextureSource::FloatSolidTextureSource(Vec4 color, U32 width, U32 height)
// 	{
// 		m_Width	 = width;
// 		m_Height = height;
// 		m_BPP    = 4 * sizeof(float);

// 		m_Pixels.resize(width * height * m_BPP);
		
// 		for (int i = 0; i < width; i++) 
// 		{
// 			for (int j = 0; j < height; j++) 
// 			{
// 				*(float*)(m_Pixels.data() + sizeof(float) * ((i + j * width) * 4 + 0)) = color.r;
// 				*(float*)(m_Pixels.data() + sizeof(float) * ((i + j * width) * 4 + 1)) = color.g;
// 				*(float*)(m_Pixels.data() + sizeof(float) * ((i + j * width) * 4 + 2)) = color.b;
// 				*(float*)(m_Pixels.data() + sizeof(float) * ((i + j * width) * 4 + 3)) = color.a;
// 			}
// 		}
// 	}
// }

// namespace Vy
// {
// 	VyTexture2::VyTexture2(
// 		const TString&                 name, 
// 		const VkImageCreateInfo&       imageInfo, 
// 		const VmaAllocationCreateInfo& allocInfo,
// 		const VkImageViewCreateInfo&   viewInfo,
// 		const VkSamplerCreateInfo&     samplerInfo
// 	) :
// 		m_DebugName  { name        },
// 		m_ImageInfo  { imageInfo   },
// 		m_AllocInfo  { allocInfo   },
// 		m_ViewInfo   { viewInfo    },
// 		m_SamplerInfo{ samplerInfo }
// 	{
// 		if (m_ImageInfo.mipLevels > 1 )
// 		{
// 			m_ImageInfo.mipLevels = static_cast<U32>(std::floor(std::log2(std::max(m_ImageInfo.extent.width, m_ImageInfo.extent.height)))) + 1;
// 		}

// 		VK_CHECK_SUCCESS(vmaCreateImage(
// 			VyContext::allocator(), 
// 			&imageInfo, 
// 			&allocInfo, 
// 			&m_Image, 
// 			&m_ImageMemory, 
// 			nullptr
// 		), "Failed to create image!");

//         VKDbg::setObjectName(m_Image, TString(m_DebugName + "_image").c_str());

//         VK_CHECK_SUCCESS(vkCreateImageView(
// 			VyContext::device(), 
// 			&m_ViewInfo, 
// 			nullptr, 
// 			&m_ImageView
// 		), "Failed to create image view!");

//         VKDbg::setObjectName(m_ImageView, TString(m_DebugName + "_image_view").c_str());
	
// 		if (m_SamplerInfo.maxAnisotropy = -1.0f)
// 		{
// 			m_SamplerInfo.maxAnisotropy = VyContext::device().limits().maxSamplerAnisotropy;
// 		}

// 		VK_CHECK_SUCCESS(vkCreateSampler(
// 			VyContext::device(), 
// 			&m_SamplerInfo, 
// 			nullptr, 
// 			&m_Sampler
// 		), "Failed to create sampler!");

// 		VKDbg::setObjectName(m_Sampler, TString(m_DebugName + "_sampler").c_str());
// 	}
// }

namespace Vy
{
	VyTextureConfig VyTextureConfig::texture2D(U32 width, U32 height, VkFormat format)
	{
		return VyTextureConfig{
			.Image = VyImageInfo{
				.Format    = format,
				.Extent    = VkExtent3D{ width, height, 1 },
				.MipLevels = VyImageInfo::CALCULATE_MIP_LEVELS,
			},
		};
	}

	VyTextureConfig VyTextureConfig::cubeMap(U32 size, VkFormat format)
	{
		return VyTextureConfig{
			.Image = VyImageInfo{
				.Flags       = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT,
				.ImageType   = VK_IMAGE_TYPE_2D,
				.Format      = format,
				.Extent      = VkExtent3D{ size, size, 1 },
				.MipLevels   = VyImageInfo::CALCULATE_MIP_LEVELS,
				.ArrayLayers = 6,
				.Usage       = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
			},
			.View = VyImageViewInfo{
				.ViewType = VK_IMAGE_VIEW_TYPE_CUBE,
			},
		};
	}



	Shared<VyTexture> VyTexture::loadFromFile(const TPath& file, VkFormat format /*VK_FORMAT_R8G8B8A8_UNORM*/)
	{
		if (!std::filesystem::exists(file))
		{
			VY_ERROR_TAG("VyTexture", "Texture file does not exist: '{}'", file.string());
			VY_ASSERT(false, "Texture file does not exist");
		}

		if (file.extension() == ".hdr")
		{
			return VyTexture::loadCubemap( file );
		}

		return VyTexture::loadTexture2D( file, format );
	}



	Shared<VyTexture> VyTexture::loadTexture2D(const TPath& file, VkFormat format)
	{
		int texW = 0;
		int texH = 0;

		auto pPixels = VyTexture::loadImage( file.string(), texW, texH, STBI_rgb_alpha );
		if (!pPixels)
		{
			VY_ERROR_TAG("VyTexture", "Failed to load image: '{}'", file.string());
			VY_ASSERT(false, "Failed to load image");
		}
		U32 width  = static_cast<U32>(texW);
		U32 height = static_cast<U32>(texH);

		VkDeviceSize imageSize = 4 * static_cast<VkDeviceSize>(texW) * static_cast<VkDeviceSize>(texH);

		VyTextureConfig info = VyTextureConfig::texture2D(width, height, format);
		{
			info.Name = Utils::filenameFromPath(file.string()) + "_tex_2d";
		}

		auto pTexture = MakeShared<VyTexture>( info );

		pTexture->image().upload(pPixels, imageSize);

		VyTexture::freeImageData( pPixels );

		return pTexture;
	}


	Shared<VyTexture> VyTexture::loadCubemap(const TPath& file)
	{
		// HDR environment maps are stored as equirectangular images (longitude/latitude 2D image)
		// To convert it to a cubemap, the image is sampled in a compute shader and written to the cubemap 

		int texW = 0;
		int texH = 0;
		auto pPixels = VyTexture::loadImageFloat( file.string(), texW, texH, STBI_rgb_alpha );
		if (!pPixels)
		{
			VY_ERROR_TAG("VyTexture", "Failed to load cubemap image: '{}'", file.string());
			VY_ASSERT(false, "Failed to load cubemap image");
		}

		U32 width  = static_cast<U32>(texW);
		U32 height = static_cast<U32>(texH);

		// Upload data to staging buffer
		VkDeviceSize imageSize = 4 * sizeof(float) * static_cast<VkDeviceSize>(texW) * static_cast<VkDeviceSize>(texH);

		VyBuffer stagingBuffer{ VyBuffer::stagingBuffer("tex_cubemap", imageSize) };
		
		stagingBuffer.singleWrite( pPixels );

		VyTexture::freeImageData( pPixels );

		// Create spherical image
		auto sphericalInfo = VyTextureConfig::texture2D(width, height, VK_FORMAT_R32G32B32A32_SFLOAT);
		{
			sphericalInfo.Name = Utils::filenameFromPath(file.string()) + "spherical_tex_2d";
			sphericalInfo.Image.MipLevels = 1;
		}

		VyTexture spherialImage{ sphericalInfo };

		// Create cubemap image
		U32 cubeSize = width / 4; // Cubemap needs 4 horizontal faces

		auto cubeInfo = VyTextureConfig::cubeMap(cubeSize, VK_FORMAT_R16G16B16A16_SFLOAT);
		{
			cubeInfo.Name = Utils::filenameFromPath(file.string()) + "_cubemap";
			cubeInfo.Image.Usage |= VK_IMAGE_USAGE_STORAGE_BIT;
		}

		auto pCubeMap = MakeShared<VyTexture>(cubeInfo);

		// Create pipeline resources
		auto descriptorSetLayout = VyDescriptorSetLayout::Builder{}
			.addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_COMPUTE_BIT)
			.addBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,          VK_SHADER_STAGE_COMPUTE_BIT)
			.buildPtr();

		VkDescriptorSet descriptorSet = descriptorSetLayout->allocate();

		VyImageView arrayImageView = VyImageView::Builder{}
			.setName    ("tex_cubemap_array")
			.setViewType(VK_IMAGE_VIEW_TYPE_2D_ARRAY)
			.build( pCubeMap->image() );
		
		VkDescriptorImageInfo cubemapImageInfo{
			.sampler     = pCubeMap->sampler().handle(),
			.imageView   = arrayImageView.handle(),
			.imageLayout = VK_IMAGE_LAYOUT_GENERAL,
		};

		VkDescriptorImageInfo spherialInfo = spherialImage.descriptorImageInfo(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		VyDescriptorWriter{ *descriptorSetLayout, *VyContext::globalPool() }
			.writeImage(0, &spherialInfo)
			.writeImage(1, &cubemapImageInfo)
			.update(descriptorSet);

		auto pipeline = VyPipeline::ComputeBuilder{}
			.addDescriptorSetLayout(descriptorSetLayout->handle())
			.setShaderStage("IBL/EquirectToCube.slang.spv")
			.buildPtr();

		// Convert spherical image to cubemap
		VkCommandBuffer cmdBuffer = VyContext::beginCommands();
		{
			VKCmd::beginDebugUtilsLabel(cmdBuffer, "Equirectangular to Cubemap");
			{
				spherialImage.image().copyFrom(cmdBuffer, stagingBuffer);
				pCubeMap    ->image().transitionLayout(cmdBuffer, VK_IMAGE_LAYOUT_GENERAL);

				pipeline->bind(cmdBuffer);

				pipeline->bindDescriptorSet(cmdBuffer, 0, descriptorSet);

				constexpr U32 kGroupSize = 16;

				U32 groupCountX = (width  + kGroupSize - 1) / kGroupSize;
				U32 groupCountY = (height + kGroupSize - 1) / kGroupSize;

				vkCmdDispatch(cmdBuffer, groupCountX, groupCountY, 6);

				pCubeMap->image().generateMipmaps(cmdBuffer, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			}
			VKCmd::endDebugUtilsLabel(cmdBuffer);
		}
		VyContext::endCommands(cmdBuffer);

		return pCubeMap;
	}


	Shared<VyTexture> VyTexture::loadFromMemory(const std::byte* pData, size_t size, VkFormat format)
	{
		int texW, texH, channels;
		auto pPixels = stbi_load_from_memory(
			reinterpret_cast<const stbi_uc*>(pData), 
			static_cast<int>(size),
			&texW, 
			&texH, 
			&channels, 
			STBI_rgb_alpha
		);

		if (!pPixels)
		{
			VY_ERROR_TAG("VyTexture", "Failed to load image from memory");

			return nullptr;
		}

		VkDeviceSize imageSize = 4 * static_cast<VkDeviceSize>(texW) * static_cast<VkDeviceSize>(texH);

		VyTextureConfig info = VyTextureConfig::texture2D(static_cast<U32>(texW), static_cast<U32>(texH), format);
		{
			info.Name = "memory_tex_2d";
		}

		auto pTexture = MakeShared<VyTexture>(info);

		pTexture->image().upload(pPixels, imageSize);

		VyTexture::freeImageData( pPixels );

		return pTexture;
	}


	Shared<VyTexture> VyTexture::createSolidColor(Vec4 color)
	{
		auto info = VyTextureConfig::texture2D(1, 1, VK_FORMAT_R32G32B32A32_SFLOAT);
		{
			info.Name = "solid_color_tex_2d";
		}

		auto texture = MakeShared<VyTexture>(info);
		
		texture->image().upload(&color, sizeof(Vec4));
		
		return texture;
	}


	Shared<VyTexture> VyTexture::createSolidColorCube(Vec4 color)
	{
		TArray<Vec4, 6> colors{ color, color, color, color, color, color };

		auto info = VyTextureConfig::cubeMap(1, VK_FORMAT_R32G32B32A32_SFLOAT);
		{
			info.Name = "solid_color_cubemap";
		}
		
		auto texture = MakeShared<VyTexture>(info);
		
		texture->image().upload(&colors, sizeof(colors));
		
		return texture;
	}


	Shared<VyTexture> VyTexture::irradianceMap(const Shared<VyTexture>& pSkybox)
	{
		// Create irradiance map
		constexpr U32 kIrradianceSize = 32;

		auto textureInfo = VyTextureConfig::cubeMap(kIrradianceSize, VK_FORMAT_R16G16B16A16_SFLOAT);
		{
			textureInfo.Name = "irradiance_cubemap";
			textureInfo.Image.Usage    |= VK_IMAGE_USAGE_STORAGE_BIT;
			textureInfo.Image.MipLevels = 1;
		}
		
		auto pIrradiance = MakeShared<VyTexture>(textureInfo);

		// Create pipeline resources
		auto descriptorSetLayout = VyDescriptorSetLayout::Builder{}
			.addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_COMPUTE_BIT)
			.addBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,          VK_SHADER_STAGE_COMPUTE_BIT)
			.build();

		VyImageView arrayImageView = VyImageView::Builder{}
			.setName    ("irradiance_cubemap_array")
			.setViewType(VK_IMAGE_VIEW_TYPE_2D_ARRAY)
			.build( pIrradiance->image() );

		VkDescriptorImageInfo cubemapImageInfo{
			.sampler     = pIrradiance->sampler(),
			.imageView   = arrayImageView.handle(),
			.imageLayout = VK_IMAGE_LAYOUT_GENERAL,
		};

		VkDescriptorImageInfo skyboxInfo = pSkybox->descriptorImageInfo(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		VkDescriptorSet descriptorSet = descriptorSetLayout.allocate();

		VyDescriptorWriter{ descriptorSetLayout, *VyContext::globalPool() }
			.writeImage(0, &skyboxInfo)
			.writeImage(1, &cubemapImageInfo)
			.update(descriptorSet);

		auto pipeline = VyPipeline::ComputeBuilder{}
			.addDescriptorSetLayout(descriptorSetLayout)
			.setShaderStage("IBL/IrradianceConvolution.slang.spv")
			.build();

		// Convert skybox to irradiance map
		VkCommandBuffer cmdBuffer = VyContext::beginCommands();
		{
			VKCmd::beginDebugUtilsLabel(cmdBuffer, "Irradiance Convolution");
			{
				pSkybox    ->image().transitionLayout(cmdBuffer, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
				pIrradiance->image().transitionLayout(cmdBuffer, VK_IMAGE_LAYOUT_GENERAL);

				pipeline.bind(cmdBuffer);

				pipeline.bindDescriptorSet(cmdBuffer, 0, descriptorSet);

				constexpr U32 kGroupSize = 16;

				U32 width  = pIrradiance->image().width();
				U32 height = pIrradiance->image().height();

				U32 groupCountX = (width  + kGroupSize - 1) / kGroupSize;
				U32 groupCountY = (height + kGroupSize)     / kGroupSize;

				vkCmdDispatch(cmdBuffer, groupCountX, groupCountY, 6);

				pIrradiance->image().transitionLayout(cmdBuffer, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			}
			VKCmd::endDebugUtilsLabel(cmdBuffer);
		}
		VyContext::endCommands(cmdBuffer);

		return pIrradiance;
	}


	Shared<VyTexture> VyTexture::prefilteredMap(const Shared<VyTexture>& pSkybox)
	{
		// Create prefiltered map
		constexpr U32 kPrefilteredSize = 128;
		constexpr U32 kMipLevelCount   = 5;

		auto textureInfo = VyTextureConfig::cubeMap(kPrefilteredSize, VK_FORMAT_R16G16B16A16_SFLOAT);
		{
			textureInfo.Name = "prefilitered_cubemap";
			textureInfo.Image.Usage    |= VK_IMAGE_USAGE_STORAGE_BIT;
			textureInfo.Image.MipLevels = kMipLevelCount;
		}

		auto pPrefiltered = MakeShared<VyTexture>(textureInfo);

		// Create pipeline resources
		auto descriptorSetLayout = VyDescriptorSetLayout::Builder{}
			.addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_COMPUTE_BIT)
			.addBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
			.build();

		struct PushConstants
		{
			float Roughness     =   0.0f;
			float EnvResolution = 512.0f;

		} pushConstants;
		
		pushConstants.EnvResolution = static_cast<float>(pSkybox->image().width());

		auto pipeline = VyPipeline::ComputeBuilder{}
			.addDescriptorSetLayout(descriptorSetLayout)
			.addPushConstantRange(VK_SHADER_STAGE_COMPUTE_BIT, sizeof(pushConstants))
			.setShaderStage("IBL/PrefilterEnvironment.slang.spv")
			.build();

		// Create image views for mip levels
		TVector<VkImageView> mipViews;
		mipViews.reserve( kMipLevelCount );

		TVector<VkDescriptorSet> descriptorSets;
		descriptorSets.reserve( kMipLevelCount );

		for (U32 i = 0; i < kMipLevelCount; ++i)
		{
			VyImageView view = VyImageView::Builder{}
				.setViewType(VK_IMAGE_VIEW_TYPE_2D_ARRAY)
				.setLevels(i, 1)
				.setLayers(0, 6)
				.build(pPrefiltered->image());

			mipViews.emplace_back( view.handle() );

			VkDescriptorImageInfo mipInfo{};
			{
				mipInfo.sampler     = VK_NULL_HANDLE;
				mipInfo.imageView   = mipViews[ i ];
				mipInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
			}

			VkDescriptorImageInfo skyboxInfo = pSkybox->descriptorImageInfo(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

			descriptorSets.emplace_back( descriptorSetLayout.allocate() );

			VyDescriptorWriter{ descriptorSetLayout, *VyContext::globalPool() }
				.writeImage(0, &skyboxInfo)
				.writeImage(1, &mipInfo)
				.update(descriptorSets[ i ]);
		}

		// Convert skybox to prefiltered map
		VkCommandBuffer cmdBuffer = VyContext::beginCommands();
		{
			VKCmd::beginDebugUtilsLabel(cmdBuffer, "Prefilter Environment");
			{
				pSkybox     ->image().transitionLayout(cmdBuffer, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
				pPrefiltered->image().transitionLayout(cmdBuffer, VK_IMAGE_LAYOUT_GENERAL);

				pipeline.bind(cmdBuffer);

				for (U32 mip = 0; mip < kMipLevelCount; ++mip)
				{
					pipeline.bindDescriptorSet(cmdBuffer, 0, descriptorSets[ mip ]);

					pushConstants.Roughness = static_cast<float>(mip) / static_cast<float>(kMipLevelCount - 1);

					pipeline.pushConstants(cmdBuffer, VK_SHADER_STAGE_COMPUTE_BIT, &pushConstants, sizeof(pushConstants));

					constexpr U32 kGroupSize = 16;

					U32 width  = pPrefiltered->image().width()  >> mip;
					U32 height = pPrefiltered->image().height() >> mip;

					U32 groupCountX = (width  + kGroupSize - 1) / kGroupSize;
					U32 groupCountY = (height + kGroupSize - 1) / kGroupSize;

					vkCmdDispatch(cmdBuffer, groupCountX, groupCountY, 6);
				}

				pPrefiltered->image().transitionLayout(cmdBuffer, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			}
			VKCmd::endDebugUtilsLabel(cmdBuffer);
		}
		VyContext::endCommands(cmdBuffer);

		return pPrefiltered;
	}


	Shared<VyTexture> VyTexture::BRDFLUT()
	{
		// Create BRDF LUT
		constexpr U32 kLUTSize = 512;

		auto textureInfo = VyTextureConfig::texture2D(kLUTSize, kLUTSize, VK_FORMAT_R16G16_SFLOAT);
		{
			textureInfo.Name = "brdf_lut_tex_2d";
			textureInfo.Image.MipLevels     = 1;
			textureInfo.Image.Usage        |= VK_IMAGE_USAGE_STORAGE_BIT;
			textureInfo.Sampler.AddressMode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		}

		auto pLUT = MakeShared<VyTexture>(textureInfo);

		// Create pipeline resources
		auto descriptorSetLayout = VyDescriptorSetLayout::Builder{}
			.addBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
			.buildPtr();

		VkDescriptorImageInfo lutInfo = pLUT->descriptorImageInfo(VK_IMAGE_LAYOUT_GENERAL);

		VkDescriptorSet descriptorSet;
		VyDescriptorWriter{ *descriptorSetLayout, *VyContext::globalPool() }
			.writeImage(0, &lutInfo)
			.build(descriptorSet);

		auto pipeline = VyPipeline::ComputeBuilder{}
			.addDescriptorSetLayout(descriptorSetLayout->handle())
			.setShaderStage("IBL/BRDFLUT.slang.spv")
			.build();

		// Convert skybox to irradiance map
		VkCommandBuffer cmdBuffer = VyContext::beginCommands();
		{
			VKCmd::beginDebugUtilsLabel(cmdBuffer, "BRDF LUT Generation");
			{
				pLUT->image().transitionLayout(cmdBuffer, VK_IMAGE_LAYOUT_GENERAL);
				
				pipeline.bind(cmdBuffer);

				pipeline.bindDescriptorSet(cmdBuffer, 0, descriptorSet);
				
				constexpr U32 kGroupSize = 16;

				// (512 + 16 - 1) / 16 = 32.9375
				U32 groupCountX = (kLUTSize + kGroupSize - 1) / kGroupSize;
				U32 groupCountY = (kLUTSize + kGroupSize - 1) / kGroupSize;

				vkCmdDispatch(cmdBuffer, groupCountX, groupCountY, 1);
				
				pLUT->image().transitionLayout(cmdBuffer, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			}
			VKCmd::endDebugUtilsLabel(cmdBuffer);
		}
		VyContext::endCommands(cmdBuffer);

		return pLUT;
	}







	U8* VyTexture::loadImage(const TString& path, int& width, int& height, int componentCount /*= 4*/)
	{
        // Force load with alpha channel, even if it does not have one
        int texChannels;

        stbi_uc* pPixels = stbi_load(path.data(), &width, &height, &texChannels, componentCount);

        VY_ASSERT(pPixels != nullptr, "Failed to load texture image \"{}\"", path);

		return static_cast<U8*>(pPixels);
	}


	float* VyTexture::loadImageFloat(const TString& path, int& width, int& height, int componentCount /*= 4*/)
	{
		int texChannels;

		float* pPixels = stbi_loadf(path.data(), &width, &height, &texChannels, componentCount);

		VY_ASSERT(pPixels != nullptr, "Failed to load texture image \"{}\"", path);

		return pPixels;
	}


	// U8* VyTexture::loadImage(const TString& path, U32& width, U32& height, int componentCount /*= 4*/)
	// {
    //     // Force load with alpha channel, even if it does not have one
    //     int texW, texH, texChannels;

    //     stbi_uc* pPixels = stbi_load(path.data(), &texW, &texH, &texChannels, componentCount);

    //     VY_ASSERT(pPixels != nullptr, "Failed to load texture image \"{}\"", path);

    //     width  = static_cast<U32>(texW);
    //     height = static_cast<U32>(texH);
        
	// 	return static_cast<U8*>(pPixels);
	// }


    void VyTexture::freeImageData(U8* pData)
    {
        stbi_image_free( pData );
    }

	void VyTexture::freeImageData(float* pData)
	{
		stbi_image_free( pData );
	}


	Shared<VyTexture> VyTexture::createWhiteTexture()
	{
		unsigned char whitePixel[ 4 ] = { 255, 255, 255, 255 };

		return MakeShared<VyTexture>( whitePixel, 1, 1, VK_FORMAT_R8G8B8A8_UNORM );
	}

	
	Shared<VyTexture> VyTexture::createNormalTexture()
	{
		// Flat normal pointing up in tangent space: (0, 0, 1) -> (128, 128, 255) in RGB
		unsigned char normalPixel[ 4 ] = { 128, 128, 255, 255 };

		return MakeShared<VyTexture>( normalPixel, 1, 1, VK_FORMAT_R8G8B8A8_UNORM );
	}


	VyTexture::VyTexture(const VyTextureConfig& info) :
		m_Image  { info.Name, info.Image                        },
		m_View   { info.Name, info.View,    m_Image             },
		m_Sampler{ info.Name, info.Sampler, m_Image.mipLevels() }
	{
	}


    VyTexture::VyTexture(const TString& filepath, VkFormat format) :
		m_Filepath{ filepath }
	{
        int desiredChannels = STBI_rgb_alpha;  // default = 4
        if (format == VK_FORMAT_R8_UNORM)     desiredChannels = STBI_grey;
        if (format == VK_FORMAT_R8G8_UNORM)   desiredChannels = STBI_grey_alpha;
        if (format == VK_FORMAT_R8G8B8_UNORM) desiredChannels = STBI_rgb;

		U8* pPixels = VyTexture::loadImage(filepath, m_Width, m_Height, desiredChannels);
		
		if (!pPixels)
		{
			VY_THROW_RUNTIME_ERROR("Failed to load texture image: " + filepath);
		}

		VkDeviceSize imageSize = static_cast<VkDeviceSize>(m_Width) * static_cast<VkDeviceSize>(m_Height) * 4; // RGBA

		// Calculate mip levels
		m_MipLevels = static_cast<U32>(std::floor(std::log2(std::max(m_Width, m_Height)))) + 1;

		// Create staging buffer and copy pixel data.
		VyBuffer stagingBuffer{ VyBuffer::stagingBuffer("texture", imageSize) };

		stagingBuffer.singleWrite( pPixels );

		VyTexture::freeImageData( pPixels );

		// Create Vulkan image
        m_Image = VyImage::Builder{}
			.setName       (Utils::filenameFromPath( filepath ))
            .setImageType  (VK_IMAGE_TYPE_2D)
            .setFormat     (format)
            .setExtent     (static_cast<U32>(m_Width), static_cast<U32>(m_Height))
            .setLevels     (m_MipLevels)
			.setLayers     (1)
			.setSamples    (VK_SAMPLE_COUNT_1_BIT)
            .setTiling     (VK_IMAGE_TILING_OPTIMAL)
			.setLayout     (VK_IMAGE_LAYOUT_UNDEFINED)
            .setUsage      (VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
            .setMemoryUsage(VMA_MEMORY_USAGE_AUTO)
        	.build();

		// Transition image layout and copy buffer to image.
		m_Image.copyFrom( stagingBuffer, false /*toShaderReadOnly*/ );

		// Generate mipmaps (this also transitions to SHADER_READ_ONLY_OPTIMAL)
		m_Image.generateMipmaps( VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL );

		// Create image view and sampler.
		createImageView(format);
		createSampler();

		VY_INFO_TAG("VyTexture", "Loaded: {0} ({1}x{2}, {3} mips)", filepath, m_Width, m_Height, m_MipLevels);
	}


	VyTexture::VyTexture(const TString& filepath, bool bSRGB, bool bFlipY) :
		m_Filepath{ filepath }
	{
		if (bFlipY)
		{
			stbi_set_flip_vertically_on_load(true);
		}

		U8* pPixels = VyTexture::loadImage(filepath, m_Width, m_Height, STBI_rgb_alpha);

		if (bFlipY)
		{
			stbi_set_flip_vertically_on_load(false);
		}

		if (!pPixels)
		{
			VY_THROW_RUNTIME_ERROR("Failed to load texture image: " + filepath);
		}

		VkDeviceSize imageSize = static_cast<VkDeviceSize>(m_Width) * static_cast<VkDeviceSize>(m_Height) * 4; // RGBA

		// Calculate mip levels
		m_MipLevels = static_cast<U32>(std::floor(std::log2(std::max(m_Width, m_Height)))) + 1;

		// Create staging buffer
		VyBuffer stagingBuffer{ VyBuffer::stagingBuffer("texture", imageSize) };

		stagingBuffer.singleWrite( pPixels );

		VyTexture::freeImageData( pPixels );

		// Choose format based on whether this is an sRGB texture (color) or linear (data)
		VkFormat format = bSRGB ? VK_FORMAT_R8G8B8A8_SRGB : VK_FORMAT_R8G8B8A8_UNORM;

		// Create Vulkan image
        m_Image = VyImage::Builder{}
			.setName       (Utils::filenameFromPath( filepath ))
            .setImageType  (VK_IMAGE_TYPE_2D)
            .setFormat     (format)
            .setExtent     (static_cast<U32>(m_Width), static_cast<U32>(m_Height))
            .setLevels     (m_MipLevels)
			.setLayers     (1)
			.setSamples    (VK_SAMPLE_COUNT_1_BIT)
            .setTiling     (VK_IMAGE_TILING_OPTIMAL)
			.setLayout     (VK_IMAGE_LAYOUT_UNDEFINED)
            .setUsage      (VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
            .setMemoryUsage(VMA_MEMORY_USAGE_AUTO)
        	.build();

		// Transition image layout and copy buffer to image.
		m_Image.copyFrom( stagingBuffer, false /*toShaderReadOnly*/ );

		// Generate mipmaps (this also transitions to SHADER_READ_ONLY_OPTIMAL)
		m_Image.generateMipmaps( VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL );

		// Create image view and sampler
		createImageView(format);
		createSampler();

		VY_INFO_TAG("VyTexture", "Loaded: {0} ({1}x{2}, {3} mips)", Utils::filenameFromPath(filepath), m_Width, m_Height, m_MipLevels);
	}


	VyTexture::VyTexture(const void* pData, size_t size, bool bSRGB /*= true*/, bool bMipmapped /*= true*/)
	{
		int      texChannels;
		stbi_uc* pPixels    = nullptr;
		bool     bIsRawData = false;

		VY_INFO_TAG("VyTexture", "Attempting to load embedded texture from memory, size: {} bytes", size);

		// First, try to load as compressed image (JPEG/PNG)
		stbi_set_flip_vertically_on_load( false ); // Temporarily disable flipping to test UV issues
        
		pPixels = stbi_load_from_memory(
			static_cast<const stbi_uc*>(pData), 
			static_cast<int>(size), 
			&m_Width, 
			&m_Height, 
			&texChannels, 
			STBI_rgb_alpha
		);

		if (!pPixels) 
		{
			// If compressed loading failed, check if this might be raw pixel data
			// Common raw texture sizes: 4096x4096, 2048x2048, 1024x1024, 512x512, etc.
			const size_t pixelSize       = 4; // RGBA
			const size_t possibleSizes[] = { 4096, 2048, 1024, 512, 256, 128, 64 };

			for (size_t dim : possibleSizes) 
			{
				if (size == dim * dim * pixelSize) 
				{
					VY_INFO_TAG("VyTexture", "Attempting to load embedded texture from memory, size: {} bytes", size);

					m_Width     = static_cast<int>(dim);
					m_Height    = static_cast<int>(dim);
					texChannels = 4;
					pPixels     = const_cast<stbi_uc*>(static_cast<const stbi_uc*>(pData));
					bIsRawData  = true;
					break;
				}
			}

			if (!bIsRawData) 
			{
				const char* error = stbi_failure_reason();
				
				VY_ERROR_TAG("VyTexture", "STB_Image error: {}\n Data size: {} bytes\n First few bytes:", (error ? error : "unknown"), size);
				
				const unsigned char* pBytes = static_cast<const unsigned char*>(pData);
				TString hexDump;
				
				for (size_t i = 0; i < std::min(size, static_cast<size_t>(16)); ++i) 
				{
					hexDump += fmt::format("{:02x} ", static_cast<int>(pBytes[ i ]));
				}

				VY_ERROR_TAG("VyTexture", "First 16 bytes: {}", hexDump);
				VY_ERROR_TAG("VyTexture", "Failed to load texture image from memory!");

				VY_THROW_RUNTIME_ERROR("Failed to load texture image from memory!");
			}
		}
		else 
		{
			VY_INFO_TAG("VyTexture", "Successfully decoded compressed texture: {} x {} channels: {}", m_Width, m_Height, texChannels);
		}

		VkDeviceSize imageSize = static_cast<VkDeviceSize>(m_Width) * static_cast<VkDeviceSize>(m_Height) * 4; // RGBA

		// Calculate mip levels
		m_MipLevels = bMipmapped 
			? static_cast<U32>(std::floor(std::log2(std::max(m_Width, m_Height)))) + 1 
			: 1;

		// Create staging buffer
		VyBuffer stagingBuffer{ VyBuffer::stagingBuffer("memory_texture", imageSize) };

		stagingBuffer.singleWrite( pPixels );

		// Free the pixel data loaded by stb_image (only if it was allocated by stb_image)
		if (!bIsRawData) 
		{
			VyTexture::freeImageData( pPixels );
		}

		// Choose format based on whether this is an sRGB texture (color) or linear (data)
		VkFormat format = bSRGB 
			? VK_FORMAT_R8G8B8A8_SRGB 
			: VK_FORMAT_R8G8B8A8_UNORM;

		// Create Vulkan image
        m_Image = VyImage::Builder{}
			.setName       ("memory_texture")
            .setImageType  (VK_IMAGE_TYPE_2D)
            .setFormat     (format)
            .setExtent     (static_cast<U32>(m_Width), static_cast<U32>(m_Height))
            .setLevels     (m_MipLevels)
			.setLayers     (1)
			.setSamples    (VK_SAMPLE_COUNT_1_BIT)
            .setTiling     (VK_IMAGE_TILING_OPTIMAL)
			.setLayout     (VK_IMAGE_LAYOUT_UNDEFINED)
            .setUsage      (VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
            .setMemoryUsage(VMA_MEMORY_USAGE_AUTO)
        	.build();

		if (bMipmapped && m_MipLevels > 1)
		{
			// Transition image layout and copy buffer to image.
			m_Image.copyFrom( stagingBuffer, false /*toShaderReadOnly*/ );
			
			// Generate mipmaps (this also transitions to SHADER_READ_ONLY_OPTIMAL)
			m_Image.generateMipmaps( VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL );
		}
		else
		{
			// Transition image layout and copy buffer to image.
			m_Image.copyFrom( stagingBuffer, false /*toShaderReadOnly*/ );
		}

		// Create image view and sampler
		createImageView(format);
		createSampler();
	}


	VyTexture::VyTexture(const void* pData, U32 width, U32 height, bool bSRGB /*= true*/, bool bMipmapped /*= true*/)
	{
		m_Width  = width;
		m_Height = height;

		int      texChannels;
		stbi_uc* pPixels    = nullptr;
		
		VkDeviceSize imageSize = static_cast<VkDeviceSize>(m_Width) * static_cast<VkDeviceSize>(m_Height) * 4; // RGBA

		VY_INFO_TAG("VyTexture", "Attempting to load embedded texture from memory, size: {} bytes", imageSize);

		// Try to load as compressed image (JPEG/PNG)
		stbi_set_flip_vertically_on_load( false ); // Temporarily disable flipping to test UV issues
        
		pPixels = stbi_load_from_memory(
			static_cast<const stbi_uc*>(pData), 
			static_cast<int>(imageSize), 
			&m_Width, 
			&m_Height, 
			&texChannels, 
			STBI_rgb_alpha
		);

		// Calculate mip levels
		m_MipLevels = bMipmapped 
			? static_cast<U32>(std::floor(std::log2(std::max(m_Width, m_Height)))) + 1 
			: 1;

		// Create staging buffer
		VyBuffer stagingBuffer{ VyBuffer::stagingBuffer("memory_texture", imageSize) };

		stagingBuffer.singleWrite( pPixels );

		// Free the pixel data loaded by stb_image (only if it was allocated by stb_image)
		if (pPixels) 
		{
			VyTexture::freeImageData( pPixels );
		}

		// Choose format based on whether this is an sRGB texture (color) or linear (data)
		VkFormat format = bSRGB 
			? VK_FORMAT_R8G8B8A8_SRGB 
			: VK_FORMAT_R8G8B8A8_UNORM;

		// Create Vulkan image
        m_Image = VyImage::Builder{}
			.setName       ("memory_texture")
            .setImageType  (VK_IMAGE_TYPE_2D)
            .setFormat     (format)
            .setExtent     (static_cast<U32>(m_Width), static_cast<U32>(m_Height))
            .setLevels     (m_MipLevels)
			.setLayers     (1)
			.setSamples    (VK_SAMPLE_COUNT_1_BIT)
            .setTiling     (VK_IMAGE_TILING_OPTIMAL)
			.setLayout     (VK_IMAGE_LAYOUT_UNDEFINED)
            .setUsage      (VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
            .setMemoryUsage(VMA_MEMORY_USAGE_AUTO)
        	.build();

		if (bMipmapped && m_MipLevels > 1)
		{
			// Transition image layout and copy buffer to image.
			m_Image.copyFrom( stagingBuffer, false /*toShaderReadOnly*/ );
			
			// Generate mipmaps (this also transitions to SHADER_READ_ONLY_OPTIMAL)
			m_Image.generateMipmaps( VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL );
		}
		else
		{
			// Transition image layout and copy buffer to image.
			m_Image.copyFrom( stagingBuffer, false /*toShaderReadOnly*/ );
		}

		// Create image view and sampler
		createImageView(format);
		createSampler();
	}




	// VyTexture::VyTexture(
	// 	U32                width,
	// 	U32                height,
	// 	VkFormat           format,
	// 	VkImageUsageFlags  usage,
	// 	VkImageAspectFlags aspectMask,
	// 	bool               bCreateSampler)  /*= true*/
	// {
	// 	// Create Vulkan image
    //     m_Image = VyImage::Builder{}
	// 		.setName       ("gbuffer_texture")
    //         .setImageType  (VK_IMAGE_TYPE_2D)
    //         .setFormat     (format)
    //         .setExtent     (width, height)
    //         .setLevels     (1)
	// 		.setLayers     (1)
	// 		.setSamples    (VK_SAMPLE_COUNT_1_BIT)
    //         .setTiling     (VK_IMAGE_TILING_OPTIMAL)
	// 		.setLayout     (VK_IMAGE_LAYOUT_UNDEFINED)
    //         .setUsage      (usage | VK_IMAGE_USAGE_SAMPLED_BIT)
    //         .setMemoryUsage(VMA_MEMORY_USAGE_AUTO)
    //     	.build();

    //     m_View = VyImageView::Builder{}
	// 		.setName    ("gbuffer_texture")
    //         .setViewType(VK_IMAGE_VIEW_TYPE_2D)
    //         .setFormat  (format)
    //         .setAspect  (aspectMask)
    //         .setLevels  (0, 1)
    //         .setLayers  (0, 1)
    //     	.build( m_Image );

    //     //optionally create nearest-filter sampler
    //     if (bCreateSampler)
	// 	{
	// 		m_Sampler = VySampler::Builder{}
	// 			.setName         ("gbuffer_texture")
	// 			.setFilters      (VK_FILTER_NEAREST)
	// 			.setMipmapMode   (VK_SAMPLER_MIPMAP_MODE_NEAREST)
	// 			.setWrap         (VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE)
	// 			.setBorder       (VK_BORDER_COLOR_INT_OPAQUE_BLACK)
	// 			.setLodRange     (0.0f, 1.0f)
	// 			.setMipLodBias   (0.0f)
	// 			.build();
	// 	}
	// }



	// VyTexture::VyTexture(const VyTextureConfig& config)
	// {
	// 	// Create Vulkan image
    //     m_Image = VyImage::Builder{}
	// 		.setName       (config.Name)
    //         .setImageType  (config.ImageType)
    //         .setFormat     (config.Format)
    //         .setExtent     (config.Extent)
    //         .setLevels     (config.MipLevels)
	// 		.setLayers     (config.LayerCount)
	// 		.setSamples    (config.Samples)
    //         .setTiling     (VK_IMAGE_TILING_OPTIMAL)
	// 		.setLayout     (VK_IMAGE_LAYOUT_UNDEFINED)
    //         .setUsage      (config.Usage)
    //         .setMemoryUsage(VMA_MEMORY_USAGE_AUTO)
    //     	.build();

	// 	m_MipLevels = m_Image.mipLevels();
	// 	m_Width  = static_cast<int>(config.Extent.width);
	// 	m_Height = static_cast<int>(config.Extent.height);

    //     m_View = VyImageView::Builder{}
	// 		.setName    (config.Name)
    //         .setViewType(config.ViewType)
    //         .setFormat  (config.Format)
    //         .setLevels  (config.BaseMipLevel, config.MipLevels)
    //         .setLayers  (config.BaseLayer, config.LayerCount)
    //     	.build( m_Image );

	// 	m_Sampler = VySampler::Builder{}
	// 		.setName         (config.Name)
	// 		.setFilters      (config.Filter)
	// 		.setMipmapMode   (config.MipmapMode)
	// 		.setWrap         (config.AddressMode)
	// 		.setBorder       (VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE)
	// 		.enableAnisotropy(config.AnisotropyEnabled)
	// 		.setLodRange     (0.0f, static_cast<float>(m_MipLevels))
	// 		.setMipLodBias   (0.0f)
	// 		.build();
	// }



	VyTexture::~VyTexture()
	{
		// std::cout << "[VyTexture] Destroyed '" << m_Image.debugName() << "'" << std::endl;
	}


	// Private constructor for creating textures from memory
	VyTexture::VyTexture(const unsigned char* pPixels, int width, int height, VkFormat format) : 
		m_Width { width  },
		m_Height{ height }
	{
		VkDeviceSize imageSize = static_cast<VkDeviceSize>(m_Width) * static_cast<VkDeviceSize>(m_Height) * 4; // RGBA

		m_MipLevels = 1; // No mipmaps for default textures

		// Create staging buffer
		VyBuffer stagingBuffer{ VyBuffer::stagingBuffer("memory_texture", imageSize) };

		stagingBuffer.singleWrite( pPixels );

		// Create Vulkan image
        m_Image = VyImage::Builder{}
			.setName       ("memory_texture")
            .setImageType  (VK_IMAGE_TYPE_2D)
            .setFormat     (format)
            .setExtent     (static_cast<U32>(m_Width), static_cast<U32>(m_Height))
            .setLevels     (m_MipLevels)
			.setLayers     (1)
			.setSamples    (VK_SAMPLE_COUNT_1_BIT)
            .setTiling     (VK_IMAGE_TILING_OPTIMAL)
			.setLayout     (VK_IMAGE_LAYOUT_UNDEFINED)
            .setUsage      (VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
            .setMemoryUsage(VMA_MEMORY_USAGE_AUTO)
        	.build();

		// Transition image layout and copy buffer to image.
		m_Image.copyFrom( stagingBuffer, true /*toShaderReadOnly*/ );

		// Create image view and sampler
		createImageView(format);
		createSampler();

		VY_INFO_TAG("VyTexture", "Loaded texture from memory ({0}x{1}, {2} mips)", m_Width, m_Height, m_MipLevels);
	}


	void VyTexture::createImageView(VkFormat format)
	{
        auto builder = VyImageView::Builder{};

		if (m_Filepath != "")
		{
			builder.setName( Utils::filenameFromPath( m_Filepath ) );
		}
		else {
			builder.setName( "memory_texture" );
		}
		
        builder.setViewType(VK_IMAGE_VIEW_TYPE_2D);
        builder.setFormat  (format);
        builder.setAspect  (VK_IMAGE_ASPECT_COLOR_BIT);
        builder.setLevels  (0, m_MipLevels);
        builder.setLayers  (0, 1);
        	
		m_View = builder.build( m_Image );
	}


	void VyTexture::createSampler()
	{
		auto builder = VySampler::Builder{};

		if (m_Filepath != "")
		{
			builder.setName( Utils::filenameFromPath( m_Filepath ) );
		}
		else {
			builder.setName( "memory_texture" );
		}

		builder.setFilters      (VK_FILTER_LINEAR);
		builder.setMipmapMode   (VK_SAMPLER_MIPMAP_MODE_LINEAR);
		builder.setWrap         (VK_SAMPLER_ADDRESS_MODE_REPEAT);
		builder.setBorder       (VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE);
		builder.enableAnisotropy(true);
		builder.setLodRange     (0.0f, static_cast<float>(m_MipLevels));
		builder.setMipLodBias   (0.0f);

		m_Sampler = builder.build();
	}


	void VyTexture::transitionImageLayout(VkCommandBuffer cmdBuffer, VkImageLayout newLayout)
	{
		m_Image.transitionLayout( cmdBuffer, newLayout );
	}


	size_t VyTexture::memorySize() const
	{
		// Calculate memory for base texture + all mipmaps
		// Format: RGBA8 (4 bytes per pixel) or sRGB8_A8 (also 4 bytes)
		size_t totalSize = 0;
		int    w         = m_Width;
		int    h         = m_Height;

		for (U32 level = 0; level < m_MipLevels; ++level)
		{
			totalSize += w * h * 4; // 4 bytes per pixel (RGBA8)

			w = std::max(1, w / 2);
			h = std::max(1, h / 2);
		}

		return totalSize;
	}

	// VyTexture::Builder::Builder(const TString& path) : 
	// 	m_SourceType{ SourceType::File }, 
	// 	m_Path      { path }, 
	// 	m_pData     { nullptr }, 
	// 	m_Width     { 0 }, 
	// 	m_Height    { 0 } 
	// {
	// }


	// VyTexture::Builder::Builder(const void* pData, U32 width, U32 height) : 
	// 	m_SourceType{ SourceType::Memory }, 
	// 	m_pData     { pData }, 
	// 	m_Width     { width }, 
	// 	m_Height    { height } 
	// {
	// }


	// Unique<VyTexture> VyTexture::Builder::buildPtr() 
	// {
	// 	if (m_SourceType == SourceType::File) 
	// 	{
	// 		return VyTexture::createFromFile(m_Path, m_SRGB);
	// 	}
	// 	else // if (m_SourceType == SourceType::Memory) 
	// 	{
	// 		return VyTexture::createFromMemory(m_pData, m_Width, m_Height, m_GenerateMipmaps, m_SRGB);
	// 	}
	// }












}