#pragma once

/**
 * @brief Core file includes for all Vulkan related files.
 */

/**
 * Volk Include
 * Ensure only volk is used for vulkan function loading.
 * NEVER include <vulkan/vulkan.h> directly in any file, use this header instead.
 */
#define VK_NO_PROTOTYPES
#include <volk.h>

/**
 * VMA Include
 * Must be included AFTER Volk.
 */
#include <vma/vk_mem_alloc.h>