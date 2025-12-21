#include <Vy/GFX/Backend/VK/VKDebug.h>

#include <Vy/GFX/Context.h>
namespace Vy
{
    void VyDebugLabel::init() 
    {
        vkCmdBeginDebugUtilsLabelEXT =
                reinterpret_cast<PFN_vkCmdBeginDebugUtilsLabelEXT>(
                    vkGetDeviceProcAddr(VyContext::device(), "vkCmdBeginDebugUtilsLabelEXT"));

        vkCmdEndDebugUtilsLabelEXT =
                reinterpret_cast<PFN_vkCmdEndDebugUtilsLabelEXT>(
                    vkGetDeviceProcAddr(VyContext::device(), "vkCmdEndDebugUtilsLabelEXT"));

        vkSetDebugUtilsObjectNameEXT =
                reinterpret_cast<PFN_vkSetDebugUtilsObjectNameEXT>(
                    vkGetDeviceProcAddr(VyContext::device(), "vkSetDebugUtilsObjectNameEXT"));

    }

    bool VyDebugLabel::isAvailable() 
    {
        return 
            vkCmdBeginDebugUtilsLabelEXT &&
            vkCmdEndDebugUtilsLabelEXT   &&
            vkSetDebugUtilsObjectNameEXT;
    }

    VyDebugLabel::ScopedCmdLabel::ScopedCmdLabel(VkCommandBuffer cmdBuffer, const String& name, const Vec4& color): 
        m_CmdBuffer(cmdBuffer) 
    {
        if (vkCmdBeginDebugUtilsLabelEXT) 
        {
            const VkDebugUtilsLabelEXT labelInfo = {
                VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT,
                nullptr,
                name.c_str(),
                { color[0], color[1], color[2], color[3] }
            };

            vkCmdBeginDebugUtilsLabelEXT(m_CmdBuffer, &labelInfo);
        }
    }

    VyDebugLabel::ScopedCmdLabel::~ScopedCmdLabel() 
    {
        if (vkCmdEndDebugUtilsLabelEXT) 
        {
            vkCmdEndDebugUtilsLabelEXT(m_CmdBuffer);
        }
    }


    void VyDebugLabel::beginCmdLabel(VkCommandBuffer cmdBuffer, const String& name, Vec4 color) 
    {
        if (vkCmdBeginDebugUtilsLabelEXT) 
        {
            const VkDebugUtilsLabelEXT labelInfo = {
                VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT,
                nullptr,
                name.c_str(),
                { color[0], color[1], color[2], color[3] }
            };

            vkCmdBeginDebugUtilsLabelEXT(cmdBuffer, &labelInfo);
        }
    }


    void VyDebugLabel::endCmdLabel(VkCommandBuffer cmdBuffer) 
    {
        if (vkCmdEndDebugUtilsLabelEXT) 
        {
            vkCmdEndDebugUtilsLabelEXT(cmdBuffer);
        }
    }


    void VyDebugLabel::setObjectName(U64 objectHandle, VkObjectType objectType, const String& name) 
    {
        if (vkSetDebugUtilsObjectNameEXT) 
        {
            const VkDebugUtilsObjectNameInfoEXT nameInfo = {
                VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
                nullptr,
                objectType,
                objectHandle,
                name.c_str()
            };
            
            vkSetDebugUtilsObjectNameEXT(VyContext::device(), &nameInfo);
        }
    }


    void VyDebugLabel::nameBuffer(VkBuffer buffer, const String& name) 
    {
        setObjectName(reinterpret_cast<U64>(buffer), VK_OBJECT_TYPE_BUFFER, name);
    }

    void VyDebugLabel::nameImage(VkImage image, const String& name) 
    {
        setObjectName(reinterpret_cast<U64>(image), VK_OBJECT_TYPE_IMAGE, name);
    }

    void VyDebugLabel::nameCommandBuffer(VkCommandBuffer cmdBuffer, const String& name) 
    {
        setObjectName(reinterpret_cast<U64>(cmdBuffer), VK_OBJECT_TYPE_COMMAND_BUFFER, name);
    }

    void VyDebugLabel::nameAllocation(VmaAllocation vma_allocation, const String& name) 
    {
        VmaAllocationInfo allocInfo;
        vmaGetAllocationInfo(VyContext::allocator(), vma_allocation, &allocInfo);

        VkDebugUtilsObjectNameInfoEXT nameInfo{};
        {
            nameInfo.sType        = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
            nameInfo.objectType   = VK_OBJECT_TYPE_DEVICE_MEMORY; // Memory is the underlying object
            nameInfo.objectHandle = reinterpret_cast<U64>(allocInfo.deviceMemory);
            nameInfo.pObjectName  = name.c_str();
        }

        vkSetDebugUtilsObjectNameEXT(VyContext::device(), &nameInfo);
    }
}



namespace Vy
{
    //--------------------------------------------------
    //    Setup
    //--------------------------------------------------
    void Debugger::setEnabled(bool enabled) 
    { 
        m_IsEnabled = enabled; 
    }
    

    void Debugger::addValidationLayer(const char* layer) 
    { 
        m_vValidationLayers.push_back(layer); 
    }


    void Debugger::setup()
    {
        if (!m_IsEnabled) 
            return;

        setupInstanceDebugUtils();
        setupDeviceDebugUtils();
    }


    void Debugger::destroy()
    {
        if (!m_IsEnabled) 
            return;

        if (m_InstanceDebugUtils.destroyDebugUtilsMessengerEXT)
        {
            m_InstanceDebugUtils.destroyDebugUtilsMessengerEXT(VyContext::instance(), m_InstanceDebugUtils.debugUtilsMessenger, nullptr);
        }
    }


    void Debugger::setDebugObjectName(U64 objectHandle, VkObjectType objectType, const String& name)
    {
        if (!m_IsEnabled)
            return;

        VkDebugUtilsObjectNameInfoEXT nameInfo{};
        {
            nameInfo.sType        = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
            nameInfo.objectType   = objectType;
            nameInfo.objectHandle = objectHandle;
            nameInfo.pObjectName  = name.c_str();
        }

        m_DeviceDebugUtils.setDebugUtilsObjectNameEXT(VyContext::device(), &nameInfo);
    }


    void Debugger::beginDebugLabel(VkCommandBuffer& cmdBuffer, const String& name, const Vec4& color)
    {
        if (!m_IsEnabled)
            return;

        VkDebugUtilsLabelEXT labelInfo{};
        {
            labelInfo.sType      = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
            labelInfo.pLabelName = name.c_str();
            labelInfo.color[0]   = color.x;
            labelInfo.color[1]   = color.y;
            labelInfo.color[2]   = color.z;
            labelInfo.color[3]   = color.w;
        }

        m_DeviceDebugUtils.cmdBeginDebugUtilsLabelEXT(cmdBuffer, &labelInfo);
    }


    void Debugger::insertDebugLabel(VkCommandBuffer& cmdBuffer, const String& name, const Vec4& color)
    {
        if (!m_IsEnabled)
            return;

        VkDebugUtilsLabelEXT labelInfo{};
        {
            labelInfo.sType      = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
            labelInfo.pLabelName = name.c_str();
            labelInfo.color[0]   = color.x;
            labelInfo.color[1]   = color.y;
            labelInfo.color[2]   = color.z;
            labelInfo.color[3]   = color.w;
        }

        m_DeviceDebugUtils.cmdInsertDebugUtilsLabelEXT(cmdBuffer, &labelInfo);
    }


    void Debugger::endDebugLabel(VkCommandBuffer& cmdBuffer)
    {
        if (!m_IsEnabled)
            return;

        m_DeviceDebugUtils.cmdEndDebugUtilsLabelEXT(cmdBuffer);
    }


    //--------------------------------------------------
    //    Accessors & Mutators
    //--------------------------------------------------
    void Debugger::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
    {
        createInfo = { VKInit::debugUtilsMessengerCreateInfoEXT() };
        {
            createInfo.messageSeverity = {
                  VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
                | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
                | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT 
            };
            createInfo.messageType = { 
                  VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
                | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
                | VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT
                | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT 
            };
            createInfo.pfnUserCallback = DebugCallback;
            createInfo.pUserData       = nullptr;
        }
    }


    bool Debugger::checkValidationLayerSupport()
    {
        U32 layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        TVector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        for (const char* layerName : m_vValidationLayers)
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
                return false;
        }

        return true;
    }


    bool Debugger::isEnabled() 
    { 
        return m_IsEnabled; 
    }


    const TVector<const char*>& Debugger::getValidationLayers()	
    { 
        return m_vValidationLayers; 
    }


    U32 Debugger::getNumberOfLayers() 
    { 
        return static_cast<U32>(m_vValidationLayers.size()); 
    }

    //--------------------------------------------------
    //    Private
    //--------------------------------------------------
    VkBool32 Debugger::DebugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT             messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void*)
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
			VY_DEBUG(ss.str());
        } 
		else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) 
		{
			VY_INFO(ss.str());
        } 
		else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) 
		{
			VY_WARN(ss.str());
        } 
		else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) 
		{
			VY_ERROR(ss.str());
        }
        else
        {
            VY_TRACE(ss.str());
        }

        return VK_FALSE;
    }


    void Debugger::setupInstanceDebugUtils()
    {
        VkDebugUtilsMessengerCreateInfoEXT createInfo{};
        populateDebugMessengerCreateInfo(createInfo);

        m_InstanceDebugUtils.createDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>
            (vkGetInstanceProcAddr(VyContext::instance(), "vkCreateDebugUtilsMessengerEXT"));

        m_InstanceDebugUtils.destroyDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>
            (vkGetInstanceProcAddr(VyContext::instance(), "vkDestroyDebugUtilsMessengerEXT"));

        if (m_InstanceDebugUtils.createDebugUtilsMessengerEXT != nullptr)
        {
            m_InstanceDebugUtils.createDebugUtilsMessengerEXT(VyContext::instance(), &createInfo, nullptr, &m_InstanceDebugUtils.debugUtilsMessenger);
        }
        else
        {
            throw std::runtime_error("Failed to find vkCreateDebugUtilsMessengerEXT!");
        }
    }


    void Debugger::setupDeviceDebugUtils()
    {
        m_DeviceDebugUtils.setDebugUtilsObjectNameEXT = reinterpret_cast<PFN_vkSetDebugUtilsObjectNameEXT>
            (vkGetDeviceProcAddr(VyContext::device(), "vkSetDebugUtilsObjectNameEXT"));

        m_DeviceDebugUtils.cmdBeginDebugUtilsLabelEXT = reinterpret_cast<PFN_vkCmdBeginDebugUtilsLabelEXT>
            (vkGetDeviceProcAddr(VyContext::device(), "vkCmdBeginDebugUtilsLabelEXT"));

        m_DeviceDebugUtils.cmdEndDebugUtilsLabelEXT = reinterpret_cast<PFN_vkCmdEndDebugUtilsLabelEXT>
            (vkGetDeviceProcAddr(VyContext::device(), "vkCmdEndDebugUtilsLabelEXT"));

        m_DeviceDebugUtils.cmdInsertDebugUtilsLabelEXT = reinterpret_cast<PFN_vkCmdInsertDebugUtilsLabelEXT>
            (vkGetDeviceProcAddr(VyContext::device(), "vkCmdInsertDebugUtilsLabelEXT"));
    }
}


// {
// 	const char* VkObjectTypeToString(VkObjectType ObjectType)
// 	{
// 		switch (ObjectType)
// 		{
// 			case VK_OBJECT_TYPE_UNKNOWN:                        return "Unknown";
// 			case VK_OBJECT_TYPE_INSTANCE:                       return "VkInstance";
// 			case VK_OBJECT_TYPE_PHYSICAL_DEVICE:                return "VkPhysicalDevice";
// 			case VK_OBJECT_TYPE_DEVICE:                         return "VkDevice";
// 			case VK_OBJECT_TYPE_QUEUE:                          return "VkQueue";
// 			case VK_OBJECT_TYPE_SEMAPHORE:                      return "VkSemaphore";
// 			case VK_OBJECT_TYPE_COMMAND_BUFFER:                 return "VkCommandBuffer";
// 			case VK_OBJECT_TYPE_FENCE:                          return "VkFence";
// 			case VK_OBJECT_TYPE_DEVICE_MEMORY:                  return "VkMemory";
// 			case VK_OBJECT_TYPE_BUFFER:                         return "VkBuffer";
// 			case VK_OBJECT_TYPE_IMAGE:                          return "VkImage";
// 			case VK_OBJECT_TYPE_EVENT:                          return "VkEvent";
// 			case VK_OBJECT_TYPE_QUERY_POOL:                     return "VkQueryPool";
// 			case VK_OBJECT_TYPE_BUFFER_VIEW:                    return "VkBufferView";
// 			case VK_OBJECT_TYPE_IMAGE_VIEW:                     return "VkImageView";
// 			case VK_OBJECT_TYPE_SHADER_MODULE:                  return "VkShaderModule";
// 			case VK_OBJECT_TYPE_PIPELINE_CACHE:                 return "VkPipelineCache";
// 			case VK_OBJECT_TYPE_PIPELINE_LAYOUT:                return "VkPipelineLayout";
// 			case VK_OBJECT_TYPE_RENDER_PASS:                    return "VkRenderPass";
// 			case VK_OBJECT_TYPE_PIPELINE:                       return "VkPipeline";
// 			case VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT:          return "VkDescriptorSetLayout";
// 			case VK_OBJECT_TYPE_SAMPLER:                        return "VkSampler";
// 			case VK_OBJECT_TYPE_DESCRIPTOR_POOL:                return "VkDescriptorPool";
// 			case VK_OBJECT_TYPE_DESCRIPTOR_SET:                 return "VkDescriptorSet";
// 			case VK_OBJECT_TYPE_FRAMEBUFFER:                    return "VkFramebuffer";
// 			case VK_OBJECT_TYPE_COMMAND_POOL:                   return "VkCommandPool";
// 			case VK_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION:       return "VkSamplerYcbcrConversion";
// 			case VK_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE:     return "VkDescriptorUpdateTemplate";
// 			case VK_OBJECT_TYPE_SURFACE_KHR:                    return "VkSurfaceKHR";
// 			case VK_OBJECT_TYPE_SWAPCHAIN_KHR:                  return "VkSwapchainKHR";
// 			case VK_OBJECT_TYPE_DISPLAY_KHR:                    return "VkDisplayKHR";
// 			case VK_OBJECT_TYPE_DISPLAY_MODE_KHR:               return "VkDisplayModeKHR";
// 			case VK_OBJECT_TYPE_DEBUG_REPORT_CALLBACK_EXT:      return "VkDebugReportCallback";
// 			case VK_OBJECT_TYPE_DEBUG_UTILS_MESSENGER_EXT:      return "VkDebugUtilsMessenger";
// 			case VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_KHR:     return "VkAccelerationStructureKHR";
// 			case VK_OBJECT_TYPE_VALIDATION_CACHE_EXT:           return "VkValidationCacheEXT";
// 			case VK_OBJECT_TYPE_PERFORMANCE_CONFIGURATION_INTEL:return "VkPerformanceConfigurationINTEL";
// 			case VK_OBJECT_TYPE_DEFERRED_OPERATION_KHR:         return "VkDeferredOperationKHR";
// 			case VK_OBJECT_TYPE_INDIRECT_COMMANDS_LAYOUT_NV:    return "VkIndirectCommandsLayoutNV";
// 			case VK_OBJECT_TYPE_PRIVATE_DATA_SLOT_EXT:          return "VkPrivateDataSlotEXT";
// 			default:                                            return "Unknown";
// 		}
// 	}


// 	void DebugUtilsMessenger::create()
// 	{
// 		// if constexpr (VY_ENABLE_VALIDATION)
// 		// {
// 			VkDebugUtilsMessengerCreateInfoEXT createInfo = populateCreateInfo();

// 			VK_CHECK(vkCreateDebugUtilsMessengerEXT(
// 				VyContext::instance(), 
// 				std::addressof(createInfo), 
// 				nullptr, 
// 				std::addressof(m_DebugMessenger)
// 			));
// 		// }
// 	}


// 	void DebugUtilsMessenger::destroy()
// 	{
// 		// if (VY_ENABLE_VALIDATION)
// 		// {
// 			vkDestroyDebugUtilsMessengerEXT(VyContext::instance(), m_DebugMessenger, nullptr);
// 		// }
// 	}


// 	VkDebugUtilsMessengerCreateInfoEXT DebugUtilsMessenger::populateCreateInfo()
// 	{
// 		VkDebugUtilsMessengerCreateInfoEXT info{ VKInit::debugUtilsMessengerCreateInfoEXT() };
// 		{
// 			info.messageSeverity = {
// 				// VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT    |
// 				VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
// 				VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT 
// 			};
// 			info.messageType = {
// 				VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT     |
// 				VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT  |
// 				VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT 
// 			};
// 			info.pfnUserCallback = debugCallback;
// 		}

// 		return info;
// 	}
	

// 	VkBool32 DebugUtilsMessenger::debugCallback(
//         VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity, 
// 		VkDebugUtilsMessageTypeFlagsEXT             messageType, 
//         const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, 
// 		void*                                       pUserData)
// 	{

// 		std::stringstream debugMessage;
// 		debugMessage << "(";

// 		if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT)
// 		{
// 			debugMessage << "GENERAL";

// 			if (messageType & (VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT))
// 			{
// 				debugMessage << ", ";
// 			}
// 		}

// 		if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT)
// 		{
// 			debugMessage << "VALIDATION";
		
// 			if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT)
// 			{
// 				debugMessage << ", ";
// 			}
// 		}

// 		if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT)
// 		{
// 			debugMessage << "PERFORMANCE";
// 		}

// 		debugMessage << "): ";

// 		debugMessage << (pCallbackData->pMessageIdName != nullptr ? pCallbackData->pMessageIdName : "<Unknown name>");
		
// 		if (pCallbackData->pMessage != nullptr)
// 		{
// 			debugMessage << std::endl
//   						 << "                " << pCallbackData->pMessage;
// 		}

// 		if (pCallbackData->objectCount > 0)
// 		{
// 			for (U32 obj = 0; obj < pCallbackData->objectCount; ++obj)
// 			{
// 				const VkDebugUtilsObjectNameInfoEXT& Object = pCallbackData->pObjects[obj];

// 				debugMessage << std::endl
// 							 << "                Object[" << obj << "] (" << VkObjectTypeToString(Object.objectType)
// 							 << "): Handle - " << std::hex << "0x" << Object.objectHandle;
							
// 				if (Object.pObjectName != nullptr)
// 				{
// 					debugMessage << ", Name: '" << Object.pObjectName << '\'';
// 				}
// 			}
// 		}

// 		if (pCallbackData->cmdBufLabelCount > 0)
// 		{
// 			for (U32 l = 0; l < pCallbackData->cmdBufLabelCount; ++l)
// 			{
// 				const VkDebugUtilsLabelEXT& Label = pCallbackData->pCmdBufLabels[l];

// 				debugMessage << std::endl
// 							 << "                Label[" << l << "]";
// 				if (Label.pLabelName != nullptr)
// 				{
// 					debugMessage << " - '" << Label.pLabelName << "'";
// 				}

// 				debugMessage << " { ";
// 				debugMessage << std::fixed << std::setw(4) << Label.color[0] << ", "
// 							 << std::fixed << std::setw(4) << Label.color[1] << ", "
// 							 << std::fixed << std::setw(4) << Label.color[2] << ", "
// 							 << std::fixed << std::setw(4) << Label.color[3] 
// 							 << " }";
// 			}
// 		}

// 		if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
// 		{
// 			VY_TRACE_TAG("Vulkan Debug", "{}", debugMessage.str().c_str());
// 		}
// 		else if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
// 		{
// 			VY_INFO_TAG("Vulkan Debug", "{}", debugMessage.str().c_str());
// 		}
// 		else if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
// 		{
// 			VY_WARN_TAG("Vulkan Debug", "{}", debugMessage.str().c_str());
// 		}
// 		else if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
// 		{
// 			VY_ERROR_TAG("Vulkan Debug", "{}", debugMessage.str().c_str());
// 		}
// 		else
// 		{
// 			VY_ERROR_TAG("Vulkan Debug", "[Unknown Log Type] {}", debugMessage.str().c_str());
// 		}

// 		// The return value of this callback controls whether the Vulkan call that caused
// 		// the validation message will be aborted or not
// 		// We return VK_FALSE as we DON'T want Vulkan calls that cause a validation message
// 		// (and return a VkResult) to abort
// 		return VK_FALSE;
// 	}
// }