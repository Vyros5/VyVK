#include <Vy/GFX/Backend/Device.h>

#include <Vy/GFX/Backend/VK/vk_enum_str.h>

#include <Vy/GFX/Context.h>
#include <Vy/Globals.h>

#include <iostream>

namespace Vy
{
    // local callback functions
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT             messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void*                                       pUserData) 
    {
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

        ss << pCallbackData->pMessage << std::endl;

        if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) 
		{
			VY_DEBUG_TAG("VyDevice", ss.str());
        } 
		else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) 
		{
			VY_INFO_TAG("VyDevice", ss.str());
        } 
		else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) 
		{
			VY_WARN_TAG("VyDevice", ss.str());
        } 
		else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) 
		{
			VY_ERROR_TAG("VyDevice", ss.str());
        }
		else
		{
			VY_TRACE_TAG("VyDevice", ss.str());
		}

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

    // ============================================================================================

    void VyDevice::initialize(VyWindow& window)
    {
        createInstance();
        setupDebugMessenger();
        createSurface(window);
        pickPhysicalDevice();
        createLogicalDevice();
        createAllocator();
        createCommandPool();
    }


    VyDevice::~VyDevice() 
    {
        vkDestroyCommandPool(m_Device, m_CommandPool, nullptr);

		vmaDestroyAllocator(m_Allocator);
        
		vkDestroyDevice(m_Device, nullptr);

        if (kEnableValidationLayers) 
        {
            DestroyDebugUtilsMessengerEXT(m_Instance, m_DebugMessenger, nullptr);
        }

        vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
		
        vkDestroyInstance(m_Instance, nullptr);
    }


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
			appInfo.pEngineName        = "VyEngine";               
			appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0); 
			appInfo.engineVersion      = VK_MAKE_VERSION(1, 0, 0); 
			appInfo.apiVersion         = kAPIVersion;
		}

		// Setup required extensions.
		auto instanceIxtensions{ queryRequiredInstanceExtensions() };

#ifdef VY_DEBUG_MODE
		displayGflwRequiredInstanceExtensions();
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
		
        volkLoadInstance(m_Instance);

		checkGflwRequiredInstanceExtensions();
    }

	// --------------------------------------------------------------------------------------------
    // MARK: Physical Device
    
    void VyDevice::pickPhysicalDevice() 
    {
        U32 deviceCount{};
        vkEnumeratePhysicalDevices(m_Instance, &deviceCount, nullptr);

		VY_ASSERT(deviceCount > 0, "Failed to find GPUs with Vulkan support!");
		
		VY_INFO_TAG("VyDevice", "Available GPUs: {}", deviceCount);

        TVector<VkPhysicalDevice> devices( deviceCount );
        vkEnumeratePhysicalDevices(m_Instance, &deviceCount, devices.data());

		// Search for a suitable device.
		for (const auto& device : devices)
		{
			QueueFamilyIndices indices = findQueueFamilies(device);

			if (!indices.isComplete())
			{
				continue;
			}

			if (!checkDeviceExtensionSupport(device))
			{
				continue;
			}

			if (!checkDeviceFeatureSupport(device))
			{
				continue;
			}

			SwapchainSupportDetails swapchainSupport = querySwapchainSupport(device);

			if (swapchainSupport.Formats.empty()      || 
				swapchainSupport.PresentModes.empty())
			{
				continue;
			}

			// Found a suitable device.
			m_PhysicalDevice = device;
			break;
		}

		VY_ASSERT(m_PhysicalDevice != VK_NULL_HANDLE, "Failed to find a suitable GPU");
		
		m_MsaaSamples = getMaxUsableSampleCount();

		vkGetPhysicalDeviceProperties(m_PhysicalDevice, &m_Properties);
		U32 major = VK_VERSION_MAJOR(m_Properties.apiVersion);
		U32 minor = VK_VERSION_MINOR(m_Properties.apiVersion);
		U32 patch = VK_VERSION_PATCH(m_Properties.apiVersion);
		
		VY_INFO_TAG("VyDevice", "Selected GPU: {} (Vulkan {}.{}.{})", m_Properties.deviceName, major, minor, patch);

		// VY_INFO_TAG("VyDevice", "maxDescriptorSetUniformBuffers: {}", m_Properties.limits.maxDescriptorSetUniformBuffers);
		VY_INFO_TAG("VyDevice", "Max Samples: {}", STR_VK_SAMPLE_COUNT_FLAG_BITS(m_MsaaSamples));
    }

	// --------------------------------------------------------------------------------------------
    // MARK: Logical Device
    
    void VyDevice::createLogicalDevice() 
    {
        QueueFamilyIndices indices = findQueueFamilies(m_PhysicalDevice);

		VY_ASSERT(indices.isComplete(), "Queue family indices are not complete");

        TVector<VkDeviceQueueCreateInfo> queueCreateInfos;
		
		TSet<U32> uniqueQueueFamilies = { 

            indices.GraphicsFamily.value(), 
            indices.PresentFamily .value()
        };

		float queuePriority = 1.0f;

		for (U32 queueFamily : uniqueQueueFamilies)
		{
			VkDeviceQueueCreateInfo queueCreateInfo{ VKInit::deviceQueueCreateInfo() };
			{
				queueCreateInfo.queueFamilyIndex = queueFamily;
				queueCreateInfo.queueCount       = 1;
				queueCreateInfo.pQueuePriorities = &queuePriority;
			}

			queueCreateInfos.push_back(queueCreateInfo);
		}

        TVector<CString> enabledExtensions( kDeviceExtensions.begin(), kDeviceExtensions.end() );

		enabledExtensions.push_back( VK_EXT_MESH_SHADER_EXTENSION_NAME );

		auto availableExtensions = queryDeviceExtensionProperties(m_PhysicalDevice);

    	const bool presentIdExtensionAvailable = std::any_of(
			availableExtensions.begin(), availableExtensions.end(), 
			[](const VkExtensionProperties& extension) 
			{
				return std::strcmp(extension.extensionName, VK_KHR_PRESENT_ID_EXTENSION_NAME) == 0;
			}
		);

		// Features

		// Vulkan 1.2 Features ( Bindless Rendering / Descriptor Indexing Features )
		VkPhysicalDeviceVulkan12Features vk12Features{};
		{
			vk12Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;

			vk12Features.descriptorIndexing                         = VK_TRUE;
			// This enables the ability to use non-uniform indexing for sampled image arrays within shaders.
			// Non-uniform indexing means that the index used to access an array can be dynamically calculated within 
			// the shader, rather than being a constant. 
			vk12Features.shaderSampledImageArrayNonUniformIndexing  = VK_TRUE;
			vk12Features.shaderStorageImageArrayNonUniformIndexing  = VK_TRUE;
			vk12Features.shaderStorageBufferArrayNonUniformIndexing = VK_TRUE;
			// This allows descriptor sets to have some bindings that are not bound to any resources.
			// This is useful for situations where you don't need to bind all resources in a descriptor set.
			vk12Features.descriptorBindingPartiallyBound            = VK_TRUE;
			vk12Features.descriptorBindingVariableDescriptorCount   = VK_TRUE;
			// This enables runtime-sized descriptor arrays, 
			// which means that the size of descriptor arrays can be determined dynamically at runtime.
			vk12Features.runtimeDescriptorArray                     = VK_TRUE;
			vk12Features.scalarBlockLayout                          = VK_TRUE;
			vk12Features.bufferDeviceAddress                        = VK_TRUE;
		}

		// Vulkan 1.3 Features ( Dynamic Rendering )
		VkPhysicalDeviceVulkan13Features vk13Features{};
		{
			vk13Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;

			vk13Features.dynamicRendering               = VK_TRUE;
			vk13Features.shaderDemoteToHelperInvocation = VK_TRUE;
		}

        VkPhysicalDeviceBufferDeviceAddressFeaturesKHR bdaFeatures{};
		{
			bdaFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES_KHR;

			bdaFeatures.bufferDeviceAddress = VK_TRUE;
		} 

		// Maintenance
		VkPhysicalDeviceMaintenance4Features maintenance4Features{};
		{
			maintenance4Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_4_FEATURES;

			maintenance4Features.maintenance4 = VK_TRUE;
		}

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

		VkPhysicalDevicePresentIdFeaturesKHR presentIdFeaturesQuery{}; 
		{
			presentIdFeaturesQuery.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENT_ID_FEATURES_KHR;

			presentIdFeaturesQuery.pNext = &meshShaderFeatures;
		}
		
		m_PresentIdSupported = false;

		if (presentIdExtensionAvailable)
		{
			VkPhysicalDeviceFeatures2 features2{}; 
			{
				features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
				
				features2.pNext = &presentIdFeaturesQuery; // Chain to presentIdFeaturesQuery
			}

			vkGetPhysicalDeviceFeatures2(m_PhysicalDevice, &features2);
			
			if (presentIdFeaturesQuery.presentId == VK_TRUE)
			{
				m_PresentIdSupported = true;
			
				enabledExtensions.push_back( VK_KHR_PRESENT_ID_EXTENSION_NAME );
			}
		}

		// Reset unsupported/unwanted mesh shader features that might have been enabled by the query
		meshShaderFeatures.multiviewMeshShader                    = VK_FALSE;
		meshShaderFeatures.primitiveFragmentShadingRateMeshShader = VK_FALSE;

		VkPhysicalDevicePresentIdFeaturesKHR presentIdFeaturesEnable{}; 
		{
			presentIdFeaturesEnable.sType     = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENT_ID_FEATURES_KHR;
			presentIdFeaturesEnable.pNext     = &meshShaderFeatures; // Chain to meshShaderFeatures

			presentIdFeaturesEnable.presentId = VK_TRUE;
		}

		// Set up pNext chain: presentId (if supported) -> meshShaderFeatures -> vk12Features

		vk12Features.pNext         = nullptr;               // Chain end
		vk13Features.pNext         = &vk12Features;         // Chain to vk12Features
		maintenance4Features.pNext = &vk12Features;         // Chain to vk12Features
		bdaFeatures.pNext          = &maintenance4Features; // Chain to maintenance4Features
		meshShaderFeatures.pNext   = &bdaFeatures;          // Chain to bdaFeatures
		
		void* pNextChain = &meshShaderFeatures;

		if (m_PresentIdSupported)
		{
			pNextChain = &presentIdFeaturesEnable;
		}

        VkPhysicalDeviceFeatures deviceFeatures{};
		{
			// Enable anisotropic filtering
			// A texture filtering technique that improves the quality of textures when viewed at oblique angles.
			deviceFeatures.samplerAnisotropy  = VK_TRUE;
			deviceFeatures.shaderInt64        = VK_TRUE;

			// Enable fill mode non solid for wireframe support
			deviceFeatures.fillModeNonSolid   = VK_TRUE;
			deviceFeatures.robustBufferAccess = VK_TRUE;
		}


        // Device
		VkDeviceCreateInfo createInfo{ VKInit::deviceCreateInfo() };
		{
			// This field is a pointer to an extension structure. 
			// It allows to chain additional information, enabling the use of Vulkan extensions. 
			// This is where you would place structures that enable newer Vulkan features.
        	createInfo.pNext                   = pNextChain;

			// pEnabledFeatures is the older, legacy way of specifying core Vulkan 1.0 features,
			// when using VkPhysicalDeviceFeatures2 set it to nullptr
			createInfo.pEnabledFeatures        = &deviceFeatures;
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
			if constexpr (kEnableValidationLayers)
			{
				createInfo.enabledLayerCount   = static_cast<U32>(kValidationLayers.size());
				createInfo.ppEnabledLayerNames = kValidationLayers.data();
			}
		}

		VK_CHECK(vkCreateDevice(m_PhysicalDevice, &createInfo, nullptr, &m_Device));

		volkLoadDevice(m_Device);

		VY_ASSERT(indices.isComplete(), "Queue family indices are not complete.");

		// Initialize debug label.
		VyDebugLabel::init();

		// Create queues.
		vkGetDeviceQueue(m_Device, indices.GraphicsFamily.value(), 0, &m_GraphicsQueue);
		vkGetDeviceQueue(m_Device, indices.PresentFamily .value(), 0, &m_PresentQueue );
		vkGetDeviceQueue(m_Device, indices.ComputeFamily .value(), 0, &m_ComputeQueue );
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

			allocatorInfo.vulkanApiVersion = kAPIVersion;
		}

		VmaVulkanFunctions vulkanFunctions;
		vmaImportVulkanFunctionsFromVolk(&allocatorInfo, &vulkanFunctions);
		
        allocatorInfo.pVulkanFunctions = &vulkanFunctions;

		VK_CHECK(vmaCreateAllocator(&allocatorInfo, &m_Allocator));
	}

	// --------------------------------------------------------------------------------------------
    // MARK: Command Pool
    
    void VyDevice::createCommandPool() 
    {
        QueueFamilyIndices queueFamilyIndices = findQueueFamilies();

		VY_ASSERT(queueFamilyIndices.GraphicsFamily.has_value(), "Graphics queue family not found.");

        // Graphics Command Pool
		VkCommandPoolCreateInfo poolInfo{ VKInit::commandPoolCreateInfo() };
		{
			poolInfo.queueFamilyIndex = queueFamilyIndices.GraphicsFamily.value();
			poolInfo.flags            = { VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT };
		}

		VK_CHECK(vkCreateCommandPool(m_Device, &poolInfo, nullptr, &m_CommandPool));
	}

	// --------------------------------------------------------------------------------------------
    // MARK: Surface
    
    void VyDevice::createSurface(VyWindow& window) 
    { 
        window.createWindowSurface(m_Instance, &m_Surface); 
    }

#pragma endregion [ Main Functions ]


// =========================================================================================================================
#pragma region [ Support ]
// =========================================================================================================================

    void VyDevice::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) 
    {
        createInfo = { VKInit::debugUtilsMessengerCreateInfoEXT() };
        {
            createInfo.messageSeverity = {
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | 
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT 
            };
            createInfo.messageType     = { 
                VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT  |
                VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT 
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


    bool VyDevice::checkValidationLayerSupport() 
    {
		U32 layerCount{};
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		TVector<VkLayerProperties> availableLayers(layerCount);
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

    
	TVector<CString> VyDevice::queryRequiredInstanceExtensions() const
	{
		U32 glfwExtensionCount{};
		CString* glfwExtensions{ glfwGetRequiredInstanceExtensions(&glfwExtensionCount) };

        if (glfwExtensions == nullptr) 
        {
            VY_THROW_RUNTIME_ERROR("Vulkan is not available on this platform!");
        }

		TVector<CString> extensions;
		extensions.reserve(glfwExtensionCount);
		extensions.insert(extensions.end(), glfwExtensions, glfwExtensions + glfwExtensionCount);

		if constexpr (kEnableValidationLayers)
		{
			extensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}

		return extensions;
	}

    
    void VyDevice::checkGflwRequiredInstanceExtensions() 
    {
		auto extensions = queryInstanceExtensionProperties();

		THashSet<String> available;
		for (const auto& extension : extensions)
		{
			available.insert(extension.extensionName);
		}

		for (const auto& required : queryRequiredInstanceExtensions())
		{
			VY_ASSERT(available.find(required) != available.end(), "Missing required glfw extension");
		}
    }


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

    
    bool VyDevice::checkDeviceExtensionSupport(VkPhysicalDevice device) 
    {
		auto availableExtensions = queryDeviceExtensionProperties(device);

		TSet<String> requiredExtensions( kDeviceExtensions.begin(), kDeviceExtensions.end() );

		for (const auto& extension : availableExtensions)
		{
			requiredExtensions.erase(extension.extensionName);
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
		if (!vk12Features.descriptorIndexing                        && 
			!vk12Features.shaderSampledImageArrayNonUniformIndexing &&
			!vk12Features.descriptorBindingPartiallyBound           && 
			!vk12Features.descriptorBindingVariableDescriptorCount  &&
			!vk12Features.runtimeDescriptorArray                    && 
			!vk12Features.bufferDeviceAddress)
		{
			return false;
		}

		// TODO: check for more features (dynamic rendering, mesh shaders, etc.)

		return true;
	}

    
	QueueFamilyIndices VyDevice::findQueueFamilies(VkPhysicalDevice device) //const
	{
		QueueFamilyIndices indices;

		U32 queueFamilyCount{};
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

		TVector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		U32 i = 0;
		for (const auto& queueFamily : queueFamilies)
		{
			// Graphics
			if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				indices.GraphicsFamily = i;
			}

			// Compute
			if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT) 
			{
				indices.ComputeFamily = i;
			}

			// Present
			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_Surface, &presentSupport);
			
			if (queueFamily.queueCount > 0 && presentSupport)
			{
				indices.PresentFamily = i;
			}

			if (indices.isComplete())
			{
				break;
			}

			i++;
		}

		return indices;
	}

        
	SwapchainSupportDetails VyDevice::querySwapchainSupport(VkPhysicalDevice device) const
	{
		SwapchainSupportDetails details;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_Surface, &details.Capabilities);

		U32 formatCount{};
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_Surface, &formatCount, nullptr);

		if (formatCount != 0)
		{
			details.Formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_Surface, &formatCount, details.Formats.data());
		}

		U32 presentModeCount{};
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_Surface, &presentModeCount, nullptr);

		if (presentModeCount != 0)
		{
			details.PresentModes.resize(presentModeCount);

			vkGetPhysicalDeviceSurfacePresentModesKHR(
				device,
				m_Surface,
				&presentModeCount,
				details.PresentModes.data()
			);
		}

		return details;
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


	VkFormat VyDevice::findDepthFormat()
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



	SwapchainSupportDetails VyDevice::querySwapchainSupport() const
	{
		return querySwapchainSupport(m_PhysicalDevice);
	}


	QueueFamilyIndices VyDevice::findQueueFamilies() // const
	{
		return findQueueFamilies(m_PhysicalDevice);
	}


#pragma endregion [ Support ]

    // ============================================================================================

#pragma region [ Utils ]


	VkCommandBuffer VyDevice::beginSingleTimeCommands() const
	{
		VkCommandBufferAllocateInfo allocInfo{ VKInit::commandBufferAllocateInfo() };
        {
            allocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            allocInfo.commandPool        = m_CommandPool;
            allocInfo.commandBufferCount = 1;
        }

		VkCommandBuffer cmdBuffer;
		vkAllocateCommandBuffers(m_Device, &allocInfo, &cmdBuffer);

		VkCommandBufferBeginInfo beginInfo{ VKInit::commandBufferBeginInfo() };
		{
			beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		}

		vkBeginCommandBuffer(cmdBuffer, &beginInfo);
		
		return cmdBuffer;
	}
	

	void VyDevice::endSingleTimeCommands(VkCommandBuffer cmdBuffer) const
	{
		vkEndCommandBuffer(cmdBuffer);

		VkSubmitInfo submitInfo{ VKInit::submitInfo() };
		{
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers    = &cmdBuffer;
		}

		vkQueueSubmit(m_GraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);

		vkQueueWaitIdle(m_GraphicsQueue);

		vkFreeCommandBuffers(m_Device, m_CommandPool, 1, &cmdBuffer);
	}


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


	VkImageView VyDevice::createImageView(const VkImageViewCreateInfo& viewInfo)
	{
		VkImageView imageView;
		VK_CHECK(vkCreateImageView(m_Device, &viewInfo, nullptr, &imageView));

		return imageView;
	}
	

	VkSampler VyDevice::createSampler(const VkSamplerCreateInfo& samplerInfo) 
	{
		VkSampler sampler;
		VK_CHECK(vkCreateSampler(m_Device, &samplerInfo, nullptr, &sampler));

		return sampler;
	}



	VkResult VyDevice::createImageView(
		VkImageView&                 imageView, 
		const VkImageViewCreateInfo& viewInfo)
	{
		return vkCreateImageView(m_Device, &viewInfo, nullptr, &imageView);
	}

	
	VkResult VyDevice::createImage(
		VkImage&                       image, 
		VmaAllocation&                 allocation, 
		const VkImageCreateInfo&       imageInfo, 
		const VmaAllocationCreateInfo& allocInfo)
	{
		return vmaCreateImage(
			m_Allocator, 
			&imageInfo, 
			&allocInfo, 
			&image, 
			&allocation, 
			nullptr
		);
	}



	// https://github.com/KabalMcBlade/Vesper/blob/3c62995e7ddc739d816917f85afd5a86a0bc8089/VesperEngine/Backend/device.cpp



	// void VyDevice::copyBufferToImage(
	// 	VkBuffer buffer, 
	// 	VkImage  image, 
	// 	U32      width, 
	// 	U32      height, 
	// 	U32      layerCount, 
    //     U32      baseArrayLayer,
    //     U32      depth) const
	// {
	// 	VkCommandBuffer cmdBuffer = beginSingleTimeCommands();
    //     {
    //         VkBufferImageCopy region{};
    //         {
    //             region.bufferOffset                    = 0;
    //             region.bufferRowLength                 = 0;
    //             region.bufferImageHeight               = 0;

    //             region.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    //             region.imageSubresource.mipLevel       = 0;
    //             region.imageSubresource.baseArrayLayer = baseArrayLayer;
    //             region.imageSubresource.layerCount     = layerCount;

    //             region.imageOffset = { 0, 0, 0 };
    //             region.imageExtent = { width, height, depth };
    //         }

    //         vkCmdCopyBufferToImage(
    //             cmdBuffer,
    //             buffer,
    //             image,
    //             VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
    //             1,
    //             &region
    //         );
    //     }
	// 	endSingleTimeCommands(cmdBuffer);
	// }


	void VyDevice::copyBufferToImageCubemap(
		VkBuffer buffer, 
		VkImage  image, 
		U32      width, 
		U32      height, 
		U32      layerCount) const
	{
		VkCommandBuffer cmdBuffer = beginSingleTimeCommands();
        {
			TVector<VkBufferImageCopy> regions;

			const U32 imageSize = width * height * 4 * 6;
			const U32 layerSize = imageSize / 6;

			for (U32 face = 0; face < 6; face++)
			{
				// TODO: add support for mip levels.

				const U32 offset = layerSize * face;

				VkBufferImageCopy region{};
				{
					region.bufferOffset = offset;

					region.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
					region.imageSubresource.mipLevel       = 0;
					region.imageSubresource.baseArrayLayer = face;
					region.imageSubresource.layerCount     = 1;

					region.imageOffset = { 0, 0, 0 };
					region.imageExtent = { width, height, 1 };
				}

				regions.push_back(region);
			}

			vkCmdCopyBufferToImage(
				cmdBuffer,
				buffer,
				image,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				static_cast<U32>(regions.size()),
				regions.data()
			);
		}
		endSingleTimeCommands(cmdBuffer);
	}





	void VyDevice::transitionImageLayout(
		VkCommandBuffer    cmdBuffer, 
		VkImage            image, 
		VkImageLayout      oldLayout,
		VkImageLayout      newLayout, 
        U32                layerCount,
        U32                baseLayer,
        U32                levelCount,
        U32                baseLevel,
		VkImageAspectFlags aspectFlags) const
	{
		// https://docs.vulkan.org/refpages/latest/refpages/source/VkImageMemoryBarrier.html

		VkImageMemoryBarrier barrier{ VKInit::imageMemoryBarrier() };
		{
			barrier.oldLayout           = oldLayout;
			barrier.newLayout           = newLayout;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.image               = image;
			
			barrier.subresourceRange.aspectMask     = aspectFlags;
			barrier.subresourceRange.baseMipLevel   = baseLevel;
			barrier.subresourceRange.levelCount     = levelCount;
			barrier.subresourceRange.baseArrayLayer = baseLayer;
			barrier.subresourceRange.layerCount     = layerCount;

			barrier.srcAccessMask = VKUtil::srcAccessMask(barrier.oldLayout);
			barrier.dstAccessMask = VKUtil::dstAccessMask(barrier.newLayout);
		}

		VkPipelineStageFlags srcStage = VKUtil::srcStage(barrier.srcAccessMask);
		VkPipelineStageFlags dstStage = VKUtil::dstStage(barrier.dstAccessMask);

		vkCmdPipelineBarrier(
			cmdBuffer,
			srcStage, dstStage,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);
	}


	void VyDevice::transitionImageLayout(
		VkImage            image, 
		VkImageLayout      oldLayout, 
		VkImageLayout      newLayout,
        U32                layerCount,
        U32                baseLayer,
        U32                levelCount,
        U32                baseLevel,
		VkImageAspectFlags aspectFlags) const
	{
		VkCommandBuffer cmdBuffer = beginSingleTimeCommands();
		{
			transitionImageLayout(
                cmdBuffer, 
                image, 
                oldLayout, 
                newLayout, 
                layerCount,
                baseLayer,
                levelCount,
                baseLevel,
                aspectFlags
            );
		}
		endSingleTimeCommands(cmdBuffer);
	}


	void VyDevice::transitionImageLayout(
		VkImage       image, 
		VkFormat      format, 
		VkImageLayout oldLayout, 
		VkImageLayout newLayout,
		U32           mipLevels) const
	{
		transitionImageLayout(image, oldLayout, newLayout, mipLevels, findAspectFlags(format));
	}

#pragma endregion [ Utils ]

    // ============================================================================================


	void VyDevice::copyBuffer(
		VkCommandBuffer cmdBuffer, 
		VkBuffer        srcBuffer, 
		VkBuffer        dstBuffer, 
		VkDeviceSize    size)
	{
		recordCopyBuffer(cmdBuffer, srcBuffer, dstBuffer, size);
	}

	void VyDevice::copyBuffer(
		VkBuffer        srcBuffer, 
		VkBuffer        dstBuffer, 
		VkDeviceSize    size)
	{
		VkCommandBuffer cmdBuffer = beginSingleTimeCommands();
		{
			recordCopyBuffer(cmdBuffer, srcBuffer, dstBuffer, size);
		}
		endSingleTimeCommands(cmdBuffer);
	}


	void VyDevice::copyBufferToImage(
		VkCommandBuffer cmdBuffer, 
		VkBuffer        buffer, 
		VkImage         image, 
		U32             width, 
		U32             height, 
		U32             layerCount /* 1 */, 
		U32             mipLevel   /* 1 */)
	{
		recordCopyBufferToImage(cmdBuffer, buffer, image, width, height, layerCount, mipLevel);
	}

	void VyDevice::copyBufferToImage(
		VkBuffer        buffer, 
		VkImage         image, 
		U32             width, 
		U32             height, 
		U32             layerCount /* 1 */, 
		U32             mipLevel   /* 1 */)
	{
		VkCommandBuffer cmdBuffer = beginSingleTimeCommands();
		{
			recordCopyBufferToImage(cmdBuffer, buffer, image, width, height, layerCount, mipLevel);
		}
		endSingleTimeCommands(cmdBuffer);
	}


	void VyDevice::copyImageToBuffer(
		VkCommandBuffer cmdBuffer, 
		VkImage         image, 
		VkBuffer        buffer, 
		U32             width, 
		U32             height, 
		U32             layerCount /* 1 */, 
		U32             mipLevel   /* 1 */)
	{
		recordCopyImageToBuffer(cmdBuffer, image, buffer, width, height, layerCount, mipLevel);
	}

	void VyDevice::copyImageToBuffer(
		VkImage         image, 
		VkBuffer        buffer, 
		U32             width, 
		U32             height, 
		U32             layerCount /* 1 */, 
		U32             mipLevel   /* 1 */)
	{
		VkCommandBuffer cmdBuffer = beginSingleTimeCommands();
		{
			recordCopyImageToBuffer(cmdBuffer, image, buffer, width, height, layerCount, mipLevel);
		}
		endSingleTimeCommands(cmdBuffer);
	}


	void VyDevice::copyImage(
		VkCommandBuffer cmdBuffer, 
		VkImage         srcImage, 
		VkImage         dstImage, 
		U32             width, 
		U32             height, 
		U32             srcBaseLayerIndex /* 0 */, 
		U32             dstBaseLayerIndex /* 0 */, 
		U32             layerCount        /* 1 */)
	{
		recordCopyImage(cmdBuffer, srcImage, dstImage, width, height, srcBaseLayerIndex, dstBaseLayerIndex, layerCount);
	}

	void VyDevice::copyImage(
		VkImage         srcImage, 
		VkImage         dstImage, 
		U32             width, 
		U32             height, 
		U32             srcBaseLayerIndex /* 0 */, 
		U32             dstBaseLayerIndex /* 0 */, 
		U32             layerCount        /* 1 */)
	{
		VkCommandBuffer cmdBuffer = beginSingleTimeCommands();
		{
			recordCopyImage(cmdBuffer, srcImage, dstImage, width, height, srcBaseLayerIndex, dstBaseLayerIndex, layerCount);
		}
		endSingleTimeCommands(cmdBuffer);
	}


	void VyDevice::transitionImageLayout(
		VkCommandBuffer cmdBuffer, 
		VkImage         image, 
		VkFormat        format, 
		VkImageLayout   oldLayout, 
		VkImageLayout   newLayout, 
		U32             baseLayerIndex /* 0 */, 
		U32             layerCount     /* 1 */, 
		U32             mipLevel       /* 1 */)
	{
		recordTransitionImageLayout(cmdBuffer, image, format, oldLayout, newLayout, baseLayerIndex, layerCount, mipLevel);
	}

	void VyDevice::transitionImageLayout(
		VkImage         image, 
		VkFormat        format, 
		VkImageLayout   oldLayout, 
		VkImageLayout   newLayout, 
		U32             baseLayerIndex /* 0 */, 
		U32             layerCount     /* 1 */, 
		U32             mipLevel       /* 1 */)
	{
		VkCommandBuffer cmdBuffer = beginSingleTimeCommands();
		{
			recordTransitionImageLayout(cmdBuffer, image, format, oldLayout, newLayout, baseLayerIndex, layerCount, mipLevel);
		}
		endSingleTimeCommands(cmdBuffer);
	}

    // ============================================================================================


	void VyDevice::recordCopyBuffer(
		VkCommandBuffer cmdBuffer, 
		VkBuffer        srcBuffer, 
		VkBuffer        dstBuffer, 
		VkDeviceSize    size)
	{
		VkBufferCopy copyRegion{};
		{
			copyRegion.srcOffset = 0;
			copyRegion.dstOffset = 0;
			copyRegion.size      = size;
		}

		vkCmdCopyBuffer(cmdBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
	}

	
	void VyDevice::recordCopyBufferToImage(
		VkCommandBuffer cmdBuffer, 
		VkBuffer        buffer, 
		VkImage         image, 
		U32             width, 
		U32             height, 
		U32             layerCount, 
		U32             mipLevel)
	{
		VkBufferImageCopy region{};
		{
			region.bufferOffset      = 0;
			region.bufferRowLength   = 0;
			region.bufferImageHeight = 0;

			region.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
			region.imageSubresource.mipLevel       = mipLevel - 1;
			region.imageSubresource.baseArrayLayer = 0;
			region.imageSubresource.layerCount     = layerCount;
			
			region.imageOffset = { 0, 0, 0 };
			region.imageExtent = { width, height, 1 };
		}

		vkCmdCopyBufferToImage(cmdBuffer, 
			buffer, 
			image, 
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
			1, 
			&region
		);
	}

	
	void VyDevice::recordCopyImageToBuffer(
		VkCommandBuffer cmdBuffer, 
		VkImage         image, 
		VkBuffer        buffer, 
		U32             width, 
		U32             height, 
		U32             layerCount, 
		U32             mipLevel)
	{
		VkBufferImageCopy region{};
		{
			region.bufferOffset      = 0;
			region.bufferRowLength   = 0;
			region.bufferImageHeight = 0;
			
			region.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
			region.imageSubresource.mipLevel       = mipLevel - 1;
			region.imageSubresource.baseArrayLayer = 0;
			region.imageSubresource.layerCount     = layerCount;
			
			region.imageOffset = { 0, 0, 0 };
			region.imageExtent = { width, height, 1 };
		}

		vkCmdCopyImageToBuffer(cmdBuffer, 
			image, 
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, 
			buffer, 
			1, 
			&region
		);
	}

	
	void VyDevice::recordCopyImage(
		VkCommandBuffer cmdBuffer, 
		VkImage         srcImage, 
		VkImage         dstImage, 
		U32             width, 
		U32             height, 
		U32             srcBaseLayerIndex, 
		U32             dstBaseLayerIndex, 
		U32             layerCount)
	{
		VkImageCopy region{};
		{
			region.srcSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
			region.srcSubresource.mipLevel       = 0;
			region.srcSubresource.baseArrayLayer = srcBaseLayerIndex;
			region.srcSubresource.layerCount     = layerCount;
			region.srcOffset = { 0, 0, 0 };
			
			region.dstSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
			region.dstSubresource.mipLevel       = 0;
			region.dstSubresource.baseArrayLayer = dstBaseLayerIndex;
			region.dstSubresource.layerCount     = layerCount;
			region.dstOffset = { 0, 0, 0 };

			region.extent = { width, height, 1 };
		}

		vkCmdCopyImage(cmdBuffer,
			srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1,
			&region
		);
	}

	
	/**
	 * EXAMPLE TRANSITIONS:
	 *
	 * Transition for Rendering:
	 * transitionImageLayout(Image, Format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	 *
	 * Transition for Saving / copy to staging buffer:
	 * transitionImageLayout(Image, Format, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
	 *
	 * Transition for Sampling in the Shader
	 * transitionImageLayout(Image, Format, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	 *
	 * Transition for general Destination
	 * transitionImageLayout(Image, Format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	 *
	 */
	void VyDevice::recordTransitionImageLayout(
		VkCommandBuffer cmdBuffer, 
		VkImage         image, 
		VkFormat        format, 
		VkImageLayout   oldLayout, 
		VkImageLayout   newLayout, 
		U32             baseLayerIndex, 
		U32             layerCount, 
		U32             mipLevel)
	{
		VkImageMemoryBarrier barrier{ VKInit::imageMemoryBarrier() };
		{
			barrier.oldLayout = oldLayout;
			barrier.newLayout = newLayout;

			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			
			barrier.image = image;
		}

		// Use format to determine the aspect mask.
		barrier.subresourceRange.aspectMask     = VKUtil::aspectFlags(format);

		barrier.subresourceRange.baseMipLevel   = 0;
		barrier.subresourceRange.levelCount     = mipLevel;
		barrier.subresourceRange.baseArrayLayer = baseLayerIndex;
		barrier.subresourceRange.layerCount     = layerCount;

		VkPipelineStageFlags srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		VkPipelineStageFlags dstStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;

		// Handle layout transitions.
		// -------------------------------------------------------------------------
		// Old: Undefined   
		// New: TransferDst 
		if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && 
			newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
		{
			// SrcAccess: 0
			// DstAccess: TransferWrite
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			// SrcStage: Top
			// DstStage: Transfer
			srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		// -------------------------------------------------------------------------
		// Old: TransferDst
		// New: ShaderRead
		else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && 
				 newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			// SrcAccess: TransferWrite
			// DstAccess: ShaderRead
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			// SrcStage: Transfer
			// DstStage: FragmentShader
			srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		// -------------------------------------------------------------------------
		// Old: Undefined
		// New: ColorAttachment
		else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && 
				 newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
		{
			// SrcAccess: 0
			// DstAccess: ColorAttachmentWrite
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

			// SrcStage: Top
			// DstStage: ColorAttachmentOutput
			srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			dstStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		}
		// -------------------------------------------------------------------------
		// Old: ColorAttachment
		// New: TransferSrc
		else if (oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && 
				 newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
		{
			// SrcAccess: ColorAttachmentWrite
			// DstAccess: TransferRead
			barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

			// SrcStage: ColorAttachmentOutput
			// DstStage: Transfer
			srcStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		// -------------------------------------------------------------------------
		// Old: TransferSrc
		// New: ShaderRead
		else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL && 
				 newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			// SrcAccess: TransferRead
			// DstAccess: ShaderRead
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			// SrcStage: Transfer
			// DstStage: FragmentShader
			srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		// -------------------------------------------------------------------------
		// Old: ShaderRead
		// New: ColorAttachment
		else if (oldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && 
				 newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
		{
			// SrcAccess: ShaderRead
			// DstAccess: ColorAttachmentWrite
			barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
			barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

			// SrcStage: FragmentShader
			// DstStage: ColorAttachmentOutput
			srcStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			dstStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		}
		// -------------------------------------------------------------------------
		// Old: TransferDst
		// New: TransferSrc
		else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && 
				 newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
		{
			// SrcAccess: TransferWrite
			// DstAccess: TransferRead
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

			// SrcStage: Transfer
			// DstStage: Transfer
			srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		// -------------------------------------------------------------------------
		// Old: ColorAttachment
		// New: ShaderRead
		else if (oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && 
				 newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			// SrcAccess: ColorAttachmentWrite
			// DstAccess: ShaderRead
			barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			// SrcStage: ColorAttachmentOutput
			// DstStage: FragmentShader
			srcStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		// -------------------------------------------------------------------------
		// Old: ColorAttachment
		// New: TransferDst
		else if (oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && 
				 newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
		{
			// SrcAccess: ColorAttachmentWrite
			// DstAccess: TransferRead
			barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			// SrcStage: ColorAttachmentOutput
			// DstStage: Transfer
			srcStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		// -------------------------------------------------------------------------
		// Old: TransferDst
		// New: ColorAttachment
		else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && 
				 newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
		{
			// SrcAccess: TransferWrite
			// DstAccess: ColorAttachmentWrite 
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

			// SrcStage: Transfer
			// DstStage: ColorAttachmentOutput 
			srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			dstStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		}
		// -------------------------------------------------------------------------
		// Old: TransferSrc
		// New: ColorAttachment
		else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL && 
				 newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
		{
			// SrcAccess: TransferRead
			// DstAccess: ColorAttachmentWrite 
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

			// SrcStage: Transfer
			// DstStage: ColorAttachmentOutput 
			srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			dstStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		}
		// -------------------------------------------------------------------------
		// Old: ColorAttachment
		// New: TransferDst
		else if (oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && 
				 newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
		{
			// SrcAccess: ColorAttachmentWrite
			// DstAccess: TransferWrite
			barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			// SrcStage: ColorAttachmentOutput 
			// DstStage: Transfer
			srcStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		// -------------------------------------------------------------------------
		else
		{
			VY_THROW_INVALID_ARGUMENT("Unsupported layout transition!");
		}

		vkCmdPipelineBarrier(cmdBuffer,
			srcStage, dstStage,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);
	}
}
























// namespace Vy
// {
// //? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// //? ~~	  SyncManager	
// //? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// //--------------------------------------------------
// //    Constructor & Destructor
// //--------------------------------------------------
// void SyncManager::create(U32 maxFramesInFlight)
// {
// 	m_MaxFrames = maxFramesInFlight;

// 	m_vFrameSyncs.resize(maxFramesInFlight);
// 	for (U32 index{}; index < maxFramesInFlight; ++index)
// 	{
// 		m_vFrameSyncs[index].imageAvailable = CreateSemaphore(context);
// 		m_vFrameSyncs[index].renderFinished = CreateSemaphore(context);
// 		m_vFrameSyncs[index].inFlight = CreateFence(context, true);
// 	}
// }
// void SyncManager::Cleanup(const Context& context)
// {
// 	for (auto& semaphore : m_vSemaphores)
// 		vkDestroySemaphore(context.device.handle(), semaphore, nullptr);

// 	for (auto& fence : m_vFences)
// 		vkDestroyFence(context.device.handle(), fence, nullptr);

// 	m_vFrameSyncs.clear();
// 	m_vSemaphores.clear();
// 	m_vFences.clear();
// }

// //--------------------------------------------------
// //    Accessors & Mutators
// //--------------------------------------------------
// const FrameSync& SyncManager::GetFrameSync(U32 frame) const { return m_vFrameSyncs[frame % m_MaxFrames]; }


// //--------------------------------------------------
// //    Makers
// //--------------------------------------------------
// const VkSemaphore& SyncManager::CreateSemaphore(const Context& context)
// {
// 	VkSemaphoreCreateInfo semaphoreInfo{};
// 	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

// 	VkSemaphore semaphore;
// 	if (vkCreateSemaphore(context.device.handle(), &semaphoreInfo, nullptr, &semaphore) != VK_SUCCESS)
// 		VY_THROW_RUNTIME_ERROR("Failed to create semaphore!");

// 	m_vSemaphores.push_back(semaphore);
// 	return m_vSemaphores.back();
// }
// const VkFence& SyncManager::CreateFence(bool signaled)
// {
// 	VkFenceCreateInfo fenceInfo{};
// 	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
// 	fenceInfo.flags = signaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0;

// 	VkFence fence;
// 	if (vkCreateFence(context.device.handle(), &fenceInfo, nullptr, &fence) != VK_SUCCESS)
// 		VY_THROW_RUNTIME_ERROR("Failed to create fence!");

// 	m_vFences.push_back(fence);
// 	return m_vFences.back();
// }


// 	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 	//? ~~	  CommandBuffer	
// 	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// 	//--------------------------------------------------
// 	//    Constructor & Destructor
// 	//--------------------------------------------------
// 	CommandBuffer::CommandBuffer(VkCommandPool pool, VkCommandBuffer buffer)
// 		: m_CmdBuffer(buffer)
// 		, m_PoolOwner(pool)
// 	{
// 	}


// 	CommandBuffer::CommandBuffer(CommandBuffer&& other) noexcept
// 	{
// 		m_CmdBuffer = std::move(other.m_CmdBuffer);
// 		m_PoolOwner = std::move(other.m_PoolOwner);

// 		other.m_CmdBuffer = VK_NULL_HANDLE;
// 		other.m_PoolOwner = VK_NULL_HANDLE;
// 	}

// 	CommandBuffer& CommandBuffer::operator=(CommandBuffer&& other) noexcept
// 	{
// 		if (this == &other)
// 		{
// 			return *this;
// 		}

// 		m_CmdBuffer = std::move(other.m_CmdBuffer);
// 		m_PoolOwner = std::move(other.m_PoolOwner);

// 		other.m_CmdBuffer = VK_NULL_HANDLE;
// 		other.m_PoolOwner = VK_NULL_HANDLE;

// 		return *this;
// 	}

// 	//--------------------------------------------------
// 	//    Accessors & Mutators
// 	//--------------------------------------------------
// 	const VkCommandBuffer& CommandBuffer::handle() const { return m_CmdBuffer; }


// 	//--------------------------------------------------
// 	//    Commands
// 	//--------------------------------------------------
// 	void CommandBuffer::begin(VkCommandBufferUsageFlags usage) const
// 	{
// 		VkCommandBufferBeginInfo beginInfo{ VKInit::commandBufferBeginInfo() };
// 		{
// 			beginInfo.flags = usage;
// 		}

// 		if (vkBeginCommandBuffer(m_CmdBuffer, &beginInfo) != VK_SUCCESS)
// 		{
// 			VY_THROW_RUNTIME_ERROR("Failed to begin recording Command Buffer!");
// 		}
// 	}

// 	void CommandBuffer::end() const
// 	{
// 		if (vkEndCommandBuffer(m_CmdBuffer) != VK_SUCCESS)
// 		{
// 			VY_THROW_RUNTIME_ERROR("Failed to record command buffer!");
// 		}
// 	}


// 	void CommandBuffer::submit(VkQueue queue, bool waitIdle, const SemaphoreInfo& semaphoreInfo, VkFence fence) const
// 	{
// 		// -- Submit Info --
// 		VkSubmitInfo2 submitInfo{ VKInit::submitInfo2() };
// 		{
// 			submitInfo.pNext = nullptr;
// 			submitInfo.flags = 0;
// 		}

// 		// -- Command Buffer Submit Info --
// 		VkCommandBufferSubmitInfo cmdSubmitInfo{ VKInit::commandBufferSubmitInfo() };
// 		{
// 			cmdSubmitInfo.pNext         = nullptr;
// 			cmdSubmitInfo.cmdBuffer = m_CmdBuffer;
// 			cmdSubmitInfo.deviceMask    = 0;
// 		}

// 		submitInfo.commandBufferInfoCount = 1;
// 		submitInfo.pCommandBufferInfos    = &cmdSubmitInfo;

// 		// -- Wait Semaphores --
// 		TVector<VkSemaphoreSubmitInfo> waitSemaphoreSubmitInfos;
// 		waitSemaphoreSubmitInfos.reserve(semaphoreInfo.SignalSemaphores.size());

// 		if (!semaphoreInfo.WaitSemaphores.empty())
// 		{
// 			U32 index = 0;
		
// 			for (const VkSemaphore& semaphore : semaphoreInfo.WaitSemaphores)
// 			{
// 				VkSemaphoreSubmitInfo semaphoreSubmitInfo{ VKInit::semaphoreSubmitInfo()};
// 				{
// 					semaphoreSubmitInfo.pNext     = nullptr;
// 					semaphoreSubmitInfo.value     = 0;
// 					semaphoreSubmitInfo.semaphore = semaphore;
// 					semaphoreSubmitInfo.stageMask = semaphoreInfo.WaitStages[ index ];
// 				}

// 				waitSemaphoreSubmitInfos.emplace_back(semaphoreSubmitInfo);
// 				++index;
// 			}

// 			submitInfo.waitSemaphoreInfoCount = static_cast<U32>(waitSemaphoreSubmitInfos.size());
// 			submitInfo.pWaitSemaphoreInfos    = waitSemaphoreSubmitInfos.data();
// 		}

// 		// -- Signal Semaphores --
// 		TVector<VkSemaphoreSubmitInfo> signalSemaphoreSubmitInfos;
// 		signalSemaphoreSubmitInfos.reserve(semaphoreInfo.SignalSemaphores.size());

// 		if (!semaphoreInfo.SignalSemaphores.empty())
// 		{
// 			for (const VkSemaphore& semaphore : semaphoreInfo.SignalSemaphores)
// 			{
// 				VkSemaphoreSubmitInfo semaphoreSubmitInfo{ VKInit::semaphoreSubmitInfo()};
// 				{
// 					semaphoreSubmitInfo.pNext     = nullptr;
// 					semaphoreSubmitInfo.value     = 0;
// 					semaphoreSubmitInfo.semaphore = semaphore;
// 				}

// 				signalSemaphoreSubmitInfos.emplace_back(semaphoreSubmitInfo);
// 			}

// 			submitInfo.signalSemaphoreInfoCount = static_cast<U32>(signalSemaphoreSubmitInfos.size());
// 			submitInfo.pSignalSemaphoreInfos    = signalSemaphoreSubmitInfos.data();
// 		}

// 		// -- Submit --
// 		if (vkQueueSubmit2(queue, 1, &submitInfo, fence) != VK_SUCCESS)
// 		{
// 			VY_THROW_RUNTIME_ERROR("Failed to submit Command Buffer!");
// 		}

// 		if (waitIdle)
// 		{
// 			vkQueueWaitIdle(queue);
// 		}
// 	}


// 	void CommandBuffer::reset() const 
// 	{ 
// 		vkResetCommandBuffer(m_CmdBuffer, 0); 
// 	}
	

// 	void CommandBuffer::free() 
// 	{ 
// 		vkFreeCommandBuffers(VyContext::device(), m_PoolOwner, 1, &m_CmdBuffer); 

// 		m_CmdBuffer = VK_NULL_HANDLE; 
// 	}


// 	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 	//? ~~	  CommandPool	
// 	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// 	//--------------------------------------------------
// 	//    Constructor & Destructor
// 	//--------------------------------------------------
// 	CommandPool& CommandPool::create()
// 	{
// 		const QueueFamilyIndices queueFamilyIndices = VyContext::device().findQueueFamilies();

// 		VkCommandPoolCreateInfo poolInfo{ VKInit::commandPoolCreateInfo() };
// 		{
// 			poolInfo.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
// 			poolInfo.queueFamilyIndex = queueFamilyIndices.GraphicsFamily.value();
// 		}

// 		if (vkCreateCommandPool(VyContext::device(), &poolInfo, nullptr, &m_CommandPool) != VK_SUCCESS)
// 		{
// 			VY_THROW_RUNTIME_ERROR("Failed to create Command Pool!");
// 		}
			
// 		return *this;
// 	}


// 	void CommandPool::destroy() const
// 	{
// 		vkDestroyCommandPool(VyContext::device(), m_CommandPool, nullptr);
// 	}


// 	//--------------------------------------------------
// 	//    Accessors & Mutators
// 	//--------------------------------------------------
// 	VkCommandPool& CommandPool::handle()
// 	{ 
// 		return m_CommandPool; 
// 	}


// 	CommandBuffer& CommandPool::getBuffer(U32 bufferIdx)
// 	{
// 		if (bufferIdx < 0 || bufferIdx >= m_CommandBuffers.size())
// 		{
// 			throw std::out_of_range("Buffer index out of range!");
// 		}

// 		return m_CommandBuffers[bufferIdx];
// 	}


// 	CommandBuffer& CommandPool::allocateCmdBuffers(U32 count, VkCommandBufferLevel level)
// 	{
// 		std::erase_if(m_CommandBuffers, [](const CommandBuffer& b)
// 			{
// 				return b.handle() == VK_NULL_HANDLE;
// 			});

// 		VkCommandBufferAllocateInfo allocInfo{ VKInit::commandBufferAllocateInfo() };
// 		{
// 			allocInfo.commandPool        = m_CommandPool;
// 			allocInfo.level              = level;
// 			allocInfo.commandBufferCount = 1;
// 		}

// 		for (U32 index{}; index < count; ++index)
// 		{
// 			VkCommandBuffer cmdBuffer;
// 			if (vkAllocateCommandBuffers(VyContext::device(), &allocInfo, &cmdBuffer) != VK_SUCCESS)
// 			{
// 				VY_THROW_RUNTIME_ERROR("Failed to allocate command buffer!");
// 			}

// 			m_CommandBuffers.emplace_back(m_CommandPool, cmdBuffer);
// 		}
		
// 		return m_CommandBuffers.back();
// 	}
// }