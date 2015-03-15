#include <sys/sysinfo.h>
#include <string.h>
#include <l4/kip.h>

#ifdef __cplusplus
extern "C" {
#endif



//           struct sysinfo {
//               long uptime;             /* Seconds since boot */
//               unsigned long loads[3];  /* 1, 5, and 15 minute load averages */
//               unsigned long totalram;  /* Total usable main memory size */
//               unsigned long freeram;   /* Available memory size */
//               unsigned long sharedram; /* Amount of shared memory */
//               unsigned long bufferram; /* Memory used by buffers */
//               unsigned long totalswap; /* Total swap space size */
//               unsigned long freeswap;  /* swap space still available */
//               unsigned short procs;    /* Number of current processes */
//               unsigned long totalhigh; /* Total high memory size */
//               unsigned long freehigh;  /* Available high memory size */
//               unsigned int mem_unit;   /* Memory unit size in bytes */
//               char _f[20-2*sizeof(long)-sizeof(int)]; /* Padding to 64 bytes */
//           };

//Return the number of available CPUs, according to the KIP/SMBIOS
int get_nprocs() {
        L4_Word_t apiv, apif, kid;
	L4_KernelInterfacePage_t * kip;
	kip = (L4_KernelInterfacePage_t *) L4_KernelInterface( &apiv, &apif, &kid );
	
	return L4_NumProcessors(kip);
}

//Initialise the sysinfo structure...
//https://github.com/klange/toaruos/blob/94c976a903181df453f39d2a264f85c9a7c90246/kernel/sys/syscall.c
int	sysinfo(struct sysinfo *aSysInfo) {

        L4_Word_t apiv, apif, kid;
	L4_KernelInterfacePage_t * kip;
	kip = (L4_KernelInterfacePage_t *) L4_KernelInterface( &apiv, &apif, &kid );

//        strcpy(aUtsName->sysname, "Enryo");
//        strcpy(aUtsName->nodename,"noname");
//        strcpy(aUtsName->release, "0");
//        strcpy(aUtsName->version, "0");
//        strcpy(aUtsName->machine, "Intel-Pentium2-only");
//        strcpy(aUtsName->domainname, "noname.tld");

    return -1; //We're unimplemented for now
    }

#ifdef __cplusplus
}
#endif

