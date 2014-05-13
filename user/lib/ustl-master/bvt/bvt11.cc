// This file is part of the uSTL library, an STL implementation.
//
// Copyright (c) 2005 by Mike Sharov <msharov@users.sourceforge.net>
// This file is free software, distributed under the MIT License.

#include "stdtest.h"

void PrintVector (const int* first, const int* last)
{
    cout << "{";
    while (first < last)
	cout << ' ' << *first++;
    cout << " }" << endl;
}

void TestSetAndMultiset (void)
{
#if HAVE_CPP11
    set<int> v (set<int>({ 1, 8, 2, 3, 1, 1, 4, 6, 1, 3, 4 }));
    multiset<int> mv (multiset<int>({ 1, 8, 9, 2, 3, 4, 6, 1, 3, 4 }));
    v.emplace (9);
    mv.emplace (1);
    mv.emplace (1);
#else
    const int vv[] = { 1, 8, 9, 2, 3, 1, 1, 4, 6, 1, 3, 4 };
    set<int> v (VectorRange (vv));
    multiset<int> mv (VectorRange (vv));
#endif
    cout << "set:\t\t";
    PrintVector (v.begin(), v.end());
    cout << "erase(3):\t";
    v.erase (3);
    PrintVector (v.begin(), v.end());
    cout << "multiset:\t";
    PrintVector (mv.begin(), mv.end());
    cout << "count(1) = " << mv.count(1) << endl;
    cout << "find(4) = " << lower_bound (mv, 4) - mv.begin() << endl;
    cout << "find(5) = " << binary_search (mv, 5) << endl;
    cout << "erase(3):\t";
    mv.erase (3);
    PrintVector (mv.begin(), mv.end());
}

StdBvtMain (TestSetAndMultiset)
