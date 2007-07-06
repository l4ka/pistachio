/*********************************************************************
 *
 * Copyright (C) 2003-2004, University of New South Wales
 *
 * File path:    elf-loader/openfirmware/console.cc
 * Description:  Open Firmware (IEEE std 1275) memory management.
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
 * $Id: memory.cc,v 1.4 2004/01/21 23:49:02 philipd Exp $
 *
 ********************************************************************/

#include <kip.h>
#include <openfirmware/memory.h>
#include <openfirmware/console.h>
#include <openfirmware/openfirmware.h>

ofw_ihandle_t ofw_mmu;    /* Handle for virtual memory  */
ofw_ihandle_t ofw_memory; /* Handle for physical memory */

/**
 *  All this code assumes the Open Firmware console is initialised.
 */

void
ofw_setup_physmem(void)
{
  ofw_phandle_t chosen;
  unsigned int buffer;

  /* Find 'chosen' package to find out what device is memory. */

  if((chosen = ofw_finddevice(ADDR2OFW_STRING("/chosen"))) ==
     INT2OFW_CELL(-1)) {

    ofw_error("ofw_setup_physmem:\tCan't find \"/chosen\"!");
  }

  /* Get memory device instance */
  if(ofw_getprop(chosen,
		 ADDR2OFW_STRING("memory"),
		 ADDR2OFW_BUFFER(&buffer),
		 sizeof(int)) != sizeof(int)) {

    ofw_error("ofw_setup_physmem:\nCan't open memory device!");
  } else {
    ofw_memory = buffer;
  }

  ofw_size_physmem();

} // ofw_setup_physmem()

void
ofw_size_physmem(void)
{
  ofw_phandle_t phys_mem; /* Main memory package.                       */
  ofw_phandle_t phys_bus; /* bus package main memory hangs off.         */
  int addr_cells;         /* Number of cells per physical address.      */
  int size_cells;         /* Number of cells per physical address size. */
  L4_Word_t mem_size;     /* Total physical memory.                     */

  /* Get memory package */
  if((phys_mem = ofw_instance2package(ofw_memory)) == INT2OFW_CELL(-1)) {
    ofw_error("ofw_size_physmem:\nCan't get memory package!");
  }

  /* Get memory packages parent bus */
  if((phys_bus = ofw_parent(phys_mem)) == INT2OFW_CELL(0)) {
    ofw_error("ofw_size_physmem:\nMemory package seems to be root device?");
  }

  /* Parent bus cell sizes */
  if((ofw_getprop(phys_bus, // number of ints per address
		  ADDR2OFW_STRING("#address-cells"),
		  ADDR2OFW_BUFFER(&addr_cells),
		  sizeof(int)) != sizeof(int)) ||
     (ofw_getprop(phys_bus, // number of ints per size
		  ADDR2OFW_STRING("#size-cells"),
		  ADDR2OFW_BUFFER(&size_cells),
		  sizeof(int)) != sizeof(int))) {

    ofw_error("ofw_size_physmem:\tCan't address/size cell numbers!");
  }

  /* Get the "reg" descriptors */
  ofw_cell_t number = BUFFER_SIZE;
  if((number = ofw_getprop(phys_mem,
			   ADDR2OFW_STRING("reg"),
			   ADDR2OFW_BUFFER(buffer),
			   number)) == INT2OFW_CELL(-1)) {
    ofw_error("ofw_setup_physmem:\tCan't access memory property \"reg\"\n");
  }

  number /= ((addr_cells + size_cells) * sizeof(int));

  printf("elf-loader:\t%d physical memory region(s) detected @\n", (int)number);

#warning awiggins (14-08-03): Need to replace this mess with proper property decoding...

  mem_size = 0;

  for(int i = 0; i < (int)number; i++) {
    
    L4_Word_t mem_hi = 0;
    L4_Word_t mem_lo = 0;

    /* Get the i'th entries base address */
    for(int j = i * (addr_cells + size_cells); j < addr_cells; j++) {
      mem_lo +=	((int *)buffer)[j] << (32 * j); // addresses are encoded as int
    }

    /* Get the i'th entries size */
    for(int j = i * (addr_cells + size_cells) + addr_cells;
	j < (addr_cells + size_cells); j++) {
      mem_hi +=	((int *)buffer)[j] << (32 * j); // sizes are encoded as int
    }

    mem_hi += mem_lo;

    /* Add memory descriptor. */

    printf("\t\tRegion [%d] 0x%lx - 0x%lx\n", i, mem_lo, mem_hi);

    if(kip_manager.add_memdesc(0, mem_lo, mem_hi, 
			       L4_ConventionalMemoryType, 0) == -1) {

      ofw_error("kip manager failed to add memory descriptor!");
    }

    mem_size += mem_hi - mem_lo;
  }

  printf("elf-loader:\tTotal physical memory 0x%lx\n", mem_size);

  /* Find what physical memory firmware resersed */

  /* Get the "available" descriptors and allocate all memory */
  number = BUFFER_SIZE;
  if((number = ofw_getprop(phys_mem,
			   ADDR2OFW_STRING("available"),
			   ADDR2OFW_BUFFER(buffer),
			   number)) == INT2OFW_CELL(-1)) {
    ofw_error("ofw_setup_physmem:\tCan't access memory property \"available\"\n");
  }

  number /= ((addr_cells + size_cells) * sizeof(int));

  printf("elf-loader:\t%d physical memory region(s) reserved @\n", (int)number);

#warning awiggins (14-08-03): Need to replace this mess with proper property decoding...

  L4_Word_t resv_lo = 0;
  L4_Word_t resv_hi = 0;

  for(int i = 0; i < (int)number; i++) {

    L4_Word_t mem_lo = 0;
    L4_Word_t mem_hi = 0;

    /* Get the i'th entries base address */
    for(int j = i * (addr_cells + size_cells); 
	j < (i * (addr_cells + size_cells) + addr_cells); j++) {

      mem_lo +=	((int *)buffer)[j] << (32 * j); // addresses are encoded as int
    }

    /* Get the i'th entries size */
    for(int j = i * (addr_cells + size_cells) + addr_cells;
	j < (i * (addr_cells + size_cells) + addr_cells + size_cells); j++) {

      mem_hi +=	((int *)buffer)[j] << (32 * j); // sizes are encoded as int
    }

    /* Allocate physical region */

    mem_hi += mem_lo;

    if(resv_lo != resv_hi) {
      
      resv_lo = mem_hi;

      printf("\t\tRegion [%d] 0x%lx - 0x%lx\n", i, resv_lo, resv_hi);
      
      if(kip_manager.add_memdesc(0, resv_lo, resv_hi, 
				 L4_BootLoaderSpecificMemoryType,
				 OFWMemorySubType_Reserved) == -1) {
	
	ofw_error("kip manager failed to add memory descriptor!");
      }
    }

    resv_hi = mem_lo;
  }
  
} // ofw_size_physmem()

void
ofw_setup_virtmem(void)
{
  ofw_phandle_t chosen;
  unsigned int buffer;

  if((chosen = ofw_finddevice(ADDR2OFW_STRING("/chosen"))) ==
     INT2OFW_CELL(-1)) {

    ofw_error("ofw_setup_virtmem:\tCan't find \"/chosen\"!");
  }

  /* Get memory device instance */
  if(ofw_getprop(chosen,
		 ADDR2OFW_STRING("mmu"),
		 ADDR2OFW_BUFFER(&buffer),
		 sizeof(int)) != sizeof(int)) {

    ofw_error("ofw_setup_virtmem:\nCan't open mmu device!");
  } else {
    ofw_mmu = buffer;
  }

} // ofw_setup_virtmem()

void
ofw_map_virtmem(L4_Word_t vaddr, L4_Word_t size)
{
  L4_Word_t paddr = kip_manager.mem_base() + vaddr; /* one-one memory map */
#warning awiggins (06-09-03): Clean up to do address encoding property.
  ofw_addr_t paddr_lo = ADDR2OFW_ADDR(paddr);
  ofw_addr_t paddr_hi = ADDR2OFW_ADDR(paddr >> 32);

#warning awiggins (06-09-03): Clean up to use ofw_call_method().
  struct {
    ofw_string_t  service;
    ofw_cell_t    num_args;
    ofw_cell_t    num_rets;
    ofw_string_t  method;       // arg 1
    ofw_ihandle_t instance;     // arg 2
    ofw_cell_t    mode;         // arg 3
    ofw_cell_t    size;         // arg 4
    ofw_addr_t    vaddr;        // arg 5
    ofw_addr_t    paddr_hi;     // arg 6
    ofw_addr_t    paddr_lo;     // arg 7
    ofw_cell_t    catch_result; // ret 1

  } args = {ADDR2OFW_STRING("call-method"),
	    7,
	    1,
	    ADDR2OFW_STRING("map"), // arg 1
	    ofw_mmu,                // arg 2
	    INT2OFW_CELL(-1),       // arg 3
	    INT2OFW_CELL(size),     // arg 4
	    ADDR2OFW_ADDR(vaddr),   // arg 5
	    paddr_hi,               // arg 6
	    paddr_lo};              // arg 7

  ofw_entry(&args);

} // ofw_map_virtmem()
