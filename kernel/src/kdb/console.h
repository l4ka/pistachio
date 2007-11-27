/*********************************************************************
 *                
 * Copyright (C) 2002, 2004, 2007,  Karlsruhe University
 *                
 * File path:     kdb/console.h
 * Description:   Generic KDB console functionality
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
 * $Id: console.h,v 1.3 2004/03/17 19:07:21 skoglund Exp $
 *                
 ********************************************************************/
#ifndef __KDB__CONSOLE_H__
#define __KDB__CONSOLE_H__

void putc (char) SECTION (".kdebug");
char getc (bool block = true) SECTION (".kdebug");
void init_console (void) SECTION (".init");

class kdb_console_t
{
public:
    const char * name;
    void (*init) (void);
    void (*putc) (char c);
    char (*getc) (bool block);
};

#define KDB_NULL_CONSOLE { NULL, NULL, NULL, NULL }

extern kdb_console_t kdb_consoles[];
extern word_t kdb_current_console;


#endif /* !__KDB__CONSOLE_H__ */
