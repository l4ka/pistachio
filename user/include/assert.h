/*
if (!(condition)) { fprintf(stderr, "Assertion failed in %s:%d: %s", __FILE__, __LINE__, #condition); abort(); }
(9:21:35 PM) klange: ah, mine is #define assert(statement) ((statement) ? (void)0 : assert_failed(__FILE__, __LINE__, #statement))
*/

//#define assert(statement) ((statement) ? (void)0 : assert_failed(__FILE__, __LINE__, #statement))

//#define assert (!(condition)) { printf ("Assertion failed in %s:%d: %s", __FILE__, __LINE__, #condition);  }

//
//#define assert(x)
//http://www.6809.net/tenk/?PG%E9%9B%91%E8%A8%98%2Fassert%E7%B3%BB%E3%83%9E%E3%82%AF%E3%83%AD%E9%96%A2%E4%BF%82%E3%81%AE%E3%83%A1%E3%83%A2
 #define assert(x)   do {                                           \
            if (!(x)) {                                             \
                printf("Assertion failed: %s, file %s, line %d\n"   \
                       , #x, __FILE__, __LINE__);                   \
                /*exit(1); */ /*abort();*/                               \
            }                                                       \
        } while (0)

