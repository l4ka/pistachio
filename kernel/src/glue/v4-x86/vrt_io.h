/*********************************************************************
 *                
 * Copyright (C) 2005-2008,  Karlsruhe University
 *                
 * File path:     glue/v4-x86/vrt_io.h
 * Description:   VRT for IO ports specific declarations
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
 * $Id: vrt_io.h,v 1.4 2006/06/08 16:02:02 skoglund Exp $
 *                
 ********************************************************************/

#ifndef __GLUE__V4_X86__VRT_IO_H__
#define __GLUE__V4_X86__VRT_IO_H__

#include <vrt.h>
#include <debug.h>
#include INC_GLUE(mdb.h)


#define VRT_IO_SIZES		{ 0, 1, 3, 8, 16 }
#define VRT_IO_NUMSIZES		4

class vrt_io_t : public vrt_t
{
    char name[sizeof ("io<>  ") + sizeof (word_t) * 2];
    word_t count;
    space_t *space;
    
public:
    enum rights_e {
	rw	   =	6,
	fullrights =	6
    };

    static word_t sizes[];
    static word_t num_sizes;

    // Space management methods

    void * operator new (word_t size);
    void operator delete (void * v);
    void init (void);
    void populate_sigma0 (void);

    void add_tcb (tcb_t * tcb);
    bool remove_tcb (tcb_t * tcb);

    // Generic VRT methods

    word_t get_radix (word_t objsize);
    word_t get_next_objsize (word_t objsize);
    word_t get_vrt_size (void);
    mdb_t * get_mapdb (void);
    const char * get_name (void);

    // Node specific methods
    void set_object (vrt_node_t * n, word_t n_sz, word_t paddr,
			     vrt_node_t * o, word_t o_sz, word_t access);

    word_t get_address (vrt_node_t * n);
    void dump (vrt_node_t * n);
    word_t make_misc (vrt_node_t * obj, mdb_node_t * map);

    // Helper methods

    static rights_e get_rights (word_t object);
    static word_t get_port (word_t object);
    static void set_rights (vrt_node_t * n, rights_e rights);
    bool is_vrt_io_t (void);
    void set_space (space_t *s);
    space_t * get_space ();

};

/**
 * Get access rights for object.  Always return full rights.
 *
 * @param object	object value
 *
 * @return access rights for object
 */
INLINE vrt_io_t::rights_e vrt_io_t::get_rights (word_t object)
{
    return vrt_io_t::fullrights;
}

/**
 * Set access rights for object.  Void operation.
 *
 * @param n		IO object node
 * @param rights	new access rights
 */
INLINE void vrt_io_t::set_rights (vrt_node_t * n, rights_e rights)
{
}

/**
 * Lookup the port number.  The port number is stored in the least
 * signigficant bits of the word.
 *
 * @param object	object value
 *
 * @return global thread number
 */
INLINE word_t vrt_io_t::get_port (word_t object)
{
    return object & 0xffff;
}

/**
 * Check whether we are really dealing with an IO space.  Checks
 * the name of the iospace, so only works if kernel debugger is
 * enabled.
 *
 * @return true if this looks like a thread space, false otherwise
 */
INLINE bool vrt_io_t::is_vrt_io_t (void)
{
    return name[0] == 'i' && name[1] == 'o' && name[2] == '<';
}

/**
 * Set the embedded space_t object. Needed for I/O bitmap manipulation
 * 
 * @param s	space_t object reference
 */
INLINE void vrt_io_t::set_space (space_t * s)
{
    space = s;
}

/**
 * Get the embedded space_t object. Needed for I/O bitmap manipulation
 *
 * @return space_t object reference
 */
INLINE space_t * vrt_io_t::get_space (void)
{
    return space;
}


/*
 * We use the VRT for IO-ports as our IO space.
 */
typedef vrt_io_t	io_space_t;



#endif /* !__GLUE__V4_X86__VRT_IO_H__ */
