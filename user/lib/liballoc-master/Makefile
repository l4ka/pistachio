# Please see LICENSE for licensing information.


# --------- FLAGS AND VARIABLES --------------------

CFLAGS=-O2 -nostdlib -nodefaultlibs -fno-builtin -fPIC -Wall 
HEADERPATH=-I./



# ---------  GENERIC MAKE RULES --------------------

all: 
	@echo "Makefile for the liballoc library."
	@echo "Please see LICENSE for licensing information."
	@echo 
	@echo "Output should be: liballoc.a "
	@echo "                  liballoc.so"
	@echo 
	@echo "Usage: make [ compile | clean | <platform> ] "
	@echo 
	@echo "Currently supported platforms:"
	@echo 
	@echo "      linux"
	@echo "      linux_debug"
	@echo
	@echo
	@echo "Please see the README for example usage"


clean:
	rm -f ./*.o
	rm -f ./*.a
	rm -f ./*.so

compile:
	gcc $(HEADERPATH) $(CFLAGS) -static -c liballoc.c
	ar -rcv liballoc.a  *.o
	gcc $(HEADERPATH) $(CFLAGS) -shared liballoc.c -o liballoc.so


linux:
	gcc $(HEADERPATH) $(CFLAGS) -static -c liballoc.c linux.c
	ar -rcv liballoc.a  *.o
	gcc $(HEADERPATH) $(CFLAGS) -shared liballoc.c linux.c -o liballoc.so


linux_debug:
	gcc -DDEBUG $(HEADERPATH) $(CFLAGS) -static -c liballoc.c linux.c
	ar -rcv liballoc.a  *.o
	gcc -DDEBUG $(HEADERPATH) $(CFLAGS) -shared liballoc.c linux.c -o liballoc.so



