######################################################################
##                
## Copyright (C) 2003,  Karlsruhe University
##                
## File path:     l4.base.mk
## Description:   Generic settings for Pistachio user-level build
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
## $Id: l4.base.mk,v 1.5 2003/09/24 19:05:50 skoglund Exp $
##                
######################################################################

include $(top_builddir)/config.mk

ECHO=		echo
ECHO_MSG=	$(ECHO) ===\>
MKDIRHIER=	$(top_srcdir)/../tools/mkdirhier

CPPFLAGS+=	$(CPPFLAGS_$(ARCH))
CFLAGS+=	-Wall -Wshadow -Wconversion \
		$(CFLAGS_$(ARCH))
LDFLAGS+=	$(LDFLAGS_$(ARCH))

# Create early targets so that a make without args (implicit all) does
# not take the first target in worker makefile (e.g., a clean target).

all:		pre-all do-all post-all
install:	pre-install do-install post-install
clean:		pre-clean do-clean post-clean

pre-all:
pre-install:
pre-clean:

post-all:
post-install:
post-clean:


.SUFFIXES:
.SUFFIXES: .cc .c .S .o .mod .bin

