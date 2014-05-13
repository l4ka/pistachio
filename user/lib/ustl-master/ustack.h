// This file is part of the uSTL library, an STL implementation.
//
// Copyright (c) 2005 by Mike Sharov <msharov@users.sourceforge.net>
// This file is free software, distributed under the MIT License.

#pragma once

namespace ustl {

/// \class stack ustack.h ustl.h
/// \ingroup Sequences
///
/// \brief Stack adapter to uSTL containers.
///
template <typename T>
class stack {
public:
    typedef T			value_type;
    typedef size_t		size_type;
    typedef ptrdiff_t		difference_type;
    typedef T&			reference;
    typedef const T&		const_reference;
    typedef T*			pointer;
    typedef const T*		const_pointer;
public:
    inline			stack (void)			: m_Storage () { }
    explicit inline		stack (const vector<T>& s)	: m_Storage (s) { }
    explicit inline		stack (const stack& s)		: m_Storage (s.m_Storage) { }
    inline bool			empty (void) const		{ return (m_Storage.empty()); }
    inline size_type		size (void) const		{ return (m_Storage.size()); }
    inline reference		top (void)			{ return (m_Storage.back()); }
    inline const_reference	top (void) const		{ return (m_Storage.back()); }
    inline void			push (const value_type& v)	{ m_Storage.push_back (v); }
    inline void			pop (void)			{ m_Storage.pop_back(); }
    inline bool			operator== (const stack& s) const	{ return (m_Storage == s.m_Storage); }
    inline bool			operator< (const stack& s) const	{ return (m_Storage.size() < s.m_Storage.size()); }
private:
    vector<T>			m_Storage;	///< Where the data actually is.
};

} // namespace ustl
