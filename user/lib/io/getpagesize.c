//Implementation of the getpagesize()

//We'll tidy this later, but just return (4096/4MB in case of default target (x86))

#include <unistd.h>
#include <nwmalloc.h>

#ifdef __cplusplus
extern "C" {
#endif

int getpagesize(void) {
	//return 4096;
	return NWGetPageSize(); 
}


#ifdef __cplusplus
}
#endif

