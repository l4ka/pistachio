/*********************************************************************
 *
 * Copyright (C) 2003, University of New South Wales
 *
 * File path:    openfirmware/openfirmware.h
 * Description:  Open Firmware (IEEE std 1275) device tree macros and
 *               data types. Builds a position independent copy of the
 *               device tree.
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
 * $Id: device_tree.h,v 1.2 2003/09/24 19:06:14 skoglund Exp $
 *
 ********************************************************************/

#ifndef __OPENFIRMWARE__DEVICE_TREE_H__
#define __OPENFIRMWARE__DEVICE_TREE_H__

#include <l4/types.h>

class ofw_devtree_item_t {
public:
  L4_Word_t length;
  char data[];

public:
  ofw_devtree_item_t * next(void);

}; // ofw_devtree_item_t

class ofw_devtree_device_t {
public:
  L4_Word_t phandle;           /* Package handle, used for searches. */
  L4_Word_t properties_number; /* Number of properties node has.     */
  L4_Word_t properties_length; /* Length of properties buffer.       */
  L4_Word_t name_length;       /* Length of name.                    */
  char      name[];            /* Name of node.                      */

public:
  void null(void);
  ofw_devtree_device_t * next(void);
  ofw_devtree_item_t * first(void);

}; // ofw_devtree_device_t

class ofw_devtree_t {

private:
  char * devtree; /* Device tree. */

public:
  L4_Word_t build(char * devtree_start);
  void add_device(ofw_devtree_device_t * device);
  ofw_devtree_device_t * first(void);

}; // ofw_devtree_t


#endif /* !__OPENFIRMWARE__DEVICE_TREE_H__ */
