#include <VyEngine/VK/Image/Sampler.h>

#include <VyEngine/VK/Context.h>

namespace Vy
{

	VySampler::VySampler(
		const TString&             name, 
		const VkSamplerCreateInfo& info
	) :
		m_Info      { info              },
		m_DebugName { name + "_sampler" }
	{
		if (m_Info.maxAnisotropy = -1.0f)
		{
			m_Info.maxAnisotropy = VyContext::device().limits().maxSamplerAnisotropy;
		}

		VK_CHECK_SUCCESS(vkCreateSampler(VyContext::device(), &m_Info, nullptr, &m_Sampler), 
			"Failed to create sampler!");

		VKDbg::setObjectName(m_Sampler, m_DebugName.c_str());
	}
		

	VySampler::VySampler(VySampler&& other) noexcept : 
        m_Sampler  { other.m_Sampler   },
		m_DebugName{ other.m_DebugName },
		m_Info     { other.m_Info      }
	{
		other.m_Sampler = VK_NULL_HANDLE;
	}


	VySampler::~VySampler()
	{
		this->destroy();
	}


	VySampler& VySampler::operator=(VySampler&& other) noexcept
	{
		if (this != &other)
		{
			this->destroy();

			m_Sampler   = other.m_Sampler;
			m_DebugName = other.m_DebugName;
			m_Info      = other.m_Info;

			other.m_Sampler = VK_NULL_HANDLE;
		}

		return *this;
	}


	void VySampler::destroy() 
	{ 
		VyContext::destroy(m_Sampler); 

		m_Sampler = VK_NULL_HANDLE;
	}



	VySampler::Builder::Builder()
	{
		// Setup Default Values
		m_Info = VKInit::samplerCreateInfo();
		{
			m_Info.magFilter               = VK_FILTER_LINEAR;
			m_Info.minFilter               = VK_FILTER_LINEAR;
			m_Info.addressModeU            = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			m_Info.addressModeV            = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			m_Info.addressModeW            = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			m_Info.anisotropyEnable        = VK_FALSE;
			m_Info.maxAnisotropy           = 0;
			m_Info.borderColor             = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
			m_Info.unnormalizedCoordinates = VK_FALSE;
			m_Info.compareEnable           = VK_FALSE;
			m_Info.compareOp               = VK_COMPARE_OP_ALWAYS;
			m_Info.mipmapMode              = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			m_Info.mipLodBias              = 0.0f;
			m_Info.minLod                  = 0.0f;
			m_Info.maxLod                  = VK_LOD_CLAMP_NONE;
		}
	}

	VySampler::Builder& 
	VySampler::Builder::setName(const TString& name)
	{
		m_Name = name; 

		return *this; 
	}


	VySampler::Builder& 
	VySampler::Builder::setFilters(VkFilter magFilter, VkFilter minFilter)
	{
		m_Info.magFilter = magFilter; 
		m_Info.minFilter = minFilter; 

		return *this; 
	}

	VySampler::Builder& 
	VySampler::Builder::setFilters(VkFilter bothFilters)
	{
		m_Info.magFilter = bothFilters; 
		m_Info.minFilter = bothFilters; 

		return *this; 
	}

	VySampler::Builder& 
	VySampler::Builder::setWrap(VkSamplerAddressMode u, VkSamplerAddressMode v, VkSamplerAddressMode w)
	{
		m_Info.addressModeU = u; 
		m_Info.addressModeV = v;
		m_Info.addressModeW = w;  

		return *this;
	}

	VySampler::Builder& 
	VySampler::Builder::setWrap(VkSamplerAddressMode mode)
	{
		m_Info.addressModeU = mode; 
		m_Info.addressModeV = mode;
		m_Info.addressModeW = mode;  

		return *this; 
	}

	VySampler::Builder& 
	VySampler::Builder::enableAnisotropy(bool enable, float maxAnisotropy /* = -1.0f*/)
	{
		m_Info.anisotropyEnable = enable;
		m_Info.maxAnisotropy = maxAnisotropy;


		return *this; 
	}

	VySampler::Builder& 
	VySampler::Builder::setBorder(VkBorderColor color)
	{
		m_Info.borderColor = color; 

		return *this; 
	}

	VySampler::Builder& 
	VySampler::Builder::enableCompare(VkCompareOp op)
	{
		m_Info.compareEnable = true; 
		m_Info.compareOp     = op; 

		return *this; 
	}

	VySampler::Builder& 
	VySampler::Builder::setMipmapMode(VkSamplerMipmapMode mode)
	{
		m_Info.mipmapMode = mode; 

		return *this; 
	}

	VySampler::Builder& 
	VySampler::Builder::setMipLodBias(float bias)
	{
		m_Info.mipLodBias = bias; 

		return *this; 
	}

	VySampler::Builder& 
	VySampler::Builder::setLodRange(float minLod, float maxLod)
	{
		m_Info.minLod = minLod; 
		m_Info.maxLod = maxLod; 

		return *this; 
	}

	VySampler 
	VySampler::Builder::build() const 
	{
		return VySampler{ m_Name, m_Info };
	}

	Unique<VySampler> 
	VySampler::Builder::buildPtr() const 
	{
		return MakeUnique<VySampler>( m_Name, m_Info );
	}
}