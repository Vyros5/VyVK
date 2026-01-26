#include <VyEngine/GFX/Resources/Texture/Texture.h>
#include <VyEngine/VK/Context.h>

#include <VyEngine/Globals.h>
#include <VyLib/Util/String.h>
// #define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <VyEngine/GFX/Resources/Mesh/Model.h>
#include <iostream>

namespace Vy
{
    VkDescriptorSetLayout VyTexture::s_BindlessSetLayout     = VK_NULL_HANDLE;
    VkDescriptorPool      VyTexture::s_BindlessPool          = VK_NULL_HANDLE; 
    VkDescriptorSet       VyTexture::s_BindlessDescriptorSet = VK_NULL_HANDLE;

	void VyTexture::initBindless(U32 maxTextures)
	{
        if (s_BindlessPool != VK_NULL_HANDLE) return; 

        // Single binding, variable count = maxTextures
        VkDescriptorSetLayoutBinding binding{};
		{
			binding.binding         = 0;
			binding.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			binding.descriptorCount = maxTextures;
			binding.stageFlags      = VK_SHADER_STAGE_FRAGMENT_BIT;
		}

        VkDescriptorBindingFlags flags = {
            VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT         |
            VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT |
            VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT 
		};

		auto flagsInfo{ VKInit::descriptorSetLayoutBindingFlagsCreateInfo() };
		{
			flagsInfo.bindingCount  = 1;
			flagsInfo.pBindingFlags = &flags;
		}

		auto layoutInfo{ VKInit::descriptorSetLayoutCreateInfo() };
		{
			layoutInfo.flags        = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
			layoutInfo.bindingCount = 1;
			layoutInfo.pBindings    = &binding;
			layoutInfo.pNext        = &flagsInfo;
		}
		VK_CHECK_SUCCESS(vkCreateDescriptorSetLayout(VyContext::device(), &layoutInfo, nullptr, &s_BindlessSetLayout), 
			"Failed to create bindless descriptor set layout!" );

		VKDbg::setObjectName(VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT, (U64)s_BindlessSetLayout, "bindless_desc_set_layout");

        // Pool for exactly maxTextures descriptors
        VkDescriptorPoolSize poolSize{};
		{
			poolSize.type            = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			poolSize.descriptorCount = maxTextures;
		}

        auto poolInfo{ VKInit::descriptorPoolCreateInfo() };
		{
			poolInfo.flags         = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
			poolInfo.poolSizeCount = 1;
			poolInfo.pPoolSizes    = &poolSize;
			poolInfo.maxSets       = 1;
		}

		VK_CHECK_SUCCESS(vkCreateDescriptorPool(VyContext::device(), &poolInfo, nullptr, &s_BindlessPool), 
			"Failed to create bindless descriptor pool!");

		VKDbg::setObjectName(VK_OBJECT_TYPE_DESCRIPTOR_POOL, (U64)s_BindlessPool, "bindless_desc_pool");

        // Allocate one set, var-count = maxTextures
		auto varInfo{ VKInit::descriptorSetVariableDescriptorCountAllocateInfo() };
		{
			varInfo.descriptorSetCount = 1;
			varInfo.pDescriptorCounts  = &maxTextures;
		}

		auto allocInfo{ VKInit::descriptorSetAllocateInfo() };
		{
			allocInfo.descriptorPool     = s_BindlessPool;
			allocInfo.descriptorSetCount = 1;
			allocInfo.pSetLayouts        = &s_BindlessSetLayout;
			allocInfo.pNext              = &varInfo;
		}

		vkAllocateDescriptorSets(VyContext::device(), &allocInfo, &s_BindlessDescriptorSet);
	}


	void VyTexture::cleanupBindless()
	{
        if (s_BindlessPool != VK_NULL_HANDLE)
        {
            vkDestroyDescriptorPool(VyContext::device(), s_BindlessPool, nullptr);

            s_BindlessPool = VK_NULL_HANDLE;
        }

        if (s_BindlessSetLayout != VK_NULL_HANDLE) 
		{
            vkDestroyDescriptorSetLayout(VyContext::device(), s_BindlessSetLayout, nullptr);

            s_BindlessSetLayout = VK_NULL_HANDLE;
        }
        
		s_BindlessDescriptorSet = VK_NULL_HANDLE;
	}


	void VyTexture::updateBindless(void* pData)
	{
        auto& data = *static_cast<VyModel::Data*>( pData );

        // we have two textures per material
        U32 N = U32( data.Textures.size() );

        TVector<VkDescriptorImageInfo> infos ( N );
        TVector<VkWriteDescriptorSet>  writes( N );

        for (U32 i = 0; i < N; ++i) 
		{
            infos[ i ] = data.Textures[ i ]->descriptorImageInfo();

			auto write{ VKInit::writeDescriptorSet() };
			{
				write.dstSet          = s_BindlessDescriptorSet;
				write.dstBinding      = 0;
				write.dstArrayElement = i;
				write.descriptorCount = 1;
				write.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				write.pImageInfo      = &infos[ i ];
			}

			writes[ i ] = write;
		}

        vkUpdateDescriptorSets(
            VyContext::device(),
            N, writes.data(),
            0, nullptr
        ); 
    }


	void VyTexture::bind(VkCommandBuffer& cmdBuffer, VkPipelineLayout& pipelineLayout)
	{
        vkCmdBindDescriptorSets(
            cmdBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipelineLayout,
            0, 1,
            &VyTexture::s_BindlessDescriptorSet,
            0, nullptr
        );
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



    VyTexture::VyTexture(const TString& filepath, VkFormat format) :
		m_Filepath{ filepath }
	{
        int desiredChannels = STBI_rgb_alpha;  // default = 4
        if (format == VK_FORMAT_R8_UNORM)     desiredChannels = STBI_grey;
        if (format == VK_FORMAT_R8G8_UNORM)   desiredChannels = STBI_grey_alpha;
        if (format == VK_FORMAT_R8G8B8_UNORM) desiredChannels = STBI_rgb;

		int texChannels;

		stbi_uc* pPixels = stbi_load(filepath.c_str(), &m_Width, &m_Height, &texChannels, desiredChannels);
		
		if (!pPixels)
		{
			VY_THROW_RUNTIME_ERROR("Failed to load texture image: " + filepath);
		}

		VkDeviceSize imageSize = m_Width * m_Height * 4;

		// Calculate mip levels
		m_MipLevels = static_cast<U32>(std::floor(std::log2(std::max(m_Width, m_Height)))) + 1;

		// Create staging buffer and copy pixel data.
		VyBuffer stagingBuffer{ VyBuffer::stagingBuffer("texture", imageSize) };

		stagingBuffer.singleWrite( pPixels );

		stbi_image_free( pPixels );

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
		// Load image using stb_image
		int texChannels;

		if (bFlipY)
		{
			stbi_set_flip_vertically_on_load(true);
		}

		stbi_uc* pPixels = stbi_load(filepath.c_str(), &m_Width, &m_Height, &texChannels, STBI_rgb_alpha);

		if (bFlipY)
		{
			stbi_set_flip_vertically_on_load(false);
		}

		if (!pPixels)
		{
			VY_THROW_RUNTIME_ERROR("Failed to load texture image: " + filepath);
		}

		VkDeviceSize imageSize = m_Width * m_Height * 4; // RGBA

		// Calculate mip levels
		m_MipLevels = static_cast<U32>(std::floor(std::log2(std::max(m_Width, m_Height)))) + 1;

		// Create staging buffer
		VyBuffer stagingBuffer{ VyBuffer::stagingBuffer("texture", imageSize) };

		stagingBuffer.singleWrite( pPixels );

		stbi_image_free( pPixels );

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


	VyTexture::VyTexture(const void* pData, size_t size, bool bSRGB /*true*/)
	{
		int      texChannels;
		stbi_uc* pPixels    = nullptr;
		bool     bIsRawData = false;

		VY_INFO_TAG("VyTexture", "Attempting to load embedded texture from memory, size: {} bytes", size);

		// First, try to load as compressed image (JPEG/PNG)
		stbi_set_flip_vertically_on_load( false ); // Temporarily disable flipping to test UV issues
        
		pPixels = stbi_load_from_memory(static_cast<const stbi_uc*>(pData), static_cast<int>(size), &m_Width, &m_Height, &texChannels, STBI_rgb_alpha);

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

		VkDeviceSize imageSize = m_Width * m_Height * 4; // RGBA

		// Calculate mip levels
		m_MipLevels = static_cast<U32>(std::floor(std::log2(std::max(m_Width, m_Height)))) + 1;

		// Create staging buffer
		VyBuffer stagingBuffer{ VyBuffer::stagingBuffer("memory_texture", imageSize) };

		stagingBuffer.singleWrite( pPixels );

		// Free the pixel data loaded by stb_image (only if it was allocated by stb_image)
		if (!bIsRawData) 
		{
			stbi_image_free( pPixels );
		}

		// Choose format based on whether this is an sRGB texture (color) or linear (data)
		VkFormat format = bSRGB ? VK_FORMAT_R8G8B8A8_SRGB : VK_FORMAT_R8G8B8A8_UNORM;

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

		// Transition image layout and copy buffer to image.
		m_Image.copyFrom( stagingBuffer, false /*toShaderReadOnly*/ );

		// Generate mipmaps (this also transitions to SHADER_READ_ONLY_OPTIMAL)
		m_Image.generateMipmaps( VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL );

		// Create image view and sampler
		createImageView(format);
		createSampler();
	}



	VyTexture::VyTexture(
		U32                width,
		U32                height,
		VkFormat           format,
		VkImageUsageFlags  usage,
		VkImageAspectFlags aspectMask,
		bool               bCreateSampler)  /*= true*/
	{
		// Create Vulkan image
        m_Image = VyImage::Builder{}
			.setName       ("gbuffer_texture")
            .setImageType  (VK_IMAGE_TYPE_2D)
            .setFormat     (format)
            .setExtent     (width, height)
            .setLevels     (1)
			.setLayers     (1)
			.setSamples    (VK_SAMPLE_COUNT_1_BIT)
            .setTiling     (VK_IMAGE_TILING_OPTIMAL)
			.setLayout     (VK_IMAGE_LAYOUT_UNDEFINED)
            .setUsage      (usage | VK_IMAGE_USAGE_SAMPLED_BIT)
            .setMemoryUsage(VMA_MEMORY_USAGE_AUTO)
        	.build();

        m_View = VyImageView::Builder{}
			.setName    ("gbuffer_texture")
            .setViewType(VK_IMAGE_VIEW_TYPE_2D)
            .setFormat  (format)
            .setAspect  (aspectMask)
            .setLevels  (0, 1)
            .setLayers  (0, 1)
        	.build( m_Image );

        //optionally create nearest-filter sampler
        if (bCreateSampler)
		{
			m_Sampler = VySampler::Builder{}
				.setName         ("gbuffer_texture")
				.setFilters      (VK_FILTER_NEAREST)
				.setMipmapMode   (VK_SAMPLER_MIPMAP_MODE_NEAREST)
				.setWrap         (VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE)
				.setBorder       (VK_BORDER_COLOR_INT_OPAQUE_BLACK)
				.setLodRange     (0.0f, 1.0f)
				.setMipLodBias   (0.0f)
				.build();
		}
	}



	VyTexture::~VyTexture()
	{
		// std::cout << "[VyTexture] Destroyed '" << m_Image.debugName() << "'" << std::endl;
	}


	// Private constructor for creating textures from memory
	VyTexture::VyTexture(const unsigned char* pPixels, int width, int height, VkFormat format) : 
		m_Width { width  },
		m_Height{ height }
	{
		VkDeviceSize imageSize = m_Width * m_Height * 4; // RGBA

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
}