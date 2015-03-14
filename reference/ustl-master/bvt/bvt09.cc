// This file is part of the uSTL library, an STL implementation.
//
// Copyright (c) 2005 by Mike Sharov <msharov@users.sourceforge.net>
// This file is free software, distributed under the MIT License.

#include "stdtest.h"

void TestStringStreams (void)
{
    const unsigned char magic_Char = 'c';
    const unsigned short magic_Short = 1234;
    const long magic_Int = -12345678;
    const unsigned long magic_UInt = 12345678;
    const float magic_Float = 123.45678;
    const double magic_Double = 123456789123456.789;
    const bool magic_Bool = true;

    char c = magic_Char;
    unsigned char uc = magic_Char;
    int i = magic_Int;
    short si = magic_Short;
    long li = magic_Int;
    unsigned int ui = magic_UInt;
    unsigned short usi = magic_Short;
    unsigned long uli = magic_UInt;
    float f = magic_Float;
    double d = magic_Double;
    bool bv = magic_Bool;

    ostringstream os;
    os << c << endl;
    os << uc << endl;
    os << bv << endl;
    os << i << endl;
    os << ui << endl;
    os << li << endl;
    os << uli << endl;
    os << f << endl;
    os << d << endl;
    os << si << endl;
    os << usi << endl << ends; 
    os.flush();
    cout << os.pos() << " bytes written" << endl;

    c = 0;
    uc = 0;
    bv = false;
    i = ui = li = uli = 0;
    f = 0; d = 0;
    si = usi = 0;

    istringstream is (os.str());
    is >> c;
    is >> uc;
    is >> bv;
    is >> i;
    is >> ui;
    is >> li;
    is >> uli;
    is >> f;
    is >> d;
    is >> si;
    is >> usi;

    cout << "Values:" << endl;
    cout.format ("char:    '%c'\n", static_cast<int>(c));
    cout.format ("u_char:  '%c'\n", static_cast<int>(uc));
    cout.format ("bool:    %d\n", static_cast<int>(bv));
    cout.format ("int:     %d\n", i);
    cout.format ("u_int:   %d\n", ui);
    cout.format ("long:    %ld\n", li);
    cout.format ("u_long:  %ld\n", uli);
    cout.format ("float:   %.2f\n", f);
    cout.format ("double:  %.2f\n", d);
    cout.format ("short:   %d\n", static_cast<int>(si));
    cout.format ("u_short: %d\n", static_cast<int>(usi));
    cout << endl;

    cout << "Dump:" << endl;
    cout << os.str().cdata() << endl;
    cout << endl;
}

StdBvtMain (TestStringStreams)
