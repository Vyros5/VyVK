#pragma once

#include <Vy/GFX/Backend/VK/VKCore.h>

namespace Vy
{
	// https://github.com/brammie15/Vovy/blob/master/src/Utils/DebugLabel.cpp
	// https://github.com/brammie15/Vovy/blob/master/src/Utils/DebugLabel.h

	class VyDebugLabel 
	{
	public:
		static void init();

		static bool isAvailable();

		class ScopedCmdLabel 
		{
		public:
			ScopedCmdLabel(VkCommandBuffer cmdBuffer, const String& name, const Vec4& color = Vec4(1.0f, 1.0f, 1.0f, 1.0f));

			~ScopedCmdLabel();

			ScopedCmdLabel(const ScopedCmdLabel&)            = delete;
			ScopedCmdLabel& operator=(const ScopedCmdLabel&) = delete;


		private:
			VkCommandBuffer m_CmdBuffer;
		};

		static void beginCmdLabel(VkCommandBuffer cmdBuffer, const String& name, Vec4 color);

		static void endCmdLabel(VkCommandBuffer cmdBuffer);

		static void setObjectName(U64 objectHandle, VkObjectType objectType, const String& name);

		static void nameBuffer(VkBuffer buffer, const String& name);

		static void nameImage(VkImage image, const String& name);

		static void nameCommandBuffer(VkCommandBuffer cmdBuffer, const String& name);

		static void nameAllocation(VmaAllocation allocation, const String& name);

	private:
		static inline PFN_vkCmdBeginDebugUtilsLabelEXT vkCmdBeginDebugUtilsLabelEXT = nullptr;
		static inline PFN_vkCmdEndDebugUtilsLabelEXT   vkCmdEndDebugUtilsLabelEXT = nullptr;
		static inline PFN_vkSetDebugUtilsObjectNameEXT vkSetDebugUtilsObjectNameEXT = nullptr;
		// static inline std::once_flag                   initFlag;
	};
}


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