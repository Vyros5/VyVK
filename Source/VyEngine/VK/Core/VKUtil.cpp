#include <VyEngine/VK/Core/VKUtil.h>
#include <VyEngine/VK/Core/VKCore.h>

#include <VyEngine/VK/Core/Include/vkImpl.h>

#include <VyEngine/Core/File/FileSystem.hpp>

namespace Vy
{
#pragma region [ VKUtil ]

	TStringView VKUtil::resultString(VkResult result)
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

		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
			return VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;

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
			VY_ASSERT(false, "Unsupported layout transition from layout: {}", STR_VK_IMAGE_LAYOUT(layout));
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

		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
			return VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

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
			VY_ASSERT(false, "Unsupported layout transition from layout: {}", STR_VK_IMAGE_LAYOUT(layout));
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


	VkAccessFlags VKUtil::getAccessMask( VkImageLayout layout )
	{
		VkAccessFlags result{ 0u };

		switch ( layout )
		{
			case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
			case VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR:
			{
				result |= VK_ACCESS_MEMORY_READ_BIT;
			
			} break;
			
			case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
			{
				result |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			
			} break;
			
			case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
			{
				result |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			
			} break;
			
			case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
			{
				result |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
			
			} break;
			
			case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
			{
				result |= VK_ACCESS_SHADER_READ_BIT;
			
			} break;
			
			case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
			{
				result |= VK_ACCESS_TRANSFER_READ_BIT;
			
			} break;
			
			case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			{
				result |= VK_ACCESS_TRANSFER_WRITE_BIT;
			
			} break;
			
			case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL:
			case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL:
			{
				result |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
				result |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			} break;

	#ifdef VK_NV_shading_rate_image
			case VK_IMAGE_LAYOUT_SHADING_RATE_OPTIMAL_NV:
			{
				result |= VK_ACCESS_SHADING_RATE_IMAGE_READ_BIT_NV;
			
			} break;
	#endif

	#ifdef VK_EXT_fragment_density_map
			case VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT:
			{
				result |= VK_ACCESS_FRAGMENT_DENSITY_MAP_READ_BIT_EXT;
			
			} break;
	#endif

			default:
				break;
		}

		return result;
	}

	VkPipelineStageFlags VKUtil::getStageMask( VkImageLayout layout )
	{
		VkPipelineStageFlags result{ 0u };

		switch ( layout )
		{
			case VK_IMAGE_LAYOUT_UNDEFINED:
			{
				result |= VK_PIPELINE_STAGE_HOST_BIT;
			
			} break;

			case VK_IMAGE_LAYOUT_GENERAL:
			{
				result |= VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
			
			} break;

			case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
			case VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR:
			{
				result |= VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
			
			} break;

			case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
			case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL:
			case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL:
			case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
			{
				result |= VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			
			} break;

			case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
			{
				result |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			
			} break;

	#ifdef VK_EXT_fragment_density_map
			case VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT:
	#endif
			case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
			{
				result |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			
			} break;

			case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
			case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			{
				result |= VK_PIPELINE_STAGE_TRANSFER_BIT;
			
			} break;

	#ifdef VK_NV_shading_rate_image
			case VK_IMAGE_LAYOUT_SHADING_RATE_OPTIMAL_NV:
			{
				result |= VK_PIPELINE_STAGE_SHADING_RATE_IMAGE_BIT_NV;
			
			} break;

	#endif
			default:
				break;
		}

		return result;
	}



    VkPipelineStageFlags imageBarrierFlags(VkImageLayout layout, VkAccessFlags& accessFlags)
    {
        VkPipelineStageFlags stageFlags;

        switch (layout)
        {
			case VK_IMAGE_LAYOUT_UNDEFINED:
			{
				accessFlags = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
				stageFlags  = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

			} break;

			case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			{
				accessFlags = VK_ACCESS_TRANSFER_WRITE_BIT;
				stageFlags  = VK_PIPELINE_STAGE_TRANSFER_BIT;
			
			} break;

			case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
			{
				accessFlags = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
				stageFlags  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			
			} break;

			case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
			case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL:
			case VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL:
			{
				accessFlags = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
				stageFlags  = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			
			} break;

			case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
			{
				accessFlags = VK_ACCESS_TRANSFER_READ_BIT;
				stageFlags  = VK_PIPELINE_STAGE_TRANSFER_BIT;
			
			} break;

			case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
			{
				accessFlags = VK_ACCESS_NONE;
				stageFlags  = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			
			} break;

			case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
			{
				accessFlags = VK_ACCESS_SHADER_READ_BIT;
				stageFlags  = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
			
			} break;

			case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
			case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL:
			case VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL:
			{
				accessFlags = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
				stageFlags  = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			
			} break;

			case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL:
			case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL:
			{
				accessFlags = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
				stageFlags  = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			
			} break;

			case VK_IMAGE_LAYOUT_GENERAL:
			{
				accessFlags = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
				stageFlags  = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			
			} break;

			default:
			{
				VY_ASSERT(false, "Unsupported layout transition from layout: {}", STR_VK_IMAGE_LAYOUT(layout));
			
			} break;
        }

        return stageFlags;
    }


	TString VKUtil::printExtensionsList(const TVector<VkExtensionProperties>& extensions, size_t numColumns)
	{
		assert(numColumns > 0);

		TVector<TString> extStrings;
		extStrings.reserve( extensions.size() );

		for (const VkExtensionProperties& ext : extensions)
		{
			std::stringstream ss;
			ss 
			<< ext.extensionName                     << ' '
			<< VK_API_VERSION_MAJOR(ext.specVersion) << '.'
			<< VK_API_VERSION_MINOR(ext.specVersion) << '.'
			<< VK_API_VERSION_PATCH(ext.specVersion);

			extStrings.emplace_back( ss.str() );
		}

		TVector<size_t> colWidth( numColumns );
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


	VkRenderingAttachmentInfo VKUtil::renderingAttachmentInfo(
        VkImageView        imageView, 
        VkImageLayout      layout, 
        VkAttachmentLoadOp loadOp, 
        VkClearValue       clearValue)
    {
		VkRenderingAttachmentInfo attachment{ VKInit::renderingAttachmentInfo() };
        {
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


	VkShaderModule VKUtil::createShaderModule(VkDevice device, const TPath& path)
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