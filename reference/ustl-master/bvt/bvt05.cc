// This file is part of the uSTL library, an STL implementation.
//
// Copyright (c) 2005 by Mike Sharov <msharov@users.sourceforge.net>
// This file is free software, distributed under the MIT License.

#include "stdtest.h"

typedef vector<int>			intvec_t;
typedef const intvec_t&			rcintvec_t;
typedef intvec_t::const_iterator	intiter_t;

static void printint (int i)
{
    cout.format ("%d ", i);
}

static void PrintVector (rcintvec_t v)
{
    cout << "{ ";
    foreach (intiter_t, i, v)
	printint (*i);
    cout << "}\n";
}

static bool is_even (int i)
{
    return (i % 2 == 0);
}

static int sqr (int i)
{
    return (i * i);
}

static int genint (void)
{
    static int counter = 0;
    return (counter++);
}

// In its own function because compilers differ in selecting const/nonconst
// members where no choice is needed.
//
static void TestEqualRange (rcintvec_t v)
{
    pair<intiter_t,intiter_t> rv;
    rv = equal_range (v, 10);
    cout.format ("Range of  10 is { %2zd, %2zd }\n", abs_distance (v.begin(), rv.first), abs_distance (v.begin(), rv.second));
    rv = equal_range (v, 0);
    cout.format ("Range of   0 is { %2zd, %2zd }\n", abs_distance (v.begin(), rv.first), abs_distance (v.begin(), rv.second));
    rv = equal_range (v, 100);
    cout.format ("Range of 100 is { %2zd, %2zd }\n", abs_distance (v.begin(), rv.first), abs_distance (v.begin(), rv.second));
}

template <typename T>
void TestBigFill (const size_t size, const T magic)
{
    vector<T> vbig (size);
    fill (vbig.begin() + 1, vbig.end(), magic);		// offset to test prealignment loop
    typename vector<T>::const_iterator iMismatch;
    iMismatch = find_if (vbig.begin() + 1, vbig.end(), bind1st (not_equal_to<T>(), magic));
    if (iMismatch == vbig.end())
	cout << "works\n";
    else
	cout.format ("does not work: mismatch at %zd, =0x%lX\n", abs_distance (vbig.begin(), iMismatch), (unsigned long)(*iMismatch));
}

template <typename T>
void TestBigCopy (const size_t size, const T magic)
{
    vector<T> vbig1 (size), vbig2 (size);
    fill (vbig1, magic);
    copy (vbig1.begin() + 1, vbig1.end(), vbig2.begin() + 1);	// offset to test prealignment loop
    typedef typename vector<T>::const_iterator iter_t;
    pair<iter_t, iter_t> iMismatch;
    iMismatch = mismatch (vbig1.begin() + 1, vbig1.end(), vbig2.begin() + 1);
    assert (iMismatch.second <= vbig2.end());
    if (iMismatch.first == vbig1.end())
	cout << "works\n";
    else
	cout.format ("does not work: mismatch at %zd, 0x%lX != 0x%lX\n", abs_distance(vbig1.begin(), iMismatch.first), (unsigned long)(*iMismatch.first), (unsigned long)(*iMismatch.second));
}

static void TestAlgorithms (void)
{
    static const int c_TestNumbers[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 10, 11, 12, 13, 13, 14, 15, 16, 17, 18 };
    const int* first = c_TestNumbers;
    const int* last = first + VectorSize(c_TestNumbers);
    intvec_t v, buf;
    v.assign (first, last);
    PrintVector (v);

    cout << "swap(1,2)\n";
    swap (v[0], v[1]);
    PrintVector (v);
    v.assign (first, last);

    cout << "copy(0,8,9)\n";
    copy (v.begin(), v.begin() + 8, v.begin() + 9);
    PrintVector (v);
    v.assign (first, last);

    cout << "copy with back_inserter\n";
    v.clear();
    copy (first, last, back_inserter(v));
    PrintVector (v);
    v.assign (first, last);

    cout << "copy with inserter\n";
    v.clear();
    copy (first, first + 5, inserter(v, v.begin()));
    copy (first, first + 5, inserter(v, v.begin() + 3));
    PrintVector (v);
    v.assign (first, last);

    cout << "copy_n(0,8,9)\n";
    copy_n (v.begin(), 8, v.begin() + 9);
    PrintVector (v);
    v.assign (first, last);

    cout << "copy_if(is_even)\n";
    intvec_t v_even;
    copy_if (v, back_inserter(v_even), &is_even);
    PrintVector (v_even);
    v.assign (first, last);

    cout << "for_each(printint)\n{ ";
    for_each (v, &printint);
    cout << "}\n";

    cout << "for_each(reverse_iterator, printint)\n{ ";
    for_each (v.rbegin(), v.rend(), &printint);
    cout << "}\n";

    cout << "find(10)\n";
    cout.format ("10 found at offset %zd\n", abs_distance (v.begin(), find (v, 10)));

    cout << "count(13)\n";
    cout.format ("%zu values of 13, %zu values of 18\n", count(v,13), count(v,18));

    cout << "transform(sqr)\n";
    transform (v, &sqr);
    PrintVector (v);
    v.assign (first, last);

    cout << "replace(13,666)\n";
    replace (v, 13, 666);
    PrintVector (v);
    v.assign (first, last);

    cout << "fill(13)\n";
    fill (v, 13);
    PrintVector (v);
    v.assign (first, last);

    cout << "fill_n(5, 13)\n";
    fill_n (v.begin(), 5, 13);
    PrintVector (v);
    v.assign (first, last);

    cout << "fill 64083 uint8_t(0x41) ";
    TestBigFill<uint8_t> (64083, 0x41);
    cout << "fill 64083 uint16_t(0x4142) ";
    TestBigFill<uint16_t> (64083, 0x4142);
    cout << "fill 64083 uint32_t(0x41424344) ";
    TestBigFill<uint32_t> (64083, 0x41424344);
    cout << "fill 64083 float(0.4242) ";
    TestBigFill<float> (64083, 0x4242f);
#if HAVE_INT64_T
    cout << "fill 64083 uint64_t(0x4142434445464748) ";
    TestBigFill<uint64_t> (64083, UINT64_C(0x4142434445464748));
#else
    cout << "No 64bit types available on this platform\n";
#endif

    cout << "copy 64083 uint8_t(0x41) ";
    TestBigCopy<uint8_t> (64083, 0x41);
    cout << "copy 64083 uint16_t(0x4142) ";
    TestBigCopy<uint16_t> (64083, 0x4142);
    cout << "copy 64083 uint32_t(0x41424344) ";
    TestBigCopy<uint32_t> (64083, 0x41424344);
    cout << "copy 64083 float(0.4242) ";
    TestBigCopy<float> (64083, 0.4242f);
#if HAVE_INT64_T
    cout << "copy 64083 uint64_t(0x4142434445464748) ";
    TestBigCopy<uint64_t> (64083, UINT64_C(0x4142434445464748));
#else
    cout << "No 64bit types available on this platform\n";
#endif

    cout << "generate(genint)\n";
    generate (v, &genint);
    PrintVector (v);
    v.assign (first, last);

    cout << "rotate(4)\n";
    rotate (v, 7);
    rotate (v, -3);
    PrintVector (v);
    v.assign (first, last);

    cout << "merge with (3,5,10,11,11,14)\n";
    const int c_MergeWith[] = { 3,5,10,11,11,14 };
    intvec_t vmerged;
    merge (v.begin(), v.end(), VectorRange(c_MergeWith), back_inserter(vmerged));
    PrintVector (vmerged);
    v.assign (first, last);

    cout << "inplace_merge with (3,5,10,11,11,14)\n";
    v.insert (v.end(), VectorRange(c_MergeWith));
    inplace_merge (v.begin(), v.end() - VectorSize(c_MergeWith), v.end());
    PrintVector (v);
    v.assign (first, last);

    cout << "remove(13)\n";
    remove (v, 13);
    PrintVector (v);
    v.assign (first, last);

    cout << "remove (elements 3, 4, 6, 15, and 45)\n";
    vector<uoff_t> toRemove;
    toRemove.push_back (3);
    toRemove.push_back (4);
    toRemove.push_back (6);
    toRemove.push_back (15);
    toRemove.push_back (45);
    typedef index_iterate<intvec_t::iterator, vector<uoff_t>::iterator> riiter_t;
    riiter_t rfirst = index_iterator (v.begin(), toRemove.begin());
    riiter_t rlast = index_iterator (v.begin(), toRemove.end());
    remove (v, rfirst, rlast);
    PrintVector (v);
    v.assign (first, last);

    cout << "unique\n";
    unique (v);
    PrintVector (v);
    v.assign (first, last);

    cout << "reverse\n";
    reverse (v);
    PrintVector (v);
    v.assign (first, last);

    cout << "lower_bound(10)\n";
    PrintVector (v);
    cout.format ("10 begins at position %zd\n", abs_distance (v.begin(), lower_bound (v, 10)));
    v.assign (first, last);

    cout << "upper_bound(10)\n";
    PrintVector (v);
    cout.format ("10 ends at position %zd\n", abs_distance (v.begin(), upper_bound (v, 10)));
    v.assign (first, last);

    cout << "equal_range(10)\n";
    PrintVector (v);
    TestEqualRange (v);
    v.assign (first, last);

    cout << "sort\n";
    reverse (v);
    PrintVector (v);
    random_shuffle (v);
    sort (v);
    PrintVector (v);
    v.assign (first, last);

    cout << "stable_sort\n";
    reverse (v);
    PrintVector (v);
    random_shuffle (v);
    stable_sort (v);
    PrintVector (v);
    v.assign (first, last);

    cout << "is_sorted\n";
    random_shuffle (v);
    const bool bNotSorted = is_sorted (v.begin(), v.end());
    sort (v);
    const bool bSorted = is_sorted (v.begin(), v.end());
    cout << "unsorted=" << bNotSorted << ", sorted=" << bSorted << endl;
    v.assign (first, last);

    cout << "find_first_of\n";
    static const int c_FFO[] = { 10000, -34, 14, 27 };
    cout.format ("found 14 at position %zd\n", abs_distance (v.begin(), find_first_of (v.begin(), v.end(), VectorRange(c_FFO))));
    v.assign (first, last);

    static const int LC1[] = { 3, 1, 4, 1, 5, 9, 3 };
    static const int LC2[] = { 3, 1, 4, 2, 8, 5, 7 };
    static const int LC3[] = { 1, 2, 3, 4 };
    static const int LC4[] = { 1, 2, 3, 4, 5 };
    cout << "lexicographical_compare";
    cout << "\nLC1 < LC2 == " << lexicographical_compare (VectorRange(LC1), VectorRange(LC2));
    cout << "\nLC2 < LC2 == " << lexicographical_compare (VectorRange(LC2), VectorRange(LC2));
    cout << "\nLC3 < LC4 == " << lexicographical_compare (VectorRange(LC3), VectorRange(LC4));
    cout << "\nLC4 < LC1 == " << lexicographical_compare (VectorRange(LC4), VectorRange(LC1));
    cout << "\nLC1 < LC4 == " << lexicographical_compare (VectorRange(LC1), VectorRange(LC4));

    cout << "\nmax_element\n";
    cout.format ("max element is %d\n", *max_element (v.begin(), v.end()));
    v.assign (first, last);

    cout << "min_element\n";
    cout.format ("min element is %d\n", *min_element (v.begin(), v.end()));
    v.assign (first, last);

    cout << "partial_sort\n";
    reverse (v);
    partial_sort (v.begin(), v.iat(v.size() / 2), v.end());
    PrintVector (v);
    v.assign (first, last);

    cout << "partial_sort_copy\n";
    reverse (v);
    buf.resize (v.size());
    partial_sort_copy (v.begin(), v.end(), buf.begin(), buf.end());
    PrintVector (buf);
    v.assign (first, last);

    cout << "partition\n";
    partition (v.begin(), v.end(), &is_even);
    PrintVector (v);
    v.assign (first, last);

    cout << "stable_partition\n";
    stable_partition (v.begin(), v.end(), &is_even);
    PrintVector (v);
    v.assign (first, last);

    cout << "next_permutation\n";
    buf.resize (3);
    iota (buf.begin(), buf.end(), 1);
    PrintVector (buf);
    while (next_permutation (buf.begin(), buf.end()))
	PrintVector (buf);
    cout << "prev_permutation\n";
    reverse (buf);
    PrintVector (buf);
    while (prev_permutation (buf.begin(), buf.end()))
	PrintVector (buf);
    v.assign (first, last);

    cout << "reverse_copy\n";
    buf.resize (v.size());
    reverse_copy (v.begin(), v.end(), buf.begin());
    PrintVector (buf);
    v.assign (first, last);

    cout << "rotate_copy\n";
    buf.resize (v.size());
    rotate_copy (v.begin(), v.iat (v.size() / 3), v.end(), buf.begin());
    PrintVector (buf);
    v.assign (first, last);

    static const int c_Search1[] = { 5, 6, 7, 8, 9 }, c_Search2[] = { 10, 10, 11, 14 };
    cout << "search\n";
    cout.format ("{5,6,7,8,9} at %zd\n", abs_distance (v.begin(), search (v.begin(), v.end(), VectorRange(c_Search1))));
    cout.format ("{10,10,11,14} at %zd\n", abs_distance (v.begin(), search (v.begin(), v.end(), VectorRange(c_Search2))));
    cout << "find_end\n";
    cout.format ("{5,6,7,8,9} at %zd\n", abs_distance (v.begin(), find_end (v.begin(), v.end(), VectorRange(c_Search1))));
    cout.format ("{10,10,11,14} at %zd\n", abs_distance (v.begin(), find_end (v.begin(), v.end(), VectorRange(c_Search2))));
    cout << "search_n\n";
    cout.format ("{14} at %zd\n", abs_distance (v.begin(), search_n (v.begin(), v.end(), 1, 14)));
    cout.format ("{13,13} at %zd\n", abs_distance (v.begin(), search_n (v.begin(), v.end(), 2, 13)));
    cout.format ("{10,10,10} at %zd\n", abs_distance (v.begin(), search_n (v.begin(), v.end(), 3, 10)));
    v.assign (first, last);

    cout << "includes\n";
    static const int c_Includes[] = { 5, 14, 15, 18, 20 };
    cout << "includes=" << includes (v.begin(), v.end(), VectorRange(c_Includes)-1);
    cout << ", not includes=" << includes (v.begin(), v.end(), VectorRange(c_Includes));
    cout << endl;

    static const int c_Set1[] = { 1, 2, 3, 4, 5, 6 }, c_Set2[] = { 4, 4, 6, 7, 8 };
    intvec_t::iterator setEnd;
    cout << "set_difference\n";
    v.resize (4);
    setEnd = set_difference (VectorRange(c_Set1), VectorRange(c_Set2), v.begin());
    PrintVector (v);
    if (setEnd != v.end()) cout << "incorrect range\n";
    cout << "set_symmetric_difference\n";
    v.resize (7);
    setEnd = set_symmetric_difference (VectorRange(c_Set1), VectorRange(c_Set2), v.begin());
    PrintVector (v);
    if (setEnd != v.end()) cout << "incorrect range\n";
    cout << "set_intersection\n";
    v.resize (2);
    setEnd = set_intersection (VectorRange(c_Set1), VectorRange(c_Set2), v.begin());
    PrintVector (v);
    if (setEnd != v.end()) cout << "incorrect range\n";
    cout << "set_union\n";
    v.resize (9);
    setEnd = set_union (VectorRange(c_Set1), VectorRange(c_Set2), v.begin());
    PrintVector (v);
    if (setEnd != v.end()) cout << "incorrect range\n";
    v.assign (first, last);
}

StdBvtMain (TestAlgorithms)
