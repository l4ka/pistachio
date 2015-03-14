// This file is part of the uSTL library, an STL implementation.
//
// Copyright (c) 2006 by Mike Sharov <msharov@users.sourceforge.net>
// This file is free software, distributed under the MIT License.

#include "bktrace.h"
#include "sostream.h"
#include "mistream.h"
#if HAVE_EXECINFO_H
    #include <execinfo.h>
#else
    static inline int backtrace (void**, int)			{ return (0); }
    static inline char** backtrace_symbols (void* const*, int)	{ return (NULL); }
#endif

namespace ustl {

/// Default constructor. The backtrace is obtained here.
CBacktrace::CBacktrace (void) noexcept
: m_Symbols (NULL),
  m_nFrames (0),
  m_SymbolsSize (0)
{
    m_nFrames = backtrace (VectorBlock (m_Addresses));
    GetSymbols();
}

/// Copy constructor.
CBacktrace::CBacktrace (const CBacktrace& v) noexcept
: m_Symbols (NULL),
  m_nFrames (0),
  m_SymbolsSize (0)
{
    operator= (v);
}

/// Copy operator.
const CBacktrace& CBacktrace::operator= (const CBacktrace& v) noexcept
{
    memcpy (m_Addresses, v.m_Addresses, sizeof(m_Addresses));
    m_Symbols = strdup (v.m_Symbols);
    m_nFrames = v.m_nFrames;
    m_SymbolsSize = v.m_SymbolsSize;
    return (*this);
}

/// Converts a string returned by backtrace_symbols into readable form.
static size_t ExtractAbiName (const char* isym, char* nmbuf) noexcept
{
    // Prepare the demangled name, if possible
    size_t nmSize = 0;
    if (isym) {
	// Copy out the name; the strings are: "file(function+0x42) [0xAddress]"
	const char* mnStart = strchr (isym, '(');
	if (++mnStart == (const char*)(1))
	    mnStart = isym;
	const char* mnEnd = strchr (isym, '+');
	const char* isymEnd = isym + strlen (isym);
	if (!mnEnd)
	    mnEnd = isymEnd;
	nmSize = min (size_t (distance (mnStart, mnEnd)), 255U);
	memcpy (nmbuf, mnStart, nmSize);
    }
    nmbuf[nmSize] = 0;
    // Demangle
    demangle_type_name (nmbuf, 255U, &nmSize);
    return (nmSize);
}

/// Tries to get symbol information for the addresses.
void CBacktrace::GetSymbols (void) noexcept
{
    char** symbols = backtrace_symbols (m_Addresses, m_nFrames);
    if (!symbols)
	return;
    char nmbuf [256];
    size_t symSize = 1;
    for (uoff_t i = 0; i < m_nFrames; ++ i)
	symSize += ExtractAbiName (symbols[i], nmbuf) + 1;
    if ((m_Symbols = (char*) calloc (symSize, 1))) {
	for (uoff_t i = 0; m_SymbolsSize < symSize - 1; ++ i) {
	    size_t sz = ExtractAbiName (symbols[i], nmbuf);
	    memcpy (m_Symbols + m_SymbolsSize, nmbuf, sz);
	    m_SymbolsSize += sz + 1;
	    m_Symbols [m_SymbolsSize - 1] = '\n';
	}
    }
    free (symbols);
}

#if SIZE_OF_LONG == 8
    #define ADDRESS_FMT	"%16p  "
#else
    #define ADDRESS_FMT	"%8p  "
#endif

/// Prints the backtrace to \p os.
void CBacktrace::text_write (ostringstream& os) const
{
    const char *ss = m_Symbols, *se;
    for (uoff_t i = 0; i < m_nFrames; ++ i) {
	os.format (ADDRESS_FMT, m_Addresses[i]);
	se = strchr (ss, '\n') + 1;
	os.write (ss, distance (ss, se));
	ss = se;
    }
}

/// Reads the object from stream \p is.
void CBacktrace::read (istream& is)
{
    assert (is.aligned (stream_align_of (m_Addresses[0])) && "Backtrace object contains pointers and must be void* aligned");
    is >> m_nFrames >> m_SymbolsSize;
    nfree (m_Symbols);
    m_Symbols = (char*) malloc (m_SymbolsSize + 1);
    is.read (m_Symbols, m_SymbolsSize);
    m_Symbols [m_SymbolsSize] = 0;
    is.align();
    is.read (m_Addresses, m_nFrames * sizeof(void*));
}

/// Writes the object to stream \p os.
void CBacktrace::write (ostream& os) const
{
    assert (os.aligned (stream_align_of (m_Addresses[0])) && "Backtrace object contains pointers and must be void* aligned");
    os << m_nFrames << m_SymbolsSize;
    os.write (m_Symbols, m_SymbolsSize);
    os.align();
    os.write (m_Addresses, m_nFrames * sizeof(void*));
}

/// Returns the size of the written object.
size_t CBacktrace::stream_size (void) const
{
    return (Align (stream_size_of (m_nFrames) +
		   stream_size_of (m_SymbolsSize) +
		   m_nFrames * sizeof(void*) +
		   m_SymbolsSize));
}

} // namespace ustl
