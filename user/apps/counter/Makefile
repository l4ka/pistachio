######################################################################
##                
## Copyright (C) 2003-2004, 2007,  Karlsruhe University
##                
## File path:     grabmem/Makefile.in
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
## $Id: Makefile.in,v 1.2 2004/04/22 15:08:18 skoglund Exp $
##                
######################################################################

srcdir=		.
top_srcdir=	../..
top_builddir=	../..

include $(top_srcdir)/Mk/l4.base.mk


PROGRAM=	counter
PROGRAM_DEPS=	$(top_builddir)/lib/l4/libl4.a \
		$(top_builddir)/lib/io/libio.a

SRCS=		crt0-$(ARCH).S  counter.cc mindrvr.c elmfat/src/ff.c elmfat/src/diskio.c mt19937ar.c

#lua-5.2.3/src/lapi.c lua-5.2.3/src/lcode.c lua-5.2.3/src/lctype.c lua-5.2.3/src/linit.c lua-5.2.3/src/ldebug.c lua-5.2.3/src/ldo.c lua-5.2.3/src/ldump.c lua-5.2.3/src/lfunc.c lua-5.2.3/src/lgc.c  lua-5.2.3/src/lmem.c lua-5.2.3/src/lopcodes.c lua-5.2.3/src/lstate.c lua-5.2.3/src/lstring.c lua-5.2.3/src/ltable.c lua-5.2.3/src/ltm.c lua-5.2.3/src/lundump.c lua-5.2.3/src/lvm.c lua-5.2.3/src/lzio.c lua-5.2.3/src/lauxlib.c lua-5.2.3/src/lbaselib.c lua-5.2.3/src/lbitlib.c lua-5.2.3/src/lcorolib.c lua-5.2.3/src/ldblib.c lua-5.2.3/src/liolib.c lua-5.2.3/src/lmathlib.c lua-5.2.3/src/loslib.c lua-5.2.3/src/lstrlib.c lua-5.2.3/src/ltablib.c lua-5.2.3/src/loadlib.c  lua-5.2.3/src/llex.c lua-5.2.3/src/lobject.c lua-5.2.3/src/lparser.c 

LIBS+=		-ll4 -lio 
LDFLAGS+=	-Ttext=$(ROOTTASK_LINKBASE)

CFLAGS_powerpc+=	-fno-builtin -msoft-float
CXXFLAGS_powerpc+=	-fno-rtti
CFLAGS_ia32 += -DLUA_COMPAT_ALL -std=c++11 -DINT_MAX -DUCHAR_MAX

include $(top_srcdir)/Mk/l4.prog.mk
