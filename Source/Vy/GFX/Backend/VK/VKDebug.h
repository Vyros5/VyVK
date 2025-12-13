#pragma once

#include <Vy/GFX/Backend/VK/VKCore.h>

namespace Vy
{
	struct InstanceDebugUtils
	{
		VkDebugUtilsMessengerEXT            debugUtilsMessenger{ VK_NULL_HANDLE };

		PFN_vkCreateDebugUtilsMessengerEXT	createDebugUtilsMessengerEXT { nullptr };
		PFN_vkDestroyDebugUtilsMessengerEXT destroyDebugUtilsMessengerEXT{ nullptr };
	};


	struct DeviceDebugUtils
	{
		PFN_vkSetDebugUtilsObjectNameEXT  setDebugUtilsObjectNameEXT { nullptr };
		PFN_vkCmdBeginDebugUtilsLabelEXT  cmdBeginDebugUtilsLabelEXT { nullptr };
		PFN_vkCmdEndDebugUtilsLabelEXT	  cmdEndDebugUtilsLabelEXT   { nullptr };
		PFN_vkCmdInsertDebugUtilsLabelEXT cmdInsertDebugUtilsLabelEXT{ nullptr };
	};


	class Debugger final
	{
	public:
		//--------------------------------------------------
		//    Setup
		//--------------------------------------------------
		static void setEnabled(bool enabled);
		static void addValidationLayer(const char* layer);
		static void setup();
		static void destroy();

		static void setDebugObjectName(U64 objectHandle, VkObjectType objectType, const String& name);
		static void beginDebugLabel(VkCommandBuffer & cmdBuffer, const String& name, const Vec4& color);
		static void insertDebugLabel(VkCommandBuffer& cmdBuffer, const String& name, const Vec4& color);
		static void endDebugLabel(VkCommandBuffer& cmdBuffer);

		//--------------------------------------------------
		//    Accessors & Mutators
		//--------------------------------------------------
		static void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
		static bool checkValidationLayerSupport();

		static bool isEnabled();

		static const TVector<const char*>& getValidationLayers();
		static U32 getNumberOfLayers();

	private:
		static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
															VkDebugUtilsMessageTypeFlagsEXT messageType,
															const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
															void* pUserData);

		static void setupInstanceDebugUtils();
		static void setupDeviceDebugUtils();

		inline static bool                 m_IsEnabled{ false };
		inline static TVector<const char*> m_vValidationLayers{ };

		inline static InstanceDebugUtils m_InstanceDebugUtils{ };
		inline static DeviceDebugUtils   m_DeviceDebugUtils{ };
	};
}


// {
// 	class DebugUtilsMessenger
// 	{
// 	public:
// 		DebugUtilsMessenger()  = default;
// 		~DebugUtilsMessenger() = default;

// 		void create();
// 		void destroy();

// 		static VkBool32 debugCallback(
// 			VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
// 			VkDebugUtilsMessageTypeFlagsEXT             messageType, 
// 			const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
// 			void*                                       pUserData
// 		);

//         /**
//          * @brief Populates the debug messenger create info structure.
//          *
//          * This function populates and returns the debug messenger create info structure with the required parameters.
//          */
// 		static VkDebugUtilsMessengerCreateInfoEXT populateCreateInfo();

// 	private:
// 		VkDebugUtilsMessengerEXT m_DebugMessenger{VK_NULL_HANDLE};
// 	};
// }