#include <VyEngine/VK/Image/Sampler.h>

#include <VyEngine/VK/Context.h>

namespace Vy
{

	VySampler::VySampler(
		const TString&       name, 
		const VySamplerInfo& info,
		U32                  mipLevels
	) :
		m_Info      { info              },
		m_DebugName { name + "_sampler" }
	{
		VkSamplerCreateInfo samplerInfo{ VKInit::samplerCreateInfo() };
		{
			samplerInfo.magFilter               = info.MagFilter;
			samplerInfo.minFilter               = info.MinFilter;
			samplerInfo.mipmapMode              = info.MipmapMode;
			samplerInfo.addressModeU            = info.AddressMode;
			samplerInfo.addressModeV            = info.AddressMode;
			samplerInfo.addressModeW            = info.AddressMode;
			samplerInfo.mipLodBias              = 0.0f;
			samplerInfo.anisotropyEnable        = info.Anisotropy;
			samplerInfo.maxAnisotropy           = VyContext::device().limits().maxSamplerAnisotropy;
			samplerInfo.compareEnable           = info.CompareEnable;
			samplerInfo.compareOp               = info.CompareOp;
			samplerInfo.minLod                  = info.MinLod;
			samplerInfo.maxLod                  = info.MaxLod < 0.0f ? static_cast<float>(mipLevels - 1) : info.MaxLod;
			samplerInfo.borderColor             = info.BorderColor;
			samplerInfo.unnormalizedCoordinates = info.UnormCoords;
		}

		VK_CHECK_SUCCESS(vkCreateSampler(
			VyContext::device(), 
			&samplerInfo, 
			nullptr, 
			&m_Sampler
		), "Failed to create sampler!");

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
	}


	VySampler::Builder& 
	VySampler::Builder::setName(const TString& name)
	{
		m_Name = name; 

		return *this; 
	}


	VySampler::Builder& 
	VySampler::Builder::setMipLevels(U32 mips)
	{
		m_Info.MipLevels = mips; 

		return *this; 
	}


	VySampler::Builder& 
	VySampler::Builder::setFilters(VkFilter magFilter, VkFilter minFilter)
	{
		m_Info.MagFilter = magFilter; 
		m_Info.MinFilter = minFilter; 

		return *this; 
	}

	VySampler::Builder& 
	VySampler::Builder::setFilters(VkFilter bothFilters)
	{
		m_Info.MagFilter = bothFilters; 
		m_Info.MinFilter = bothFilters; 

		return *this; 
	}

	// VySampler::Builder& 
	// VySampler::Builder::setWrap(VkSamplerAddressMode u, VkSamplerAddressMode v, VkSamplerAddressMode w)
	// {
	// 	m_Info.AddressModeU = u; 
	// 	m_Info.AddressModeV = v;
	// 	m_Info.AddressModeW = w;  

	// 	return *this;
	// }

	VySampler::Builder& 
	VySampler::Builder::setWrap(VkSamplerAddressMode mode)
	{
		m_Info.AddressMode = mode;
		// m_Info.AddressModeU = mode; 
		// m_Info.AddressModeV = mode;
		// m_Info.AddressModeW = mode;  

		return *this; 
	}

	VySampler::Builder& 
	VySampler::Builder::enableAnisotropy(bool enable, float maxAnisotropy /* = -1.0f*/)
	{
		m_Info.Anisotropy    = enable;
		m_Info.MaxAnisotropy = maxAnisotropy;


		return *this; 
	}

	VySampler::Builder& 
	VySampler::Builder::setBorder(VkBorderColor color)
	{
		m_Info.BorderColor = color; 

		return *this; 
	}

	VySampler::Builder& 
	VySampler::Builder::enableCompare(VkCompareOp op)
	{
		m_Info.CompareEnable = true; 
		m_Info.CompareOp     = op; 

		return *this; 
	}

	VySampler::Builder& 
	VySampler::Builder::setMipmapMode(VkSamplerMipmapMode mode)
	{
		m_Info.MipmapMode = mode; 

		return *this; 
	}

	VySampler::Builder& 
	VySampler::Builder::setMipLodBias(float bias)
	{
		m_Info.MipLodBias = bias; 

		return *this; 
	}

	VySampler::Builder& 
	VySampler::Builder::setLodRange(float minLod, float maxLod)
	{
		m_Info.MinLod = minLod; 
		m_Info.MaxLod = maxLod; 

		return *this; 
	}

	VySampler 
	VySampler::Builder::build() const 
	{
		return VySampler{ m_Name, m_Info, m_Info.MipLevels };
	}

	Unique<VySampler> 
	VySampler::Builder::buildPtr() const 
	{
		return MakeUnique<VySampler>( m_Name, m_Info, m_Info.MipLevels );
	}
}