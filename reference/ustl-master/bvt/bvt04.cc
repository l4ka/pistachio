// This file is part of the uSTL library, an STL implementation.
//
// Copyright (c) 2005 by Mike Sharov <msharov@users.sourceforge.net>
// This file is free software, distributed under the MIT License.

#include "stdtest.h"

class A {
public:
    A (void)
    { cout << "A::A\n"; }
    A (int a)
    { cout << "A::A(" << a << ")\n"; }
    A (const A&)
    { cout << "Copy A::A\n"; }
    const A& operator= (const A&)
    { cout << "A::operator=\n"; return (*this); }
    ~A (void)
    { cout << "A::~A\n"; }
};

void TestVector (void)
{
    static const int c_TestNumbers[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 13, 14, 15, 16, 17, 18 };
    vector<int> v;
    v.push_back (1);
    cout << v << endl;
    v.reserve (20);
    cout.format ("Reserved to capacity() == %zu (%zu used, ", v.capacity(), v.size());
    if (v.max_size() == SIZE_MAX / sizeof(int))
	cout << "SIZE_MAX/elsize";
    else
	cout << v.max_size();
    cout << " max)\n";
    v.insert (v.begin() + 1, 1 + VectorRange(c_TestNumbers));
    cout << v << endl;
    cout.format ("front() = %d, back() = %d\n", v.front(), v.back());
    v.erase (v.begin());
    v.pop_back();
    cout << v << endl;
    v.insert (v.begin() + 10, 3, 666);
    v.at(5) = 777;
    cout << v << endl;
    v.resize (v.size() - 5);
    if (v.empty())
	cout << "v is now empty\n";
    cout << v << endl;
    cout.format ("v[5] == %d\n", v[5]);
    v.clear();
    if (v.empty())
	cout << "v is now empty\n";
    vector<int> v2 (20, 66);
    cout << v2 << endl;
    v2.assign (20, 33);
    cout << v2 << endl;
    v.assign (VectorRange (c_TestNumbers));
    cout << v << endl;
    if (v == v2)
	cout << "v == v2\n";
    v2 = v;
    if (v == v2)
	cout << "v == v2\n";
    vector<A> ctv;
    A a;
    ctv.assign (3, a);
    ctv.pop_back();
    cout << "Class insertion testing successful\n";
#if HAVE_CPP11
    v = vector<int>(vector<int>({1,2,3,4,5,6,7,8}));
    v.insert (v.begin()+3, {11,12,13});
    v.emplace (v.begin()+7, 22);
    cout << '{';
    for (auto i : v)
	cout << ' ' << i;
    cout << " }" << endl;
    ctv.emplace_back (15);
#else
    cout << "{ 1 2 3 11 12 13 4 22 5 6 7 8 }\nA::A(15)\nA::~A\n";
#endif
}

StdBvtMain (TestVector)
