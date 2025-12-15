#include <Vy/Managers/ResourceManager.h>

#include <Vy/Managers/TextureManager.h>

#include <Vy/GFX/Context.h>
#include <Vy/Globals.h>

#include <algorithm>
#include <chrono>
#include <iomanip>
#include <sstream>

#include <Vy/GFX/Resources/Mesh.h>
#include <Vy/GFX/Resources/Texture.h>

// Simple SHA256 implementation for content hashing
#include <cstdint>
#include <cstring>

namespace Vy
{
    // Simple FNV-1a hash (fast, good distribution)
    static U64 hashBytes(const unsigned char* data, size_t length)
    {
        U64 hash = 14695981039346656037ULL; // FNV offset basis

        for (size_t i = 0; i < length; ++i)
        {
            hash ^= data[i];
            hash *= 1099511628211ULL; // FNV prime
        }

        return hash;
    }


    VyResourceManager::VyResourceManager()
    {
        M_TextureManager = std::make_unique<VyTextureManager>();
        M_MeshManager    = std::make_unique<VyMeshManager>();

        // Initialize thread pool with hardware concurrency
        size_t numThreads = std::thread::hardware_concurrency();

        if (numThreads == 0) 
        {
            numThreads = 4; // Fallback
        }
        
        initThreadPool(numThreads);
    }


    VyResourceManager::~VyResourceManager()
    {
        shutdownThreadPool();
    }


    String VyResourceManager::makeTextureKey(const String& path, bool srgb) const
    {
        // Include srgb flag in key since same texture can be loaded with different formats
        return path + (srgb ? "|srgb" : "|linear");
    }


    String VyResourceManager::makeModelKey(const String& path, bool enableTextures, bool loadMaterials, bool enableMorphTargets) const
    {
        // Include loading flags in key since same model can be loaded with different settings
        std::ostringstream oss;
        oss << path << "|tex=" << enableTextures << "|mat=" << loadMaterials << "|morph=" << enableMorphTargets;

        return oss.str();
    }


    Shared<VySampledTexture> 
    VyResourceManager::loadTexture(
        const String&    path, 
        bool             srgb, 
        bool             flipY, 
        ResourcePriority priority)
    {
        String key = makeTextureKey(path, srgb) + (flipY ? "|flipY" : "");

        // Lock for thread-safe access
        std::lock_guard<std::mutex> lock(m_TextureMutex);

        // Check if texture is already cached
        auto it = m_TextureCache.find(key);

        if (it != m_TextureCache.end())
        {
            // Try to lock the weak_ptr to get a shared_ptr.
            if (auto cachedTexture = it->second.lock())
            {
                // Texture still exists, update LRU access time and priority.
                updateTextureAccess(key, cachedTexture->memorySize(), priority);

                return cachedTexture;
            }
            else
            {
                // Texture was deleted, remove stale entry.
                m_TextureCache.erase(it);

                // Remove from access tracking.
                m_TextureAccessOrder.erase(
                    std::remove_if(m_TextureAccessOrder.begin(), m_TextureAccessOrder.end(), 
                    [&key](const ResourceInfo& info) 
                    { 
                        return info.Key == key; 
                    }),
                    m_TextureAccessOrder.end()
                );
            }
        }

        // Load new texture.
        auto   texture = std::make_shared<VySampledTexture>(path, srgb, flipY);
        size_t memSize = texture->memorySize();

        // Check memory budget and evict if necessary.
        if (m_MemoryBudget > 0)
        {
            m_CachedTextureMemory += memSize;
            
            while (m_CachedTextureMemory > m_MemoryBudget && !m_TextureCache.empty())
            {
                evictLRUTextures();
            }
        }

        // Cache the texture (as weak_ptr).
        m_TextureCache[key] = texture;
        updateTextureAccess(key, memSize, priority);

        // Register with VyTextureManager.
        U32 globalIndex = M_TextureManager->addTexture(texture);

        texture->setGlobalIndex(globalIndex);

        return texture;
    }


    Shared<Model>
    VyResourceManager::loadModel(
        const String&    path, 
        bool             enableTextures, 
        bool             loadMaterials, 
        bool             enableMorphTargets, 
        ResourcePriority priority)
    {
        String key = makeModelKey(path, enableTextures, loadMaterials, enableMorphTargets);

        // Lock for thread-safe access.
        std::lock_guard<std::mutex> lock(m_ModelMutex);

        // Check if model is already cached.
        auto it = m_ModelCache.find(key);

        if (it != m_ModelCache.end())
        {
            // Try to lock the weak_ptr to get a shared_ptr.
            if (auto cachedModel = it->second.lock())
            {
                // Model still exists, update LRU access time and priority.
                updateModelAccess(key, cachedModel->memorySize(), priority);

                return cachedModel;
            }
            else
            {
                // Model was deleted, remove stale entry.
                m_ModelCache.erase(it);

                // Remove from access tracking.
                m_ModelAccessOrder.erase(
                    std::remove_if(m_ModelAccessOrder.begin(), m_ModelAccessOrder.end(), 
                    [&key](const ResourceInfo& info) 
                    { 
                        return info.Key == key; 
                    }),
                    m_ModelAccessOrder.end()
                );
            }
        }

        // Load new model.
        auto   model   = Shared<Model>(Model::createModelFromFile(path, enableTextures, loadMaterials, enableMorphTargets));
        size_t memSize = model->memorySize();

        // Check memory budget and evict if necessary.
        if (m_MemoryBudget > 0)
        {
            m_CachedModelMemory += memSize;

            while (m_CachedModelMemory > m_MemoryBudget && !m_ModelCache.empty())
            {
                evictLRUModels();
            }
        }

        // Cache the model (as weak_ptr).
        m_ModelCache[key] = model;
        updateModelAccess(key, memSize, priority);

        // Register with VyMeshManager.
        U32 meshId = M_MeshManager->registerModel(model.get());
        model->setMeshId(meshId);

        return model;
    }


    Shared<VySampledTexture>
    VyResourceManager::loadTextureFromMemory(
        const unsigned char* data, 
        size_t               dataSize, 
        const String&        debugName, 
        bool                 srgb, 
        ResourcePriority     priority)
    {
        // Compute content hash for deduplication.
        String contentHash = computeContentHash(data, dataSize);
        String cacheKey;

        // Lock for thread-safe access.
        std::lock_guard<std::mutex> lock(m_TextureMutex);

        // Check if we've already loaded this exact content.
        auto hashIt = m_ContentHashToKey.find(contentHash);

        if (hashIt != m_ContentHashToKey.end())
        {
            cacheKey = hashIt->second;
            auto it  = m_TextureCache.find(cacheKey);

            if (it != m_TextureCache.end())
            {
                if (auto cachedTexture = it->second.lock())
                {
                    // Same content already loaded, return cached instance.
                    updateTextureAccess(cacheKey, cachedTexture->memorySize(), priority);
                    
                    return cachedTexture;
                }
            }
        }

        // Create unique cache key: hash + debug name + format.
        cacheKey = "embedded:" + contentHash + "|" + debugName + (srgb ? "|srgb" : "|linear");

        // Check if this specific key is cached (shouldn't happen, but safe check).
        auto it = m_TextureCache.find(cacheKey);

        if (it != m_TextureCache.end())
        {
            if (auto cachedTexture = it->second.lock())
            {
                updateTextureAccess(cacheKey, cachedTexture->memorySize(), priority);
                
                return cachedTexture;
            }
        }

        // Load texture from memory.
        // Note: This requires a VySampledTexture constructor that accepts memory data
        // For now, we'll need to save to a temp file or extend VySampledTexture class
        // As a workaround, we use the file-based loader with a unique temp path

        // TODO: Implement VySampledTexture::createFromMemory() for true zero-copy loading
        // For now, fall back to file-based loading
        String tempPath = "/tmp/embedded_texture_" + contentHash + ".dat";
        // In production, you'd write data to tempPath here.

        auto   texture = std::make_shared<VySampledTexture>(tempPath, srgb);
        size_t memSize = texture->memorySize();

        // Check memory budget and evict if necessary.
        if (m_MemoryBudget > 0)
        {
            m_CachedTextureMemory += memSize;

            while (m_CachedTextureMemory > m_MemoryBudget && !m_TextureCache.empty())
            {
                evictLRUTextures();
            }
        }

        // Cache the texture
        m_TextureCache[cacheKey]        = texture;
        m_ContentHashToKey[contentHash] = cacheKey;
        updateTextureAccess(cacheKey, memSize, priority);

        // Register with VyTextureManager.
        U32 globalIndex = M_TextureManager->addTexture(texture);

        texture->setGlobalIndex(globalIndex);

        return texture;
    }


    size_t VyResourceManager::garbageCollect()
    {
        size_t removedCount = 0;

        // Clean up textures
        {
            std::lock_guard<std::mutex> lock(m_TextureMutex);

            for (auto it = m_TextureCache.begin(); it != m_TextureCache.end();)
            {
                if (it->second.expired())
                {
                    // Remove from access tracking
                    m_TextureAccessOrder.erase(
                        std::remove_if(m_TextureAccessOrder.begin(), m_TextureAccessOrder.end(), 
                        [&it](const ResourceInfo& info) 
                        { 
                            return info.Key == it->first; 
                        }),
                        m_TextureAccessOrder.end()
                    );

                    it = m_TextureCache.erase(it);
                    ++removedCount;
                }
                else
                {
                    ++it;
                }
            }

            // Recalculate cached memory
            m_CachedTextureMemory = 0;

            for (const auto& [key, weakTexture] : m_TextureCache)
            {
                if (auto texture = weakTexture.lock())
                {
                    m_CachedTextureMemory += texture->memorySize();
                }
            }
        }

        // Clean up models
        {
            std::lock_guard<std::mutex> lock(m_ModelMutex);

            for (auto it = m_ModelCache.begin(); it != m_ModelCache.end();)
            {
                if (it->second.expired())
                {
                    // Remove from access tracking
                    m_ModelAccessOrder.erase(
                        std::remove_if(m_ModelAccessOrder.begin(), m_ModelAccessOrder.end(), 
                        [&it](const ResourceInfo& info) 
                        { 
                            return info.Key == it->first; 
                        }),
                        m_ModelAccessOrder.end()
                    );

                    it = m_ModelCache.erase(it);
                    ++removedCount;
                }
                else
                {
                    ++it;
                }
            }

            // Recalculate cached memory
            m_CachedModelMemory = 0;

            for (const auto& [key, weakModel] : m_ModelCache)
            {
                if (auto model = weakModel.lock())
                {
                    m_CachedModelMemory += model->memorySize();
                }
            }
        }

        return removedCount;
    }


    size_t VyResourceManager::getMemoryUsage() const
    {
        size_t totalMemory = 0;

        // Texture memory (accurate calculation)
        {
            std::lock_guard<std::mutex> lock(m_TextureMutex);

            for (const auto& [key, weakTexture] : m_TextureCache)
            {
                if (auto texture = weakTexture.lock())
                {
                    totalMemory += texture->memorySize();
                }
            }
        }

        // Model memory (accurate calculation)
        {
            std::lock_guard<std::mutex> lock(m_ModelMutex);

            for (const auto& [key, weakModel] : m_ModelCache)
            {
                if (auto model = weakModel.lock())
                {
                    totalMemory += model->memorySize();
                }
            }
        }

        return totalMemory;
    }


    size_t VyResourceManager::getCachedTextureCount() const
    {
        std::lock_guard<std::mutex> lock(m_TextureMutex);

        // Count only alive textures
        size_t count = 0;

        for (const auto& [key, weakTexture] : m_TextureCache)
        {
            if (!weakTexture.expired())
            {
                ++count;
            }
        }

        return count;
    }


    size_t VyResourceManager::getCachedModelCount() const
    {
        std::lock_guard<std::mutex> lock(m_ModelMutex);

        // Count only alive models
        size_t count = 0;

        for (const auto& [key, weakModel] : m_ModelCache)
        {
            if (!weakModel.expired())
            {
                ++count;
            }
        }

        return count;
    }


    void VyResourceManager::clearAll()
    {
        {
            std::lock_guard<std::mutex> lock(m_TextureMutex);

            m_TextureCache.clear();
            m_TextureAccessOrder.clear();
            m_CachedTextureMemory = 0;
        }

        {
            std::lock_guard<std::mutex> lock(m_ModelMutex);

            m_ModelCache.clear();
            m_ModelAccessOrder.clear();
            m_CachedModelMemory = 0;
        }
    }


    bool VyResourceManager::isTextureCached(const String& path) const
    {
        std::lock_guard<std::mutex> lock(m_TextureMutex);

        // Check both srgb and linear variants.
        String srgbKey   = makeTextureKey(path, true);
        String linearKey = makeTextureKey(path, false);

        auto srgbIt   = m_TextureCache.find(srgbKey);
        auto linearIt = m_TextureCache.find(linearKey);

        bool srgbCached   = (srgbIt   != m_TextureCache.end() && !srgbIt->second.expired());
        bool linearCached = (linearIt != m_TextureCache.end() && !linearIt->second.expired());

        return srgbCached || linearCached;
    }


    bool VyResourceManager::isModelCached(const String& path) const
    {
        std::lock_guard<std::mutex> lock(m_ModelMutex);

        // Check if any variant of this model path is cached
        for (const auto& [key, weakModel] : m_ModelCache)
        {
            if (key.find(path) == 0 && !weakModel.expired())
            {
                return true;
            }
        }

        return false;
    }


    void VyResourceManager::setMemoryBudget(size_t budgetBytes)
    {
        m_MemoryBudget = budgetBytes;

        // Evict resources if we're already over budget.
        if (budgetBytes > 0)
        {
            {
                std::lock_guard<std::mutex> lock(m_TextureMutex);

                while (m_CachedTextureMemory > m_MemoryBudget && !m_TextureCache.empty())
                {
                    evictLRUTextures();
                }
            }

            {
                std::lock_guard<std::mutex> lock(m_ModelMutex);

                while (m_CachedModelMemory > m_MemoryBudget && !m_ModelCache.empty())
                {
                    evictLRUModels();
                }
            }
        }
    }


    void VyResourceManager::updateTextureAccess(const String& key, size_t memorySize, ResourcePriority priority)
    {
        // Remove existing entry if present
        m_TextureAccessOrder.erase(
            std::remove_if(m_TextureAccessOrder.begin(), m_TextureAccessOrder.end(), 
            [&key](const ResourceInfo& info) 
            { 
                return info.Key == key; 
            }),
            m_TextureAccessOrder.end()
        );

        // Add to end (most recently used) with priority
        m_TextureAccessOrder.push_back({key, memorySize, getCurrentTime(), priority});
    }


    void VyResourceManager::updateModelAccess(const String& key, size_t memorySize, ResourcePriority priority)
    {
        // Remove existing entry if present
        m_ModelAccessOrder.erase(
            std::remove_if(m_ModelAccessOrder.begin(), m_ModelAccessOrder.end(), 
            [&key](const ResourceInfo& info) 
            { 
                return info.Key == key; 
            }),
            m_ModelAccessOrder.end()
        );

        // Add to end (most recently used) with priority
        m_ModelAccessOrder.push_back({key, memorySize, getCurrentTime(), priority});
    }


    void VyResourceManager::evictLRUTextures()
    {
        if (m_TextureAccessOrder.empty()) return;

        // Sort by priority first (low priority first), then by access time (oldest first)
        std::sort(
            m_TextureAccessOrder.begin(), m_TextureAccessOrder.end(),
            [](const ResourceInfo& a, const ResourceInfo& b) 
            {
                if (a.Priority != b.Priority) 
                {
                    return a.Priority < b.Priority; // Lower priority evicted first
                }

                return a.LastAccessTime < b.LastAccessTime; // Then oldest
            }
        );

        // Skip CRITICAL priority resources
        size_t evictIndex = 0;
        while (evictIndex < m_TextureAccessOrder.size() && 
               m_TextureAccessOrder[evictIndex].Priority == ResourcePriority::CRITICAL)
        {
            ++evictIndex;
        }

        if (evictIndex >= m_TextureAccessOrder.size())
        {
            // All resources are CRITICAL, cannot evict
            return;
        }

        // Evict resource at evictIndex
        const auto& toEvict = m_TextureAccessOrder[evictIndex];
        auto        it      = m_TextureCache.find(toEvict.Key);

        if (it != m_TextureCache.end())
        {
            m_TextureCache.erase(it);
            m_CachedTextureMemory -= toEvict.MemorySize;
        }

        m_TextureAccessOrder.erase(m_TextureAccessOrder.begin() + evictIndex);
    }


    void VyResourceManager::evictLRUModels()
    {
        if (m_ModelAccessOrder.empty()) return;

        // Sort by priority first (low priority first), then by access time (oldest first)
        std::sort(
            m_ModelAccessOrder.begin(), m_ModelAccessOrder.end(), 
            [](const ResourceInfo& a, const ResourceInfo& b) 
            {
                if (a.Priority != b.Priority) 
                {
                    return a.Priority < b.Priority; // Lower priority evicted first
                }
            
                return a.LastAccessTime < b.LastAccessTime; // Then oldest
            }
        );

        // Skip CRITICAL priority resources
        size_t evictIndex = 0;
        while (evictIndex < m_ModelAccessOrder.size() && 
               m_ModelAccessOrder[evictIndex].Priority == ResourcePriority::CRITICAL)
        {
            ++evictIndex;
        }

        if (evictIndex >= m_ModelAccessOrder.size())
        {
            // All resources are CRITICAL, cannot evict
            return;
        }

        // Evict resource at evictIndex
        const auto& toEvict = m_ModelAccessOrder[evictIndex];
        auto        it      = m_ModelCache.find(toEvict.Key);
        if (it != m_ModelCache.end())
        {
            m_ModelCache.erase(it);
            m_CachedModelMemory -= toEvict.MemorySize;
        }

        m_ModelAccessOrder.erase(m_ModelAccessOrder.begin() + evictIndex);
    }


    U64 VyResourceManager::getCurrentTime() const
    {
        return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
    }


    String VyResourceManager::computeContentHash(const unsigned char* data, size_t dataSize) const
    {
        // Use FNV-1a hash for fast content-based deduplication
        U64 hash = hashBytes(data, dataSize);

        // Convert to hex string
        std::ostringstream oss;
        oss << std::hex << std::setfill('0') << std::setw(16) << hash;

        return oss.str();
    }



    // ============================================================================
    // ASYNC LOADING IMPLEMENTATION
    // ============================================================================

    void VyResourceManager::initThreadPool(size_t numThreads)
    {
        m_WorkerThreads.reserve(numThreads);

        for (size_t i = 0; i < numThreads; ++i)
        {
            m_WorkerThreads.emplace_back( &VyResourceManager::workerThreadLoop, this );
        }
    }


    void VyResourceManager::shutdownThreadPool()
    {
        {
            std::lock_guard<std::mutex> lock(m_TaskQueueMutex);

            m_ShutdownThreadPool = true;
        }

        m_TaskQueueCV.notify_all();

        for (auto& thread : m_WorkerThreads)
        {
            if (thread.joinable())
            {
                thread.join();
            }
        }

        m_WorkerThreads.clear();
    }


    void VyResourceManager::workerThreadLoop()
    {
        while (true)
        {
            std::function<void()> task;

            {
                std::unique_lock<std::mutex> lock(m_TaskQueueMutex);

                m_TaskQueueCV.wait(lock, [this] { return m_ShutdownThreadPool || !m_TaskQueue.empty(); });

                if (m_ShutdownThreadPool && m_TaskQueue.empty())
                {
                    return;
                }

                if (!m_TaskQueue.empty())
                {
                    task = std::move(m_TaskQueue.front());
                    m_TaskQueue.pop();
                    
                    m_ActiveTasks++;
                }
            }

            if (task)
            {
                task();

                m_ActiveTasks--;
            }
        }
    }


    std::future<Shared<VySampledTexture>> 
    VyResourceManager::loadTextureAsync(const String& path, bool srgb, ResourcePriority priority)
    {
        // Check if already cached (fast path)
        String key = makeTextureKey(path, srgb);
        {
            std::lock_guard<std::mutex> lock(m_TextureMutex);
            
            auto it = m_TextureCache.find(key);

            if (it != m_TextureCache.end())
            {
                if (auto existingTexture = it->second.lock())
                {
                    // Update access time
                    updateTextureAccess(key, existingTexture->memorySize(), priority);

                    // Return immediately resolved future
                    std::promise<Shared<VySampledTexture>> promise;
                    
                    promise.set_value(existingTexture);

                    return promise.get_future();
                }
            }
        }

        // Create promise/future pair
        auto                                  promise = std::make_shared<std::promise<Shared<VySampledTexture>>>();
        std::future<Shared<VySampledTexture>> future  = promise->get_future();

        // Enqueue async task
        {
            std::lock_guard<std::mutex> lock(m_TaskQueueMutex);

            m_TaskQueue.push([this, path, srgb, priority, promise]() 
            {
                try
                {
                    // Load texture synchronously on worker thread
                    auto texture = loadTexture(path, srgb, false, priority);
                    
                    promise->set_value(texture);
                }
                catch (const std::exception& e)
                {
                    promise->set_exception(std::current_exception());
                }
            });
        }

        m_TaskQueueCV.notify_one();

        return future;
    }


    std::future<Shared<Model>>
    VyResourceManager::loadModelAsync(const String& path, bool enableTextures, bool loadMaterials, bool enableMorphTargets, ResourcePriority priority)
    {
        // Check if already cached (fast path)
        String key = makeModelKey(path, enableTextures, loadMaterials, enableMorphTargets);
        {
            std::lock_guard<std::mutex> lock(m_ModelMutex);

            auto it = m_ModelCache.find(key);
            
            if (it != m_ModelCache.end())
            {
                if (auto existingModel = it->second.lock())
                {
                    // Update access time
                    updateModelAccess(key, existingModel->memorySize(), priority);

                    // Return immediately resolved future
                    std::promise<Shared<Model>> promise;
                    promise.set_value(existingModel);
                    
                    return promise.get_future();
                }
            }
        }

        // Create promise/future pair
        auto                                promise = std::make_shared<std::promise<Shared<Model>>>();
        std::future<Shared<Model>> future = promise->get_future();

        // Enqueue async task
        {
            std::lock_guard<std::mutex> lock(m_TaskQueueMutex);

            m_TaskQueue.push([this, path, enableTextures, loadMaterials, enableMorphTargets, priority, promise]() 
            {
                try
                {
                    // Load model synchronously on worker thread
                    auto model = loadModel(path, enableTextures, loadMaterials, enableMorphTargets, priority);
                    
                    promise->set_value(model);
                }
                catch (const std::exception& e)
                {
                    promise->set_exception(std::current_exception());
                }
            });
        }

        m_TaskQueueCV.notify_one();

        return future;
    }


    size_t VyResourceManager::getPendingAsyncLoads() const
    {
        std::lock_guard<std::mutex> lock(m_TaskQueueMutex);

        return m_TaskQueue.size() + m_ActiveTasks;
    }


    void VyResourceManager::waitForAsyncLoads()
    {
        while (getPendingAsyncLoads() > 0)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
}