#pragma once

#include <cstdint>
#include <cstddef>

#include <cassert>
#include <stdexcept>

#include <limits>

namespace Vy
{
    // --------------------------------------------------------------------------------------------

    /**
     * Links:
     *  - Fixed-Width Integer Types
     *    https://en.cppreference.com/w/cpp/types/integer.html
     * 
     *  - Fundamental Types
     *    https://en.cppreference.com/w/cpp/language/types.html
     * 
     * Standards:
     *  - char      - Width of at least 08-bits.
     *  - short     - Width of at least 16-bits.
     *  - int       - Width of at least 16-bits. (Is at least 32-bits on 32/64-bit Systems.)
     *  - long      - Width of at least 32-bits.
     *  - long long - Width of at least 64-bits.
     */

    // --------------------------------------------------------------------------------------------
    
    using Bool   = bool;
    using Char   = char;
    using WChar  = wchar_t;
    using UChar  = unsigned char;
    using Byte   = unsigned char;

    using Short  = signed   short;
    using UShort = unsigned short;

    using Int    = signed   int;
    using UInt   = unsigned int;

    using Long   = signed   long int;
    using ULong  = unsigned long long int;

    using Float  = float;
    using Double = double; 

    static_assert(sizeof(bool)   == 1, "Wrong size for bool");
    static_assert(sizeof(char)   == 1, "Wrong size for char");
    static_assert(sizeof(short)  == 2, "Wrong size for short");
    static_assert(sizeof(int)    == 4, "Wrong size for int");
    static_assert(sizeof(long)   == 4, "Wrong size for long");
    static_assert(sizeof(float)  == 4, "Wrong size for float");
    static_assert(sizeof(double) == 8, "Wrong size for double");

    // -------------------------------------------------------------------

    // Signed integer -  8-bits (signed char)
    using     I8            = int8_t;
    constexpr I8   kMaxI8   = std::numeric_limits<I8>::max();
    constexpr I8   kMinI8   = std::numeric_limits<I8>::min();
    
    // Signed integer - 16-bits (signed short)
    using     I16           = int16_t;
    constexpr I16  kMaxI16  = std::numeric_limits<I16>::max();
    constexpr I16  kMinI16  = std::numeric_limits<I16>::min();

    // Signed integer - 32-bits (signed int)
    using     I32           = int32_t;
    constexpr I32  kMaxI32  = std::numeric_limits<I32>::max();
    constexpr I32  kMinI32  = std::numeric_limits<I32>::min();

    // Signed integer - 64-bits (signed long long)
    using     I64           = int64_t;
    constexpr I64  kMaxI64  = std::numeric_limits<I64>::max();
    constexpr I64  kMinI64  = std::numeric_limits<I64>::min();

    // Fast signed integer - at least 32-bits (signed int)
    using     I32x          = int_fast32_t; 
    constexpr I32x kMaxI32x = std::numeric_limits<I32x>::max();
    constexpr I32x kMinI32x = std::numeric_limits<I32x>::min();

    // -------------------------------------------------------------------

    // Unsigned integer -  8-bits (unsigned char)
    using     U8           = uint8_t;
    constexpr U8   kMaxU8  = std::numeric_limits<U8>::max();
    constexpr U8   kMinU8  = std::numeric_limits<U8>::min();

    // Unsigned integer - 16-bits (unsigned short)
    using     U16           = uint16_t;
    constexpr U16  kMaxU16  = std::numeric_limits<U16>::max();
    constexpr U16  kMinU16  = std::numeric_limits<U16>::min();

    // Unsigned integer - 32-bits (unsigned int)
    using     U32           = uint32_t;
    constexpr U32  kMaxU32  = std::numeric_limits<U32>::max();
    constexpr U32  kMinU32  = std::numeric_limits<U32>::min();

    // Unsigned integer - 64-bits (unsigned long long)
    using     U64           = uint64_t;
    constexpr U64  kMaxU64  = std::numeric_limits<U64>::max();
    constexpr U64  kMinU64  = std::numeric_limits<U64>::min();

    // Fast unsigned integer - at least 32-bits (unsigned int)
    using     U32x          = uint_fast32_t; 
    constexpr U32x kMaxU32x = std::numeric_limits<U32x>::max();
    constexpr U32x kMinU32x = std::numeric_limits<U32x>::min();

    // -------------------------------------------------------------------

    // Pointer size - (size_t)
    using     USize             = std::size_t;
    constexpr USize kMaxPtrSize = std::numeric_limits<USize>::max();
    constexpr USize kMinPtrSize = std::numeric_limits<USize>::min();
    static_assert(sizeof(USize) == sizeof(void*), "Wrong size for size_t");

    using UPtr = std::uintptr_t;

    // -------------------------------------------------------------------

    // Floating point - 32-bits (float)
    using     F32         =  Float; 
    constexpr F32 kMaxF32 =  std::numeric_limits<F32>::max();
    constexpr F32 kMinF32 = -std::numeric_limits<F32>::max();

    // Floating point - 64-bits (double)
    using     F64         =  Double; 
    constexpr F64 kMaxF64 =  std::numeric_limits<F64>::max();
    constexpr F64 kMinF64 = -std::numeric_limits<F64>::max();

    // -------------------------------------------------------------------

}