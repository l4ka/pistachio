/*

These are defined at http://pubs.opengroup.org/onlinepubs/9699919799/basedefs/dlfcn.h.html#tag_13_09 (dlfcn.h).

We only define the stub implementation in this file.

int    dlclose(void *);
char  *dlerror(void);
void  *dlopen(const char *, int);
void  *dlsym(void *restrict, const char *restrict);
*/

int dlclose (void *) {
	return -1; //not implemented
}

char *dlerror(void) {
	return 0; //not implemented
}

void *dlopen(const char *, int) {
}

void *dlsym(void *restrict, const char *restrict) {
}



