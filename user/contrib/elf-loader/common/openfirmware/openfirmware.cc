/*********************************************************************
 *                
 * Copyright (C) 2003, University of New South Wales
 *                
 * File path:    elf-loader/common/openfirmware/openfirmware.cc
 * Description:  Open firmware (IEEE std 1275) function stubs.
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
 * $Id: openfirmware.cc,v 1.4 2003/09/24 19:06:11 skoglund Exp $
 *                
 ********************************************************************/

#include <openfirmware/openfirmware.h>

char buffer[BUFFER_SIZE]; /* Buffer for loading properties into */

/* Client interface handle see IEEE std 1275 */
volatile void * ofw_cihandle;

/**************
* Device Tree *
**************/

ofw_phandle_t
ofw_peer(ofw_phandle_t peer)
{
  struct {
    ofw_string_t  service;
    ofw_cell_t    num_args;
    ofw_cell_t    num_rets;
    ofw_phandle_t peer_arg; // art 1
    ofw_phandle_t peer_ret; // ret 1

  } args = {ADDR2OFW_STRING("peer"),
	    1,
	    1,
	    peer}; // arg 1

  ofw_entry(&args);

  return args.peer_ret;

} // ofw_peer()

ofw_phandle_t
ofw_child(ofw_phandle_t parent)
{
  struct {
    ofw_string_t  service;
    ofw_cell_t    num_args;
    ofw_cell_t    num_rets;
    ofw_phandle_t parent;   // arg 1
    ofw_phandle_t child;    // ret 1

  } args = {ADDR2OFW_STRING("child"),
	    1,
	    1,
	    parent}; // arg 1

  ofw_entry(&args);

  return args.child; // ret 1

} // ofw_child()

ofw_phandle_t
ofw_parent(ofw_phandle_t child)
{
  struct {
    ofw_string_t  service;
    ofw_cell_t    num_args;
    ofw_cell_t    num_rets;
    ofw_phandle_t child;    // arg 1
    ofw_phandle_t parent;   // ret 1

  } args = {ADDR2OFW_STRING("parent"),
	    1,
	    1,
	    child}; // arg 1

  ofw_entry(&args);

  return args.parent; // ret 1

} // ofw_parent()

ofw_cell_t
ofw_nextprop(ofw_phandle_t package, ofw_string_t previous, ofw_buffer_t buffer)
{
  struct {
    ofw_string_t  service;
    ofw_cell_t    num_args;
    ofw_cell_t    num_rets;
    ofw_phandle_t package;  // arg 1
    ofw_string_t  previous; // arg 2
    ofw_buffer_t  buffer;   // arg 3
    ofw_cell_t    flag;     // ret 1

  } args = {ADDR2OFW_STRING("getproplen"),
	    3,
	    1,
	    package,  // arg 1
	    previous, // arg 2
	    buffer};  // arg 3

  ofw_entry(&args);

  return args.flag; // ret 1

} // ofw_nextprop()

ofw_cell_t
ofw_getproplen(ofw_phandle_t package, ofw_string_t property)
{
  struct {
    ofw_string_t  service;
    ofw_cell_t    num_args;
    ofw_cell_t    num_rets;
    ofw_phandle_t package;  // arg 1
    ofw_string_t  property; // arg 2
    ofw_cell_t    size;     // ret 1

  } args = {ADDR2OFW_STRING("getproplen"),
	    2,
	    1,
	    package,   // arg 1
	    property}; // arg 2

  ofw_entry(&args);

  return args.size; // ret 1

} // ofw_getproplen()

ofw_cell_t
ofw_getprop(ofw_phandle_t package, ofw_string_t property, 
	    ofw_buffer_t buffer, ofw_cell_t buffer_size)
{
  struct {
    ofw_string_t  service;
    ofw_cell_t    num_args;
    ofw_cell_t    num_rets;
    ofw_phandle_t package;     // arg 1
    ofw_string_t  property;    // arg 2
    ofw_buffer_t  buffer;      // arg 3 
    ofw_cell_t    buffer_size; // arg 4
    ofw_cell_t    size;        // ret 1

  } args = {ADDR2OFW_STRING("getprop"), 
	    4,
	    1,
	    package,      // arg 1
	    property,     // arg 2
	    buffer,       // arg 3
	    buffer_size}; // arg 4

  ofw_entry(&args);

  return args.size; // ret 1

} // ofw_getprop()

ofw_phandle_t
ofw_finddevice(ofw_string_t device)
{
  struct {
    ofw_string_t  service;
    ofw_cell_t    num_args;
    ofw_cell_t    num_rets;
    ofw_string_t  dev_name; // arg 1 
    ofw_phandle_t package;  // ret 1

  } args = {ADDR2OFW_STRING("finddevice"),
	    1,
	    1,
	    device}; // arg 1

  ofw_entry(&args);
 
  return args.package; // ret 1

} // ofw_finddevice()

ofw_phandle_t
ofw_instance2package(ofw_ihandle_t instance)
{
  struct {
    ofw_string_t  service;
    ofw_cell_t    num_args;
    ofw_cell_t    num_rets;
    ofw_ihandle_t instance; // arg 1
    ofw_phandle_t package;  // ret 1

  } args = {ADDR2OFW_STRING("instance-to-package"),
	    1,
	    1,
	    instance}; // arg 1

  ofw_entry(&args);

  return args.package; // ret 1

} // ofw_instance2package()

ofw_cell_t
ofw_package2path(ofw_phandle_t package, ofw_buffer_t buffer,
		 ofw_cell_t buffer_size)
{
  struct {
    ofw_string_t  service;
    ofw_cell_t    num_args;
    ofw_cell_t    num_rets;
    ofw_phandle_t package;     // arg 1
    ofw_buffer_t  buffer;      // arg 2
    ofw_cell_t    buffer_size; // arg 3
    ofw_cell_t    size;        // ret 1

  } args = {ADDR2OFW_STRING("package-to-path"),
	    3,
	    1,
	    package,      // arg 1
	    buffer,       // arg 2
	    buffer_size}; // arg 3

  ofw_entry(&args);

  return args.size; // ret 1

} // ofw_package2path()

/*************
* Device I/O *
*************/

ofw_ihandle_t
ofw_open(ofw_string_t device)
{
  struct {
    ofw_string_t  service;
    ofw_cell_t    num_args;
    ofw_cell_t    num_rets;
    ofw_string_t  device;   // arg 1
    ofw_ihandle_t instance; // ret 1

  } args = {ADDR2OFW_STRING("open"),
	    1,
	    1,
	    device}; // arg 1

  ofw_entry(&args);

  return args.instance; // ret 1

} // ofw_open()

void
ofw_close(ofw_ihandle_t device)
{
  struct {
    ofw_string_t  service;
    ofw_cell_t    num_args;
    ofw_cell_t    num_rets;
    ofw_ihandle_t device;   // arg 1

  } args = {ADDR2OFW_STRING("close"),
	    1,
	    0,
	    device}; // arg 1

  ofw_entry(&args);

  return;

} // ofw_close()

ofw_cell_t
ofw_read(ofw_ihandle_t device, ofw_buffer_t buffer, ofw_cell_t length)
{
  struct {
    ofw_string_t  service;
    ofw_cell_t    num_args;
    ofw_cell_t    num_rets;
    ofw_ihandle_t device;   // arg 1
    ofw_buffer_t  buffer;   // arg 2
    ofw_cell_t    length;   // arg 3
    ofw_cell_t    actual;   // ret 1

  } args = {ADDR2OFW_STRING("read"),
	    3,
	    1,
	    device,  // arg 1
	    buffer,  // arg 2
	    length}; // arg 3

  ofw_entry(&args);

  return args.actual;

}  // ofw_read()

ofw_cell_t
ofw_write(ofw_ihandle_t device, ofw_buffer_t buffer, ofw_cell_t length)
{
  struct {
    ofw_string_t  service;
    ofw_cell_t    num_args;
    ofw_cell_t    num_rets;
    ofw_ihandle_t device;   // arg 1
    ofw_buffer_t  buffer;   // arg 2
    ofw_cell_t    length;   // arg 3
    ofw_cell_t    actual;   // ret 1

  } args = {ADDR2OFW_STRING("write"),
	    3,
	    1,
	    device,  // arg 1
	    buffer,  // arg 2
	    length}; // arg 3

  ofw_entry(&args);

  return args.actual;

} // ofw_write()

ofw_cell_t
ofw_seek(ofw_ihandle_t device, ofw_cell_t pos_hi, ofw_cell_t pos_lo)
{
  struct {
    ofw_string_t  service;
    ofw_cell_t    num_args;
    ofw_cell_t    num_rets;
    ofw_ihandle_t device;   // arg 1
    ofw_cell_t    pos_hi;   // arg 2
    ofw_cell_t    pos_lo;   // arg 3
    ofw_cell_t    status;   // ret 1

  } args = {ADDR2OFW_STRING("seek"),
	    3,
	    1,
	    device,  // arg 1
	    pos_hi,  // arg 2
	    pos_lo}; // arg 3

  ofw_entry(&args);

  return args.status;

} // ofw_seek()

/*********
* Memory *
*********/

ofw_addr_t
ofw_claim(ofw_addr_t vaddr, ofw_cell_t size, ofw_cell_t align)
{
  struct {
    ofw_string_t service;
    ofw_cell_t   num_args;
    ofw_cell_t   num_rets;
    ofw_addr_t   vaddr;    // arg 1
    ofw_cell_t   size;     // arg 2
    ofw_cell_t   align;    // arg 3
    ofw_addr_t   vbase;    // ret 1 

  } args = {ADDR2OFW_STRING("claim"),
	    3,
	    1,
	    vaddr,  // arg 1
	    size,   // arg 2
	    align}; // arg 3

  ofw_entry(&args);

  return args.vbase;

} // ofw_claim()

void
ofw_release(ofw_addr_t vaddr, ofw_cell_t size)
{
  struct {
    ofw_string_t service;
    ofw_cell_t   num_args;
    ofw_cell_t   num_rets;
    ofw_addr_t   vaddr;    // arg 1
    ofw_cell_t   size;     // arg 2

  } args = {ADDR2OFW_STRING("release"),
	    2,
	    0,
	    vaddr, // arg 1
	    size}; // arg 2

  ofw_entry(&args);

  return;

} // ofw_release()

/*******************
* Control Transfer *
*******************/

void
ofw_enter(void)
{
  struct {
    ofw_string_t service;
    ofw_cell_t   num_args;
    ofw_cell_t   num_rets;

  } args = {ADDR2OFW_STRING("enter"),
	    0,
	    0};

  ofw_entry(&args);

  return;

} // ofw_enter()

#warning awiggins (03-09-03): ofw_chain does not seem to work...
void
ofw_chain(void * vaddr, ofw_cell_t size,
	  void * entry, void * args, ofw_cell_t len)
{
  struct {
    ofw_string_t service;
    ofw_cell_t   num_args;
    ofw_cell_t   num_rets;
    ofw_addr_t   vaddr;    // arg 1
    ofw_cell_t   size;     // arg 2
    ofw_addr_t   entry;    // arg 3
    ofw_addr_t   args;     // arg 4
    ofw_cell_t   len;      // arg 5

  } arg = {ADDR2OFW_STRING("chain"),
	    5,
	    0,
	    ADDR2OFW_ADDR(vaddr), // arg 1
	    size,                 // arg 2
	    ADDR2OFW_ADDR(entry), // arg 3
	    ADDR2OFW_ADDR(args),  // arg 4
	    len};                 // arg 5

  ofw_entry(&arg);

  return;
	    
} // ofw_chain()
