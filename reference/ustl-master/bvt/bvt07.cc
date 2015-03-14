// This file is part of the uSTL library, an STL implementation.
//
// Copyright (c) 2005 by Mike Sharov <msharov@users.sourceforge.net>
// This file is free software, distributed under the MIT License.

#include "stdtest.h"

void MyFormat (const char* fmt, ...) __attribute__((__format__(__printf__,1,2)));

void TestString (void)
{
    static const char c_TestString1[] = "123456789012345678901234567890";
    static const char c_TestString2[] = "abcdefghijklmnopqrstuvwxyz";
    static const char c_TestString3[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    string s1 (c_TestString1);
    string s2 (VectorRange(c_TestString2)-1);
    string s3 (s1);

    cout << s1 << endl;
    cout << s2 << endl;
    cout << s3 << endl;
    s3.reserve (48);
    s3.resize (20);

    uoff_t i;
    for (i = 0; i < s3.length(); ++ i)
	s3.at(i) = s3.at(i);
    for (i = 0; i < s3.length(); ++ i)
	s3[i] = s3[i];
    cout.format ("%s\ns3.size() = %zu, max_size() = ", s3.c_str(), s3.size());
    if (s3.max_size() == SIZE_MAX - 1)
	cout << "(SIZE_MAX/elsize)-1";
    else
	cout << s3.max_size();
    cout.format (", capacity() = %zu\n", s3.capacity());

    s1.unlink();
    s1 = c_TestString2;
    s1 += c_TestString3;
    s1 += '$';
    cout << s1 << endl;

    s1 = "Hello";
    s2.unlink();
    s2 = "World";
    s3 = s1 + s2;
    cout << s3 << endl;
    s3 = "Concatenated " + s1 + s2 + " string.";
    cout << s3 << endl;

    if (s1 < s2)
	cout << "s1 < s2\n";
    if (s1 == s1)
	cout << "s1 == s1\n";
    if (s1[0] != s1[0])
	cout << "s1[0] != s1[0]\n";

    string s4;
    s4.link (s1);
    if (s1 == s4)
	cout << "s1 == s4\n";

    s1 = c_TestString1;
    string s5 (s1.begin() + 4, s1.begin() + 4 + 5);
    string s6 (s1.begin() + 4, s1.begin() + 4 + 5);
    if (s5 == s6)
	cout.format ("%s == %s\n", s5.c_str(), s6.c_str());
    string tail (s1.begin() + 7, s1.end());
    cout.format ("&s1[7] = %s\n", tail.c_str());

    cout.format ("initial:\t\t%s\n", s1.c_str());
    cout << "erase(5,find(9)-5)\t";
    s1.erase (5, s1.find ('9')-5);
    cout << s1 << endl;
    cout << "erase(5,5)\t\t";
    s1.erase (s1.begin() + 5, 2U);
    s1.erase (5, 3);
    assert (!*s1.end());
    cout << s1 << endl;
    cout << "push_back('x')\t\t";
    s1.push_back ('x');
    assert (!*s1.end());
    cout << s1 << endl;
    cout << "pop_back()\n";
    s1.pop_back();
    assert (!*s1.end());
    cout << "insert(10,#)\t\t";
    s1.insert (s1.begin() + 10, '#');
    assert (!*s1.end());
    cout << s1 << endl;
    cout << "replace(0,5,@)\t\t";
    s1.replace (s1.begin(), s1.begin() + 5, 1, '@');
    assert (!*s1.end());
    cout << s1 << endl;

    s1 = c_TestString1;
    cout.format ("8 found at %zu\n", s1.find ('8'));
    cout.format ("9 found at %zu\n", s1.find ("9"));
    cout.format ("7 rfound at %zu\n", s1.rfind ('7'));
    cout.format ("7 rfound again at %zu\n", s1.rfind ('7', s1.rfind ('7') - 1));
    cout.format ("67 rfound at %zu\n", s1.rfind ("67"));
    if (s1.rfind("X") == string::npos)
	cout << "X was not rfound\n";
    else
	cout.format ("X rfound at %zu\n", s1.rfind ("X"));
    uoff_t poundfound = s1.find ("#");
    if (poundfound != string::npos)
	cout.format ("# found at %zu\n", poundfound);
    cout.format ("[456] found at %zu\n", s1.find_first_of ("456"));
    cout.format ("[456] last found at %zu\n", s1.find_last_of ("456"));

    s2.clear();
    assert (!*s2.end());
    if (s2.empty())
	cout.format ("s2 is empty [%s], capacity %zu bytes\n", s2.c_str(), s2.capacity());

    s2.format ("<const] %d, %s, 0x%08X", 42, "[rfile>", 0xDEADBEEF);
    cout.format ("<%zu bytes of %zu> Format '%s'\n", s2.length(), s2.capacity(), s2.c_str());
    MyFormat ("'<const] %d, %s, 0x%08X'", 42, "[rfile>", 0xDEADBEEF);

    cout.format ("hash_value(s2) = %08X, string::hash(s2) = %08X\n", hash_value (s2.begin()), string::hash (s2.begin(), s2.end()));
}

void MyFormat (const char* fmt, ...)
{
    string buf;
    simd::reset_mmx();
    va_list args;
    va_start (args, fmt);
    buf.vformat (fmt, args);
    cout.format ("Custom vararg MyFormat: %s\n", buf.c_str());
    va_end (args);
}

StdBvtMain (TestString)
