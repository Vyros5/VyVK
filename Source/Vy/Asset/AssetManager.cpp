// #include <Vy/Asset/AssetManager.h>

// #include <ranges>

// constexpr std::chrono::duration<float> GC_COLLECTION_DURATION = std::chrono::duration<float>(2.0f);
// constexpr std::uint32_t                MAX_UNUSED_COUNT       = 2;

// namespace Vy
// {
// 	VyAssetManager::VyAssetManager() : 
//         m_AssetsListener(*this) 
//     {
// 		populateHandleFromResource();
// 	}


// 	VyAssetHandle 
//     VyAssetManager::getHandleFromAbsolute(const Path& absolutePath)
// 	{
// 		const auto normalized = FS::absolute(absolutePath);

// 		if (m_AssetPaths.contains(normalized))
//         {
//             return m_AssetPaths[normalized];
//         }

// 		const auto handle = Serializer<VyAssetHandle>::deserialize(normalized.string() + ".imp");

// 		if (!handle)
// 		{
// 			VY_ERROR("Failed to load asset at {} : .imp file missing or invalid.\nPlease reimport it.", normalized.string());

// 			return VyAssetHandle::Invalid();
// 		}

// 		m_AssetPaths[normalized] = *handle;

// 		return m_AssetPaths[normalized];
// 	}


// 	VyAssetHandle 
//     VyAssetManager::getHandleFromAsset(const Path& relativeToAssetsPath)
// 	{
// 		const auto absolute = m_Project.projectPath() / m_Project.projectConfig().AssetDirectory / relativeToAssetsPath;

// 		return getHandleFromAbsolute(absolute);
// 	}


// 	VyAssetHandle 
//     VyAssetManager::getHandleFromRelative(const Path& relativePath)
// 	{
// 		if (relativePath.is_relative()) 
//         {
// 			return getHandleFromAbsolute(FS::absolute(relativePath));
// 		}
		
//         return getHandleFromAbsolute(relativePath);
// 	}


// 	void VyAssetManager::populateHandleFromProject(const VyProject& project)
// 	{
// 		m_Project = project;

// 		auto&      config             = m_Project.projectConfig();
// 		const auto absoluteAssetsPath = m_Project.projectPath() / config.AssetDirectory;

// 		populateHandleFromPath(absoluteAssetsPath);

// 		m_FileWatcher.removeWatch(m_WatchID);
// 		m_WatchID = m_FileWatcher.addWatch(absoluteAssetsPath.string(), &m_AssetsListener, true);
// 		m_FileWatcher.watch();
// 	}


// 	void VyAssetManager::populateHandleFromResource()
// 	{
// 		populateHandleFromPath("Data");
// 	}


// 	Path 
//     VyAssetManager::relativePathFromHandle(const VyAssetHandle& handle) const
// 	{
// 		const auto path = absolutePathFromHandle(handle);

//         if (!path.empty())
//         {
//             return FS::relative(path, m_Project.projectPath() / m_Project.projectConfig().AssetDirectory);
//         }

// 		return {};
// 	}


// 	Path 
//     VyAssetManager::absolutePathFromHandle(const VyAssetHandle& handle) const
// 	{
// 		for (const auto& [ path, h ] : m_AssetPaths) 
//         {
// 			if (h == handle)
//             {
//                 return path;
//             }
// 		}

//         return {};
// 	}


// 	VyAssetHandle 
//     VyAssetManager::addPathEntry(const Path& absolutePath)
// 	{
// 		VyAssetHandle newHandle;
// 		m_AssetPaths[ FS::absolute(absolutePath) ] = newHandle;

// 		return newHandle;
// 	}


// 	void VyAssetManager::populateHandleFromPath(const Path& abs)
// 	{
// 		THashSet<Path> alreadyVisited;

// 		for (const auto& entry : FS::recursive_directory_iterator(abs)) 
//         {
// 			auto path = FS::absolute(entry.path());

// 			if (path.extension() == ".imp") 
//             {
// 				if (loadHandleFromPath(path))
//                 {
//                     alreadyVisited.insert(path);
//                 }

//                 continue;
// 			}

// 			const auto& impPath = FS::absolute(entry.path().string() + ".imp");

// 			if (alreadyVisited.contains(impPath) || FS::exists(impPath))
//             {
//                 continue;
//             }

// 			registerNewHandle(path);
// 		}
// 	}


// 	bool VyAssetManager::loadHandleFromPath(const Path& abs)
// 	{
// 		auto       path   = FS::absolute(abs);
// 		const auto handle = Serializer<VyAssetHandle>::deserialize(path);
		
//         if (handle) 
//         {
// 			m_AssetPaths[ path.replace_extension() ] = *handle;

// 			return true;
// 		}

// 		return false;
// 	}


// 	void VyAssetManager::registerNewHandle(const Path& assetPath)
// 	{
// 		VyAssetHandle handle;
// 		m_AssetPaths[ FS::absolute(assetPath) ] = handle;

// 		Serializer<VyAssetHandle> serializer(handle);
// 		serializer.serialize(assetPath.string() + ".imp");
// 	}


// 	void VyAssetManager::reloadAssetFromHandle(const VyAssetHandle& handle)
// 	{
// 		const auto it = m_Assets.find(handle);

// 		if (it == m_Assets.end())
//         {
//             return;
//         }

// 		const auto& asset   = it->second;
// 		const auto assetPtr = std::static_pointer_cast<VyAsset>(asset.Resource);

// 		assetPtr->reload();
// 	}


// 	void VyAssetManager::removePathForHandle(const VyAssetHandle& handle)
// 	{
// 		for (auto it = m_AssetPaths.begin(); it != m_AssetPaths.end(); ++it) 
//         {
// 			if (it->second == handle) 
//             {
// 				m_AssetPaths.erase(it);

// 				return;
// 			}
// 		}
// 	}


// 	void VyAssetManager::reloadAllDirty() 
//     {
// 		if (!std::ranges::any_of(m_Assets, [](const auto& pair) { return pair.second.Dirty; })) 
//         {
// 			return;
// 		}
		
//         for (auto& asset : m_Assets | std::views::values) 
//         {
// 			const auto assetPtr = std::static_pointer_cast<VyAsset>(asset.Resource);
			
//             if (asset.Dirty) 
//             {
// 				assetPtr->reload();

// 				asset.Dirty = false;
// 			} 
//             else 
//             {
// 				asset.Dirty = assetPtr->checkForDirtyInDependencies();
// 			}
// 		}

// 		for (auto& asset: m_Assets | std::views::values) 
//         {
// 			if (asset.Dirty) 
//             {
// 				const auto assetPtr = std::static_pointer_cast<VyAsset>(asset.Resource);
				
//                 assetPtr->reload();

// 				asset.Dirty = false;
// 			}
// 		}
// 	}


// 	void VyAssetManager::setDirty(const VyAssetHandle &handle)
// 	{
// 		const auto it = m_Assets.find(handle);

// 		if (it == m_Assets.end())
// 		{
//             return;
//         }

// 		it->second.Dirty = true;
// 	}


// 	void VyAssetManager::update()
// 	{
// 		// if (!Application::Get().GetWindow().IsFocused()) return;

// 		m_AssetsListener.watcherRoutine();
// //		collectUnusedAssets();
// 	}


// 	void VyAssetManager::collectUnusedAssets(const bool force)
// 	{
// 		static auto lastCollection = std::chrono::steady_clock::now();

// 		if (!force && std::chrono::steady_clock::now() - lastCollection < GC_COLLECTION_DURATION)
//         {
//             return;
//         }

// 		for (auto it = m_Assets.begin(); it != m_Assets.end();) 
//         {
// 			if (it->second.Resource.use_count() == 1) 
//             {
// 				++it->second.UnusedCounter;
// 			}

// 			if (force || it->second.UnusedCounter > MAX_UNUSED_COUNT) 
//             {
// 				auto path = absolutePathFromHandle(it->first);

// 				VY_INFO("Unloading asset at {}", path.string());

// 				it = m_Assets.erase(it);
// 			}
// 			else 
//             {
// 				++it;
// 			}
// 		}

// 		lastCollection = std::chrono::steady_clock::now();
// 	}


// 	bool VyAssetManager::isDirty(const VyAssetHandle& handle) const
// 	{
// 		const auto it = m_Assets.find(handle);

// 		if (it == m_Assets.end())
// 		{
//             return false;
//         }	

// 		return it->second.Dirty;
// 	}


// 	void VyAssetManager::addAssetEntry(const Shared<VyAsset>& asset)
// 	{
// 		m_Assets[ asset->assetHandle() ] = AssetRefCounter(asset);
// 	}



//     // =====================================================================================================================




// 	void VyAssetManager::AssetsListener::handleFileAction(
//         efsw::WatchID watchId, 
//         const String& dir,
//         const String& filename, 
//         efsw::Action  action,
//         const String  oldFilename)
// 	{
// 		std::scoped_lock<std::mutex> lock( m_WatcherMutex );

// 		FileUpdateInfo& info = m_Queue.emplace();

// 		info.Action = action;

// 		info.NewPath = dir;
// 		info.NewPath /= filename;

// 		if (!oldFilename.empty()) 
//         {
// 			info.OldPath = dir;
// 			info.OldPath /= oldFilename;
// 		}
// 	}

	
//     void VyAssetManager::AssetsListener::watcherRoutine() 
//     {
// 		{
// 			// nested scope to unlock mutex as soon as possible
// 			std::scoped_lock<std::mutex> lock( m_WatcherMutex );

// 			while (!m_Queue.empty()) 
//             {
// 				auto& info = m_Queue.front();
                
// 				computeFileInfo(info);

// 				m_Queue.pop();
// 			}
// 		}

// 		m_AssetsManager.reloadAllDirty();
// 	}


// 	void VyAssetManager::AssetsListener::computeFileInfo(const VyAssetManager::FileUpdateInfo& updateInfo) const
// 	{
// 		switch (updateInfo.Action) 
//         {
// 			case efsw::Actions::Add:      onFileAdded  (updateInfo.NewPath); break;
// 			case efsw::Actions::Delete:	  onFileRemoved(updateInfo.NewPath); break;
// 			case efsw::Actions::Modified: onFileUpdate (updateInfo.NewPath); break;
// 			case efsw::Actions::Moved:    onFileRename (updateInfo.OldPath, updateInfo.NewPath); break;
// 		}
// 	}


// 	void VyAssetManager::AssetsListener::onFileAdded(const Path& newPath) const
// 	{
// 		if (newPath.extension() == ".imp")
// 		{
// 			m_AssetsManager.loadHandleFromPath(newPath);

// 			return;
// 		}

// 		const auto impPath = newPath.string() + ".imp";

// 		if (FS::exists(impPath))
//         {
//             return;
//         }

// 		m_AssetsManager.registerNewHandle(newPath);
// 	}


// 	void VyAssetManager::AssetsListener::onFileRemoved(const Path& newPath) const
// 	{
// 		auto finalPath = newPath;

// 		if (finalPath.extension() == ".imp") 
//         {
// 			finalPath.replace_extension();
// 		}

// 		const auto handle = m_AssetsManager.getHandleFromAbsolute(finalPath);

// 		m_AssetsManager.setDirty(handle);
// 		m_AssetsManager.removePathForHandle(handle);
// 	}


// 	void VyAssetManager::AssetsListener::onFileUpdate(const Path& newPath) const
// 	{
// 		if (newPath.extension() == ".imp") 
//         {
// 			auto finalPath = newPath;
// 			finalPath.replace_extension();

// 			const auto oldHandle = m_AssetsManager.getHandleFromAbsolute(finalPath);

// 			m_AssetsManager.setDirty(oldHandle);
// 			m_AssetsManager.removePathForHandle(oldHandle);

// 			m_AssetsManager.loadHandleFromPath(finalPath);

// 			return;
// 		}

// 		const auto handle = m_AssetsManager.getHandleFromAbsolute(newPath);

// 		m_AssetsManager.setDirty(handle);
// 	}


// 	void VyAssetManager::AssetsListener::onFileRename(const Path& oldPath, const Path& newPath) const
// 	{
// 		auto finalNewPath = FS::absolute(newPath);

// 		if (oldPath.extension() == ".imp") 
//         {
// 			auto finalOldPath = oldPath;
// 			finalOldPath.replace_extension();

// 			const auto oldHandle = m_AssetsManager.getHandleFromAbsolute(finalOldPath);

// 			m_AssetsManager.removePathForHandle(oldHandle);

// 			if (newPath.extension() == ".imp") 
//             {
// 				finalNewPath.replace_extension();

// 				m_AssetsManager.m_AssetPaths[ finalNewPath ] = oldHandle;
// 			}

// 			return;
// 		}

// 		const auto oldHandle = m_AssetsManager.getHandleFromAbsolute(oldPath);
		
//         m_AssetsManager.removePathForHandle(oldHandle);

// 		m_AssetsManager.m_AssetPaths[ finalNewPath ] = oldHandle;
// 	}
// }