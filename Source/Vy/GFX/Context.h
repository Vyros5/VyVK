#pragma once

#include <Vy/GFX/Backend/Device.h>
#include <Vy/GFX/Backend/Descriptors.h>

#include <Vy/Core/Window.h>

namespace Vy
{
    /**
     * @brief Manages the deletion of vulkan objects to ensure they are not deleted while in use.
     * 
     * @note Deletion is deferred for MAX_FRAMES_IN_FLIGHT frames.
     */
	class DeletionQueue
	{
	public:
		DeletionQueue() = default;
		
		DeletionQueue(const DeletionQueue&) = delete;
		DeletionQueue(DeletionQueue&&)      = delete;

		~DeletionQueue();

		DeletionQueue& operator=(const DeletionQueue&) = delete;
		DeletionQueue& operator=(DeletionQueue&&)      = delete;

		void schedule(Function<void()>&& function);

		void flush(U32 frameIndex);
		
        void flushAll();

	private:
		struct DeletorQueue
		{
			TVector<Function<void()>> Deletors;
		};

		TArray<DeletorQueue, MAX_FRAMES_IN_FLIGHT> m_PendingDeletions;
		U32                                        m_CurrentFrameIndex{ 0 };
	};

    constexpr U32 MAX_DESCRIPTOR_SETS  = 1000;

	/**
	 * @class VyContext
	 * 
	 * @brief Manages the Vulkan context, including instance, surface, physical device, and logical device.
	 * 
	 * This class is responsible for creating and managing the Vulkan context, including the instance,
	 * surface, physical device, and logical device. It also provides helper functions for buffer and image operations.
	 */
	class VyContext
	{
	public:
		~VyContext() = default;

		VyContext(const VyContext&) = delete;
		VyContext(VyContext&&)      = delete;

		VyContext& operator=(const VyContext&) = delete;
		VyContext& operator=(VyContext&&)      = delete;

		VY_NODISCARD static VyContext&        get();
		VY_NODISCARD static VyDevice&         device()         { return get().m_Device;                  }
        VY_NODISCARD static VkInstance        instance()       { return get().device().instance();       }
        VY_NODISCARD static VkPhysicalDevice  physicalDevice() { return get().device().physicalDevice(); }
        VY_NODISCARD static VmaAllocator      allocator()      { return get().device().allocator();      }
		VY_NODISCARD static DeletionQueue&    deletionQueue()  { return get().m_DeletionQueue;           }
		// VY_NODISCARD static VyDescriptorPool& descriptorPool() { return get().m_DescriptorPool;          }

		VY_NODISCARD static Shared<VyDescriptorPool> globalPool() { return get().m_GlobalPool; }

        /**
         * @brief Initialize VyContext, VyDevice, and the global descriptor pool.
         * 
         * @note Called on VyRenderer creation. `VyRenderer::VyRenderer(VyWindow& window)`
         */
		static VyContext& initialize(VyWindow& window);

		static void destroy();

		static void destroy(VkBuffer         buffer, VmaAllocation allocation);
		static void destroy(VkImage          image,  VmaAllocation allocation);
		static void destroy(VkImageView      view);
		static void destroy(VkSampler        sampler);
		static void destroy(VkPipeline       pipeline);
		static void destroy(VkPipelineLayout pipelineLayout);

        /**
         * @brief Flush deletion queue.
         * 
         * @note Called from `VyRenderer::endFrame()`.
         * 
         * @param frameIndex 
         */
		static void flushDeletionQueue(U32 frameIndex);

		static void waitIdle();

		static bool allocateSet(VkDescriptorSetLayout layout, VkDescriptorSet& set);

		static void releaseSets(TVector<VkDescriptorSet>& sets);

		static void cleanup();

	private:

		VyContext() = default;

		VyDevice         m_Device{};
		VyDescriptorPool m_DescriptorPool{};
		
		DeletionQueue    m_DeletionQueue{};

		Shared<VyDescriptorPool> m_GlobalPool;
	};


    // class VyDescriptorAllocator 
    // {
    // public:
    //     static void initialize();

    //     static bool allocate(VkDescriptorSetLayout layout, VkDescriptorSet& set);

    //     static Shared<VyDescriptorPool> globalPool();

    //     static void cleanup();
        
    //     static void release(TVector<VkDescriptorSet>& sets);

    // private:
    //     static Shared<VyDescriptorPool> s_GlobalPool; // less memory overhead
    // };
}