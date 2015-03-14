// This file is part of the uSTL library, an STL implementation.
//
// Copyright (c) 2005 by Mike Sharov <msharov@users.sourceforge.net>
// This file is free software, distributed under the MIT License.

#pragma once
#include "unew.h"
#if HAVE_ALLOCA_H
    #include <alloca.h>
#else
    #include <stdlib.h>
#endif
#include "uiterator.h"
#include "ulimits.h"
#include "upair.h"

namespace ustl {

/// \class auto_ptr umemory.h ustl.h
/// \ingroup MemoryManagement
///
/// \brief A smart pointer.
///
/// Calls delete in the destructor; assignment transfers ownership.
/// This class does not work with void pointers due to the absence
/// of the required dereference operator.
///
template <typename T>
class auto_ptr {
public:
    typedef T		value_type;
    typedef T*		pointer;
    typedef T&		reference;
public:
    /// Takes ownership of \p p.
    inline explicit	auto_ptr (pointer p = NULL)	: m_p (p) {}
    /// Takes ownership of pointer in \p p. \p p relinquishes ownership.
    inline		auto_ptr (auto_ptr<T>& p)	: m_p (p.release()) {}
    /// Deletes the owned pointer.
    inline	       ~auto_ptr (void)			{ delete m_p; }
    /// Returns the pointer without relinquishing ownership.
    inline pointer	get (void) const		{ return (m_p); }
    /// Returns the pointer and gives up ownership.
    inline pointer	release (void)			{ pointer rv (m_p); m_p = NULL; return (rv); }
    /// Deletes the pointer and sets it equal to \p p.
    inline void		reset (pointer p)		{ if (p != m_p) { delete m_p; m_p = p; } }
    /// Takes ownership of \p p.
    inline auto_ptr<T>&	operator= (pointer p)		{ reset (p); return (*this); }
    /// Takes ownership of pointer in \p p. \p p relinquishes ownership.
    inline auto_ptr<T>&	operator= (auto_ptr<T>& p)	{ reset (p.release()); return (*this); }
    inline reference	operator* (void) const		{ return (*m_p); }
    inline pointer	operator-> (void) const		{ return (m_p); }
    inline bool		operator== (const pointer p) const	{ return (m_p == p); }
    inline bool		operator== (const auto_ptr<T>& p) const	{ return (m_p == p.m_p); }
    inline bool		operator< (const auto_ptr<T>& p) const	{ return (p.m_p < m_p); }
private:
    pointer		m_p;
};

/// Calls the placement new on \p p.
/// \ingroup RawStorageAlgorithms
///
template <typename T>
inline void construct (T* p)
{
    new (p) T;
}

/// Calls the placement new on \p p.
/// \ingroup RawStorageAlgorithms
///
template <typename ForwardIterator>
inline void construct (ForwardIterator first, ForwardIterator last)
{
    typedef typename iterator_traits<ForwardIterator>::value_type value_type;
    if (numeric_limits<value_type>::is_integral)
	memset ((void*) first, 0, max(distance(first,last),0)*sizeof(value_type));
    else
	for (--last; intptr_t(first) <= intptr_t(last); ++first)
	    construct (&*first);
}

/// Calls the placement new on \p p.
/// \ingroup RawStorageAlgorithms
///
template <typename T>
inline void construct (T* p, const T& value)
{
    new (p) T (value);
}

/// Calls the destructor of \p p without calling delete.
/// \ingroup RawStorageAlgorithms
///
template <typename T>
inline void destroy (T* p) noexcept
{
    p->~T();
}

// Helper templates to not instantiate anything for integral types.
template <typename T>
void dtors (T first, T last) noexcept
    { for (--last; intptr_t(first) <= intptr_t(last); ++first) destroy (&*first); }
template <typename T, bool bIntegral>
struct Sdtorsr {
    inline void operator()(T first, T last) noexcept { dtors (first, last); }
};
template <typename T>
struct Sdtorsr<T,true> {
    inline void operator()(T, T) noexcept {}
};

/// Calls the destructor on elements in range [first, last) without calling delete.
/// \ingroup RawStorageAlgorithms
///
template <typename ForwardIterator>
inline void destroy (ForwardIterator first, ForwardIterator last) noexcept
{
    typedef typename iterator_traits<ForwardIterator>::value_type value_type;
    Sdtorsr<ForwardIterator,numeric_limits<value_type>::is_integral>()(first, last);
}

/// Casts \p p to the type of the second pointer argument.
template <typename T> inline T* cast_to_type (void* p, const T*) { return ((T*) p); }

/// \brief Creates a temporary buffer pair from \p p and \p n
/// This is intended to be used with alloca to create temporary buffers.
/// The size in the returned pair is set to 0 if the allocation is unsuccessful.
/// \ingroup RawStorageAlgorithms
///
template <typename T>
inline pair<T*, ptrdiff_t> make_temporary_buffer (void* p, size_t n, const T* ptype)
{
    return (make_pair (cast_to_type(p,ptype), ptrdiff_t(p ? n : 0)));
}

#if HAVE_ALLOCA_H
    /// \brief Allocates a temporary buffer, if possible.
    /// \ingroup RawStorageAlgorithms
    #define get_temporary_buffer(size, ptype)	make_temporary_buffer (alloca(size_of_elements(size, ptype)), size, ptype)
    #define return_temporary_buffer(p)
#else
    #define get_temporary_buffer(size, ptype)	make_temporary_buffer (malloc(size_of_elements(size, ptype)), size, ptype)
    #define return_temporary_buffer(p)		if (p) free (p), p = NULL
#endif

/// Copies [first, last) into result by calling copy constructors in result.
/// \ingroup RawStorageAlgorithms
///
template <typename InputIterator, typename ForwardIterator>
ForwardIterator uninitialized_copy (InputIterator first, InputIterator last, ForwardIterator result)
{
    for (; first < last; ++result, ++first)
	construct (&*result, *first);
    return (result);
}

/// Copies [first, first + n) into result by calling copy constructors in result.
/// \ingroup RawStorageAlgorithms
///
template <typename InputIterator, typename ForwardIterator>
ForwardIterator uninitialized_copy_n (InputIterator first, size_t n, ForwardIterator result)
{
    for (++n; --n; ++result, ++first)
	construct (&*result, *first);
    return (result);
}

/// Calls construct on all elements in [first, last) with value \p v.
/// \ingroup RawStorageAlgorithms
///
template <typename ForwardIterator, typename T>
void uninitialized_fill (ForwardIterator first, ForwardIterator last, const T& v)
{
    for (; first < last; ++first)
	construct (&*first, v);
}

/// Calls construct on all elements in [first, first + n) with value \p v.
/// \ingroup RawStorageAlgorithms
///
template <typename ForwardIterator, typename T>
ForwardIterator uninitialized_fill_n (ForwardIterator first, size_t n, const T& v)
{
    for (++n; --n; ++first)
	construct (&*first, v);
    return (first);
}
    
} // namespace ustl

namespace std {	// Internal stuff must be in std::

/// Internal class for compiler support of C++11 initializer lists
template <typename T>
class initializer_list {
public:
    typedef T 			value_type;
    typedef size_t 		size_type;
    typedef const T& 		const_reference;
    typedef const_reference	reference;
    typedef const T* 		const_iterator;
    typedef const_iterator	iterator;
private:
    iterator			m_Data;
    size_type			m_Size;
private:
    /// This object is only constructed by the compiler when the {1,2,3}
    /// syntax is used, so the constructor must be private
    inline constexpr		initializer_list (const_iterator p, size_type sz) noexcept : m_Data(p), m_Size(sz) {}
public:
    inline constexpr		initializer_list (void)noexcept	: m_Data(NULL), m_Size(0) {}
    inline constexpr size_type	size (void) const noexcept	{ return (m_Size); }
    inline constexpr const_iterator begin() const noexcept	{ return (m_Data); }
    inline constexpr const_iterator end() const noexcept	{ return (begin()+size()); }
};

template <typename T>
inline constexpr const T* begin (initializer_list<T> il) noexcept { return (il.begin()); }
template <typename T>
inline constexpr const T* end (initializer_list<T> il) noexcept { return (il.end()); }

} // namespace std
