#include <l4/types.h>

#include <l4/thread.h>

#include <l4/kip.h>

#include <l4io.h>

void hello() {

printf("%x",L4_SystemClock().raw);
}
