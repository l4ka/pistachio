// "Testing string reads" 12345678 4321 0x78675645 1.234567890123456
// (the above line is the input to this test, so must be at the beginning)
//
// This file is part of the uSTL library, an STL implementation.
//
// Copyright (c) 2005 by Mike Sharov <msharov@users.sourceforge.net>
// This file is free software, distributed under the MIT License.

#include "stdtest.h"
#include <stdio.h>

void TestCoutCinCerr (void)
{
    string testStr;
    cin >> testStr;
    if (testStr != "//") {
	cout.format ("You must put bvt13.cc on stdin (read \"%s\")\n", testStr.c_str());
	return;
    }
    uint32_t n1 = 0, n3 = 0;
    uint16_t n2 = 0;
    double f1 = 0.0;
    cin >> testStr >> n1 >> n2 >> n3 >> f1;
    cout << testStr << endl;
    cout << "A string printed to stdout\n";
    cout.format ("%d %s: %d, %hd, 0x%08X, %1.15f\n", 4, "numbers", n1, n2, n3, f1);
    string testString;
    testString.format ("A ustl::string object printed %d times\n", 3);
    for (int i = 0; i < 3; ++ i)
	cout << testString;
    cout.flush();
    fprintf (stderr, "All ");
    cerr << "done.\n";
}

StdBvtMain (TestCoutCinCerr)
