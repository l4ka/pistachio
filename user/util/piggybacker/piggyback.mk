######################################################################
##                
## Copyright (C) 2003,  Karlsruhe University
##                
## File path:     piggyback.mk
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
## $Id: piggyback.mk,v 1.7 2003/10/27 00:37:34 cvansch Exp $
##                
######################################################################

include Makeconf.local

OBJCOPY=	$(TOOLPREFIX)objcopy
STRIP=		$(TOOLPREFIX)strip

SIGMA0?=	$(top_builddir)/serv/sigma0/sigma0
ROOT_TASK?=	$(top_builddir)/apps/bench/pingpong/pingpong
KERNEL?=	$(kerneldir)/$(ARCH)-kernel

OBJS+=		sigma0.bin root_task.bin kernel.bin

BINARYARCH=	$(ARCH)
ifeq ($(ARCH), powerpc64)
BINARYARCH=	powerpc
endif

sigma0.mod: $(SIGMA0)
	@$(ECHO_MSG) `echo $(srcdir)/$@ | sed s,^$(top_srcdir)/,,`
	$(STRIP) --strip-all --preserve-dates $? -o $@

root_task.mod: $(ROOT_TASK)
	@$(ECHO_MSG) `echo $(srcdir)/$@ | sed s,^$(top_srcdir)/,,`
	$(STRIP) --strip-all --preserve-dates $? -o $@

kernel.mod: $(KERNEL)
	@$(ECHO_MSG) `echo $(srcdir)/$@ | sed s,^$(top_srcdir)/,,`
	$(STRIP) --strip-all --preserve-dates $? -o $@

do-clean:
	rm -f *.mod *.bin

.mod.bin:
	@$(ECHO_MSG) `echo $(srcdir)/$@ | sed s,^$(top_srcdir)/,,`
	$(OBJCOPY) --input-target=binary --output-target=default \
		--binary-architecture=$(BINARYARCH) $< $@

Makeconf.local:
	@echo "Generating a Makeconf.local in your build directory."
	@echo > Makeconf.local
	@echo 'SIGMA0=		$$(top_builddir)/serv/sigma0/sigma0'\
		>> Makeconf.local
#	@echo 'ROOT_TASK=	$$(top_builddir)/apps/bench/pingpong/pingpong'\
		>> Makeconf.local
	@echo 'ROOT_TASK=	$$(top_builddir)/apps/l4test/l4test'\
		>> Makeconf.local
	@echo 'KERNEL=		$$(kerneldir)/$$(ARCH)-kernel'\
		>> Makeconf.local


