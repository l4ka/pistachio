
//Plan9-compatibility - on real Plan9, these are in u.h
typedef unsigned long long uvlong;
typedef long long vlong;



typedef unsigned char		uchar;
typedef unsigned short		ushort;
typedef unsigned int		uint;
typedef unsigned long		ulong;

typedef uchar _utfuchar;
typedef ushort _utfushort;
typedef uint _utfuint;

typedef ulong _utfulong;
typedef vlong _utfvlong;
typedef uvlong _utfuvlong; 

//typedef _

#define nelem(x) (sizeof(x)/sizeof((x)[0]))
#define nil ((void*)0)


