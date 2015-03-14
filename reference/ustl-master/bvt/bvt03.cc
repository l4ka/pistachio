// This file is part of the uSTL library, an STL implementation.
//
// Copyright (c) 2005 by Mike Sharov <msharov@users.sourceforge.net>
// This file is free software, distributed under the MIT License.

#include "stdtest.h"
#include <unistd.h>

void TestStreams (void)
{
    const uint8_t magic_Char = 0x12;
    const uint16_t magic_Short = 0x1234;
    const uint32_t magic_Int = 0x12345678;
    const float magic_Float = 0.12345678;
    const double magic_Double = 0.123456789123456789;
    const bool magic_Bool = true;

    char c = magic_Char;
    unsigned char uc = magic_Char;
    int i = magic_Int;
    short si = magic_Short;
    long li = magic_Int;
    unsigned int ui = magic_Int;
    unsigned short usi = magic_Short;
    unsigned long uli = magic_Int;
    float f = magic_Float;
    double d = magic_Double;
    bool bv = magic_Bool;

    size_t totalSize = stream_size_of(c);
    totalSize += stream_size_of(uc);
    totalSize = Align (totalSize, stream_align_of(bv));
    totalSize += stream_size_of(bv);
    totalSize = Align (totalSize, stream_align_of(i));
    totalSize += stream_size_of(i);
    totalSize += stream_size_of(ui);
    totalSize = Align (totalSize);
    totalSize += stream_size_of(li);
    totalSize += stream_size_of(uli);
    totalSize = Align (totalSize, stream_align_of(f));
    totalSize += stream_size_of(f);
    totalSize = Align (totalSize, stream_align_of(d));
    totalSize += stream_size_of(d);
    totalSize += stream_size_of(si);
    totalSize += stream_size_of(usi);

    memblock b;
    b.resize (totalSize);
    b.fill (b.begin(), "\xCD", 1, b.size());
    ostream os (b);

    os << c;
    os << uc;
    os << ios::talign<bool>() << bv;
    os << ios::talign<int>() << i;
    os << ui;
    os << ios::align() << li;
    os << uli;
    os << ios::talign<float>() << f;
    os << ios::talign<double>() << d;
    os << si;
    os << usi;
    if (b.size() == os.pos())
	cout << "Correct number of bytes written\n";
    else
	cout.format ("Incorrect (%zu of %zu) number of bytes written\n", os.pos(), b.size());
    cout.flush();

    c = 0;
    uc = 0;
    bv = false;
    i = ui = li = uli = 0;
    f = 0; d = 0;
    si = usi = 0;

    istream is (b);
    is >> c;
    is >> uc;
    is >> ios::talign<bool>() >> bv;
    is >> ios::talign<int>() >> i;
    is >> ui;
    is >> ios::align() >> li;
    is >> uli;
    is >> ios::talign<float>() >> f;
    is >> ios::talign<double>() >> d;
    is >> si;
    is >> usi;
    if (is.pos() != b.size())
	cout << "Positional error\n";

    cout.format ("Values:\n"
	"char:    0x%02X\n"
	"u_char:  0x%02X\n"
	"bool:    %d\n"
	"int:     0x%08X\n"
	"u_int:   0x%08X\n"
	"long:    0x%08lX\n"
	"u_long:  0x%08lX\n"
	"float:   %.8f\n"
	"double:  %.16f\n"
	"short:   0x%04X\n"
	"u_short: 0x%04X\n",
	static_cast<int>(c), static_cast<int>(uc), static_cast<int>(bv),
	i, ui, li, uli, f, d, static_cast<int>(si), static_cast<int>(usi));

    if (isatty (STDIN_FILENO)) {
	cout << "\nBinary dump:\n";
	foreach (memblock::const_iterator, pc, b) {
	    if (pc > b.begin() && !(distance(b.begin(), pc) % 8))
		cout << endl;
	    cout.format ("%02X ", uint8_t(*pc));
	}
	cout << endl;
    }
}

StdBvtMain (TestStreams)
