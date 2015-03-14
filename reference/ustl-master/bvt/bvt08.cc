// This file is part of the uSTL library, an STL implementation.
//
// Copyright (c) 2005 by Mike Sharov <msharov@users.sourceforge.net>
// This file is free software, distributed under the MIT License.

#include "stdtest.h"

inline void PrintString (const string& str)
{
    cout << str << endl;
}

void TestStringVector (void)
{
    vector<string> v;

    vector<string>::iterator bogusi = find (v, string("bogus"));
    if (bogusi != v.end())
	cout << "bogus found at position " << bogusi - v.begin() << endl;

    v.push_back (string("Hello world!"));
    v.push_back (string("Hello again!"));
    v.push_back (string("element3"));
    v.push_back (string("element4"));
    v.push_back (string("element5_long_element5"));
    for_each (v, &PrintString);

    if (!(v[2] == string("element3")))
	cout << "operator== failed" << endl;
    vector<string>::iterator el3i = find (v, string("element3"));
    if (el3i != v.end())
	cout << *el3i << " found at position " << el3i - v.begin() << endl;
    bogusi = find (v, string("bogus"));
    if (bogusi != v.end())
	cout << *bogusi << " found at position " << bogusi - v.begin()<< endl;

    vector<string> v2;
    v2 = v;
    v = v2;
    v.erase (v.end(), v.end());
    cout << "After erase (end,end):" << endl;
    for_each (v, &PrintString);
    v = v2;
    v.erase (v.begin() + 2, 2);
    cout << "After erase (2,2):" << endl;
    for_each (v, &PrintString);
    v = v2;
    v.pop_back();
    cout << "After pop_back():" << endl;
    for_each (v, &PrintString);
    v = v2;
    v.insert (v.begin() + 1, v2.begin() + 1, v2.begin() + 1 + 3);
    cout << "After insert(1,1,3):" << endl;
    for_each (v, &PrintString);
}

StdBvtMain (TestStringVector)
