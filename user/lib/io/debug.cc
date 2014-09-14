#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

void EDebugPrintf(const char *aTag, const char *aText) {
	printf("\n\n[%s] : %s\n\n", aTag, aText);
}

#ifdef __cplusplus
}
#endif

