/*
 * Copyright (c) 1999 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * Portions Copyright (c) 1999 Apple Computer, Inc.  All Rights
 * Reserved.  This file contains Original Code and/or Modifications of
 * Original Code as defined in and that are subject to the Apple Public
 * Source License Version 1.1 (the "License").  You may not use this file
 * except in compliance with the License.  Please obtain a copy of the
 * License at http://www.apple.com/publicsource and read it before using
 * this file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE OR NON- INFRINGEMENT.  Please see the
 * License for the specific language governing rights and limitations
 * under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */
#ifdef SHLIB
#include "shlib.h"
#endif
/*
 *	File:	memory_funcs.c
 *	Author:	Avadis Tevanian, Jr.
 *
 *	Streams implemented as buffered memory operations.
 */

#include "defs.h"
#include "streamsextra.h"
#include <mach/mach.h>
#include <mach/mach_types.h>
#include <mach/machine/vm_param.h> 
#include <libc.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <mach-o/loader.h>
#include <mach-o/fat.h>
#include <mach-o/swap.h>

/* 
 * getsectbynamefromheaderwithswap
 *
 * NOTE:  This comes from libc_proj getsecbyname.c.  It is not currently public because it
 * is new for 3.2, and as such programs using it in 3.2 would not work under 3.1.
 * At some point this should become public and the extern declaration should move to libc.h
 */
extern const struct section *getsectbynamefromheaderwithswap(struct mach_header *mhp, const char *segname, const char *sectname, int fSwap);

static int memory_flush(register NXStream *s);
static int memory_fill(register NXStream *s);
static void memory_change(register NXStream *s);
static void memory_seek(register NXStream *s, long offset);
static void memory_close(NXStream *s);
static int cheap_flush(NXStream *s);
static void cheap_seek(NXStream *s, long offset);
static void cheap_close(NXStream *s);

static void verify_memory_stream(NXStream *s);

/*
 * bool defines
 */
#define NO 0
#define YES 1

static struct stream_functions memory_functions = {
	NXDefaultRead,
	NXDefaultWrite,
	memory_flush,
	memory_fill,
	memory_change,
	memory_seek,
	memory_close,
};

static struct stream_functions cheap_functions = {
	NXDefaultRead,
	NXDefaultWrite,
	cheap_flush,
	memory_fill,
	memory_change,
	cheap_seek,
	cheap_close,
};


/* verifies we have a memory stream.  Call this in all memory stream
   specific routines AFTER calling _NXVerifyStream.
 */
static void verify_memory_stream(NXStream *s)
{
    if ((s->functions != &memory_functions) &&
	(s->functions != &cheap_functions))
	NX_RAISE(NX_illegalStream, s, 0);
}


#define INITIAL_SIZE (256)		/* initial size of cheap streams */
#define MAX_MALLOC vm_page_size		/* max malloc size of cheap streams */
#define	CHUNK_SIZE	(256*1024)	/* XXX */
/*
 *	memory_extend: Extend the memory region to at least the
 *	specified size.
 */

static void memory_extend(register NXStream *s, int size)
{
    vm_size_t       new_size;
    vm_offset_t     new_addr;
    int             cur_offset;
    kern_return_t   ret;

    new_size = (size + CHUNK_SIZE) & (~(vm_page_size - 1));
    ret = vm_allocate(mach_task_self(), &new_addr, new_size, TRUE);
    if (ret != KERN_SUCCESS)
	NX_RAISE(NX_streamVMError, s, (void *)ret);
    cur_offset = 0;
    if (s->buf_base) {
	int             copySize;

	copySize = s->buf_size;
	if (copySize % vm_page_size)
	    copySize += vm_page_size - (copySize % vm_page_size);
	ret = vm_copy(mach_task_self(),
		      (vm_offset_t)s->buf_base,
		      (vm_size_t)copySize,
		      (vm_offset_t)new_addr);
	if (ret != KERN_SUCCESS)
	    NX_RAISE(NX_streamVMError, s, (void *)ret);
	ret = vm_deallocate(mach_task_self(),
			    (vm_offset_t)s->buf_base,
			    (vm_size_t)s->buf_size);
	if (ret != KERN_SUCCESS)
	    NX_RAISE(NX_streamVMError, s, (void *)ret);
	cur_offset = s->buf_ptr - s->buf_base;
    }
    s->buf_base = (unsigned char *)new_addr;
    s->buf_size = new_size;
    s->buf_ptr = s->buf_base + cur_offset;
    s->buf_left = new_size - size;
    s->flags &= ~NX_USER_OWNS_BUF;
}

/*
 *	memory_flush:  flush a stream buffer.
 */

static int memory_flush(register NXStream *s)
{

    int bufSize = s->buf_size - s->buf_left;
    int reading = (s->flags & NX_READFLAG);

    if (reading)
	return 0;
    if (bufSize > s->eof)
	s->eof = bufSize;
    if (s->buf_left > 0)
	return bufSize;
    memory_extend(s, bufSize);
    if (bufSize > s->eof)
	s->eof = bufSize;
    return bufSize;
}

/*
 *	memory_fill:  fill a stream buffer, return how much
 *	we filled.
 */

static int memory_fill(register NXStream *s)
{
    return 0;
}

static void memory_change(register NXStream *s)
{
    long offset = NXTell(s);

    if (offset > s->eof)
	s->eof = offset;
    s->buf_ptr = s->buf_base + offset;
    if (s->flags & NX_WRITEFLAG) {
	s->buf_left = s->eof - offset;
    } else {
	s->buf_left = s->buf_size - offset;
    }
}

static void memory_seek(register NXStream *s, long offset)
{
    int bufSize = s->buf_size - s->buf_left;

    if (s->flags & NX_READFLAG) {
	if (offset > s->eof)
	    NX_RAISE(NX_illegalSeek, s, 0);
	s->buf_ptr = s->buf_base + offset;
	s->buf_left = s->eof - offset;
    } else {
	if (bufSize > s->eof)
	    s->eof = bufSize;
	while (s->buf_size < offset)
	    memory_extend(s, s->buf_size);
	if (offset > s->eof)
	    s->eof = offset;
	s->buf_ptr = s->buf_base + offset;
	s->buf_left = s->buf_size - offset;
    }
}


static void memory_close(NXStream *s)
{
    vm_offset_t     end;
    int             userBuf;
    kern_return_t   ret;

 /*
  * Free extra pages after end of buffer. 
  */
    userBuf = (s->flags & NX_USER_OWNS_BUF) != 0;
    if ((s->flags & NX_CANWRITE) && !userBuf) {
	int remove;
	int bufSize = s->buf_size - s->buf_left;

	if (bufSize > s->eof)
	    s->eof = bufSize;
	end = round_page((vm_size_t)(s->buf_base + s->eof));
	remove = s->buf_size - (end - (vm_offset_t) s->buf_base);
	ret = vm_deallocate(mach_task_self(), end, remove);
	if (ret != KERN_SUCCESS)
	    NX_RAISE(NX_streamVMError, s, (void *)ret);
	s->buf_size -= remove;
    }
}

 /*
  * betterCopy: 
  *
  * Copy routine for the realloc functionality of the small memory streams.
  * Tries to use vm_copy whenever possible, but then defaults to bcopy when
  * this is not possible. 
  */
  
static void betterCopy(void *source, unsigned bytes, void *dest, NXStream *s) {
    if (bytes < vm_page_size ||
	(((unsigned)source | (unsigned)dest) & (vm_page_size - 1))) {
	bcopy(source, dest, bytes);
    } else {
	kern_return_t ret;
        ret = vm_copy(mach_task_self(), (vm_address_t)source, round_page(bytes), 
	    (vm_address_t)dest);
	if (ret != KERN_SUCCESS)
	    NX_RAISE(NX_streamVMError, s, (void *)ret);
    }
}

 /*
  * cheap_extend: 
  *
  * Like memory_extend of memory streams, this uses malloc for buffers under
  * MAX_MALLOC bytes, otherwise uses vm_allocate to extend page aligned data. 
  */
 
static void cheap_extend(NXStream *s, int desiredSize)
{
    int		    new_size = s->buf_size;
    char	   *new_addr;
    int		    cur_offset = 0;
    kern_return_t   ret;
    
    while (new_size < desiredSize)
	new_size = new_size * 2;
    if (new_size >= MAX_MALLOC && new_size < CHUNK_SIZE)
	new_size = CHUNK_SIZE;
    if (new_size < MAX_MALLOC)
	new_addr = malloc(new_size);
    else {
	new_size = round_page(new_size);
	ret = vm_allocate(mach_task_self(), (vm_address_t *)&new_addr, new_size, 
	    TRUE);
	if (ret != KERN_SUCCESS)
	    NX_RAISE(NX_streamVMError, s, (void *)ret);
    }
    if (s->buf_base) {
	cur_offset = s->buf_ptr - s->buf_base;
	betterCopy(s->buf_base, s->buf_size, new_addr, s);
	if (s->buf_size < MAX_MALLOC)
	    free(s->buf_base);
	else {
	    ret = vm_deallocate(mach_task_self(),
				(vm_offset_t)s->buf_base,
				(vm_size_t)s->buf_size);
	    if (ret != KERN_SUCCESS)
		NX_RAISE(NX_streamVMError, s, (void *)ret);
	}
    }
    s->buf_base = (unsigned char *)new_addr;
    s->buf_left += new_size - s->buf_size;
    s->buf_size = new_size;
    s->buf_ptr = s->buf_base + cur_offset;
}

 /*
  * cheap_flush: 
  *
  * like memory_flush, but needs to use cheap_extend instead of memory_extend.
  * It uses a slightly different algorithm for extending the buffer size, 
  *
  */
 
static int cheap_flush(NXStream *s)
{

    int bufSize = s->buf_size - s->buf_left;
    int reading = (s->flags & NX_READFLAG);

    if (reading)
	return 0;
    if (bufSize > s->eof)
	s->eof = bufSize;
    if (s->buf_left > 0)
	return bufSize;
    cheap_extend(s, s->buf_size * 2);
    if (bufSize > s->eof)
	s->eof = bufSize;
    return bufSize;
}

 /*
  * cheap_seek: 
  *
  * like memory_seek, but needs to use cheap_extend instead of memory_extend.
  *
  */
static void cheap_seek(NXStream *s, long offset)
{
    int bufSize = s->buf_size - s->buf_left;

    if (s->flags & NX_READFLAG) {
	if (offset > s->eof)
	    NX_RAISE(NX_illegalSeek, s, 0);
	s->buf_ptr = s->buf_base + offset;
	s->buf_left = s->eof - offset;
    } else {
	if (bufSize > s->eof)
	    s->eof = bufSize;
	while (s->buf_size < offset)
	    cheap_extend(s, offset);
	if (offset > s->eof)
	    s->eof = offset;
	s->buf_ptr = s->buf_base + offset;
	s->buf_left = s->buf_size - offset;
    }
}


 /*
  * cheap_close: 
  *
  * closes the stream and frees all associated memory.
  *
  */

static void cheap_close(NXStream *s)
{
    if (s->buf_size < MAX_MALLOC)
	free(s->buf_base);
    else {
	vm_deallocate(mach_task_self(), (vm_offset_t)s->buf_base,
	    round_page(s->buf_size));
    }
}

/*
 *	Begin memory stream specific exported routines.
 */

/*
 *	NXOpenMemory:
 *
 *	open a stream using a memory backing.  Initial address and
 *	size are specified for a buffer.  Address must be paged aligned
 *	and size must be a multiple of a page!  (They should be the
 *	result of a vm_allocate).  Address and size may be 0.
 */

NXStream *NXOpenMemory(const char *addr, int size, int mode)
{
    NXStream		*s;
    int			newMode = mode;

    if ((int)addr % vm_page_size && (mode != NX_READONLY))
	return 0;

    s = NXStreamCreate(newMode, FALSE);
    s->functions = &memory_functions;
    s->buf_base = (unsigned char *)addr;
    s->buf_size = size;
    s->buf_left = size;
    s->buf_ptr = (unsigned char *)addr;
    s->eof = size;
    s->flags |= NX_CANSEEK;
    if (addr)
	s->flags |= NX_USER_OWNS_BUF;
    else
	switch(mode) {
	    case NX_READONLY:
		break;
	    case NX_WRITEONLY:
		memory_flush(s);	/* get initial buffer */
		break;
	    case NX_READWRITE:
		if (s->flags & NX_READFLAG) {
		    NXChangeBuffer(s);
		    memory_flush(s);	/* get initial buffer */
		}
		break;
	}
    return (s);
}

 /*
  * NXOpenSmallMemory: 
  *
  * open a stream using a memory backing.  mode may only be NX_WRITEONLY or
  * NX_READWRITE. The memory buffer is malloced if under MAX_MALLOC bytes,
  * otherwise it is vm_allocated. 
  */

NXStream *NXOpenSmallMemory(int mode){
    NXStream		*s;
    int newMode = mode;
    char *addr;
    if (mode != NX_WRITEONLY && mode != NX_READWRITE)
	return NULL;
    addr = malloc(INITIAL_SIZE);
    s = NXStreamCreate(newMode, FALSE);
    s->functions = &cheap_functions;
    s->buf_base = (unsigned char *)addr;
    s->buf_size = INITIAL_SIZE;
    s->buf_left = INITIAL_SIZE;
    s->buf_ptr = (unsigned char *)addr;
    s->eof = 0;
    s->flags |= NX_CANSEEK;
    switch(mode) {
	case NX_READWRITE:
	    if (s->flags & NX_READFLAG) {
		NXChangeBuffer(s);
	    }
	    break;
    }
    return (s);
}

NXStream *NXMapFile(const char *name, int mode)
{
    int             fd;
    char           *buf;
    struct stat     info;
    NXStream 	   *s = NULL;

    fd = open(name, O_RDONLY, 0666);
    if (fd >= 0) {
	if (fstat(fd, &info) >= 0) {
	    if (info.st_size > 0 || (info.st_mode & S_IFMT) == S_IFDIR) {
		if (map_fd(fd, 0, (vm_offset_t *)&buf, TRUE, (vm_size_t)info.st_size) ==
								KERN_SUCCESS) {
		    s = NXOpenMemory(buf, info.st_size, mode);
		    s->flags &= ~NX_USER_OWNS_BUF;
		}
	    } else {
		s = NXOpenMemory(NULL, 0, mode);
	    }
	}
	if (close(fd) < 0) {
	    NXCloseMemory(s, NX_FREEBUFFER);
	    s = NULL;
	}
    }
    return s;
}


int NXSaveToFile(register NXStream *s, const char *name )
{
    int             fd;
    char           *buf;
    int             len, max, retval;

    _NXVerifyStream(s);
    verify_memory_stream(s);
    fd = open(name, O_WRONLY | O_CREAT, 0666);
    if (fd >= 0) {
	NXGetMemoryBuffer(s, &buf, &len, &max);
	retval = write(fd, buf, len);
	if (retval < 0)
	    return retval;
	retval = ftruncate(fd, len);
	if (retval < 0)
	    return retval;
	retval = fsync(fd);
	if (retval < 0)
	    return retval;
	retval = close(fd);
	if (retval < 0)
	    return retval;
    } else
	return -1;
    return 0;
}

void NXCloseMemory(register NXStream *s, int option)
{
    int userBuf;
    int isCheapStream;

    _NXVerifyStream(s);
    verify_memory_stream(s);
    isCheapStream = (s->functions == &cheap_functions) ? 1:0;
    userBuf = (s->flags & NX_USER_OWNS_BUF) != 0;
    if (!userBuf) {
	switch (option) {
	    case NX_FREEBUFFER:
		if (isCheapStream && s->buf_size < MAX_MALLOC)
		    free(s->buf_base);
		else {
		    kern_return_t ret = vm_deallocate(mach_task_self(),
			(vm_offset_t)s->buf_base, (vm_size_t)s->buf_size);
		    if (ret != KERN_SUCCESS)
			NX_RAISE(NX_streamVMError, s, (void *)ret);
		}
		break;
	    case NX_TRUNCATEBUFFER:
		if (!isCheapStream || s->buf_size >= MAX_MALLOC)
		    memory_close(s);
		break;
	    case NX_SAVEBUFFER: break;
	}
    }
    NXStreamDestroy(s);
}


void NXGetMemoryBuffer(NXStream *s, char **addr, int *len, int *maxlen)
{
    int bufSize = s->buf_size - s->buf_left;
    
    _NXVerifyStream(s);
    verify_memory_stream(s);
    *addr = (char *)s->buf_base;
    *len = ((s->flags&NX_READFLAG) || (bufSize <= s->eof)) ? s->eof : bufSize;
    *maxlen = s->buf_size;
}

/*
 *	NXGetStreamOnSection -- open a stream on some section of a Mach-O file.
 *	Note: In supporting fat binaries, this routine will fail if no proper match 
 *	is found for the current host cpu-type.
 */

static int check_wellformed_header(struct mach_header *mhp, unsigned size, int fSwap)
{
    long	i;
    struct segment_command	*sgp;
    if (mhp->sizeofcmds + sizeof(struct mach_header) > size) return 0;
    sgp = (struct segment_command*)((char*)mhp + sizeof(struct mach_header));
    for (i = 0; i < mhp->ncmds; i++) {
	if ((unsigned)sgp & (sizeof(unsigned) - 1)) return 0;  // check allignment
	if (((char*)sgp - (char*)mhp + sizeof(struct segment_command)) > size) return 0;
	sgp = (struct segment_command*)((char*)sgp + (fSwap ? NXSwapLong(sgp->cmdsize) : sgp->cmdsize));
    }
    if (((char*)sgp - (char*)mhp) > size) return 0;
    return 1;
}

NXStream *NXGetStreamOnSection(const char *fileName, const char *segmentName, const char *sectionName)
{
    int             fd;
    struct stat     info;
    NXStream 	   *s = NULL;
    struct fat_header *fh;
    struct mach_header *mh;
    const struct section *sect;
    vm_offset_t mh_page, sect_page;
    unsigned long archOffset;
    unsigned int cnt = HOST_BASIC_INFO_COUNT;
    struct host_basic_info hbi;

    if (host_info(mach_host_self(), HOST_BASIC_INFO, (host_info_t)(&hbi), &cnt) != KERN_SUCCESS)
      return NULL;

    fd = open(fileName, O_RDONLY, 0444);
    if (fd < 0 || fstat(fd, &info) < 0)
    	return NULL;

    if (((info.st_mode & S_IFMT) != S_IFREG) || (info.st_size < sizeof(*fh))) {
	close(fd);
	return NULL;
    }

    if (map_fd(fd, 0, (vm_offset_t *)&fh, TRUE, (vm_size_t)info.st_size) != KERN_SUCCESS) {
	close(fd);
	return NULL;
    }

#ifdef __BIG_ENDIAN__
    if (fh->magic == FAT_MAGIC) {
#endif __BIG_ENDIAN__
#ifdef __LITTLE_ENDIAN__
    if (fh->magic == NXSwapLong(FAT_MAGIC)) {
#endif __LITTLE_ENDIAN__
	int i;
	struct fat_arch *fa = (struct fat_arch*)(fh + 1);
#ifdef __LITTLE_ENDIAN__
	enum NXByteOrder host_byte_sex = NXHostByteOrder();
	swap_fat_header(fh, host_byte_sex);
#endif __LITTLE_ENDIAN__
	if ((fh->nfat_arch <= 0) || (info.st_size < sizeof(*fh)+sizeof(*fa)*fh->nfat_arch)) {
		vm_deallocate(mach_task_self(), (vm_offset_t)fh, info.st_size);
		close(fd);
		return NULL;
	}
#ifdef __LITTLE_ENDIAN__
	swap_fat_arch(fa, fh->nfat_arch, host_byte_sex);
#endif __LITTLE_ENDIAN__
	for (i = 0; i < fh->nfat_arch; i++, fa++) {
		if (fa->cputype == hbi.cpu_type) {
//****		** check for best cpu_subtype here ** (fa->cpusubtype == hbi.cpu_subtype)
			break;	// for now, accept all subtypes
		}
	}
	if (i >= fh->nfat_arch) {
		vm_deallocate(mach_task_self(), (vm_offset_t)fh, info.st_size);
		close(fd);
 		return NULL;
	}
	archOffset = fa->offset;
	mh = (struct mach_header*)((char*)fh + archOffset);
    } else {
	archOffset = 0L;
    	mh = (struct mach_header*)fh;
    }
    
    if ((info.st_size < archOffset + sizeof(*mh)) ||
	(mh->magic != MH_MAGIC) || (mh->cputype != hbi.cpu_type) ||
	(info.st_size < archOffset + sizeof(*mh) + mh->sizeofcmds) ||
	!check_wellformed_header(mh, info.st_size - archOffset, NO)) { // bug#21223
	vm_deallocate(mach_task_self(), (vm_offset_t)fh, info.st_size);
	close(fd);
 	return NULL;
    }

    /*
     * Get the section data.
     */
    sect = getsectbynamefromheader(mh, segmentName, sectionName);
    if (sect == NULL || sect->size == 0 ||
	(info.st_size < archOffset + sect->offset + sect->size)) {
	vm_deallocate(mach_task_self(), (vm_offset_t)fh, info.st_size);
	close(fd);
	return NULL;
    }

    /*
     * Create the stream.
     */
    s = NXOpenMemory((char *)mh + sect->offset, sect->size,
	NX_READONLY);
    s->flags &= ~NX_USER_OWNS_BUF;

    /*
     * Through away the parts of the file not needed.  Assert that all
     * pages that the file lives on are used only by the file.
     */
    sect_page = round_page((vm_offset_t)mh + sect->offset + sect->size);
    mh_page = round_page((vm_offset_t)fh + info.st_size);
    if (mh_page - sect_page)
	vm_deallocate(mach_task_self(), sect_page, mh_page - sect_page);

    mh_page = trunc_page((vm_offset_t)fh);
    sect_page = trunc_page((vm_offset_t)mh + sect->offset);
    if (sect_page - mh_page)
	vm_deallocate(mach_task_self(), mh_page, sect_page - mh_page);

    if (close(fd) < 0) {
	NXCloseMemory(s, NX_FREEBUFFER);
	s = NULL;
    }

    return s;
}



NXStream *NXGetStreamOnSectionForBestArchitecture(
	const char *fileName,
	const char *segmentName,
	const char *sectionName)
{
    int             fd;
    struct stat     info;
    NXStream 	   *s = NULL;
    struct fat_header *fh;
    struct mach_header *mh;
    const struct section *sect;
    vm_offset_t mh_page, sect_page;
    unsigned long archOffset;
    unsigned int cnt = HOST_BASIC_INFO_COUNT;
    struct host_basic_info hbi;
    int fSwap = NO;

    if (host_info(mach_host_self(), HOST_BASIC_INFO, (host_info_t)(&hbi), &cnt) != KERN_SUCCESS)
      return NULL;

    fd = open(fileName, O_RDONLY, 0444);
    if (fd < 0 || fstat(fd, &info) < 0)
    	return NULL;

    if (((info.st_mode & S_IFMT) != S_IFREG) || (info.st_size < sizeof(*fh))) {
	close(fd);
	return NULL;
    }

    if (map_fd(fd, 0, (vm_offset_t *)&fh, TRUE, (vm_size_t)info.st_size) != KERN_SUCCESS) {
	close(fd);
	return NULL;
    }

#ifdef __BIG_ENDIAN__
    if (fh->magic == FAT_MAGIC) {
#endif __BIG_ENDIAN__
#ifdef __LITTLE_ENDIAN__
    if (fh->magic == NXSwapLong(FAT_MAGIC)) {
#endif __LITTLE_ENDIAN__
	int i;
	struct fat_arch *fa = (struct fat_arch*)(fh + 1);
#ifdef __LITTLE_ENDIAN__
	enum NXByteOrder host_byte_sex = NXHostByteOrder();
	swap_fat_header(fh, host_byte_sex);
#endif __LITTLE_ENDIAN__
	if ((fh->nfat_arch <= 0) || (info.st_size < sizeof(*fh)+sizeof(*fa)*fh->nfat_arch)) {
	    vm_deallocate(mach_task_self(), (vm_offset_t)fh, info.st_size);
	    close(fd);
	    return NULL;
	}
#ifdef __LITTLE_ENDIAN__
	swap_fat_arch(fa, fh->nfat_arch, host_byte_sex);
#endif __LITTLE_ENDIAN__
	for (i = 0; i < fh->nfat_arch; i++, fa++) {
	    if (fa->cputype == hbi.cpu_type) {
//****		** check for best cpu_subtype here ** (fa->cpusubtype == hbi.cpu_subtype)
		break;	// for now, accept all subtypes
	    }
	}
	if (i >= fh->nfat_arch) {
	    /*
	     * If do not have the correct cpu_type, just use the last type
	     * in file.
	     * NOTE: we could have a list passed in, and choose the best
	     *       based upon that list.
	     */
	    fa--;
	}
	archOffset = fa->offset;
	mh = (struct mach_header*)((char*)fh + archOffset);
    } else {
	archOffset = 0L;
    	mh = (struct mach_header*)fh;
    }
    
    if (info.st_size < archOffset + sizeof(*mh)) {
	vm_deallocate(mach_task_self(), (vm_offset_t)fh, info.st_size);
	close(fd);
 	return NULL;
    }
    
    /* 
     * Do we need to swap the header?  Header is always in byte-order of machine it
     * was compiled for.
     */
    if (mh->magic == NXSwapLong(MH_MAGIC)) {
	fSwap = YES;
#ifdef __LITTLE_ENDIAN__
	swap_mach_header(mh, NX_LittleEndian);
#else
	swap_mach_header(mh, NX_BigEndian);
#endif __LITTLE_ENDIAN__
    }

    
    if ((mh->magic != MH_MAGIC) ||
	(info.st_size < archOffset + sizeof(*mh) + mh->sizeofcmds) ||
	!check_wellformed_header(mh, info.st_size - archOffset, fSwap)) { // bug#21223
	vm_deallocate(mach_task_self(), (vm_offset_t)fh, info.st_size);
	close(fd);
 	return NULL;
    }

    /*
     * Get the section data.
     */
    sect = getsectbynamefromheaderwithswap(mh, segmentName, sectionName, fSwap);
    if (sect == NULL || sect->size == 0 ||
	(info.st_size < archOffset + sect->offset + sect->size)) {
	vm_deallocate(mach_task_self(), (vm_offset_t)fh, info.st_size);
	close(fd);
	return NULL;
    }

    /*
     * Create the stream.
     */
    s = NXOpenMemory((char *)mh + sect->offset, sect->size, NX_READONLY);
    s->flags &= ~NX_USER_OWNS_BUF;

    /*
     * Through away the parts of the file not needed.  Assert that all
     * pages that the file lives on are used only by the file.
     */
    sect_page = round_page((vm_offset_t)mh + sect->offset + sect->size);
    mh_page = round_page((vm_offset_t)fh + info.st_size);
    if (mh_page - sect_page)
	vm_deallocate(mach_task_self(), sect_page, mh_page - sect_page);

    mh_page = trunc_page((vm_offset_t)fh);
    sect_page = trunc_page((vm_offset_t)mh + sect->offset);
    if (sect_page - mh_page)
	vm_deallocate(mach_task_self(), mh_page, sect_page - mh_page);

    if (close(fd) < 0) {
	NXCloseMemory(s, NX_FREEBUFFER);
	s = NULL;
    }

    return s;
}
