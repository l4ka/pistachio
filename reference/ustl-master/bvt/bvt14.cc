// This file is part of the uSTL library, an STL implementation.
//
// Copyright (c) 2005 by Mike Sharov <msharov@users.sourceforge.net>
// This file is free software, distributed under the MIT License.

#include "stdtest.h"

void TestMap (void)
{
    typedef map<string,int> monthmap_t;
    monthmap_t months;
    months["january"] = 31;
    months["february"] = 28;
    months["march"] = 31;
    months["april"] = 30;
    months["may"] = 31;
    months["june"] = 30;
    months["july"] = 31;
    months["august"] = 31;
#if HAVE_CPP11
    months.insert ({
	{"september",30},
	{"october",31},
	{"november",30},
	{"december",31}
    });
#else
    months["september"] = 30;
    months["october"] = 31;
    months["november"] = 30;
    months["december"] = 31;
#endif
    
    const monthmap_t& cmonths = months;
    cout << "There are " << cmonths["january"] << " days in january." << endl;
    cout << "There are " << cmonths.at("september") << " days in september." << endl;
    cout << "There are " << cmonths["december"] << " days in december." << endl;
    monthmap_t::const_iterator found_may = months.find ("may");
    cout << found_may->first << " found at index " << found_may - months.begin() << endl;
    cout << "Alphabetical listing:" << endl;

    monthmap_t::const_iterator i;
    for (i = months.begin(); i < months.end(); ++ i)
	cout << i->first << " has " << i->second << " days." << endl;

    monthmap_t mcopy (months);
    mcopy.erase ("may");
    cout << "After erasing may:" << endl;
    for (i = mcopy.begin(); i < mcopy.end(); ++ i)
	cout << i->first << " ";
    cout << endl;

    mcopy.assign (months.begin(), months.end() - 1);
    mcopy.erase (mcopy.begin() + 1, mcopy.begin() + 4);
    cout << "After erasing months 2, 3, 4, and the last one:" << endl;
    for (i = mcopy.begin(); i < mcopy.end(); ++ i)
	cout << i->first << " ";
    cout << endl;

    mcopy = months;
#if HAVE_CPP11
    monthmap_t::iterator frob = mcopy.emplace_hint (mcopy.begin(), make_pair (string("frobuary"), 42));
#else
    monthmap_t::iterator frob = mcopy.insert (mcopy.begin(), make_pair (string("frobuary"), 42));
#endif
    cout << "After inserting " << frob->first << "," << frob->second << ":" << endl;
    for (i = mcopy.begin(); i < mcopy.end(); ++ i)
	cout << i->first << " ";
    cout << endl;
}

StdBvtMain (TestMap)
