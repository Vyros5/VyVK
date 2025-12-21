#pragma once

#include <Vy/GFX/Backend/Device.h>

namespace Vy
{
	struct VySamplerCreateInfo
	{
		static constexpr float kUseMipLevels = -1.0f;

		U32                  MipLevels			     = 1;

		String               DebugName               = "";
		VkFilter             MagFilter               = VK_FILTER_LINEAR;
		VkFilter             MinFilter               = VK_FILTER_LINEAR;
		VkSamplerMipmapMode  MipmapMode              = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		VkSamplerAddressMode AddressMode             = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		float                MipLodBias              = 0.0f;
		bool                 Anisotropy              = true;
		bool                 CompareEnable           = false;
		VkCompareOp          CompareOp               = VK_COMPARE_OP_ALWAYS;
		float                MinLod                  = 0.0f;
		float                MaxLod                  = kUseMipLevels;
		VkBorderColor        BorderColor             = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
		bool                 UnnormalizedCoordinates = false;
	};
	
	class VySampler
	{
	public:
		class Builder
		{
			friend class VySampler;

		public:
			Builder() = default;

			Builder& name(String strName)
			{
				m_Info.DebugName = strName; 
				return *this;
			}

			Builder& mipLevels(U32 levels)
			{
				m_Info.MipLevels = levels; 

				return *this; 
			}

			Builder& filters(VkFilter magFilter, VkFilter minFilter)
			{
				m_Info.MagFilter = magFilter; 
				m_Info.MinFilter = minFilter; 

				return *this; 
			}

			Builder& filters(VkFilter bothFilters)
			{
				m_Info.MagFilter = bothFilters; 
				m_Info.MinFilter = bothFilters; 

				return *this; 
			}

			Builder& addressMode(VkSamplerAddressMode mode)
			{
				m_Info.AddressMode = mode; 

				return *this; 
			}

			Builder& enableAnisotropy(bool enable)
			{
				m_Info.Anisotropy = enable; 

				return *this; 
			}

			Builder& borderColor(VkBorderColor color)
			{
				m_Info.BorderColor = color; 

				return *this; 
			}

			Builder& enableCompare(VkCompareOp op)
			{
				m_Info.CompareEnable = true; 
				m_Info.CompareOp     = op; 

				return *this; 
			}

			Builder& mipmapMode(VkSamplerMipmapMode mode)
			{
				m_Info.MipmapMode = mode; 

				return *this; 
			}

			Builder& mipLodBias(float bias)
			{
				m_Info.MipLodBias = bias; 

				return *this; 
			}

			Builder& lodRange(float minLod, float maxLod = VySamplerCreateInfo::kUseMipLevels)
			{
				m_Info.MinLod = minLod; 
				m_Info.MaxLod = maxLod; 

				return *this; 
			}

			VySampler build() const 
			{
				return VySampler{ m_Info };
			}

			Unique<VySampler> buildUnique() const 
			{
				return MakeUnique<VySampler>(m_Info);
			}

			Shared<VySampler> buildShared() const 
			{
				return MakeShared<VySampler>(m_Info);
			}

		private:
			VySamplerCreateInfo m_Info{};
		};
	public:

		VySampler() = default;

		VySampler(const VySamplerCreateInfo& desc);

		VySampler(const VySampler&) = delete;
		VySampler(VySampler&& other) noexcept;
        
		~VySampler();

		VySampler& operator=(const VySampler&) = delete;
		VySampler& operator=(VySampler&& other) noexcept;

		operator     VkSampler()        const { return m_Sampler; }
		VY_NODISCARD VkSampler handle() const { return m_Sampler; }

		VY_NODISCARD VySamplerCreateInfo createInfo() const { return m_Info; }

		void setName(const String& name) const;
	
	private:
		void destroy();

		VkSampler           m_Sampler = VK_NULL_HANDLE;
		VySamplerCreateInfo m_Info;
	};
}