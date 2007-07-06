/*********************************************************************
 *                
 * Copyright (C) 2002-2004,  University of New South Wales
 *                
 * File path:     kdb/arch/alpha/console.cc
 * Description:   Console routine for Alpha systems  
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
 * $Id: console.cc,v 1.12 2004/03/17 19:13:23 skoglund Exp $
 *                
 ********************************************************************/

#include <debug.h>
#include <kdb/kdb.h>
#include <kdb/cmd.h>
#include <kdb/console.h>

#include INC_ARCH(hwrpb.h)
#include INC_ARCH(console.h)
#include INC_ARCH(palcalls.h)
#include INC_GLUE(config.h)


static unsigned long cons_dev;

unsigned long hwrpb_vaddr = HWRPB_VADDR;

/* Takes a varied number of args */
extern "C" unsigned long dispatch(word_t a0, word_t a1, word_t a2, word_t a3, struct hwrpb *hwrpb);

static struct hwrpb *hwrpb_to_use = NULL;

extern void halt(void);

void halt(void)
{
    PAL::halt();
}

/* we use this so that we can do without the ctype library */
#define is_digit(c)     ((c) >= '0' && (c) <= '9')
#define is_xdigit(c) \
  (is_digit(c) || ((c) >= 'A' && (c) <= 'F') || ((c) >= 'a' && (c) <= 'f'))
#define is_lower(c)     (((c) >= 'a') && ((c) <= 'z'))
#define to_upper(c)     ((c) + 'a' - 'A')

unsigned long simple_strtoul(const char *cp, char **endp, unsigned int base)
{
        unsigned long result = 0, value;

        if (!base) {
                base = 10;
                if (*cp == '0') {
                        base = 8;
                        cp++;
                        if ((*cp == 'x') && is_xdigit(cp[1])) {
                                cp++;
                                base = 16;
                        }
                }
        }
        while (is_xdigit(*cp)
               && (value =
                   (is_digit(*cp)
                    ? *cp - '0' : ((is_lower(*cp)
                                    ? to_upper(*cp) : *cp) - 'A' + 10)))
                   < base)
        {
                result = result*base + value;
                cp++;
        }
        if (endp)
                *endp = (char *)cp;
        return result;
}

static long cons_puts(const char *str, long len)
{
	long remaining, written;
	union ccb_stsdef {
		long int l_sts;
		struct {
			int written;
			unsigned discard : 29;
			unsigned v_sts0  : 1;
			unsigned v_sts1  : 1;
			unsigned v_err   : 1;
		} s;
	} ccb_sts;
	
	for (remaining = len; remaining; remaining -= written) {
		ccb_sts.l_sts = dispatch((word_t) CCB_PUTS, cons_dev, (word_t) str, remaining, hwrpb_to_use);
		if (!ccb_sts.s.v_err) {
			written = ccb_sts.s.written;
			str += written;
		} else {
			if (ccb_sts.s.v_sts1)
				halt();		/* This is a hard error */
			written = 0;
		}
	}
	return len;
}

void putc_srm(char c)
{
	char buf[2];

	buf[0] = c;
	buf[1] = 0;
	cons_puts(buf,1);
	if (c == '\n')
	    putc_srm('\r');
}

long getc_srm_nonblock(void)
{
    return dispatch((word_t) CCB_GETC, cons_dev, 0, 0, hwrpb_to_use);
}

char getc_srm(bool block)
{
    long c;
    
    while ((c = getc_srm_nonblock()) < 0)
	;
    return c;
}

static long cons_getenv(long index, char *envval, long maxlen)
{
	long len = dispatch((word_t) CCB_GET_ENV, index, (word_t) envval, maxlen - 1, hwrpb_to_use);
	return len;
}

#if 0
static long cons_open(const char *devname)
{
	return dispatch((word_t) CCB_OPEN, devname, strlen(devname), 0, hwrpb_to_use);
}

static long cons_close(long dev)
{
	return dispatch((word_t) CCB_CLOSE, dev, 0, 0, hwrpb_to_use);
}
#endif

#if defined(CONFIG_KDB_BREAKIN)
void SECTION(".kdebug") kdebug_check_breakin(void)
{
    switch(getc_srm_nonblock()) {
    case '\e': /* ESC*/
	enter_kdebug("KDB Breakin");
	return;
	
    default:
    case -1:
	return;
    }	
}
#endif

void init_srm(void)
{
	static char envval[256];

	if(hwrpb_to_use == NULL) {
	    hwrpb_to_use = INIT_HWRPB;

	    if (cons_getenv(ENV_TTY_DEV, (char *) envval, sizeof(envval) * 8) < 0) {
		halt();		/* better than random crash */
	    }

	    cons_dev = simple_strtoul((char *) envval, 0, 10);
	} else {
	    hwrpb_to_use = (struct hwrpb *) CONSOLE_AREA_START;
	}
}
word_t kdb_current_console = 0;

kdb_console_t kdb_consoles[] = {
    { "SRM", &init_srm, &putc_srm, &getc_srm },
    KDB_NULL_CONSOLE
};
