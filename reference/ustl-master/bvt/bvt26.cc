// This file is part of the uSTL library, an STL implementation.
//
// Copyright (c) 2005 by Mike Sharov <msharov@users.sourceforge.net>
// This file is free software, distributed under the MIT License.

#include "stdtest.h"

void TestMacros (void)
{
    #define VARNAME(n) LARG_NUMBER(v,n)
    #define VARDECL(n) VARNAME(n) = n
    int COMMA_LIST (9, VARDECL);
    cout << LIST(9, VARNAME, <<) << endl;
    #define TO_STRING(n) #n
    #define PRINT_N(n) REPEAT(n, TO_STRING) "\n"
    cout << LIST(9, PRINT_N, <<);
}

StdBvtMain (TestMacros)
