#pragma once

#include <VyEngine/VK/Device/Device.h>

namespace Vy
{
	class VySampler final
	{
	public:

		class  Builder;
		friend Builder;

	public:

		VySampler() = default;

		explicit VySampler(const TString& name, const VkSamplerCreateInfo& info);
		
        /**
         * @brief Deleted Copy constructor.
         */
		VySampler(const VySampler&) = delete;

        /**
         * @brief Move constructor.
         */
		VySampler(VySampler&& other) noexcept;
        
        /**
         * @brief Destructor for the VySampler class.
         *
         * Releases the Vulkan sampler.
         */
		~VySampler();

        /**
         * @brief Deleted Copy assignment operator.
         */
		VySampler& operator=(VySampler&) = delete;

        /**
         * @brief Move assignment operator.
         */
		VySampler& operator=(VySampler&& other) noexcept;
		
		
		void destroy();

		operator const VkSampler&() const 
		{ 
			return m_Sampler; 
		}

		const VkSampler& handle() const
		{ 
			return m_Sampler; 
		}

		VY_NODISCARD bool valid() const { return m_Sampler != VK_NULL_HANDLE; }

	private:
		VkSampler           m_Sampler  { VK_NULL_HANDLE };
		VkSamplerCreateInfo m_Info     {};
		TString             m_DebugName{ "unnamed" };
	};


	class VySampler::Builder
	{
	public:
		Builder();

		Builder& setName(const TString& name);
		Builder& setFilters(VkFilter magFilter, VkFilter minFilter);
		Builder& setFilters(VkFilter bothFilters);
		Builder& setWrap(VkSamplerAddressMode u, VkSamplerAddressMode v, VkSamplerAddressMode w);
		Builder& setWrap(VkSamplerAddressMode mode);
		Builder& enableAnisotropy(bool enable, float maxAnisotropy = -1.0f);
		Builder& setBorder(VkBorderColor color);
		Builder& enableCompare(VkCompareOp op);
		Builder& setMipmapMode(VkSamplerMipmapMode mode);
		Builder& setMipLodBias(float bias);
		Builder& setLodRange(float minLod, float maxLod);

		VySampler         build()    const;
		Unique<VySampler> buildPtr() const;

	private:
		VkSamplerCreateInfo m_Info {};
		TString              m_Name{ "unnamed" };
	};
}