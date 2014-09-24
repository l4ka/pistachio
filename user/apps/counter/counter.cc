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
//http://cvs.savannah.gnu.org/viewvc/*checkout*/hurd/hurd-l4/libc/hurd-l4/sysdeps/l4/gettimeofday.c?revision=1.1

//Include the internal shell header...
#include "internalshell.h"
#include <sys/utsname.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
 
#define KB(x) (x*1024)
#define MB(x) (x*1024*1024)
#define GB(x) (x*1024*1024*1024)

#define stdin GetPolledKbdLine()

//http://www.a1k0n.net/2011/06/26/obfuscated-c-yahoo-logo.html
//http://forge.voodooprojects.org/p/chameleon/source/tree/2261/branches/prasys/i386/libsaio/cddrvr.c
//http://stackoverflow.com/questions/8156603/is-usleep-in-c-implemented-as-busy-wait
//http://www.dreamincode.net/forums/topic/192079-command-line-interpreter/

//https://github.com/toddfries/OpenBSD-lib-patches/edit/master/libc/stdio/fgets.c#

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

int CrushTest() {
int *p;
while(1) {
    int inc=1024*1024*sizeof(char);
    p=(int*) calloc(1,inc);
    if(!p) break;

int counter = 0;
//counter++;

printf("%d iterations \n", counter++); 

    }
}

int CrushTest2() {
  int Mb = 0;
  while ( malloc(1<<20)) ++Mb;
  printf("Allocated %d Mb total\n", Mb);
return RUNNING;
}

/*

//http://www.club.cc.cmu.edu/~cmccabe/blog_strerror.html

void strerror_r_improved(int err, char *str, size_t str_len)
{
    if (err < sys_nerr)
        snprintf(str, str_len, "%s", sys_errlist[err]);
    else
        snprintf(str, str_len, "Unknown error %d", err);
}

*/

void InitHwDev() {

FATFS* fileSys;

disk_initialize (0);

printf("\n\n[root-task] : f_mount(default drive, %d)\n\n",f_mount(fileSys, "drive0:/", 1));

BYTE iData;



for (int pos = 0; pos < 250; pos++) {

int readRes = disk_read(0, &iData, pos, 250);
printf("\n[counter] : Result of reading sector is %d\n", readRes);
printf("%x",iData);
}

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
#define MAX 30

//int main(int argc, char *argv[]){
char data[MAX][200] = {0}; /* 入力データ保存用 */
int i,nIdx,n;

for(i=0;i<MAX;i++){
printf("%02d position：",i+1);
sgets(data[i],  sizeof(data[i]));
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
int *test = new int(1);
char *iEnvStatus[255];
struct utsname blah;
InitHwDev();
iEnvStatus[CMD_RESULT] = (char*)WAITING;
iEnvStatus[ACTIVE_CMD] = GetPolledKbdLine();

//setenv("CMD_RESULT", (char*)WAITING, 1);
setenv("ACTIVE_CMD", iEnvStatus[ACTIVE_CMD], 1);

//printf(iEnvStatus[ACTIVE_CMD]);

printf(getenv("ACTIVE_CMD"));

setenv("NIXNADA", "Bada bing", 1);

printf(getenv("NIXNADA"));

//Try initialising uname

uname(&blah);
printf("\n\nToday's recipe is brought to you by %s.\n A car you can trust!\n\n", blah.sysname);

//Lambda example from http://www.drdobbs.com/cpp/lambdas-in-c11/240168241?pgno=1
auto sum = [](int x, int y) -> int { return x + y; };

printf("\n\n%d\n\n",sum(1,2));

    InternalShell ish;

while(WAITING) {

if (obsd_strcmp(iEnvStatus[ACTIVE_CMD], "strtod") == 0) {
	
	iEnvStatus[CMD_RESULT] = (char*)RUNNING;
//http://www.cplusplus.com/reference/cstdlib/strtod/
  char szOrbits[] = "365.24 29.53";
  char* pEnd;
  double d1, d2;
  d1 = strtod (szOrbits, &pEnd);
  d2 = strtod (pEnd, NULL);
  printf ("The moon completes %.2f orbits per Earth year.\n", d1/d2);

       //Return to prompt
        iEnvStatus[CMD_RESULT] = (char*)WAITING;
        printf(iEnvStatus[ACTIVE_CMD]);

        iEnvStatus[ACTIVE_CMD] = GetPolledKbdLine();
}


if (obsd_strcmp(iEnvStatus[ACTIVE_CMD], "memcrush") == 0) {
	
	iEnvStatus[CMD_RESULT] = (char*)CrushTest();

       //Return to prompt
        iEnvStatus[CMD_RESULT] = (char*)WAITING;
        printf(iEnvStatus[ACTIVE_CMD]);

        iEnvStatus[ACTIVE_CMD] = GetPolledKbdLine();
}

if (obsd_strcmp(iEnvStatus[ACTIVE_CMD], "memcrush2") == 0) {
	
	iEnvStatus[CMD_RESULT] = (char*)CrushTest2();

       //Return to prompt
        iEnvStatus[CMD_RESULT] = (char*)WAITING;
        printf(iEnvStatus[ACTIVE_CMD]);

        iEnvStatus[ACTIVE_CMD] = GetPolledKbdLine();
}

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

    ish.ShellHelp();

}


if (obsd_strcmp(iEnvStatus[ACTIVE_CMD], "shiritori") == 0)
{
printf("Entered shiritori\n");
	iEnvStatus[CMD_RESULT] = (char*)ShiritoriGame();
	
	//Return to prompt
	iEnvStatus[CMD_RESULT] = (char*)RUNNING;
	
}

 else {

    EDebugPrintf("root-task", "Unsupported command, please type \"help\", or wait...");
    ish.ShellHelp();

	iEnvStatus[CMD_RESULT] = (char*)WAITING;

	iEnvStatus[ACTIVE_CMD] = GetPolledKbdLine();	
		printf("Entered: %s", iEnvStatus[ACTIVE_CMD]);
}

}

	return 0;
}



