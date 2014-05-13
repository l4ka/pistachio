// 011010011001011001011000100100
// This file is part of the uSTL library, an STL implementation.
//
// Copyright (c) 2005 by Mike Sharov <msharov@users.sourceforge.net>
// This file is free software, distributed under the MIT License.

#include "stdtest.h"

size_t SizeOfSet (const bitset<30>& v)
{
    return (stream_size_of (v));
}

void TestBitset (void)
{
    bitset<30> bs1;
    cout.format ("bitset<%zu> bs1: capacity() = %zu, sizeof() = %zu\n", bs1.size(), bs1.capacity(), sizeof(bs1));
    cout << bs1 << endl;
    bs1.set();
    bs1.set (6, false);
    cout << bs1 << endl;
    bs1.flip();
    cout << bs1 << endl;
    bs1.flip();
    cout << bs1 << endl;

    bs1.reset();
    string comment;	// See line 0 in this file
    cin >> comment >> bs1;
    cout << bs1 << endl;
    cout.format ("count = %zu\n", bs1.count());

    bs1.reset();
    cout << bs1;
    static const char tf[2][6] = { "false", "true" };
    cout.format ("\nany = %s, none = %s, count = %zu\n", tf[bs1.any()], tf[bs1.none()], bs1.count());
    bs1.flip();
    cout << bs1;
    cout.format ("\nany = %s, none = %s, count = %zu\n", tf[bs1.any()], tf[bs1.none()], bs1.count());
    bs1.reset();
    bs1.set (4);
    bs1.set (7);
    bs1.set (8);
    cout << bs1;
    cout.format ("\ntest(7) == %s, [9] = %s, [8] = %s", tf[bs1.test(7)], tf[bs1[9]], tf[bs1[8]]);
    cout.format ("\nany = %s, none = %s, count = %zu\n", tf[bs1.any()], tf[bs1.none()], bs1.count());
    cout << "~bs1 == " << ~bs1;
    cout.format ("\nto_value == 0x%X\n", bs1.to_value());

    bitset<70> bs2 ("0101101");
    cout.format ("bitset<%zu> bs2: capacity() = %zu, sizeof() = %zu\n", bs2.size(), bs2.capacity(), sizeof(bs2));
    cout << bs2;
    bs2.set (34, 40, 13);
    cout << "\nbs2.set(34,40,13)\n";
    cout << bs2;
    cout.format ("\nbs2.at(34,40) = %u\n", bs2.at(34,40));

    bitset<256> bs3 (0x3030);
    cout.format ("bitset<%zu> bs3: capacity() = %zu, sizeof() = %zu\n", bs3.size(), bs3.capacity(), sizeof(bs3));
    cout.format ("bs3.to_value() == 0x%X\n", bs3.to_value());

    bitset<30> bs4 (bs1);
    if (bs1 == bs4)
	cout << "bs4 == bs1\n";

    bs4 = 0x50505050;
    cout << "bs4 = 0x50505050: " << bs4;
    bs1 = 0x30303030;
    cout << "\nbs1 = 0x30303030: " << bs1;
    bs4 &= bs1;
    cout << "\nbs4 &= bs1; bs4 = " << bs4;
    bs4 = 0x50505050;
    bs4 &= bs1;
    cout << "\nbs4 & bs1;  bs4 = " << bs4;
    bs4 = 0x50505050;
    bs4 |= bs1;
    cout << "\nbs4 |= bs1; bs4 = " << bs4;
    bs4 = 0x50505050;
    bs4 = bs4 | bs1;
    cout << "\nbs4 | bs1;  bs4 = " << bs4;
    bs4 = 0x50505050;
    bs4 ^= bs1;
    cout << "\nbs4 ^= bs1; bs4 = " << bs4;
    bs4 = 0x50505050;
    bs4 = bs4 ^ 0x30303030;
    cout << "\nbs4 ^ bs1;  bs4 = " << bs4;

    memblock b (stream_size_of (bs4));
    ostream os (b);
    os << bs4;
    istream is (b);
    bs4 = 0;
    is >> bs4;
    cout.format ("\nstream[%zu];  bs4 = ", b.size());
    cout << bs4 << endl;
}

StdBvtMain (TestBitset)
