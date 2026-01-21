#pragma once

#include <VyLib/STL/Containers.h>
#include <VyLib/Core/Assert.h>
#include <VyLib/Core/Numeric.h>

namespace Vy
{

    /// A strongly typed mask for a set of enum flags. Requires that the underlying enum values are powers of two.
    template <typename enum_t>
    class EnumBitMask 
    {
    public:
        using EnumType      = enum_t;
        using EnumValueType = typename std::underlying_type<enum_t>::type;

        /// The raw value of the bit mask.
        EnumValueType value;

        EnumBitMask() : EnumBitMask(enum_t(0)) {}

        constexpr EnumBitMask(enum_t value) : 
            EnumBitMask(static_cast<EnumValueType>(value)) 
        {
        }

        explicit constexpr EnumBitMask(EnumValueType value) : value(value) 
        {
            VY_STATIC_ASSERT(std::is_enum<enum_t>::value, "Template parameter is not an enum");
        }

        explicit constexpr operator enum_t() const 
        {
            return static_cast<enum_t>(value);
        }

        explicit constexpr operator EnumValueType() const 
        {
            return value;
        }

        /// Returns `true` if this bit mask contains the given flag.
        constexpr bool contains(enum_t flag) const 
        {
            return (value & static_cast<EnumValueType>(flag)) != 0;
        }

        /// Returns `true` if this bit mask contains *any* flag.
        constexpr bool containsAny() const 
        {
            return value != 0;
        }

        /// Returns `true` if this bit mask contains *any* of the flags of the other bit mask.
        /// Meaning the intersection of the sets of flags the bit masks represent is not empty.
        constexpr bool containsAny(EnumBitMask<enum_t> other) const 
        {
            return (value & other.value) != 0;
        }

        /// Returns `true` if this bit mask contains *all* of the flags of the other bit mask.
        /// Meaning the set of flags represented by this bit mask is a superset of the other.
        constexpr bool containsAll(EnumBitMask<enum_t> other) const 
        {
            return (value & other.value) == other.value;
        }

        /// Returns the number of flags set in the bit mask.
        U32 countFlagsSet() const 
        {
            // Brian Kernighan's bit count
            EnumValueType v = value;
            U32 c;
            
            for (c = 0; v; c++) 
            {
                v &= v - 1;
            }
            
            return c;
        }

        EnumBitMask<enum_t>& operator=(enum_t value) 
        {
            this->value = static_cast<EnumValueType>(value);
            return *this;
        }

        EnumBitMask<enum_t>& operator|=(EnumBitMask<enum_t> other) 
        {
            value |= other.value;
            return *this;
        }

        EnumBitMask<enum_t>& operator|=(enum_t other) 
        {
            value |= static_cast<EnumValueType>(other);
            return *this;
        }

        EnumBitMask<enum_t>& operator&=(EnumBitMask<enum_t> other) 
        {
            value &= other.value;
            return *this;
        }

        EnumBitMask<enum_t>& operator&=(enum_t other) 
        {
            value &= static_cast<EnumValueType>(other);
            return *this;
        }

        EnumBitMask<enum_t>& operator^=(EnumBitMask<enum_t> other) 
        {
            value ^= other.value;
            return *this;
        }

        EnumBitMask<enum_t>& operator^=(enum_t other) 
        {
            value ^= static_cast<EnumValueType>(other);
            return *this;
        }

        /// Returns a bit mask with no bits set.
        static constexpr EnumBitMask<enum_t> None() 
        {
            return enum_t(0);
        }
    };

    template <typename enum_t>
    constexpr bool operator==(EnumBitMask<enum_t> lhs, EnumBitMask<enum_t> rhs) 
    {
        return lhs.value == rhs.value;
    }

    template <typename enum_t>
    constexpr bool operator==(EnumBitMask<enum_t> lhs, enum_t rhs) 
    {
        return lhs.value == static_cast<typename std::underlying_type<enum_t>::type>(rhs);
    }

    template <typename enum_t>
    constexpr bool operator==(enum_t lhs, EnumBitMask<enum_t> rhs) 
    {
        return static_cast<typename std::underlying_type<enum_t>::type>(lhs) == rhs.value;
    }

    template <typename enum_t>
    constexpr bool operator!=(EnumBitMask<enum_t> lhs, EnumBitMask<enum_t> rhs) 
    {
        return lhs.value != rhs.value;
    }

    template <typename enum_t>
    constexpr bool operator!=(EnumBitMask<enum_t> lhs, enum_t rhs) 
    {
        return lhs.value != static_cast<typename std::underlying_type<enum_t>::type>(rhs);
    }

    template <typename enum_t>
    constexpr bool operator!=(enum_t lhs, EnumBitMask<enum_t> rhs) 
    {
        return static_cast<typename std::underlying_type<enum_t>::type>(lhs) != rhs.value;
    }

    template <typename enum_t>
    constexpr EnumBitMask<enum_t> operator|(EnumBitMask<enum_t> lhs, EnumBitMask<enum_t> rhs) 
    {
        return EnumBitMask<enum_t>(lhs.value | rhs.value);
    }

    template <typename enum_t>
    constexpr EnumBitMask<enum_t> operator|(EnumBitMask<enum_t> lhs, enum_t rhs) 
    {
        return lhs | EnumBitMask<enum_t>(rhs);
    }

    template <typename enum_t>
    constexpr EnumBitMask<enum_t> operator|(enum_t lhs, EnumBitMask<enum_t> rhs) 
    {
        return EnumBitMask<enum_t>(lhs) | rhs;
    }

    template <typename enum_t>
    constexpr EnumBitMask<enum_t> operator&(EnumBitMask<enum_t> lhs, EnumBitMask<enum_t> rhs) 
    {
        return EnumBitMask<enum_t>(lhs.value & rhs.value);
    }

    template <typename enum_t>
    constexpr EnumBitMask<enum_t> operator&(EnumBitMask<enum_t> lhs, enum_t rhs) 
    {
        return lhs & EnumBitMask<enum_t>(rhs);
    }

    template <typename enum_t>
    constexpr EnumBitMask<enum_t> operator&(enum_t lhs, EnumBitMask<enum_t> rhs) 
    {
        return EnumBitMask<enum_t>(lhs) & rhs;
    }

    template <typename enum_t>
    constexpr EnumBitMask<enum_t> operator^(EnumBitMask<enum_t> lhs, EnumBitMask<enum_t> rhs) 
    {
        return EnumBitMask<enum_t>(lhs.value ^ rhs.value);
    }

    template <typename enum_t>
    constexpr EnumBitMask<enum_t> operator^(EnumBitMask<enum_t> lhs, enum_t rhs) 
    {
        return lhs ^ EnumBitMask<enum_t>(rhs);
    }

    template <typename enum_t>
    constexpr EnumBitMask<enum_t> operator^(enum_t lhs, EnumBitMask<enum_t> rhs) 
    {
        return EnumBitMask<enum_t>(lhs) ^ rhs;
    }

    template <typename enum_t>
    constexpr EnumBitMask<enum_t> operator~(EnumBitMask<enum_t> rhs) 
    {
        return EnumBitMask<enum_t>(~rhs.value);
    }

    #define VY_MAKE_ENUM_BIT_MASK(enumMaskName, enumName)              \
        /** A bitmask of Vy::##enumName values. */                     \
        using enumMaskName = EnumBitMask<enumName>;                    \
        constexpr enumMaskName operator|(enumName lhs, enumName rhs)   \
        {                                                              \
            return enumMaskName(lhs) | enumMaskName(rhs);              \
        }                                                              \
        constexpr enumMaskName operator~(enumName rhs)                 \
        {                                                              \
            return ~enumMaskName(rhs);                                 \
        }



    /// An iterator over a Vy::ContiguousEnumView.
    template <typename enum_t>
    class ContiguousEnumIterator 
    {
    public:
        using EnumType      = enum_t;
        using EnumValueType = typename std::underlying_type<enum_t>::type;

        EnumValueType value;

        constexpr ContiguousEnumIterator(EnumValueType value) : 
            value(value) 
        {
        }

        explicit constexpr ContiguousEnumIterator(enum_t element) 
        {
            VY_STATIC_ASSERT(std::is_enum<enum_t>::value, "Template parameter is not an enum");
            value = static_cast<EnumValueType>(element);
        }

        constexpr ContiguousEnumIterator operator++() 
        {
            ++value;
            return *this;
        }

        constexpr enum_t operator*() const 
        {
            return static_cast<enum_t>(value);
        }

        constexpr bool operator!=(const ContiguousEnumIterator& other) const 
        {
            return value != other.value;
        }
    };

    /// Represents an array view of all the values of a contiguous enum type. The values of the enum need to be consecutive
    /// and the largest value needs to be known.
    template <typename enum_t, enum_t Last>
    class ContiguousEnumView 
    {
    public:
        using EnumType      = enum_t;
        using EnumValueType = typename std::underlying_type<enum_t>::type;

        VY_STATIC_ASSERT(std::is_enum<enum_t>::value, "Template parameter is not an enum");

        /// Returns an iterator to the first enum value.
        constexpr ContiguousEnumIterator<enum_t> begin() const 
        {
            return ContiguousEnumIterator<enum_t>(0);
        }

        /// Returns an iterator to the last enum value.
        constexpr ContiguousEnumIterator<enum_t> end() const 
        {
            return ++ContiguousEnumIterator<enum_t>(Last);
        }

        /// Returns the number of consecutive enum values in the view.
        static constexpr std::size_t size() 
        {
            return static_cast<std::size_t>(Last) + 1;
        }
    };

    #define VY_MAKE_CONTIGUOUS_ENUM_VIEW(enumViewName, enumName, lastValue) \
        using enumViewName = ContiguousEnumView<enumName, enumName::lastValue>

}