######################################################################
##                
## Copyright (C) 2003,  Karlsruhe University
##                
## File path:     l4.subdir.mk
## Description:   Rules for building subdirectories
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
## $Id: l4.subdir.mk,v 1.7 2003/09/24 19:05:51 skoglund Exp $
##                
######################################################################

include $(top_srcdir)/Mk/l4.build.mk

do-all:		subdirs-all
do-install:	subdirs-install
do-clean:	subdirs-clean


subdirs-all: $(MKFILE_DEPS)
	@for D in $(SUBDIRS); do \
	  (cd $$D && $(MAKE) all) || exit ; \
	done

subdirs-install: subdirs-all $(MKFILE_DEPS)
	@for D in $(SUBDIRS); do \
	  (cd $$D && $(MAKE) install) || exit ; \
	done

subdirs-clean: $(MKFILE_DEPS)
	@for D in $(SUBDIRS); do \
	  (cd $$D && $(MAKE) clean) || exit ; \
	done
