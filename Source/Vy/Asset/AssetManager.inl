

// #include <Vy/Asset/AssetManager.h>


// namespace Vy
// {
// 	template<AssetType T>
// 	inline Shared<T> VyAssetManager::load(const Path& path)
// 	{
// 		return load<T>(getHandleFromAsset(path));
// 	}


// 	template<AssetType T>
// 	inline Shared<T> VyAssetManager::load(const VyAssetHandle& handle)
// 	{
// 		VY_ASSERT(false, "Asset type not supported");
// 	}


// 	// template<>
// 	// inline Shared<VyTexture> VyAssetManager::load(const VyAssetHandle& handle)
// 	// {
// 	// 	if (m_Assets.contains(handle)) 
//     //     {
// 	// 		return std::static_pointer_cast<VyTexture>(m_Assets.at(handle).Resource);
// 	// 	}
// 	// 	auto path = absolutePathFromHandle(handle);

// 	// 	auto texture = VyTexture::load(path);

// 	// 	// Shared<VyTexture> texture = Texture::Builder(m_Renderer.GetDevice())
// 	// 	// 	.SetAssetHandle(handle)
// 	// 	// 	.Build();

// 	// 	m_Assets[handle] = AssetRefCounter(texture);

// 	// 	return texture;
// 	// }


// 	// template<>
// 	// inline Shared<Material> VyAssetManager::load(const VyAssetHandle& handle)
// 	// {
// 	// 	if (m_Assets.contains(handle)) {
// 	// 		return std::static_pointer_cast<Material>(m_Assets.at(handle).Resource);
// 	// 	}
// 	// 	auto path = AbsolutePathFromHandle(handle);
// 	// 	auto materialData = Serializer<MaterialData>::deserialize(path);
// 	// 	if (!materialData) {
// 	// 		return nullptr;
// 	// 	}

// 	// 	auto material = std::make_shared<Material>(m_Renderer.GetDevice(), *materialData, handle);
// 	// 	m_Assets[handle] = AssetRefCounter(material);

// 	// 	return material;
// 	// }


// 	// template<>
// 	// inline Shared<MeshRenderer> VyAssetManager::load(const VyAssetHandle& handle)
// 	// {
// 	// 	if (m_Assets.contains(handle)) {
// 	// 		return std::static_pointer_cast<MeshRenderer>(m_Assets.at(handle).Resource);
// 	// 	}
// 	// 	const auto path = AbsolutePathFromHandle(handle);
// 	// 	auto materialData = Model::FromFile(path);

// 	// 	auto material = std::make_shared<MeshRenderer>(m_Renderer.GetDevice(), handle, materialData);
// 	// 	m_Assets[handle] = AssetRefCounter(material);

// 	// 	return material;
// 	// }

    
// 	// template<>
// 	// inline Shared<Shader> VyAssetManager::load(const VyAssetHandle& handle)
// 	// {
// 	// 	return nullptr;
// 	// }
// }