#include "alpha/hwrpb.h"

#include <l4/types.h>


extern "C" int printf(const char *s, ...);


extern "C" int switch_to_osf_pal(L4_Word_t, L4_Word_t, L4_Word_t, L4_Word_t, L4_Word_t);
extern "C" void halt(void);

extern "C" void memset (char * p, char c, int size)
{
    for (;size--;)
	*(p++)=c;
}

extern "C" __attribute__ ((weak)) void *
memcpy (void * dst, const void * src, unsigned int len)
{
    unsigned char *d = (unsigned char *) dst;
    unsigned char *s = (unsigned char *) src;

    while (len-- > 0)
	*d++ = *s++;

    return dst;
}


unsigned long find_pa(unsigned long *vptb, unsigned long *pcb)
{
         unsigned long address = (unsigned long) pcb;
         unsigned long result;
 
         result = vptb[address >> 13];
         result >>= 32;
         result <<= 13;
         result |= address & 0x1fff;
         return result;
}

extern char _start[];
extern char _end[];

/*
 * This function moves into OSF/1 pal-code, and has a temporary
 * PCB for that. The kernel proper should replace this PCB with
 * the real one as soon as possible.
 *
 * The page table muckery in here depends on the fact that the boot
 * code has the L1 page table identity-map itself in the second PTE
 * in the L1 page table. Thus the L1-page is virtually addressable
 * itself (through three levels) at virtual address 0x200802000.
 */
#define pcb_va ((struct pcb_struct *) 0x20000000)
#define old_vptb (0x0000000200000000UL)
void init_pal(void)
{
    unsigned long i, rev;
	unsigned long *L1;
	struct percpu_struct * percpu;
	struct pcb_struct * pcb_pa;

	/* Find the level 1 page table and duplicate it in high memory */
	L1 = (unsigned long *) 0x200802000UL; /* (1<<33 | 1<<23 | 1<<13) */

	/* Bad assumption here that we are running on CPU 0 */
	percpu = (struct percpu_struct *) (INIT_HWRPB->processor_offset
					   + (unsigned long) INIT_HWRPB),
	pcb_va->ksp = 0;
	pcb_va->usp = 0;
	pcb_va->ptbr = L1[1] >> 32;
	pcb_va->asn = 0;
	pcb_va->pcc = 0;
	pcb_va->unique = 0;
	pcb_va->flags = 1;
	pcb_pa = (struct pcb_struct *) find_pa((unsigned long *) old_vptb, (unsigned long *) pcb_va);

	printf("elf-loader:\tswitching to OSF/1 PALcode ");
	/*
	 * a0 = 2 (OSF)
	 * a1 = return address, but we give the asm the virtual addr of the PCB
	 * a2 = physical addr of PCB
	 * a3 = new virtual page table pointer
	 * a4 = KSP (but we give it 0, asm sets it)
	 */
	i = switch_to_osf_pal(
		2,
		(L4_Word_t) pcb_va,
		(L4_Word_t) pcb_pa,
		old_vptb,
		0);
	if (i) {
		printf("--- failed, code %ld\n", i);
		halt();
	}

	rev = percpu->pal_revision = percpu->palcode_avail[2];

	printf("OK: version %ld.%ld\n", (rev >> 8) & 0xff, rev & 0xff);
}
