// This file is part of the uSTL library, an STL implementation.
//
// Copyright (c) 2005 by Mike Sharov <msharov@users.sourceforge.net>
// This file is free software, distributed under the MIT License.

#include "stdtest.h"
using namespace tm;

void Print (int v)
    { cout.format ("PrintInt %d\n", v); }
void Print (short v)
    { cout.format ("PrintShort %d\n", v); }
void Print (float v)
    { cout.format ("PrintFloat %.2f\n", v); }

class Base { };
class Derived : public Base { };

void TestTypelists (void)
{
    cout << "----------------------------------------------------------------------\n"
	    " Testing functionality from typet.h\n"
	    "----------------------------------------------------------------------\n";
    NullType nullType;
    cout.format ("sizeof(NullType) = %zu\n", sizeof(nullType));
    cout.format ("Int2Type(42)::value = %d\n", Int2Type<42>::value);
    Type2Type<int>::OriginalType t2tiv = 56;
    cout.format ("Type2Type type value = %d\n", t2tiv);

    cout << "int == int is " << bool(IsSameType<int, int>::value) << endl;
    cout << "float == int is " << bool(IsSameType<float, int>::value) << endl;

    Print (Select<Conversion<long,int>::exists2Way, int, float>::Result(567));
    cout << "Base is SuperSubclass from Derived is " << bool(SuperSubclass<Base,Derived>::value) << endl;
    cout << "Base is SuperSubclass from Base is " << bool(SuperSubclass<Base,Base>::value) << endl;
    cout << "Base is SuperSubclassStrict from Derived is " << bool(SuperSubclassStrict<Base,Derived>::value) << endl;
    cout << "Base is SuperSubclassStrict from Base is " << bool(SuperSubclassStrict<Base,Base>::value) << endl;

    cout << "\n----------------------------------------------------------------------\n"
	    " Testing functionality from typelist.h\n"
	    "----------------------------------------------------------------------\n";
    typedef tl::Seq<char, int, short, long>::Type IntTypesList;
    typedef tl::Seq<float, double>::Type FloatTypesList;
    cout.format ("Length of IntTypesList is %d\n", tl::Length<IntTypesList>::value);
    Print (tl::TypeAt<IntTypesList, 2>::Result(1234));
    Print (tl::TypeAtNonStrict<FloatTypesList, 0, int>::Result(1235));
    Print (tl::TypeAtNonStrict<FloatTypesList, 2, short>::Result(1236));
    typedef tl::Append<IntTypesList, FloatTypesList>::Result AllTypesList;
    cout.format ("Index of double in AllTypesList is %d\n", tl::IndexOf<AllTypesList,double>::value);
    typedef tl::Erase<AllTypesList, float>::Result NoFloatList;
    cout.format ("Index of float in NoFloatList is %d\n", tl::IndexOf<NoFloatList,float>::value);
    cout.format ("Index of double in NoFloatList is %d\n", tl::IndexOf<NoFloatList,double>::value);
    typedef tl::Reverse<AllTypesList>::Result ReversedList;
    cout.format ("Index of double in ReversedList is %d\n", tl::IndexOf<ReversedList,double>::value);
}

StdBvtMain (TestTypelists)
