/*********************************************************************
 *                
 * Copyright (C) 2002-2004,  Karlsruhe University
 *                
 * File path:     kdb/generic/input.cc
 * Description:   Various input functions
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
 * $Id: input.cc,v 1.14 2004/03/17 19:16:19 skoglund Exp $
 *                
 ********************************************************************/
#include <debug.h>
#include <kdb/input.h>
#include <kdb/kdb.h>
#include <kdb/console.h>


/**
 * Read hex value from console
 * 
 * @param prompt	prompt
 * @param defnum	default value
 * @param defstr	default string
 *
 * Prompt for a hex value using PROMPT and return the typed in value.
 * If PROMPT is NULL, do not display it.  If user just presses return,
 * DEFNUM will be displayed and returned.  If DEFSTR is non-nil, it
 * will be displayed as a default value instead of DEFNUM.
 * 
 * @returns value read from console
 */
word_t SECTION(SEC_KDEBUG) get_hex (const char * prompt, const word_t defnum, const char * defstr)
{
    word_t num = 0;
    word_t len = 0;
    char c, r;

    if (prompt)
    {
	if (defstr)
	    printf ("%s [%s]: ", prompt, defstr);
	else
	    printf ("%s [0x%lx]: ", prompt, defnum);
    }

    while (len < (sizeof (word_t) * 2))
    {
	switch (r = c = getc ())
	{
	case '0': case '1': case '2': case '3': case '4':
	case '5': case '6': case '7': case '8': case '9':
	    num *= 16;
	    num += c - '0';
	    putc (r);
	    len++;
	    break;

	case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
	    c += 'a' - 'A';
	case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
	    num *= 16;
	    num += c - 'a' + 10;
	    putc (r);
	    len++;
	    break;

	case 'x': case 'X':
	    // Allow "0x" prefix
	    if (len == 1 && num == 0)
	    {
		putc (r);
		len--;
	    }
	    break;

	case '\b':
	    // Backspace
	    if (len > 0)
	    {
		printf ("\b \b");
		num /= 16;
		len--;
	    }
	    break;

	case KEY_RETURN:
	    if (len == 0)
	    {
		// Use default value
		if (defstr)
		    printf ("%s\n", defstr);
		else
		    printf ("%lx\n", defnum);
		return defnum;
	    }
	    len = sizeof (word_t) * 2;
	    break;

	case '\e':
	    putc ('\n');
	    return ABORT_MAGIC;
	}
    }

    putc ('\n');
    return num;
}


/**
 * Read decimal value from console
 * 
 * @param prompt	prompt
 * @param defnum	default value
 * @param defstr	default string
 *
 * Prompt for a decimal value using PROMPT and return the typed in value.
 * If PROMPT is NULL, do not display it.  If user just presses return,
 * DEFNUM will be displayed and returned.  If DEFSTR is non-nil, it
 * will be displayed as a default value instead of DEFNUM.
 * 
 * @returns value read from console
 */
word_t SECTION(SEC_KDEBUG) get_dec (const char * prompt, const word_t defnum, const char * defstr)
{
    word_t num = 0;
    int len = 0;
    char c;

    if (prompt)
    {
	if (defstr)
	    printf ("%s [%s]: ", prompt, defstr);
	else
	    printf ("%s [%d]: ", prompt, defnum);
    }

    for (;;)
    {
	switch (c = getc ())
	{
	case '0': case '1': case '2': case '3': case '4':
	case '5': case '6': case '7': case '8': case '9':
	    num *= 10;
	    num += c - '0';
	    putc (c);
	    len++;
	    break;

	case '\b':
	    // Backspace
	    if (len > 0)
	    {
		printf ("\b \b");
		num /= 10;
		len--;
	    }
	    break;

	case KEY_RETURN:
	    if (len == 0)
	    {
		// Use default value
		if (defstr)
		    printf ("%s\n", defstr);
		else
		    printf ("%d\n", defnum);
		return defnum;
	    }
	    putc ('\n');
	    return num;

	case '\e':
	    putc ('\n');
	    return ABORT_MAGIC;
	}
    }

    /* NOTREACHED */
    return 0;
}



static char * get_key_string (char key, const char * choices);

/**
 * Prompt for a choice
 * 
 * @param prompt	prompt
 * @param choices	string of choices separated by '/'
 * @param def		default choice
 * 
 * Prompt for a choice using PROMPT and return the character
 * associated by the choice.  The choices string consist of choices
 * separated by the '/' character.  An uppercase letter in the choice
 * string indicates the key to be pressed for selecting the choice.  A
 * choice string with a single character also indicates the
 * appropriate key for selecting the choice.  The function does not
 * return until user has selected a valid choice. Be absolutely quiet
 * if PROMPT is NULL.
 * 
 * @returns the character associated with the choice made
 */
char SECTION(SEC_KDEBUG) get_choice (const char * prompt, const char * choices, char def)
{
    char *p, c;

    if (prompt)
    {
	printf ("%s (%s)", prompt, choices);
	
	if ((p = get_key_string (def, choices)))
	{
	    printf (" [");
	    for (; *p != '/' && *p != 0; p++)
		putc (*p >= 'A' && *p <= 'Z' ? (*p + ('a'-'A')) : *p);
	    putc (']');
	}

	printf (": ");
    }

    for (;;)
    {
	c = getc ();
	p = get_key_string (c == KEY_RETURN ? def : c, choices);
	for (; p && *p != '/' && *p != 0; p++)
	    if (prompt)
		putc (*p >= 'A' && *p <= 'Z' ? (*p + ('a'-'A')) : *p);
	if (p || c == KEY_RETURN)
	    break;
    }

    if (prompt)
	putc ('\n');

    return (c == KEY_RETURN) ? def : c;
}

static char SECTION(SEC_KDEBUG) * get_key_string (char key, const char * choices)
{
    const char * p;

    if (key == 0)
	return NULL;

    for (p = choices; *p; p++)
    {
	if ((*p >= 'A' && *p <= 'Z') && (*p + ('a'-'A')) == key)
	    break;
	if ((p == choices || p[-1] == '/') && (p[1] == 0 || p[1] == '/') &&
	    (*p == key))
	    break;
    }

    if (*p == 0)
	return NULL;

    while (p > choices && p[-1] != '/')
	p--;

    return (char *) p;
}
