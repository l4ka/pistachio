/****************************************************************************
 *
 * Copyright (C) 2003, University of New South Wales
 *
 * File path:	piggybacker/ofppc64/donote.cc
 * Description:	Add a note program header to the loader image
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
 * $Id: donote.cc,v 1.1 2003/10/24 04:46:53 cvansch Exp $
 *
 ***************************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>


char *target = "PowerPC";

typedef unsigned int	uint;

typedef struct {
    uint real_mode;
    uint read_base;
    uint read_size;
    uint virt_base;
    uint virt_size;
    uint load_base;
} of_note_t;

#define read_be8(a)	buf[a]
#define read_be16(a)	(unsigned short)((buf[a] << 8) | buf[(a)+1])
#define read_be32(a)	(unsigned short)(((int)buf[a] << 24) | ((int)buf[(a)+1] << 16) | ((int)buf[(a)+2] << 8) | buf[(a)+3])
#define write_be16(a,x)	{ buf[a] = (x)>>8; buf[(a)+1] = (x)&0xff; }
#define write_be32(a,x)	{ buf[a] = (x)>>24; buf[(a)+1] = ((x)>>16)&0xff; buf[(a)+2] = ((x)>>8)&0xff; buf[(a)+3] = (x)&0xff; }


/* Structure of an ELF file */
#define E_IDENT		0	/* ELF header */
#define E_MACHINE	18
#define	E_PHOFF		28
#define E_PHENTSIZE	42
#define E_PHNUM		44
#define E_HSIZE		52	/* size of ELF header */

#define EI_MAGIC	0	/* offsets in E_IDENT area */
#define EI_CLASS	4
#define EI_DATA		5

#define PH_TYPE		0	/* ELF program header */
#define PH_OFFSET	4
#define PH_FILESZ	16
#define PH_HSIZE	32	/* size of program header */

#define PT_NOTE		4	/* Program header type = note */

#define ELFCLASS32	1
#define ELFDATA2MSB	2

#define EM_PPC		20	/* PowerPC */

char *elf_magic = "\177ELF";
of_note_t note = { -1u, 0xc00000, -1u, -1u, -1u, 0x4000 };

unsigned char buf[512];

int main( int argc, char **argv )
{
    int file;
    int note_size, count;
    int valid = 1, i;
    int phoff, tsize, tcount;
    int noteoff;

    if ( argc != 2 )
    {
	printf( "usage: %s loader-image\n", *argv );
	return 1;
    }

    if ( (file = open(argv[1], O_RDWR)) < 0 )
    {
	perror( argv[1] );
	return 1;
    }

    note_size = (strlen(target) + 1) + sizeof(of_note_t) + 12;

    if ( (count = read( file, buf, sizeof(buf) )) < 0 )
    {
	perror( argv[1] );
	return 1;
    }

    for ( i = 0; i < 4; i++ )
	if (buf[i] != elf_magic[i]) valid = 0;

    phoff =  read_be32(E_PHOFF);
    tsize =  read_be16(E_PHENTSIZE);
    tcount = read_be16(E_PHNUM);

    if ( (count < E_HSIZE) || (!valid) || (phoff < E_HSIZE) || (tsize < PH_HSIZE) || (tcount < 1) )
    {
	printf( "%s is not a valid elf file\n", argv[1] );
	return 1;
    }

    if ( (read_be16(E_MACHINE) != EM_PPC) ||
	 (read_be8 (EI_CLASS) != ELFCLASS32) ||
	 (read_be8 (EI_DATA) != ELFDATA2MSB) )
    {
	printf( "%s is not a big endian ppc elf32 file\n", argv[1] );
	return 1;
    }

    if (phoff + (tcount + 1) * tsize + note_size > count)
    {
	printf( "The headers are too large or there is no space\n" );
	return 1;
    }

    for ( i = 0; i < tcount; i++ )
    {
	if ( read_be32(phoff + i * tsize + PH_TYPE) == PT_NOTE )
	{
	    printf( "A NOTE section already exits in file %s\n", argv[1] );
	    return 1;
	}
    }

    noteoff = phoff + tcount * tsize + PH_TYPE;

    for (i = 0; i < (tsize + note_size); i++)
	if (read_be8 (noteoff + i) != 0)
	{
	    printf( "No free space in the headers was found\n" );
	    return 1;
	}

    write_be32(noteoff + PH_TYPE, PT_NOTE);
    write_be32(noteoff + PH_OFFSET, noteoff+tsize);
    write_be32(noteoff + PH_FILESZ, note_size);

    noteoff += tsize;
    write_be32(noteoff, strlen(target) + 1);
    write_be32(noteoff + 4, sizeof(of_note_t));
    write_be32(noteoff + 8, 0x1275);

    strcpy(&(char)buf[noteoff + 12], target);
    noteoff += 12 + strlen(target) + 1;

    for (i = 0; i < sizeof(of_note_t); i+= 4)
	write_be32( noteoff + i, ((uint*)&note)[i/4] );

    write_be16( E_PHNUM, tcount + 1 );

    lseek( file, 0l, SEEK_SET );
    i = write (file, buf, count );

    if (i < count)
    {
	printf( "error writing to %s\n", argv[1] );
    }

    return 0;
}

