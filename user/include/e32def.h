/* This implmenents an interface broadly similar to that provided by Symbian OS's
      e32def.h.... */

#pragma once


#ifdef __cplusplus
extern "C" {
#endif

//#include <string.h>

//Equivalent to standard void, and preferred to void* when pointerised...
typedef void TAny;

//This is always a signed, 8-bit integer...
typedef signed char TInt8;

//This is always an unsigned char...
typedef unsigned char TUint8;

//Always an unsigned integer, likely to be 8-bit, but not guaranteed...
typedef unsigned int TUint;

//Always a 32-bit(?), signed long integer...
typedef long int TInt32;

//Always a 32-bit, signed integer
typedef signed int TInt;

//64-bit, floating-point number, but not guaranteed
typedef double TReal64;

//This is a synonym for TReal64
typedef double TReal;

//Guaranteed to be an unsigned, encoding-agnostic character (8 bits)...
typedef unsigned char TText8;

//16-bit, unsigned character, synonym for wchar_t (or Plan9 runes equivalent)
typedef unsigned short int TText16;

//Boolean type, 0 = false, all other values = true
typedef int TBool;

//Unsigned, long long integer (64-bit)
typedef unsigned long long Uint64;

//64-bit signed integer
typedef long long Int64;

//This is a synonym for Int64
typedef	Int64	TInt64;

typedef unsigned long int TUint32;

//Reformat these, later
#define ETrue 1
#define EFalse 0

#ifdef __cplusplus
}
#endif
