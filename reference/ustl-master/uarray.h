// This file is part of the uSTL library, an STL implementation.
//
// Copyright (c) 2014 by Mike Sharov <msharov@users.sourceforge.net>
// This file is free software, distributed under the MIT License.

#pragma once
#include "mistream.h"
#include "mostream.h"

namespace ustl {

/// \class array array.h stl.h
/// \ingroup Sequences
///
/// \brief A fixed-size array of \p N \p Ts.
///
template <typename T, size_t N>
class array {
public:
    typedef T						value_type;
    typedef unsigned					size_type;
    typedef value_type*					pointer;
    typedef const value_type*				const_pointer;
    typedef value_type&					reference;
    typedef const value_type&				const_reference;
    typedef pointer					iterator;
    typedef const_pointer				const_iterator;
    typedef ::ustl::reverse_iterator<iterator>		reverse_iterator;
    typedef ::ustl::reverse_iterator<const_iterator>	const_reverse_iterator;
    typedef std::initializer_list<value_type>		initlist_t;
public:
    inline			array (void)				{ fill_n (_v, N, T()); }
    template <typename T2>
    inline			array (const array<T2,N>& v)		{ copy_n (v.begin(), N, _v); }
    inline			array (const array& v)			{ copy_n (v.begin(), N, _v); }
    inline			array (const_pointer v)			{ copy_n (v, N, _v); }
    explicit inline		array (const_reference v0)		{ fill_n (_v, N, v0); }
    inline			array (const_reference v0, const_reference v1)	{ _v[0] = v0; fill_n (_v+1,N-1,v1); }
    inline			array (const_reference v0, const_reference v1, const_reference v2)	{ _v[0] = v0; _v[1] = v1; fill_n (_v+2,N-2,v2); }
    inline			array (const_reference v0, const_reference v1, const_reference v2, const_reference v3)	{ _v[0] = v0; _v[1] = v1; _v[2] = v2; fill_n (_v+3,N-3,v3); }
    inline iterator		begin (void)				{ return (_v); }
    inline const_iterator	begin (void) const			{ return (_v); }
    inline iterator		end (void)				{ return (begin() + N); }
    inline const_iterator	end (void) const			{ return (begin() + N); }
    inline size_type		size (void) const			{ return (N); }
    inline size_type		max_size (void) const			{ return (N); }
    inline bool			empty (void) const			{ return (N == 0); }
    inline const_reference	at (size_type i) const			{ return (_v[i]); }
    inline reference		at (size_type i)			{ return (_v[i]); }
    inline void			read (istream& is)			{ nr_container_read (is, *this); }
    inline void			write (ostream& os) const		{ nr_container_write (os, *this); }
    inline void			text_write (ostringstream& os) const	{ container_text_write (os, *this); }
    inline size_t		stream_size (void) const		{ return (nr_container_stream_size (*this)); }
    inline const_reference	operator[] (size_type i) const		{ return (_v[i]); }
    inline reference		operator[] (size_type i)		{ return (_v[i]); }
    template <typename T2>
    inline array&		operator= (const array<T2,N>& v)	{ copy_n (v.begin(), N, _v); return (*this); }
    inline array&		operator= (const array& v)		{ copy_n (v.begin(), N, _v); return (*this); }
    inline array&		operator+= (const_reference v)		{ for (size_type i = 0; i < N; ++i) _v[i] += v; return (*this); }
    inline array&		operator-= (const_reference v)		{ for (size_type i = 0; i < N; ++i) _v[i] -= v; return (*this); }
    inline array&		operator*= (const_reference v)		{ for (size_type i = 0; i < N; ++i) _v[i] *= v; return (*this); }
    inline array&		operator/= (const_reference v)		{ for (size_type i = 0; i < N; ++i) _v[i] /= v; return (*this); }
    inline array		operator+ (const_reference v) const	{ array result; for (size_type i = 0; i < N; ++i) result[i] = _v[i] + v; return (result); }
    inline array		operator- (const_reference v) const	{ array result; for (size_type i = 0; i < N; ++i) result[i] = _v[i] - v; return (result); }
    inline array		operator* (const_reference v) const	{ array result; for (size_type i = 0; i < N; ++i) result[i] = _v[i] * v; return (result); }
    inline array		operator/ (const_reference v) const	{ array result; for (size_type i = 0; i < N; ++i) result[i] = _v[i] / v; return (result); }
#if HAVE_CPP11
    inline			array (initlist_t v)			{ copy_n (v.begin(), N, _v); }
    inline array&		operator= (initlist_t v)		{ copy_n (v.begin(), N, _v); return (*this); }
    inline array&		operator+= (initlist_t v)		{ for (size_type i = 0; i < N; ++i) _v[i] += v.begin()[i]; return (*this); }
    inline array&		operator-= (initlist_t v)		{ for (size_type i = 0; i < N; ++i) _v[i] -= v.begin()[i]; return (*this); }
    inline array&		operator*= (initlist_t v)		{ for (size_type i = 0; i < N; ++i) _v[i] *= v.begin()[i]; return (*this); }
    inline array&		operator/= (initlist_t v)		{ for (size_type i = 0; i < N; ++i) _v[i] /= v.begin()[i]; return (*this); }
    inline array		operator+ (initlist_t v) const	{ array result; for (size_type i = 0; i < N; ++i) result[i] = _v[i] + v.begin()[i]; return (result); }
    inline array		operator- (initlist_t v) const	{ array result; for (size_type i = 0; i < N; ++i) result[i] = _v[i] - v.begin()[i]; return (result); }
    inline array		operator* (initlist_t v) const	{ array result; for (size_type i = 0; i < N; ++i) result[i] = _v[i] * v.begin()[i]; return (result); }
    inline array		operator/ (initlist_t v) const	{ array result; for (size_type i = 0; i < N; ++i) result[i] = _v[i] / v.begin()[i]; return (result); }
#endif
private:
    T				_v [N];
};

//----------------------------------------------------------------------

template <typename T1, size_t N, typename T2>
inline bool operator== (const array<T1,N>& t1, const array<T2,N>& t2)
{
    for (unsigned i = 0; i < N; ++ i)
	if (t1[i] != t2[i])
	    return (false);
    return (true);
}

template <typename T1, size_t N, typename T2>
inline bool operator< (const array<T1,N>& t1, const array<T2,N>& t2)
{
    for (unsigned i = 0; i < N && t1[i] <= t2[i]; ++ i)
	if (t1[i] < t2[i])
	    return (true);
    return (false);
}

template <typename T1, size_t N, typename T2>
inline const array<T1,N>& operator+= (array<T1,N>& t1, const array<T2,N>& t2)
    { for (unsigned i = 0; i < N; ++ i) t1[i] = T1(t1[i] + t2[i]); return (t1); }

template <typename T1, size_t N, typename T2>
inline const array<T1,N>& operator-= (array<T1,N>& t1, const array<T2,N>& t2)
    { for (unsigned i = 0; i < N; ++ i) t1[i] = T1(t1[i] - t2[i]); return (t1); }

template <typename T1, size_t N, typename T2>
inline const array<T1,N>& operator*= (array<T1,N>& t1, const array<T2,N>& t2)
    { for (unsigned i = 0; i < N; ++ i) t1[i] = T1(t1[i] * t2[i]); return (t1); }

template <typename T1, size_t N, typename T2>
inline const array<T1,N>& operator/= (array<T1,N>& t1, const array<T2,N>& t2)
    { for (unsigned i = 0; i < N; ++ i) t1[i] = T1(t1[i] / t2[i]); return (t1); }

template <typename T1, size_t N, typename T2>
inline const array<T1,N> operator+ (const array<T1,N>& t1, const array<T2,N>& t2)
{
    array<T1,N> result;
    for (unsigned i = 0; i < N; ++ i) result[i] = T1(t1[i] + t2[i]);
    return (result);
}

template <typename T1, size_t N, typename T2>
inline const array<T1,N> operator- (const array<T1,N>& t1, const array<T2,N>& t2)
{
    array<T1,N> result;
    for (unsigned i = 0; i < N; ++ i) result[i] = T1(t1[i] - t2[i]);
    return (result);
}

template <typename T1, size_t N, typename T2>
inline const array<T1,N> operator* (const array<T1,N>& t1, const array<T2,N>& t2)
{
    array<T1,N> result;
    for (unsigned i = 0; i < N; ++ i) result[i] = T1(t1[i] * t2[i]);
    return (result);
}

template <typename T1, size_t N, typename T2>
inline const array<T1,N> operator/ (const array<T1,N>& t1, const array<T2,N>& t2)
{
    array<T1,N> result;
    for (unsigned i = 0; i < N; ++ i) result[i] = T1(t1[i] / t2[i]);
    return (result);
}

} // namespace ustl
