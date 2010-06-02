/*********************************************************************
 *                
 * Copyright (C) 2010,  Karlsruhe University
 *                
 * File path:     powerpc.h
 * Description:   
 *                
 * @LICENSE@
 *                
 * $Id:$
 *                
 ********************************************************************/
#include <l4/types.h>

#define stringify(s)	tostring(s)
#define tostring(s)	#s

#define mfdcr(rn)	({unsigned int rval; \
			asm volatile("mfdcr %0," stringify(rn) \
				     : "=r" (rval)); rval;})
#define mtdcr(rn, v)	asm volatile("mtdcr " stringify(rn) ",%0" : : "r" (v))

#define mfmsr()		({unsigned int rval; \
			asm volatile("mfmsr %0" : "=r" (rval)); rval;})
#define mtmsr(v)	asm volatile("mtmsr %0" : : "r" (v))

#define mfspr(rn)	({unsigned int rval; \
			asm volatile("mfspr %0," stringify(rn) \
				     : "=r" (rval)); rval;})
#define mtspr(rn, v)	asm volatile("mtspr " stringify(rn) ",%0" : : "r" (v))

#define tlbie(v)	asm volatile("tlbie %0 \n sync" : : "r" (v))



/*
 * 8, 16 and 32 bit, big and little endian I/O operations, with barrier.
 *
 * Read operations have additional twi & isync to make sure the read
 * is actually performed (i.e. the data has come back) before we start
 * executing any following instructions.
 */
L4_INLINE int in_8(const volatile unsigned char *addr)
{
	int ret;

	__asm__ __volatile__(
		"sync; lbz%U1%X1 %0,%1;\n"
		"twi 0,%0,0;\n"
		"isync" : "=r" (ret) : "m" (*addr));
	return ret;
}

L4_INLINE void out_8(volatile unsigned char *addr, int val)
{
	__asm__ __volatile__("stb%U0%X0 %1,%0; eieio" : "=m" (*addr) : "r" (val));
}

L4_INLINE int in_le16(const volatile unsigned short *addr)
{
	int ret;

	__asm__ __volatile__("sync; lhbrx %0,0,%1;\n"
			     "twi 0,%0,0;\n"
			     "isync" : "=r" (ret) :
			      "r" (addr), "m" (*addr));
	return ret;
}

L4_INLINE int in_be16(const volatile unsigned short *addr)
{
	int ret;

	__asm__ __volatile__("sync; lhz%U1%X1 %0,%1;\n"
			     "twi 0,%0,0;\n"
			     "isync" : "=r" (ret) : "m" (*addr));
	return ret;
}

L4_INLINE void out_le16(volatile unsigned short *addr, int val)
{
	__asm__ __volatile__("sync; sthbrx %1,0,%2" : "=m" (*addr) :
			      "r" (val), "r" (addr));
}

L4_INLINE void out_be16(volatile unsigned short *addr, int val)
{
	__asm__ __volatile__("sync; sth%U0%X0 %1,%0" : "=m" (*addr) : "r" (val));
}

L4_INLINE unsigned in_le32(const volatile unsigned *addr)
{
	unsigned ret;

	__asm__ __volatile__("sync; lwbrx %0,0,%1;\n"
			     "twi 0,%0,0;\n"
			     "isync" : "=r" (ret) :
			     "r" (addr), "m" (*addr));
	return ret;
}

L4_INLINE unsigned in_be32(const volatile unsigned *addr)
{
	unsigned ret;

	__asm__ __volatile__("sync; lwz%U1%X1 %0,%1;\n"
			     "twi 0,%0,0;\n"
			     "isync" : "=r" (ret) : "m" (*addr));
	return ret;
}

L4_INLINE void out_le32(volatile unsigned *addr, int val)
{
	__asm__ __volatile__("sync; stwbrx %1,0,%2" : "=m" (*addr) :
			     "r" (val), "r" (addr));
}

L4_INLINE void out_be32(volatile unsigned *addr, int val)
{
	__asm__ __volatile__("sync; stw%U0%X0 %1,%0" : "=m" (*addr) : "r" (val));
}
