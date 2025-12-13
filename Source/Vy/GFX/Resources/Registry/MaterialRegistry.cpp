// #include <Vy/GFX/Registry/MaterialRegistry.h>
// #include <Vy/GFX/Registry/TextureRegistry.h>

// #include <Vy/Asset/Types/MaterialAsset.h>


// #include <Vy/GFX/Context.h>

// namespace Vy
// {
// 	VyMaterialRegistry::VyMaterialRegistry(VyTextureRegistry& textureRegistry) : 
//         m_TextureRegistry(textureRegistry) 
//     {
// 		std::fill(m_MaterialSets.begin(), m_MaterialSets.end(), VK_NULL_HANDLE);

// 		m_MaterialSetLayout   = nullptr;
// 		m_DescriptorAllocator = nullptr;
// 	}

	
//     void VyMaterialRegistry::setDescriptorAllocator(Shared<VyDescriptorAllocator> descriptorAllocator) 
//     {
// 		m_DescriptorAllocator = descriptorAllocator;
// 	}


// 	MaterialIndex VyMaterialRegistry::add(const Shared<VyMaterialAsset> material) 
//     {
// 		const auto index = static_cast<MaterialIndex>(m_Materials.size());

// 		m_Materials.push_back(material);
// 		m_IDToIndex[material->AssetID] = index;
		
//         return index;
// 	}


// 	MaterialIndex VyMaterialRegistry::getIndex(const VyAssetID& id) const 
//     {
// 		auto it = m_IDToIndex.find(id);

// 		return it != m_IDToIndex.end() ? it->second : 0;
// 	}


// 	VkDescriptorSet VyMaterialRegistry::getDescriptorSet(int frameIndex) 
//     {
// 		return m_MaterialSets[frameIndex];
// 	}


// 	VkDescriptorSetLayout VyMaterialRegistry::getDescriptorSetLayout() 
//     {
// 		return m_MaterialSetLayout->handle();
// 	}


// 	void VyMaterialRegistry::createDescriptorSets() 
//     {
// 		m_MaterialSetLayout = VyDescriptorSetLayout::Builder()
// 			.addBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, 1)
// 			.buildUnique();

// 		for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) 
//         {
// 			m_DescriptorAllocator->allocate(
// 				m_MaterialSetLayout->handle(),
// 				m_MaterialSets[i]
// 			);
// 		}
// 	}


// 	void VyMaterialRegistry::updateDescriptorSet(int frameIndex) 
//     {
// 		TVector<MaterialData> materialsData;
		
//         for (const auto& material : m_Materials) 
//         {
// 			materialsData.push_back(getMaterialData(material));
// 		}

// 		VkDeviceSize bufferSize = sizeof(MaterialData) * materialsData.size();

// 		Unique<VyBuffer> stagingBuffer = MakeUnique<VyBuffer>( VyBuffer::stagingBuffer(bufferSize), false );

// 		stagingBuffer->map();
// 		stagingBuffer->write(materialsData.data() /*, bufferSize*/);
// 		stagingBuffer->unmap();

// 		m_MaterialsGpuBuffers[frameIndex] = MakeUnique<VyBuffer>( VyBuffer::storageBuffer(bufferSize, 1) );

// 		VyContext::device().copyBuffer(stagingBuffer->handle(), m_MaterialsGpuBuffers[frameIndex]->handle(), bufferSize);

// 		auto bufferInfo = m_MaterialsGpuBuffers[frameIndex]->descriptorBufferInfo();

// 		VyDescriptorWriter(*m_MaterialSetLayout)
// 			.writeBuffer(0, bufferInfo)
// 			.update(m_MaterialSets[frameIndex]);
// 	}


// 	MaterialData VyMaterialRegistry::getMaterialData(Shared<VyMaterialAsset> material) 
//     {
// 		constexpr MaterialIndex kInvalidMaterialIndex = std::numeric_limits<MaterialIndex>::max();

// 		MaterialData data;
//         {
//             data.albedoColor              = material->getAlbedoColor();
//             data.emissiveColor            = material->getEmissiveColor();
//             data.albedoMapIndex           = m_TextureRegistry.getIndex(material->getAlbedoMap()->AssetID);
//             data.normalMapIndex           = m_TextureRegistry.getIndex(material->getNormalMap()->AssetID);
//             data.ambientOcclusionMapIndex = m_TextureRegistry.getIndex(material->getAmbientOcclusionMap()->AssetID);
            
//             data.metallic         = material->getMetallic();
//             data.metallicMapIndex = kInvalidMaterialIndex;

//             if (material->getMetallicMap()) 
//             {
//                 data.metallicMapIndex = m_TextureRegistry.getIndex(material->getMetallicMap()->AssetID);
//             }
            
//             data.roughness         = material->getRoughness();
//             data.roughnessMapIndex = kInvalidMaterialIndex;

//             if (material->getRoughnessMap()) 
//             {
//                 data.roughnessMapIndex = m_TextureRegistry.getIndex(material->getRoughnessMap()->AssetID);
//             }
            
//             data.emissiveMapIndex = m_TextureRegistry.getIndex(material->getEmissiveMap()->AssetID);
//             data.transmission     = material->getTransmission();
//             data.ior              = material->getIndexOfRefraction();
            
//             data.blinnPhongSpecularIntensity = material->getBlinnPhongSpecularIntensity();
//             data.blinnPhongSpecularShininess = material->getBlinnPhongSpecularShininess();
//         }
	
// 		return data;
// 	}
// }