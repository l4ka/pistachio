#ifndef __STDIO_H__
#define __STDIO_H__
#include <l4io.h>

#define EOF (-1)
#include <sys/types.h>

//Be BeOS-compatible...
int is_computer_on();

extern "C" {
	//Print debugging text with a file name/class tag
	void EDebugPrintf(const char *aTag, const char *aText);
}

#endif /* !__STDIO_H__ */

