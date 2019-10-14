#include <sys/utsname.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

//struct utsname {
   // char	sysname[_SYS_NMLN];	/* Name of this OS. */
   // char	nodename[_SYS_NMLN];	/* Name of this network node. */
   // char	release[_SYS_NMLN];	/* Release level. */
   // char	version[_SYS_NMLN];	/* Version level. */
  //  char	machine[_SYS_NMLN];	/* Hardware type. */
//};

//https://github.com/klange/toaruos/blob/94c976a903181df453f39d2a264f85c9a7c90246/kernel/sys/syscall.c
int	uname(struct utsname *aUtsName) {

        strcpy(aUtsName->sysname, "Enryo");
        strcpy(aUtsName->nodename,"noname");
        strcpy(aUtsName->release, "0");
        strcpy(aUtsName->version, "0");
        strcpy(aUtsName->machine, "Intel-Pentium2-only");
        strcpy(aUtsName->domainname, "noname.tld");

    return 0;
    }

#ifdef __cplusplus
}
#endif

