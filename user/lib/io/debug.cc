#include <stdio.h>

void EDebugPrintf(const char *aTag, const char *aText) {
	printf("\n\n[%s] : %s\n\n", aTag, aText);
}
