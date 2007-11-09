######################################################################
#                
# Copyright (C) 1999, 2000, 2001,  Karlsruhe University
#                
# File path:     config/Makefile
# Description:   Makefile for pistachio configurator
#                
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
# OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
# OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
# SUCH DAMAGE.
#                
# $Id: Makefile,v 1.13 2006/06/22 14:08:23 ud3 Exp $
#                
######################################################################

CMLPATH=	$(SRCDIR)/../contrib/cml2
CMLCOMPILE=	$(CMLPATH)/cmlcompile.py
CMLCONFIG=	$(CMLPATH)/cmlconfigure.py
CMLCONFIGTRANS=	$(CMLPATH)/configtrans.py

CML_INPUT=	$(addprefix $(SRCDIR)/config/, rules.cml \
		  x86.cml	\
		  powerpc.cml	\
		  powerpc64.cml	\
		 )

CMLBATCH_PARAMS ?=
CMLCONFIG_OPTIONS = -o config.out -i config.out \
  $(addprefix -D,$(CMLBATCH_PARAMS)) rules.out

rules.out: $(CML_INPUT)
	@$(CMLCOMPILE) -o $@ $<

menuconfig: CMLMODE=-c
batchconfig: CMLMODE=-b
ttyconfig: CMLMODE=-t
xconfig: CMLMODE=-x

# Always invoke the configuration tool, but only rebuild targets down the
# dependency chain when the config.out changed.
.PHONY: always
config.out: rules.out always
	@$(CMLCONFIG) $(CMLMODE) $(CMLCONFIG_OPTIONS)

config.h .config: config.out
	@$(CMLCONFIGTRANS) -h $(@D)/config.h -s $(@D)/.config $<

$(BUILDDIR)/Makeconf.local: .config
	@echo "Rewriting $(@F) ..."
	@awk -F'[_=]' '/^CONFIG_ARCH_[^_]*=y/ { ARCH=$$3 }\
		       /^CONFIG_SUBARCH_[^_]*=y/ { SUBARCH=$$3 }\
		       /^CONFIG_CPU_[^_]*_[^_]*=y/ { CPU=$$4 }\
		       /^CONFIG_PLAT_[^_]*=y/ { PLATFORM=$$3 } END\
		       { { printf(",s/^ARCH *=.*/ARCH=%s/\n,s/^SUBARCH *=.*/SUBARCH=%s/\n,s/^CPU *=.*/CPU=%s/\n,s/^PLATFORM *=.*/PLATFORM=%s/\nwq\n",\
			 tolower(ARCH), tolower(SUBARCH), tolower(CPU), tolower(PLATFORM)) }}' $< | ed -s $@

menuconfig batchconfig ttyconfig xconfig: $(BUILDDIR)/Makeconf.local

