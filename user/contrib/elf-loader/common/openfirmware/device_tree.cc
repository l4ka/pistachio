/*********************************************************************
 *
 * Copyright (C) 2003, University of New South Wales
 *
 * File path:    elf-loader/openfirmware/device_tree.cc
 * Description:  Open Firmware (IEEE std 1275) device tree code.
 *               Builds a position independent copy of the device
 *               tree.
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
 * $Id: device_tree.cc,v 1.2 2003/09/24 19:06:11 skoglund Exp $
 *
 ********************************************************************/

#include <string.h>
#include <elf-loader.h>
#include <openfirmware/device_tree.h>
#include <openfirmware/openfirmware.h>

/**
 *  ofw_devtree_t::build()
 *  Builds a copy of the Open Firmware device tree.
 *  
 *  Return value: Size of the device tree structure.
 */
L4_Word_t
ofw_devtree_t::build(char * devtree_start)
{
  ofw_phandle_t next, node;
  ofw_devtree_device_t * device;
 
  devtree = devtree_start;

  node = ofw_peer(OFW_NULL_PHANDLE); /* Get root node. */

  device = this->first();

  while(node != OFW_NULL_PHANDLE) {
    /* Copy information of the current node. */
    int length =
      ofw_package2path(node, (ofw_buffer_t)device->name, BUFFER_SIZE);
    if(length > 0) {
      device->name_length  = length + 1; /* space for null character. */
      device->phandle = (L4_Word_t)node;
      add_device(device);
      device = device->next();
    }

    /* Copy children of the current node. */
    next = ofw_child(node);
    if(next != OFW_NULL_PHANDLE) {
      node = next;
      continue;
    }

    /* Copy siblings of the current node. */
    next = ofw_peer(node);
    if(next != OFW_NULL_PHANDLE) {
      node = next;
      continue;
    }

    /* Search for a parent with a sibling. */
    while(1) {
      node = ofw_parent(node);
      if(node == OFW_NULL_PHANDLE) {
	break;
      }
      next = ofw_peer(node);
      if(next != OFW_NULL_PHANDLE) {
	node = next;
	break;
      }
    }
  }

  device->null(); /* Terminate the data structure with a null record. */

  return (L4_Word_t)device - (L4_Word_t)devtree_start;

} // ofw_device_tree_t::build()

void
ofw_devtree_t::add_device(ofw_devtree_device_t * device)
{
  ofw_devtree_item_t * item, * prev_item, * property;
  ofw_cell_t finished; // Any more properties left.
  L4_Word_t start_offset;

  device->properties_number = 0;
  device->properties_length = 0;

  item = device->first();
  start_offset = (L4_Word_t)item;
  
  /* Initialise data pointer */
  prev_item = item;
  prev_item->data[0] = '\0'; /* Seed for the first lookup. */

  /* Get the first prop name. */
  finished = ofw_nextprop(device->phandle, ADDR2OFW_STRING(prev_item->data),
			  ADDR2OFW_BUFFER(item->data));
  
  /* Process the properties. */
  while(finished > 0) {
    device->properties_number++;
    prev_item = item;
	
    /* Set the length of the property name. */
    item->length = strnlen(item->data, 32) + 1; /* space for null character. */

    /* Grab the property and its length. */
    property = item->next();
    property->length = ofw_getproplen(device->phandle,
				      ADDR2OFW_STRING(item->data));
    if(property->length < 0) {
      property->length = 0;
    } else {
      ofw_getprop(device->phandle, ADDR2OFW_STRING(item->data),
		  ADDR2OFW_BUFFER(property->data), property->length);
	
      //if((property->len == sizeof(prom_handle_t)) && 
      //	 !strcmp(dev->name, "/chosen") ) {
      //	  // Convert instance handles into package handles.
      //	  prom_handle_t *handle = (prom_handle_t *)&prop->data;
      //	  *handle = prom_instance_to_package( *handle );
      //}
    }
    
    /* Move to the next item. */
    item = property->next();
    /* Grab the next property name. */
    finished = ofw_nextprop(device->phandle, ADDR2OFW_STRING(prev_item->data),
			    ADDR2OFW_BUFFER(item->data));
  }
  
  device->properties_length = (L4_Word_t)item - start_offset;

} // ofw_devtree_t::add_device()

ofw_devtree_device_t *
ofw_devtree_t::first(void)
{
  return (ofw_devtree_device_t *)wrap_up((L4_Word_t)devtree, sizeof(L4_Word_t));
}

void
ofw_devtree_device_t::null(void)
{
  this->phandle = this->name_length = 0;
  this->properties_number = this->properties_length = 0;
}

ofw_devtree_device_t *
ofw_devtree_device_t::next(void)
{
  return (ofw_devtree_device_t *)wrap_up((L4_Word_t)this->name +
					 this->name_length +
					 this->properties_length,
					 sizeof(L4_Word_t));
}

ofw_devtree_item_t *
ofw_devtree_device_t::first(void)
{
  return (ofw_devtree_item_t *)wrap_up((L4_Word_t)this->name +
				       this->name_length, sizeof(L4_Word_t));
}

ofw_devtree_item_t *
ofw_devtree_item_t::next(void)
{
  return (ofw_devtree_item_t *)wrap_up((L4_Word_t)this->data +
				       this->length, sizeof(L4_Word_t));
}
