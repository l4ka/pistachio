/*********************************************************************
 *                
 * Copyright (C) 2002-2003,   University of New South Wales
 *                
 * File path:     arch/alpha/console.h
 * Description:   
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
 * $Id: console.h,v 1.4 2003/09/24 19:04:25 skoglund Exp $
 *                
 ********************************************************************/

#ifndef __ARCH__ALPHA__CONSOLE_H__
#define __ARCH__ALPHA__CONSOLE_H__

/*
 * Console callback routine numbers
 */
#define CCB_GETC		0x01
#define CCB_PUTS		0x02
#define CCB_RESET_TERM		0x03
#define CCB_SET_TERM_INT	0x04
#define CCB_SET_TERM_CTL	0x05
#define CCB_PROCESS_KEYCODE	0x06

#define CCB_OPEN		0x10
#define CCB_CLOSE		0x11
#define CCB_IOCTL		0x12
#define CCB_READ		0x13
#define CCB_WRITE		0x14

#define CCB_SET_ENV		0x20
#define CCB_RESET_ENV		0x21
#define CCB_GET_ENV		0x22
#define CCB_SAVE_ENV		0x23

#define CCB_PSWITCH		0x30
#define CCB_BIOS_EMUL		0x32
/*
 * Environment variable numbers
 */
#define ENV_AUTO_ACTION		0x01
#define ENV_BOOT_DEV		0x02
#define ENV_BOOTDEF_DEV		0x03
#define ENV_BOOTED_DEV		0x04
#define ENV_BOOT_FILE		0x05
#define ENV_BOOTED_FILE		0x06
#define ENV_BOOT_OSFLAGS	0x07
#define ENV_BOOTED_OSFLAGS	0x08
#define ENV_BOOT_RESET		0x09
#define ENV_DUMP_DEV		0x0A
#define ENV_ENABLE_AUDIT	0x0B
#define ENV_LICENSE		0x0C
#define ENV_CHAR_SET		0x0D
#define ENV_LANGUAGE		0x0E
#define ENV_TTY_DEV		0x0F

#define NO_SRM_CONSOLE		-1L

/* this isn't in the kernel for some reason */
#define CTB_TYPE_NONE     0
#define CTB_TYPE_DETACHED 1
#define CTB_TYPE_SERIAL   2
#define CTB_TYPE_GRAPHICS 3
#define CTB_TYPE_MULTI    4

struct ctb_struct {
	unsigned long type;
	unsigned long id;
	unsigned long reserved;
	unsigned long dsd_len;
	char dsd[0];
};

void remap_console(void);

#endif /* __ARCH__ALPHA__CONSOLE_H__ */
