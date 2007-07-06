/*********************************************************************
 *                
 * Copyright (C) 2003-2004, University of New South Wales
 *                
 * File path:    arch/sparc64/ttable.h
 * Description:  Assembler macros for SPARC v9 Trap Table. 
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
 * $Id: ttable.h,v 1.4 2004/02/10 01:08:49 philipd Exp $
 *                
 ********************************************************************/

#ifndef __ARCH__SPARC64__TTABLE_H__
#define __ARCH__SPARC64__TTABLE_H__

#include INC_GLUE_API_ARCH(ttable.h)

/* Each Trap Table 'trap type' entry is 32 bytes long. */
#define TTABLE_ENTRY_BITS 5
#define TTABLE_ENTRY_SIZE (1 << 5)

/* Trap level (TL) offsets into trap table. */
#define TT_OFFSET_TLO 0
#define TT_OFFSET_TLX 0x200

/*********
* Macros *
*********/

/**
 *  trap_type: 'Trap type'of request in the trap table.
 *  tl:        O if 'Trap level (TL)' = 0, otherwise X.
 */
#define TRAPTYPE2TTOFFSET(trap_type, tl)                                   \
    ((trap_type + TT_OFFSET_TL##tl) << TTABLE_ENTRY_BITS)

/**
 *  trap_type: 'Trap type'of request in the trap table.
 *  tl:        O if 'Trap level (TL)' = 0, otherwise X.
 */
#warning awiggins (20-08-03): This needs to be fixed at some stage, ld problem?
#define TRAPTYPE2ADDR(trap_type, tl)                                       \
    (/*ttable*/ABSOLUTE(0x80000008000) + TRAPTYPE2TTOFFSET(trap_type, tl))

/**
 *  BEGIN_TTABLE_ENTRY: Begin a Trap Table entry point procedure.
 *  name:      Name of the trap request.
 *  tl:        O if 'Trap level (TL)' = 0, otherwise X.
 *
 *  Make sure there is NO whitespace in argument tl! Otherwise things break...
 */
#define BEGIN_TTABLE_ENTRY(name, tl)                                       \
    .section .ttable.##tl##.##name;                                        \
    .globl  ttable_##tl##_##name;                                          \
    .type ttable_##tl##_##name,@function;                                  \
ttable_##tl##_##name:

/**
 *  UNIMPLEMENTED_TTABLE_ENTRY: Declare an unimplemented Trap Table entry point.
 */
#define UNIMPLEMENTED_TTABLE_ENTRY(name, tl)                               \
    BEGIN_TTABLE_ENTRY(name,tl)                                            \
    UNIMPLEMENTED_##tl##_##TRAP()

/**
 *  UNUSED_TTABLE_ENTRY: Declare an unused Trap Table entry point.
 */
#define UNUSED_TTABLE_ENTRY(name, tl)                                      \
    BEGIN_TTABLE_ENTRY(name,tl)                                            \
    UNUSED_##tl##_##TRAP()


#endif /* !__ARCH__SPARC64__TTABLE_H__ */
