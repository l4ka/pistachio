// This file is part of the uSTL library, an STL implementation.
//
// Copyright (c) 2005 by Mike Sharov <msharov@users.sourceforge.net>
// This file is free software, distributed under the MIT License.

#include "stdtest.h"

void TestEnumArithmetic (void)
{
    enum EFruit {
	apple,
	orange,
	plum,
	peach,
	pear,
	nectarine,
	NFruits
    };
    const char* fruits [NFruits + 1] = {
	"apple",
	"orange",
	"plum",
	"peach",
	"pear",
	"nectarine",
	"invalid"
    };
    cout << "Testing operator+" << endl;
    cout << "apple = " << fruits [apple] << endl;
    cout << "peach = " << fruits [apple + 3] << endl;
}

StdBvtMain (TestEnumArithmetic)
