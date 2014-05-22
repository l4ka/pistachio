/*********************************************************************
 *                
 * Copyright (C) 2003-2004,  Karlsruhe University
 * Copyright (C) 2014, Tyson Key <tyson.key@gmail.com>
 *                
 * File path:     counter/counter.cc
 * Description:   System root task (superserver)
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

#include <liballoc.h>
//#include <cstring>

//ATA mini driver
#include <mindrvr.h>
#include "elmfat/src/diskio.h"
#include "elmfat/src/ff.h"

#include <l4io.h>
#include <l4/sigma0.h>
#include <l4/kdebug.h>

//ToAru PC speaker shim, move later
#include <pcspkr_shim.h>

/* Track the environment status */
#define ACTIVE_CMD 0
#define CMD_RESULT 1

#define FINISHED 0x00
#define FAILED 0x01
#define WAITING 0x02
#define RUNNING 0x03

#define KB(x) (x*1024)
#define MB(x) (x*1024*1024)
#define GB(x) (x*1024*1024*1024)

#define stdin GetPolledKbdLine()

//http://www.a1k0n.net/2011/06/26/obfuscated-c-yahoo-logo.html
//http://forge.voodooprojects.org/p/chameleon/source/tree/2261/branches/prasys/i386/libsaio/cddrvr.c
//http://mirror.fsf.org/pmon2000/2.x/src/include/ctype.h
//http://stackoverflow.com/questions/8156603/is-usleep-in-c-implemented-as-busy-wait
//http://www.dreamincode.net/forums/topic/192079-command-line-interpreter/

#define	toascii(c)	((c) & 0177)

/* Pull in the string handling functions from lib/io/lib.h, since linking/headers are dodgy... */
unsigned strlen( const char *src )
{
    unsigned cnt = 0;

    while( src && src[cnt] )
	cnt++;
    return cnt;
}
/* End for now */

//https://github.com/toddfries/OpenBSD-lib-patches/edit/master/libc/stdio/fgets.c#

int ShellHelp() {
	printf("\n\n This shell supports the following commands: \n");
	
	printf("\n\t * beep          : Beep the PC speaker.\n");
	printf("\n\t * help, h, Help : Print this shell help notice.\n");
	printf("\n\t * malloc_test_1 : Test the liballoc port (should return 25).\n");
	printf("\n\t * shiritori     : Start the Shiritori game \(buggy!\).\n");
	printf("\n\t * tettheme      : Play part of the Tetris theme. \n");

	printf("\n\n");

printf("\n다른 여자 만나니까 좋더라\n");

printf("Raw system clock time: %d\n\n",L4_SystemClock().raw);


return FINISHED;	
}


//http://www.codingunit.com/c-tutorial-the-functions-malloc-and-free
int MallocTestOne() {
	int *ptr_one;

		ptr_one = (int *)malloc(sizeof(int));

		if (ptr_one == 0)
		{
			printf("ERROR: Out of memory\n");
			return FAILED;
		}

		*ptr_one = 25;
		printf("%d\n", *ptr_one);

		free(ptr_one);

		return FINISHED;
}

void InitHwDev() {

//TetrisTheme();
	//printf("[root-task] : Found %d PATA devices", reg_config());
/* static int exec_pio_data_in_cmd(
 unsigned char dev,
unsigned char * bufAddr,
long numSect, int multiCnt ) 
CMD_READ_SECTORS http://www.t13.org/documents/UploadedDocuments/project/d1410r3b-ATA-ATAPI-6.pdf

int reg_pio_data_in_lba28(0, CMD_READ_SECTORS, 0x0, 0x00, LBA?, placeToDump, 0x00, 0x00)

int reg_pio_data_in_lba28( unsigned char dev,         // device (0 or 1)
 
                           unsigned char cmd,         // command register
 
                           int fr,                    // feature register
 
                           int sc,                    // sector count
 
                           long lba,                  // LBA
 
                           unsigned char * bufAddr,   // buffer address
 
                           int numSect,               // number of sectors to transfer
 
                           int multiCnt );            // current multiple count
 

*/

	//printf("[root-task] : PATA device 0 reset status: %d ", reg_reset(0));
FATFS* fileSys;
//int result = exec_pio_data_in_cmd(0,
disk_initialize (0);

printf("[root-task] : f_mount(default drive, %d)",f_mount(fileSys, NULL, 1));
//char[] firstSector;
//disk_read (0, firstSector, 0, 1);

}

int Beep() {

	int status = FINISHED;
	RingTheBell();
	
	return status;

}
//http://stackoverflow.com/questions/4346598/gets-function-in-c
char * sgets(char *buffer, int size)
{
	return GetPolledKbdLine(); //HAX
}

//http://detail.chiebukuro.yahoo.co.jp/qa/question_detail/q1390514758
int ShiritoriGame() {

//#include <stdio.h>
//#include <string.h>

#define MAX 30

//int main(int argc, char *argv[]){
char data[MAX][200] = {0}; /* 入力データ保存用 */
int i,nIdx,n;

for(i=0;i<MAX;i++){
printf("%02d position：",i+1);
sgets(data[i],  sizeof(data[i]));
//data[MAX][i] = atoi(GetPolledKbdLine());
n=strlen(data[i])-3;  /* 文末の位置 */
if( memcmp( &data[i][n], "n", 1) == 0){
printf("Quitting from pressing '\n\' \n" );
break;
}
if( i!=0 && /* 初回は比較しない */ 
memcmp( &data[i-1][nIdx], &data[i][0], 1) != 0){ /* 今回の文頭と前回の文末を比較*/
printf("Quit from disconnected\n" );
break;
}
nIdx =n;
}
if(i==MAX){
printf("%d times of quit\n",MAX );
}
return 0;
}
 

int main (void) {

char *iEnvStatus[255];

//auto shiny = 0;
// int *car = new int;
//car = (int)shiny;

//car = 0;

InitHwDev();

iEnvStatus[CMD_RESULT] = (char*)WAITING;
iEnvStatus[ACTIVE_CMD] = GetPolledKbdLine();

printf(iEnvStatus[ACTIVE_CMD]);


while(WAITING) {

if (obsd_strcmp(iEnvStatus[ACTIVE_CMD], "tettheme") == 0) {
	
	iEnvStatus[CMD_RESULT] = (char*)TetrisTheme();

       //Return to prompt
        iEnvStatus[CMD_RESULT] = (char*)WAITING;
        printf(iEnvStatus[ACTIVE_CMD]);

        iEnvStatus[ACTIVE_CMD] = GetPolledKbdLine();


}

if (obsd_strcmp(iEnvStatus[ACTIVE_CMD], "beep") == 0)
{
printf("Entered beep\n");
	iEnvStatus[CMD_RESULT] = (char*)Beep();
	
	printf("\n%d\n\n", (char*)iEnvStatus[CMD_RESULT]);

	//Return to prompt
	iEnvStatus[CMD_RESULT] = (char*)WAITING;
	printf(iEnvStatus[ACTIVE_CMD]);
	
	iEnvStatus[ACTIVE_CMD] = GetPolledKbdLine();
	
}


//Do a malloc test
if (obsd_strcmp(iEnvStatus[ACTIVE_CMD], "malloc_test_1") == 0) 
{
	MallocTestOne();

}


//Help the user...
if (obsd_strcmp(iEnvStatus[ACTIVE_CMD], "help") == 0 || 
obsd_strcmp(iEnvStatus[ACTIVE_CMD], "h") == 0 ||
obsd_strcmp(iEnvStatus[ACTIVE_CMD], "Help") == 0 ) 
{
	ShellHelp();

}


if (obsd_strcmp(iEnvStatus[ACTIVE_CMD], "shiritori") == 0)
{
printf("Entered shiritori\n");
	iEnvStatus[CMD_RESULT] = (char*)ShiritoriGame();
	
	//Return to prompt
	iEnvStatus[CMD_RESULT] = (char*)RUNNING;
	
}

 else {

	printf("\n\n[root-task] : Unsupported command, please type \"help\", or wait...\n\n");
	ShellHelp();

	iEnvStatus[CMD_RESULT] = (char*)WAITING;

	iEnvStatus[ACTIVE_CMD] = GetPolledKbdLine();	
		printf("Entered: %s", iEnvStatus[ACTIVE_CMD]);
}

}

	return 0;
}



