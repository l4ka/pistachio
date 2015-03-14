// This file is part of the uSTL library, an STL implementation.
//
// Copyright (c) 2005 by Mike Sharov <msharov@users.sourceforge.net>
// This file is free software, distributed under the MIT License.

#include "ustdxept.h"
#include "mistream.h"
#include "mostream.h"
#include "strmsize.h"
#include "uiosfunc.h"
#include "uspecial.h"

namespace ustl {

//----------------------------------------------------------------------

/// \p arg contains a description of the error.
error_message::error_message (const char* arg) noexcept
: m_Arg ()
{
    try { m_Arg = arg; } catch (...) {}
    set_format (xfmt_ErrorMessage);
}

/// Virtual destructor
error_message::~error_message (void) noexcept
{
}

/// Returns a descriptive error message. fmt="%s: %s"
void error_message::info (string& msgbuf, const char* fmt) const noexcept
{
    if (!fmt) fmt = "%s: %s";
    try { msgbuf.format (fmt, name(), m_Arg.cdata()); } catch (...) {}
}

/// Reads the object from stream \p is.
void error_message::read (istream& is)
{
    exception::read (is);
    is >> m_Arg >> ios::align();
}

/// Writes the object to stream \p os.
void error_message::write (ostream& os) const
{
    exception::write (os);
    os << m_Arg << ios::align();
}

/// Returns the size of the written object.
size_t error_message::stream_size (void) const noexcept
{
    return (exception::stream_size() + Align (stream_size_of (m_Arg)));
}

} // namespace ustl
