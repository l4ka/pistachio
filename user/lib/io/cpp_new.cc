#include <stddef.h>
#include <liballoc.h>

 
extern "C" void *operator new(size_t size)
{
    return malloc(size);
}
 
extern "C" void *operator new[](size_t size)
{
    return malloc(size);
}
 
extern "C" void operator delete(void *p)
{
    free(p);
}
 
extern "C" void operator delete[](void *p)
{
    free(p);
}
