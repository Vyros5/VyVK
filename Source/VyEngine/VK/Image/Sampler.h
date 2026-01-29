#pragma once

#include <VyEngine/VK/Device/Device.h>

namespace Vy
{
	struct VySamplerInfo
	{
		static constexpr float USE_MIP_LEVELS = -1.0f;

		VkFilter             MagFilter     = VK_FILTER_LINEAR;
		VkFilter             MinFilter     = VK_FILTER_LINEAR;
		VkSamplerMipmapMode  MipmapMode    = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		VkSamplerAddressMode AddressMode   = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		// VkSamplerAddressMode AddressModeU  = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		// VkSamplerAddressMode AddressModeV  = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		VkSamplerAddressMode AddressModeW  = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		bool                 Anisotropy    = true;
		float                MaxAnisotropy = 0;
		float                MipLodBias    = 0.0f;
		bool                 CompareEnable = false;
		VkCompareOp          CompareOp     = VK_COMPARE_OP_ALWAYS;
		float                MinLod        = 0.0f;
		float                MaxLod        = USE_MIP_LEVELS;
		VkBorderColor        BorderColor   = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		bool                 UnormCoords   = false;

		U32                  MipLevels     = 1;
	};

	class VySampler final
	{
	public:

		class  Builder;
		friend Builder;

	public:

		VySampler() = default;

		explicit VySampler(const TString& name, const VySamplerInfo& info, U32 mipLevels = 1);
		
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
		VkSampler     m_Sampler  { VK_NULL_HANDLE };
		VySamplerInfo m_Info     {};
		TString       m_DebugName{ "unnamed" };
	};


	class VySampler::Builder
	{
	public:
		Builder();

		Builder& setName(const TString& name);
		Builder& setMipLevels(U32 mips);
		Builder& setFilters(VkFilter magFilter, VkFilter minFilter);
		Builder& setFilters(VkFilter bothFilters);
		// Builder& setWrap(VkSamplerAddressMode u, VkSamplerAddressMode v, VkSamplerAddressMode w);
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
		TString       m_Name{ "unnamed" };
		VySamplerInfo m_Info {};
	};
}