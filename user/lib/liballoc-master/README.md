liballoc - a small memory allocator
===================================

This is liballoc, a memory allocator for hobby operating systems, originally
written by Durand. According to the original page for liballoc it was released
into the public domain, but the copy I have contains the 3 clause BSD license.

liballoc.c/h are the original release of liballoc taken from the spoon tarball
while liballoc_1_1.c/h are later versions found by detective work using Google.

Using liballoc
==============

There are 4 functions which you need to implement on your system:

	int   liballoc_lock();
	int   liballoc_unlock();
	void* liballoc_alloc(int);
	int   liballoc_free(void*,int);

1) Have a look at liballoc.h for information about what each function is 
supposed to do.


2) Have a look at linux.c for an example of how to implement the library 
on linux. 


NOTE: There are two ways to build the library:

    1) Compile the library with a new system file. For example, I've
	   left linux.c with the default distribution. It gets compiled
	   directly into the liballoc_linux.so file.

	2) Implement the functions in your application and then just
	   link against the default liballoc.so library when you compile
	   your app.


Quick Start
===========

You can simply type: "make linux" to build the linux shared
library.  Thereafter, you can link it directly into your applications
during build or afterwards by export the LD_PRELOAD environment
variable. 


To run bash with the library, for example:

    LD_PRELOAD=/full/path/to/liballoc.so bash


The above command will pre-link the library into the application,
essentially replacing the default malloc/free calls at runtime. It's
quite cool.


Originally by:
Durand Miller






