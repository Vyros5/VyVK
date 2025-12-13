// #include <Vy/GFX/Registry/TextureRegistry.h>

// #include <Vy/GFX/Resources/Image/Texture.h>

// #include <Vy/GFX/Context.h>

// namespace Vy
// {
// 	VyTextureRegistry::VyTextureRegistry()
// 	{
// 		m_TextureSet          = VK_NULL_HANDLE;
// 		m_TextureSetLayout    = nullptr;
// 		m_DescriptorAllocator = nullptr;
// 	}


// 	void VyTextureRegistry::setDescriptorAllocator(Shared<VyDescriptorAllocator> descriptorAllocator) 
//     {
// 		m_DescriptorAllocator = descriptorAllocator;
// 	}


// 	TextureIndex VyTextureRegistry::add(const Shared<VyImageAsset>& image) 
//     {
// 		auto* texture = dynamic_cast<VyTexture2D*>(image.get());

// 		if (!texture) 
//         {
// 			return 0;
// 		}

// 		const auto index = static_cast<U32>(m_Textures.size());

// 		m_Textures.push_back(image);
// 		m_IDToIndex[image->AssetID] = index;

// 		if (!texture->AssetAlias.empty()) 
//         {
// 			m_AliasToIndex[texture->AssetAlias] = index;
// 		}

// 		return index;
// 	}


// 	TextureIndex VyTextureRegistry::getIndex(const VyAssetID& id) const 
//     {
// 		auto it = m_IDToIndex.find(id);
	
//         return it != m_IDToIndex.end() ? it->second : 0;
// 	}


// 	TextureIndex VyTextureRegistry::getIndex(const String& alias) const 
//     {
// 		auto it = m_AliasToIndex.find(alias);

// 		return it != m_AliasToIndex.end() ? it->second : 0;
// 	}


// 	VkDescriptorSet VyTextureRegistry::getDescriptorSet() 
//     {
// 		return m_TextureSet;
// 	}


// 	VkDescriptorSetLayout VyTextureRegistry::getDescriptorSetLayout() 
//     {
// 		return m_TextureSetLayout->handle();
// 	}


// 	void VyTextureRegistry::createDescriptorSet() 
//     {
// 		m_TextureSetLayout = VyDescriptorSetLayout::Builder()
// 			.addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, static_cast<U32>(m_Textures.size()))
// 			.buildUnique();

// 		TVector<VkDescriptorImageInfo> imageInfos;

// 		for (const auto& image : m_Textures) 
//         {
// 			const auto texture = std::static_pointer_cast<VyTexture2D>(image);

// 			VkDescriptorImageInfo imageInfo{};
//             {
//                 imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
//                 imageInfo.imageView   = texture->view();
//                 imageInfo.sampler     = texture->sampler();
//             }

// 			imageInfos.push_back(imageInfo);
// 		}

// 		m_DescriptorAllocator->allocate(m_TextureSetLayout->handle(), m_TextureSet);

// 		DescriptorWriter(*m_TextureSetLayout)
// 			.writeImages(0, imageInfos.data(), static_cast<U32>(imageInfos.size()))
// 			.update(m_TextureSet);
// 	}
// }