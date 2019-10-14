// This file is part of the uSTL library, an STL implementation.
//
// Copyright (c) 2005 by Mike Sharov <msharov@users.sourceforge.net>
// This file is free software, distributed under the MIT License.

#include "stdtest.h"

void WriteCML (const memblock& l)
{
    cout.format ("memblock{%zu}: ", l.size());
    const char* pc = reinterpret_cast<const char*>(l.cdata());
    size_t nc = l.size();
    while (nc && pc[nc - 1] == 0)
	-- nc;
    cout.write (l.cdata(), nc);
    cout << endl;
}

void TestMB (void)
{
    char strTest[] = "abcdefghijklmnopqrstuvwxyz";
    const size_t strTestLen = strlen(strTest);
    const char* cstrTest = strTest;

    memblock a, b;
    a.link (strTest, strTestLen);
    if (a.begin() != strTest)
	cout << "begin() failed on memblock\n";
    if (a.begin() + 5 != &strTest[5])
	cout << "begin() + 5 failed on memblock\n";
    if (0 != memcmp (a.begin(), strTest, strTestLen))
	cout << "memcmp failed on memblock\n";
    WriteCML (a);
    b.link (cstrTest, strTestLen);
    if (b.data() != cstrTest)
	cout << "begin() of const failed on memblock\n";
    if (b.cmemlink::begin() != cstrTest)
	cout << "cmemlink::begin() failed on memblock\n";
    WriteCML (b);
    if (!(a == b))
	cout << "operator== failed on memblock\n";
    b.copy_link();
    if (b.data() == NULL || b.cdata() == cstrTest)
	cout << "copy_link failed on memblock\n";
    if (!(a == b))
	cout << "copy_link didn't copy\n";
    b.resize (strTestLen - 2);
    a = b;
    if (a.begin() == b.begin())
	cout << "Assignment does not copy a link\n";
    a.deallocate();
    a.assign (strTest, strTestLen);
    WriteCML (a);
    a.insert (a.begin() + 5, 9);
    a.fill (a.begin() + 5, "-", 1, 9);
    WriteCML (a);
    a.erase (a.begin() + 2, 7);
    a.fill (a.end() - 7, "=", 1, 7);
    WriteCML (a);
    a.fill (a.begin() + 5, "TEST", 4, 3); 
    WriteCML (a);

    a.resize (26 + 24);
    a.fill (a.begin() + 26, "-+=", 3, 24 / 3);
    WriteCML (a);
    a.resize (0);
    WriteCML (a);
    a.resize (strTestLen + strTestLen / 2);
    WriteCML (a);
    cout << "Capacity " << a.capacity();
    a.shrink_to_fit();
    cout << ", shrunk " << a.capacity() << endl;
}

StdBvtMain (TestMB)
