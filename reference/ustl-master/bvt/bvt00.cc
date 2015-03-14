// This file is part of the uSTL library, an STL implementation.
//
// Copyright (c) 2005 by Mike Sharov <msharov@users.sourceforge.net>
// This file is free software, distributed under the MIT License.

#include "stdtest.h"

void WriteCML (const cmemlink& l)
{
    cout.format ("cmemlink{%zu}: ", l.size());
    const void* pv = l.cdata();
    const char* pc = reinterpret_cast<const char*>(pv);
    size_t nc = l.size();
    if (pc[nc - 1] == 0)
	-- nc;
    cout.write (l.begin(), nc);
    cout << endl;
}

void TestCML (void)
{
    const char hello[] = "Hello world!";
    const char* phello = hello; // const storage is sometimes copied on pointing

    cmemlink a, b;
    a.link (phello, VectorSize(hello));
    if (a.begin() != phello)
	cout.format ("a.begin() failed: %p != %p\n", a.begin(), phello);
    a.link (VectorRange (hello));
    if (*(const char*)(a.begin() + 5) != hello[5])
	cout.format ("begin()[5] failed: %c != %c\n", *(const char*)(a.begin() + 5), VectorElement(hello,5));
    if (a.size() != VectorSize(hello))
	cout << "link to VectorRange doesn't work\n";
    if (0 != memcmp (a.begin(), hello, VectorSize(hello)))
	cout << "memcmp failed on cmemlink\n";
    b.static_link (hello);
    WriteCML (a);
    WriteCML (b);
    if (!(a == b))
	cout << "operator== failed on cmemlink\n";
    b.resize (VectorSize(hello) - 5);
    a = b;
    WriteCML (a);
}

StdBvtMain (TestCML)
