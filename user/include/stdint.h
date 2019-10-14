#pragma once

#include <sys/cdefs.h>

//Defined for NICTA's ELF loader
typedef	unsigned long long	uint64_t;
typedef long long __int64_t;
//typedef unsigned int		uintptr_t;
typedef unsigned long int uintptr_t;
#define UINT64_MAX              0xffffffffffffffffULL

//Temporary for libfmt
typedef unsigned char		uchar;
typedef unsigned short		ushort;
typedef unsigned int		uint;
typedef unsigned long		ulong;
typedef unsigned long long	uvlong;
typedef long long		vlong;

//http://gitweb.dragonflybsd.org/dragonfly.git/blob/2c42b17bb2eaf0cb73cc31a03c90d486b13b2b7a:/sys/sys/stdint.h
typedef __int64_t       __off_t;

typedef __off_t off_t;
