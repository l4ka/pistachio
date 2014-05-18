/*

Copyright (c) 2011-2014 Kevin Lange.  All rights reserved.

                          Dedicated to the memory of
                               Dennis Ritchie
                                  1941-2011

Developed by: ToAruOS Kernel Development Team
              http://toaruos.org

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to
deal with the Software without restriction, including without limitation the
rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
  1. Redistributions of source code must retain the above copyright notice,
     this list of conditions and the following disclaimers.
  2. Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimers in the
     documentation and/or other materials provided with the distribution.
  3. Neither the names of the ToAruOS Kernel Development Team, Kevin Lange,
     nor the names of its contributors may be used to endorse
     or promote products derived from this Software without specific prior
     written permission.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
CONTRIBUTORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
WITH THE SOFTWARE.
*/

#include <stdio.h>
#include <lib/io/ia32.h>

#define outportb outb
#define inportb inb
#define uint8_t int
#define uint32_t unsigned long

static void note(int length, int freq) {

	uint32_t div = 11931800 / freq;
	uint8_t  t;


	outportb(0x43, 0xb6);
	outportb(0x42, (uint8_t)(div));
	outportb(0x42, (uint8_t)(div >> 8));

	t = inportb(0x61);
	outportb(0x61, t | 0x3);

	unsigned long s, ss;
	//relative_time(0, length, &s, &ss);
	//sleep_until((process_t *)current_process, s, ss);
	//switch_task(0);

	t = inportb(0x61) & 0xFC;
	outportb(0x61, t);

}

void TetrisTheme() {
	//fprintf(tty, "beep\n");
	printf("beep\n");

	note(20, 15680);
	note(10, 11747);
	note(10, 12445);
	note(20, 13969);
	note(10, 12445);
	note(10, 11747);
	note(20, 10465);
	note(10, 10465);
	note(10, 12445);
	note(20, 15680);
	note(10, 13969);
	note(10, 12445);
	note(30, 11747);
	note(10, 12445);
	note(20, 13969);
	note(20, 15680);
	note(20, 12445);
	note(20, 10465);
	note(20, 10465);

	//return 0;
}

