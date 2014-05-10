/*
if (!(condition)) { fprintf(stderr, "Assertion failed in %s:%d: %s", __FILE__, __LINE__, #condition); abort(); }
(9:21:35 PM) klange: ah, mine is #define assert(statement) ((statement) ? (void)0 : assert_failed(__FILE__, __LINE__, #statement))
*/

//#define assert(statement) ((statement) ? (void)0 : assert_failed(__FILE__, __LINE__, #statement))

#define assert (!(condition)) { printf ("Assertion failed in %s:%d: %s", __FILE__, __LINE__, #condition);  }



