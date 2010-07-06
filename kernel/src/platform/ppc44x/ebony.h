/*********************************************************************
 *                
 * Copyright (C) 2010,  Karlsruhe Institute of Technology
 *                
 * Filename:      ebony.h
 * Author:        Jan Stoess <stoess@kit.edu>
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
 ********************************************************************/
#ifndef __PLATFORM__PPC44X__EBONY_H__
#define __PLATFORM__PPC44X__EBONY_H__

#include INC_PLAT(fdt.h)

INLINE fdt_header_t *find_cpu( fdt_t *fdt, word_t cpu )
{
    fdt_header_t *fdtcpu = fdt->find_subtree("/cpus");
    if (!fdtcpu)
	return false;

    fdt_header_t *curr = fdt->find_first_subtree_node(fdtcpu);
    while(curr) 
    {
	fdt_property_t *prop = fdt->find_property_node(curr, "device_type");
	if (prop && strcmp(prop->get_string(), "cpu") == 0)
	{
	    prop = fdt->find_property_node(curr, "reg");
	    if (prop && prop->get_word(0) == cpu)
		return curr;
	}
	printf("FDT prop: %s\n", curr->name);
	curr = fdt->find_next_subtree_node(curr);
    }
    return NULL;
}

INLINE bool get_cpu_speed( word_t cpu, word_t *cpu_hz, word_t *bus_hz )
{
    fdt_t *fdt = get_dtree();
    fdt_property_t *prop;

    fdt_header_t *fdtcpu = find_cpu(fdt, cpu);
    if (fdtcpu)
    {
	prop = fdt->find_property_node(fdtcpu, "clock-frequency");
	if (!prop)
	    return false;
	*cpu_hz = prop->get_word(0);

	prop = fdt->find_property_node(fdtcpu, "timebase-frequency");
	if (!prop)
	    return false;
	*bus_hz = prop->get_word(0);
	return true;
    }
    return false;
}

INLINE int get_cpu_count()
{
    int count = 0;
    fdt_t *fdt = get_dtree();
    fdt_header_t *fdtcpu = fdt->find_subtree("/cpus");
    if (!fdtcpu)
	return 1;

    fdt_header_t *curr = fdt->find_first_subtree_node(fdtcpu);
    while(curr)
    {
	fdt_property_t *prop = fdt->find_property_node(curr, "device_type");
	if (prop && strcmp(prop->get_string(), "cpu") == 0)
	    count++;
	curr = fdt->find_next_subtree_node(curr);
    }
    return count > 0 ? count : 1;
}


#endif /* !__PLATFORM__PPC44X__EBONY_H__ */
