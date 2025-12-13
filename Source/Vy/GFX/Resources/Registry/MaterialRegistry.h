// #pragma once

// // #include <Vy/Asset/Types/MaterialAsset.h>

// #include <Vy/GFX/Backend/Descriptors.h>
// #include <Vy/GFX/Backend/Device.h>
// #include <Vy/GFX/Backend/Resources/Buffer.h>
// #include <Vy/GFX/Backend/Swapchain.h>

// namespace Vy
// {
//     class VyTextureRegistry;

//     using MaterialIndex = U32;

// 	/**
// 	 * @struct MaterialData
// 	 *
// 	 * @brief This struct is the GPU-side representation of a material's properties. 
// 	 * It is designed to be tightly packed and uploaded to a Shader Storage Buffer Object (SSBO).
// 	 *
// 	 * @note The alignas(16) specifier is crucial. It ensures that the struct's size is a multiple
// 	 * of 16 bytes, matching the std430 layout rules for SSBOs in GLSL. This prevents memory 
// 	 * alignment issues on the GPU when accessing an array of these structs.
// 	 */
// 	struct alignas(16) MaterialData 
//     {
// 		Vec4  albedoColor;
// 		Vec4  emissiveColor;
		
// 		int   albedoMapIndex;
// 		int   normalMapIndex;
// 		int   ambientOcclusionMapIndex;
		
// 		float metallic;
// 		int   metallicMapIndex;

// 		float roughness;
// 		int   roughnessMapIndex;
		
// 		int   emissiveMapIndex;

// 		float transmission;
// 		float ior;
		
// 		float blinnPhongSpecularIntensity;
// 		float blinnPhongSpecularShininess;
// 	};

// 	/**
// 	 * @class VyMaterialRegistry
// 	 *
// 	 * @brief This class is a central manager for all Material resources in the application.
// 	 * It orchestrates the conversion of CPU-side Material objects into a GPU-consumable buffer and
// 	 * provides the necessary Vulkan descriptors for shaders to access this data.
// 	 */
// 	class VyMaterialRegistry 
//     {
// 	public:
// 		VyMaterialRegistry(VyTextureRegistry& textureRegistry);

// 		/**
// 		 * @brief Sets the descriptor allocator used to allocate descriptor sets.
// 		 *
// 		 * @param descriptorAllocator Shared pointer to a growable descriptor allocator.
// 		 */
// 		void setDescriptorAllocator(Shared<VyDescriptorAllocator> descriptorAllocator);

// 		/**
// 		 * @brief Adds a material to the registry.
// 		 *
// 		 * @param material Shared pointer to the material to add.
// 		 *
// 		 * @return Index of the added material in the registry.
// 		 */
// 		MaterialIndex add(const Shared<VyMaterialAsset> material);

// 		/**
// 		 * @brief Retrieves the index of a material by its resource ID.
// 		 *
// 		 * @param id The resource ID of the material.
// 		 *
// 		 * @return Index of the material if found, otherwise 0.
// 		 */
// 		MaterialIndex getIndex(const VyAssetID& id) const;

// 		/**
// 		 * @brief Gets the Vulkan descriptor set used for the materials in the frame frameIndex.
// 		 *
// 		 * @return The Vulkan descriptor set.
// 		 */
// 		VkDescriptorSet getDescriptorSet(int frameIndex);

// 		/**
// 		 * @brief Gets the Vulkan descriptor set layout used for the materials.
// 		 *
// 		 * @return The Vulkan descriptor set layout.
// 		 */
// 		VkDescriptorSetLayout getDescriptorSetLayout();

// 		/**
// 		 * @brief Creates the descriptor set and GPU buffer for the registered materials.
// 		 *
// 		 * This method prepares the material data for use in shaders by uploading it to GPU memory
// 		 * and writing it into a Vulkan descriptor set.
// 		 */
// 		void createDescriptorSets();

// 		void updateDescriptorSet(int frameIndex);

// 	private:
// 		/**
// 		 * @brief Converts a Material object into its corresponding GPU-ready MaterialData structure.
// 		 *
// 		 * @param material Shared pointer to the material.
// 		 *
// 		 * @return A MaterialData struct containing data for GPU usage.
// 		 */
// 		MaterialData getMaterialData(Shared<VyMaterialAsset> material);

// 		VyTextureRegistry&            m_TextureRegistry;
// 		Shared<VyDescriptorAllocator> m_DescriptorAllocator;

// 		TVector<Shared<VyMaterialAsset>>   m_Materials;
// 		THashMap<VyAssetID, MaterialIndex> m_IDToIndex;

// 		TVector<Unique<VyBuffer>>     m_MaterialsGpuBuffers { MAX_FRAMES_IN_FLIGHT };
// 		TVector<VkDescriptorSet>      m_MaterialSets        { MAX_FRAMES_IN_FLIGHT };
// 		Shared<VyDescriptorSetLayout> m_MaterialSetLayout = nullptr;
// 	};
// }