// This file is part of the uSTL library, an STL implementation.
//
// Copyright (c) 2005 by Mike Sharov <msharov@users.sourceforge.net>
// This file is free software, distributed under the MIT License.

#include "stdtest.h"

template <size_t NX, size_t NY, typename T>
void TestMatrix (void)
{
    matrix<NX,NY,T> m1, m2;
    load_identity (m1);
    cout << "load_identity(m1)"
	    "\n    m1 = " << m1;
    m2 = m1;
    cout << "\nm1 = m2"
	    "\n    m2 = " << m2;
    m1 += m2;
    cout << "\nm1 += m2"
	    "\n    m1 = " << m1;
    m1 /= 2;
    cout << "\nm1 /= 2"
	    "\n    m1 = " << m1;
    m1 = m1 * m2;
    cout << "\nm1 = m1 * m2"
	    "\n    m1 = " << m1;
    m1 += 3;
    cout << "\nm1 += 3"
	    "\n    m1 = " << m1;
    load_identity (m2);
    m2 *= 2;
    m1 = m1 * m2;
    cout << "\nm1 *= I(2)";
    cout << "\n    m1 = " << m1;
    iota (m1.begin(), m1.end(), 1);
    cout << "\nm1 = iota(1)"
	    "\n    m1 = " << m1;
    cout << "\n    m1 row [1] = " << m1.row(1);
    cout << "\n    m1 column [2] = " << m1.column(2);
    m1 = m1 * m2;
    cout << "\nm1 *= I(2)"
	    "\n    m1 = " << m1;
    typename matrix<NX,NY,T>::column_type v, vt;
    iota (v.begin(), v.end(), 1);
    cout << "\nv = iota(1)"
	    "\n    v = " << v;
    load_identity (m2);
    m2 *= 2;
    for (uoff_t y = 0; y < NY - 1; ++ y)
	m2[NY - 1][y] = 1;
    cout << "\nm2 = I(2) + T(1)"
	    "\n    m2 = " << m2;
    vt = v * m2;
    cout << "\nvt = v * m2"
	    "\n    vt = " << vt << endl;
}

void TestMatrixAlgorithms (void)
{
    cout << "========================================\n"
	    "Testing 4x4 int matrix:\n"
	    "========================================\n";
    TestMatrix<4,4,int>();
    cout << "========================================\n"
	    "Testing 4x4 float matrix:\n"
	    "========================================\n";
    cout.set_precision (1);
    TestMatrix<4,4,float>();
}

StdBvtMain (TestMatrixAlgorithms)
