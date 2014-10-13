#pragma once

//http://sourceforge.jp/projects/openbsd-octeon/scm/git/openbsd-octeon/blobs/master/src/sys/arch/i386/include/float.h

int
__flt_rounds(void);


#define FLT_ROUNDS      __flt_rounds()
#define FLT_RADIX       2               /* b */

#define DBL_DIG         15

 #define DBL_MAX_10_EXP  308

#define FLT_MIN_EXP     (-125)          /* emin */
#define LDBL_MIN_EXP    (-16381)
#define DBL_MAX_EXP     1024
#define DBL_MANT_DIG    53
#define DBL_MAX         1.7976931348623157E+308
#define LDBL_MANT_DIG   64
#define LDBL_MAX_EXP    16384
