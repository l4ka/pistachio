// This file is part of the uSTL library, an STL implementation.
//
// Copyright (c) 2005 by Mike Sharov <msharov@users.sourceforge.net>
// This file is free software, distributed under the MIT License.

#pragma once
#include "ualgo.h"
#include "metamac.h"

namespace ustl {

/// \class tuple utuple.h ustl.h
/// \ingroup Sequences
///
/// \brief A fixed-size array of \p N \p Ts.
///
template <size_t N, typename T>
class tuple {
public:
    typedef T						value_type;
    typedef size_t					size_type;
    typedef value_type*					pointer;
    typedef const value_type*				const_pointer;
    typedef value_type&					reference;
    typedef const value_type&				const_reference;
    typedef pointer					iterator;
    typedef const_pointer				const_iterator;
    typedef ::ustl::reverse_iterator<iterator>		reverse_iterator;
    typedef ::ustl::reverse_iterator<const_iterator>	const_reverse_iterator;
    typedef pair<iterator,iterator>			range_t;
    typedef pair<const_iterator,const_iterator>		const_range_t;
    typedef std::initializer_list<value_type>		initlist_t;
public:
    template <typename T2>
    inline			tuple (const tuple<N,T2>& t);
    inline			tuple (const tuple<N,T>& t);
    inline			tuple (const_pointer v);
    inline			tuple (void);
    inline explicit		tuple (const_reference v0);
    inline			tuple (const_reference v0, const_reference v1);
    inline			tuple (const_reference v0, const_reference v1, const_reference v2);
    inline			tuple (const_reference v0, const_reference v1, const_reference v2, const_reference v3);
    inline iterator		begin (void)			{ return (m_v); }
    inline const_iterator	begin (void) const		{ return (m_v); }
    inline iterator		end (void)			{ return (begin() + N); }
    inline const_iterator	end (void) const		{ return (begin() + N); }
    inline size_type		size (void) const		{ return (N); }
    inline size_type		max_size (void) const		{ return (N); }
    inline bool			empty (void) const		{ return (N == 0); }
    inline const_reference	at (size_type i) const		{ return (m_v[i]); }
    inline reference		at (size_type i)		{ return (m_v[i]); }
    inline const_reference	operator[] (size_type i) const	{ return (m_v[i]); }
    inline reference		operator[] (size_type i)	{ return (m_v[i]); }
    template <typename T2>
    inline tuple&		operator= (const tuple<N,T2>& src);
    inline tuple&		operator= (const tuple<N,T>& src);
    inline tuple&		operator+= (const_reference v)
				    { for (uoff_t i = 0; i < N; ++ i) m_v[i] += v; return (*this); }
    inline tuple&		operator-= (const_reference v)
				    { for (uoff_t i = 0; i < N; ++ i) m_v[i] -= v; return (*this); }
    inline tuple&		operator*= (const_reference v)
				    { for (uoff_t i = 0; i < N; ++ i) m_v[i] *= v; return (*this); }
    inline tuple&		operator/= (const_reference v)
				    { for (uoff_t i = 0; i < N; ++ i) m_v[i] /= v; return (*this); }
    inline tuple		operator+ (const_reference v) const
				    { tuple result; for (uoff_t i = 0; i < N; ++ i) result[i] = m_v[i] + v; return (result); }
    inline tuple		operator- (const_reference v) const
				    { tuple result; for (uoff_t i = 0; i < N; ++ i) result[i] = m_v[i] - v; return (result); }
    inline tuple		operator* (const_reference v) const
				    { tuple result; for (uoff_t i = 0; i < N; ++ i) result[i] = m_v[i] * v; return (result); }
    inline tuple		operator/ (const_reference v) const
				    { tuple result; for (uoff_t i = 0; i < N; ++ i) result[i] = m_v[i] / v; return (result); }
    inline void			swap (tuple<N,T>& v)
				    { for (uoff_t i = 0; i < N; ++ i) ::ustl::swap (m_v[i], v.m_v[i]); }
    inline void			read (istream& is)			{ nr_container_read (is, *this); }
    inline void			write (ostream& os) const		{ nr_container_write (os, *this); }
    inline void			text_write (ostringstream& os) const	{ container_text_write (os, *this); }
    inline size_t		stream_size (void) const		{ return (nr_container_stream_size (*this)); }
#if HAVE_CPP11
    inline			tuple (initlist_t v)			{ assign (v); }
    inline tuple&		assign (initlist_t v);
    inline tuple&		operator= (initlist_t v)		{ return (assign(v)); }
    inline tuple&		operator+= (initlist_t v)
				    { for (uoff_t i = 0; i < min(N,v.size()); ++ i) m_v[i] += v.begin()[i]; return (*this); }
    inline tuple&		operator-= (initlist_t v)
				    { for (uoff_t i = 0; i < min(N,v.size()); ++ i) m_v[i] -= v.begin()[i]; return (*this); }
    inline tuple&		operator*= (initlist_t v)
				    { for (uoff_t i = 0; i < min(N,v.size()); ++ i) m_v[i] *= v.begin()[i]; return (*this); }
    inline tuple&		operator/= (initlist_t v)
				    { for (uoff_t i = 0; i < min(N,v.size()); ++ i) m_v[i] /= v.begin()[i]; return (*this); }
#endif
private:
    T				m_v [N];
};

} // namespace ustl

#include "simd.h"

namespace ustl {

template <size_t N, typename T>
template <typename T2>
inline tuple<N,T>::tuple (const tuple<N,T2>& t)
    { simd::pconvert (t, *this, simd::fcast<T2,T>()); }

template <size_t N, typename T>
inline tuple<N,T>::tuple (const tuple<N,T>& t)
    { simd::passign (t, *this); }

template <size_t N, typename T>
inline tuple<N,T>::tuple (const_pointer v)
    { simd::ipassign (v, *this); }

template <size_t N, typename T>
inline tuple<N,T>::tuple (void)
    { fill_n (m_v, N, T()); }

template <size_t N, typename T>
inline tuple<N,T>::tuple (const_reference v0)
    { fill_n (m_v, N, v0); }

template <size_t N, typename T>
inline tuple<N,T>::tuple (const_reference v0, const_reference v1)
{
    m_v[0] = v0;
    fill_n (m_v+1, N-1, v1);
}

template <size_t N, typename T>
inline tuple<N,T>::tuple (const_reference v0, const_reference v1, const_reference v2)
{
    m_v[0] = v0;
    m_v[1] = v1;
    fill_n (m_v+2, N-2, v2);
}

template <size_t N, typename T>
inline tuple<N,T>::tuple (const_reference v0, const_reference v1, const_reference v2, const_reference v3)
{
    m_v[0] = v0;
    m_v[1] = v1;
    m_v[2] = v2;
    fill_n (m_v+3, N-3, v3);
}

#if HAVE_CPP11
template <size_t N, typename T>
inline tuple<N,T>& tuple<N,T>::assign (initlist_t v)
{
    const size_t isz = min (v.size(), N);
    copy_n (v.begin(), isz, begin());
    fill_n (begin()+isz, N-isz, T());
    return (*this);
}
#endif

template <size_t N, typename T>
template <typename T2>
inline tuple<N,T>& tuple<N,T>::operator= (const tuple<N,T2>& src)
{ simd::pconvert (src, *this, simd::fcast<T2,T>()); return (*this); }

template <size_t N, typename T>
inline tuple<N,T>& tuple<N,T>::operator= (const tuple<N,T>& src)
{ simd::passign (src, *this); return (*this); }

template <size_t N, typename T1, typename T2>
inline bool operator== (const tuple<N,T1>& t1, const tuple<N,T2>& t2)
{
    for (uoff_t i = 0; i < N; ++ i)
	if (t1[i] != t2[i])
	    return (false);
    return (true);
}

template <size_t N, typename T1, typename T2>
inline bool operator< (const tuple<N,T1>& t1, const tuple<N,T2>& t2)
{
    for (uoff_t i = 0; i < N && t1[i] <= t2[i]; ++ i)
	if (t1[i] < t2[i])
	    return (true);
    return (false);
}

template <size_t N, typename T1, typename T2>
inline tuple<N,T1>& operator+= (tuple<N,T1>& t1, const tuple<N,T2>& t2)
    { for (uoff_t i = 0; i < N; ++ i) t1[i] = T1(t1[i] + t2[i]); return (t1); }

template <size_t N, typename T1, typename T2>
inline tuple<N,T1>& operator-= (tuple<N,T1>& t1, const tuple<N,T2>& t2)
    { for (uoff_t i = 0; i < N; ++ i) t1[i] = T1(t1[i] - t2[i]); return (t1); }

template <size_t N, typename T1, typename T2>
inline tuple<N,T1>& operator*= (tuple<N,T1>& t1, const tuple<N,T2>& t2)
    { for (uoff_t i = 0; i < N; ++ i) t1[i] = T1(t1[i] * t2[i]); return (t1); }

template <size_t N, typename T1, typename T2>
inline tuple<N,T1>& operator/= (tuple<N,T1>& t1, const tuple<N,T2>& t2)
    { for (uoff_t i = 0; i < N; ++ i) t1[i] = T1(t1[i] / t2[i]); return (t1); }

template <size_t N, typename T1, typename T2>
inline tuple<N,T1> operator+ (const tuple<N,T1>& t1, const tuple<N,T2>& t2)
{
    tuple<N,T1> result;
    for (uoff_t i = 0; i < N; ++ i) result[i] = T1(t1[i] + t2[i]);
    return (result);
}

template <size_t N, typename T1, typename T2>
inline tuple<N,T1> operator- (const tuple<N,T1>& t1, const tuple<N,T2>& t2)
{
    tuple<N,T1> result;
    for (uoff_t i = 0; i < N; ++ i) result[i] = T1(t1[i] - t2[i]);
    return (result);
}

template <size_t N, typename T1, typename T2>
inline tuple<N,T1> operator* (const tuple<N,T1>& t1, const tuple<N,T2>& t2)
{
    tuple<N,T1> result;
    for (uoff_t i = 0; i < N; ++ i) result[i] = T1(t1[i] * t2[i]);
    return (result);
}

template <size_t N, typename T1, typename T2>
inline tuple<N,T1> operator/ (const tuple<N,T1>& t1, const tuple<N,T2>& t2)
{
    tuple<N,T1> result;
    for (uoff_t i = 0; i < N; ++ i) result[i] = T1(t1[i] / t2[i]);
    return (result);
}

//----------------------------------------------------------------------
// Define SIMD specializations for member functions.

#if CPU_HAS_SSE
#define SSE_TUPLE_SPECS(n,type)							\
template <> inline tuple<n,type>::tuple (void)					\
{ asm("xorps %%xmm0, %%xmm0\n\tmovups %%xmm0, %0":"+m"(m_v[0])::"xmm0","memory"); } \
template<> inline void tuple<n,type>::swap (tuple<n,type>& v)			\
{										\
    asm ("movups %0,%%xmm0\n\tmovups %1,%%xmm1\n\t"				\
	"movups %%xmm0,%1\n\tmovups %%xmm1,%0"					\
	: "+m"(m_v[0]), "+m"(v.m_v[0]) :: "xmm0","xmm1","memory");		\
}
SSE_TUPLE_SPECS(4,float)
SSE_TUPLE_SPECS(4,int32_t)
SSE_TUPLE_SPECS(4,uint32_t)
#undef SSE_TUPLE_SPECS
#endif
#if SIZE_OF_LONG == 8 && __GNUC__
#define LONG_TUPLE_SPECS(n,type)		\
template <> inline tuple<n,type>::tuple (void)	\
{ asm("":"+m"(m_v[0])::"memory");		\
  *noalias_cast<long*>(m_v) = 0; }				\
template<> inline void tuple<n,type>::swap (tuple<n,type>& v)	\
{ asm("":"+m"(m_v[0]),"+m"(v.m_v[0])::"memory");			\
  iter_swap (noalias_cast<long*>(m_v), noalias_cast<long*>(v.m_v));	\
  asm("":"+m"(m_v[0]),"+m"(v.m_v[0])::"memory");			\
}
LONG_TUPLE_SPECS(2,float)
LONG_TUPLE_SPECS(4,int16_t)
LONG_TUPLE_SPECS(4,uint16_t)
LONG_TUPLE_SPECS(2,int32_t)
LONG_TUPLE_SPECS(2,uint32_t)
LONG_TUPLE_SPECS(8,int8_t)
LONG_TUPLE_SPECS(8,uint8_t)
#undef LONG_TUPLE_SPECS
#elif CPU_HAS_MMX
#define MMX_TUPLE_SPECS(n,type)		\
template <> inline tuple<n,type>::tuple (void)	\
{  asm ("pxor %%mm0, %%mm0\n\tmovq %%mm0, %0"	\
	:"=m"(m_v[0])::"mm0","st","memory"); simd::reset_mmx(); }	\
template<> inline void tuple<n,type>::swap (tuple<n,type>& v)		\
{  asm ("movq %2,%%mm0\n\tmovq %3,%%mm1\n\t"				\
	"movq %%mm0,%1\n\tmovq %%mm1,%0"				\
	:"=m"(m_v[0]),"=m"(v.m_v[0]):"m"(m_v[0]),"m"(v.m_v[0]):"mm0","mm1","st","st(1)","memory"); \
   simd::reset_mmx();							\
}
MMX_TUPLE_SPECS(2,float)
MMX_TUPLE_SPECS(4,int16_t)
MMX_TUPLE_SPECS(4,uint16_t)
MMX_TUPLE_SPECS(2,int32_t)
MMX_TUPLE_SPECS(2,uint32_t)
MMX_TUPLE_SPECS(8,int8_t)
MMX_TUPLE_SPECS(8,uint8_t)
#undef MMX_TUPLE_SPECS
#endif

#if __i386__ || __x86_64__
#define UINT32_TUPLE_SPECS(type,otype)		\
template <> inline tuple<2,type>::tuple (void)	\
{ asm("":"+m"(m_v[0]),"+m"(m_v[1])::"memory");	\
  *noalias_cast<uint32_t*>(m_v) = 0;		\
  asm("":"+m"(m_v[0]),"+m"(m_v[1])::"memory"); }\
template <> inline tuple<2,type>& tuple<2,type>::operator= (const tuple<2,type>& v)\
{ asm ("mov %3, %0"							\
       :"=m"(*noalias_cast<uint32_t*>(m_v)),"=m"(m_v[0]),"=m"(m_v[1])	\
       :"r"(*noalias_cast<const uint32_t*>(v.begin())),"m"(v[0]),"m"(v[1]):"memory");	\
  return (*this); }							\
template <> template <>							\
inline tuple<2,type>& tuple<2,type>::operator= (const tuple<2,otype>& v)\
{ asm ("mov %3, %0"							\
       :"=m"(*noalias_cast<uint32_t*>(m_v)),"=m"(m_v[0]),"=m"(m_v[1])	\
       :"r"(*noalias_cast<const uint32_t*>(v.begin())),"m"(v[0]),"m"(v[1]):"memory");	\
  return (*this); }							\
template <> inline tuple<2,type>::tuple (const tuple<2,type>& v)	\
{ operator= (v); }							\
template <> template <>							\
inline tuple<2,type>::tuple (const tuple<2,otype>& v)			\
{ operator= (v); }							\
template<> inline void tuple<2,type>::swap (tuple<2,type>& v)		\
{ asm(""::"m"(m_v[0]),"m"(m_v[1]),"m"(v.m_v[0]),"m"(v.m_v[1]):"memory");\
  iter_swap (noalias_cast<uint32_t*>(m_v), noalias_cast<uint32_t*>(v.m_v));			\
  asm("":"=m"(m_v[0]),"=m"(m_v[1]),"=m"(v.m_v[0]),"=m"(v.m_v[1])::"memory"); }			\
template <> inline tuple<2,type>& operator+= (tuple<2,type>& t1, const tuple<2,type>& t2)	\
    { t1[0] += t2[0]; t1[1] += t2[1]; return (t1); }						\
template <> inline tuple<2,type>& operator-= (tuple<2,type>& t1, const tuple<2,type>& t2)	\
    { t1[0] -= t2[0]; t1[1] -= t2[1]; return (t1); }						\
template <> inline tuple<2,type> operator+ (const tuple<2,type>& t1, const tuple<2,type>& t2)	\
    { return (tuple<2,type> (t1[0] + t2[0], t1[1] + t2[1])); }					\
template <> inline tuple<2,type> operator- (const tuple<2,type>& t1, const tuple<2,type>& t2)	\
    { return (tuple<2,type> (t1[0] - t2[0], t1[1] - t2[1])); }
UINT32_TUPLE_SPECS(int16_t,uint16_t)
UINT32_TUPLE_SPECS(uint16_t,int16_t)
#undef UINT32_TUPLE_SPECS
#endif

#undef TUPLEV_R1
#undef TUPLEV_R2
#undef TUPLEV_W1
#undef TUPLEV_W2

#define SIMD_TUPLE_PACKOP(N,T)	\
template <> inline tuple<N,T>& operator+= (tuple<N,T>& t1, const tuple<N,T>& t2)	\
    { simd::padd (t2, t1); return (t1); }						\
template <> inline tuple<N,T>& operator-= (tuple<N,T>& t1, const tuple<N,T>& t2)	\
    { simd::psub (t2, t1); return (t1); }						\
template <> inline tuple<N,T>& operator*= (tuple<N,T>& t1, const tuple<N,T>& t2)	\
    { simd::pmul (t2, t1); return (t1); }						\
template <> inline tuple<N,T>& operator/= (tuple<N,T>& t1, const tuple<N,T>& t2)	\
    { simd::pdiv (t2, t1); return (t1); }						\
template <> inline tuple<N,T> operator+ (const tuple<N,T>& t1, const tuple<N,T>& t2)	\
    { tuple<N,T> result (t1); simd::padd (t2, result); return (result); }		\
template <> inline tuple<N,T> operator- (const tuple<N,T>& t1, const tuple<N,T>& t2)	\
    { tuple<N,T> result (t1); simd::psub (t2, result); return (result); }		\
template <> inline tuple<N,T> operator* (const tuple<N,T>& t1, const tuple<N,T>& t2)	\
    { tuple<N,T> result (t1); simd::pmul (t2, result); return (result); }		\
template <> inline tuple<N,T> operator/ (const tuple<N,T>& t1, const tuple<N,T>& t2)	\
    { tuple<N,T> result (t1); simd::pdiv (t2, result); return (result); }
SIMD_TUPLE_PACKOP(4,float)
SIMD_TUPLE_PACKOP(2,float)
SIMD_TUPLE_PACKOP(2,double)
SIMD_TUPLE_PACKOP(4,int32_t)
SIMD_TUPLE_PACKOP(4,uint32_t)
SIMD_TUPLE_PACKOP(4,int16_t)
SIMD_TUPLE_PACKOP(4,uint16_t)
SIMD_TUPLE_PACKOP(2,int32_t)
SIMD_TUPLE_PACKOP(2,uint32_t)
SIMD_TUPLE_PACKOP(8,int8_t)
SIMD_TUPLE_PACKOP(8,uint8_t)
#undef SIMD_TUPLE_PACKOP

} // namespace ustl
