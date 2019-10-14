/* Copyright (c) 2002-2006 Lucent Technologies; see LICENSE */
/* Copyright (c) 2004 Google Inc.; see LICENSE */

#include <stdio.h>
#include <stdarg.h>
#include <utf.h>
#include "plan9.h"
#include "fmt.h"
#include "fmtdef.h"

int
main(int argc, char *argv[])
{
	quotefmtinstall();
	print("hello world\n");
	print("x: %x\n", 0x87654321);
	print("u: %u\n", 0x87654321);
	print("d: %d\n", 0x87654321);
	print("s: %s\n", "hi there");
	print("q: %q\n", "hi i'm here");
	print("c: %c\n", '!');
	print("g: %g %g %g\n", 3.14159, 3.14159e10, 3.14159e-10);
	print("e: %e %e %e\n", 3.14159, 3.14159e10, 3.14159e-10);
	print("f: %f %f %f\n", 3.14159, 3.14159e10, 3.14159e-10);
	print("smiley: %C\n", (Rune)0x263a);
	print("%g %.18g\n", 2e25, 2e25);
	print("%2.18g\n", 1.0);
	print("%2.18f\n", 1.0);
	print("%f\n", 3.1415927/4);
	print("%d\n", 23);
	print("%i\n", 23);
	print("%0.10d\n", 12345);

	/* test %4$d formats */
	print("%3$d %4$06d %2$d %1$d\n", 444, 333, 111, 222);
	print("%3$d %4$06d %2$d %1$d\n", 444, 333, 111, 222);
	print("%3$d %4$*5$06d %2$d %1$d\n", 444, 333, 111, 222, 20);
	print("%3$hd %4$*5$06d %2$d %1$d\n", 444, 333, (short)111, 222, 20);
	print("%3$lld %4$*5$06d %2$d %1$d\n", 444, 333, 111LL, 222, 20);

	/* test %'d formats */
	print("%'d %'d %'d\n", 1, 2222, 33333333);
	print("%'019d\n", 0);
	print("%08d %08d %08d\n", 1, 2222, 33333333);
	print("%'08d %'08d %'08d\n", 1, 2222, 33333333);
	print("%'x %'X %'b\n", 0x11111111, 0xabcd1234, 12345);
	print("%'lld %'lld %'lld\n", 1LL, 222222222LL, 3333333333333LL);
	print("%019lld %019lld %019lld\n", 1LL, 222222222LL, 3333333333333LL);
	print("%'019lld %'019lld %'019lld\n", 1LL, 222222222LL, 3333333333333LL);
	print("%'020lld %'020lld %'020lld\n", 1LL, 222222222LL, 3333333333333LL);
	print("%'llx %'llX %'llb\n", 0x111111111111LL, 0xabcd12345678LL, 112342345LL);
	return 0;
}
