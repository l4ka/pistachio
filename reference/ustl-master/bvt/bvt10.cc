// This file is part of the uSTL library, an STL implementation.
//
// Copyright (c) 2005 by Mike Sharov <msharov@users.sourceforge.net>
// This file is free software, distributed under the MIT License.

#include "stdtest.h"

template <typename Container>
void PrintVector (const Container& ctr)
{
    cout << "{";
    foreach (typename Container::const_iterator, i, ctr)
	cout << ' ' << *i;
    cout << " }\n";
}

class A {
public:
        A (int dv = 6) : m_v1 (0), m_v (dv) {}
    int	addsix (int i) { return (i + m_v); }
    void addsix (int& i) const { i += m_v; }
    void addtosix (int i) { m_v += i; }
    inline void text_write (ostringstream& os) const { os << m_v; }
public:
    int m_v1;
    int m_v;
};

INTEGRAL_STREAMABLE(A)
TEXT_STREAMABLE(A)

void TestFunctors (void)
{
    vector<int> v;
    v.resize (20);
    fill (v, 2);
    foreach (vector<int>::iterator, i, v)
	*i -= distance(v.begin(), i) & 1;
    vector<int> v1 (v);

    cout << "start:\t\t\t";
    PrintVector (v);

    v = v1;
    cout << "plus:\t\t\t";
    transform (v, v.begin(), v.begin(), plus<int>());
    PrintVector (v);

    v = v1;
    cout << "minus:\t\t\t";
    transform (v, v.begin(), v.begin(), minus<int>());
    PrintVector (v);

    v = v1;
    cout << "divides:\t\t";
    transform (v, v.begin(), v.begin(), divides<int>());
    PrintVector (v);

    v = v1;
    cout << "multiplies:\t\t";
    transform (v, v.begin(), v.begin(), multiplies<int>());
    PrintVector (v);

    v = v1;
    cout << "modulus:\t\t";
    transform (v, v.begin(), v.begin(), modulus<int>());
    PrintVector (v);

    v = v1;
    cout << "logical_and:\t\t";
    transform (v, v.begin(), v.begin(), logical_and<int>());
    PrintVector (v);

    v = v1;
    cout << "logical_or:\t\t";
    transform (v, v.begin(), v.begin(), logical_or<int>());
    PrintVector (v);

    v = v1;
    cout << "equal_to:\t\t";
    transform (v, v.begin(), v.begin(), equal_to<int>());
    PrintVector (v);

    v = v1;
    cout << "not_equal_to:\t\t";
    transform (v, v.begin(), v.begin(), not_equal_to<int>());
    PrintVector (v);

    v = v1;
    cout << "greater:\t\t";
    transform (v, v.begin(), v.begin(), greater<int>());
    PrintVector (v);

    v = v1;
    cout << "less:\t\t\t";
    transform (v, v.begin(), v.begin(), less<int>());
    PrintVector (v);

    v = v1;
    cout << "greater_equal:\t\t";
    transform (v, v.begin(), v.begin(), greater_equal<int>());
    PrintVector (v);

    v = v1;
    cout << "less_equal:\t\t";
    transform (v, v.begin(), v.begin(), less_equal<int>());
    PrintVector (v);

    v = v1;
    cout << "compare:\t\t";
    transform (v, v.begin(), v.begin(), compare<int>());
    PrintVector (v);

    v = v1;
    cout << "negate:\t\t\t";
    transform (v, negate<int>());
    PrintVector (v);

    v = v1;
    cout << "logical_not:\t\t";
    transform (v, logical_not<int>());
    PrintVector (v);

    v = v1;
    cout << "unary_neg(negate):\t";
    transform (v, unary_negator(negate<int>()));
    PrintVector (v);

    v = v1;
    cout << "binder1st(plus,5):\t";
    transform (v, bind1st(plus<int>(), 5));
    PrintVector (v);

    v = v1;
    cout << "binder2nd(minus,1):\t";
    transform (v, bind2nd(minus<int>(), 1));
    PrintVector (v);

    v = v1;
    cout << "compose1(-,+5):\t\t";
    transform (v, compose1 (negate<int>(), bind2nd(plus<int>(), 5)));
    PrintVector (v);

    v = v1;
    cout << "compose1(-,-4):\t\t";
    transform (v, compose1 (negate<int>(), bind2nd(minus<int>(), 4)));
    PrintVector (v);

    v = v1;
    cout << "compose2(/,+6,-4):\t";
    transform (v, compose2 (divides<int>(), bind2nd(plus<int>(), 6), bind2nd(minus<int>(), 4)));
    PrintVector (v);

    cout << "mem_var(plus,6):\t";
    vector<A> av;
    for (uoff_t i = 0; i < 20; ++ i)
	av.push_back (A(i));
    transform (av, mem_var1(&A::m_v, bind2nd(plus<int>(), 6)));
    PrintVector (av);

    vector<A>::iterator found = find_if (av, mem_var_equal_to(&A::m_v, 14));
    cout << "14 found at position " << found - av.begin() << endl;
    found = lower_bound (av.begin(), av.end(), 18, mem_var_less(&A::m_v));
    cout << "18 found at position " << found - av.begin() << endl;

    cout << "add next:\t\t";
    transform (av.begin(), av.end() - 1, av.begin() + 1, av.begin(), mem_var2(&A::m_v, plus<int>()));
    PrintVector (av);
}

StdBvtMain (TestFunctors)
