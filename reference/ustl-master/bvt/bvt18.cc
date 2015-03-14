// This file is part of the uSTL library, an STL implementation.
//
// Copyright (c) 2005 by Mike Sharov <msharov@users.sourceforge.net>
// This file is free software, distributed under the MIT License.

#include "stdtest.h"

template <size_t N, typename T>
void TestTuple (const char* ctrType)
{
    cout << "================================================" << endl;
    cout << "Testing " << ctrType << endl;
    cout << "================================================" << endl;
    assert (N <= 8);
#if HAVE_CPP11
    tuple<N,T> pt1 ({1,2,3,4,5,6,7,8});
    tuple<N,T> pt2;
    pt2 = {4,4,4,4};
    pt2 += {1,2,3,4};
#else
    T pt1v[12] = { 1, 2, 3, 4, 5, 6, 7, 8 };
    tuple<N,T> pt1 (pt1v);
    tuple<N,T> pt2 (&pt1v[4]);
#endif

    cout << "pt1:\t\t\tsize = " << pt1.size() << ", value = " << pt1 << endl;
    cout << "pt2:\t\t\t" << pt2 << endl;
    iota (pt2.begin(), pt2.end(), 10);
    cout << "pt2:\t\t\t" << pt2 << endl;

    pt1 *= 3;
    cout << "pt1 *= 3:\t\t" << pt1 << endl;
    pt1 /= 3;
    cout << "pt1 /= 3:\t\t" << pt1 << endl;
    pt1 += 3;
    cout << "pt1 += 3:\t\t" << pt1 << endl;
    pt1 -= 3;
    cout << "pt1 -= 3:\t\t" << pt1 << endl;

    pt1 *= pt2;
    cout << "pt1 *= pt2:\t\t" << pt1 << endl;
    pt1 /= pt2;
    cout << "pt1 /= pt2:\t\t" << pt1 << endl;
    pt1 += pt2;
    cout << "pt1 += pt2:\t\t" << pt1 << endl;
    pt1 -= pt2;
    cout << "pt1 -= pt2:\t\t" << pt1 << endl;

    pt1 = pt1 * pt2;
    cout << "pt1 = pt1 * pt2:\t" << pt1 << endl;
    pt1 = pt1 / pt2;
    cout << "pt1 = pt1 / pt2:\t" << pt1 << endl;
    pt1 = pt1 + pt2;
    cout << "pt1 = pt1 + pt2:\t" << pt1 << endl;
    pt1 = pt1 - pt2;
    cout << "pt1 = pt1 - pt2:\t" << pt1 << endl;
}

void TestIntegralTuples (void)
{
    TestTuple<4,float> ("tuple<4,float>");
    TestTuple<2,float> ("tuple<2,float>");
    TestTuple<4,int32_t> ("tuple<4,int32_t>");
    TestTuple<4,uint32_t> ("tuple<4,uint32_t>");
    TestTuple<2,int32_t> ("tuple<2,int32_t>");
    TestTuple<2,uint32_t> ("tuple<2,uint32_t>");
    TestTuple<4,int16_t> ("tuple<4,int16_t>");
    TestTuple<4,uint16_t> ("tuple<4,uint16_t>");
    TestTuple<8,int8_t> ("tuple<8,int8_t>");
    TestTuple<8,uint8_t> ("tuple<8,uint8_t>");

    cout << "================================================" << endl;
    cout << "Testing tuple<3,string>" << endl;
    cout << "================================================" << endl;
    tuple<3, string> strv;
    strv[0] = "str0";
    strv[1] = "str1";
    strv[2] = "str2";
    cout << "str: " << strv << endl;
}

StdBvtMain (TestIntegralTuples)
