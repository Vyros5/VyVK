
# ========================================================================
# Vulkan Related

find_package(Vulkan REQUIRED)
if(NOT Vulkan_FOUND)
    message(FATAL_ERROR "Vulkan was not found. Make sure the Vulkan SDK is installed")
endif()

find_package(Volk CONFIG REQUIRED)
# find_package(VulkanHeaders CONFIG REQUIRED)
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

find_package(assimp CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(EnTT CONFIG REQUIRED)
find_package(Stb REQUIRED)
find_package(yaml-cpp CONFIG REQUIRED)
find_package(cpptrace CONFIG REQUIRED)
find_package(efsw CONFIG REQUIRED)
find_package(JsonCpp CONFIG REQUIRED)
find_package(meshoptimizer CONFIG REQUIRED)

# ========================================================================
