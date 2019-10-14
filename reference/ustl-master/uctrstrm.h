// This file is part of the uSTL library, an STL implementation.
//
// Copyright (c) 2005 by Mike Sharov <msharov@users.sourceforge.net>
// This file is free software, distributed under the MIT License.
//
/// \file uctrstrm.h
///
/// \brief Serialization templates for standard containers.
/// Because containers are templates, a single operator>> is impossible.
/// Making virtual read/write is also impossible because not all containers
/// contain serializable elements. Therefore, use the macros in this file.

#pragma once
#include "mistream.h"
#include "sostream.h"
#include "uiosfunc.h"
#include "typeinfo.h"

namespace ustl {

//----------------------------------------------------------------------
// Macros for easily declaring a container streamable.
//----------------------------------------------------------------------

/// \brief Declares container template \p type streamable.
///
/// Use TEMPLATE_TYPE and TEMPLATE_DECL macros to pass in templated
/// type with commas and the template declaration.
///
#define STD_TEMPLATE_CTR_STREAMABLE(type, template_decl)	\
    template_decl						\
    inline istream& operator>> (istream& is, type& v)		\
    { return (container_read (is, v)); } 			\
    template_decl						\
    inline ostream& operator<< (ostream& os, const type& v)	\
    { return (container_write (os, v)); } 			\
    template_decl						\
    inline ostringstream& operator<< (ostringstream& os, const type& v)	\
    { return (container_text_write (os, v)); }			\
    template_decl						\
    struct object_stream_size<type > {				\
	inline size_t operator()(const type& v)	const		\
	    { return (container_stream_size (v)); }		\
    };

/// \brief Declares non-resizable container template \p type streamable.
#define STD_TEMPLATE_NR_CTR_STREAMABLE(type, template_decl)	\
    template_decl						\
    inline istream& operator>> (istream& is, type& v)		\
    { return (nr_container_read (is, v)); } 			\
    template_decl						\
    inline ostream& operator<< (ostream& os, const type& v)	\
    { return (nr_container_write (os, v)); } 			\
    template_decl						\
    inline ostringstream& operator<< (ostringstream& os, const type& v)	\
    { return (container_text_write (os, v)); }			\
    template_decl						\
    struct object_stream_size<type > {				\
	inline size_t operator()(const type& v)	const		\
	    { return (nr_container_stream_size (v)); }		\
    };

//----------------------------------------------------------------------
// Fixed size container serialization.
//----------------------------------------------------------------------

/// Reads fixed size container \p v from stream \p is.
template <typename Container>
inline istream& nr_container_read (istream& is, Container& v)
{
    foreach (typename Container::iterator, i, v)
	is >> *i;
    return (is);
}

/// Writes fixed size container \p v into stream \p os.
template <typename Container>
inline ostream& nr_container_write (ostream& os, const Container& v)
{
    foreach (typename Container::const_iterator, i, v)
	os << *i;
    return (os);
}

/// Computes the stream size of a fixed size standard container.
template <typename Container>
inline size_t nr_container_stream_size (const Container& v)
{
    typedef typename Container::const_iterator vciter_t;
    if (!v.size())
	return (0);
    size_t s = 0, dvs;
    vciter_t i = v.begin();
    do {
	dvs = stream_size_of(*i);
	s += dvs;
    } while (++i != v.end() && !__builtin_constant_p(dvs));
    if (__builtin_constant_p(dvs))
	s *= v.size();
    return (s);
}

//----------------------------------------------------------------------
// Resizable container serialization.
//----------------------------------------------------------------------

/// Reads container \p v from stream \p is.
template <typename Container>
istream& container_read (istream& is, Container& v)
{
    typedef typename Container::value_type value_type;
    typedef typename Container::written_size_type written_size_type;
    written_size_type n = 0;
    is >> n;
    const size_t expectedSize = n * stream_size_of(value_type());
    if (!is.verify_remaining ("read", typeid(v).name(), expectedSize))
	return (is);
    if (stream_align_of(NullValue<value_type>()) > stream_align_of(n))
	is >> ios::talign<value_type>();
    v.resize (n);
    nr_container_read (is, v);
    is >> ios::talign<written_size_type>();
    return (is);
}

/// Writes the vector to stream \p os.
template <typename Container>
ostream& container_write (ostream& os, const Container& v)
{
    typedef typename Container::value_type value_type;
    typedef typename Container::written_size_type written_size_type;
    const written_size_type sz (v.size());
    os << sz;
    if (stream_align_of(NullValue<value_type>()) > stream_align_of(sz))
	os << ios::talign<value_type>();
    nr_container_write (os, v);
    os << ios::talign<written_size_type>();
    return (os);
}

/// Computes the stream size of a standard container.
template <typename Container>
size_t container_stream_size (const Container& v)
{
    typedef typename Container::value_type value_type;
    typedef typename Container::written_size_type written_size_type;
    const written_size_type sz (v.size());
    size_t sizeSize = stream_size_of (sz);
    if (stream_align_of(NullValue<value_type>()) > stream_align_of(sz))
	sizeSize = Align (sizeSize, stream_align_of(NullValue<value_type>()));
    return (Align (sizeSize + nr_container_stream_size (v), stream_align_of(sz)));
}

/// \brief Writes element \p v into stream \p os as text.
/// Specialize to custom print elements.
template <typename T>
inline ostringstream& container_element_text_write (ostringstream& os, const T& v)
{ return (os << v); }

/// Writes container \p v into stream \p os as text.
template <typename Container>
ostringstream& container_text_write (ostringstream& os, const Container& v)
{
    typename Container::const_iterator i = v.begin();
    os << '(';
    while (i < v.end()) {
	container_element_text_write (os, *i);
	os << ",)"[++i == v.end()];
    }
    return (os);
}

//----------------------------------------------------------------------

} // namespace ustl
