/*********************************************************************
 *                
 * Copyright (C) 2002-2003,  Karlsruhe University
 *                
 * File path:     pistachio.cvs/kernel/kdb/api/v4/kernelinterface.cc
 * Description:   Kernel interface page dump command
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
 * $Id: kernelinterface.cc,v 1.13 2003/09/24 19:05:03 skoglund Exp $
 *                
 ********************************************************************/
#include <debug.h>
#include <kdb/kdb.h>
#include <kdb/cmd.h>
#include INC_API(kernelinterface.h)


/**
 * cmd_dump_kip: Dump kernel interface page.
 */
DECLARE_CMD (cmd_dump_kip, root, 'K', "kip", "dump kernel interface page");

CMD(cmd_dump_kip, cg)
{
    kernel_interface_page_t * kip = get_kip ();
    kernel_descriptor_t * kdesc = (kernel_descriptor_t *)
	((word_t) kip + kip->kernel_desc_ptr);
    word_t i, n;

    static const char * sizenames[] = {
	"1K", "2K", "4K", "8K", "16K", "32K", "64K", "128K", "256K", "512K",
	"1M", "2M", "4M", "8M", "16M", "32M", "64M", "128M", "256M", "512M",
	"1G", "2G", "4G", "8G", "16G", "32G", "64G", "128G", "256G", "512G",
	"1T", "2T", "4T", "8T", "16T", "32T", "64T", "128T", "256T", "512T"
    };

    static const char * monthnames[] = {
	"Jan", "Feb", "Mar", "Apr", "May", "Jun",
	"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
    };

    // Configuration entries
    printf ("Kernel Interface Page:\n");

    printf ("  %16s %c%c%c%c\n",
	    "Magic",
	    kip->magic.string[0], kip->magic.string[1],
	    kip->magic.string[2], kip->magic.string[3]);

    printf ("  %16s 0x%2x.0x%2x\n",
	    "API version",
	    kip->api_version.get_version (),
	    kip->api_version.get_subversion ());

    printf ("  %16s %s-endian, %d-bit\n",
	    "API flags",
	    kip->api_flags.get_endian () == 0 ? "litte" : "big",
	    kip->api_flags.get_word_size () == 0 ? 32 : 64);

    printf ("  %16s min size: %dKB, alignment: %d, UTCB size: %d\n",
	    "UTCB area info",
	    kip->utcb_info.get_minimal_size () / 1024,
	    kip->utcb_info.get_utcb_alignment (),
	    kip->utcb_info.get_utcb_size ());

    printf ("  %16s min size: %dKB\n",
	    "KIP area info",
	    kip->kip_area_info.get_size () / 1024);

    printf ("  %16s 0x%p\n",
	    "Boot info",
	    kip->boot_info);

    printf ("  %16s read prec: 0x%4x, schedule prec: 0x%4x\n",
	    "Clock info",
	    kip->clock_info.get_read_precision (),
	    kip->clock_info.get_schedule_precision ());

    printf ("  %16s user base: 0x%3x, system base: 0x%3x, thread bits: %d\n",
	    "Thread info",
	    kip->thread_info.get_user_base (),
	    kip->thread_info.get_system_base (),
	    kip->thread_info.get_significant_threadbits ());

    printf ("  %16s sizes:", "Page info");
    for (word_t mask = kip->page_info.get_page_size_mask () >> 10, n = 0;
	 mask != 0;
	 mask >>= 1, n++)
	if (mask & 0x01)
	    printf(" %s", sizenames[n]);
    printf (", rights: %s%s%s\n",
	    kip->page_info.get_access_rights () & 4 ? "r" : "",
	    kip->page_info.get_access_rights () & 2 ? "w" : "",
	    kip->page_info.get_access_rights () & 1 ? "x" : "");

    // Servers
    printf( "\nRoot servers:\n" );
    printf( "  %16s ip: 0x%08x, ", "sigma0", kip->sigma0.ip );
    printf( "sp: 0x%08x, ", kip->sigma0.sp );
    printf( "0x%08x:0x%08x\n", kip->sigma0.mem_region.low,
	    kip->sigma0.mem_region.high );

    printf( "  %16s ip: 0x%08x, ", "sigma1", kip->sigma1.ip );
    printf( "sp: 0x%08x, ", kip->sigma1.sp );
    printf( "0x%08x:0x%08x\n", kip->sigma1.mem_region.low,
	    kip->sigma1.mem_region.high );

    printf( "  %16s ip: 0x%08x, ", "root server", kip->root_server.ip );
    printf( "sp: 0x%08x, ", kip->root_server.sp );
    printf( "0x%08x:0x%08x\n", kip->root_server.mem_region.low,
	    kip->root_server.mem_region.high );

    // Kernel descriptor
    printf ("\nKernel descriptor:\n");

    printf ("  %16s %d.%d\n",
	    "Kernel ID",
	    kdesc->kernel_id.get_id (),
	    kdesc->kernel_id.get_subid ());

    printf ("  %16s %s %d, %d\n",
	    "Kernel gen date",	
	    monthnames[kdesc->kernel_gen_date.get_month () - 1],
	    kdesc->kernel_gen_date.get_day (),
	    kdesc->kernel_gen_date.get_year ());

    printf ("  %16s %d.%d.%d\n",
	    "Kernel version",
	    kdesc->kernel_version.get_ver (),
	    kdesc->kernel_version.get_subver (),
	    kdesc->kernel_version.get_subsubver ());

    printf ("  %16s %c%c%c%c\n",
	    "Kernel supplier",
	    kdesc->kernel_supplier.string[0],
	    kdesc->kernel_supplier.string[1],
	    kdesc->kernel_supplier.string[2],
	    kdesc->kernel_supplier.string[3]);

    printf ("  %16s %s\n",
	    "Version string",
	    kdesc->get_version_string ());

    printf ("  %16s ", "Features");
    char * f = kdesc->get_version_string ();
    bool first_p = 1;
    while (*f++ != 0) {} // Skip kernel version string
    while (*f != 0)
    {
	printf (first_p ? "%s%s" : "\n  %16s %s", "", f);
	while (*f++ != 0) {}
	first_p = 0;
    }
    printf("\n");

    // System calls
    printf ("\nSystem call offsets:\n");
    printf ("  %16s 0x%08x    %17s 0x%08x\n",
	    "SpaceControl", kip->space_control_syscall,
	    "ThreadControl", kip->thread_control_syscall);
    printf ("  %16s 0x%08x    %17s 0x%08x\n",
	    "ProcessorControl", kip->processor_control_syscall,
	    "MemoryControl", kip->memory_control_syscall);
    printf ("  %16s 0x%08x    %17s 0x%08x\n",
	    "Ipc", kip->ipc_syscall,
	    "Lipc", kip->lipc_syscall);
    printf ("  %16s 0x%08x    %17s 0x%08x\n",
	    "Unmap", kip->unmap_syscall,
	    "ExchangeRegisters", kip->exchange_registers_syscall);
    printf ("  %16s 0x%08x    %17s 0x%08x\n",
	    "SystemClock", kip->system_clock_syscall,
	    "ThreadSwitch", kip->thread_switch_syscall);
    printf ("  %16s 0x%08x\n",
	    "Schedule", kip->schedule_syscall);

    // Processor descriptors
    word_t nproc = kip->processor_info.get_num_processors ();
    procdesc_t * pdesc = kip->processor_info.get_procdesc(0);
    printf ("\nProcessors %d:\n", nproc);
    for (word_t i = 0; i < nproc; i++)
    {
	printf ("  Proc%3d:       "
		"external freq = %dMHz, internal freq = %dMHz\n",
		i, pdesc->external_freq / 1000, pdesc->internal_freq / 1000);
	pdesc++;
    }

    // Memory descriptors
    word_t num_mdesc = kip->memory_info.get_num_descriptors ();
    memdesc_t * mdesc = kip->memory_info.get_memdesc (0);
    static const char * memtypes[] = {
	"undefined", "conventional", "reserved", "dedicated", "shared"
    };

    printf ("\nMemory regions (%d):\n", num_mdesc);
    printf ("  %16s ", "Physical:");
    for (i = n = 0; i < num_mdesc; i++)
	if (! (mdesc[i].low () == 0 && mdesc[i].high () == 0) &&
	    ! mdesc[i].is_virtual ())
	{
	    word_t t = mdesc[i].type ();
	    if (n++ != 0) printf ("  %16s ", "");
	    printf (t == 0xe || t == 0xf ?
		    "0x%p - 0x%p   %s (%d)\n" : "0x%p - 0x%p   %s\n",
		    mdesc[i].low (), mdesc[i].high (),
		    t == 0xe ? "bootloader specific" :
		    t == 0xf ? "architecture specific" :
		    t >= memdesc_t::max_type ? "<unknown>" :
		    memtypes[t], mdesc[i].subtype ());
	}
    if (n == 0) printf ("\n");

    printf ("  %16s ", "Virtual:");
    for (i = n = 0; i < num_mdesc; i++)
	if (! (mdesc[i].low () == 0 && mdesc[i].high () == 0) &&
	    mdesc[i].is_virtual ())
	{
	    if (n++ != 0) printf ("  %16s ", "");
	    printf ("0x%p - 0x%p   %s\n", mdesc[i].low (), mdesc[i].high (),
		    mdesc[i].type () >= memdesc_t::max_type ?
		    "<unknown>" : memtypes[mdesc[i].type ()]);
	}
    if (n == 0) printf ("\n");

    // Memory regions
    printf ("\nMemory regions:\n");

    printf ("  %16s 0x%p - 0x%p\n", "Main mem",
	    kip->main_mem.low, kip->main_mem.high);

    printf ("  %16s 0x%p - 0x%p\n", "Reserved mem0",
	    kip->reserved_mem0.low, kip->reserved_mem0.high);
    printf ("  %16s 0x%p - 0x%p\n", "Reserved mem1",
	    kip->reserved_mem1.low, kip->reserved_mem1.high);

    printf ("  %16s 0x%p - 0x%p\n", "Dedicated mem0",
	    kip->dedicated_mem0.low, kip->dedicated_mem0.high);
    printf ("  %16s 0x%p - 0x%p\n", "Dedicated mem1",
	    kip->dedicated_mem1.low, kip->dedicated_mem1.high);
    printf ("  %16s 0x%p - 0x%p\n", "Dedicated mem2",
	    kip->dedicated_mem2.low, kip->dedicated_mem2.high);
    printf ("  %16s 0x%p - 0x%p\n", "Dedicated mem3",
	    kip->dedicated_mem3.low, kip->dedicated_mem3.high);
    printf ("  %16s 0x%p - 0x%p\n", "Dedicated mem4",
	    kip->dedicated_mem4.low, kip->dedicated_mem4.high);
    
    return CMD_NOQUIT;
}
