#include <nwmalloc.h> 
    
//https://www.novell.com/documentation/developer/clib/prog_enu/data/sdk645.html says that this should usually return  4096 bytes, but that feels wrong on this platform


#ifdef __cplusplus
extern "C" {
#endif

  size_t NWGetPageSize  (void) {

//HACK based on example code, as used in our liballoc port...
#define PAGE_BITS		(12)
#define PAGE_SIZE		(1 << PAGE_BITS)
#define SCRATCHMEM_START        (16*1024*1024)

return PAGE_SIZE;

}


#ifdef __cplusplus
}
#endif

