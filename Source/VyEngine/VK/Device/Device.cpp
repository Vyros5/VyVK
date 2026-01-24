#include <VyEngine/VK/Device/Device.h>
#include <iostream>
#include <VyLib/Common/AnsiColor.h>

namespace Vy
{
	// Helper class for pNext layers Injection
	// Credit: https://github.com/21thCenturyBoy/Razix/tree/de89fce98defc03ce265c8d02944f555802e9981
	class PNextChain
	{
	public:
		template<typename T>
		void inject(T& feature)
		{
			feature.pNext = m_Head;
			m_Head        = &feature;

			m_Chain.push_back( &feature );
		}

		void* getHead() const
		{
			return m_Head;
		}

		void clear()
		{
			m_Head = nullptr;
			m_Chain.clear();
		}

	private:
		void*          m_Head = nullptr;
		TVector<void*> m_Chain;
	};


    // local callback functions
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT             messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void*                                       pUserData) 
    {
		if ((TString(pCallbackData->pMessageIdName) == "VUID-VkShaderModuleCreateInfo-pCode-08742")) 
		{
			return VK_FALSE;
		}

        std::stringstream ss;

        // Select prefix depending on flags passed to the callback.
        if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT) 
		{
            ss << "\033[95m[GENERAL]\033[0m ";
        }
        if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT) 
		{
            ss << "\033[95m[VALIDATION]\033[0m ";
        }
        if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT) 
		{
            ss << "\033[95m[PERFORMANCE]\033[0m ";
        }

		if (pCallbackData->pMessageIdName) 
		{
			ss << "\n  - ID      : " << pCallbackData->pMessageIdName << " (" << pCallbackData->messageIdNumber << ")" << "\n";
		}
		
		ss << "  - Message : " << pCallbackData->pMessage << "\n";

		if (pCallbackData->queueLabelCount > 0)
		{
			ss << "  - Queue Labels:\n";
			for (U32 i = 0; i < pCallbackData->queueLabelCount; i++)
			{
				ss << "        labelName = <" << pCallbackData->pQueueLabels[ i ].pLabelName << ">\n";
			}
		}
		if (pCallbackData->cmdBufLabelCount > 0)
		{
			ss << "  - CommandBuffer Labels:\n";
			for (U32 i = 0; i < pCallbackData->cmdBufLabelCount; i++)
			{
				ss << "        labelName = <" << pCallbackData->pCmdBufLabels[ i ].pLabelName << ">\n";
			}
		}

		if (pCallbackData->objectCount > 0) 
		{
			ss << "  - Objects involved:" << "\n";

			for (U32 i = 0; i < pCallbackData->objectCount; i++) 
			{
				const auto& obj = pCallbackData->pObjects[ i ];

				ss << "        [" << i << "] " << STR_VK_OBJECT_TYPE(obj.objectType);
				ss << " (0x" << std::hex << obj.objectHandle << std::dec << ")";
				if (obj.pObjectName) 
				{
					ss << " \"" << obj.pObjectName << "\"";
				}
				ss << "\n";
			}
		}

        if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) 
		{
			// VY_DEBUG_TAG("VyDevice", ss.str());
			std::cerr << "[" << CYAN "VERBOSE" RESET "] " << ss.str() << "\n" << std::endl;
			return VK_FALSE;
        } 
		else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) 
		{
			// VY_INFO_TAG("VyDevice", ss.str());
			std::cerr << "[" << BLUE "INFO" RESET "] " << ss.str() << "\n" << std::endl;
			return VK_FALSE;
        } 
		else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) 
		{
			// VY_WARN_TAG("VyDevice", ss.str());
			std::cerr << "[" << YELLOW "WARNING" RESET "] " << ss.str() << "\n" << std::endl;
			return VK_FALSE;
        } 
		else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) 
		{
			// VY_ERROR_TAG("VyDevice", ss.str());
			std::cerr << "[" << RED "ERROR" RESET "] " << ss.str() << "\n" << std::endl;
			return VK_FALSE;
        }
		else
		{
			// VY_ERROR_TAG("VyDevice", ss.str());
			std::cerr << "[" << RED "UNKNOWN" RESET "] " << ss.str() << "\n" << std::endl;
			return VK_FALSE;
        }
		// else
		// {
		// 	VY_TRACE_TAG("VyDevice", ss.str());
		// }
		// if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT)
		// {
		// // General message
		// std::cerr << "[ " << GREEN "GENERAL" RESET;
		// }
		// else if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT)
		// {
		// // Validation message
		// std::cerr << "[ " << YELLOW "VALIDATION" RESET;
		// }
		// else if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT)
		// {
		// // Performance message
		// std::cerr << "[ " << BLUE "PERFORMANCE" RESET;
		// }
		// if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
		// {
		// std::cerr << RED " ERROR" RESET " ] " << pCallbackData->pMessage << std::endl;
		// return VK_FALSE;
		// }
		// else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
		// {
		// std::cerr << YELLOW " WARNING" RESET " ] " << pCallbackData->pMessage << std::endl;
		// return VK_FALSE;
		// }
		// else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
		// {
		// std::cerr << BLUE " INFO" RESET " ] " << pCallbackData->pMessage << std::endl;
		// return VK_FALSE;
		// }
		// else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
		// {
		// std::cerr << CYAN " VERBOSE" RESET " ] " << pCallbackData->pMessage << std::endl;
		// return VK_FALSE;
		// }
		// return VK_FALSE;
        return VK_FALSE;
    }


    VkResult CreateDebugUtilsMessengerEXT(
        VkInstance                                instance,
        const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
        const VkAllocationCallbacks*              pAllocator,
        VkDebugUtilsMessengerEXT*                 pDebugMessenger) 
    {
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
        
        if (func != nullptr) 
        {
            return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
        } 
        else 
        {
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }
    }


    void DestroyDebugUtilsMessengerEXT(
        VkInstance                   instance,
        VkDebugUtilsMessengerEXT     debugMessenger,
        const VkAllocationCallbacks* pAllocator) 
    {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
        
        if (func != nullptr) 
        {
            func(instance, debugMessenger, pAllocator);
        }
    }


    static TVector<CString> getGlfwRequiredExtensions() 
    {
        U32 glfwExtensionCount{};
        CString* glfwExtensions{ glfwGetRequiredInstanceExtensions(&glfwExtensionCount) };

        if (glfwExtensions == nullptr) 
        {
            VY_THROW_RUNTIME_ERROR("Vulkan is not available on this platform!");
        }

        return TVector<CString>{ glfwExtensions, glfwExtensions + glfwExtensionCount };
    }


    static void displayGflwRequiredInstanceExtensions() 
    {
        U32 extensionCount{};
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

        TVector<VkExtensionProperties> extensions(extensionCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

        // Log System available instance extension
        VY_INFO_TAG("VyDevice", "Instance Available extensions:");

        for (const auto& [ extensionName, specVersion ]: extensions) 
        {
            VY_INFO_TAG("VyDevice", "\t{}", extensionName);
        }

        // Log GLFW required extensions
        VY_INFO_TAG("VyDevice", "GLFW required extensions:");

        const auto requiredExtensions{ getGlfwRequiredExtensions() };
        
		for (const auto& required: requiredExtensions) 
        {
            VY_INFO_TAG("VyDevice", "\t- {}", required);
        }
    }

// =========================================================================================================================
#pragma region [ Device ]
// =========================================================================================================================

    void VyDevice::initialize(VyWindow& window)
    {
        createInstance();
        setupDebugMessenger();
        createSurface( window );
        pickPhysicalDevice();
        createLogicalDevice();
        createAllocator();
        createCommandPools();
    }


    VyDevice::~VyDevice() 
    {
		if (m_GraphicsCommandPool)
		{
			vkDestroyCommandPool(m_Device, m_GraphicsCommandPool, nullptr);
			m_GraphicsCommandPool = VK_NULL_HANDLE;
		}

		if (m_Allocator)
		{
			vmaDestroyAllocator(m_Allocator);
			m_Allocator = VK_NULL_HANDLE;
		}
        
		if (m_Device)
		{
			vkDestroyDevice(m_Device, nullptr);
			m_Device = VK_NULL_HANDLE;
		}
		
        if (kEnableValidationLayers && m_DebugMessenger) 
        {
            DestroyDebugUtilsMessengerEXT(m_Instance, m_DebugMessenger, nullptr);
			m_DebugMessenger = VK_NULL_HANDLE;
        }

		if (m_Surface)
		{
			vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
			m_Surface = VK_NULL_HANDLE;
		}
		
		if (m_Instance)
		{
			vkDestroyInstance(m_Instance, nullptr);
			m_Instance = VK_NULL_HANDLE;
		}
    }

#pragma endregion Device


// =========================================================================================================================
#pragma region [ Main Functions ]
// =========================================================================================================================


	// --------------------------------------------------------------------------------------------
    // MARK: Instance
    
    void VyDevice::createInstance() 
    {
        VK_CHECK(volkInitialize());

        if (kEnableValidationLayers && !checkValidationLayerSupport()) 
        {
            VY_THROW_RUNTIME_ERROR("Validation layers requested, but not available!");
        }

		VkApplicationInfo appInfo{ VKInit::applicationInfo() };
		{
			appInfo.pApplicationName   = "VyApp";
			appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);

			appInfo.pEngineName        = "VyEngine";
			appInfo.engineVersion      = VK_MAKE_VERSION(1, 0, 0);

			appInfo.apiVersion         = kAPIVersion;
		}

		// Setup required extensions.
		auto instanceIxtensions{ queryRequiredInstanceExtensions() };

#ifdef VY_DEBUG_MODE
		// displayGflwRequiredInstanceExtensions();
#endif

		VkInstanceCreateInfo createInfo{ VKInit::instanceCreateInfo() };
		{
			createInfo.pNext                   = nullptr;
			createInfo.pApplicationInfo        = &appInfo;
			
			createInfo.enabledExtensionCount   = static_cast<U32>(instanceIxtensions.size());
			createInfo.ppEnabledExtensionNames = instanceIxtensions.data();
			
			createInfo.enabledLayerCount       = 0;
			
			if constexpr (kEnableValidationLayers)
			{
				createInfo.enabledLayerCount   = static_cast<U32>(kValidationLayers.size());
				createInfo.ppEnabledLayerNames = kValidationLayers.data();
				
				VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
                populateDebugMessengerCreateInfo(debugCreateInfo);

				createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
				
				VY_INFO_TAG("VyDevice", "Vulkan Validation Layers enabled");
			}
		}

		VK_CHECK(vkCreateInstance(&createInfo, nullptr, &m_Instance));
		
        volkLoadInstance( m_Instance );

		checkGflwRequiredInstanceExtensions();
    }

	// --------------------------------------------------------------------------------------------
    // MARK: Physical Device
    
    void VyDevice::pickPhysicalDevice() 
    {
        U32 deviceCount{};
        vkEnumeratePhysicalDevices(m_Instance, &deviceCount, nullptr);

		VY_ASSERT(deviceCount > 0, "Failed to find GPUs with Vulkan support!");
		
		VY_INFO_TAG("VyDevice", "Checking for GPU support among {} physical devices...", deviceCount);

        TVector<VkPhysicalDevice> devices( deviceCount );
        vkEnumeratePhysicalDevices(m_Instance, &deviceCount, devices.data());

		// Search for a suitable device.
		for (const auto& device : devices)
		{
			m_QueueFamilyIndices = findQueueFamilies(device);

			if (!m_QueueFamilyIndices.hasValidQueueSupport())
			{
				continue;
			}

			if (!checkDeviceExtensionSupport( device ))
			{
				continue;
			}

			if (!checkDeviceFeatureSupport( device ))
			{
				continue;
			}

			SwapchainSupportDetails swapchainSupport = querySwapchainSupport( device );

			if (swapchainSupport.Formats     .empty() || 
				swapchainSupport.PresentModes.empty())
			{
				continue;
			}

			// Found a suitable device.
			m_PhysicalDevice = device;
			break;
		}

		VY_ASSERT(m_PhysicalDevice != VK_NULL_HANDLE, "Failed to find a suitable GPU");
		
		vkGetPhysicalDeviceProperties(m_PhysicalDevice, &m_Properties);
		U32 major = VK_VERSION_MAJOR(m_Properties.apiVersion);
		U32 minor = VK_VERSION_MINOR(m_Properties.apiVersion);
		U32 patch = VK_VERSION_PATCH(m_Properties.apiVersion);
		
		VY_INFO_TAG("VyDevice", "Selected GPU: {} (Vulkan {}.{}.{})", m_Properties.deviceName, major, minor, patch);
		
		m_MsaaSamples = getMaxUsableSampleCount();

		VY_INFO_TAG("VyDevice", "Max MSAA Samples: {}", STR_VK_SAMPLE_COUNT_FLAG_BITS(m_MsaaSamples));
    }

	// --------------------------------------------------------------------------------------------
    // MARK: Logical Device
    
    void VyDevice::createLogicalDevice() 
    {
        m_QueueFamilyIndices = findQueueFamilies( m_PhysicalDevice );

		VY_ASSERT(m_QueueFamilyIndices.hasValidQueueSupport(), "Queue family indices are not complete");
		
		TSet<U32> uniqueQueueFamilies = { 

            m_QueueFamilyIndices.GraphicsFamily.value(), 
            m_QueueFamilyIndices.PresentFamily .value()
        };

		TVector<VkDeviceQueueCreateInfo> queueCreateInfos;

		float queuePriority = 1.0f;
		for (U32 queueFamily : uniqueQueueFamilies)
		{
			VkDeviceQueueCreateInfo queueCreateInfo{ VKInit::deviceQueueCreateInfo() };
			{
				queueCreateInfo.queueFamilyIndex = queueFamily;
				queueCreateInfo.queueCount       = 1;
				queueCreateInfo.pQueuePriorities = &queuePriority;
			}

			queueCreateInfos.push_back( queueCreateInfo );
		}

		// ----------------------------------------------------------------------------------------
		// Extensions

        TVector<CString> enabledExtensions( kDeviceExtensions.begin(), kDeviceExtensions.end() );

		// ----------------------------------------------------------------------------------------
		// Features

		// pNext Injection utility
		PNextChain pNextFeatureChain;

        VkPhysicalDeviceFeatures deviceFeatures{};
		{
			// Enable anisotropic filtering
			// A texture filtering technique that improves the quality of textures when viewed at oblique angles.
			deviceFeatures.samplerAnisotropy  = VK_TRUE;
			deviceFeatures.shaderInt64        = VK_TRUE;

			// Enable fill mode non solid for wireframe support
			// deviceFeatures.fillModeNonSolid   = VK_TRUE;
			// deviceFeatures.robustBufferAccess = VK_TRUE;
		}

		// Vulkan 1.2 Features ( Bindless Rendering / Descriptor Indexing Features )
		VkPhysicalDeviceVulkan12Features vk12Features{};
		{
			vk12Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;

			vk12Features.descriptorIndexing                            = VK_TRUE;
			
			// This enables the ability to use non-uniform indexing for sampled image arrays within shaders.
			// Non-uniform indexing means that the index used to access an array can be dynamically calculated within 
			// the shader, rather than being a constant. 
			vk12Features.shaderSampledImageArrayNonUniformIndexing     = VK_TRUE;
			vk12Features.shaderStorageImageArrayNonUniformIndexing     = VK_TRUE;
			vk12Features.shaderStorageBufferArrayNonUniformIndexing    = VK_TRUE;
			vk12Features.shaderUniformBufferArrayNonUniformIndexing    = VK_TRUE;

			vk12Features.descriptorBindingUniformBufferUpdateAfterBind = VK_TRUE;
			vk12Features.descriptorBindingStorageBufferUpdateAfterBind = VK_TRUE;
			vk12Features.descriptorBindingSampledImageUpdateAfterBind  = VK_TRUE;
			
			// This allows descriptor sets to have some bindings that are not bound to any resources.
			// This is useful for situations where you don't need to bind all resources in a descriptor set.
			vk12Features.descriptorBindingPartiallyBound               = VK_TRUE;
			vk12Features.descriptorBindingVariableDescriptorCount      = VK_TRUE;


			// This enables runtime-sized descriptor arrays, 
			// which means that the size of descriptor arrays can be determined dynamically at runtime.
			vk12Features.runtimeDescriptorArray                        = VK_TRUE;
			vk12Features.scalarBlockLayout                             = VK_TRUE;
			vk12Features.bufferDeviceAddress                           = VK_TRUE;

			// SPIR-V Capability Int8
			vk12Features.shaderInt8              = VK_TRUE;
			vk12Features.storageBuffer8BitAccess = VK_TRUE;
		}

		pNextFeatureChain.inject( vk12Features );

		// Vulkan 1.3 Features ( Dynamic Rendering )
		VkPhysicalDeviceVulkan13Features vk13Features{};
		{
			vk13Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;

			// vk13Features.dynamicRendering               = VK_TRUE;
			vk13Features.shaderDemoteToHelperInvocation = VK_TRUE;
			vk13Features.maintenance4                   = VK_TRUE;
			// vk13Features.synchronization2               = VK_TRUE;
		}

		pNextFeatureChain.inject( vk13Features );

		// Mesh Shaders
		VkPhysicalDeviceMeshShaderFeaturesEXT meshShaderFeatures{};
		{
			meshShaderFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT;

			meshShaderFeatures.taskShader                             = VK_TRUE;
			meshShaderFeatures.meshShader                             = VK_TRUE;

			meshShaderFeatures.multiviewMeshShader                    = VK_FALSE;
			meshShaderFeatures.primitiveFragmentShadingRateMeshShader = VK_FALSE;
			meshShaderFeatures.meshShaderQueries                      = VK_FALSE;
		}

		// pNextFeatureChain.inject( meshShaderFeatures );


		VkPhysicalDeviceMultiviewFeaturesKHR multiviewFeatures{};
		{
			multiviewFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_FEATURES_KHR;

			multiviewFeatures.multiview = VK_TRUE;
		}

		pNextFeatureChain.inject( multiviewFeatures );
		
		VkPhysicalDeviceFeatures2 deviceFeatures2{};
		{
			deviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;

			deviceFeatures2.features = deviceFeatures;
			deviceFeatures2.pNext    = pNextFeatureChain.getHead();
		}

		vkGetPhysicalDeviceFeatures2(m_PhysicalDevice, &deviceFeatures2);
		
		// ----------------------------------------------------------------------------------------
        // Device

		VkDeviceCreateInfo createInfo{ VKInit::deviceCreateInfo() };
		{
			// This field is a pointer to an extension structure. 
			// It allows to chain additional information, enabling the use of Vulkan extensions. 
			// This is where you would place structures that enable newer Vulkan features.
        	createInfo.pNext                   = &deviceFeatures2;

			// pEnabledFeatures is the older, legacy way of specifying core Vulkan 1.0 features,
			// when using VkPhysicalDeviceFeatures2 set it to nullptr
			createInfo.pEnabledFeatures        = nullptr;
			createInfo.flags                   = 0;
			
            // Queue Creation
			createInfo.queueCreateInfoCount    = static_cast<U32>(queueCreateInfos.size());
			createInfo.pQueueCreateInfos       = queueCreateInfos.data();
			
            // Device Extensions
			createInfo.enabledExtensionCount   = static_cast<U32>(enabledExtensions.size());
			createInfo.ppEnabledExtensionNames = enabledExtensions.data();

            // Layers (Deprecated)
			createInfo.enabledLayerCount       = 0;
			createInfo.ppEnabledLayerNames     = nullptr;
			
			// Not necessary anymore because device specific validation layers have been deprecated.
			if constexpr ( kEnableValidationLayers )
			{
				createInfo.enabledLayerCount   = static_cast<U32>(kValidationLayers.size());
				createInfo.ppEnabledLayerNames = kValidationLayers.data();
			}
		}

		VK_CHECK(vkCreateDevice(m_PhysicalDevice, &createInfo, nullptr, &m_Device));

		volkLoadDevice( m_Device );

		VY_ASSERT(m_QueueFamilyIndices.hasValidQueueSupport(), "Queue family indices are not complete.");

		// Create queues.
		vkGetDeviceQueue(m_Device, m_QueueFamilyIndices.GraphicsFamily.value(), 0, &m_GraphicsQueue);
		vkGetDeviceQueue(m_Device, m_QueueFamilyIndices.PresentFamily .value(), 0, &m_PresentQueue );
    }

	// --------------------------------------------------------------------------------------------
	// MARK: Allocator
    
	void VyDevice::createAllocator()
	{
		VmaAllocatorCreateInfo allocatorInfo{};
		{
			allocatorInfo.physicalDevice   = m_PhysicalDevice;
			allocatorInfo.device           = m_Device;
			allocatorInfo.instance         = m_Instance;

			allocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;

			allocatorInfo.vulkanApiVersion = kAPIVersion;
		}

		VmaVulkanFunctions vulkanFunctions;
		vmaImportVulkanFunctionsFromVolk(&allocatorInfo, &vulkanFunctions);
		
        allocatorInfo.pVulkanFunctions = &vulkanFunctions;

		VK_CHECK(vmaCreateAllocator(&allocatorInfo, &m_Allocator));
	}

	// --------------------------------------------------------------------------------------------
    // MARK: Cmd Pools
    
    void VyDevice::createCommandPools() 
    {
        QueueFamilyIndices queueFamilyIndices = findQueueFamilies();

		// Individual command pools for each queue family we may want to submit to.
		
        // Graphics Command Pool
		{
			VY_ASSERT(queueFamilyIndices.GraphicsFamily.has_value(), "Graphics queue family not found.");
			
			VkCommandPoolCreateInfo poolInfo{ VKInit::commandPoolCreateInfo() };
			{
				poolInfo.queueFamilyIndex = queueFamilyIndices.GraphicsFamily.value();
				poolInfo.flags            = { VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT };
			}
			
			VK_CHECK_SUCCESS(vkCreateCommandPool(m_Device, &poolInfo, nullptr, &m_GraphicsCommandPool),
				"Failed to create graphics command pool!");
		}
	}

	// --------------------------------------------------------------------------------------------
    // MARK: Surface
    
    void VyDevice::createSurface(VyWindow& window) 
    { 
        window.createWindowSurface(m_Instance, &m_Surface); 
    }

#pragma endregion Main Functions


// =========================================================================================================================
#pragma region [ Resource ]
// =========================================================================================================================

	void VyDevice::createBuffer(
		VkBuffer&                buffer, 
		VmaAllocation&           allocation, 
		VkDeviceSize             size,
		VkBufferUsageFlags       bufferUsage, 
		VmaAllocationCreateFlags allocFlags, 
		VmaMemoryUsage           memoryUsage) const
	{
		VkBufferCreateInfo bufferInfo{ VKInit::bufferCreateInfo() };
		{
			bufferInfo.size        = size;
			bufferInfo.usage       = bufferUsage;
			bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		}

		VmaAllocationCreateInfo allocInfo{};
		{
			allocInfo.usage = memoryUsage;
			allocInfo.flags = allocFlags;
		}

		VK_CHECK(vmaCreateBuffer(
			m_Allocator, 
			&bufferInfo, 
			&allocInfo, 
			&buffer, 
			&allocation, 
			nullptr
		));
	}


	void VyDevice::createImage(
		VkImage&                       image, 
		VmaAllocation&                 allocation, 
		const VkImageCreateInfo&       imageInfo,
		const VmaAllocationCreateInfo& allocInfo) const
	{
		VK_CHECK(vmaCreateImage(
			m_Allocator, 
			&imageInfo, 
			&allocInfo, 
			&image, 
			&allocation, 
			nullptr
		));
	}

#pragma endregion Resource


// =========================================================================================================================
#pragma region [ Queue Families & Swapchain ]
// =========================================================================================================================

	QueueFamilyIndices VyDevice::findQueueFamilies(VkPhysicalDevice device) const
	{
		QueueFamilyIndices indices;

		// Query physical device queue family properties.
		U32 queueFamilyCount{};
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

		TVector<VkQueueFamilyProperties> queueFamilies( queueFamilyCount );
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		U32 i = 0;
		for (const auto& queueFamily : queueFamilies)
		{
			if (queueFamily.queueCount > 0 && (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT))
			{
				indices.GraphicsFamily = i;
			}

			VkBool32 presentSupport = VK_FALSE;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_Surface, &presentSupport);
			if (queueFamily.queueCount > 0 && presentSupport == VK_TRUE)
			{
				indices.PresentFamily = i;
			}

			if (indices.hasValidQueueSupport())
			{
				break;
			}

			++i;
		}

        // Most graphics tasks on modern hardware make use of 3 queue types:
        //  - Combined Graphics & Compute queues
        //  - Dedicated Compute Queues
        //  - Dedicated Transfer Queues

        //  Some (particularly old / embedded) hardware does not have a dedicated compute queue.
        //  In such cases, we have to force all the the work linearly through a combined graphics/compute queue.

        // Additionally having prioritised graphics queues is beneficial for frame throughput
        //  (e.g. when we want to quickly finish off a frame for presenting when also starting to rasterize another).
        //  Thus having *at least two* graphics queues available is useful for this separation.

        // First pass: Try and find all desired queue types exactly.
        // for (U32 i = 0; i < static_cast<U32>(queueFamilies.size()); i++) 
		// {
        //     if (indices.hasValidQueueSupport()) break; // short circuit if required queues are found

        //     VkQueueFamilyProperties& currentProperties = queueFamilies[ i ];

        //     if (currentProperties.queueCount <= 0) continue;

        //     // Present Family
        //     if (!indices.PresentFamily.has_value()) 
		// 	{
        //         VkBool32 bPresentSupport = false;

        //         vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_Surface, &bPresentSupport);
                
		// 		if (bPresentSupport) 
		// 		{
        //             indices.PresentFamily = i;
        //         }
        //     }

        //     // Graphics Family
        //     if (!indices.GraphicsFamily.has_value() &&
        //         currentProperties.queueFlags & VK_QUEUE_GRAPHICS_BIT) 
		// 	{
        //         indices.GraphicsFamily            = i;
        //         indices.HasMultipleGraphicsQueues = (currentProperties.queueCount > 1);
        //     }

        //     // Dedicated Compute Family
        //     if (!indices.ComputeFamily.has_value() &&
        //           currentProperties.queueFlags & VK_QUEUE_COMPUTE_BIT && 
        //         !(currentProperties.queueFlags & VK_QUEUE_GRAPHICS_BIT)) 
		// 	{
        //         indices.ComputeFamily             = i;
        //         indices.HasDedicatedComputeFamily = true;
        //     }

        //     // Dedicated Transfer Family
        //     if (!indices.TransferFamily.has_value() &&
        //           currentProperties.queueFlags & VK_QUEUE_TRANSFER_BIT  &&
        //         !(currentProperties.queueFlags & VK_QUEUE_GRAPHICS_BIT) &&
        //         !(currentProperties.queueFlags & VK_QUEUE_COMPUTE_BIT)) 
		// 	{
        //         indices.TransferFamily             = i;
        //         indices.HasDedicatedTransferFamily = true;
        //     }
        // }

        // // Deal with cases where dedicated queues are not present.
        // if (!indices.ComputeFamily.has_value()) 
		// {
        //     // Find any applicable compute queue to use instead.
        //     indices.ComputeFamily = findFirstValidQueueFamily(device, VK_QUEUE_COMPUTE_BIT);
        // }

        // if (!indices.TransferFamily.has_value()) 
		// {
        //     indices.TransferFamily = findFirstValidQueueFamily(device,VK_QUEUE_TRANSFER_BIT);
        // }

        return indices;
	}


	QueueFamilyIndices VyDevice::findQueueFamilies() const
	{
		return findQueueFamilies( m_PhysicalDevice );
	}


	U32 VyDevice::findFirstValidQueueFamily(VkPhysicalDevice device, VkQueueFlags flags) const
	{
		U32 validFamilyIndex{};

        U32 queueFamilyCount{};
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        TVector<VkQueueFamilyProperties> queueFamilies( queueFamilyCount );
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        for (U32 i = 0; i < static_cast<U32>(queueFamilies.size()); i++) 
		{
            VkQueueFamilyProperties& currentProperties = queueFamilies[i];

            if (currentProperties.queueCount <= 0) continue;

            if (currentProperties.queueFlags & flags) 
			{
                validFamilyIndex = i;
            }
        }

        return validFamilyIndex;
	}


	SwapchainSupportDetails VyDevice::querySwapchainSupport(VkPhysicalDevice device) const
	{
		SwapchainSupportDetails details;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_Surface, &details.Capabilities);

		U32 formatCount{};
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_Surface, &formatCount, nullptr);

		if (formatCount != 0)
		{
			details.Formats.resize( formatCount );

			vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_Surface, &formatCount, details.Formats.data());
		}

		U32 presentModeCount{};
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_Surface, &presentModeCount, nullptr);

		if (presentModeCount != 0)
		{
			details.PresentModes.resize( presentModeCount );

			vkGetPhysicalDeviceSurfacePresentModesKHR(
				device,
				m_Surface,
				&presentModeCount,
				details.PresentModes.data()
			);
		}

		return details;
	}


	SwapchainSupportDetails VyDevice::querySwapchainSupport() const
	{
		return querySwapchainSupport( m_PhysicalDevice );
	}

#pragma endregion Queue Families & Swapchain


// =========================================================================================================================
#pragma region [ Debug ]
// =========================================================================================================================

    void VyDevice::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) 
    {
        createInfo = { VKInit::debugUtilsMessengerCreateInfoEXT() };
        {
            createInfo.messageSeverity = {
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | 
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT   |
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
            };
            createInfo.messageType     = { 
                VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT  |
                VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT 
			// | VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT     
            };
            createInfo.pfnUserCallback = debugCallback;
            createInfo.pUserData       = nullptr;
        }
    }


    void VyDevice::setupDebugMessenger() 
    {
        if (!kEnableValidationLayers) return;

        VkDebugUtilsMessengerCreateInfoEXT createInfo;
        populateDebugMessengerCreateInfo(createInfo);
        
        VK_CHECK(CreateDebugUtilsMessengerEXT(m_Instance, &createInfo, nullptr, &m_DebugMessenger));
    }

#pragma endregion Debug


// =========================================================================================================================
#pragma region [ Extra ]
// =========================================================================================================================

	VkImageAspectFlags VyDevice::findAspectFlags(VkFormat format) const
	{
		switch (format)
		{
			// Depth
			case VK_FORMAT_D16_UNORM:
			case VK_FORMAT_D32_SFLOAT:
			case VK_FORMAT_X8_D24_UNORM_PACK32:
				return VK_IMAGE_ASPECT_DEPTH_BIT;

			// Stencil
			case VK_FORMAT_S8_UINT:
				return VK_IMAGE_ASPECT_STENCIL_BIT;

			// Depth | Stencil
			case VK_FORMAT_D16_UNORM_S8_UINT:
			case VK_FORMAT_D24_UNORM_S8_UINT:
			case VK_FORMAT_D32_SFLOAT_S8_UINT:
				return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;

			// Color
			default:
				return VK_IMAGE_ASPECT_COLOR_BIT;
		}
	}


    U32 VyDevice::findMemoryType(U32 typeFilter, VkMemoryPropertyFlags properties) 
    {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(m_PhysicalDevice, &memProperties);

        for (U32 i = 0; i < memProperties.memoryTypeCount; i++) 
        {
            if ((typeFilter & (1 << i)) && 
                (memProperties.memoryTypes[i].propertyFlags & properties) == properties) 
            {
                return i;
            }
        }

        VY_THROW_RUNTIME_ERROR("Failed to find suitable memory type!");
    }


	VkFormat VyDevice::findSupportedFormat(
		const TVector<VkFormat>& candidates, 
		VkImageTiling            tiling,
		VkFormatFeatureFlags     features) const
	{
		for (VkFormat format : candidates)
		{
			VkFormatProperties props;
			vkGetPhysicalDeviceFormatProperties(m_PhysicalDevice, format, &props);

			if      (tiling == VK_IMAGE_TILING_LINEAR  && (props.linearTilingFeatures  & features) == features)
			{
				return format;
			}
			else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
			{
				return format;
			}
		}

		VY_ASSERT(false, "Failed to find supported format!");
		return VK_FORMAT_UNDEFINED;
	}


	VkFormat VyDevice::findDepthFormat() const
	{
        return findSupportedFormat(
            {
                VK_FORMAT_D32_SFLOAT, 
                VK_FORMAT_D32_SFLOAT_S8_UINT, 
                VK_FORMAT_D24_UNORM_S8_UINT
            },
            VK_IMAGE_TILING_OPTIMAL, 
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
        );
	}


	bool VyDevice::getSupportedDepthsFormat(VkFormat* pDepthFormat)
	{
        //https://github.com/SaschaWillems/Vulkan/blob/master/base/VulkanTools.cpp#L95
        TVector<VkFormat> formatList = {
			VK_FORMAT_D32_SFLOAT_S8_UINT,
			VK_FORMAT_D32_SFLOAT,
			VK_FORMAT_D24_UNORM_S8_UINT,
			VK_FORMAT_D16_UNORM_S8_UINT,
			VK_FORMAT_D16_UNORM
        };

        for (auto& format : formatList)
        {
            VkFormatProperties formatProps;
            vkGetPhysicalDeviceFormatProperties(m_PhysicalDevice, format, &formatProps);

            if (formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
            {
                *pDepthFormat = format;
                return true;
            }
        }

        return false;
    }


	VkSampleCountFlagBits VyDevice::getMaxUsableSampleCount() 
	{
		VkSampleCountFlags countsFlags = 
			m_Properties.limits.framebufferColorSampleCounts & 
			m_Properties.limits.framebufferDepthSampleCounts;

		if (countsFlags & VK_SAMPLE_COUNT_64_BIT) return VK_SAMPLE_COUNT_64_BIT;
		if (countsFlags & VK_SAMPLE_COUNT_32_BIT) return VK_SAMPLE_COUNT_32_BIT; 
		if (countsFlags & VK_SAMPLE_COUNT_16_BIT) return VK_SAMPLE_COUNT_16_BIT;
		if (countsFlags & VK_SAMPLE_COUNT_8_BIT)  return VK_SAMPLE_COUNT_8_BIT;
		if (countsFlags & VK_SAMPLE_COUNT_4_BIT)  return VK_SAMPLE_COUNT_4_BIT;
		if (countsFlags & VK_SAMPLE_COUNT_2_BIT)  return VK_SAMPLE_COUNT_2_BIT;

		return VK_SAMPLE_COUNT_1_BIT;
	}

#pragma endregion Extra


// =========================================================================================================================
#pragma region [ Extensions & Features ]
// =========================================================================================================================

	TVector<VkExtensionProperties> VyDevice::queryDeviceExtensionProperties(VkPhysicalDevice device)
	{
		U32 extensionCount{};
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

		TVector<VkExtensionProperties> availableExtensions( extensionCount );
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

		return availableExtensions;
	}


	TVector<VkExtensionProperties> VyDevice::queryInstanceExtensionProperties()
	{
		U32 extensionCount{};
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

		TVector<VkExtensionProperties> availableExtensions( extensionCount );
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, availableExtensions.data());

		return availableExtensions;
	}


	TVector<CString> VyDevice::queryRequiredInstanceExtensions() const
	{
		U32 glfwExtensionCount{};
		CString* glfwExtensions{ glfwGetRequiredInstanceExtensions(&glfwExtensionCount) };

        if (glfwExtensions == nullptr) 
        {
            VY_THROW_RUNTIME_ERROR("Vulkan is not available on this platform!");
        }

		TVector<CString> extensions;
		extensions.reserve( glfwExtensionCount );
		extensions.insert(extensions.end(), glfwExtensions, glfwExtensions + glfwExtensionCount);

		if constexpr (kEnableValidationLayers)
		{
			extensions.emplace_back( VK_EXT_DEBUG_UTILS_EXTENSION_NAME );
		}

		return extensions;
	}


    void VyDevice::checkGflwRequiredInstanceExtensions() 
    {
		auto extensions = queryInstanceExtensionProperties();

		THashSet<TString> available;
		for (const auto& extension : extensions)
		{
			available.insert( extension.extensionName );
		}

		for (const auto& required : queryRequiredInstanceExtensions())
		{
			VY_ASSERT(available.find( required ) != available.end(), "Missing required glfw extension");
		}
    }


    bool VyDevice::checkDeviceExtensionSupport(VkPhysicalDevice device) 
    {
		auto availableExtensions = queryDeviceExtensionProperties(device);

		TSet<TString> requiredExtensions( kDeviceExtensions.begin(), kDeviceExtensions.end() );

		for (const auto& extension : availableExtensions)
		{
			requiredExtensions.erase( extension.extensionName );
		}

		return requiredExtensions.empty();
    }


	bool VyDevice::checkDeviceFeatureSupport(VkPhysicalDevice device)
	{
		VkPhysicalDeviceFeatures supportedFeatures;
		vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

		VkPhysicalDeviceVulkan12Features vk12Features{};
		{
			vk12Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
		}

		VkPhysicalDeviceFeatures2 features2{};
		{
			features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
			features2.pNext = &vk12Features;
		}

		vkGetPhysicalDeviceFeatures2(device, &features2);

		// Sampler Anistropy Support
		if (!supportedFeatures.samplerAnisotropy)
		{
			return false;
		}

		// Shader Int64 Support
		if (!supportedFeatures.shaderInt64)
		{
			return false;
		}
		
		// Bindless Support
		// if (!vk12Features.descriptorIndexing                        && 
		// 	!vk12Features.shaderSampledImageArrayNonUniformIndexing &&
		// 	!vk12Features.descriptorBindingPartiallyBound           && 
		// 	!vk12Features.descriptorBindingVariableDescriptorCount  &&
		// 	!vk12Features.runtimeDescriptorArray                    && 
		// 	!vk12Features.bufferDeviceAddress)
		// {
		// 	return false;
		// }

		// TODO: check for more features (dynamic rendering, mesh shaders, etc.)

		return true;
	}


    bool VyDevice::checkValidationLayerSupport() 
    {
		U32 layerCount{};
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		TVector<VkLayerProperties> availableLayers( layerCount );
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		for (auto layerName : kValidationLayers)
		{
			bool layerFound = false;

			for (const auto& layerProperties : availableLayers)
			{
				if (strcmp(layerName, layerProperties.layerName) == 0)
				{
					layerFound = true;

					break;
				}
			}

			if (!layerFound)
			{
				return false;
			}
		}

		return true;
    }


#pragma endregion Extensions & Features


// =========================================================================================================================
#pragma region [ Cmd Buffer ]
// =========================================================================================================================

	VkCommandBuffer VyDevice::beginSingleTimeCommands() const
	{
		VkCommandBufferAllocateInfo allocInfo{ VKInit::commandBufferAllocateInfo() };
        {
            allocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            allocInfo.commandPool        = m_GraphicsCommandPool;
            allocInfo.commandBufferCount = 1;
        }

		VkCommandBuffer cmdBuffer{ VK_NULL_HANDLE };
		vkAllocateCommandBuffers(m_Device, &allocInfo, &cmdBuffer);

		VkCommandBufferBeginInfo beginInfo{ VKInit::commandBufferBeginInfo() };
		{
			beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		}

		vkBeginCommandBuffer(cmdBuffer, &beginInfo);

		VKDbg::setObjectName(VK_OBJECT_TYPE_COMMAND_BUFFER, (U64)cmdBuffer, "single_time_cmd_buffer");
		
		return cmdBuffer;
	}


	void VyDevice::endSingleTimeCommands(VkCommandBuffer cmdBuffer) const
	{
		VK_CHECK(vkEndCommandBuffer(cmdBuffer));

		VkSubmitInfo submitInfo{ VKInit::submitInfo() };
		{
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers    = &cmdBuffer;
		}

		vkQueueSubmit(m_GraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);

		vkQueueWaitIdle(m_GraphicsQueue);

		vkFreeCommandBuffers(m_Device, m_GraphicsCommandPool, 1, &cmdBuffer);
	}


	VkCommandBuffer VyDevice::createCommandBuffer(VkCommandBufferLevel level, bool bBegin)
	{
		return createCommandBuffer( level, m_GraphicsCommandPool, bBegin );
	}


	VkCommandBuffer VyDevice::createCommandBuffer(VkCommandBufferLevel level, VkCommandPool pool, bool bBegin)
	{
		VkCommandBufferAllocateInfo allocInfo{ VKInit::commandBufferAllocateInfo() };
        {
            allocInfo.level              = level;
            allocInfo.commandPool        = pool;
            allocInfo.commandBufferCount = 1;
        }

		VkCommandBuffer cmdBuffer{ VK_NULL_HANDLE };
		vkAllocateCommandBuffers(m_Device, &allocInfo, &cmdBuffer);

		// If requested, also start recording for the new command buffer
		if (bBegin)
		{
			VkCommandBufferBeginInfo beginInfo{ VKInit::commandBufferBeginInfo() };
			
			vkBeginCommandBuffer(cmdBuffer, &beginInfo);
		}
		
		return cmdBuffer;
	}


	/// Ends, submits and frees a one‑time command buffer
	void VyDevice::flushCommandBuffer(VkCommandBuffer cmdBuffer, VkQueue queue, bool bFree)
	{
		return flushCommandBuffer(cmdBuffer, m_GraphicsQueue, m_GraphicsCommandPool, bFree);
	}
	

	void VyDevice::flushCommandBuffer(VkCommandBuffer cmdBuffer, VkQueue queue, VkCommandPool pool, bool bFree)
	{
		if (cmdBuffer == VK_NULL_HANDLE)
		{
			return;
		}

		VK_CHECK(vkEndCommandBuffer(cmdBuffer));

		VkSubmitInfo submitInfo{ VKInit::submitInfo() };
		{
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers    = &cmdBuffer;
		}

		// Create fence to ensure that the command buffer has finished executing
		VkFenceCreateInfo fenceInfo{ VKInit::fenceCreateInfo( VK_FLAGS_NONE ) };

		VkFence fence;
		VK_CHECK(vkCreateFence(m_Device, &fenceInfo, nullptr, &fence));
		
		// Submit to the queue
		VK_CHECK(vkQueueSubmit(queue, 1, &submitInfo, fence));

		// Wait for the fence to signal that command buffer has finished executing
		VK_CHECK(vkWaitForFences(m_Device, 1, &fence, VK_TRUE, DEFAULT_FENCE_TIMEOUT));
		vkDestroyFence(m_Device, fence, nullptr);

		if (bFree)
		{
			vkFreeCommandBuffers(m_Device, pool, 1, &cmdBuffer);
		}
	}


	// void VyDevice::immediateSubmit(TFunction<void(VkCommandBuffer cmd)>&& func) const
	// {
	// 	VkCommandBufferAllocateInfo allocInfo{ VKInit::commandBufferAllocateInfo() };
	// 	{
	// 		allocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	// 		allocInfo.commandPool        = m_ImmCommandPool;
	// 		allocInfo.commandBufferCount = 1;
	// 	}

	// 	VkCommandBuffer cmd;
	// 	if (vkAllocateCommandBuffers(m_Device, &allocInfo, &cmd) != VK_SUCCESS) 
	// 	{
	// 		VY_ERROR("Failed to allocate immediate command buffer!");
	// 		return;
	// 	}

	// 	VkCommandBufferBeginInfo beginInfo{ VKInit::commandBufferBeginInfo() };
	// 	{
	// 		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	// 	}

	// 	vkBeginCommandBuffer(cmd, &beginInfo);
	// 	{
	// 		func( cmd );
	// 	}
	// 	vkEndCommandBuffer(cmd);

	// 	VkFenceCreateInfo fenceInfo{ VKInit::fenceCreateInfo() };
		
	// 	VkFence fence;
	// 	vkCreateFence(m_Device, &fenceInfo, nullptr, &fence);

	// 	VkSubmitInfo submitInfo{ VKInit::submitInfo() };
	// 	{
	// 		submitInfo.commandBufferCount = 1;
	// 		submitInfo.pCommandBuffers    = &cmd;
	// 	}

	// 	vkQueueSubmit(m_GraphicsQueue, 1, &submitInfo, fence);

	// 	// Wait for the fence (more robust than QueueWaitIdle)
	// 	vkWaitForFences(m_Device, 1, &fence, VK_TRUE, UINT64_MAX);

	// 	vkDestroyFence(m_Device, fence, nullptr);
	// 	vkFreeCommandBuffers(m_Device, m_ImmCommandPool, 1, &cmd);
	// 	}

#pragma endregion Cmd Buffer 
}