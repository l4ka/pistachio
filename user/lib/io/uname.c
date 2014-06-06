#include <sys/utsname.h>



//struct utsname {
   // char	sysname[_SYS_NMLN];	/* Name of this OS. */
   // char	nodename[_SYS_NMLN];	/* Name of this network node. */
   // char	release[_SYS_NMLN];	/* Release level. */
   // char	version[_SYS_NMLN];	/* Version level. */
  //  char	machine[_SYS_NMLN];	/* Hardware type. */
//};

//https://github.com/klange/toaruos/blob/94c976a903181df453f39d2a264f85c9a7c90246/kernel/sys/syscall.c

    int	uname(struct utsname *aUtsName) {

        /*
    strcpy(name->sysname,  __kernel_name);
    strcpy(name->nodename, hostname);
    strcpy(name->release,  version_number);
    strcpy(name->version,  version_string);
    strcpy(name->machine,  __kernel_arch);
    strcpy(name->domainname, "");*/

    return 0;
    }
