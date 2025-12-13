#pragma once


#if defined(DEBUG) || defined(_DEBUG)
#	define VY_DEBUG_MODE 1
#else
#	define VY_DEBUG_MODE 0
#endif



#if defined(_WIN32)
#   define VY_PLATFORM_WINDOWS
#elif defined(__linux__)
#   define VY_PLATFORM_LINUX
#else
#   error "Unsupported operating system."
#endif


#if defined(__clang__)
	#define VY_COMPILER_CLANG
#elif defined(__GNUC__)
	#define VY_COMPILER_GNU
#elif defined(_MSC_VER)
	#define VY_COMPILER_MSVC
#else
	#error "Unsupported Compiler."
#endif


#ifdef VY_COMPILER_MSVC
	#define VY_FORCE_INLINE __forceinline
#elif defined(__GNUC__)
	#define VY_FORCE_INLINE __attribute__((always_inline)) inline
#else
	#define VY_FORCE_INLINE inline
#endif

#ifndef VY_INLINE
#	define VY_INLINE inline
#endif


#ifndef VY_NODISCARD
#	define VY_NODISCARD [[nodiscard]]
#endif

#define __MACRO_EXPENDER_INTERNAL(X, Y) X##Y
#define MACRO_EXPENDER(X, Y) __MACRO_EXPENDER_INTERNAL(X, Y)
#define MACRO_EXPENDER_ARGS(...) __VA_ARGS__

#include <spdlog/fmt/fmt.h>
#include <cpptrace/cpptrace.hpp>

#define VY_THROW_RUNTIME_ERROR(MESSAGE) \
    cpptrace::generate_trace().print(); \
    throw std::runtime_error(fmt::format("Message: {}\n@File: {}\n@Line: {}", MESSAGE, __FILE__, __LINE__))


#define VY_THROW_INVALID_ARGUMENT(MESSAGE)  \
    cpptrace::generate_trace().print();     \
    throw std::invalid_argument(fmt::format("Message: {}\n@File: {}\n@Line: {}", MESSAGE, __FILE__, __LINE__))


#define VY_BIT(x) (1u << x)

// Settings for Windows OS
#ifdef VY_PLATFORM_WINDOWS

    // Disable any MSVC specific warnings
    #pragma warning(disable : 4251)

    // 16-byte Memory alignment for the Windows OS/Engine
    #define MEM_DEF_ALIGNMENT_16 16

    #define VY_MEM_ALIGN_16 alignas(MEM_DEF_ALIGNMENT_16)    // This is from std c++ that is supposed to be available everywhere

	// Symbols Export settings for Engine
  	#ifdef VY_BUILD_DLL
    	#define VY_API __declspec(dllexport)
  	#else
    	#define VY_API __declspec(dllimport)
  	#endif

	// Debug functions for breaking the debugger or interrupting the code.
	#define VY_DEBUG_BREAK __debugbreak()	// MSVC


#endif
