/*********************************************************************
 *                
 * Copyright (C) 2002,  University of New South Wales
 *                
 * File path:     kdb/platform/dp264/console.cc
 * Description:   Console routine for Alpha systems  
 *                
 * @License@
 *                
 * $Id: console.cc,v 1.1 2003/04/30 17:20:17 sjw Exp $
 *                
 ********************************************************************/

#include "alpha/hwrpb.h"
#include "alpha/console.h"
#include <l4/alpha/pal.h>

#include <l4/types.h>

static unsigned long cons_dev;

/* Takes a varied number of args */
extern "C" unsigned long dispatch(L4_Word_t a0, L4_Word_t a1, L4_Word_t a2, L4_Word_t a3, hwrpb_struct *hwrpb);
extern "C" void halt(void);

static hwrpb_struct *hwrpb_to_use = INIT_HWRPB;

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
		ccb_sts.l_sts = dispatch((L4_Word_t) CCB_PUTS, cons_dev, (L4_Word_t) str, remaining, hwrpb_to_use);
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

extern "C" void putc(char c)
{
	char buf[2];

	buf[0] = c;
	buf[1] = 0;
	cons_puts(buf,1);

	if(c == '\n') 
	    putc('\r');
}

extern "C" char getc(void)
{
	long c;

	while ((c = dispatch((L4_Word_t) CCB_GETC, cons_dev, 0, 0, hwrpb_to_use)) < 0)
		;
	return c;
}

static long cons_getenv(long index, char *envval, long maxlen)
{
	long len = dispatch((L4_Word_t) CCB_GET_ENV, index, (L4_Word_t) envval, maxlen - 1, hwrpb_to_use);
	return len;
}

static char envval[256];
void init_console(void)
{
	hwrpb_to_use = INIT_HWRPB;

	if (cons_getenv(ENV_TTY_DEV, (char *) envval, sizeof(envval) * 8) < 0) {
	    halt();		/* better than random crash */
	}
	cons_dev = simple_strtoul((char *) envval, 0, 10);
}
