#define NL_TEXTMAX	255

//http://gitweb.dragonflybsd.org/dragonfly.git/blob/c8dd1ae615f4c3a6042727e69e5c05434f781224:/sys/cpu/i386/include/limits.h

#define ULONG_MAX       0xffffffffUL    /* max value for an unsigned long */

#define ULLONG_MAX      0xffffffffffffffffULL
#define UQUAD_MAX       ULLONG_MAX      /* max value for a uquad_t */

#define LLONG_MIN       (-0x7fffffffffffffffLL - 1)  /* min for a long long */
#define QUAD_MIN        LLONG_MIN

#define LLONG_MAX       0x7fffffffffffffffLL    /* max value for a long long */
#define QUAD_MAX        LLONG_MAX       /* max value for a quad_t */

#define INT_MAX         0x7fffffff      /* max value for an int */
