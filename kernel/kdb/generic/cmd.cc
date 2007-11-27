/*********************************************************************
 *                
 * Copyright (C) 2002, 2004, 2007,  Karlsruhe University
 *                
 * File path:     kdb/generic/cmd.cc
 * Description:   Command dialogs and standard command functions.
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
 * $Id: cmd.cc,v 1.16 2004/03/17 19:16:19 skoglund Exp $
 *                
 ********************************************************************/
#include <debug.h>
#include <kdb/kdb.h>
#include <kdb/cmd.h>
#include <kdb/console.h>


/* Forward declared functions. */
static int strncmp (char * s1, char * s2, int len);
static void print_cmd_path (cmd_group_t * cg);



/**
 * cmd_group_t::interact_by_key: Do user interaction by simple keystrokes.
 */
cmd_t SECTION(SEC_KDEBUG) * cmd_group_t::interact_by_key (void)
{
    cmd_t * cmd;

    /*
     * Loop until user performs a valid keystroke.
     */
    do {
	char c = getc ();
	reset ();
	while ((cmd = next ()) != NULL)
	    if (cmd->key == c)
		break;
    } while (cmd == NULL);
    printf ("%s\n", cmd->command);

    return cmd;
}


/**
 * cmd_group_t::interact_by_command: Do user interaction by command line.
 */
cmd_t SECTION(SEC_KDEBUG) * cmd_group_t::interact_by_command (void)
{
    char cmdstr[64], c;
    cmd_t * cmd;
    word_t cmdlen = 0;

    /*
     * Loop until user specifies a valid command.
     */
    for (;;)
    {
	/* Loop until return key is pressed */
	do {
	    switch (c = getc ())
	    {
	    case KEY_TAB:
	    {
		/* Check number of matching commands */
		cmd_t * match = NULL;
		int nummatch = 0;
		reset ();
		while ((cmd = next ()) != NULL)
		    if (strncmp ((char *) cmd->command, cmdstr, cmdlen) == 0)
			match = cmd, nummatch++;
	    
		if (nummatch == 1)
		{
		    /* Only one match.  Do completion */
		    printf ("%s", match->command + cmdlen);
		    for (cmdlen = 0; match->command[cmdlen] != 0; cmdlen++)
			cmdstr[cmdlen] = match->command[cmdlen];
		    cmdstr[cmdlen] = 0;
		    break;
		}
		else if (nummatch >= 1)
		{
		    /* Print list of matching commands */
		    putc ('\n');
		    reset ();
		    while ((cmd = next ()) != NULL)
			if (strncmp ((char *) cmd->command, cmdstr, cmdlen) == 0)
			    printf ("%s\n", cmd->command);
		    cmdstr[cmdlen] = 0;
		    printf (TXT_BRIGHT);
		    print_cmd_path (this);
		    printf ("> " TXT_NORMAL "%s", cmdstr);
		}
		break;
	    }
	    case KEY_BS:
		if (cmdlen > 0)
		{
		    printf ("\b \b");
		    cmdlen--;
		}
		break;
	    case KEY_RETURN:
		putc ('\n');
		break;
	    default:
		if (cmdlen < sizeof (cmdstr) - 1)
		{
		    cmdstr[cmdlen++] = c;
		    putc (c);
		}
	    }
	} while (c != KEY_RETURN);

	/* Check for matching command */
	reset ();
	while ((cmd = next ()) != NULL)
	{
	    if (strncmp ((char *) cmd->command, cmdstr, cmdlen) == 0 &&
		cmd->command[cmdlen] == 0)
		return cmd;
	}

	cmdstr[cmdlen] = 0;
	if (cmdlen > 0)
	    printf ("Unknown command: %s\n", cmdstr);
	printf (TXT_BRIGHT);
	print_cmd_path (this);
	printf ("> " TXT_NORMAL);
	cmdlen = 0;
    }

    /* NOTREACHED */
    return NULL;
}


/**
 * cmd_group_t::interact: Perform user interaction on command group.
 */
cmd_ret_t SECTION(SEC_KDEBUG) cmd_group_t::interact (cmd_group_t * myparent, const char * myname)
{
    cmd_t * cmd;

    parent = myparent;
    name = myname;

    for (;;)
    {
	printf (TXT_BRIGHT);
	print_cmd_path (this);
	printf ("> " TXT_NORMAL);

	/* Determine command  */
	if (kdb.kdb_cmd_mode == CMD_KEYMODE)
	    cmd = interact_by_key ();
	else
	    cmd = interact_by_command ();

	/* Execute command */
	cmd_ret_t r = cmd->function (this);
	if (r == CMD_QUIT)
	    return r;
	else if (r == CMD_ABORT)
	    return CMD_NOQUIT;
    }
}


/**
 * cmd__help: Display help message for current command group.
 */
CMD(cmd__help, cg)
{
    cmd_t * cmd;

    cg->reset ();
    while ((cmd = cg->next ()) != NULL)
    {
	if (kdb.kdb_cmd_mode == CMD_KEYMODE)
	{
	    switch (cmd->key) {
	    case KEY_RETURN:	printf (" RET "); break;
	    case KEY_ESC:	printf (" ESC "); break;
	    case KEY_BS:	printf ("  BS "); break;
	    case ' ':		printf (" SPC "); break;
	    default:		printf ("  %c  ", cmd->key);
	    }
	}
	else
	{
	    const int width = 12;
	    int n = width - printf ("%s ", cmd->command);
	    while (n-- > 0)
		printf (" ");
	}
	printf ("- %s\n", cmd->description);
    }

    return CMD_NOQUIT;
}


/**
 * cmd__abort: Back up to previous command group.
 */
CMD(cmd__abort, cg)
{
    return CMD_ABORT;
}


/**
 * cmd__prior: Back up to previous command group.
 */
CMD(cmd__prior, cg)
{
    return CMD_ABORT;
}


/**
 * cmd_mode_switch: Change kernel debugger interaction mode.
 */
DECLARE_CMD (cmd_mode_switch, config, 'm', "modeswitch",
	     "change kernel debugger operation mode");

CMD(cmd_mode_switch, cg)
{
    if (kdb.kdb_cmd_mode == CMD_KEYMODE)
    {
	kdb.kdb_cmd_mode = CMD_LINEMODE;
	printf ("KDB mode: Command line\n");
    }
    else
    {
	kdb.kdb_cmd_mode = CMD_KEYMODE;
	printf ("KDB mode: Keystroke\n");
    }

    return CMD_NOQUIT;
}




/*
 * Helper functions.  Located at end to avoid inlining.
 */

static int SECTION(SEC_KDEBUG) strncmp (char * s1, char * s2, int len)

{
    for (int i = 0; i < len; i++)
	if (s1[i] != s2[i])
	    return s2[i] - s1[i];
    return 0;
}

static void SECTION(SEC_KDEBUG) print_cmd_path (cmd_group_t * cg)
{
    if (cg->parent != NULL)
    {
	print_cmd_path (cg->parent);
	printf ("/");
    }
    printf ("%s", cg->name);
}
