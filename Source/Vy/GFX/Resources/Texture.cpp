#include <Vy/GFX/Resources/Texture.h>

#include <Vy/GFX/Context.h>

// #include <Vy/GFX/Backend/Pipeline.h>
// #include <Vy/GFX/Backend/Descriptors.h>
#include <Vy/GFX/Backend/Buffer/Buffer.h>

// #include <Vy/Engine.h>
#include <Vy/Globals.h>
// #include <Vylib/Util/Hash.h>

// #define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace Vy
{
	Shared<VySampledTexture> VySampledTexture::createWhiteTexture()
	{
		unsigned char whitePixel[4] = { 255, 255, 255, 255 };

		return MakeShared<VySampledTexture>(whitePixel, 1, 1, VK_FORMAT_R8G8B8A8_UNORM);
	}

	
	Shared<VySampledTexture> VySampledTexture::createNormalTexture()
	{
		// Flat normal pointing up in tangent space: (0, 0, 1) -> (128, 128, 255) in RGB
		unsigned char normalPixel[4] = { 128, 128, 255, 255 };

		return MakeShared<VySampledTexture>(normalPixel, 1, 1, VK_FORMAT_R8G8B8A8_UNORM);
	}


	VySampledTexture::VySampledTexture(const String& filepath, bool bSRGB, bool bFlipY)
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
		VyBuffer stagingBuffer{ VyBuffer::stagingBuffer(imageSize) };

		stagingBuffer.map();
		stagingBuffer.writeToBuffer(pPixels);
		stagingBuffer.unmap();

		stbi_image_free(pPixels);

		// Choose format based on whether this is an sRGB texture (color) or linear (data)
		VkFormat format = bSRGB ? VK_FORMAT_R8G8B8A8_SRGB : VK_FORMAT_R8G8B8A8_UNORM;

		// Create Vulkan image
        m_Image = VyImage::Builder{}
            .imageType  (VK_IMAGE_TYPE_2D)
            .format     (format)
            .extent     (static_cast<U32>(m_Width), static_cast<U32>(m_Height))
            .mipLevels  (m_MipLevels)
			.arrayLayers(1)
			.sampleCount(VK_SAMPLE_COUNT_1_BIT)
            .tiling     (VK_IMAGE_TILING_OPTIMAL)
			.imageLayout(VK_IMAGE_LAYOUT_UNDEFINED)
            .usage      (VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
            .memoryUsage(VMA_MEMORY_USAGE_AUTO)
        .build();

		// Transition image layout and copy buffer to image.
		m_Image.copyFrom(stagingBuffer, false /*toShaderReadOnly*/);

		// Generate mipmaps (this also transitions to SHADER_READ_ONLY_OPTIMAL)
		m_Image.generateMipmaps(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		// Create image view and sampler
		createImageView(format);
		createSampler();

		VY_INFO_TAG("VySampledTexture", "Loaded: {0} ({1}x{2}, {3} mips)", filepath, m_Width, m_Height, m_MipLevels);
	}


	VySampledTexture::~VySampledTexture()
	{
	}


	// Private constructor for creating textures from memory
	VySampledTexture::VySampledTexture(const unsigned char* pPixels, int width, int height, VkFormat format) : 
		m_Width { width  }, 
		m_Height{ height }
	{
		VkDeviceSize imageSize = m_Width * m_Height * 4; // RGBA

		m_MipLevels = 1; // No mipmaps for default textures

		// Create staging buffer
		VyBuffer stagingBuffer{ VyBuffer::stagingBuffer(imageSize) };

		stagingBuffer.map();
		stagingBuffer.writeToBuffer((void*)pPixels);
		stagingBuffer.unmap();

		// Create Vulkan image
        m_Image = VyImage::Builder{}
            .imageType  (VK_IMAGE_TYPE_2D)
            .format     (format)
            .extent     (static_cast<U32>(m_Width), static_cast<U32>(m_Height))
            .mipLevels  (m_MipLevels)
			.arrayLayers(1)
			.sampleCount(VK_SAMPLE_COUNT_1_BIT)
            .tiling     (VK_IMAGE_TILING_OPTIMAL)
			.imageLayout(VK_IMAGE_LAYOUT_UNDEFINED)
            .usage      (VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
            .memoryUsage(VMA_MEMORY_USAGE_AUTO)
        .build();

		// Transition image layout and copy buffer to image.
		m_Image.copyFrom(stagingBuffer, true /*toShaderReadOnly*/);

		// Create image view and sampler
		createImageView(format);
		createSampler();

		VY_INFO_TAG("VySampledTexture", "Loaded texture from memory ({1}x{2}, {3} mips)", m_Width, m_Height, m_MipLevels);
	}


	void VySampledTexture::createImageView(VkFormat format)
	{
        m_View = VyImageView::Builder{}
            .viewType   (VK_IMAGE_VIEW_TYPE_2D)
            .format     (format)
            .aspectMask (VK_IMAGE_ASPECT_COLOR_BIT)
            .mipLevels  (0, m_MipLevels)
            .arrayLayers(0, 1)
        .build(m_Image);
	}


	void VySampledTexture::createSampler()
	{
        m_Sampler = VySampler::Builder{}
            .filters         (VK_FILTER_LINEAR)
            .mipmapMode      (VK_SAMPLER_MIPMAP_MODE_LINEAR)
            .addressMode     (VK_SAMPLER_ADDRESS_MODE_REPEAT)
            .borderColor     (VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE)
			.enableAnisotropy(true)
            .lodRange        (0.0f, static_cast<float>(m_MipLevels))
            .mipLodBias      (0.0f)
        .build();
	}


	size_t VySampledTexture::memorySize() const
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