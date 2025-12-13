#include <Vy/GFX/Backend/Resources/Sampler.h>

#include <Vy/GFX/Context.h>

namespace Vy
{
	VySampler::VySampler(const VySamplerCreateInfo& desc)
	{
		destroy();

		VkSamplerCreateInfo samplerInfo{ VKInit::samplerCreateInfo() };
		{
			// magFilter & minFilter determine how the texture is sampled when scaled up (mag) or down (min).
			// VK_FILTER_LINEAR:  Linear interpolation (blurry but smooth).
			// VK_FILTER_NEAREST: Nearest neighbor interpolation (pixelated appearance).
			samplerInfo.magFilter               = desc.MagFilter;
			samplerInfo.minFilter               = desc.MinFilter;
			
			// addressMode (U-V-W): determine what happens when texture coordinates go beyond the image boundaries.
			// Example: https://vulkan-tutorial.com/images/texture_addressing.png
			// VK_SAMPLER_ADDRESS_MODE_REPEAT:          Repeats the texture.
			// VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT: Flips every repeat.
			// VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE:   Stretches edge texels.
			// VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER: Uses a specified border color.
			samplerInfo.addressModeU            = desc.AddressMode;
			samplerInfo.addressModeV            = desc.AddressMode;
			samplerInfo.addressModeW            = desc.AddressMode;

			// enable anisotropic filtering, which improves texture quality at oblique angles.
			// https://en.wikipedia.org/wiki/Anisotropic_filtering
			samplerInfo.anisotropyEnable        = desc.Anisotropy;
			samplerInfo.maxAnisotropy           = VyContext::device().properties().limits.maxSamplerAnisotropy;

			// Mainly used for percentage-closer filtering on shadow maps.
			// https://developer.nvidia.com/gpugems/gpugems/part-ii-lighting-and-shadows/chapter-11-shadow-map-antialiasing
			samplerInfo.compareEnable           = desc.CompareEnable;
			samplerInfo.compareOp               = desc.CompareOp;

			// Which color to use when sampling outside the image borders (only if address mode is clamp to border).
			samplerInfo.borderColor             = desc.BorderColor;

			// VK_FALSE: Uses normalized texture coordinates [0,1] range, useful for giving same coords to different resolution textures.
			// VK_TRUE:  Uses actual texel coordinates [0, (texWidth) and [0, (texHeight)]
			samplerInfo.unnormalizedCoordinates = desc.UnnormalizedCoordinates;

			// Mipmapping settings
			samplerInfo.mipmapMode              = desc.MipmapMode;
			samplerInfo.mipLodBias              = desc.MipLodBias;
			samplerInfo.minLod                  = desc.MinLod;
			samplerInfo.maxLod                  = desc.MaxLod < 0.0f ? static_cast<float>(desc.MipLevels - 1) : desc.MaxLod;
		}

		VK_CHECK(vkCreateSampler(VyContext::device(), &samplerInfo, nullptr, &m_Sampler));
	}


	VySampler::VySampler(VySampler&& other) noexcept : 
        m_Sampler{ other.m_Sampler }
	{
		other.m_Sampler = VK_NULL_HANDLE;
	}


	VySampler::~VySampler()
	{
		destroy();
	}


	VySampler& VySampler::operator=(VySampler&& other) noexcept
	{
		if (this != &other)
		{
			destroy();

			m_Sampler = other.m_Sampler;

			other.m_Sampler = VK_NULL_HANDLE;
		}

		return *this;
	}


	void VySampler::destroy()
	{
		VyContext::destroy(m_Sampler);

		m_Sampler = VK_NULL_HANDLE;
	}
}