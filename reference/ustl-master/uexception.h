// This file is part of the uSTL library, an STL implementation.
//
// Copyright (c) 2005 by Mike Sharov <msharov@users.sourceforge.net>
// This file is free software, distributed under the MIT License.

#pragma once
#include "utypes.h"
#ifndef WITHOUT_LIBSTDCPP
    #include <exception>
    #include <new>
#endif
#include "bktrace.h"

#if WITHOUT_LIBSTDCPP
namespace std {
/// If you write a replacement terminate handler, it must be of this type.
typedef void (*terminate_handler) (void);
/// If you write a replacement unexpected handler, it must be of this type.
typedef void (*unexpected_handler) (void);
/// Takes a new handler function as an argument, returns the old function.
terminate_handler set_terminate (terminate_handler pHandler) noexcept;
/// The runtime will call this function if exception handling must be
/// abandoned for any reason.  It can also be called by the user.
void terminate (void) noexcept __attribute__ ((__noreturn__));
/// Takes a new handler function as an argument, returns the old function.
unexpected_handler set_unexpected (unexpected_handler pHandler) noexcept;
/// The runtime will call this function if an exception is thrown which
/// violates the function's exception specification.
void unexpected (void) __attribute__ ((__noreturn__));
/// Returns true when the caught exception violates the throw specification.
bool uncaught_exception() noexcept;
} // namespace std
#endif

namespace ustl {

class string;

typedef uint32_t	xfmt_t;

enum {
    xfmt_Exception,
    xfmt_BadAlloc,
    xfmt_LibcException		= 12,
    xfmt_FileException		= 13,
    xfmt_StreamBoundsException	= 14
};

/// \class exception uexception.h ustl.h
/// \ingroup Exceptions
///
/// \brief Base class for exceptions, equivalent to std::exception.
///
#if WITHOUT_LIBSTDCPP
class exception {
#else
class exception : public std::exception {
#endif
public:
    typedef const CBacktrace& rcbktrace_t;
public:
    inline		exception (void) noexcept : m_Format (xfmt_Exception) {}
    inline virtual     ~exception (void) noexcept {}
    inline virtual const char* what (void) const noexcept { return ("error"); }
    virtual void	info (string& msgbuf, const char* fmt = NULL) const noexcept;
    virtual void	read (istream& is);
    virtual void	write (ostream& os) const;
    void		text_write (ostringstream& os) const noexcept;
    inline virtual size_t stream_size (void) const noexcept { return (sizeof(m_Format) + sizeof(uint32_t) + m_Backtrace.stream_size()); }
    /// Format of the exception is used to lookup exception::info format string.
    /// Another common use is the instantiation of serialized exceptions, used
    /// by the error handler node chain to troubleshoot specific errors.
    inline xfmt_t	format (void) const	{ return (m_Format); }
    inline rcbktrace_t	backtrace (void) const	{ return (m_Backtrace); }
protected:
    inline void		set_format (xfmt_t fmt) { m_Format = fmt; }
private:
    CBacktrace		m_Backtrace;	///< Backtrace of the throw point.
    xfmt_t		m_Format;	///< Format of the exception's data.
};

/// \class bad_cast uexception.h ustl.h
/// \ingroup Exceptions
///
/// \brief Thrown to indicate a bad dynamic_cast usage.
///
class bad_cast : public exception {
public:
    inline 			bad_cast (void) noexcept		: exception() {}
    inline virtual const char*	what (void) const noexcept	{ return ("bad cast"); }
};

class bad_typeid : public exception {
public:
    inline			bad_typeid (void) noexcept	{ }
    inline virtual const char*	what (void) const noexcept	{ return ("bad typeid"); }
};

//----------------------------------------------------------------------

/// \class bad_alloc uexception.h ustl.h
/// \ingroup Exceptions
///
/// \brief Exception thrown on memory allocation failure by memblock::reserve.
///
#if WITHOUT_LIBSTDCPP
} // namespace ustl
namespace std {
class bad_alloc : public ::ustl::exception {
#else

class bad_alloc : public std::bad_alloc, public exception {
#endif
public:
    explicit		bad_alloc (size_t nBytes = 0) noexcept;
    inline virtual const char*	what (void) const noexcept { return ("memory allocation failed"); }
    virtual void	info (ustl::string& msgbuf, const char* fmt = NULL) const noexcept;
    virtual void	read (ustl::istream& is);
    virtual void	write (ustl::ostream& os) const;
    virtual size_t	stream_size (void) const noexcept;
protected:
    size_t		m_nBytesRequested;	///< Number of bytes requested by the failed allocation.
};

#if WITHOUT_LIBSTDCPP
} // namespace std
namespace ustl {
typedef std::bad_alloc bad_alloc;
#endif

/// \class libc_exception uexception.h ustl.h
/// \ingroup Exceptions
///
/// \brief Thrown when a libc function returns an error.
///
/// Contains an errno and description. This is a uSTL extension.
///
class libc_exception : public exception {
public:
    explicit		libc_exception (const char* operation) noexcept;
			libc_exception (const libc_exception& v) noexcept;
    const libc_exception& operator= (const libc_exception& v);
    inline virtual const char*	what (void) const noexcept { return ("libc function failed"); }
    virtual void	info (string& msgbuf, const char* fmt = NULL) const noexcept;
    virtual void	read (istream& is);
    virtual void	write (ostream& os) const;
    virtual size_t	stream_size (void) const noexcept;
protected:
    intptr_t		m_Errno;		///< Error code returned by the failed operation.
    const char*		m_Operation;		///< Name of the failed operation.
};

/// \class file_exception uexception.h ustl.h
/// \ingroup Exceptions
///
/// \brief File-related exceptions.
///
/// Contains the file name. This is a uSTL extension.
///
class file_exception : public libc_exception {
public:
			file_exception (const char* operation, const char* filename) noexcept;
    inline virtual const char* what (void) const noexcept { return ("file error"); }
    virtual void	info (string& msgbuf, const char* fmt = NULL) const noexcept;
    virtual void	read (istream& is);
    virtual void	write (ostream& os) const;
    virtual size_t	stream_size (void) const noexcept;
protected:
    char		m_Filename [PATH_MAX];	///< Name of the file causing the error.
};

/// \class stream_bounds_exception uexception.h ustl.h
/// \ingroup Exceptions
///
/// \brief Stream bounds checking.
///
/// Only thrown in debug builds unless you say otherwise in config.h
/// This is a uSTL extension.
///
class stream_bounds_exception : public libc_exception {
public:
			stream_bounds_exception (const char* operation, const char* type, uoff_t offset, size_t expected, size_t remaining) noexcept;
    inline virtual const char*	what (void) const noexcept { return ("stream bounds exception"); }
    virtual void	info (string& msgbuf, const char* fmt = NULL) const noexcept;
    virtual void	read (istream& is);
    virtual void	write (ostream& os) const;
    virtual size_t	stream_size (void) const noexcept;
protected:
    const char*		m_TypeName;
    uoff_t		m_Offset;
    size_t		m_Expected;
    size_t		m_Remaining;
};

const char* demangle_type_name (char* buf, size_t bufSize, size_t* pdmSize = NULL) noexcept;

} // namespace ustl
