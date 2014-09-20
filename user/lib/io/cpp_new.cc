#include <stddef.h>
#include <stdlib.h>
#include <l4e/misc.h>
 
 void *operator new(size_t size)
{
    return calloc(size, l4e_min_pagesize();
}
 
void *operator new[](size_t size)
{
    return calloc(size, l4e_min_pagesize();
}
 
 void operator delete(void *p)
{
    cfree(p);
}
 
void operator delete[](void *p)
{
    cfree(p);
}
