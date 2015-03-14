// This file is part of the uSTL library, an STL implementation.
//
// Copyright (c) 2005 by Mike Sharov <msharov@users.sourceforge.net>
// This file is free software, distributed under the MIT License.

#pragma once
#ifndef WITHOUT_LIBSTDCPP
    #include <typeinfo>
#else
#include "uexception.h"

namespace __cxxabiv1 { class __class_type_info; }

namespace std {
class type_info {
public:
    inline virtual	~type_info (void)			{ }
    inline const char*	name (void) const			{ return (__name[0] == '*' ? __name + 1 : __name); }
    inline bool		before (const type_info& v) const	{ return (__name < v.__name); }
    inline bool		operator==(const type_info& v) const	{ return (__name == v.__name); }
    inline bool		operator!=(const type_info& v) const	{ return (!operator==(v)); }
    virtual bool	__is_pointer_p (void) const;
    virtual bool	__is_function_p (void) const;
    virtual bool	__do_catch (const type_info* __thr_type, void** __thr_obj, unsigned __outer) const;
    virtual bool	__do_upcast (const __cxxabiv1::__class_type_info* __target, void** __obj_ptr) const;
    explicit inline	type_info (const char* newname)		: __name(newname) { }
private:
    inline void		operator= (const type_info&)		{ }
    inline		type_info (const type_info&)		{ }
protected:
    const char*		__name;
};

} // namespace std

#endif
