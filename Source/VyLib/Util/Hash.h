#pragma once

#include <VyLib/Common/Numeric.h>
#include <VyLib/Common/Math.h>
#include <VyLib/STL/String.h>
#include <iostream>
#include <functional>

namespace Vy::Hash
{
    namespace _Internal
    {
        // 16777619U   = FNV Prime
        // 2166136261U = FNV offset basis

        // C++11 compile-time hash of literal strings. 
        // Source: https://gist.github.com/Lee-R/3839813
        VY_INLINE constexpr U32 fnv1a_32(const char* str, USize count)
        {
            return ((count ? fnv1a_32(str, count - 1) : 2166136261U) ^ str[count]) * 16777619U;
        }

        // A constexpr FNV-1a hash function
        constexpr U32 hash_string_fnv1a(StringView str) noexcept 
        {
            U32 hash = 2166136261U; // FNV offset basis

            for (unsigned char c : str) 
            {
                hash ^= c;
                hash *= 16777619U; // FNV prime
            }

            return hash;
        }

        // User-defined literal for convenience
        // Usage: "my_string"_hash
        constexpr U32 operator"" _hash(const char* str, USize len) noexcept 
        {
            return hash_string_fnv1a(StringView{ str, len });
        }
    }


    /**
     * @brief Combine multiple hash values into a single hash value
     * 
     * @tparam T Type of the first value
     * @tparam Rest Types of the other values
     * @param seed Hash value where the final hash will be stored
     * @param v    First value to hash
     * @param rest Other values that will be hashed
     */
    template <typename T, typename... Rest>
    void hashCombine(std::size_t& seed, const T& v, const Rest&... rest) 
    {
        // Combine the hash of the current value with the seed
        seed ^= std::hash<T>{}(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);

        // Recursively combine the hash of the other values
        (hashCombine(seed, rest), ...);
    };

    VY_INLINE constexpr U32 operator "" _Hash(const char* str, size_t count)
    {
        return _Internal::fnv1a_32(str, count);
    }

    VY_INLINE constexpr U32 cstringHash(const char* str, size_t count)
    {
        return _Internal::fnv1a_32(str, count);
    }

    VY_INLINE U32 hashString(const String& value)
    {
        const U32 hash = Hash::cstringHash(value.c_str(), value.length());

        return hash;
    }
}