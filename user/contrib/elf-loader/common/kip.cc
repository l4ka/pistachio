/*********************************************************************
 *
 * Copyright (C) 2003, University of New South Wales
 *
 * File path:    contrib/elf-loader/kip.cc
 * Description:  KIP helper code.
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
 * $Id: kip.cc,v 1.3 2003/09/24 19:06:11 skoglund Exp $
 *
 ********************************************************************/

#include <kip.h>
#include <arch.h>
#include <elf-loader.h>
#include <l4/types.h>

kip_manager_t kip_manager;

bool
kip_manager_t::init(void * kip_laddr)
{
  kip = (L4_KernelConfigurationPage_t *) kip_laddr;

  boot_info     = 0;
  total_memdesc = 0;

  /* Init servers */
  for(unsigned i = 0; i < kip_server_t::server_total; i++) {
    servers[i].clear();
  }

  return true;

} // kip_manager_t::init()

bool
kip_manager_t::update(void)
{
  /* Copy memory descriptors. */

  kip->MemoryInfo.n = total_memdesc;

  L4_MemoryDesc_t * memdesc_dest =
    (L4_MemoryDesc_t *)((L4_Word_t)kip + kip->MemoryInfo.MemDescPtr);

  for(int i = 0; i < (int)total_memdesc; i++) {
    memdesc_dest[i].raw[0] = memdesc[i].raw[0];
    memdesc_dest[i].raw[1] = memdesc[i].raw[1];
  }

  /* Copy servers. */

  return true;

} // kip_manager_t::update()

bool
kip_manager_t::add_server(kip_server_t::kip_server_e server,
			  L4_Word_t sp, L4_Word_t ip,
			  L4_Word_t low, L4_Word_t high)
{
  if(server >= kip_server_t::server_total) {

    return false;
  }

  servers[server].sp = sp;
  servers[server].ip = ip;
  servers[server].low = low;
  servers[server].high = high;

  return true;

} // kip_manager_t::add_server()

int
kip_manager_t::add_memdesc(bool is_virt, L4_Word_t low, L4_Word_t high, 
			   L4_Word_t type, L4_Word_t sub_type)
{
  if(total_memdesc == MAX_MEMDESC) {

    return -1; // Indicate error

  } else {

    memdesc[total_memdesc].x.high = (high -1) >> 10;
    memdesc[total_memdesc].x.low =  low >> 10; 
    memdesc[total_memdesc].x.v =    is_virt;
    memdesc[total_memdesc].x.type = type;
    memdesc[total_memdesc].x.t =    sub_type;

    return total_memdesc++;
  }

} // kip_manager_t::add_memdesc()

/**
 *  kip_manager_t::mem_base() returns the base address of the first memory bank
 *  of physical memory.
 *
 *  precondition: ofw_size_physmem() has been called.
 */
L4_Word_t
kip_manager_t::mem_base(void)
{
  return memdesc[0].x.low << 10;

} // kip_manager_t::mem_base()

/**
 *  kip_manager_t::first_avail_page()
 *  Finds first page of free memory.
 */
L4_Word_t
kip_manager_t::first_avail_page(void)
{
  extern char mod_root_end;

  return wrap_up(mod_root_end, PAGE_SIZE);

} // kip_manager_t::first_avail_page()

