#pragma once


#include <future>
#include <thread>

#include <VyLib/STL/Atomic.h>
#include <VyLib/STL/Mutex.h>
#include <VyLib/STL/Utility.h>

#include <Vy/Managers/MeshManager.h>

namespace Vy
{
    // Forward declarations
    class VySampledTexture;
    class Model;
    class VyTextureManager;

    /**
     * @brief Async loading status for tracking resource load progress
     */
    enum class LoadStatus
    {
        PENDING,  // Queued for loading
        LOADING,  // Currently loading
        COMPLETE, // Successfully loaded
        FAILED    // Load failed
    };

    /**
     * @brief Handle for tracking async resource loading
     */
    template <typename T> struct AsyncLoadHandle
    {
        std::future<Shared<T>> future;
        LoadStatus                      status;
        String                     path;
        float                           progress; // 0.0 to 1.0
    };

    /**
     * @brief Resource priority for eviction policy
     * Higher priority resources are kept in cache longer
     */
    enum class ResourcePriority
    {
        LOW      = 0, // Evict first (distant objects, unused LODs)
        MEDIUM   = 1, // Standard priority (most resources)
        HIGH     = 2, // Evict last (nearby objects, current level)
        CRITICAL = 3  // Never evict (UI, player character, essential assets)
    };

    /**
     * @brief Centralized resource management with automatic deduplication and lifetime tracking
     *
     * Features:
     * - Automatic resource deduplication (same path loaded once)
     * - Memory tracking and budgeting
     * - Automatic cleanup of unused resources via weak_ptr
     * - Thread-safe resource access
     * - Prepared for future async loading support
     */
    class VyResourceManager
    {
    public:
        explicit VyResourceManager();
        ~VyResourceManager();

        // Delete copy and move operations (contains mutexes which are not movable)
        VyResourceManager(const VyResourceManager&)            = delete;
        VyResourceManager& operator=(const VyResourceManager&) = delete;
        
        VyResourceManager(VyResourceManager&&)                 = delete;
        VyResourceManager& operator=(VyResourceManager&&)      = delete;

        /**
         * @brief Load a texture from file with automatic caching
         * @param path Absolute or relative path to texture file
         * @param srgb Whether to load as sRGB format (true for color textures, false for data)
         * @param priority Resource priority for eviction policy
         * @return Shared pointer to texture (returns cached instance if already loaded)
         */
        Shared<VySampledTexture> loadTexture(const String& path, bool srgb = true, bool flipY = false, ResourcePriority priority = ResourcePriority::MEDIUM);

        /**
         * @brief Load a texture from memory with automatic caching (for embedded textures)
         * @param data VySampledTexture data in memory
         * @param dataSize Size of texture data in bytes
         * @param debugName Debug name for the texture (used for cache key)
         * @param srgb Whether to load as sRGB format
         * @param priority Resource priority for eviction policy
         * @return Shared pointer to texture (returns cached instance if same data already loaded)
         */
        Shared<VySampledTexture> loadTextureFromMemory(const unsigned char* data,
                                                    size_t               dataSize,
                                                    const String&   debugName,
                                                    bool                 srgb     = true,
                                                    ResourcePriority     priority = ResourcePriority::MEDIUM);

        /**
         * @brief Load a model from file with automatic caching
         * @param path Absolute or relative path to model file
         * @param enableTextures Whether to load textures from MTL file
         * @param loadMaterials Whether to load materials from MTL file
         * @param enableMorphTargets Whether to enable morph target support
         * @param priority Resource priority for eviction policy
         * @return Shared pointer to model (returns cached instance if already loaded)
         */
        Shared<Model> loadModel(const String& path,
                                        bool               enableTextures     = false,
                                        bool               loadMaterials      = true,
                                        bool               enableMorphTargets = false,
                                        ResourcePriority   priority           = ResourcePriority::MEDIUM);

        /**
         * @brief Remove unused resources from cache (those with no external references)
         * Call periodically (e.g., after scene transitions) to free memory
         * @return Number of resources removed
         */
        size_t garbageCollect();

        /**
         * @brief Get actual memory usage of all cached resources
         * @return Memory usage in bytes (actual, not estimated)
         */
        size_t getMemoryUsage() const;

        /**
         * @brief Get number of cached textures
         */
        size_t getCachedTextureCount() const;

        /**
         * @brief Get number of cached models
         */
        size_t getCachedModelCount() const;

        /**
         * @brief Set memory budget (resources evicted when exceeded)
         * @param budgetBytes Memory budget in bytes (0 = unlimited)
         */
        void setMemoryBudget(size_t budgetBytes);

        /**
         * @brief Get current memory budget
         * @return Memory budget in bytes (0 = unlimited)
         */
        size_t getMemoryBudget() const { return m_MemoryBudget; }

        /**
         * @brief Clear all cached resources immediately
         * Warning: This will invalidate all external shared_ptr references
         */
        void clearAll();

        /**
         * @brief Check if a texture is cached
         */
        bool isTextureCached(const String& path) const;

        /**
         * @brief Check if a model is cached
         */
        bool isModelCached(const String& path) const;

        // ========================================================================
        // ASYNC LOADING API
        // ========================================================================

        /**
         * @brief Load a texture asynchronously in background thread
         * @param path Absolute or relative path to texture file
         * @param srgb Whether to load as sRGB format
         * @param priority Resource priority for eviction policy
         * @return Future that resolves to texture when loading completes
         */
        std::future<Shared<VySampledTexture>> loadTextureAsync(const String& path, bool srgb = true, ResourcePriority priority = ResourcePriority::MEDIUM);

        /**
         * @brief Load a model asynchronously in background thread
         * @param path Absolute or relative path to model file
         * @param enableTextures Whether to load textures from MTL file
         * @param loadMaterials Whether to load materials from MTL file
         * @param enableMorphTargets Whether to enable morph target support
         * @param priority Resource priority for eviction policy
         * @return Future that resolves to model when loading completes
         */
        std::future<Shared<Model>> loadModelAsync(const String& path,
                                                        bool               enableTextures     = false,
                                                        bool               loadMaterials      = true,
                                                        bool               enableMorphTargets = false,
                                                        ResourcePriority   priority           = ResourcePriority::MEDIUM);

        /**
         * @brief Check if async loading is ready (non-blocking)
         * @return True if future is ready, false if still loading
         */
        template <typename T> static bool isReady(const std::future<Shared<T>>& future)
        {
        return future.valid() && future.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
        }

        /**
         * @brief Get number of pending async load tasks
         */
        size_t getPendingAsyncLoads() const;

        /**
         * @brief Wait for all pending async loads to complete
         */
        void waitForAsyncLoads();

        /**
         * @brief Get the VySampledTexture Manager for bindless rendering
         */
        VyTextureManager& textureManager() const { return *M_TextureManager; }

        /**
         * @brief Get the Mesh Manager for bindless rendering
         */
        VyMeshManager& meshManager() const { return *M_MeshManager; }

    private:
        Unique<VyTextureManager> M_TextureManager;
        Unique<VyMeshManager>    M_MeshManager;

        // Resource caches (weak_ptr allows automatic cleanup)
        mutable std::mutex                          m_TextureMutex;
        THashMap<String, WeakRef<VySampledTexture>> m_TextureCache;

        mutable std::mutex               m_ModelMutex;
        THashMap<String, WeakRef<Model>> m_ModelCache;

        // LRU tracking for eviction policy
        struct ResourceInfo
        {
            String           Key;
            size_t           MemorySize;
            U64              LastAccessTime;
            ResourcePriority Priority;
        };
        TVector<ResourceInfo> m_TextureAccessOrder;
        TVector<ResourceInfo> m_ModelAccessOrder;

        // Content hash cache for embedded textures (hash -> cache key)
        THashMap<String, String> m_ContentHashToKey;

        // Memory management
        size_t         m_MemoryBudget        = 0; // 0 = unlimited
        mutable size_t m_CachedTextureMemory = 0;
        mutable size_t m_CachedModelMemory   = 0;

        // Helper to generate cache key from path and parameters
        String makeTextureKey(const String& path, bool srgb) const;
        String makeModelKey(const String& path, bool enableTextures, bool loadMaterials, bool enableMorphTargets) const;

        // Memory management helpers
        void        updateTextureAccess(const String& key, size_t memorySize, ResourcePriority priority);
        void        updateModelAccess(const String& key, size_t memorySize, ResourcePriority priority);
        void        evictLRUTextures();
        void        evictLRUModels();
        U64    getCurrentTime() const;
        String computeContentHash(const unsigned char* data, size_t dataSize) const;

        // ========================================================================
        // ASYNC LOADING INFRASTRUCTURE
        // ========================================================================

        // Thread pool for async loading
        TVector<std::thread>              m_WorkerThreads;
        std::queue<std::function<void()>> m_TaskQueue;
        mutable std::mutex                m_TaskQueueMutex;
        std::condition_variable           m_TaskQueueCV;
        std::atomic<bool>                 m_ShutdownThreadPool{false};
        std::atomic<size_t>               m_ActiveTasks{0};

        // Thread pool management
        void initThreadPool(size_t numThreads = 4);
        void shutdownThreadPool();
        void workerThreadLoop();

        // Async loading helpers
        template <typename T> void enqueueTask(std::function<void()> task);
    };
}