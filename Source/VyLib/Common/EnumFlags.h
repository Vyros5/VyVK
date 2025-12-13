#pragma once

#include <VyLib/Common/Numeric.h>
#include <VyLib/Core/Assert.h>

#include <type_traits>

namespace Vy
{
    template <typename FlagBitsType>
    struct VyFlagTraits
    {
        static constexpr bool IsBitmask = false;
    };



    template <typename BitType>
    class VyFlags
    {
    public:
        using MaskType = typename std::underlying_type<BitType>::type;

        // constructors
        constexpr VyFlags() noexcept : 
            m_Mask(0)
        {
        }

        constexpr VyFlags(BitType bit) noexcept : 
            m_Mask(static_cast<MaskType>(bit))
        {
        }

        constexpr VyFlags(VyFlags<BitType> const& rhs) noexcept = default;

        constexpr explicit VyFlags(MaskType flags) noexcept : 
            m_Mask(flags)
        {
        }

        // relational operators
        auto operator<=>(VyFlags<BitType> const&) const = default;

        // logical operator
        constexpr bool operator!() const noexcept
        {
            return !m_Mask;
        }

        // bitwise operators
        constexpr VyFlags<BitType> operator&(VyFlags<BitType> const& rhs) const noexcept
        {
            return VyFlags<BitType>(m_Mask & rhs.m_Mask);
        }

        constexpr VyFlags<BitType> operator|(VyFlags<BitType> const& rhs) const noexcept
        {
            return VyFlags<BitType>(m_Mask | rhs.m_Mask);
        }

        constexpr VyFlags<BitType> operator^(VyFlags<BitType> const& rhs) const noexcept
        {
            return VyFlags<BitType>(m_Mask ^ rhs.m_Mask);
        }

        // The ~ operator here assumes that VyFlagTraits<BitType>::AllFlags is defined, 
        // or you will get a compile error.
        constexpr VyFlags<BitType> operator~() const noexcept
        {
            return VyFlags<BitType>(m_Mask ^ VyFlagTraits<BitType>::AllFlags.m_Mask);
        }

        // assignment operators
        constexpr VyFlags<BitType>& operator=(VyFlags<BitType> const& rhs) noexcept = default;

        constexpr VyFlags<BitType>& operator|=(VyFlags<BitType> const& rhs) noexcept
        {
            m_Mask |= rhs.m_Mask;
            return *this;
        }

        constexpr VyFlags<BitType>& operator&=(VyFlags<BitType> const& rhs) noexcept
        {
            m_Mask &= rhs.m_Mask;
            return *this;
        }

        constexpr VyFlags<BitType>& operator^=(VyFlags<BitType> const& rhs) noexcept
        {
            m_Mask ^= rhs.m_Mask;
            return *this;
        }

        // cast operators
        constexpr operator bool() const noexcept
        {
            return !!m_Mask;
        }

        explicit constexpr operator MaskType() const noexcept
        {
            return m_Mask;
        }

    private:
        MaskType m_Mask;
    };

    // ------------------------------------------------------------------
    // Bitwise operators for combining a BitType with VyFlags<BitType>
    // ------------------------------------------------------------------
    template <typename BitType>
    constexpr VyFlags<BitType> operator&(BitType bit, VyFlags<BitType> const& flags) noexcept
    {
        return flags.operator&(bit);
    }

    template <typename BitType>
    constexpr VyFlags<BitType> operator|(BitType bit, VyFlags<BitType> const& flags) noexcept
    {
        return flags.operator|(bit);
    }

    template <typename BitType>
    constexpr VyFlags<BitType> operator^(BitType bit, VyFlags<BitType> const& flags) noexcept
    {
        return flags.operator^(bit);
    }

    // ------------------------------------------------------------------
    // Bitwise operators on BitType (requires IsBitmask = true)
    // ------------------------------------------------------------------
    template <typename BitType, typename std::enable_if<VyFlagTraits<BitType>::IsBitmask, bool>::type = true>
    VY_INLINE constexpr VyFlags<BitType> operator&(BitType lhs, BitType rhs) noexcept
    {
        return VyFlags<BitType>(lhs) & rhs;
    }

    template <typename BitType, typename std::enable_if<VyFlagTraits<BitType>::IsBitmask, bool>::type = true>
    VY_INLINE constexpr VyFlags<BitType> operator|(BitType lhs, BitType rhs) noexcept
    {
        return VyFlags<BitType>(lhs) | rhs;
    }

    template <typename BitType, typename std::enable_if<VyFlagTraits<BitType>::IsBitmask, bool>::type = true>
    VY_INLINE constexpr VyFlags<BitType> operator^(BitType lhs, BitType rhs) noexcept
    {
        return VyFlags<BitType>(lhs) ^ rhs;
    }

    template <typename BitType, typename std::enable_if<VyFlagTraits<BitType>::IsBitmask, bool>::type = true>
    VY_INLINE constexpr VyFlags<BitType> operator~(BitType bit) noexcept
    {
        return ~(VyFlags<BitType>(bit));
    }    
}