
#  Validation Suite Makefile
#   
#  Copyright (C) 2000  Monta Vista Software Inc.
# 
#  Author : Gary S. Robertson
# 
#  This program is free software; you can redistribute it and/or
#  modify it under the terms of the GNU General Public License
#  as published by the Free Software Foundation; either version 2
#  of the License, or (at your option) any later version.
# 
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#============================================================================
#
#============================================================================
#
# FILE NAME :  validate - 
#              Wind River VxWorks (R) on Linux pthreads validation suite test
#
# VxWorks is a registered trademark of Wind River Systems, Inc.
#
#============================================================================

#----------------------------------------------------------------------------
# COMPILE macros
#----------------------------------------------------------------------------

.c.o:
	$(CC) $(CFLAGS) -c $*.c

CFLAGS	+= -Wall -I. -Itarget_h -I- -D_GNU_SOURCE -D_REENTRANT

#----------------------------------------------------------------------------
# Make the program...
#----------------------------------------------------------------------------
OBJS =  \
	lkernelLib.o ltaskLib.o lmsgQLib.o lsemLib.o lwdLib.o validate.o

LIB_SHORT = v2lin
LIB_FULL = lib$(LIB_SHORT).so

PROG = validate

all:	$(PROG)

$(LIB_FULL): $(OBJS) Makefile
	$(CC) -shared $(CFLAGS) $(OBJS) -o $(LIB_FULL) -lpthread

$(PROG):	$(LIB_FULL)
	$(CC) $(CFLAGS) -o $(PROG) -L. -l$(LIB_SHORT)

#----------------------------------------------------------------------------
# Compile modules w/ Inference rules
#----------------------------------------------------------------------------
clean:
	rm -f $(OBJS) $(PROG) $(LIB_FULL)

depend:
	makedepend -s "# DO NOT DELETE" -- *.c

# DO NOT DELETE THIS LINE -- make depend depends on it.


