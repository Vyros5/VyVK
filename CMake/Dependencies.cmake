
# ========================================================================
# Vulkan Related
set(VULKAN_SDK "C:/VulkanSDK/1.4.335.0" CACHE PATH "Path to the Vulkan SDK")
# set(ENV{VULKAN_SDK} "C:/VulkanSDK/1.4.335.0")
find_package(Vulkan REQUIRED)
if(NOT Vulkan_FOUND)
    message(FATAL_ERROR "Vulkan was not found. Make sure the Vulkan SDK is installed")
endif()

find_package(Volk CONFIG REQUIRED)
if(NOT Volk_FOUND)
    message(FATAL_ERROR "Volk was not found.")
endif()

find_package(VulkanMemoryAllocator CONFIG REQUIRED)

# ========================================================================

find_package(glfw3 CONFIG REQUIRED)
set(GLFW_LIB glfw)
message(STATUS "Found GLFW")
if (NOT GLFW_LIB)
	message(FATAL_ERROR "Could not find glfw library!")
else()
	message(STATUS "Using glfw lib at: ${GLFW_LIB}")
endif()

# ========================================================================

# set(IMGUI_USE_VOLK ON)
add_subdirectory(Extern/tinygltf)

# ========================================================================

find_package(glm CONFIG REQUIRED)
find_package(tinyobjloader CONFIG REQUIRED)
# find_package(tinygltf CONFIG REQUIRED)
# find_package(unofficial-spirv-reflect CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(EnTT CONFIG REQUIRED)
find_package(Stb REQUIRED)
find_package(KTX CONFIG REQUIRED)
find_package(mikktspace CONFIG REQUIRED)
find_package(yaml-cpp CONFIG REQUIRED)
find_package(cpptrace CONFIG REQUIRED)
find_package(efsw CONFIG REQUIRED)
find_package(JsonCpp CONFIG REQUIRED)
find_package(meshoptimizer CONFIG REQUIRED)

# find_package(spirv_cross_core CONFIG REQUIRED)
# find_package(spirv_cross_glsl CONFIG REQUIRED)

# ========================================================================
