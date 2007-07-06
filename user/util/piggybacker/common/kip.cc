/****************************************************************************
 *
 * Copyright (C) 2002-2003, Karlsruhe University
 *
 * File path:	piggybacker/common/kip.cc
 * Description:	
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
 * $Id: kip.cc,v 1.10 2004/01/16 11:29:06 joshua Exp $
 *
 ***************************************************************************/

#include <piggybacker/kip.h>
#include <piggybacker/elf.h>
#include <piggybacker/io.h>
#include <piggybacker/macros.h>
#include PIGGYBACK_ARCH(page.h)


typedef union {
    char string[4];
    L4_Word32_t raw;
} magic_t;

magic_t l4_magic = {string: {'L', '4', 230, 'K'}};
magic_t kip_magic = {string: {'.', 'k', 'i', 'p'}};

bool kip_manager_t::virt_to_phys( L4_Word_t virt, L4_Word_t elf_start, L4_Word_t *phys )
{
    elf_ehdr_t *ehdr = (elf_ehdr_t *)elf_start;
    elf_phdr_t *phdr_list, *phdr;
    int p_cnt;

    phdr_list = (elf_phdr_t *)(elf_start + ehdr->e_phoff);
    for( p_cnt = 0; p_cnt < ehdr->e_phnum; p_cnt++ )
    {
	phdr = &phdr_list[ p_cnt ];

	if ((virt >= phdr->p_vaddr) && (virt < (phdr->p_vaddr + phdr->p_memsz)))
	{
	    *phys = virt - phdr->p_vaddr + phdr->p_paddr;
	    return true;
	}
    }

    return false;
}

bool kip_manager_t::find_kip( L4_Word_t kernel_start )
{
    elf_ehdr_t *ehdr = (elf_ehdr_t *)kernel_start;
    elf_shdr_t *shdr_list, *shdr, *str_hdr;
    int s_cnt;
    char *sec_name;
    L4_Word_t kip_virt;
    L4_Word32_t *kip_start, *kip_end;

    /* Walk the section headers.
     */
    shdr_list = (elf_shdr_t *)(kernel_start + ehdr->e_shoff);
    str_hdr = &shdr_list[ ehdr->e_shstrndx ];
    for( s_cnt = 0; s_cnt < ehdr->e_shnum; s_cnt++ )
    {
	/* Look for a section named .kip
	 */
	shdr = &shdr_list[ s_cnt ];
	sec_name = (char *)(shdr->sh_name + kernel_start + str_hdr->sh_offset);
	if( *(L4_Word32_t *)sec_name != kip_magic.raw )
	    continue;

	/* Now look for the L4uK magic number, which identifies the start
	 * of the kip page.
	 */
	kip_start = (L4_Word32_t *)( kernel_start + shdr->sh_offset );
	kip_end = (L4_Word32_t *)( (L4_Word_t)kip_start + shdr->sh_size );
	while( (*kip_start != l4_magic.raw) && (kip_start < kip_end) )
	    kip_start++;
	if( *kip_start != l4_magic.raw )
	    continue;

	// The physical location of the kip in the kernel's elf image.
	this->kip_src = (struct L4_KernelConfigurationPage *)kip_start;

	// The virtual address of the kip, as visible to the kernel.
	kip_virt = (L4_Word_t)kip_start - shdr->sh_offset - kernel_start 
	    + shdr->sh_addr;

	// Convert the kernel's virtual address to a physical address.  This
	// is where we must install the kip.
	if( !this->virt_to_phys(kip_virt, kernel_start, 
		    (L4_Word_t *)&this->kip_dst) )
	    return false;

	puts( "[==== Found the kernel interface page ====]" );
	print_hex( "- kip virt", kip_virt );
	print_hex( ", kip phys", (L4_Word_t)this->kip_dst );
	print_hex( ", kip size", shdr->sh_size );
	puts( "" );

	return true;
    }

    return false;
}

void kip_manager_t::install_module( L4_Word_t mod_start, L4_Word_t mod_end, 
	kip_server_t *server )
{
    elf_ehdr_t *ehdr = (elf_ehdr_t *)mod_start;
    elf_phdr_t *phdr_list, *phdr;
    int p_cnt;
    L4_Word_t addr;

    if( mod_start == mod_end )
	return;	/* Empty module. */

    // Verify alignment, so that we can dereference Elf structures.
    if( mod_start % 4 ) {
	puts( "Fatal error: the module is not 4-byte aligned!" );
	print_hex( "Module start", mod_start );
	while( 1 ) ;
    }

    server->ip = ehdr->e_entry;
    server->start = 0xffffffff;
    server->end = 0;

    // Walk the elf file's program headers.
    phdr_list = (elf_phdr_t *)(mod_start + ehdr->e_phoff);
    for( p_cnt = 0; p_cnt < ehdr->e_phnum; p_cnt++ )
    {
	phdr = &phdr_list[ p_cnt ];
	if( phdr->p_type != PT_LOAD ) {
	    print_hex( "- skipping an unloadable elf section: virt=", phdr->p_vaddr);
	    puts( "" );
	    continue;
	}

	print_hex( "- virt addr", phdr->p_vaddr );
	print( ", " );
	print_hex( "phys addr", phdr->p_paddr );
	print( ", " );
	print_hex( "mem size", phdr->p_memsz );
	puts( "" );
	print_hex( "  file size", phdr->p_filesz );
	print( ", " );
	print_hex( "file offset", phdr->p_offset );

	puts( "" );

	/* Copy the data associated with the program header to its 
	 * target physical address.  I wonder what happens if this
	 * overlaps with the boot loader, or the exception vectors,
	 * or the platform's memory, or some other module?
	 */
	memcpy_cache_flush( (L4_Word_t *)phdr->p_paddr, 
		(L4_Word_t *)(mod_start + phdr->p_offset), phdr->p_filesz );

	/* Calculate the start page of the region's physical range. */
    	addr = phdr->p_paddr & PAGE_MASK;
	if( addr < server->start )
	    server->start = addr;

	/* Calculate the end page of the region's physical range. */
	addr = wrap_up( phdr->p_paddr + phdr->p_memsz, PAGE_SIZE );
	if( addr > server->end )
	    server->end = addr;
    }

    print_hex( "- ip", server->ip );
    print( ", " );
    print_hex( " start", server->start );
    print( ", " );
    print_hex( " end", server->end );
    puts( "" );
}


void kip_manager_t::install_sigma0( L4_Word_t mod_start, L4_Word_t mod_end )
{
    puts( "[==== Installing sigma0 ====]" );
    this->install_module( mod_start, mod_end, &this->servers[sigma0] );
}

void kip_manager_t::install_root_task( L4_Word_t mod_start, L4_Word_t mod_end )
{
    puts( "[==== Installing the root task ====]" );
    this->install_module( mod_start, mod_end, &this->servers[root_task] );
}

void kip_manager_t::install_kernel( L4_Word_t mod_start, L4_Word_t mod_end )
{
    puts( "[==== Installing the kernel ====]" );
    this->install_module( mod_start, mod_end, &this->servers[kernel] );
}

void kip_manager_t::update_kip()
{
    this->kip_dst->sigma0.ip = this->servers[sigma0].ip;
    this->kip_dst->sigma0.low = this->servers[sigma0].start;
    this->kip_dst->sigma0.high = this->servers[sigma0].end;

    this->kip_dst->root_server.ip = this->servers[root_task].ip;
    this->kip_dst->root_server.low = this->servers[root_task].start;
    this->kip_dst->root_server.high = this->servers[root_task].end;

    this->kip_dst->BootInfo = this->boot_info;

    this->kip_dst->MemoryInfo.n = this->mem_desc_cnt;
}

inline L4_Word_t addr_align_up (L4_Word_t addr, L4_Word_t align)
{
    return (addr + (align-1)) & (~(align-1));
}

void kip_manager_t::setup_main_memory( L4_Word_t start, L4_Word_t end )
{
    this->kip_dst->MainMem.low = start;
    this->kip_dst->MainMem.high = end;
}

void kip_manager_t::dedicate_memory( L4_Word_t start, L4_Word_t end, L4_Word_t type, L4_Word_t sub_type )
{
    L4_MemoryDesc_t *desc = 
	L4_MemoryDesc( (void *)this->kip_dst, this->mem_desc_cnt );
    if( desc == (L4_MemoryDesc_t *)0 )
    {
	puts( "Error: insufficient memory descriptors in the kernel interface "
		"page!" );
	puts( "Aborting ..." );
	while( 1 ) ;
    }

    desc->x.type = type;
    desc->x.t = sub_type;
    desc->x.v = 0;
    desc->x.low = start >> 10;
    desc->x.high = (addr_align_up(end, 4096) - 1) >> 10;

    this->mem_desc_cnt++;
}

L4_Word_t kip_manager_t::first_avail_page()
{
    L4_Word_t end = this->servers[0].end;
    for( unsigned i = 1; i < kip_manager_t::tot; i++ )
	if( this->servers[i].end > end )
	    end = this->servers[i].end;
    return wrap_up( end, PAGE_SIZE );
}

void kip_manager_t::init()
{
    this->boot_info = 0;
    this->mem_desc_cnt = 0;

    for( unsigned i = 0; i < kip_manager_t::tot; i++ )
	this->servers[i].clear();
}

