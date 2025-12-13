// #pragma once

// #include <Vy/Asset/Asset.h>
// #include <VyLib/STL/Path.h>
// #include <Vy/Core/Project.h>

// #include <Vy/GFX/Resources/Texture.h>

// #include <type_traits>

// #include <efsw/efsw.hpp>

// namespace Vy
// {
// 	struct AssetRefCounter
// 	{
// 		AssetRefCounter() = default;

// 		explicit AssetRefCounter(Shared<void> resource) : 
//             Resource{ std::move(resource) }
//         {
//         }
		
//         Shared<void> Resource;
// 		bool         Dirty         = false;
// 		U32          UnusedCounter = 0;
// 	};
// }


// namespace Vy
// {
// 	template <typename T> concept AssetType = std::is_base_of_v<VyAsset, T>;

// 	class VyAssetManager
// 	{
// 	public:
// 		explicit VyAssetManager();

// 		template<AssetType T> Shared<T> load(const Path& path);
// 		template<AssetType T> Shared<T> load(const VyAssetHandle& handle);

// 		VyAssetHandle getHandleFromRelative(const Path& relativePath);
// 		VyAssetHandle getHandleFromAbsolute(const Path& absolutePath);
// 		VyAssetHandle getHandleFromAsset(const Path& relativeToAssetsPath);
// 		VyAssetHandle addPathEntry(const Path& absolutePath);
// 		void          addAssetEntry(const Shared<VyAsset>& asset);

// 		void setDirty(const VyAssetHandle& handle);
// 		bool isDirty(const VyAssetHandle& handle) const;
// 		void reloadAllDirty();

// 		void populateHandleFromProject(const VyProject& project);
// 		void populateHandleFromResource();

// 		void reloadAssetFromHandle(const VyAssetHandle& handle);

// 		Path relativePathFromHandle(const VyAssetHandle& handle) const;
// 		Path absolutePathFromHandle(const VyAssetHandle& handle) const;

// 		void update();

// 	private:
// 		void populateHandleFromPath(const Path& abs);
// 		bool loadHandleFromPath(const Path& abs);
// 		void registerNewHandle(const Path& abs);

// 		void removePathForHandle(const VyAssetHandle& handle);

// 		void collectUnusedAssets(bool force = false);

// 		// const Renderer& m_Renderer;

// 		VyProject                                m_Project;
// 		THashMap<VyAssetHandle, AssetRefCounter> m_Assets;
// 		THashMap<Path, VyAssetHandle>            m_AssetPaths;


// 		// file watcher
// 		struct FileUpdateInfo 
//         {
// 			Path         OldPath;
// 			Path         NewPath;
// 			efsw::Action Action;
// 		};

// 		class AssetsListener : public efsw::FileWatchListener
// 		{
// 		public:
// 			explicit AssetsListener(VyAssetManager& assetsManager) : 
//                 m_AssetsManager(assetsManager) 
//             {
//             }

//             void handleFileAction(
//                 efsw::WatchID watchId, 
//                 const String& dir, 
//                 const String& filename,
//                 efsw::Action  action, 
//                 String        oldFilename
//             ) override;

// 			void watcherRoutine();
// 		private:
// 			void onFileAdded  (const Path& newPath) const;
// 			void onFileRemoved(const Path& newPath) const;
// 			void onFileUpdate (const Path& newPath) const;
// 			void onFileRename (const Path& oldPath, const Path& newPath) const;

// 			void computeFileInfo(const FileUpdateInfo& updateInfo) const;

// 			VyAssetManager&         m_AssetsManager;
// 			TQueue<FileUpdateInfo> m_Queue;
// 			std::mutex             m_WatcherMutex;
// 		};

// 		AssetsListener m_AssetsListener;

// 		efsw::FileWatcher m_FileWatcher;
// 		efsw::WatchID     m_WatchID = 0;
// 	};
// }

// #include <Vy/Asset/AssetManager.inl>