######################################################################
##                
## Copyright (C) 2003-2004, 2007,  Karlsruhe University
##                
## File path:     config.mk.in
## Description:   Configuration settings for current build
##                
## Redistribution and use in source and binary forms, with or without
## modification, are permitted provided that the following conditions
## are met:
## 1. Redistributions of source code must retain the above copyright
##    notice, this list of conditions and the following disclaimer.
## 2. Redistributions in binary form must reproduce the above copyright
##    notice, this list of conditions and the following disclaimer in the
##    documentation and/or other materials provided with the distribution.
## 
## THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
## ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
## IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
## ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
## FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
## DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
## OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
## HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
## LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
## OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
## SUCH DAMAGE.
##                
## $Id: config.mk.in,v 1.13 2006/10/21 04:08:17 reichelt Exp $
##                
######################################################################

# config.mk.  Generated from config.mk.in by configure.

ARCH=		amd64
PLAT=           amd64-pc99
KERNEL=		x86-kernel

prefix=		/usr/local
exec_prefix=	${prefix}
includedir=	${prefix}/include
libdir=		${exec_prefix}/lib/l4
libexecdir=	${exec_prefix}/libexec/l4
kerneldir=	$(top_builddir)


SHELL=		/bin/bash
CC=		gcc
CXX=		$(CC) -x c++
AS=		$(CC)
CFLAGS=		-fno-stack-protector -nostdinc -g -O2 -m64 -mno-red-zone
CXXFLAGS=	$(CFLAGS) -fno-exceptions
LDFLAGS=	-N -L$(top_builddir)/lib -L/usr/lib/gcc/x86_64-linux-gnu/4.8 -nostdlib  -melf_x86_64
CPPFLAGS=	-I$(top_srcdir)/include -I$(top_builddir) -I/usr/lib/gcc/x86_64-linux-gnu/4.8/include 
LGCC=		-lgcc

TOOLPREFIX=	
AR=		$(TOOLPREFIX)ar
RANLIB=		$(TOOLPREFIX)ranlib
LD=		$(TOOLPREFIX)ld
OBJCOPY=	$(TOOLPREFIX)objcopy
RMDIR=		rmdir
LN_S=		ln -s
AUTOCONF=	autoconf
MKDIRHIER=	$(top_srcdir)/../tools/mkdirhier
AWK=		mawk

INSTALL=	/usr/bin/install -c
INSTALL_PROGRAM=${INSTALL}
INSTALL_DATA=	${INSTALL} -m 644
INSTALL_SCRIPT=	${INSTALL}

KICKSTART_LINKBASE=	00100000
SIGMA0_LINKBASE=	00f00000
ROOTTASK_LINKBASE=	01000000
