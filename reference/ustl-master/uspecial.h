// This file is part of the uSTL library, an STL implementation.
//
// Copyright (c) 2005 by Mike Sharov <msharov@users.sourceforge.net>
// This file is free software, distributed under the MIT License.

#pragma once
#include "uvector.h"
#include "ustring.h"
#include "uset.h"
#include "umultiset.h"
#include "ubitset.h"
#include "ulaalgo.h"
#include "uarray.h"
#include "uctralgo.h"
#include "ufunction.h"
#include "uctrstrm.h"
#include "sistream.h"
#include <ctype.h>

namespace ustl {

//----------------------------------------------------------------------
// Alogrithm specializations not in use by the library code.
//----------------------------------------------------------------------

template <> inline void swap (cmemlink& a, cmemlink& b)			{ a.swap (b); }
template <> inline void swap (memlink& a, memlink& b)			{ a.swap (b); }
template <> inline void swap (memblock& a, memblock& b)			{ a.swap (b); }
template <> inline void swap (string& a, string& b)			{ a.swap (b); }
#define TEMPLATE_SWAP_PSPEC(type, template_decl)	\
template_decl inline void swap (type& a, type& b) { a.swap (b); }
TEMPLATE_SWAP_PSPEC (TEMPLATE_TYPE1 (vector,T),		TEMPLATE_DECL1 (T))
TEMPLATE_SWAP_PSPEC (TEMPLATE_TYPE1 (set,T),		TEMPLATE_DECL1 (T))
TEMPLATE_SWAP_PSPEC (TEMPLATE_TYPE1 (multiset,T),	TEMPLATE_DECL1 (T))
TEMPLATE_SWAP_PSPEC (TEMPLATE_TYPE2 (tuple,N,T),	TEMPLATE_FULL_DECL2 (size_t,N,typename,T))

//----------------------------------------------------------------------
// Streamable definitions. Not used in the library and require streams.
//----------------------------------------------------------------------

//----{ pair }----------------------------------------------------------

/// \brief Reads pair \p p from stream \p is.
template <typename T1, typename T2>
istream& operator>> (istream& is, pair<T1,T2>& p)
{
    is >> p.first;
    is.align (stream_align_of(p.second));
    is >> p.second;
    is.align (stream_align_of(p.first));
    return (is);
}

/// Writes pair \p p to stream \p os.
template <typename T1, typename T2>
ostream& operator<< (ostream& os, const pair<T1,T2>& p)
{
    os << p.first;
    os.align (stream_align_of(p.second));
    os << p.second;
    os.align (stream_align_of(p.first));
    return (os);
}

/// Writes pair \p p to stream \p os.
template <typename T1, typename T2>
ostringstream& operator<< (ostringstream& os, const pair<T1,T2>& p)
{
    os << '(' << p.first << ',' << p.second << ')';
    return (os);
}

/// Returns the written size of the object.
template <typename T1, typename T2>
struct object_stream_size<pair<T1,T2> > {
    inline size_t operator()(const pair<T1,T2>& v) const
    {
	return (Align (stream_size_of(v.first), stream_align_of(v.second)) +
		Align (stream_size_of(v.second), stream_align_of(v.first)));
    }
};

/// \brief Takes a pair and returns pair.first
/// This is an extension, available in uSTL and the SGI STL.
template <typename Pair> struct select1st : public unary_function<Pair,typename Pair::first_type> {
    typedef typename Pair::first_type result_type;
    inline const result_type&	operator()(const Pair& a) const { return (a.first); }
    inline result_type&		operator()(Pair& a) const { return (a.first); }
};

/// \brief Takes a pair and returns pair.second
/// This is an extension, available in uSTL and the SGI STL.
template <typename Pair> struct select2nd : public unary_function<Pair,typename Pair::second_type> {
    typedef typename Pair::second_type result_type;
    inline const result_type&	operator()(const Pair& a) const { return (a.second); }
    inline result_type&		operator()(Pair& a) const { return (a.second); }
};

/// \brief Converts a const_iterator pair into an iterator pair
/// Useful for converting pair ranges returned by equal_range, for instance.
/// This is an extension, available in uSTL.
template <typename Container>
inline pair<typename Container::iterator, typename Container::iterator>
unconst (const pair<typename Container::const_iterator, typename Container::const_iterator>& i, Container&)
{
    typedef pair<typename Container::iterator, typename Container::iterator> unconst_pair_t;
    return (*noalias_cast<unconst_pair_t*>(&i));
}

//----{ vector }--------------------------------------------------------

template <typename T>
inline size_t stream_align_of (const vector<T>&)
{
    typedef typename vector<T>::written_size_type written_size_type;
    return (stream_align_of (written_size_type()));
}

//----{ bitset }--------------------------------------------------------

/// Writes bitset \p v into stream \p os.
template <size_t Size>
istringstream& operator>> (istringstream& is, bitset<Size>& v)
{
    char c;
    for (int i = Size; --i >= 0 && (is >> c).good();)
	v.set (i, c == '1');
    return (is);
}

//----{ tuple }---------------------------------------------------------

template <size_t N, typename T>
inline istream& operator>> (istream& is, tuple<N,T>& v)
    { v.read (is); return (is); }
template <size_t N, typename T>
inline ostream& operator<< (ostream& os, const tuple<N,T>& v)
    { v.write (os); return (os); }
template <size_t N, typename T>
inline ostringstream& operator<< (ostringstream& os, const tuple<N,T>& v)
    { v.text_write (os); return (os); }

template <size_t N, typename T>
struct numeric_limits<tuple<N,T> > {
    typedef numeric_limits<T> value_limits;
    static inline tuple<N,T> min (void)	{ tuple<N,T> v; fill (v, value_limits::min()); return (v); }
    static inline tuple<N,T> max (void)	{ tuple<N,T> v; fill (v, value_limits::max()); return (v); }
    static const bool is_signed = value_limits::is_signed;
    static const bool is_integer = value_limits::is_integer;
    static const bool is_integral = value_limits::is_integral;
};

template <size_t N, typename T>
inline size_t stream_align_of (const tuple<N,T>&) { return (stream_align_of (NullValue<T>())); }

template <typename T, typename IntT>
inline ostringstream& chartype_text_write (ostringstream& os, const T& v)
{
    os.format (_FmtPrtChr[!isprint(v)], v);
    return (os);
}

template <>
inline ostringstream& container_element_text_write (ostringstream& os, const uint8_t& v)
{ return (chartype_text_write<uint8_t, unsigned int> (os, v)); }
template <>
inline ostringstream& container_element_text_write (ostringstream& os, const int8_t& v)
{ return (chartype_text_write<int8_t, int> (os, v)); }

//----{ matrix }--------------------------------------------------------

/// Writes tuple \p v into stream \p os.
template <size_t NX, size_t NY, typename T>
ostringstream& operator<< (ostringstream& os, const matrix<NX,NY,T>& v)
{
    os << '(';
    for (uoff_t row = 0; row < NY; ++ row) {
	os << '(';
        for (uoff_t column = 0; column < NX; ++column)
	    os << v[row][column] << ",)"[column == NX-1];
    }
    os << ')';
    return (os);
}

//----{ long4grain }----------------------------------------------------

#if SIZE_OF_LONG == 8 && HAVE_INT64_T
// Helper class for long4grain and ptr4grain wrappers.
class _long4grain {
public:
    inline	_long4grain (uint64_t v)	: m_v (v) {}
#if __x86_64__
    inline void	read (istream& is)
    {
	assert (is.aligned(4));
	#if WANT_STREAM_BOUNDS_CHECKING
	    if (!is.verify_remaining ("read", "long4grain", sizeof(m_v))) return;
	#else
	    assert (is.remaining() >= sizeof(m_v));
	#endif
	m_v = *reinterpret_cast<const uint64_t*>(is.ipos());
	is.skip (sizeof(m_v));
    }
    inline void	write (ostream& os) const
    {
	assert (os.aligned(4));
	#if WANT_STREAM_BOUNDS_CHECKING
	    if (!os.verify_remaining ("write", "long4grain", sizeof(m_v))) return;
	#else
	    assert (os.remaining() >= sizeof(m_v));
	#endif
	*reinterpret_cast<uint64_t*>(os.ipos()) = m_v;
	os.skip (sizeof(m_v));
    }
#elif USTL_BYTE_ORDER == USTL_BIG_ENDIAN
    inline void	read (istream& is)		{ uint32_t vl, vh; is >> vh >> vl; m_v = (uint64_t(vh) << 32) | uint64_t(vl); }
    inline void	write (ostream& os) const	{ os << uint32_t(m_v >> 32) << uint32_t(m_v); }
#else
    inline void	read (istream& is)		{ uint32_t vl, vh; is >> vl >> vh; m_v = (uint64_t(vh) << 32) | uint64_t(vl); }
    inline void	write (ostream& os) const	{ os << uint32_t(m_v) << uint32_t(m_v >> 32); }
#endif
    inline size_t stream_size (void) const	{ return (stream_size_of(m_v)); }
private:
    uint64_t	m_v;
};

/// Wrap long values to allow writing them on 4-grain even on 64bit platforms.
inline _long4grain& long4grain (unsigned long& v)		{ asm("":"+m"(v)); return (*noalias_cast<_long4grain*>(&v)); }
/// Wrap long values to allow writing them on 4-grain even on 64bit platforms.
inline const _long4grain long4grain (const unsigned long& v)	{ return (_long4grain(v)); }
/// Wrap pointer values to allow writing them on 4-grain even on 64bit platforms.
template <typename T>
inline _long4grain& ptr4grain (T*& p)				{ asm("":"+m"(p)); return (*noalias_cast<_long4grain*>(&p)); }
/// Wrap pointer values to allow writing them on 4-grain even on 64bit platforms.
template <typename T>
inline const _long4grain ptr4grain (const T* const& p)		{ return (_long4grain(uintptr_t(p))); }
#else	// if not SIZE_OF_LONG == 8 && HAVE_INT64_T
inline unsigned long& long4grain (unsigned long& v)		{ return (v); }
inline const unsigned long& long4grain (const unsigned long& v)	{ return (v); }
template <typename T> inline T*& ptr4grain (T*& p)		{ return (p); }
template <typename T> inline const T* const& ptr4grain (const T* const& p) { return (p); }
#endif	// SIZE_OF_LONG == 8

//----------------------------------------------------------------------

} // namespace ustl

// This is here because there really is no other place to put it.
#if SIZE_OF_BOOL != SIZE_OF_CHAR
// bool is a big type on some machines (like DEC Alpha), so it's written as a byte.
ALIGNOF(bool, sizeof(uint8_t))
CAST_STREAMABLE(bool, uint8_t)
#endif
#if SIZE_OF_LONG == 8 && HAVE_INT64_T
ALIGNOF (_long4grain, 4)
#endif

ALIGNOF(ustl::CBacktrace, sizeof(void*))
ALIGNOF (ustl::string, stream_align_of (string::value_type()))
