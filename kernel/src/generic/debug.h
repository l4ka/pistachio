/*********************************************************************
 *                
 * Copyright (C) 2002-2004, 2007-2008,  Karlsruhe University
 *                
 * File path:     generic/debug.h
 * Description:   Debug functions
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
 * $Id: debug.h,v 1.31 2007/01/22 21:00:07 skoglund Exp $
 *                
 ********************************************************************/
#ifndef __DEBUG_H__
#define __DEBUG_H__

#define SEC_KDEBUG ".kdebug"


/*
 * Escape codes for controlling text color, brightness, etc.
 */

#define TXT_CLRSCREEN		"\e[2J"
#define TXT_NORMAL		"\e[0m"
#define TXT_BRIGHT		"\e[1m"
#define TXT_REVERSED		"\e[7m"
#define TXT_FG_BLACK		"\e[30m"
#define TXT_FG_RED		"\e[31m"
#define TXT_FG_GREEN		"\e[32m"
#define TXT_FG_YELLOW		"\e[33m"
#define TXT_FG_BLUE		"\e[34m"
#define TXT_FG_MAGENTA		"\e[35m"
#define TXT_FG_CYAN		"\e[36m"
#define TXT_FG_WHITE		"\e[37m"
#define TXT_BG_BLACK		"\e[40m"
#define TXT_BG_RED		"\e[41m"
#define TXT_BG_GREEN		"\e[42m"
#define TXT_BG_YELLOW		"\e[43m"
#define TXT_BG_BLUE		"\e[44m"
#define TXT_BG_MAGENTA		"\e[45m"
#define TXT_BG_CYAN		"\e[46m"
#define TXT_BG_WHITE		"\e[47m"


#include INC_GLUE(debug.h)

#if defined(CONFIG_DEBUG)

void init_console (void);
extern "C" int printf (const char * format, ...);
tcb_t *get_kdebug_tcb();

# define UNIMPLEMENTED()				\
do {							\
    printf ("\nNot implemented: %s\n%s, line %d\n",	\
	    __PRETTY_FUNCTION__, __FILE__, __LINE__);	\
    for (;;)						\
	enter_kdebug ("unimplemented");			\
} while (false)

#if !defined(CONFIG_KDB_NO_ASSERTS)
# define ASSERT(x)							\
do {									\
    if (EXPECT_FALSE(! (x))) {						\
	printf ("Assertion "#x" failed in file %s, line %d (fn=%p)\n",	\
		__FILE__, __LINE__, __builtin_return_address(0));	\
	enter_kdebug ("assert");					\
    }									\
} while(false)

# define WARNING(fmt, args...)						\
do {									\
    printf ("WARNING: %s, line %d (fn=%p)\n===> " fmt,			\
	    __FILE__, __LINE__, __builtin_return_address(0) , ## args);	\
    enter_kdebug ("warning");						\
} while (false)

# define TRACEF(f, x...)					\
do {								\
    printf ("%s:%d: " f, __FUNCTION__, __LINE__ ,##x);		\
} while(false)
# define TRACE(x...)	printf(x)

# else /* defined(CONFIG_KDB_NO_ASSERTS) */

#define ASSERT(x)
#define TRACE(x...)
#define TRACEF(x...)
#define WARNING(fmt...)

# endif

# define TID(x)		((x).get_raw())

/* From kdb/generic/entry.cc */
void kdebug_entry (void *);

bool kdebug_check_interrupt();

#else /* !CONFIG_DEBUG */

/*
 * Define all functions as empty.
 */

# define init_console(...)
# define printf(fmt, args...)		do { } while (false)
# define enter_kdebug(x)		do { } while (true)
# define get_kdebug_tcb()		(NULL)
# define kdebug_check_interrupt()	(false)
# define UNIMPLEMENTED()		do { } while (true)
# define ASSERT(x)			do { } while (false)
# define WARNING(fmt, args...)		do { } while (false)
# define TRACE(x...)			do { } while (false)
# define TRACEF(x...)			do { } while (false)
# define spin_forever(x...)		do { } while (true)
# define spin(x...)			do { } while (false)

#endif /* CONFIG_DEBUG */



#define panic(x...)					\
do {							\
    printf ("PANIC in %s, %s, line %d:\n===> ",		\
	    __PRETTY_FUNCTION__, __FILE__, __LINE__);	\
    printf (x);						\
    for (;;)						\
	enter_kdebug ("panic");				\
} while (false)

/*
 * Verbose initialization.
 */

#if defined(CONFIG_VERBOSE_INIT)
# define TRACE_INIT(x...)	printf (x)
#else
# define TRACE_INIT(x...)
#endif

/*
 * kdebug break-in
 */
# if defined(CONFIG_KDB_BREAKIN)
void kdebug_check_breakin();
# else
#  define kdebug_check_breakin()
# endif /* CONFIG_DEBUG_BREAKIN */


#endif /* !__DEBUG_H__ */
