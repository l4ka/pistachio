// This file is part of the uSTL library, an STL implementation.
//
// Copyright (c) 2005 by Mike Sharov <msharov@users.sourceforge.net>
// This file is free software, distributed under the MIT License.

#include "stdtest.h"

void ObjectSerialization (void)
{
    #define RW(stream)	rws[stream.pos() == expect]
    const void* pBufC = NULL;
    void* pBuf = NULL;
    memblock buffer;
    string testString ("TestString");
    const string* pStrC = NULL;
    string* pStr = NULL;
    vector<uint16_t> tv (7);
    static const char* rws[2] = { "wrong", "right" };

    const size_t bufSize = stream_size_of(pBufC) +
			   stream_size_of(pBuf) +
			   Align(stream_size_of(testString)) +
			   stream_size_of(pStrC) +
			   stream_size_of(pStr) +
			   stream_size_of(tv);
    buffer.resize (bufSize);
    pBufC = buffer.cdata();
    pBuf = buffer.data();

    uoff_t expect = 0;
    ostream os (buffer);
    os << pBufC; expect += stream_size_of(pBufC);
    cout << "Write const void*, pos is " << RW(os) << endl;
    os << pBuf; expect += stream_size_of(pBuf);
    cout << "Write void*, pos is " << RW(os) << endl;
    os << testString; expect += stream_size_of(testString);
    cout << "Write string, pos is " << RW(os) << endl;
    os.align(); expect = Align (expect);
    os << const_cast<const string*>(&testString); expect += stream_size_of(&testString);
    cout << "Write const string*, pos is " << RW(os) << endl;
    os << &testString; expect += stream_size_of(&testString);
    cout << "Write string*, pos is " << RW(os) << endl;
    os << tv; expect += stream_size_of(tv);
    cout << "Write vector<uint16_t>(7), pos is " << RW(os) << endl;
    if (os.pos() != bufSize)
	cout << "Incorrect number of bytes written: " << os.pos() << " of " << bufSize << endl;
    
    istream is (buffer);
    expect = 0;
    is >> pBufC;
    expect += stream_size_of(pBufC);
    cout << "Read const void*, pos is " << RW(is);
    cout << ", value is " << rws[pBufC == buffer.cdata()] << endl;
    is >> pBuf;
    expect += stream_size_of(pBuf);
    cout << "Read void*, pos is " << RW(is);
    cout << ", value is " << rws[pBuf == buffer.cdata()] << endl;
    testString.clear();
    is >> testString;
    expect += stream_size_of(testString);
    cout << "Read string, pos is " << RW(is) << ", value is " << testString << endl;
    is.align();
    expect = Align (expect);
    is >> pStrC;
    expect += stream_size_of(pStrC);
    cout << "Read const string*, pos is " << RW(is);
    cout << ", value is " << rws[pStrC == &testString] << endl;
    is >> pStr;
    expect += stream_size_of(pStr);
    cout << "Read string*, pos is " << RW(is);
    cout << ", value is " << rws[pStr == &testString] << endl;
    vector<uint16_t> rv;
    is >> rv;
    expect += stream_size_of(rv);
    cout << "Read vector<uint16_t>(" << rv.size() << "), pos is " << RW(is);
    cout << ", value is " << rws[rv == tv] << endl;
    if (is.pos() != bufSize)
	cout << "Incorrect number of bytes read: " << is.pos() << " of " << bufSize << endl;
}

StdBvtMain (ObjectSerialization)
