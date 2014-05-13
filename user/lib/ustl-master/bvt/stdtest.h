// This file is part of the uSTL library, an STL implementation.
//
// Copyright (c) 2005 by Mike Sharov <msharov@users.sourceforge.net>
// This file is free software, distributed under the MIT License.

#pragma once
#include "../ustl.h"
using namespace ustl;

typedef void (*stdtestfunc_t)(void);

int StdTestHarness (stdtestfunc_t testFunction);

#define StdBvtMain(function)	int main (void) { return (StdTestHarness (&function)); }
