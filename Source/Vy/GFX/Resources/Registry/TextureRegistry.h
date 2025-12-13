// #pragma once

// // #include <Vy/GFX/Resources/Material.h>

// // #include <Vy/Asset/Types/ImageAsset.h>

// #include <Vy/GFX/Backend/Descriptors.h>
// #include <Vy/GFX/Backend/Device.h>
// #include <Vy/GFX/Backend/Resources/Buffer.h>
// #include <Vy/GFX/Backend/Swapchain.h>

// namespace Vy
// {
//     using TextureIndex = U32;

// 	/**
// 	 * @class VyTextureRegistry
// 	 *
// 	 * @brief Manages a collection of textures and their binding to GPU descriptor sets.
// 	 */
// 	class VyTextureRegistry 
//     {
// 	public:
// 		explicit VyTextureRegistry();

// 		/**
// 		 * @brief Sets the descriptor allocator for the registry.
// 		 *
// 		 * @param descriptorAllocator Shared pointer to a growable descriptor allocator.
// 		 */
// 		void setDescriptorAllocator(Shared<VyDescriptorAllocator> descriptorAllocator);

// 		/**
// 		 * @brief Adds a texture to the registry.
// 		 *
// 		 * Only 2D textures (Texture2D) are supported. 
// 		 * If the provided image is not a Texture2D, the function returns 0.
// 		 *
// 		 * @param image Shared pointer to the image.
// 		 *
// 		 * @return Index of the added texture if valid, otherwise 0.
// 		 */
// 		TextureIndex add(const Shared<VyImageAsset>& image);

// 		/**
// 		 * @brief Gets the index of a texture in the registry by its resource ID.
// 		 *
// 		 * @param id Resource ID of the texture.
// 		 *
// 		 * @return Index if found, otherwise 0.
// 		 */
// 		VY_NODISCARD TextureIndex getIndex(const VyAssetID& id) const;

// 		/**
// 		 * @brief Gets the index of a texture in the registry by its resource alias (string path or unique name).
// 		 *
// 		 * @param id Resource ID of the texture.
// 		 *
// 		 * @return Index if found, otherwise 0.
// 		 */
// 		VY_NODISCARD TextureIndex getIndex(const String& alias) const;


// 		U32 getTextureCount() const 
//         {
// 			return static_cast<U32>(m_Textures.size());
// 		}

// 		/**
// 		 * @brief Returns the Vulkan descriptor set that holds all texture bindings.
// 		 *
// 		 * @return Vulkan descriptor set.
// 		 */
// 		VkDescriptorSet getDescriptorSet();

// 		/**
// 		 * @brief Returns the Vulkan descriptor set layout used for texture bindings.
// 		 *
// 		 * @return Vulkan descriptor set layout.
// 		 */
// 		VkDescriptorSetLayout getDescriptorSetLayout();

// 		/**
// 		 * @brief Creates a Vulkan descriptor set for all registered textures.
// 		 *
// 		 * This function constructs a descriptor set layout with a combined image sampler binding,
// 		 * prepares image info for each texture, allocates the descriptor set, and writes all image bindings to it.
// 		 */
// 		void createDescriptorSet();

// 	private:
// 		TVector<Shared<VyImageAsset>>     m_Textures;
// 		THashMap<VyAssetID, TextureIndex> m_IDToIndex;
// 		THashMap<String,    TextureIndex> m_AliasToIndex;

// 		Shared<VyDescriptorAllocator> m_DescriptorAllocator;
// 		Shared<VyDescriptorSetLayout> m_TextureSetLayout;
// 		VkDescriptorSet               m_TextureSet;
// 	};
// }