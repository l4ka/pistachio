#ifndef __LIBKERN_H__
#define __LIBKERN_H__

#include <l4/types.h>

#include <stddef.h>

/* Export bzero() */
void	 bzero(void *, size_t);

//Export memcmp()
int	 memcmp(const void *, const void *, size_t);

#endif /* __LIBKERN_H__ */
