#pragma once

#include <VyLib/Core/VyLogger.h>
#include <VyLib/Core/Defines.h>

#include <atomic>
#include <cassert>



#if defined(_DEBUG) || defined(VY_DEBUG)
#   define VY_ENABLE_ASSERTS
#   define VY_ENABLE_VERIFY
#endif

#define VY_STATIC_ASSERT(...) static_assert(__VA_ARGS__)

#ifdef VY_ENABLE_ASSERTS
#   define VY_CORE_ASSERT_MESSAGE_INTERNAL(...) ::Vy::VyLogger::printAssertMessage(::Vy::LogType::Core,   "Assertion Failed", ##__VA_ARGS__)
#   define VY_ASSERT_MESSAGE_INTERNAL(...)      ::Vy::VyLogger::printAssertMessage(::Vy::LogType::Client, "Assertion Failed", ##__VA_ARGS__)

#define VY_CORE_ASSERT(condition, ...)                    \
    do                                                    \
    {                                                     \
        if (!(condition))                                 \
        {                                                 \
            VY_CORE_ASSERT_MESSAGE_INTERNAL(__VA_ARGS__); \
            VY_DEBUG_BREAK;                               \
        }                                                 \
    } while (false)
#define VY_ASSERT(condition, ...)                    \
    do                                               \
    {                                                \
        if (!(condition))                            \
        {                                            \
            VY_ASSERT_MESSAGE_INTERNAL(__VA_ARGS__); \
            VY_DEBUG_BREAK;                          \
        }                                            \
    } while (false)
#else
#define VY_CORE_ASSERT(condition, ...) \
    do                                 \
    {                                  \
    } while (false)
#define VY_ASSERT(condition, ...) \
    do                            \
    {                             \
    } while (false)
#endif

#ifdef VY_ENABLE_VERIFY
// #   ifdef VY_COMPILER_CLANG
// #       define VY_CORE_VERIFY_MESSAGE_INTERNAL(...) ::Vy::VyLogger::printAssertMessage(::Vy::LogType::Core,   "Verify Failed", ##__VA_ARGS__)
// #       define VY_VERIFY_MESSAGE_INTERNAL(...)      ::Vy::VyLogger::printAssertMessage(::Vy::LogType::Client, "Verify Failed", ##__VA_ARGS__)
// #   else
#       define VY_CORE_VERIFY_MESSAGE_INTERNAL(...) ::Vy::VyLogger::printAssertMessage(::Vy::LogType::Core,   "Verify Failed" __VA_OPT__(, ) __VA_ARGS__)
#       define VY_VERIFY_MESSAGE_INTERNAL(...)      ::Vy::VyLogger::printAssertMessage(::Vy::LogType::Client, "Verify Failed" __VA_OPT__(, ) __VA_ARGS__)

#       define VY_CORE_REL_ASSERT_MESSAGE_INTERNAL(...) ::Vy::VyLogger::getCoreLogger()->error("Verify Failed" __VA_OPT__(, ) __VA_ARGS__)
#       define VY_REL_ASSERT_MESSAGE_INTERNAL(...)      ::Vy::VyLogger::getCoreLogger()->error("Verify Failed" __VA_OPT__(, ) __VA_ARGS__)
// #   endif

#define VY_CORE_VERIFY(condition, ...)                    \
    do                                                    \
    {                                                     \
        if (!(condition))                                 \
        {                                                 \
            VY_CORE_VERIFY_MESSAGE_INTERNAL(__VA_ARGS__); \
            VY_DEBUG_BREAK;                               \
        }                                                 \
    } while (false)
#define VY_VERIFY(condition, ...)                    \
    do                                               \
    {                                                \
        if (!(condition))                            \
        {                                            \
            VY_VERIFY_MESSAGE_INTERNAL(__VA_ARGS__); \
            VY_DEBUG_BREAK;                          \
        }                                            \
    } while (false)

#define VY_CORE_REL_ASSERT(condition, ...)                    \
    do                                                        \
    {                                                         \
        if (!(condition))                                     \
        {                                                     \
            VY_CORE_REL_ASSERT_MESSAGE_INTERNAL(__VA_ARGS__); \
        }                                                     \
    } while (false)
#define VY_VERIFY_REL_ASSERT(condition, ...)             \
    do                                                   \
    {                                                    \
        if (!(condition))                                \
        {                                                \
            VY_REL_ASSERT_MESSAGE_INTERNAL(__VA_ARGS__); \
        }                                                \
    } while (false)
#else
#define VY_CORE_VERIFY(condition, ...) \
    do                                 \
    {                                  \
    } while (false)
#define VY_VERIFY(condition, ...) \
    do                            \
    {                             \
    } while (false)
#endif
