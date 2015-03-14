//sysinfo is probably a Linuxism, but some third-party software assumes its availability

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

//This is taken from "man sysinfo"...
//Since Linux 2.3.23 (i386), 2.3.48 (all architectures) the structure is:
           struct sysinfo {
               long uptime;             /* Seconds since boot */
               unsigned long loads[3];  /* 1, 5, and 15 minute load averages */
               unsigned long totalram;  /* Total usable main memory size */
               unsigned long freeram;   /* Available memory size */
               unsigned long sharedram; /* Amount of shared memory */
               unsigned long bufferram; /* Memory used by buffers */
               unsigned long totalswap; /* Total swap space size */
               unsigned long freeswap;  /* swap space still available */
               unsigned short procs;    /* Number of current processes */
               unsigned long totalhigh; /* Total high memory size */
               unsigned long freehigh;  /* Available high memory size */
               unsigned int mem_unit;   /* Memory unit size in bytes */
               char _f[20-2*sizeof(long)-sizeof(int)]; /* Padding to 64 bytes */
           };

//Sadly, for compatibility purposes, a clean-room implementation would have to share the same variable names

//Populate the members of the struct
int sysinfo(struct sysinfo *aSysInfo);

#ifdef __cplusplus

}
#endif
