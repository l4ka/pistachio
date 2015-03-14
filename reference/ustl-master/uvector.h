// This file is part of the uSTL library, an STL implementation.
//
// Copyright (c) 2005 by Mike Sharov <msharov@users.sourceforge.net>
// This file is free software, distributed under the MIT License.

#pragma once
#include "memblock.h"
#include "umemory.h"
#include "upredalgo.h"

namespace ustl {

/// \class vector uvector.h ustl.h
/// \ingroup Sequences
///
/// \brief STL vector equivalent.
///
/// Provides a typed array-like interface to a managed memory block, including
/// element access, iteration, modification, resizing, and serialization. In
/// this design elements frequently undergo bitwise move, so don't put it in
/// here if it doesn't support it. This mostly means having no self-pointers.
///
template <typename T>
class vector {
public:
    typedef T				value_type;
    typedef value_type*			pointer;
    typedef const value_type*		const_pointer;
    typedef value_type&			reference;
    typedef const value_type&		const_reference;
    typedef pointer			iterator;
    typedef const_pointer		const_iterator;
    typedef memblock::size_type		size_type;
    typedef memblock::written_size_type	written_size_type;
    typedef memblock::difference_type	difference_type;
    typedef ::ustl::reverse_iterator<iterator>	reverse_iterator;
    typedef ::ustl::reverse_iterator<const_iterator>	const_reverse_iterator;
public:
    inline			vector (void);
    inline explicit		vector (size_type n);
				vector (size_type n, const T& v);
				vector (const vector<T>& v);
				vector (const_iterator i1, const_iterator i2);
    inline			~vector (void) noexcept;
    inline const vector<T>&	operator= (const vector<T>& v);
    inline bool			operator== (const vector<T>& v) const	{ return (m_Data == v.m_Data); }
    inline			operator cmemlink (void) const	{ return (cmemlink (m_Data)); }
    inline			operator cmemlink (void)	{ return (cmemlink (m_Data)); }
    inline			operator memlink (void)		{ return (memlink (m_Data)); }
    inline void			reserve (size_type n, bool bExact = false);
    inline void			resize (size_type n, bool bExact = true);
    inline size_type		capacity (void) const		{ return (m_Data.capacity() / sizeof(T));	}
    inline size_type		size (void) const		{ return (m_Data.size() / sizeof(T));		}
    inline size_type		max_size (void) const		{ return (m_Data.max_size() / sizeof(T));	}
    inline bool			empty (void) const		{ return (m_Data.empty());			}
    inline iterator		begin (void)			{ return (iterator (m_Data.begin()));		}
    inline const_iterator	begin (void) const		{ return (const_iterator (m_Data.begin()));	}
    inline iterator		end (void)			{ return (iterator (m_Data.end()));		}
    inline const_iterator	end (void) const		{ return (const_iterator (m_Data.end()));	}
    inline const_iterator	cbegin (void) const		{ return (begin()); }
    inline const_iterator	cend (void) const		{ return (end()); }
    inline reverse_iterator	rbegin (void)			{ return (reverse_iterator (end()));		}
  inline const_reverse_iterator	rbegin (void) const		{ return (const_reverse_iterator (end()));	}
    inline reverse_iterator	rend (void)			{ return (reverse_iterator (begin()));		}
  inline const_reverse_iterator	rend (void) const		{ return (const_reverse_iterator (begin()));	}
  inline const_reverse_iterator	crbegin (void) const		{ return (rbegin()); }
  inline const_reverse_iterator	crend (void) const		{ return (rend()); }
    inline iterator		data (void)			{ return (m_Data.data()); }
    inline const_iterator	data (void) const		{ return (m_Data.data()); }
    inline const_iterator	cdata (void) const		{ return (m_Data.cdata()); }
    inline iterator		iat (size_type i)		{ assert (i <= size()); return (begin() + i); }
    inline const_iterator	iat (size_type i) const		{ assert (i <= size()); return (begin() + i); }
    inline reference		at (size_type i)		{ assert (i < size()); return (begin()[i]); }
    inline const_reference	at (size_type i) const		{ assert (i < size()); return (begin()[i]); }
    inline reference		operator[] (size_type i)	{ return (at (i)); }
    inline const_reference	operator[] (size_type i) const	{ return (at (i)); }
    inline reference		front (void)			{ return (at(0)); }
    inline const_reference	front (void) const		{ return (at(0)); }
    inline reference		back (void)			{ assert (!empty()); return (end()[-1]); }
    inline const_reference	back (void) const		{ assert (!empty()); return (end()[-1]); }
    inline void			push_back (const T& v = T());
    inline void			pop_back (void)			{ destroy (end()-1); m_Data.memlink::resize (m_Data.size() - sizeof(T)); }
    inline void			clear (void)			{ destroy (begin(), end()); m_Data.clear(); }
    inline void			shrink_to_fit (void)		{ m_Data.shrink_to_fit(); }
    inline void			deallocate (void) noexcept;
    inline void			assign (const_iterator i1, const_iterator i2);
    inline void			assign (size_type n, const T& v);
    inline void			swap (vector<T>& v)		{ m_Data.swap (v.m_Data); }
    inline iterator		insert (const_iterator ip, const T& v);
    inline iterator		insert (const_iterator ip, size_type n, const T& v);
    inline iterator		insert (const_iterator ip, const_iterator i1, const_iterator i2);
    inline iterator		erase (const_iterator ep, size_type n = 1);
    inline iterator		erase (const_iterator ep1, const_iterator ep2);
    inline void			manage (pointer p, size_type n)		{ m_Data.manage (p, n * sizeof(T)); }
    inline bool			is_linked (void) const			{ return (m_Data.is_linked()); }
    inline void			unlink (void)				{ m_Data.unlink(); }
    inline void			copy_link (void)			{ m_Data.copy_link(); }
    inline void			link (const_pointer p, size_type n)	{ m_Data.link (p, n * sizeof(T)); }
    inline void			link (pointer p, size_type n)		{ m_Data.link (p, n * sizeof(T)); }
    inline void			link (const vector<T>& v)		{ m_Data.link (v); }
    inline void			link (vector<T>& v)			{ m_Data.link (v); }
    inline void			link (const_pointer first, const_pointer last)	{ m_Data.link (first, last); }
    inline void			link (pointer first, pointer last)		{ m_Data.link (first, last); }
    inline void			read (istream& is)			{ container_read (is, *this); }
    inline void			write (ostream& os) const		{ container_write (os, *this); }
    inline void			text_write (ostringstream& os) const	{ container_text_write (os, *this); }
    inline size_t		stream_size (void) const		{ return (container_stream_size (*this)); }
#if HAVE_CPP11
    inline			vector (vector<T>&& v)			: m_Data(forward<memblock>(v.m_Data)) {}
    inline			vector (std::initializer_list<T> v)	: m_Data() { uninitialized_copy_n (v.begin(), v.size(), append_hole(v.size())); }
    inline void			swap (vector<T>&& v)			{ m_Data.swap (v.m_Data); }
    inline vector<T>&		operator= (vector<T>&& v)		{ swap (forward<vector>(v)); return(*this); }
    template <typename... Args>
    inline iterator		emplace (const_iterator ip, Args&&... args);
    template <typename... Args>
    inline void			emplace_back (Args&&... args);
    inline void			push_back (T&& v)			{ emplace_back (forward<T>(v)); }
    inline iterator		insert (const_iterator ip, T&& v)	{ return (emplace (ip, forward<T>(v))); }
    inline iterator		insert (const_iterator ip, std::initializer_list<T> v)	{ return (insert (ip, v.begin(), v.end())); }
#endif
protected:
    inline iterator		insert_space (const_iterator ip, size_type n);
private:
    inline iterator		insert_hole (const_iterator ip, size_type n);
    inline iterator		append_hole (size_type n);
private:
    memblock			m_Data;	///< Raw element data, consecutively stored.
};

/// Allocates space for at least \p n elements.
template <typename T>
inline void vector<T>::reserve (size_type n, bool bExact)
{
    m_Data.reserve (n * sizeof(T), bExact);
}

template <typename T>
inline typename vector<T>::iterator vector<T>::append_hole (size_type n)
{
    m_Data.reserve (m_Data.size() + n*sizeof(T));
    m_Data.memlink::resize (m_Data.size()+n*sizeof(T));
    return (end()-n);
}

/// Resizes the vector to contain \p n elements.
template <typename T>
inline void vector<T>::resize (size_type n, bool bExact)
{
    destroy (begin()+n, end());
    const size_type nb = n * sizeof(T);
    if (m_Data.capacity() < nb)
	reserve (n, bExact);
    construct (end(), end() + (nb - m_Data.size())/sizeof(T));
    m_Data.memlink::resize (nb);
}

/// Calls element destructors and frees storage.
template <typename T>
inline void vector<T>::deallocate (void) noexcept
{
    destroy (begin(), end());
    m_Data.deallocate();
}

/// Initializes empty vector.
template <typename T>
inline vector<T>::vector (void)
: m_Data ()
{
}

/// Initializes a vector of size \p n.
template <typename T>
inline vector<T>::vector (size_type n)
: m_Data ()
{
    resize (n);
}

/// Copies \p n elements from \p v.
template <typename T>
vector<T>::vector (size_type n, const T& v)
: m_Data ()
{
    uninitialized_fill_n (append_hole (n), n, v);
}

/// Copies \p v.
template <typename T>
vector<T>::vector (const vector<T>& v)
: m_Data ()
{
    uninitialized_copy_n (v.begin(), v.size(), append_hole(v.size()));
}

/// Copies range [\p i1, \p i2]
template <typename T>
vector<T>::vector (const_iterator i1, const_iterator i2)
: m_Data ()
{
    uninitialized_copy (i1, i2, append_hole(distance(i1,i2)));
}

/// Destructor
template <typename T>
inline vector<T>::~vector (void) noexcept
{
    destroy (begin(), end());
}

/// Copies the range [\p i1, \p i2]
template <typename T>
inline void vector<T>::assign (const_iterator i1, const_iterator i2)
{
    assert (i1 <= i2);
    resize (distance (i1, i2));
    ::ustl::copy (i1, i2, begin());
}

/// Copies \p n elements with value \p v.
template <typename T>
inline void vector<T>::assign (size_type n, const T& v)
{
    resize (n);
    ::ustl::fill (begin(), end(), v);
}

/// Copies contents of \p v.
template <typename T>
inline const vector<T>& vector<T>::operator= (const vector<T>& v)
{
    assign (v.begin(), v.end());
    return (*this);
}

/// Inserts \p n uninitialized elements at \p ip.
template <typename T>
inline typename vector<T>::iterator vector<T>::insert_hole (const_iterator ip, size_type n)
{
    const uoff_t ipmi = distance (m_Data.begin(), memblock::const_iterator(ip));
    reserve (size() + n);
    return (iterator (m_Data.insert (m_Data.iat(ipmi), n * sizeof(T))));
}

/// Inserts \p n uninitialized elements at \p ip.
template <typename T>
inline typename vector<T>::iterator vector<T>::insert_space (const_iterator ip, size_type n)
{
    iterator ih = insert_hole (ip, n);
    construct (ih, ih+n);
    return (ih);
}

/// Inserts \p n elements with value \p v at offsets \p ip.
template <typename T>
inline typename vector<T>::iterator vector<T>::insert (const_iterator ip, size_type n, const T& v)
{
    iterator d = insert_hole (ip, n);
    uninitialized_fill_n (d, n, v);
    return (d);
}

/// Inserts value \p v at offset \p ip.
template <typename T>
inline typename vector<T>::iterator vector<T>::insert (const_iterator ip, const T& v)
{
    iterator d = insert_hole (ip, 1);
    construct (d, v);
    return (d);
}

/// Inserts range [\p i1, \p i2] at offset \p ip.
template <typename T>
inline typename vector<T>::iterator vector<T>::insert (const_iterator ip, const_iterator i1, const_iterator i2)
{
    assert (i1 <= i2);
    iterator d = insert_hole (ip, distance (i1, i2));
    uninitialized_copy (i1, i2, d);
    return (d);
}

/// Removes \p count elements at offset \p ep.
template <typename T>
inline typename vector<T>::iterator vector<T>::erase (const_iterator ep, size_type n)
{
    iterator d = const_cast<iterator>(ep);
    destroy (d, d+n);
    return (iterator (m_Data.erase (memblock::iterator(d), n * sizeof(T))));
}

/// Removes elements from \p ep1 to \p ep2.
template <typename T>
inline typename vector<T>::iterator vector<T>::erase (const_iterator ep1, const_iterator ep2)
{
    assert (ep1 <= ep2);
    return (erase (ep1, distance(ep1, ep2)));
}

/// Inserts value \p v at the end of the vector.
template <typename T>
inline void vector<T>::push_back (const T& v)
{
    construct (append_hole(1), v);
}

#if HAVE_CPP11

/// Constructs value at \p ip
template <typename T>
template <typename... Args>
inline typename vector<T>::iterator vector<T>::emplace (const_iterator ip, Args&&... args)
{
    return (new (insert_hole(ip,1)) T (forward<Args>(args)...));
}

/// Constructs value at the end of the vector.
template <typename T>
template <typename... Args>
inline void vector<T>::emplace_back (Args&&... args)
{
    new (append_hole(1)) T (forward<Args>(args)...);
}

#endif

/// Use with vector classes to allocate and link to stack space. \p n is in elements.
#define typed_alloca_link(m,T,n)	(m).link ((T*) alloca ((n) * sizeof(T)), (n))

} // namespace ustl
