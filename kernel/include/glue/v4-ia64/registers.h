/*********************************************************************
 *                
 * Copyright (C) 2002, 2003,  Karlsruhe University
 *                
 * File path:     glue/v4-ia64/registers.h
 * Description:   IA-64 register association
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
 * $Id: registers.h,v 1.9 2003/09/24 19:04:37 skoglund Exp $
 *                
 ********************************************************************/
#ifndef __GLUE__V4_IA64__REGISTERS_H__
#define __GLUE__V4_IA64__REGISTERS_H__


/*
 * User-level read-only registers
 */

#define r_GLOBAL_ID		ar.k5
#define r_LOCAL_ID		ar.k6
#define r_KERNEL_SP		ar.k7
#define r_PHYS_TCB_ADDR		ar.k3


/**
 * Counter which keeps track of whether kernel stacks are currently
 * valid.  Counter increases by two upon each taken
 * exception/interruption, but may temporarily equal 1 if interruption
 * occurs within a syscall binding.
 *
 *   value == 0 - invalid SP, invalid AR.BSPSTORE
 *   value == 1 - valid SP, invalid AR.BSPSTORE
 *   value >= 2 - valid SP, valid AR.BSPSTORE
 *
 */
#define r_KERNEL_STACK_COUNTER	ar.k4



#endif /* !__GLUE__V4_IA64__REGISTERS_H__ */
