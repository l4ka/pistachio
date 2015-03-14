// This file is part of the uSTL library, an STL implementation.
//
// Copyright (c) 2005 by Mike Sharov <msharov@users.sourceforge.net>
// This file is free software, distributed under the MIT License.

#pragma once
#include "mistream.h"
#include "ustring.h"
#ifndef EOF
#define EOF (-1)
#endif

namespace ustl {

/// \class istringstream sistream.h ustl.h
/// \ingroup TextStreams
///
/// \brief A stream that reads textual data from a memory block.
///
class istringstream : public istream {
public:
    static const size_type	c_MaxDelimiters = 16;	///< Maximum number of word delimiters.
public:
				istringstream (void) noexcept;
				istringstream (const void* p, size_type n) noexcept;
    explicit			istringstream (const cmemlink& source) noexcept;
    void			iread (int8_t& v)	{ v = skip_delimiters(); }
    void			iread (int32_t& v);
    void			iread (double& v);
    void			iread (bool& v);
    void			iread (wchar_t& v);
    void			iread (string& v);
#if HAVE_INT64_T
    void			iread (int64_t& v);
#endif
#if HAVE_LONG_LONG && (!HAVE_INT64_T || SIZE_OF_LONG_LONG > 8)
    void			iread (long long& v);
#endif
    inline string		str (void) const	{ string s; s.link (*this); return (s); }
    inline istringstream&	str (const string& s)	{ link (s); return (*this); }
    inline istringstream&	get (char& c)	{ return (read (&c, sizeof(c))); }
    inline int			get (void)	{ char c = EOF; get(c); return (c); }
    istringstream&		get (char* p, size_type n, char delim = '\n');
    istringstream&		get (string& s, char delim = '\n');
    istringstream&		getline (char* p, size_type n, char delim = '\n');
    istringstream&		getline (string& s, char delim = '\n');
    istringstream&		ignore (size_type n, char delim = '\0');
    inline char			peek (void)	{ int8_t v; iread (v); ungetc(); return (v); }
    inline istringstream&	putback (char)	{ ungetc(); return (*this); }
    inline istringstream&	unget (void)	{ ungetc(); return (*this); }
    inline void			set_delimiters (const char* delimiters);
    inline void			set_base (short base);
    inline void			set_decimal_separator (char)	{ }
    inline void			set_thousand_separator (char)	{ }
    istringstream&		read (void* buffer, size_type size);
    inline istringstream&	read (memlink& buf)		{ return (read (buf.begin(), buf.size())); }
    inline istringstream&	seekg (off_t p, seekdir d =beg)	{ istream::seekg(p,d); return (*this); }
    inline int			sync (void)			{ skip (remaining()); return (0); }
protected:
    char			skip_delimiters (void);
private:
    inline void			read_strz (string&)	{ assert (!"Reading nul characters is not allowed from text streams"); }
    inline bool			is_delimiter (char c) const noexcept;
    template <typename T> void	read_number (T& v);
private:
    char			m_Delimiters [c_MaxDelimiters];
    uint8_t			m_Base;
};

//----------------------------------------------------------------------

/// Sets the numeric base used to read numbers.
inline void istringstream::set_base (short base)
{
    m_Base = base;
}

/// Sets delimiters to the contents of \p delimiters.
inline void istringstream::set_delimiters (const char* delimiters)
{
#if (__i386__ || __x86_64__) && CPU_HAS_SSE && HAVE_VECTOR_EXTENSIONS
    typedef uint32_t v16ud_t __attribute__((vector_size(16)));
    asm("xorps\t%%xmm0, %%xmm0\n\tmovups\t%%xmm0, %0":"=m"(*noalias_cast<v16ud_t*>(m_Delimiters))::"xmm0");
#else
    memset (m_Delimiters, 0, sizeof(m_Delimiters));
#endif
    memcpy (m_Delimiters, delimiters, min (strlen(delimiters),sizeof(m_Delimiters)-1));
}

/// Reads one type as another.
template <typename RealT, typename CastT>
inline void _cast_read (istringstream& is, RealT& v)
{
    CastT cv;
    is.iread (cv);
    v = RealT (cv);
}

/// Reads a line of text from \p is into \p s
inline istringstream& getline (istringstream& is, string& s)
    { return (is.getline (s)); }

//----------------------------------------------------------------------

template <typename T> struct object_text_reader {
    inline void operator()(istringstream& is, T& v) const { v.text_read (is); }
};
template <typename T> struct integral_text_object_reader {
    inline void operator()(istringstream& is, T& v) const { is.iread (v); }
};
template <typename T>
inline istringstream& operator>> (istringstream& is, T& v) {
    typedef typename tm::Select <numeric_limits<T>::is_integral,
	integral_text_object_reader<T>, object_text_reader<T> >::Result object_reader_t;
    object_reader_t()(is, v);
    return (is);
}

//----------------------------------------------------------------------

template <> struct object_text_reader<string> {
    inline void operator()(istringstream& is, string& v) const { is.iread (v); }
};
#define ISTRSTREAM_CAST_OPERATOR(RealT, CastT)		\
template <> struct integral_text_object_reader<RealT> {	\
    inline void operator() (istringstream& is, RealT& v) const	\
	{ _cast_read<RealT,CastT>(is, v); }		\
};
ISTRSTREAM_CAST_OPERATOR (uint8_t,	int8_t)
ISTRSTREAM_CAST_OPERATOR (int16_t,	int32_t)
ISTRSTREAM_CAST_OPERATOR (uint16_t,	int32_t)
ISTRSTREAM_CAST_OPERATOR (uint32_t,	int32_t)
ISTRSTREAM_CAST_OPERATOR (float,	double)
#if HAVE_THREE_CHAR_TYPES
ISTRSTREAM_CAST_OPERATOR (char,		int8_t)
#endif
#if HAVE_INT64_T
ISTRSTREAM_CAST_OPERATOR (uint64_t,	int64_t)
#endif
#if SIZE_OF_LONG == SIZE_OF_INT
ISTRSTREAM_CAST_OPERATOR (long,		int)
ISTRSTREAM_CAST_OPERATOR (unsigned long,int)
#endif
#if HAVE_LONG_LONG && (!HAVE_INT64_T || SIZE_OF_LONG_LONG > 8)
ISTRSTREAM_CAST_OPERATOR (unsigned long long, long long)
#endif
#undef ISTRSTREAM_CAST_OPERATOR

} // namespace ustl
