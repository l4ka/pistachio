/* 
 * Mach Operating System
 * Copyright (c) 1993-1989 Carnegie Mellon University
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 * 
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS"
 * CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND FOR
 * ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 * 
 * Carnegie Mellon requests users of this software to return to
 * 
 *  Software Distribution Coordinator  or  Software.Distribution@CS.CMU.EDU
 *  School of Computer Science
 *  Carnegie Mellon University
 *  Pittsburgh PA 15213-3890
 * 
 * any improvements or extensions that they make and grant Carnegie Mellon
 * the rights to redistribute these changes.
 */

/*
 * an ANSI compliant atexit function
 */

typedef void (*void_function_ptr)();
void_function_ptr _atexit_functions[32];
int _atexit_index = 0;

int _atexit(function)
     void (*function)();
{
     /*
      * We must support at least 32 atexit functions
      * but we don't have to support any more.
      */
     if (_atexit_index >= 32)
         return -1;

     _atexit_functions[_atexit_index++] = function;
     return 0;
}
     
int atexit(function)
     void (*function)();
{
     return _atexit(function);
}

/*
 * Call atexit functions in reverse order.
 */
void _run_atexits()
{
     int i;

     for (i = _atexit_index - 1; i >= 0; i--)
         (*_atexit_functions[i])();
}

void exit(code)
    int code;
{
    _run_atexits();
    _exit(code);
}
