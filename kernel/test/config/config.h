/* Automatically generated, don't edit */
/* Generated on: tyson-Lenovo-ideapad-120S-14IAP */
/* At: Mon, 14 Oct 2019 22:07:00 +0000 */
/* Linux version 5.0.0-31-generic (buildd@lcy01-amd64-010) (gcc version 8.3.0 (Ubuntu 8.3.0-6ubuntu1)) #33-Ubuntu SMP Mon Sep 30 18:51:59 UTC 2019 */

/* Pistachio Kernel Configuration System */

/* Hardware */

/* Basic Architecture */
#define CONFIG_ARCH_X86 1
#undef  CONFIG_ARCH_POWERPC
#undef  CONFIG_ARCH_POWERPC64


/* X86 Processor Architecture */
#undef  CONFIG_SUBARCH_X32
#define CONFIG_SUBARCH_X64 1


/* Processor Type */
#undef  CONFIG_CPU_X86_I486
#undef  CONFIG_CPU_X86_I586
#undef  CONFIG_CPU_X86_I686
#define CONFIG_CPU_X86_P4 1
#undef  CONFIG_CPU_X86_K8
#undef  CONFIG_CPU_X86_C3
#undef  CONFIG_CPU_X86_SIMICS


/* Platform */
#define CONFIG_PLAT_PC99 1
#undef  CONFIG_PLAT_EFI


/* Miscellaneous */
#define CONFIG_IOAPIC 1
#define CONFIG_MAX_IOAPICS 2
#define CONFIG_APIC_TIMER_TICK 1000

#undef  CONFIG_SMP


/* Kernel */
#undef  CONFIG_EXPERIMENTAL
#undef  CONFIG_IPC_FASTPATH
#define CONFIG_DEBUG 1
#undef  CONFIG_DEBUG_SYMBOLS
#undef  CONFIG_PERFMON
#undef  CONFIG_SPIN_WHEELS
#undef  CONFIG_NEW_MDB
#undef  CONFIG_STATIC_TCBS
#define CONFIG_X86_COMPATIBILITY_MODE 1


/* Debugger */

/* Kernel Debugger Console */
#define CONFIG_KDB_CONS_COM 1
#define CONFIG_KDB_COMPORT 0x0
#define CONFIG_KDB_COMSPEED 115200
#undef  CONFIG_KDB_CONS_KBD
#define CONFIG_KDB_BOOT_CONS 0

#undef  CONFIG_KDB_DISAS
#undef  CONFIG_KDB_ON_STARTUP
#undef  CONFIG_KDB_BREAKIN
#undef  CONFIG_KDB_INPUT_HLT
#undef  CONFIG_KDB_NO_ASSERTS

/* Trace Settings */
#undef  CONFIG_VERBOSE_INIT
#undef  CONFIG_TRACEPOINTS
#undef  CONFIG_KMEM_TRACE
#undef  CONFIG_TRACEBUFFER



/* Code Generator Options */


/* Derived symbols */
#undef  CONFIG_HAVE_MEMORY_CONTROL
#define CONFIG_X86_PSE 1
#undef  CONFIG_BIGENDIAN
#undef  CONFIG_PPC_MMU_TLB
#define CONFIG_X86_SYSENTER 1
#define CONFIG_X86_PGE 1
#define CONFIG_X86_FXSR 1
#undef  CONFIG_IS_32BIT
#define CONFIG_X86_HTT 1
#define CONFIG_X86_PAT 1
#undef  CONFIG_PPC_BOOKE
#define CONFIG_IS_64BIT 1
#define CONFIG_MULTI_ARCHITECTURE 1
#define CONFIG_X86_EM64T 1
#undef  CONFIG_PPC_CACHE_L1_WRITETHROUGH
#undef  CONFIG_PPC_TLB_INV_LOCAL
#undef  CONFIG_PPC_CACHE_ICBI_LOCAL
#undef  CONFIG_X86_SMALL_SPACES_GLOBAL
#define CONFIG_X86_HVM 1
#undef  CONFIG_PPC_MMU_SEGMENTS
#define CONFIG_X86_TSC 1
/* That's all, folks! */
#define AUTOCONF_INCLUDED
