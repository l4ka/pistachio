/* Automatically generated, don't edit */
/* Generated on: tyson-VirtualBox */
/* At: Thu, 08 May 2014 01:33:39 +0000 */
/* Linux version 3.11.0-12-generic (buildd@komainu) (gcc version 4.8.1 (Ubuntu/Linaro 4.8.1-10ubuntu7) ) #19-Ubuntu SMP Wed Oct 9 16:12:00 UTC 2013 */

/* Pistachio Kernel Configuration System */

/* Hardware */

/* Basic Architecture */
#define CONFIG_ARCH_X86 1
#undef  CONFIG_ARCH_POWERPC
#undef  CONFIG_ARCH_POWERPC64


/* X86 Processor Architecture */
#define CONFIG_SUBARCH_X32 1
#undef  CONFIG_SUBARCH_X64


/* Processor Type */
#undef  CONFIG_CPU_X86_I486
#undef  CONFIG_CPU_X86_I586
#define CONFIG_CPU_X86_I686 1
#undef  CONFIG_CPU_X86_P4
#undef  CONFIG_CPU_X86_K8
#undef  CONFIG_CPU_X86_C3
#undef  CONFIG_CPU_X86_SIMICS


/* Platform */
#define CONFIG_PLAT_PC99 1


/* Miscellaneous */
#define CONFIG_IOAPIC 1
#define CONFIG_MAX_IOAPICS 2
#define CONFIG_APIC_TIMER_TICK 1000

#undef  CONFIG_SMP


/* Kernel */
#undef  CONFIG_EXPERIMENTAL

/* Experimental Features */
#undef  CONFIG_X_PAGER_EXREGS
#undef  CONFIG_X_CTRLXFER_MSG
#undef  CONFIG_X_EVT_LOGGING

/* Kernel scheduling policy */
#undef  CONFIG_SCHED_RR
#undef  CONFIG_X_SCHED_HS


#undef  CONFIG_IPC_FASTPATH
#define CONFIG_DEBUG 1
#undef  CONFIG_DEBUG_SYMBOLS
#undef  CONFIG_PERFMON
#define CONFIG_SPIN_WHEELS 1
#undef  CONFIG_NEW_MDB
#undef  CONFIG_STATIC_TCBS
#undef  CONFIG_X86_SMALL_SPACES


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
#define CONFIG_VERBOSE_INIT 1
#undef  CONFIG_TRACEPOINTS
#undef  CONFIG_KMEM_TRACE
#undef  CONFIG_TRACEBUFFER
#undef  CONFIG_X86_KEEP_LAST_BRANCHES



/* Code Generator Options */


/* Derived symbols */
#undef  CONFIG_HAVE_MEMORY_CONTROL
#define CONFIG_X86_PSE 1
#undef  CONFIG_BIGENDIAN
#undef  CONFIG_PPC_MMU_TLB
#define CONFIG_X86_SYSENTER 1
#define CONFIG_X86_PGE 1
#define CONFIG_X86_FXSR 1
#define CONFIG_IS_32BIT 1
#undef  CONFIG_X86_HTT
#define CONFIG_X86_PAT 1
#undef  CONFIG_PPC_BOOKE
#undef  CONFIG_IS_64BIT
#undef  CONFIG_MULTI_ARCHITECTURE
#undef  CONFIG_X86_EM64T
#undef  CONFIG_PPC_CACHE_L1_WRITETHROUGH
#undef  CONFIG_PPC_TLB_INV_LOCAL
#undef  CONFIG_PPC_CACHE_ICBI_LOCAL
#undef  CONFIG_X86_SMALL_SPACES_GLOBAL
#undef  CONFIG_X86_HVM
#undef  CONFIG_PPC_MMU_SEGMENTS
#define CONFIG_X86_TSC 1
/* That's all, folks! */
#define AUTOCONF_INCLUDED
