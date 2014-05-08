/*********************************************************************
 *                
 * Copyright (C) 2003-2004,  Karlsruhe University
 *                
 * File path:     grabmem/roottask.cc
 * Description:   A simple root task
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
 * $Id: grabmem.cc,v 1.2 2004/02/26 19:11:07 skoglund Exp $
 *                
 ********************************************************************/


//ATA mini driver
#include <mindrvr.h>

#include <l4io.h>
#include <l4/sigma0.h>
#include <l4/kdebug.h>

/* Track the environment status */
#define ACTIVE_CMD 0
#define CMD_RESULT 1

#define FINISHED 0x00
#define FAILED 0x01
#define WAITING 0x02

#define KB(x) (x*1024)
#define MB(x) (x*1024*1024)
#define GB(x) (x*1024*1024*1024)

//http://www.a1k0n.net/2011/06/26/obfuscated-c-yahoo-logo.html
//http://forge.voodooprojects.org/p/chameleon/source/tree/2261/branches/prasys/i386/libsaio/cddrvr.c
//http://mirror.fsf.org/pmon2000/2.x/src/include/ctype.h
//http://stackoverflow.com/questions/8156603/is-usleep-in-c-implemented-as-busy-wait
//http://www.dreamincode.net/forums/topic/192079-command-line-interpreter/

#define	toascii(c)	((c) & 0177)

void InitHwDev() {
	printf("[root-task] : Found %d PATA devices", reg_config());
}

int Beep() {

	int status = FINISHED;
	RingTheBell();
	
	return status;

}



int main (void) {

char *iEnvStatus[255];

InitHwDev();

iEnvStatus[CMD_RESULT] = (char*)WAITING;
iEnvStatus[ACTIVE_CMD] = GetPolledKbdLine();

printf(iEnvStatus[ACTIVE_CMD]);


while(WAITING) {

//if (iEnvStatus[ACTIVE_CMD] == "beep\0" || iEnvStatus[ACTIVE_CMD] == "beep"
//|| iEnvStatus[ACTIVE_CMD] == "beep\0\n" || iEnvStatus[ACTIVE_CMD] == "beep\n") {
if (obsd_strcmp(iEnvStatus[ACTIVE_CMD], "beep") == 0)
{
printf("Entered beep\n");
	iEnvStatus[CMD_RESULT] = (char*)Beep();
	
	printf("\n%s\n\n", (char*)iEnvStatus[CMD_RESULT]);

	//Return to prompt
	iEnvStatus[CMD_RESULT] = (char*)WAITING;
	printf(iEnvStatus[ACTIVE_CMD]);
	
	iEnvStatus[ACTIVE_CMD] = GetPolledKbdLine();
	
	
}

 else {


	iEnvStatus[CMD_RESULT] = (char*)WAITING;
	printf("Entered: %s", iEnvStatus[ACTIVE_CMD]);
	iEnvStatus[ACTIVE_CMD] = GetPolledKbdLine();	
	
}

}

	return 0;
}



