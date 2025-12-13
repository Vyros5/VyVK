#pragma once

#include <VyLib/Common/Numeric.h>
#include <VyLib/Common/Math.h>
#include <VyLib/Common/VyUUID.h>

#include <VyLib/STL/String.h>
#include <VyLib/STL/Utility.h>
#include <VyLib/STL/Pointers.h>
#include <VyLib/STL/Containers.h>

#include <VyLib/STL/Ref/Ref.h>

#include <VyLib/Core/Assert.h>
#include <VyLib/Core/Defines.h>
#include <VyLib/Core/VyLogger.h>
#include <VyLib/Core/Exceptions.h>

namespace Vy
{
    class NonCopyable 
    {
    protected:
        // Protected default constructor and destructor
        // Allows instantiation by derived classes, but not directly
        NonCopyable() = default;

        ~NonCopyable() = default;

        // Deleted copy constructor and copy assignment operator
        NonCopyable(const NonCopyable &) = delete;

        NonCopyable &operator=(const NonCopyable &) = delete;
    };


#ifdef __cplusplus
#	define VY_ENUM_CLASS_FLAG(VALUE_TYPE, ENUM_TYPE)																	   \
	constexpr ENUM_TYPE  operator| (ENUM_TYPE a,  VALUE_TYPE b) { return (ENUM_TYPE)((VALUE_TYPE)(a) | b); }  \
	constexpr ENUM_TYPE  operator| (ENUM_TYPE a,  ENUM_TYPE  b) { return (ENUM_TYPE)((VALUE_TYPE)(a) | (VALUE_TYPE)(b)); }  \
	constexpr ENUM_TYPE  operator& (ENUM_TYPE a,  VALUE_TYPE b) { return (ENUM_TYPE)((VALUE_TYPE)(a) & b); }  \
	constexpr ENUM_TYPE  operator& (ENUM_TYPE a,  ENUM_TYPE  b) { return (ENUM_TYPE)((VALUE_TYPE)(a) & (VALUE_TYPE)(b)); }  \
	constexpr ENUM_TYPE& operator|=(ENUM_TYPE& a, VALUE_TYPE b) { a = (ENUM_TYPE)((VALUE_TYPE)(a) | b); return a; }  \
	constexpr ENUM_TYPE& operator|=(ENUM_TYPE& a, ENUM_TYPE  b) { a = (ENUM_TYPE)((VALUE_TYPE)(a) | (VALUE_TYPE)(b)); return a; }   \
	constexpr ENUM_TYPE& operator&=(ENUM_TYPE& a, VALUE_TYPE b) { a = (ENUM_TYPE)((VALUE_TYPE)(a) & b); return a; }  \
	constexpr ENUM_TYPE& operator&=(ENUM_TYPE& a, ENUM_TYPE  b) { a = (ENUM_TYPE)((VALUE_TYPE)(a) & (VALUE_TYPE)(b)); return a; } 
#endif

#define VY_HAS_ENUM_FLAG(VALUE, FLAG) static_cast<bool>((VALUE) & (FLAG))
}