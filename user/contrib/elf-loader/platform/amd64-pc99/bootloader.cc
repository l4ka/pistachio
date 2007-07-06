/********************************************************************
 *                
 * Copyright (C) 2002-2004,  Karlsruhe University
 *                
 * File path:     bootloader.cc
 * Description:   Wraps the kernel binary in an ELF32 file - 
 *                nothing more  
 *                
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *                
 * $Id: bootloader.cc,v 1.2 2006/10/19 22:57:40 ud3 Exp $
 *                
 ********************************************************************/

#include "globals.h"


/* L4µK */
static char kip_magic[] = {'L', '4', 230, 'K'};


/* Kernel Fn */
typedef  __attribute__ ((__noreturn__)) void (* kernel_entry_t)(void);

/* This is a hack to save mb_magic and mb_info to a 
 * in case we rerun the kernel without rerunning grub...
 */
//#define DEBUG_THE_LOADER
#if defined(DEBUG_THE_LOADER)
struct multiboot_info **mb_info_save;
unsigned int *mb_magic_save;
#endif

void spin(unsigned int pos)
{
    while(1)
    {
	((unsigned char *)(0xb8000))[pos] += 1;
    }
}

extern "C" void startup(struct multiboot_info *mb_info, unsigned int mb_magic ){

    printf("\033[2J\n");
    printf("Hello, I'm your Bootloader, ...\n");

    Elf64_Ehdr *kernel_ehdr, *module_ehdr;
    L4_Word64_t dummy;
    L4_Word64_t kernel_entry_addr;

    
    /* Check Kernel */
    if ((kernel_ehdr = valid_elf64((L4_Word_t)_binary_kernel_mod_start, &kernel_entry_addr)) == NULL){
	printf("PANIC: Wrong kernel image format at 0x%x!\n", (L4_Word_t) &_binary_kernel_mod_start);
	spin(1);
    }
    
    /* Load kernel */
    L4_Word64_t kernel_start;
    L4_Word64_t kernel_end;
    elf64_install_image(kernel_ehdr, &kernel_start, &kernel_end);
    
    kip_t *kip = 0;
    
    for (L4_Word_t p = (L4_Word_t) kernel_start & ~0xFFF; p <  (L4_Word_t)  kernel_end; p += 0x1000)
	if (!strncmp( ((kip_t *)p)->magic.string, kip_magic, 4)){
	    kip = (kip_t*) p;
	    break;
	}

    if (kip == 0){
	printf("PANIC: Did not find valid Kernel Interface Page!\n");
	spin(1);
    }	 
     
    printf("Kernel Interface Page is at 0x%x\n", (L4_Word_t) kip);
     
     
    if (kip->api_version.version == 0x84){
	printf("API Version %x, Subversion = %x\n", kip->api_version.version, kip->api_version.subversion);
    }	 
    else if (kip->api_version.version == 0x04 ){
	printf("API Version %x, Subversion = %x\n", kip->api_version.version, kip->api_version.subversion);
    }	 
    else{
	printf("PANIC: Wrong API Version !\n");
	printf("API Version %x, Subversion = %x\n", kip->api_version.version, kip->api_version.subversion);
	spin(1);
    }	 
     
     

#if defined(DEBUG_THE_LOADER)
    mb_magic_save = (unsigned int *) 0x500000;
    mb_info_save = (struct multiboot_info **) 0x600000;
     
    if (*mb_magic_save == MULTIBOOT_BOOTLOADER_MAGIC){
	mb_magic = *mb_magic_save;
	mb_info = *mb_info_save;
    }
    else{ 
	*mb_magic_save = mb_magic;
	*mb_info_save = mb_info;
    }
#endif
     
    /* Find sigma0, root_task */
    if (mb_magic != MULTIBOOT_BOOTLOADER_MAGIC){
	printf("PANIC: Did not find valid Multiboot Info!\n");
	spin(1);
    }
     
    if (!(mb_info->flags & MULTIBOOT_FLAGS_MODS) || (mb_info->mods_count < 2)){
	printf("PANIC: No or not enough modules available! (mods_count=%d)\n",\
	       (unsigned int)mb_info->mods_count);
	spin(1);
    }

    printf ("mods_count = %d, mods_addr = 0x%x\n",
	    (int) mb_info->mods_count, (int) mb_info->mods_addr);

    module_t *modules = (module_t *) mb_info->mods_addr;

    /* sigma0 */
    L4_Word64_t sigma0_entry;
    L4_Word64_t sigma0_start;
    L4_Word64_t sigma0_end;
      
    if ((module_ehdr = valid_elf64(modules[0].mod_start, &sigma0_entry)) == NULL){
	printf("PANIC: Wrong sigma0 image format at 0x%x!\n", (L4_Word_t) sigma0_entry);
	spin(1);
	 	
    } 

    kip->sigma0.ip = sigma0_entry;
    elf64_install_image(module_ehdr, &sigma0_start, &sigma0_end );						

    printf("sigma0_start = 0x%x, sigma0_end = 0x%x, sigma0_entry = %x\n",
	   (L4_Word_t) sigma0_start, 
	   (L4_Word_t) sigma0_end, 
	   (L4_Word_t) sigma0_entry);



    /* root_server */
    L4_Word64_t root_entry;
    L4_Word64_t root_start;
    L4_Word64_t root_end;
      
    if ((module_ehdr = valid_elf64(modules[1].mod_start, &root_entry)) == NULL){
	printf("PANIC: Wrong root image format at 0x%x!\n", (L4_Word_t) root_entry);
	spin(1);
	 	
    } 

    elf64_install_image(module_ehdr, &root_start, &root_end );	 
      
    printf("root_start = 0x%x, root_end = 0x%x, root_entry = %x\n",
	   (L4_Word_t) root_start, 
	   (L4_Word_t) root_end, 
	   (L4_Word_t) root_entry);


      
    /* 
     * Register sigma0, sigma1, root_server
     */
    kip->sigma0.ip = sigma0_entry;
    kip->sigma0.sp = 0;
    kip->sigma0.low = sigma0_start;
    kip->sigma0.high = sigma0_end;
      
    kip->sigma1.ip = 0;
    kip->sigma1.sp = 0;
    kip->sigma1.low = 0;
    kip->sigma1.high = 0;

    kip->root_server.ip = root_entry;
    kip->root_server.sp = 0;
    kip->root_server.low = root_start;
    kip->root_server.high = root_end;
      

    /* 
     * Register memory regions
     */

    kip_memreg_t *memdesc = (kip_memreg_t *) ((L4_Word_t) kip + (L4_Word_t) kip->memory_info.memdesc_ptr) ;

    printf("Room for maximum %d memory descriptors.\n",  kip->memory_info.n);


    int md_count = 0;

    /* 
     * conventional / reserved mem 
     */
      
    if (mb_info->flags & MULTIBOOT_FLAGS_MMAP){
	  
	memory_map_t *m = (memory_map_t*) mb_info->mmap_addr;

	while ((L4_Word_t) m < (mb_info->mmap_addr + mb_info->mmap_length))
	{	       
	       
	    memdesc[md_count].low  = m->base_addr_low >> 10;
	    memdesc[md_count].high = ((m->base_addr_low+m->length_low) + (1 << 10) - 1) >> 10;
	    memdesc[md_count].type = (m->type == 1)
		? MEM_REGION_CONVENTIONAL
		: MEM_REGION_ARCH_SPECIFIC,
		memdesc[md_count].t    = m->type;
	    md_count++;

	    m = (memory_map_t*) ((L4_Word_t) m + 4 + m->size);
	       
	}
    }
	       

    
    /* The standard PC's VGA memory hasn't been seen in any BIOS
     * memory map so far. So we fake an entry for it. */
    memdesc[md_count].low  = (0xA0000) >> 10;
    memdesc[md_count].high = ((0xC0000) + (1 << 10) - 1) >> 10;
    memdesc[md_count].type = MEM_REGION_SHARED;
    md_count++;
 
    /* Standard PC's may have VGA and Extension ROMs -- fake
     * another entry */
    memdesc[md_count].low  = (0xC0000) >> 10;
    memdesc[md_count].high = ((0xF0000) + (1 << 10) - 1) >> 10;
    memdesc[md_count].type = MEM_REGION_SHARED;
    md_count++;

      
    /* sigma0, root_server
     * 
     * at the moment, sigma0 reserves these regions
     * so we don't do it here
     */
    //memdesc[md_count].low  = (sigma0_start) >> 10;
    //memdesc[md_count].high = ((sigma0_end) + (1 << 10) - 1) >> 10;
    //memdesc[md_count].type = MEM_REGION_RESERVED;
    //md_count++;
      
    //memdesc[md_count].low  = (root_start) >> 10;
    //memdesc[md_count].high = ((root_end) + (1 << 10) - 1) >> 10;
    //memdesc[md_count].type = MEM_REGION_RESERVED;
    //md_count++;


    //for (int i=0; i<md_count; i++)
    //  printf("md[%d] = %x-%x %d\n", i, 
    //	 (L4_Word_t) (memdesc[i].low << 10), 
    //	 (L4_Word_t) (memdesc[i].high << 10), 
    //	 memdesc[i].type);

    kip->memory_info.n = md_count;

      
    /*
     * Multiboot Info 
     */ 
    kip->boot_info = (L4_Word_t) mb_info;


    /*
     * Done, now load kernel 
     */ 


    kernel_entry_t kernel_entry =  (kernel_entry_t) (L4_Word_t) kernel_entry_addr;

    printf("Loading Kernel at 0x%x...\n", (L4_Word_t ) kernel_entry);

    kernel_entry();
      
    printf("PANIC: Return from kernel to elf-loader!\n");

    spin(0);    
      
      

}
