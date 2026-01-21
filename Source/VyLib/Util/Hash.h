#pragma once

#include <VyLib/Core/Numeric.h>
#include <VyLib/Core/Math.h>
#include <VyLib/Core/Assert.h>
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
        VY_INLINE constexpr U32 fnv1a_32(const char* cStr, USize count)
        {
            return ((count ? fnv1a_32(cStr, count - 1) : 2166136261U) ^ cStr[count]) * 16777619U;
        }

        // A constexpr FNV-1a hash function
        constexpr U32 hash_string_fnv1a(TStringView cStr) noexcept 
        {
            U32 hash = 2166136261U; // FNV offset basis

            for (unsigned char c : cStr) 
            {
                hash ^= c;
                hash *= 16777619U; // FNV prime
            }

            return hash;
        }

        // User-defined literal for convenience
        // Usage: "my_string"_hash
        constexpr U32 operator"" _hash(const char* cStr, USize len) noexcept 
        {
            return hash_string_fnv1a(TStringView{ cStr, len });
        }
    }


    /**
     * @brief Combine multiple hash values into a single hash value
     * 
     * @tparam T Type of the first value
     * @tparam Rest Types of the other values
     * @param seed Hash value where the final hash will be stored
     * @param value    First value to hash
     * @param rest Other values that will be hashed
     * 
     * From: https://stackoverflow.com/a/57595105
     */
    template <typename T, typename... Rest>
    void hashCombine(std::size_t& seed, const T& value, const Rest&... rest) 
    {
        // Combine the hash of the current value with the seed.
        seed ^= std::hash<T>{}(value) + 0x9e3779b9 + (seed << 6) + (seed >> 2);

        // Recursively combine the hash of the other values
        (hashCombine(seed, rest), ...);
    };

    VY_INLINE constexpr U32 operator "" _Hash(const char* cStr, size_t count)
    {
        return _Internal::fnv1a_32(cStr, count);
    }

    VY_INLINE constexpr U32 cstringHash(const char* cStr, size_t count)
    {
        return _Internal::fnv1a_32(cStr, count);
    }

    VY_INLINE U32 hashString(const TString& value)
    {
        const U32 hash = Hash::cstringHash(value.c_str(), value.length());

        return hash;
    }
}

namespace Vy::Util
{
    using hash_t = U64;

    class Hasher
    {
    public:
        explicit Hasher(hash_t hash)
            : m_Hash(hash)
        {
        }

        Hasher() = default;

        template <typename T>
        VY_INLINE void data(const T* pData, size_t size)
        {
            size /= sizeof(*pData);

            for (size_t i = 0; i < size; i++)
            {
                m_Hash = (m_Hash * 0x100000001b3ull) ^ pData[i];
            }
        }

        VY_INLINE void u32(U32 value)
        {
            m_Hash = (m_Hash * 0x100000001b3ull) ^ value;
        }

        VY_INLINE void s32(int32_t value)
        {
            u32(U32(value));
        }

        VY_INLINE void f32(float value)
        {
            union
            {
                float f32;
                U32   u32;
            } u;

            u.f32 = value;
            
            u32(u.u32);
        }

        VY_INLINE void u64(U64 value)
        {
            u32(value & 0xffffffffu);
            u32(value >> 32);
        }

        template <typename T>
        VY_INLINE void pointer(T* pPtr)
        {
            u64(reinterpret_cast<std::uintptr_t>(pPtr));
        }

        VY_INLINE void string(const char* cStr)
        {
            char c;
            u32(0xff);

            while ((c = *cStr++) != '\0')
            {
                u32(U8(c));
            }
        }

        VY_INLINE void string(const TString& str)
        {
            u32(0xff);
            
            for (auto& c : str)
            {
                u32(U8(c));
            }
        }

        VY_INLINE hash_t get() const
        {
            return m_Hash;
        }

    private:
        hash_t m_Hash = 0xcbf29ce484222325ull;
    };
}