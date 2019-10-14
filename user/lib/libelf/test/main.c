#if 0
/*
 * Australian Public Licence B (OZPLB)
 * 
 * Version 1-0
 * 
 * Copyright (c) 2004 National ICT Australia
 * 
 * All rights reserved. 
 * 
 * Developed by: Embedded, Real-time and Operating Systems Program (ERTOS)
 *               National ICT Australia
 *               http://www.ertos.nicta.com.au
 * 
 * Permission is granted by National ICT Australia, free of charge, to
 * any person obtaining a copy of this software and any associated
 * documentation files (the "Software") to deal with the Software without
 * restriction, including (without limitation) the rights to use, copy,
 * modify, adapt, merge, publish, distribute, communicate to the public,
 * sublicense, and/or sell, lend or rent out copies of the Software, and
 * to permit persons to whom the Software is furnished to do so, subject
 * to the following conditions:
 * 
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimers.
 * 
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimers in the documentation and/or other materials provided
 *       with the distribution.
 * 
 *     * Neither the name of National ICT Australia, nor the names of its
 *       contributors, may be used to endorse or promote products derived
 *       from this Software without specific prior written permission.
 * 
 * EXCEPT AS EXPRESSLY STATED IN THIS LICENCE AND TO THE FULL EXTENT
 * PERMITTED BY APPLICABLE LAW, THE SOFTWARE IS PROVIDED "AS-IS", AND
 * NATIONAL ICT AUSTRALIA AND ITS CONTRIBUTORS MAKE NO REPRESENTATIONS,
 * WARRANTIES OR CONDITIONS OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 * BUT NOT LIMITED TO ANY REPRESENTATIONS, WARRANTIES OR CONDITIONS
 * REGARDING THE CONTENTS OR ACCURACY OF THE SOFTWARE, OR OF TITLE,
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, NONINFRINGEMENT,
 * THE ABSENCE OF LATENT OR OTHER DEFECTS, OR THE PRESENCE OR ABSENCE OF
 * ERRORS, WHETHER OR NOT DISCOVERABLE.
 * 
 * TO THE FULL EXTENT PERMITTED BY APPLICABLE LAW, IN NO EVENT SHALL
 * NATIONAL ICT AUSTRALIA OR ITS CONTRIBUTORS BE LIABLE ON ANY LEGAL
 * THEORY (INCLUDING, WITHOUT LIMITATION, IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHERWISE) FOR ANY CLAIM, LOSS, DAMAGES OR OTHER
 * LIABILITY, INCLUDING (WITHOUT LIMITATION) LOSS OF PRODUCTION OR
 * OPERATION TIME, LOSS, DAMAGE OR CORRUPTION OF DATA OR RECORDS; OR LOSS
 * OF ANTICIPATED SAVINGS, OPPORTUNITY, REVENUE, PROFIT OR GOODWILL, OR
 * OTHER ECONOMIC LOSS; OR ANY SPECIAL, INCIDENTAL, INDIRECT,
 * CONSEQUENTIAL, PUNITIVE OR EXEMPLARY DAMAGES, ARISING OUT OF OR IN
 * CONNECTION WITH THIS LICENCE, THE SOFTWARE OR THE USE OF OR OTHER
 * DEALINGS WITH THE SOFTWARE, EVEN IF NATIONAL ICT AUSTRALIA OR ITS
 * CONTRIBUTORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH CLAIM, LOSS,
 * DAMAGES OR OTHER LIABILITY.
 * 
 * If applicable legislation implies representations, warranties, or
 * conditions, or imposes obligations or liability on National ICT
 * Australia or one of its contributors in respect of the Software that
 * cannot be wholly or partly excluded, restricted or modified, the
 * liability of National ICT Australia or the contributor is limited, to
 * the full extent permitted by the applicable legislation, at its
 * option, to:
 * a.  in the case of goods, any one or more of the following:
 * i.  the replacement of the goods or the supply of equivalent goods;
 * ii.  the repair of the goods;
 * iii. the payment of the cost of replacing the goods or of acquiring
 *  equivalent goods;
 * iv.  the payment of the cost of having the goods repaired; or
 * b.  in the case of services:
 * i.  the supplying of the services again; or
 * ii.  the payment of the cost of having the services supplied again.
 * 
 * The construction, validity and performance of this licence is governed
 * by the laws in force in New South Wales, Australia.
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>

#include <elf32.h>

#include "binfostruct.h"

#define MY_BINFO_MAGIC 0x14b0021d /* So we don't have to include L4 headers */
#define BINFO_SIMPLEELF 0x37

int
is_bootinfo(struct aiarchinfo *arch, void *data)
{
	/* Just check the signature for now */
	if (ai_read(arch, dite_bootinfo, "magic", data) == MY_BINFO_MAGIC)
		return 1;
	
	return 0;
}

void
indent(int indentlevel)
{
	while (indentlevel--)
		printf("\t");
}

void
display_bootinfo(struct aiarchinfo *arch, void *data)
{
	int i, count;
	void *binfo;

	binfo = data + ai_read(arch, dite_bootinfo, "first_entry", data); 
	count = ai_read(arch, dite_bootinfo, "num_entries", data);
	i = 0;

	while (i < count) {
		if (ai_read(arch, dite_bootrec, "type", binfo) == BINFO_SIMPLEELF) {
			void *simpleelf_header;
			printf("Simple Elf structure: %s\n", 
					binfo + ai_read(arch, dite_binfo_simpleelf, "cmdline_offset", binfo));
			simpleelf_header = binfo +
					ai_read(arch, dite_binfo_simpleelf, "elf_header_offset", binfo);
			assert(arch->word_size == AI_WS_32BIT); 
			show_elf32_info(simpleelf_header, 1);
		}
		binfo += ai_read(arch, dite_bootrec, "offset_next", binfo);
		i++;
	}
}

int
show_elf32_info(void *mfile, int il)
{
	int i, total;
	struct aiarchinfo arch;
	
	/* FIXME */
	arch.word_size = AI_WS_32BIT;
	arch.endianness = AI_E_LSB;

	total = elf32_getNumSections(mfile);
	indent(il); printf("Number of sections: %d\n", total);

	elf32_printStringTable(mfile);
	
	for (i=0; i < total; i++) {
		indent(il); printf("Section %d: %s\n", i, elf32_getSectionName(mfile, i));
	}

	total = elf32_getNumProgramSegments(mfile);
	indent(il); printf("Number of segments: %d\n", total);

	/* Search for a BootInfo segment and read that. */
	for (i=0; i < total; i++) {
		uint64_t p_vaddr, p_paddr, p_filesz, p_offset, p_memsz;
		elf32_getSegmentInfo(mfile, i, &p_vaddr, &p_paddr, &p_filesz, &p_offset,
				&p_memsz);
		void *bootinfo = mfile + p_offset;
		if (is_bootinfo(&arch, bootinfo)) {
			indent(il); printf("Bootinfo found.\n");
			display_bootinfo(&arch, bootinfo);
		}
	}
}

int
show_elf_info(char *filename)
{
	int fd;
	void *mfile;
	struct stat statbuf;
	
	fd = open(filename, O_RDONLY);
	if (fd < 0) {
		perror("Couldn't open file.");
		return 1;
	}

	fstat(fd, &statbuf);

	mfile = mmap(0, statbuf.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
	if (mfile == MAP_FAILED) {
		perror("Couldn't mmap file.\n");
		return 1;
	}

	/* Find out if it's the sort of elf file we can deal with. */
	if (elf32_checkFile(mfile) != 0) {
		printf("Unsupported elf file. :(\n");
		return 1;
	}
	/* Display info on it! */
	show_elf32_info(mfile, 0);

	munmap(mfile, statbuf.st_size);
	close(fd);
}

int
main(int argc, char **argv)
{
	if (argc != 2) {
		printf("usage: %s <elf-filename>\n", argv[0]);
		exit(1);
	}

	show_elf_info(argv[1]);
}
#endif
