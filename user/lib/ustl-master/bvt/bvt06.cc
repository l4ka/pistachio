// This file is part of the uSTL library, an STL implementation.
//
// Copyright (c) 2005 by Mike Sharov <msharov@users.sourceforge.net>
// This file is free software, distributed under the MIT License.

#include "stdtest.h"

void PrintBlock (const cmemlink& l)
{
    const int* numbers = reinterpret_cast<const int*>(l.begin());
    const size_t nNumbers = l.size() / sizeof(int);
    for (size_t i = 0; i < nNumbers; ++ i)
	cout << numbers[i] << ' ';
    cout << endl;
}

void TestObjectVector (void)
{
    vector<memblock> v;
    const size_t nNumbers = 1000;
    int numbers [nNumbers];
    const size_t nLinks = 10;
    cmemlink links [nLinks];
    for (size_t i = 0; i < nNumbers; ++ i)
	numbers[i] = i;
    uoff_t offset = 0;
    for (size_t l = 0; l < nLinks; ++ l) {
	links[l].link (numbers + offset, l * sizeof(int));
	offset += l;
	v.push_back (memblock(links[l]));
    }
    cout.format ("---\nvector<memblock> of %zu elements:\n---\n", v.size());
    for_each (v.begin(), v.end(), &PrintBlock);
    cout.format ("---\nsize() = %zu, max_size() = ", v.size());
    if (v.max_size() == SIZE_MAX / sizeof(memblock))
	cout << "SIZE_MAX/elsize";
    else
	cout << v.max_size();
    static const char tf[2][6]={"false","true"};
    cout.format (", empty() = %s\n", tf[v.empty()]);
    v.push_back (memblock(5));
    cout.format ("back()->size() = %zu\n", v.back().size());
    v.back().resize (40);
    cout.format ("back()->size() = %zu\n", v.back().size());
    v.pop_back();
    PrintBlock (v.back());
    vector<memblock> cache;
    cache.assign (v.begin(), v.end());
    v.clear();
    v.assign (cache.begin(), cache.end());
    v.erase (v.begin() + 5, 2);
    v.erase (v.end() - 1, 1);
    v.erase (v.end(), streamsize(0));
    cout.format ("---\nvector of %zu elements backwards:\n---\n", v.size());
    for_each (v.rbegin(), v.rend(), &PrintBlock);
    cout << "---\n";
}

StdBvtMain (TestObjectVector)
