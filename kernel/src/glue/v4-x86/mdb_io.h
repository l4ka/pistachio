/*********************************************************************
 *                
 * Copyright (C) 2005, 2007-2008,  Karlsruhe University
 *                
 * File path:     glue/v4-x86/mdb_io.h
 * Description:   MDB for IO ports specific declarations
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
 * $Id: mdb_io.h,v 1.3 2007/01/08 14:08:10 skoglund Exp $
 *                
 ********************************************************************/
#ifndef __GLUE__V4_X86__MDB_IO_H__
#define __GLUE__V4_X86__MDB_IO_H__

#include <mdb.h>
#include INC_GLUE(mdb.h)
#include INC_GLUE(vrt_io.h)

#define MDB_IO_SIZES		VRT_IO_SIZES
#define MDB_IO_NUMSIZES		VRT_IO_NUMSIZES

class mdb_io_t : public mdb_t
{
public:
    static word_t sizes[];
    static word_t num_sizes;

    word_t get_radix (word_t objsize);
    word_t get_next_objsize (word_t objsize);
    const char * get_name (void);

    // Node specific operations

    void clear (mdb_node_t * node);
    word_t get_rights (mdb_node_t * node);
    void set_rights (mdb_node_t * node, word_t r);
    void flush_cached_entry (mdb_node_t * node, range_t range);
    bool allow_attribute_update (mdb_node_t * node);
    void set_attribute (mdb_node_t * node, word_t attrib);
    word_t get_phys_address (mdb_node_t * node);
    word_t get_purged_status (mdb_node_t * node);
    void reset_purged_status (mdb_node_t * node);
    void update_purged_status (mdb_node_t * node, word_t status);
    word_t get_effective_status (mdb_node_t * node);
    word_t reset_effective_status (mdb_node_t * node);
    void update_effective_status (mdb_node_t * node, word_t status);
    void dump (mdb_node_t * node);
};

extern mdb_io_t mdb_io;
extern mdb_node_t * sigma0_ionode;



#endif /* !__GLUE__V4_X86__MDB_IO_H__ */
