#pragma once
#include <string.h>

//http://developer.nokia.com/community/wiki/Fundamental_Types_in_symbian

//We don't have DLLs
#define EXPORT_C 


//These are supposed to be defined in e32def.h, in Symbian OS
typedef unsigned int TUint;
typedef long int TInt32;

//Symbian ints are always signed, unless marked as unsigned explicitly
typedef signed int TInt;

//Compatibility value...
typedef long long Int64;
typedef	Int64	TInt64;

//Real number types
typedef double TReal;
typedef double TReal64;
