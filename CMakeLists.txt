cmake_minimum_required (VERSION 3.26)

set(vcpkg ${CMAKE_CURRENT_LIST_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake)

if(NOT CMAKE_TOOLCHAIN_FILE OR NOT EXISTS ${CMAKE_TOOLCHAIN_FILE})
    if ( EXISTS "${vcpkg}")
        set(CMAKE_TOOLCHAIN_FILE "${vcpkg}" CACHE FILEPATH "CMake toolchain file")
    else()
        message( FATAL_ERROR "Missing VCPKG CMake toolchain file" )
    endif()
endif()

message(STATUS "vcpkg toolchain found in: ${CMAKE_TOOLCHAIN_FILE}")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(VCPKG_MANIFEST_INSTALL ON CACHE BOOL "Install the dependencies listed in your manifest." FORCE)

project(
    VyEngine
    VERSION 0.1.0
    LANGUAGES C CXX
)

include(CMake/Dependencies.cmake)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -MP")

add_definitions(-DPROJECT_DIR="${CMAKE_SOURCE_DIR}")
add_definitions(-DBUILD_DIR="${CMAKE_CURRENT_BINARY_DIR}")

add_compile_definitions(NOMINMAX)

file(GLOB_RECURSE SOURCES
    ${PROJECT_SOURCE_DIR}/Source/Vy/*cpp
    ${PROJECT_SOURCE_DIR}/Source/VyLib/*cpp
#     ${PROJECT_SOURCE_DIR}/Runtime/*cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})

# find_path(TINYGLTF_INCLUDE_DIR "tiny_gltf.h")

# Include and link Vulkan and other submodule libraries
target_include_directories(${PROJECT_NAME} 
    PUBLIC
        ${PROJECT_SOURCE_DIR}/Source
        ${PROJECT_SOURCE_DIR}/Source/VyVK
    PRIVATE 
        ${Stb_INCLUDE_DIR}
        ${Vulkan_INCLUDE_DIRS}
        ${PROJECT_SOURCE_DIR}/Extern/tinygltf
)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_23)

target_link_libraries(${PROJECT_NAME} PRIVATE
    glm::glm
    Vulkan::Vulkan
    volk::volk_headers
    GPUOpen::VulkanMemoryAllocator 
    glfw
    # Vulkan::Vulkan
    tinyobjloader::tinyobjloader
    spdlog::spdlog
    EnTT::EnTT
    tinygltf #::tinygltf
    # imgui::imgui
    # imguizmo::imguizmo
    yaml-cpp::yaml-cpp
    cpptrace::cpptrace
    assimp::assimp
    efsw::efsw
    JsonCpp::JsonCpp
    meshoptimizer::meshoptimizer
)


# target_precompile_headers(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/Source/Vy/VyPCH.h)

add_subdirectory(Data/Shaders)


# Print configuration
message("==========================================================================")
message("CMake Configuration:")
message(STATUS "CMake Version:          ${CMAKE_VERSION}")
message(STATUS "CMake Generator:        ${CMAKE_GENERATOR}")
message(STATUS "CMake Build Type:       ${CMAKE_BUILD_TYPE}")
message(STATUS "CMake source directory: ${CMAKE_SOURCE_DIR}")
message(STATUS "CMake binary directory: ${CMAKE_BINARY_DIR}")
message(STATUS "C++ Compiler:           ${CMAKE_CXX_COMPILER}")
message(STATUS "Vulkan SDK              ${Vulkan_VERSION}: ${Vulkan_LIBRARY}")
message(STATUS "Vulkan inc directory:   ${Vulkan_INCLUDE_DIRS}")
message(STATUS "GLSL Compiler:          ${Vulkan_GLSLC_EXECUTABLE}")
message("==========================================================================")


# set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
# set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)

# set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${PROJECT_BINARY_DIR}/Debug/lib)
# set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${PROJECT_BINARY_DIR}/Debug/lib)
# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${PROJECT_BINARY_DIR}/Debug/bin)

        
# "CMAKE_TOOLCHAIN_FILE": "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"