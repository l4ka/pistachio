// This file is part of the uSTL library, an STL implementation.
//
// Copyright (c) 2005 by Mike Sharov <msharov@users.sourceforge.net>
// This file is free software, distributed under the MIT License.

#include "stdtest.h"

void WriteCML (const cmemlink& l)
{
    cout.format ("memlink{%zu}: ", l.size());
    const char* pc = reinterpret_cast<const char*>(l.cdata());
    size_t nc = l.size();
    if (pc[nc - 1] == 0)
	-- nc;
    cout.write (l.cdata(), nc);
    cout << endl;
}

void TestML (void)
{
    char str[] = "abcdefghijklmnopqrstuvwzyz";
    memlink::const_pointer cstr = str;

    memlink a, b;
    a.static_link (str);
    if (a.begin() != str)
	cout << "begin() failed on memlink\n";
    a.link (VectorRange(str));
    if (a.begin() + 5 != &str[5])
	cout << "begin() + 5 failed on memlink\n";
    if (0 != memcmp (a.begin(), str, VectorSize(str)))
	cout << "memcmp failed on memlink\n";
    WriteCML (a);
    b.link (cstr, VectorSize(str));
    if (b.data() != cstr)
	cout << "begin() of const failed on cmemlink\n";
    if (b.cmemlink::begin() != cstr)
	cout << "begin() failed on cmemlink\n";
    WriteCML (b);
    if (!(a == b))
	cout << "operator== failed on cmemlink\n";
    b.resize (VectorSize(str) - 2);
    a = b;
    if (a.data() != b.data())
	cout << "begin() after assignment failed on cmemlink\n";
    a.relink (str, VectorSize(str) - 1);
    WriteCML (a);
    a.insert (a.begin() + 5, 9);
    a.fill (a.begin() + 5, "-", 1, 9);
    WriteCML (a);
    a.erase (a.begin() + 9, 7);
    a.fill (a.end() - 7, "=", 1, 7);
    WriteCML (a);
    a.fill (a.begin() + 5, "TEST", 4, 3); 
    WriteCML (a);
    copy_n (cstr, VectorSize(str) - 1, a.begin());
    WriteCML (a);
}

StdBvtMain (TestML)
