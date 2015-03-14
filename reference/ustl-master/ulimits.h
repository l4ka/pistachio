// This file is part of the uSTL library, an STL implementation.
//
// Copyright (c) 2005 by Mike Sharov <msharov@users.sourceforge.net>
// This file is free software, distributed under the MIT License.

#pragma once
#include "utypes.h"

namespace ustl {

namespace {
template <typename T> struct __limits_digits { enum { value = sizeof(T)*8 };};
template <typename T> struct __limits_digits10 { enum { value = sizeof(T)*8*643/2136+1 };};
}

/// \class numeric_limits ulimits.h ustl.h
/// \brief Defines numeric limits for a type.
///
template <typename T> 
struct numeric_limits {
    /// Returns the minimum value for type T.
    static inline constexpr T min (void)		{ return (T(0)); }
    /// Returns the minimum value for type T.
    static inline constexpr T max (void)		{ return (T(0)); }
    static const bool is_signed = false;	///< True if the type is signed.
    static const bool is_integer = false;	///< True if stores an exact value.
    static const bool is_integral = false;	///< True if fixed size and cast-copyable.
    static const unsigned digits = __limits_digits<T>::value;		///< Number of bits in T
    static const unsigned digits10 = __limits_digits10<T>::value;	///< Maximum number of decimal digits in printed version of T
};

#ifndef DOXYGEN_SHOULD_SKIP_THIS

template <typename T>
struct numeric_limits<T*> {
    static inline constexpr T* min (void)	{ return (NULL); }
    static inline constexpr T* max (void)	{ return (reinterpret_cast<T*>(UINTPTR_MAX)); }
    static const bool is_signed = false;
    static const bool is_integer = true;
    static const bool is_integral = true;
    static const unsigned digits = __limits_digits<T*>::value;
    static const unsigned digits10 = __limits_digits10<T*>::value;
};

#define _NUMERIC_LIMITS(type, minVal, maxVal, bSigned, bInteger, bIntegral)	\
template <>								\
struct numeric_limits<type> {						\
    static inline constexpr type min (void)	{ return (minVal); }	\
    static inline constexpr type max (void)	{ return (maxVal); }	\
    static const bool is_signed = bSigned;				\
    static const bool is_integer = bInteger;				\
    static const bool is_integral = bIntegral;				\
    static const unsigned digits = __limits_digits<type>::value;	\
    static const unsigned digits10 = __limits_digits10<type>::value;	\
}

//--------------------------------------------------------------------------------------
//		type		min		max		signed	integer	integral
//--------------------------------------------------------------------------------------
_NUMERIC_LIMITS (bool,		false,		true,		false,	true,	true);
_NUMERIC_LIMITS (char,		CHAR_MIN,	CHAR_MAX,	true,	true,	true);
_NUMERIC_LIMITS (int,		INT_MIN,	INT_MAX,	true,	true,	true);
_NUMERIC_LIMITS (short,		SHRT_MIN,	SHRT_MAX,	true,	true,	true);
_NUMERIC_LIMITS (long,		LONG_MIN,	LONG_MAX,	true,	true,	true);
#if HAVE_THREE_CHAR_TYPES
_NUMERIC_LIMITS (signed char,	SCHAR_MIN,	SCHAR_MAX,	true,	true,	true);
#endif
_NUMERIC_LIMITS (unsigned char,	0,		UCHAR_MAX,	false,	true,	true);
_NUMERIC_LIMITS (unsigned int,	0,		UINT_MAX,	false,	true,	true);
_NUMERIC_LIMITS (unsigned short,0,		USHRT_MAX,	false,	true,	true);
_NUMERIC_LIMITS (unsigned long,	0,		ULONG_MAX,	false,	true,	true);
_NUMERIC_LIMITS (wchar_t,	0,		WCHAR_MAX,	false,	true,	true);
_NUMERIC_LIMITS (float,		FLT_MIN,	FLT_MAX,	true,	false,	true);
_NUMERIC_LIMITS (double,	DBL_MIN,	DBL_MAX,	true,	false,	true);
_NUMERIC_LIMITS (long double,	LDBL_MIN,	LDBL_MAX,	true,	false,	true);
#if HAVE_LONG_LONG
_NUMERIC_LIMITS (long long,	LLONG_MIN,	LLONG_MAX,	true,	true,	true);
_NUMERIC_LIMITS (unsigned long long,	0,	ULLONG_MAX,	false,	true,	true);
#endif
//--------------------------------------------------------------------------------------

#endif // DOXYGEN_SHOULD_SKIP_THIS

/// Macro for defining numeric_limits specializations
#define NUMERIC_LIMITS(type, minVal, maxVal, bSigned, bInteger, bIntegral)	\
namespace ustl { _NUMERIC_LIMITS (type, minVal, maxVal, bSigned, bInteger, bIntegral); }

} // namespace ustl
