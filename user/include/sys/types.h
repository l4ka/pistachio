#ifndef __SYS_TYPES_H__
#define __SYS_TYPES_H__

#ifdef __cplusplus
extern "C" {
#endif

//http://www.cplusplus.com/reference/cstdlib/

#include <l4/types.h>
#include <sys/_types.h>

/* L4 defines these:

   * typedef unsigned int            L4_Word32_t;

   * typedef unsigned short          L4_Word16_t;

   * typedef unsigned char           L4_Word8_t;

   * typedef unsigned long           L4_Word_t;

*/

//Rand wants this
//http://fxr.watson.org/fxr/source/sys/types.h?v=OPENBSD
typedef unsigned int    u_int;
typedef unsigned int    uint8_t; 

typedef L4_Word32_t int32_t;
typedef	unsigned short		uint16_t;
typedef	signed char	int8_t;

#define CHAR_BIT 8
#define	RAND_MAX	2147483647 // Like Linux

//typedef	unsigned long		__size_t;
//ftp://ftp.fr.openbsd.org/pub/OpenBSD/src/sys/arch/i386/include/_types.h
//typedef	unsigned long		size_t;

typedef unsigned long long u_quad_t;
typedef long long quad_t;
typedef L4_Word_t u_long;
void * memset(void *s1, int c, size_t n);
size_t strlen(const char *str);


//For compatibility with OpenBSD
typedef unsigned char u_char;
typedef __uint16_t u_int16_t;
#ifdef __cplusplus
}
#endif


#endif /* !__SYS_TYPES_H__ */

