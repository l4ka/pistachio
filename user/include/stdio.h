#ifndef __STDIO_H__
#define __STDIO_H__
#include <l4io.h>

#define EOF (-1)
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

//This is a hack, we should replace it with something nicer, ASAP
extern int *stdin;
extern int *stdout;
extern int *stderr;

//Be BeOS-compatible...
int is_computer_on();

//Print debugging text with a file name/class tag
void EDebugPrintf(const char *aTag, const char *aText);

#ifdef __cplusplus
}
#endif

#endif /* !__STDIO_H__ */

