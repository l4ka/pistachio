/*********************************************************************
 *                
 * Copyright (C) 2003-2004,  National ICT Australia (NICTA)
 *                
 * File path:     arch/arm/debug.h
 * Description:   Debug support
 *                
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *                
 * $Id: debug.h,v 1.7 2004/06/04 02:14:22 cvansch Exp $
 *                
 ********************************************************************/

#ifndef __ARCH__ARM__DEBUG_H__
#define __ARCH__ARM__DEBUG_H__

#include INC_GLUE(syscalls.h)
#include INC_GLUE(debug.h)


INLINE int spin_forever(int pos = 0)
{
    while(1) { asm ("mov        r0, r0          \n"); }
    return 0;
}

INLINE void spin(int pos = 0, int cpu = 0)
{
}


#define HERE()               printf("Here %s:%d\n", __PRETTY_FUNCTION__, __LINE__)

#endif /* __ARCH__ARM__DEBUG_H__ */
