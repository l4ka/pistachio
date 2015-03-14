// This file is part of the uSTL library, an STL implementation.
//
// Copyright (c) 2005 by Mike Sharov <msharov@users.sourceforge.net>
// This file is free software, distributed under the MIT License.

#pragma once

namespace ustl {

/// \class queue uqueue.h ustl.h
/// \ingroup Sequences
///
/// \brief Queue adapter to uSTL containers.
///
/// The most efficient way to use this implementation is to fill the queue
/// and the completely empty it before filling again.
///
template <typename T>
class queue {
public:
    typedef T			value_type;
    typedef size_t		size_type;
    typedef ptrdiff_t		difference_type;
    typedef T&			reference;
    typedef const T&		const_reference;
    typedef T*			pointer;
    typedef const T*		const_pointer;
public:
    inline			queue (void)			: m_Storage (), m_Front (0) { }
    explicit inline		queue (const vector<T>& s)	: m_Storage (s), m_Front (0) { }
    explicit inline		queue (const queue& s)		: m_Storage (s.m_Storage), m_Front (0) { }
    inline size_type		size (void) const		{ return (m_Storage.size() - m_Front); }
    inline bool			empty (void) const		{ return (!size()); }
    inline reference		front (void)			{ return (m_Storage [m_Front]); }
    inline const_reference	front (void) const		{ return (m_Storage [m_Front]); }
    inline reference		back (void)			{ return (m_Storage.back()); }
    inline const_reference	back (void) const		{ return (m_Storage.back()); }
    inline void			push (const value_type& v);
    inline void			pop (void);
    inline bool			operator== (const queue& s) const	{ return (m_Storage == s.m_Storage && m_Front == s.m_Front); }
    inline bool			operator< (const queue& s) const	{ return (size() < s.size()); }
private:
    vector<T>			m_Storage;	///< Where the data actually is.
    size_type			m_Front;	///< Index of the element returned by next pop.
};

/// Pushes \p v on the queue.
template <typename T>
inline void queue<T>::push (const value_type& v)
{
    if (m_Front) {
	m_Storage.erase (m_Storage.begin(), m_Front);
	m_Front = 0;
    }
    m_Storage.push_back (v);
}

/// Pops the topmost element from the queue.
template <typename T>
inline void queue<T>::pop (void)
{
    if (++m_Front >= m_Storage.size())
	m_Storage.resize (m_Front = 0);
}

} // namespace ustl
