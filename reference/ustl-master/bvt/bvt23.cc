// This file is part of the uSTL library, an STL implementation.
//
// Copyright (c) 2005 by Mike Sharov <msharov@users.sourceforge.net>
// This file is free software, distributed under the MIT License.

#include "stdtest.h"
using namespace ustl::simd;

template <typename Ctr>
void TestBitwiseOperations (Ctr op1, Ctr op2, const Ctr op3)
{
    passign (op3, op2);
    pand (op1, op2);
    cout << "pand(op1,op2) = " << op2 << endl;
    passign (op3, op2);
    por (op1, op2);
    cout << "por(op1,op2) = " << op2 << endl;
    passign (op3, op2);
    pxor (op1, op2);
    cout << "pxor(op1,op2) = " << op2 << endl;
    passign (op3, op2);
    pshl (op1, op2);
    cout << "pshl(op1,op2) = " << op2 << endl;
    passign (op3, op2);
    pshr (op1, op2);
    cout << "pshr(op1,op2) = " << op2 << endl;
}

template <> inline void TestBitwiseOperations (tuple<2,float>, tuple<2,float>, const tuple<2,float>) {}
template <> inline void TestBitwiseOperations (tuple<4,float>, tuple<4,float>, const tuple<4,float>) {}

template <typename Ctr>
void TestCtr (const char* ctrType)
{
    cout << "================================================" << endl;
    cout << "Testing " << ctrType << endl;
    cout << "================================================" << endl;
    Ctr op1, op2, op3;
    fill (op1, 2);
    iota (op2.begin(), op2.end(), 1);
    cout << "op1 = " << op1 << endl;
    cout << "op2 = " << op2 << endl;
    passign (op2, op3);
    cout << "passign(op2,op3) = " << op3 << endl;
    padd (op1, op2);
    cout << "padd(op1,op2) = " << op2 << endl;
    psub (op1, op2);
    cout << "psub(op1,op2) = " << op2 << endl;
    pmul (op1, op2);
    cout << "pmul(op1,op2) = " << op2 << endl;
    pdiv (op1, op2);
    cout << "pdiv(op1,op2) = " << op2 << endl;
    TestBitwiseOperations (op1, op2, op3);
    passign (op3, op2);
    reverse (op2);
    pmin (op3, op2);
    cout << "pmin(op3,op2) = " << op2 << endl;
    passign (op3, op2);
    reverse (op2);
    pmax (op3, op2);
    cout << "pmax(op3,op2) = " << op2 << endl;
    passign (op3, op2);
    reverse (op2);
    reset_mmx();
    pavg (op3, op2);
    cout << "pavg(op3,op2) = " << op2 << endl;
    reset_mmx();
}

template <typename SrcCtr, typename DstCtr, typename Operation>
void TestConversion (const char* ctrType)
{
    cout << "================================================" << endl;
    cout << "Testing " << ctrType << endl;
    cout << "================================================" << endl;
    SrcCtr src;
    DstCtr dst;
    typedef typename SrcCtr::value_type srcval_t;
    iota (src.begin(), src.end(), srcval_t(-1.4));
    pconvert (src, dst, Operation());
    cout << src << " -> " << dst << endl;
    iota (src.begin(), src.end(), srcval_t(-1.5));
    pconvert (src, dst, Operation());
    cout << src << " -> " << dst << endl;
    iota (src.begin(), src.end(), srcval_t(-1.7));
    pconvert (src, dst, Operation());
    cout << src << " -> " << dst << endl;
}

void TestSimdAlgorithms (void)
{
    TestCtr<tuple<8,uint8_t> >("uint8_t[8]");
    TestCtr<tuple<8,int8_t> >("int8_t[8]");
    TestCtr<tuple<4,uint16_t> >("uint16_t[4]");
    TestCtr<tuple<4,int16_t> >("int16_t[4]");
    TestCtr<tuple<2,uint32_t> >("uint32_t[2]");
    TestCtr<tuple<2,int32_t> >("int32_t[2]");
    #if HAVE_INT64_T
	TestCtr<tuple<1,uint64_t> >("uint64_t[1]");
	TestCtr<tuple<1,int64_t> >("int64_t[1]");
    #else
	cout << "No 64bit types available on this platform" << endl;
    #endif
    TestCtr<tuple<2,float> >("float[2]");
    TestCtr<tuple<4,float> >("float[4]");
    TestCtr<tuple<7,uint32_t> >("uint32_t[7]");

    #if HAVE_MATH_H
	#define CVT_TEST(size,src,dest,op) \
	TestConversion<tuple<size,src>, tuple<size,dest>, op<src,dest> > (#op " " #src " -> " #dest)
	CVT_TEST(4,int32_t,float,fround);
	CVT_TEST(4,int32_t,double,fround);
	CVT_TEST(4,float,int32_t,fround);
	CVT_TEST(4,double,int32_t,fround);
	CVT_TEST(4,float,int32_t,fcast);
    #else
	cout << "CAN'T TEST: math.h functions are not available on this platform." << endl;
    #endif
}

StdBvtMain (TestSimdAlgorithms)
