// This file is part of the uSTL library, an STL implementation.
//
// Copyright (c) 2005 by Mike Sharov <msharov@users.sourceforge.net>
// This file is free software, distributed under the MIT License.

#include "stdtest.h"

template <typename T>
void TestBswap (T v)
{
    const T vsw (bswap(v));
#if BYTE_ORDER == LITTLE_ENDIAN
    const T vbe (vsw), vle (v);
#elif BYTE_ORDER == BIG_ENDIAN
    const T vbe (v), vle (vsw);
#endif
    static const char ok[2][4] = { "bad", "ok" };
    cout << "bswap(" << v << ") = " << vsw << endl;
    cout << "le_to_native(" << v << ") = " << ok[le_to_native(vle) == v] << endl;
    cout << "native_to_le(" << v << ") = " << ok[native_to_le(v) == vle] << endl;
    cout << "be_to_native(" << v << ") = " << ok[be_to_native(vbe) == v] << endl;
    cout << "native_to_be(" << v << ") = " << ok[native_to_be(v) == vbe] << endl;
}

void TestUtility (void)
{
    cout << "DivRU(13,5) = " << DivRU(13,5) << endl;
    cout << "DivRU(15,5) = " << DivRU(15,5) << endl;
    cout << "DivRU(-12,5) = " << DivRU(-12,5) << endl;
    cout << endl;
    cout << "Align(5) = " << Align(5) << endl;
    cout << "Align(5,2) = " << Align(5,2) << endl;
    cout << "Align(17,7) = " << Align(17,7) << endl;
    cout << "Align(14,7) = " << Align(14,7) << endl;
    cout << endl;
    cout << "NextPow2(0) = " << NextPow2(0) << endl;
    cout << "NextPow2(1) = " << NextPow2(1) << endl;
    cout << "NextPow2(4) = " << NextPow2(4) << endl;
    cout << "NextPow2(3827) = " << NextPow2(3827) << endl;
    cout << "NextPow2(0xFFFFFFF0) = " << NextPow2(0xFFFFFFF0) << endl;
    cout << endl;
    cout << "advance(42,0) = " << advance(42,0) << endl;
    cout << "advance(42,3) = " << advance(42,3) << endl;
    const void *cvp = (const void*) 0x1234;
    void* vp = (void*) 0x4321;
    cout << ios::hex;
    cout << "cvp = " << cvp << endl;
    cout << "vp = " << vp << endl;
    cout << "advance(cvp,5) = " << advance(cvp,5) << endl;
    cout << "advance(vp,4) = " << advance(vp,4) << endl;
    cout << "distance(cvp,vp) = " << distance(cvp,vp) << endl;
    cout << "abs_distance(vp,cvp) = " << abs_distance(vp,cvp) << endl;
    cout << ios::dec << endl;
    const int32_t c_Numbers[] = { 1, 2, 3, 4, 5 };
    const int32_t c_Empty[] = { };
    cout << "size_of_elements(3, c_Numbers) = " << size_of_elements(3, c_Numbers) << endl;
    cout << "VectorSize(c_Numbers[5]) = " << VectorSize(c_Numbers) << endl;
    cout << "VectorSize(c_Numbers[0]) = " << VectorSize(c_Empty) << endl;
    cout << endl;
    cout << "BitsInType(uint32_t) = " << BitsInType(uint32_t) << endl;
    cout << "BitsInType(int16_t) = " << BitsInType(int16_t) << endl;
    cout << "BitsInType(char) = " << BitsInType(char) << endl;
    cout << ios::hex << endl;
    cout << "BitMask(uint32_t,12) = " << BitMask(uint32_t,12) << endl;
    cout << "BitMask(uint16_t,1) = " << BitMask(uint16_t,1) << endl;
    cout << "BitMask(uint8_t,8) = " << BitMask(uint8_t,8) << endl;
    cout << "BitMask(uint16_t,0) = " << BitMask(uint16_t,0) << endl;
    cout << endl;
    uint16_t packed16 = 0xCDCD;
    pack_type (uint8_t(0x42), packed16);
    cout << "pack_type(uint8_t, uint16_t) = " << packed16 << endl;
    uint32_t packed32 = 0xCDCDCDCD;
    pack_type (uint8_t(0x42), packed32);
    cout << "pack_type(uint8_t, uint32_t) = " << packed32 << endl;
    packed32 = 0xCDCDCDCD;
    pack_type (uint16_t(0x4243), packed32);
    cout << "pack_type(uint16_t, uint32_t) = " << packed32 << endl;
    #if HAVE_INT64_T
	uint64_t packed64 = UINT64_C(0x123456789ABCDEF0);
	pack_type (uint8_t(0x42), packed64);
	cout << "pack_type(uint8_t, uint64_t) = " << packed64 << endl;
	packed64 = UINT64_C(0x123456789ABCDEF0);
	pack_type (uint32_t(0x42434445), packed64);
	cout << "pack_type(uint32_t, uint64_t) = " << packed64 << endl;
    #else
	cout << "No 64bit types available on this platform" << endl;
    #endif
    cout << endl;
    TestBswap (uint16_t (0x1234));
    TestBswap (uint32_t (0x12345678));
    #if HAVE_INT64_T
	TestBswap (uint64_t (UINT64_C(0x123456789ABCDEF0)));
    #else
	cout << "No 64bit types available on this platform" << endl;
    #endif
    cout << ios::dec << endl;
    cout << "absv(12) = " << absv(12) << endl;
    cout << "absv(-12) = " << absv(-12) << endl;
    cout << "sign(12) = " << sign(12) << endl;
    cout << "sign(-12) = " << sign(-12) << endl;
    cout << "sign(0) = " << sign(0) << endl;
    cout << "min(3,4) = " << min(3,4) << endl;
    cout << "min(6U,1U) = " << min(6U,1U) << endl;
    cout << "max(-3,-6) = " << max(-3,-6) << endl;
    cout << "max(-3L,6L) = " << max(-3L,6L) << endl;
}

StdBvtMain (TestUtility)
