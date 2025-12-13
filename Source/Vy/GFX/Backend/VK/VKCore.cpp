#define VOLK_IMPLEMENTATION

#include <Vy/GFX/Backend/VK/VKCore.h>

#include <Vy/GFX/Context.h>

#define VMA_IMPLEMENTATION
// #define VMA_STATIC_VULKAN_FUNCTIONS  0
// #define VMA_DYNAMIC_VULKAN_FUNCTIONS 0
#include <vma/vk_mem_alloc.h>

#include <Vy/Core/File/FileSystem.hpp>

#include <Vy/GFX/Resources/Texture.h>

namespace Vy
{

#pragma region [ VKUtil ]

	StringView VKUtil::resultString(VkResult result)
	{
		switch (result)
		{
#define CASE(name) case name: return #name
			CASE(VK_SUCCESS);
			CASE(VK_NOT_READY);
			CASE(VK_TIMEOUT);
			CASE(VK_EVENT_SET);
			CASE(VK_EVENT_RESET);
			CASE(VK_INCOMPLETE);
			CASE(VK_ERROR_OUT_OF_HOST_MEMORY);
			CASE(VK_ERROR_OUT_OF_DEVICE_MEMORY);
			CASE(VK_ERROR_INITIALIZATION_FAILED);
			CASE(VK_ERROR_DEVICE_LOST);
			CASE(VK_ERROR_MEMORY_MAP_FAILED);
			CASE(VK_ERROR_LAYER_NOT_PRESENT);
			CASE(VK_ERROR_EXTENSION_NOT_PRESENT);
			CASE(VK_ERROR_FEATURE_NOT_PRESENT);
			CASE(VK_ERROR_INCOMPATIBLE_DRIVER);
			CASE(VK_ERROR_TOO_MANY_OBJECTS);
			CASE(VK_ERROR_FORMAT_NOT_SUPPORTED);
			CASE(VK_ERROR_FRAGMENTED_POOL);
			CASE(VK_ERROR_UNKNOWN);
			CASE(VK_ERROR_OUT_OF_POOL_MEMORY);
			CASE(VK_ERROR_INVALID_EXTERNAL_HANDLE);
			CASE(VK_ERROR_FRAGMENTATION);
			CASE(VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS);
			CASE(VK_PIPELINE_COMPILE_REQUIRED);
			CASE(VK_ERROR_SURFACE_LOST_KHR);
			CASE(VK_ERROR_NATIVE_WINDOW_IN_USE_KHR);
			CASE(VK_SUBOPTIMAL_KHR);
			CASE(VK_ERROR_OUT_OF_DATE_KHR);
			CASE(VK_ERROR_INCOMPATIBLE_DISPLAY_KHR);
			CASE(VK_ERROR_VALIDATION_FAILED_EXT);
			CASE(VK_ERROR_INVALID_SHADER_NV);
			CASE(VK_ERROR_IMAGE_USAGE_NOT_SUPPORTED_KHR);
			CASE(VK_ERROR_VIDEO_PICTURE_LAYOUT_NOT_SUPPORTED_KHR);
			CASE(VK_ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR);
			CASE(VK_ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR);
			CASE(VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR);
			CASE(VK_ERROR_VIDEO_STD_VERSION_NOT_SUPPORTED_KHR);
			CASE(VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT);
			CASE(VK_ERROR_NOT_PERMITTED_KHR);
			CASE(VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT);
			CASE(VK_THREAD_IDLE_KHR);
			CASE(VK_THREAD_DONE_KHR);
			CASE(VK_OPERATION_DEFERRED_KHR);
			CASE(VK_OPERATION_NOT_DEFERRED_KHR);
			CASE(VK_ERROR_INVALID_VIDEO_STD_PARAMETERS_KHR);
			CASE(VK_ERROR_COMPRESSION_EXHAUSTED_EXT);
			CASE(VK_INCOMPATIBLE_SHADER_BINARY_EXT);
#undef CASE
		default:
			return "RESULT_UNKNOWN";
		}
	}


	I32 VKUtil::bytesPerPixel(VkFormat format)
	{
		switch (format) 
		{
			case VK_FORMAT_R8_UNORM:
			case VK_FORMAT_R8_SNORM:
			case VK_FORMAT_R8_UINT:
			case VK_FORMAT_R8_SINT:
			case VK_FORMAT_R8_SRGB:
				return 1; // 1 byte per pixel

			case VK_FORMAT_R8G8_UNORM:
			case VK_FORMAT_R8G8_SNORM:
			case VK_FORMAT_R8G8_UINT:
			case VK_FORMAT_R8G8_SINT:
			case VK_FORMAT_R8G8_SRGB:
				return 2; // 2 bytes per pixel

			case VK_FORMAT_R8G8B8_UNORM:
			case VK_FORMAT_R8G8B8_SNORM:
			case VK_FORMAT_R8G8B8_UINT:
			case VK_FORMAT_R8G8B8_SINT:
			case VK_FORMAT_R8G8B8_SRGB:
			case VK_FORMAT_B8G8R8_UNORM:
			case VK_FORMAT_B8G8R8_SNORM:
			case VK_FORMAT_B8G8R8_UINT:
			case VK_FORMAT_B8G8R8_SINT:
			case VK_FORMAT_B8G8R8_SRGB:
				return 3; // 3 bytes per pixel

			case VK_FORMAT_R8G8B8A8_UNORM:
			case VK_FORMAT_R8G8B8A8_SNORM:
			case VK_FORMAT_R8G8B8A8_UINT:
			case VK_FORMAT_R8G8B8A8_SINT:
			case VK_FORMAT_R8G8B8A8_SRGB:
			case VK_FORMAT_B8G8R8A8_UNORM:
			case VK_FORMAT_B8G8R8A8_SNORM:
			case VK_FORMAT_B8G8R8A8_UINT:
			case VK_FORMAT_B8G8R8A8_SINT:
			case VK_FORMAT_B8G8R8A8_SRGB:
				return 4; // 4 bytes per pixel

			case VK_FORMAT_R16_UNORM:
			case VK_FORMAT_R16_SNORM:
			case VK_FORMAT_R16_UINT:
			case VK_FORMAT_R16_SINT:
			case VK_FORMAT_R16_SFLOAT:
				return 2; // 2 bytes per pixel

			case VK_FORMAT_R16G16_UNORM:
			case VK_FORMAT_R16G16_SNORM:
			case VK_FORMAT_R16G16_UINT:
			case VK_FORMAT_R16G16_SINT:
			case VK_FORMAT_R16G16_SFLOAT:
				return 4; // 4 bytes per pixel

			case VK_FORMAT_R16G16B16_UNORM:
			case VK_FORMAT_R16G16B16_SNORM:
			case VK_FORMAT_R16G16B16_UINT:
			case VK_FORMAT_R16G16B16_SINT:
			case VK_FORMAT_R16G16B16_SFLOAT:
				return 6; // 6 bytes per pixel

			case VK_FORMAT_R16G16B16A16_UNORM:
			case VK_FORMAT_R16G16B16A16_SNORM:
			case VK_FORMAT_R16G16B16A16_UINT:
			case VK_FORMAT_R16G16B16A16_SINT:
			case VK_FORMAT_R16G16B16A16_SFLOAT:
				return 8; // 8 bytes per pixel

			case VK_FORMAT_R32_SFLOAT:
				return 4; // 4 bytes per pixel

			case VK_FORMAT_R32G32_SFLOAT:
				return 8; // 8 bytes per pixel

			case VK_FORMAT_R32G32B32_SFLOAT:
				return 12; // 12 bytes per pixel

			case VK_FORMAT_R32G32B32A32_SFLOAT:
				return 16; // 16 bytes per pixel

			default:
				return -1; // Unsupported format
		}
	}
	
	// Source layouts (old)
	// Source Access Mask: Controls actions that have to be finished on the old layout 
	// before it will be transitioned to the new layout.
	
	// Target layouts (new)
	// Destination Access Mask: Controls the dependency for the new image layout.


	//     | OldLayout       | NewLayout       | SrcAccess            | DstAccess            | SrcStage              | DstStage              |
	//-----|-----------------|-----------------|----------------------|----------------------|-----------------------|-----------------------|
	//  1. | Undefined       | TransferDst     | 0                    | TransferWrite        | Top                   | Transfer              |
	//  2. | TransferDst     | TransferDst     | TransferWrite        | ShaderRead           | Transfer              | FragmentShader        |
	//  3. | Undefined       | ColorAttachment | 0                    | ColorAttachmentWrite | Top                   | ColorAttachmentOutput |
	//  4. | ColorAttachment | TransferSrc     | ColorAttachmentWrite | TransferRead         | ColorAttachmentOutput | Transfer              |
	//  5. | TransferSrc     | ShaderRead      | TransferRead         | ShaderRead           | Transfer              | FragmentShader        |
	//  6. | ShaderRead      | ColorAttachment | ShaderRead           | ColorAttachmentWrite | FragmentShader        | ColorAttachmentOutput |
	//  7. | TransferDst     | TransferSrc     | TransferWrite        | TransferRead         | Transfer              | Transfer              |
	//  8. | ColorAttachment | ShaderRead      | ColorAttachmentWrite | ShaderRead           | ColorAttachmentOutput | FragmentShader        |
	//  9. | ColorAttachment | TransferDst     | ColorAttachmentWrite | TransferRead         | ColorAttachmentOutput | Transfer              |
	// 10. | TransferDst     | ColorAttachment | TransferWrite        | ColorAttachmentWrite | Transfer              | ColorAttachmentOutput |
	// 11. | TransferSrc     | ColorAttachment | TransferRead         | ColorAttachmentWrite | Transfer              | ColorAttachmentOutput |
	// 12. | ColorAttachment | TransferDst     | ColorAttachmentWrite | TransferWrite        | ColorAttachmentOutput | Transfer              |

	// TransferWriteAccess -> TransferStage
	// TransferReadAccess  -> TransferStage

	//  1. -------------------------------------------------------------------------
	// OldLayout: Undefined   
	// NewLayout: TransferDst 
	// SrcAccess: 0
	// DstAccess: TransferWrite
	// SrcStage:  Top
	// DstStage:  Transfer
	//  2. -------------------------------------------------------------------------
	// OldLayout: TransferDst
	// NewLayout: ShaderRead
	// SrcAccess: TransferWrite
	// DstAccess: ShaderRead
	// SrcStage:  Transfer
	// DstStage:  FragmentShader
	//  3. -------------------------------------------------------------------------
	// OldLayout: Undefined
	// NewLayout: ColorAttachment
	// SrcAccess: 0
	// DstAccess: ColorAttachmentWrite
	// SrcStage:  Top
	// DstStage:  ColorAttachmentOutput
	//  4. -------------------------------------------------------------------------
	// OldLayout: ColorAttachment
	// NewLayout: TransferSrc
	// SrcAccess: ColorAttachmentWrite
	// DstAccess: TransferRead
	// SrcStage:  ColorAttachmentOutput
	// DstStage:  Transfer
	//  5. -------------------------------------------------------------------------
	// OldLayout: TransferSrc
	// NewLayout: ShaderRead
	// SrcAccess: TransferRead
	// DstAccess: ShaderRead
	// SrcStage:  Transfer
	// DstStage:  FragmentShader
	//  6. -------------------------------------------------------------------------
	// OldLayout: ShaderRead
	// NewLayout: ColorAttachment
	// SrcAccess: ShaderRead
	// DstAccess: ColorAttachmentWrite
	// SrcStage:  FragmentShader
	// DstStage:  ColorAttachmentOutput
	//  7. -------------------------------------------------------------------------
	// OldLayout: TransferDst
	// NewLayout: TransferSrc
	// SrcAccess: TransferWrite
	// DstAccess: TransferRead
	// SrcStage:  Transfer
	// DstStage:  Transfer
	//  8. -------------------------------------------------------------------------
	// OldLayout: ColorAttachment
	// NewLayout: ShaderRead
	// SrcAccess: ColorAttachmentWrite
	// DstAccess: ShaderRead
	// SrcStage:  ColorAttachmentOutput
	// DstStage:  FragmentShader
	//  9. -------------------------------------------------------------------------
	// OldLayout: ColorAttachment
	// NewLayout: TransferDst
	// SrcAccess: ColorAttachmentWrite
	// DstAccess: TransferRead
	// SrcStage:  ColorAttachmentOutput
	// DstStage:  Transfer
	// 10. -------------------------------------------------------------------------
	// OldLayout: TransferDst
	// NewLayout: ColorAttachment
	// SrcAccess: TransferWrite
	// DstAccess: ColorAttachmentWrite 
	// SrcStage:  Transfer
	// DstStage:  ColorAttachmentOutput 
	// 11. -------------------------------------------------------------------------
	// OldLayout: TransferSrc
	// NewLayout: ColorAttachment
	// SrcAccess: TransferRead
	// DstAccess: ColorAttachmentWrite 
	// SrcStage:  Transfer
	// DstStage:  ColorAttachmentOutput 
	// 12. -------------------------------------------------------------------------
	// OldLayout: ColorAttachment
	// NewLayout: TransferDst
	// SrcAccess: ColorAttachmentWrite
	// DstAccess: TransferWrite
	// SrcStage:  ColorAttachmentOutput 
	// DstStage:  Transfer
	// -------------------------------------------------------------------------


	VkAccessFlags VKUtil::srcAccessMask(VkImageLayout layout)
    {
		switch (layout)
		{
		// Undefined
		case VK_IMAGE_LAYOUT_UNDEFINED:
			return 0;

		// General
		case VK_IMAGE_LAYOUT_GENERAL:
			// Image is used as a general image
			// Make sure any writes to the image have been finished.
			return VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;

		// ColorAttachment
		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
			// Image is a color attachment.
			// Make sure any writes to the color buffer have been finished.
			return VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		// DepthStencilAttachment
		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
			// Image is a depth/stencil attachment.
			// Make sure any writes to the depth/stencil buffer have been finished.
			return VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		// ShaderRead
		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
			// Image is read by a shader.
			// Make sure any shader reads from the image have been finished.
			return VK_ACCESS_SHADER_READ_BIT;

		// PresentSrc
		case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
			return 0;

		// TransferSrc
		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
			// Image is a transfer source.
			// Make sure any reads from the image have been finished.
			return VK_ACCESS_TRANSFER_READ_BIT; 

		// TransferDst
		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			// Image is a transfer destination
			// Make sure any writes to the image have been finished.
			return VK_ACCESS_TRANSFER_WRITE_BIT;

		default:
			VY_ASSERT(false, "Unsupported layout transition");
			return 0;
		}
    }

	VkAccessFlags VKUtil::dstAccessMask(VkImageLayout layout)
    {
		switch (layout)
		{
		// Undefined
		case VK_IMAGE_LAYOUT_UNDEFINED:
			return 0;

		// General
		case VK_IMAGE_LAYOUT_GENERAL:
			// Image will be used as a general image.
			// Make sure any writes to the image have been finished.
			return VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;

		// ColorAttachment
		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
			// Image will be used as a color attachment.
			// Make sure any writes to the color buffer have been finished.
			return VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;

		// DepthStencilAttachment
		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
			// Image layout will be used as a depth/stencil attachment.
			// Make sure any writes to depth/stencil buffer have been finished.
			return VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;

		// ShaderRead
		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
			// Image will be read in a shader (sampler, input attachment).
			return VK_ACCESS_SHADER_READ_BIT;

		// PresentSrc
		case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
			return 0;

		// TransferSrc
		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
			// Image will be used as a transfer source.
			// Make sure any reads to the image have been finished.
			return VK_ACCESS_TRANSFER_READ_BIT;

		// TransferDst
		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			// Image will be used as a transfer destination.
			// Make sure any writes to the image have been finished.
			return VK_ACCESS_TRANSFER_WRITE_BIT;

		default:
			VY_ASSERT(false, "Unsupported layout transition");
			return 0;
		}
    }

	/**
	 * VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT         VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
	 * VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT
	 * VK_ACCESS_SHADER_WRITE_BIT                   VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT
	 * VK_ACCESS_SHADER_READ_BIT                    VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT
	 * VK_ACCESS_TRANSFER_READ_BIT                  VK_PIPELINE_STAGE_TRANSFER_BIT
	 * VK_ACCESS_TRANSFER_WRITE_BIT                 VK_PIPELINE_STAGE_TRANSFER_BIT
	 */
	VkPipelineStageFlags VKUtil::srcStage(VkAccessFlags access)
    {
		// ColorAttachmentWrite -> ColorAttachmentOutput
		if (access & VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT)
		{
			return VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		}

		// DepthStencilAttachmentWrite -> LateFragmentTests
		if (access & VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT)
		{
			return VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		}

		// ShaderWrite -> ComputeShader
		if (access & VK_ACCESS_SHADER_WRITE_BIT)
		{
			return VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
		}

		// ShaderRead -> FragmentShader
		if (access & VK_ACCESS_SHADER_READ_BIT)
		{
			return VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}

		// TransferRead -> Transfer
		if (access & VK_ACCESS_TRANSFER_READ_BIT)
		{
			return VK_PIPELINE_STAGE_TRANSFER_BIT;
		}

		// TransferWrite -> Transfer
		if (access & VK_ACCESS_TRANSFER_WRITE_BIT)
		{
			return VK_PIPELINE_STAGE_TRANSFER_BIT;
		}

		// Top
		return VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    }

	/**
	 * VK_ACCESS_COLOR_ATTACHMENT_READ_BIT          VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
	 * VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT  VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT
	 * VK_ACCESS_SHADER_WRITE_BIT                   VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT
	 * VK_ACCESS_SHADER_READ_BIT                    VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT
	 * VK_ACCESS_TRANSFER_READ_BIT                  VK_PIPELINE_STAGE_TRANSFER_BIT
	 * VK_ACCESS_TRANSFER_WRITE_BIT                 VK_PIPELINE_STAGE_TRANSFER_BIT
	 */
	VkPipelineStageFlags VKUtil::dstStage(VkAccessFlags access)
    {
		// ColorAttachmentRead -> ColorAttachmentOutput
		if (access & VK_ACCESS_COLOR_ATTACHMENT_READ_BIT)
		{
			return VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		}

		// DepthStencilAttachmentRead -> EarlyFragmentTests
		if (access & VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT)
		{
			return VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		}

		// ShaderWrite -> ComputeShader
		if (access & VK_ACCESS_SHADER_WRITE_BIT)
		{
			return VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
		}

		// ShaderRead -> FragmentShader
		if (access & VK_ACCESS_SHADER_READ_BIT)
		{
			return VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}

		// TransferRead -> Transfer
		if (access & VK_ACCESS_TRANSFER_READ_BIT)
		{
			return VK_PIPELINE_STAGE_TRANSFER_BIT;
		}

		// TransferWrite -> Transfer
		if (access & VK_ACCESS_TRANSFER_WRITE_BIT)
		{
			return VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		
		// Bottom
		return VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    }


	VkImageAspectFlags VKUtil::aspectFlags(VkFormat format)
    {
		switch (format)
		{
		case VK_FORMAT_D16_UNORM:
		case VK_FORMAT_X8_D24_UNORM_PACK32:
		case VK_FORMAT_D32_SFLOAT:
        {
            return VK_IMAGE_ASPECT_DEPTH_BIT;
        }

		case VK_FORMAT_S8_UINT:
        {
            return VK_IMAGE_ASPECT_STENCIL_BIT;
        }

		case VK_FORMAT_D16_UNORM_S8_UINT:
		case VK_FORMAT_D24_UNORM_S8_UINT:
		case VK_FORMAT_D32_SFLOAT_S8_UINT:
        {
            return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
        }

		default:
			return VK_IMAGE_ASPECT_COLOR_BIT;  // For all color formats
		}
    }


	String VKUtil::printExtensionsList(const TVector<VkExtensionProperties>& extensions, size_t numColumns)
	{
		assert(numColumns > 0);

		TVector<String> extStrings;
		extStrings.reserve(extensions.size());
		for (const VkExtensionProperties& ext : extensions)
		{
			std::stringstream ss;
			ss 
			<< ext.extensionName                     << ' '
			<< VK_API_VERSION_MAJOR(ext.specVersion) << '.'
			<< VK_API_VERSION_MINOR(ext.specVersion) << '.'
			<< VK_API_VERSION_PATCH(ext.specVersion);

			extStrings.emplace_back(ss.str());
		}

		TVector<size_t> colWidth(numColumns);
		for (size_t i = 0; i < extensions.size();)
		{
			for (size_t col = 0; col < numColumns && i < extensions.size(); ++col, ++i)
			{
				colWidth[col] = std::max(colWidth[col], extStrings[i].length());
			}
		}

		std::stringstream ss;
		for (size_t i = 0; i < extensions.size();)
		{
			for (size_t col = 0; col < numColumns && i < extensions.size(); ++col, ++i)
			{
				ss << (col == 0 ? "\n    " : "    ");

				if (col + 1 < numColumns && i + 1 < extensions.size())
				{
					ss << std::setw(static_cast<int>(colWidth[col])) << std::left;
				}

				ss << extStrings[i];
			}
		}

		return ss.str();
	}


	VkRenderingAttachmentInfo VKUtil::renderingAttachmentInfo(VkImageView imageView, VkImageLayout layout, VkAttachmentLoadOp loadOp, VkClearValue clearValue)
    {
		VkRenderingAttachmentInfo attachment{};
        {
            attachment.sType       = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
            attachment.imageView   = imageView;
            attachment.imageLayout = layout;
            attachment.loadOp      = loadOp;
            attachment.storeOp     = VK_ATTACHMENT_STORE_OP_STORE;
            attachment.clearValue  = clearValue;
        }

        return attachment;
    }
	

	// VkRenderingAttachmentInfo VKUtil::renderingAttachmentInfo(const VyTexture& texture, VkAttachmentLoadOp loadOp, VkClearValue clearValue)
    // {
    //     return renderingAttachmentInfo(texture.view(), texture.image().layout(), loadOp, clearValue);
    // }


	VkShaderModule VKUtil::createShaderModule(VkDevice device, const TVector<char>& code)
    {
		VkShaderModuleCreateInfo createInfo{ VKInit::shaderModuleCreateInfo() };
        {
            createInfo.codeSize = code.size();
            createInfo.pCode    = reinterpret_cast<const U32*>(code.data());
        }

		VkShaderModule shaderModule = nullptr;
		VK_CHECK(vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule));

		return shaderModule;
    }


	VkShaderModule VKUtil::createShaderModule(VkDevice device, const Path& path)
    {
		auto code = FileSystem::readBinary(path);

		if (code.empty())
        {
            VY_FATAL("Failed to read shader file: {}", path.string());
        }

		VY_ASSERT(!code.empty(), "Shader code is empty");

		return createShaderModule(device, code);
	}


	VkPipelineShaderStageCreateInfo VKUtil::createShaderStage(VkShaderStageFlagBits stage, VkShaderModule module, const char* entryPoint)
    {
		VkPipelineShaderStageCreateInfo shaderStage{ VKInit::pipelineShaderStageCreateInfo() };
        {
            shaderStage.stage               = stage;
            shaderStage.module              = module;
            shaderStage.pName               = entryPoint;
            shaderStage.flags               = 0;
            shaderStage.pNext               = nullptr;
            shaderStage.pSpecializationInfo = nullptr;
        }

        return shaderStage;
    }


    SwapchainSupportDetails VKUtil::getSwapchainSupport(const VkPhysicalDevice& device, const VkSurfaceKHR& surface)
    {
        SwapchainSupportDetails details{};
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.Capabilities);

        U32 formatCount{};
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
        if (formatCount != 0) 
        {
            details.Formats.resize(formatCount);

            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.Formats.data());
        }

        U32 presentModeCount{};
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
        if (presentModeCount != 0) 
        {
            details.PresentModes.resize(presentModeCount);

            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.PresentModes.data());
        }

        return details;
    }


    bool VKUtil::hasGraphicsQueue(const VkQueueFamilyProperties& queueFamily) 
    {
        return queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT;
    }


    bool VKUtil::hasComputeQueue(const VkQueueFamilyProperties& queueFamily) 
    {
        return queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT;
    }


    bool VKUtil::hasPresentQueue(
        const VkPhysicalDevice&        device, 
        const U32                      queueFamilyIndex, 
        const VkSurfaceKHR&            surface, 
        const VkQueueFamilyProperties& queueFamilyProperties) 
    {
        VkBool32 presentSupport{ VK_FALSE };
        if (vkGetPhysicalDeviceSurfaceSupportKHR(device, queueFamilyIndex, surface, &presentSupport) != VK_SUCCESS) 
        {
            VY_ERROR("VKUtil::hasPresentQueue - Failed to get Physical device surface support.");
            return false;
        }

        // Has present queues and at least one of them is available.
        if (queueFamilyProperties.queueCount > 0 && presentSupport == VK_TRUE) 
        {
            return true;
        }

        return false;
    }
}

#pragma endregion


#pragma region [ VKCmd ]

namespace Vy
{
    void VKCmd::copyBufferToImage(VkCommandBuffer cmdBuffer, VkBuffer buffer, VkImage image, VkExtent2D extent)
    {
		VkBufferImageCopy region{};
        {
            region.bufferOffset      = 0;
            region.bufferRowLength   = 0;
            region.bufferImageHeight = 0;
            
            region.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
            region.imageSubresource.mipLevel       = 0;
            region.imageSubresource.baseArrayLayer = 0;
            region.imageSubresource.layerCount     = 1;
            
            region.imageOffset = { 0, 0, 0 };
            region.imageExtent = { extent.width, extent.height, 1 };
        }

		vkCmdCopyBufferToImage(
            cmdBuffer,
			buffer,
			image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1,
			&region
        );
    }


    void VKCmd::copyBufferToImage(VkCommandBuffer cmdBuffer, VkBuffer buffer, VkImage image, VkExtent3D extent, U32 layerCount)
    {
		VkBufferImageCopy region{};
        {
            region.bufferOffset      = 0;
            region.bufferRowLength   = 0;
            region.bufferImageHeight = 0;
            
            region.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
            region.imageSubresource.mipLevel       = 0;
            region.imageSubresource.baseArrayLayer = 0;
            region.imageSubresource.layerCount     = layerCount;
            
            region.imageOffset = { 0, 0, 0 };
            region.imageExtent = extent;
        }

		vkCmdCopyBufferToImage(
            cmdBuffer,
			buffer,
			image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1,
			&region
        );
    }


    void VKCmd::dispatch(VkCommandBuffer cmdBuffer, VkExtent2D extent, VkExtent2D groupSize)
    {
		U32 groupCountX = (extent.width  + groupSize.width  - 1) / groupSize.width;
		U32 groupCountY = (extent.height + groupSize.height - 1) / groupSize.height;

		vkCmdDispatch(cmdBuffer, groupCountX, groupCountY, 1);
    }


    void VKCmd::dispatch(VkCommandBuffer cmdBuffer, VkExtent3D extent, VkExtent3D groupSize)
    {
		U32 groupCountX = (extent.width  + groupSize.width  - 1) / groupSize.width;
		U32 groupCountY = (extent.height + groupSize.height - 1) / groupSize.height;
		U32 groupCountZ = (extent.depth  + groupSize.depth  - 1) / groupSize.depth;

		vkCmdDispatch(cmdBuffer, groupCountX, groupCountY, groupCountZ);
    }


    void VKCmd::pipelineBarrier(
		VkCommandBuffer    cmdBuffer, 
		VkImage            image, 
		VkImageLayout      oldLayout, 
		VkImageLayout      newLayout, 
		VkImageAspectFlags aspectMask)
    {
		VkImageMemoryBarrier barrier{ VKInit::imageMemoryBarrier() };
        {
            barrier.oldLayout           = oldLayout;
            barrier.newLayout           = newLayout;
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.image               = image;
            
            barrier.subresourceRange.aspectMask     = aspectMask;
            barrier.subresourceRange.baseMipLevel   = 0;
            barrier.subresourceRange.levelCount     = 1;
            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.layerCount     = 1;
            
            barrier.srcAccessMask = VKUtil::srcAccessMask(oldLayout);
            barrier.dstAccessMask = VKUtil::dstAccessMask(newLayout);
        }

		vkCmdPipelineBarrier(
            cmdBuffer,
			VKUtil::srcStage(barrier.srcAccessMask), 
            VKUtil::dstStage(barrier.dstAccessMask),
			0,
			0, nullptr,
			0, nullptr,
			1, 
			&barrier
		);
    }


    void VKCmd::pipelineBarrier(
		VkCommandBuffer                      cmdBuffer, 
		VkPipelineStageFlags                 srcStage, 
		VkPipelineStageFlags                 dstStage, 
		const TVector<VkImageMemoryBarrier>& barriers)
    {
		if (barriers.empty())
			return;

		vkCmdPipelineBarrier(cmdBuffer,
			srcStage, dstStage,
			0,
			0, nullptr,
			0, nullptr,
			static_cast<U32>(barriers.size()), 
			barriers.data()
		);
    }


    void VKCmd::scissor(VkCommandBuffer cmdBuffer, VkExtent2D extent)
    {
		VkRect2D scissor{};
        {
            scissor.offset = { 0, 0 };
            scissor.extent = extent;
        }

        vkCmdSetScissor(cmdBuffer, 0, 1, &scissor);
    }
	

    void VKCmd::transitionImageLayout(
		VkCommandBuffer cmdBuffer, 
		VkImage         image, 
		VkFormat        format, 
		VkImageLayout   oldLayout, 
		VkImageLayout   newLayout, 
		U32             miplevels, 
		U32             layoutCount)
    {
		if (oldLayout == newLayout)
			return;

		VkImageMemoryBarrier barrier{ VKInit::imageMemoryBarrier() };
        {
            barrier.image               = image;

            barrier.oldLayout           = oldLayout;
            barrier.newLayout           = newLayout;
            
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

            barrier.subresourceRange.aspectMask     = VKUtil::aspectFlags(format);
            barrier.subresourceRange.baseMipLevel   = 0;
            barrier.subresourceRange.levelCount     = miplevels;
            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.layerCount     = layoutCount;

            barrier.srcAccessMask = VKUtil::srcAccessMask(barrier.oldLayout);
            barrier.dstAccessMask = VKUtil::dstAccessMask(barrier.newLayout);
        }

		VkPipelineStageFlags srcStage = VKUtil::srcStage(barrier.srcAccessMask);
		VkPipelineStageFlags dstStage = VKUtil::dstStage(barrier.dstAccessMask);

		vkCmdPipelineBarrier(cmdBuffer,
			srcStage, dstStage, 0,
			0, nullptr,
			0, nullptr,
			1, 
			&barrier
		);
    }


	void VKCmd::transitionImageLayout(
		VkCommandBuffer         cmdbuffer,
		VkImage                 image,
		VkImageLayout           oldImageLayout,
		VkImageLayout           newImageLayout,
		VkImageSubresourceRange subresourceRange,
		VkPipelineStageFlags    srcStageMask,
		VkPipelineStageFlags    dstStageMask,
		U32                     srcQueueFamilyIndex,
		U32                     dstQueueFamilyIndex)
	{
		VkImageMemoryBarrier barrier{ VKInit::imageMemoryBarrier() };
        {
            barrier.oldLayout           = oldImageLayout;
            barrier.newLayout           = newImageLayout;
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.image               = image;

            barrier.subresourceRange    = subresourceRange;

			barrier.srcQueueFamilyIndex = srcQueueFamilyIndex;
			barrier.dstQueueFamilyIndex = dstQueueFamilyIndex;
		}

        // Source layouts (old)
        // Source access mask controls actions that have to be finished on the old layout
        // before it will be transitioned to the new layout
        switch (oldImageLayout) 
		{
            case VK_IMAGE_LAYOUT_UNDEFINED:
                // Image layout is undefined (or does not matter)
                // Only valid as initial layout
                // No flags required, listed only for completeness
                barrier.srcAccessMask = 0;
                break;

            case VK_IMAGE_LAYOUT_PREINITIALIZED:
                // Image is preinitialized
                // Only valid as initial layout for linear images, preserves memory contents
                // Make sure host writes have been finished
                barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
                break;

            case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
                // Image is a color attachment
                // Make sure any writes to the color buffer have been finished
                barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                break;

            case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
                // Image is a depth/stencil attachment
                // Make sure any writes to the depth/stencil buffer have been finished
                barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                break;

            case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
                // Image is a transfer source
                // Make sure any reads from the image have been finished
                barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                break;

            case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
                // Image is a transfer destination
                // Make sure any writes to the image have been finished
                barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                break;

            case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
                // Image is read by a shader
                // Make sure any shader reads from the image have been finished
                barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
                break;
            default:
                // Other source layouts aren't handled (yet)
                break;
        }

        // Target layouts (new)
        // Destination access mask controls the dependency for the new image layout
        switch (newImageLayout) 
		{
            case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
                // Image will be used as a transfer destination
                // Make sure any writes to the image have been finished
                barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                break;

            case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
                // Image will be used as a transfer source
                // Make sure any reads from the image have been finished
                barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                break;

            case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
                // Image will be used as a color attachment
                // Make sure any writes to the color buffer have been finished
                barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                break;

            case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
                // Image layout will be used as a depth/stencil attachment
                // Make sure any writes to depth/stencil buffer have been finished
                barrier.dstAccessMask = barrier.dstAccessMask | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                break;

            case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
                // Image will be read in a shader (sampler, input attachment)
                // Make sure any writes to the image have been finished
                if (barrier.srcAccessMask == 0) 
				{
                    barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
                }
                barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
                break;
            default:
                // Other source layouts aren't handled (yet)
                break;
        }

        // Put barrier inside setup command buffer
        vkCmdPipelineBarrier(cmdbuffer,
            srcStageMask, dstStageMask, 0,
            0, nullptr,
            0, nullptr,
            1, 
			&barrier
        );
	}


    void VKCmd::viewport(VkCommandBuffer cmdBuffer, VkExtent2D extent)
    {
		VkViewport viewport{};
        {
            viewport.width    = static_cast<float>(extent.width);
            viewport.height   = static_cast<float>(extent.height);
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;
        }

		vkCmdSetViewport(cmdBuffer, 0, 1, &viewport);
    }


    void VKCmd::beginDebugUtilsLabel(VkCommandBuffer cmdBuffer, const char* label, const Vec4& color)
    {
		// if constexpr (VY_ENABLE_VALIDATION)
		// {
			VkDebugUtilsLabelEXT labelInfo{};
            {
                labelInfo.sType      = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
                labelInfo.pLabelName = label;
                labelInfo.color[0]   = color.r;
                labelInfo.color[1]   = color.g;
                labelInfo.color[2]   = color.b;
                labelInfo.color[3]   = color.a;
            }

            vkCmdBeginDebugUtilsLabelEXT(cmdBuffer, &labelInfo);
		// }
    }


    void VKCmd::endDebugUtilsLabel(VkCommandBuffer cmdBuffer)
    {
		// if constexpr (VY_ENABLE_VALIDATION)
		// {
			vkCmdEndDebugUtilsLabelEXT(cmdBuffer);
		// }
    }

#pragma endregion


#pragma region [ VKDbg ]

	void VKDbg::cmdBeginLabel(VkCommandBuffer cmdBuffer, const char* labelName, Vec4 color)
	{
	#ifdef VK_RENDERER_DEBUG
		VkDebugUtilsLabelEXT label = { VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT };
		label.pLabelName           = labelName;
		memcpy(label.color, &color, sizeof(float) * 4);

		vkCmdBeginDebugUtilsLabelEXT(cmdBuffer, &label);
	#endif
	}


	void VKDbg::cmdInsertLabel(VkCommandBuffer cmdBuffer, const char* labelName, Vec4 color)
	{
	#ifdef VK_RENDERER_DEBUG
		VkDebugUtilsLabelEXT label = { VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT };
		label.pLabelName           = labelName;
		memcpy(label.color, &color, sizeof(float) * 4);

		vkCmdInsertDebugUtilsLabelEXT(cmdBuffer, &label);
	#endif
	}


	void VKDbg::cmdEndLabel(VkCommandBuffer cmdBuffer)
	{
	#ifdef VK_RENDERER_DEBUG
		vkCmdEndDebugUtilsLabelEXT(cmdBuffer);
	#endif
	}


	void VKDbg::queueBeginLabel(VkQueue queue, const char* labelName, Vec4 color)
	{
	#ifdef VK_RENDERER_DEBUG
		VkDebugUtilsLabelEXT label = { VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT };
		label.pLabelName           = labelName;
		memcpy(label.color, &color, sizeof(float) * 4);

		vkQueueBeginDebugUtilsLabelEXT(queue, &label);
	#endif
	}


	void VKDbg::queueInsertLabel(VkQueue queue, const char* labelName, Vec4 color)
	{
	#ifdef VK_RENDERER_DEBUG
		VkDebugUtilsLabelEXT label = { VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT };
		label.pLabelName           = labelName;
		memcpy(label.color, &color, sizeof(float) * 4);

		vkQueueInsertDebugUtilsLabelEXT(queue, &label);
	#endif
	}


	void VKDbg::queueEndLabel(VkQueue queue)
	{
	#ifdef VK_RENDERER_DEBUG
		vkQueueEndDebugUtilsLabelEXT(queue);
	#endif
	}


	void VKDbg::setObjectName(VkObjectType objectType, U64 objectHandle, const char* objectName)
	{
	#ifdef VK_RENDERER_DEBUG
		VkDebugUtilsObjectNameInfoEXT nameInfo = { VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT };
		nameInfo.objectType                    = objectType;
		nameInfo.objectHandle                  = objectHandle;
		nameInfo.pObjectName                   = objectName;

		vkSetDebugUtilsObjectNameEXT(VyContext::device(), &nameInfo);
	#endif
	}


	void VKDbg::setObjectTag(VkObjectType objectType, U64 objectHandle, U64 tagId, void* tag, size_t tagSize)
	{
	#ifdef VK_RENDERER_DEBUG
		VkDebugUtilsObjectTagInfoEXT tagInfo = { VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_TAG_INFO_EXT };
		tagInfo.objectType                   = objectType;
		tagInfo.objectHandle                 = objectHandle;
		tagInfo.tagName                      = tagId;
		tagInfo.tagSize                      = tagSize;
		tagInfo.pTag                         = tag;

		vkSetDebugUtilsObjectTagEXT(VyContext::device(), &tagInfo);
	#endif
	}

#pragma endregion
}