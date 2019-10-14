// This file is part of the uSTL library, an STL implementation.
//
// Copyright (c) 2005 by Mike Sharov <msharov@users.sourceforge.net>
// This file is free software, distributed under the MIT License.

#pragma once
#include "uexception.h"
#include "ustring.h"

namespace ustl {

enum { 
    xfmt_ErrorMessage = 2,
    xfmt_LogicError = xfmt_ErrorMessage,
    xfmt_RuntimeError = xfmt_ErrorMessage
};

/// \class logic_error ustdxept.h ustl.h
/// \ingroup Exceptions
///
/// \brief Logic errors represent problems in the internal logic of the program.
///
class error_message : public exception {
public:
    explicit		error_message (const char* arg) noexcept;
    virtual	       ~error_message (void) noexcept;
    inline virtual const char*	what (void) const noexcept { return (m_Arg.c_str()); }
    inline virtual const char*	name (void) const noexcept { return ("error"); }
    virtual void	info (string& msgbuf, const char* fmt = NULL) const noexcept;
    virtual void	read (istream& is);
    virtual void	write (ostream& os) const;
    virtual size_t	stream_size (void) const noexcept;
protected:
    string		m_Arg;
};

/// \class logic_error ustdxept.h ustl.h
/// \ingroup Exceptions
///
/// \brief Logic errors represent problems in the internal logic of the program.
///
class logic_error : public error_message {
public:
    inline explicit		logic_error (const char* arg) noexcept : error_message (arg) {}
    inline virtual const char*	name (void) const noexcept { return ("logic error"); }
};

/// \class domain_error ustdxept.h ustl.h
/// \ingroup Exceptions
///
/// \brief Reports domain errors ("domain" is in the mathematical sense)
///
class domain_error : public logic_error {
public:
    inline explicit		domain_error (const char* arg) noexcept : logic_error (arg) {}
    inline virtual const char*	name (void) const noexcept { return ("domain error"); }
};

/// \class invalid_argument ustdxept.h ustl.h
/// \ingroup Exceptions
///
/// \brief Reports an invalid argument to a function.
///
class invalid_argument : public logic_error {
public:
    inline explicit		invalid_argument (const char* arg) noexcept : logic_error (arg) {}
    inline virtual const char*	name (void) const noexcept { return ("invalid argument"); }
};

/// \class length_error ustdxept.h ustl.h
/// \ingroup Exceptions
///
/// \brief Reports when an object exceeds its allowed size.
///
class length_error : public logic_error {
public:
    inline explicit		length_error (const char* arg) noexcept : logic_error (arg) {} 
    inline virtual const char*	name (void) const noexcept { return ("length error"); }
};

/// \class out_of_range ustdxept.h ustl.h
/// \ingroup Exceptions
///
/// \brief Reports arguments with values out of allowed range.
///
class out_of_range : public logic_error {
public:
    inline explicit		out_of_range (const char* arg) noexcept : logic_error (arg) {}
    inline virtual const char*	name (void) const noexcept { return ("out of range"); }
};

/// \class runtime_error ustdxept.h ustl.h
/// \ingroup Exceptions
///
/// \brief Reports errors that are dependent on the data being processed.
///
class runtime_error : public error_message {
public:
    inline explicit		runtime_error (const char* arg) noexcept : error_message (arg) {}
    inline virtual const char*	name (void) const noexcept { return ("runtime error"); }
};

/// \class range_error ustdxept.h ustl.h
/// \ingroup Exceptions
///
/// \brief Reports data that does not fall within the permitted range.
///
class range_error : public runtime_error {
public:
    inline explicit		range_error (const char* arg) noexcept : runtime_error (arg) {}
    inline virtual const char*	name (void) const noexcept { return ("range error"); }
};

/// \class overflow_error ustdxept.h ustl.h
/// \ingroup Exceptions
///
/// \brief Reports arithmetic overflow.
///
class overflow_error : public runtime_error {
public:
    inline explicit		overflow_error (const char* arg) noexcept : runtime_error (arg) {}
    inline virtual const char*	name (void) const noexcept { return ("overflow error"); }
};

/// \class underflow_error ustdxept.h ustl.h
/// \ingroup Exceptions
///
/// \brief Reports arithmetic underflow.
///
class underflow_error : public runtime_error {
public:
    inline explicit		underflow_error (const char* arg) noexcept : runtime_error (arg) {}
    inline virtual const char*	name (void) const noexcept { return ("underflow error"); }
};

} // namespace ustl
