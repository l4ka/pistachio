/*
 * Arm configuration.
 */


#define ARCH_NAME "Arm"

#define PAGE_BITS		(12)
#define PAGE_SIZE		(1 << PAGE_BITS)
#define MAX_MEM			(256L*1024L*1024L)
#define PAGE_SIZES		(1)

#define STACK_PAGES  4


/* some place to start reading/writing junk */
#define SCRATCHMEM_START 0xC0700000
