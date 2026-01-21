#pragma once

#include <VyLib/Core/Assert.h>
#include <VyLib/Core/Defines.h>
#include <VyLib/Core/VyLogger.h>
#include <VyLib/Core/Exceptions.h>
#include <VyLib/Core/Numeric.h>
#include <VyLib/Core/Math.h>

#include <VyLib/Common/UUID.h>

#include <VyLib/STL/String.h>
#include <VyLib/STL/Utility.h>
#include <VyLib/STL/Pointers.h>
#include <VyLib/STL/Containers.h>

namespace Vy
{
#define VY_NON_COPYABLE(class_name)                      \
	class_name(const class_name&)              = delete; \
	class_name& operator = (const class_name&) = delete; 

#define VY_NON_MOVEABLE(class_name)                      \
	class_name(class_name&&)                   = delete; \
	class_name& operator = (class_name&&)      = delete;

}

namespace Vy
{

// Utility to enable bitmask operators on enum classes.
// To use define an enum class with valid bitmask values and an underlying type
// then use the macro to enable support:
//  enum class MyBitmask : U32 {
//    kFoo = 1 << 0,
//    kBar = 1 << 1,
//  };
//  VY_BITMASK(MyBitmask);
//  MyBitmask value = ~(MyBitmask::kFoo | MyBitmask::kBar);

#define VY_ENUM_BITMASK(ENUM_CLASS)                                                                     \
	inline ENUM_CLASS operator|(ENUM_CLASS lhs, ENUM_CLASS rhs)                                    \
	{                                                                                              \
		typedef typename std::underlying_type<ENUM_CLASS>::type enum_type;                         \
		return static_cast<ENUM_CLASS>(static_cast<enum_type>(lhs) | static_cast<enum_type>(rhs));   \
	}                                                                                              \
	inline ENUM_CLASS& operator|=(ENUM_CLASS& lhs, ENUM_CLASS rhs)                                 \
	{                                                                                              \
		typedef typename std::underlying_type<ENUM_CLASS>::type enum_type;                         \
		lhs = static_cast<ENUM_CLASS>(static_cast<enum_type>(lhs) | static_cast<enum_type>(rhs));  \
		return lhs;                                                                                \
	}                                                                                              \
	inline ENUM_CLASS operator&(ENUM_CLASS lhs, ENUM_CLASS rhs)                                    \
	{                                                                                              \
		typedef typename std::underlying_type<ENUM_CLASS>::type enum_type;                         \
		return static_cast<ENUM_CLASS>(static_cast<enum_type>(lhs) & static_cast<enum_type>(rhs)); \
	}                                                                                              \
	inline ENUM_CLASS& operator&=(ENUM_CLASS& lhs, ENUM_CLASS rhs)                                 \
	{                                                                                              \
		typedef typename std::underlying_type<ENUM_CLASS>::type enum_type;                         \
		lhs = static_cast<ENUM_CLASS>(static_cast<enum_type>(lhs) & static_cast<enum_type>(rhs));  \
		return lhs;                                                                                \
	}                                                                                              \
	inline ENUM_CLASS operator^(ENUM_CLASS lhs, ENUM_CLASS rhs)                                    \
	{                                                                                              \
		typedef typename std::underlying_type<ENUM_CLASS>::type enum_type;                         \
		return static_cast<ENUM_CLASS>(static_cast<enum_type>(lhs) ^ static_cast<enum_type>(rhs)); \
	}                                                                                              \
	inline ENUM_CLASS& operator^=(ENUM_CLASS& lhs, ENUM_CLASS rhs)                                 \
	{                                                                                              \
		typedef typename std::underlying_type<ENUM_CLASS>::type enum_type;                         \
		lhs = static_cast<ENUM_CLASS>(static_cast<enum_type>(lhs) ^ static_cast<enum_type>(rhs));  \
		return lhs;                                                                                \
	}                                                                                              \
	inline ENUM_CLASS operator~(ENUM_CLASS lhs)                                                    \
	{                                                                                              \
		typedef typename std::underlying_type<ENUM_CLASS>::type enum_type;                         \
		return static_cast<ENUM_CLASS>(~static_cast<enum_type>(lhs));                              \
	}                                                                                              \
	inline bool any(ENUM_CLASS lhs)                                                                \
	{                                                                                              \
		typedef typename std::underlying_type<ENUM_CLASS>::type enum_type;                         \
		return static_cast<enum_type>(lhs) != 0;                                                   \
	}                                                                                              

}