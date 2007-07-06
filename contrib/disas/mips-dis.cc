/* Print mips instructions for GDB, the GNU debugger, or for objdump.
   Copyright 1989, 1991, 1992, 1993, 1994, 1995, 1996, 1997, 1998, 1999,
   2000, 2001, 2002, 2003
   Free Software Foundation, Inc.
   Contributed by Nobuyuki Hikichi(hikichi@sra.co.jp).

This file is part of GDB, GAS, and the GNU binutils.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

#include "mips.h"

/* FIXME: These are needed to figure out if the code is mips16 or
   not. The low bit of the address is often a good indicator.  No
   symbol table is available when this code runs out in an embedded
   system as when it is used for disassembler support in a monitor.  */

#ifndef _
#define _(string) string
#endif

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof (a) / sizeof ((a)[0]))
#endif

static void print_address( unsigned long addr );

static int strcmp( const char *s1, const char *s2 )
{
    while( 1 ) {
	if( !*s1 && !*s2 )
	    return 0;
	if( (!*s1 && *s2) || (*s1 < *s2) )
	    return -1;
	if( (*s1 && !*s2) || (*s1 > *s2) )
	    return 1;
	s1++;
	s2++;
    }
}

/* Mips instructions are at maximum this many bytes long.  */
#define INSNLEN 4

static int _print_insn_mips
  PARAMS ((word_t, unsigned long, unsigned long format ));
static int print_insn_mips
  PARAMS ((unsigned long, unsigned long int, unsigned long format));
static void print_insn_args
  PARAMS ((const char *, unsigned long, unsigned , unsigned long format));

/* FIXME: These should be shared with gdb somehow.  */

struct mips_cp0sel_name {
	unsigned int cp0reg;
	unsigned int sel;
	const char * const name;
};

#if 0
static const char * const mips_gpr_names_numeric[32] = {
  "$0",   "$1",   "$2",   "$3",   "$4",   "$5",   "$6",   "$7",
  "$8",   "$9",   "$10",  "$11",  "$12",  "$13",  "$14",  "$15",
  "$16",  "$17",  "$18",  "$19",  "$20",  "$21",  "$22",  "$23",
  "$24",  "$25",  "$26",  "$27",  "$28",  "$29",  "$30",  "$31"
};

static const char * const mips_gpr_names_oldabi[32] = {
  "zero", "at",   "v0",   "v1",   "a0",   "a1",   "a2",   "a3",
  "t0",   "t1",   "t2",   "t3",   "t4",   "t5",   "t6",   "t7",
  "s0",   "s1",   "s2",   "s3",   "s4",   "s5",   "s6",   "s7",
  "t8",   "t9",   "k0",   "k1",   "gp",   "sp",   "s8",   "ra"
};

static const char * const mips_gpr_names_newabi[32] = {
  "zero", "at",   "v0",   "v1",   "a0",   "a1",   "a2",   "a3",
  "a4",   "a5",   "a6",   "a7",   "t0",   "t1",   "t2",   "t3",
  "s0",   "s1",   "s2",   "s3",   "s4",   "s5",   "s6",   "s7",
  "t8",   "t9",   "k0",   "k1",   "gp",   "sp",   "s8",   "ra"
};
#endif

static const char * const mips_gpr_names_pistachio[32] = {
  "zero", "at",   "v0",   "v1",   "a0",   "a1",   "a2",   "a3",
  "a4",   "a5",   "a6",   "a7",   "t4",   "t5",   "t6",   "t7",
  "s0",   "s1",   "s2",   "s3",   "s4",   "s5",   "s6",   "s7",
  "t8",   "t9",   "k0",   "k1",   "gp",   "sp",   "s8",   "ra"
};

static const char * const mips_fpr_names_numeric[32] = {
  "$f0",  "$f1",  "$f2",  "$f3",  "$f4",  "$f5",  "$f6",  "$f7",
  "$f8",  "$f9",  "$f10", "$f11", "$f12", "$f13", "$f14", "$f15",
  "$f16", "$f17", "$f18", "$f19", "$f20", "$f21", "$f22", "$f23",
  "$f24", "$f25", "$f26", "$f27", "$f28", "$f29", "$f30", "$f31"
};

#if 0
static const char * const mips_fpr_names_32[32] = {
  "fv0",  "fv0f", "fv1",  "fv1f", "ft0",  "ft0f", "ft1",  "ft1f",
  "ft2",  "ft2f", "ft3",  "ft3f", "fa0",  "fa0f", "fa1",  "fa1f",
  "ft4",  "ft4f", "ft5",  "ft5f", "fs0",  "fs0f", "fs1",  "fs1f",
  "fs2",  "fs2f", "fs3",  "fs3f", "fs4",  "fs4f", "fs5",  "fs5f"
};

static const char * const mips_fpr_names_n32[32] = {
  "fv0",  "ft14", "fv1",  "ft15", "ft0",  "ft1",  "ft2",  "ft3",
  "ft4",  "ft5",  "ft6",  "ft7",  "fa0",  "fa1",  "fa2",  "fa3",
  "fa4",  "fa5",  "fa6",  "fa7",  "fs0",  "ft8",  "fs1",  "ft9",
  "fs2",  "ft10", "fs3",  "ft11", "fs4",  "ft12", "fs5",  "ft13"
};

static const char * const mips_fpr_names_64[32] = {
  "fv0",  "ft12", "fv1",  "ft13", "ft0",  "ft1",  "ft2",  "ft3",
  "ft4",  "ft5",  "ft6",  "ft7",  "fa0",  "fa1",  "fa2",  "fa3",
  "fa4",  "fa5",  "fa6",  "fa7",  "ft8",  "ft9",  "ft10", "ft11",
  "fs0",  "fs1",  "fs2",  "fs3",  "fs4",  "fs5",  "fs6",  "fs7"
};
#endif

static const char * const mips_cp0_names_r4x00[32] = {
  "c0_index",     "c0_random",    "c0_entrylo0",  "c0_entrylo1",
  "c0_context",   "c0_pagemask",  "c0_wired",     "$7",
  "c0_badvaddr",  "c0_count",     "c0_entryhi",   "c0_compare",
  "c0_status",    "c0_cause",     "c0_epc",       "c0_prid",
  "c0_config",    "c0_lladdr",    "c0_watchlo",   "c0_watchhi",
  "c0_xcontext",  "$21",          "$22",          "$23",
  "$24",	  "$25",	  "c0_ecc",	  "c0_cacheerr",
  "c0_taglo",     "c0_taghi",     "c0_errorepc",  "$31",
};


#if 0
static const char * const mips_cp0_names_mips3264[32] = {
  "c0_index",     "c0_random",    "c0_entrylo0",  "c0_entrylo1",
  "c0_context",   "c0_pagemask",  "c0_wired",     "$7",
  "c0_badvaddr",  "c0_count",     "c0_entryhi",   "c0_compare",
  "c0_status",    "c0_cause",     "c0_epc",       "c0_prid",
  "c0_config",    "c0_lladdr",    "c0_watchlo",   "c0_watchhi",
  "c0_xcontext",  "$21",          "$22",          "c0_debug",
  "c0_depc",      "c0_perfcnt",   "c0_errctl",    "c0_cacheerr",
  "c0_taglo",     "c0_taghi",     "c0_errorepc",  "c0_desave",
};

static const struct mips_cp0sel_name mips_cp0sel_names_mips3264[] = {
  { 16, 1, "c0_config1"		},
  { 16, 2, "c0_config2"		},
  { 16, 3, "c0_config3"		},
  { 18, 1, "c0_watchlo,1"	},
  { 18, 2, "c0_watchlo,2"	},
  { 18, 3, "c0_watchlo,3"	},
  { 18, 4, "c0_watchlo,4"	},
  { 18, 5, "c0_watchlo,5"	},
  { 18, 6, "c0_watchlo,6"	},
  { 18, 7, "c0_watchlo,7"	},
  { 19, 1, "c0_watchhi,1"	},
  { 19, 2, "c0_watchhi,2"	},
  { 19, 3, "c0_watchhi,3"	},
  { 19, 4, "c0_watchhi,4"	},
  { 19, 5, "c0_watchhi,5"	},
  { 19, 6, "c0_watchhi,6"	},
  { 19, 7, "c0_watchhi,7"	},
  { 25, 1, "c0_perfcnt,1"	},
  { 25, 2, "c0_perfcnt,2"	},
  { 25, 3, "c0_perfcnt,3"	},
  { 25, 4, "c0_perfcnt,4"	},
  { 25, 5, "c0_perfcnt,5"	},
  { 25, 6, "c0_perfcnt,6"	},
  { 25, 7, "c0_perfcnt,7"	},
  { 27, 1, "c0_cacheerr,1"	},
  { 27, 2, "c0_cacheerr,2"	},
  { 27, 3, "c0_cacheerr,3"	},
  { 28, 1, "c0_datalo"		},
  { 29, 1, "c0_datahi"		}
};

static const char * const mips_cp0_names_mips3264r2[32] = {
  "c0_index",     "c0_random",    "c0_entrylo0",  "c0_entrylo1",
  "c0_context",   "c0_pagemask",  "c0_wired",     "c0_hwrena",
  "c0_badvaddr",  "c0_count",     "c0_entryhi",   "c0_compare",
  "c0_status",    "c0_cause",     "c0_epc",       "c0_prid",
  "c0_config",    "c0_lladdr",    "c0_watchlo",   "c0_watchhi",
  "c0_xcontext",  "$21",          "$22",          "c0_debug",
  "c0_depc",      "c0_perfcnt",   "c0_errctl",    "c0_cacheerr",
  "c0_taglo",     "c0_taghi",     "c0_errorepc",  "c0_desave",
};

static const struct mips_cp0sel_name mips_cp0sel_names_mips3264r2[] = {
  {  4, 1, "c0_contextconfig"	},
  {  5, 1, "c0_pagegrain"	},
  { 12, 1, "c0_intctl"		},
  { 12, 2, "c0_srsctl"		},
  { 12, 3, "c0_srsmap"		},
  { 15, 1, "c0_ebase"		},
  { 16, 1, "c0_config1"		},
  { 16, 2, "c0_config2"		},
  { 16, 3, "c0_config3"		},
  { 18, 1, "c0_watchlo,1"	},
  { 18, 2, "c0_watchlo,2"	},
  { 18, 3, "c0_watchlo,3"	},
  { 18, 4, "c0_watchlo,4"	},
  { 18, 5, "c0_watchlo,5"	},
  { 18, 6, "c0_watchlo,6"	},
  { 18, 7, "c0_watchlo,7"	},
  { 19, 1, "c0_watchhi,1"	},
  { 19, 2, "c0_watchhi,2"	},
  { 19, 3, "c0_watchhi,3"	},
  { 19, 4, "c0_watchhi,4"	},
  { 19, 5, "c0_watchhi,5"	},
  { 19, 6, "c0_watchhi,6"	},
  { 19, 7, "c0_watchhi,7"	},
  { 23, 1, "c0_tracecontrol"	},
  { 23, 2, "c0_tracecontrol2"	},
  { 23, 3, "c0_usertracedata"	},
  { 23, 4, "c0_tracebpc"	},
  { 25, 1, "c0_perfcnt,1"	},
  { 25, 2, "c0_perfcnt,2"	},
  { 25, 3, "c0_perfcnt,3"	},
  { 25, 4, "c0_perfcnt,4"	},
  { 25, 5, "c0_perfcnt,5"	},
  { 25, 6, "c0_perfcnt,6"	},
  { 25, 7, "c0_perfcnt,7"	},
  { 27, 1, "c0_cacheerr,1"	},
  { 27, 2, "c0_cacheerr,2"	},
  { 27, 3, "c0_cacheerr,3"	},
  { 28, 1, "c0_datalo"		},
  { 28, 2, "c0_taglo1"		},
  { 28, 3, "c0_datalo1"		},
  { 28, 4, "c0_taglo2"		},
  { 28, 5, "c0_datalo2"		},
  { 28, 6, "c0_taglo3"		},
  { 28, 7, "c0_datalo3"		},
  { 29, 1, "c0_datahi"		},
  { 29, 2, "c0_taghi1"		},
  { 29, 3, "c0_datahi1"		},
  { 29, 4, "c0_taghi2"		},
  { 29, 5, "c0_datahi2"		},
  { 29, 6, "c0_taghi3"		},
  { 29, 7, "c0_datahi3"		},
};
#endif

#if CONFIG_CPU_MIPS64_SB1

/* SB-1: MIPS64 (mips_cp0_names_mips3264) with minor mods.  */
static const char * const mips_cp0_names_sb1[32] = {
  "c0_index",     "c0_random",    "c0_entrylo0",  "c0_entrylo1",
  "c0_context",   "c0_pagemask",  "c0_wired",     "$7",
  "c0_badvaddr",  "c0_count",     "c0_entryhi",   "c0_compare",
  "c0_status",    "c0_cause",     "c0_epc",       "c0_prid",
  "c0_config",    "c0_lladdr",    "c0_watchlo",   "c0_watchhi",
  "c0_xcontext",  "$21",          "$22",          "c0_debug",
  "c0_depc",      "c0_perfcnt",   "c0_errctl",    "c0_cacheerr_i",
  "c0_taglo_i",   "c0_taghi_i",   "c0_errorepc",  "c0_desave",
};

static const struct mips_cp0sel_name mips_cp0sel_names_sb1[] = {
  { 16, 1, "c0_config1"		},
  { 18, 1, "c0_watchlo,1"	},
  { 19, 1, "c0_watchhi,1"	},
  { 22, 0, "c0_perftrace"	},
  { 23, 3, "c0_edebug"		},
  { 25, 1, "c0_perfcnt,1"	},
  { 25, 2, "c0_perfcnt,2"	},
  { 25, 3, "c0_perfcnt,3"	},
  { 25, 4, "c0_perfcnt,4"	},
  { 25, 5, "c0_perfcnt,5"	},
  { 25, 6, "c0_perfcnt,6"	},
  { 25, 7, "c0_perfcnt,7"	},
  { 26, 1, "c0_buserr_pa"	},
  { 27, 1, "c0_cacheerr_d"	},
  { 27, 3, "c0_cacheerr_d_pa"	},
  { 28, 1, "c0_datalo_i"	},
  { 28, 2, "c0_taglo_d"		},
  { 28, 3, "c0_datalo_d"	},
  { 29, 1, "c0_datahi_i"	},
  { 29, 2, "c0_taghi_d"		},
  { 29, 3, "c0_datahi_d"	},
};

#endif

static const char * const mips_hwr_names_numeric[32] = {
  "$0",   "$1",   "$2",   "$3",   "$4",   "$5",   "$6",   "$7",
  "$8",   "$9",   "$10",  "$11",  "$12",  "$13",  "$14",  "$15",
  "$16",  "$17",  "$18",  "$19",  "$20",  "$21",  "$22",  "$23",
  "$24",  "$25",  "$26",  "$27",  "$28",  "$29",  "$30",  "$31"
};

#if 0
static const char * const mips_hwr_names_mips3264r2[32] = {
  "hwr_cpunum",   "hwr_synci_step", "hwr_cc",     "hwr_ccres",
  "$4",          "$5",            "$6",           "$7",
  "$8",   "$9",   "$10",  "$11",  "$12",  "$13",  "$14",  "$15",
  "$16",  "$17",  "$18",  "$19",  "$20",  "$21",  "$22",  "$23",
  "$24",  "$25",  "$26",  "$27",  "$28",  "$29",  "$30",  "$31"
};
#endif

struct mips_abi_choice {
  const char *name;
  const char * const *gpr_names;
  const char * const *fpr_names;
};

struct mips_arch_choice {
  const char *name;
  int bfd_mach_valid;
  unsigned long bfd_mach;
  int processor;
  int isa;
  const char * const *cp0_names;
  const struct mips_cp0sel_name *cp0sel_names;
  unsigned int cp0sel_names_len;
  const char * const *hwr_names;
};

#if 0
const struct mips_arch_choice mips_arch_choices[] = {
  { "numeric",	0, 0, 0, 0,
    mips_cp0_names_numeric, NULL, 0, mips_hwr_names_numeric },

  { "r3000",	1, bfd_mach_mips3000, CPU_R3000, ISA_MIPS1,
    mips_cp0_names_numeric, NULL, 0, mips_hwr_names_numeric },
  { "r3900",	1, bfd_mach_mips3900, CPU_R3900, ISA_MIPS1,
    mips_cp0_names_numeric, NULL, 0, mips_hwr_names_numeric },
  { "r4000",	1, bfd_mach_mips4000, CPU_R4000, ISA_MIPS3,
    mips_cp0_names_numeric, NULL, 0, mips_hwr_names_numeric },
  { "r4010",	1, bfd_mach_mips4010, CPU_R4010, ISA_MIPS2,
    mips_cp0_names_numeric, NULL, 0, mips_hwr_names_numeric },
  { "vr4100",	1, bfd_mach_mips4100, CPU_VR4100, ISA_MIPS3,
    mips_cp0_names_numeric, NULL, 0, mips_hwr_names_numeric },
  { "vr4111",	1, bfd_mach_mips4111, CPU_R4111, ISA_MIPS3,
    mips_cp0_names_numeric, NULL, 0, mips_hwr_names_numeric },
  { "vr4120",	1, bfd_mach_mips4120, CPU_VR4120, ISA_MIPS3,
    mips_cp0_names_numeric, NULL, 0, mips_hwr_names_numeric },
  { "r4300",	1, bfd_mach_mips4300, CPU_R4300, ISA_MIPS3,
    mips_cp0_names_numeric, NULL, 0, mips_hwr_names_numeric },
  { "r4400",	1, bfd_mach_mips4400, CPU_R4400, ISA_MIPS3,
    mips_cp0_names_numeric, NULL, 0, mips_hwr_names_numeric },
  { "r4600",	1, bfd_mach_mips4600, CPU_R4600, ISA_MIPS3,
    mips_cp0_names_numeric, NULL, 0, mips_hwr_names_numeric },
  { "r4650",	1, bfd_mach_mips4650, CPU_R4650, ISA_MIPS3,
    mips_cp0_names_numeric, NULL, 0, mips_hwr_names_numeric },
  { "r5000",	1, bfd_mach_mips5000, CPU_R5000, ISA_MIPS4,
    mips_cp0_names_numeric, NULL, 0, mips_hwr_names_numeric },
  { "vr5400",	1, bfd_mach_mips5400, CPU_VR5400, ISA_MIPS4,
    mips_cp0_names_numeric, NULL, 0, mips_hwr_names_numeric },
  { "vr5500",	1, bfd_mach_mips5500, CPU_VR5500, ISA_MIPS4,
    mips_cp0_names_numeric, NULL, 0, mips_hwr_names_numeric },
  { "r6000",	1, bfd_mach_mips6000, CPU_R6000, ISA_MIPS2,
    mips_cp0_names_numeric, NULL, 0, mips_hwr_names_numeric },
  { "r8000",	1, bfd_mach_mips8000, CPU_R8000, ISA_MIPS4,
    mips_cp0_names_numeric, NULL, 0, mips_hwr_names_numeric },
  { "r10000",	1, bfd_mach_mips10000, CPU_R10000, ISA_MIPS4,
    mips_cp0_names_numeric, NULL, 0, mips_hwr_names_numeric },
  { "r12000",	1, bfd_mach_mips12000, CPU_R12000, ISA_MIPS4,
    mips_cp0_names_numeric, NULL, 0, mips_hwr_names_numeric },
  { "mips5",	1, bfd_mach_mips5, CPU_MIPS5, ISA_MIPS5,
    mips_cp0_names_numeric, NULL, 0, mips_hwr_names_numeric },

  /* For stock MIPS32, disassemble all applicable MIPS-specified ASEs.
     Note that MIPS-3D and MDMX are not applicable to MIPS32.  (See
     _MIPS32 Architecture For Programmers Volume I: Introduction to the
     MIPS32 Architecture_ (MIPS Document Number MD00082, Revision 0.95),
     page 1.  */
  { "mips32",	1, bfd_mach_mipsisa32, CPU_MIPS32,
    ISA_MIPS32 | INSN_MIPS16,
    mips_cp0_names_mips3264,
    mips_cp0sel_names_mips3264, ARRAY_SIZE (mips_cp0sel_names_mips3264),
    mips_hwr_names_numeric },

  { "mips32r2",	1, bfd_mach_mipsisa32r2, CPU_MIPS32R2,
    ISA_MIPS32R2 | INSN_MIPS16,
    mips_cp0_names_mips3264r2,
    mips_cp0sel_names_mips3264r2, ARRAY_SIZE (mips_cp0sel_names_mips3264r2),
    mips_hwr_names_mips3264r2 },

  /* For stock MIPS64, disassemble all applicable MIPS-specified ASEs.  */
  { "mips64",	1, bfd_mach_mipsisa64, CPU_MIPS64,
    ISA_MIPS64 | INSN_MIPS16 | INSN_MIPS3D | INSN_MDMX,
    mips_cp0_names_mips3264,
    mips_cp0sel_names_mips3264, ARRAY_SIZE (mips_cp0sel_names_mips3264),
    mips_hwr_names_numeric },

  { "sb1",	1, bfd_mach_mips_sb1, CPU_SB1,
    ISA_MIPS64 | INSN_MIPS3D | INSN_SB1,
    mips_cp0_names_sb1,
    mips_cp0sel_names_sb1, ARRAY_SIZE (mips_cp0sel_names_sb1),
    mips_hwr_names_numeric },

  /* This entry, mips16, is here only for ISA/processor selection; do
     not print its name.  */
  { "",		1, bfd_mach_mips16, CPU_MIPS16, ISA_MIPS3 | INSN_MIPS16,
    mips_cp0_names_numeric, NULL, 0, mips_hwr_names_numeric },
};
#endif

/* ISA and processor type to disassemble for, and register names to use.
   set_default_mips_dis_options and parse_mips_dis_options fill in these
   values.  */
static int mips_processor;
static int mips_isa;
static const char * const *mips_gpr_names;
static const char * const *mips_fpr_names;
static const char * const *mips_cp0_names;
static const struct mips_cp0sel_name *mips_cp0sel_names;
static int mips_cp0sel_names_len;
static const char * const *mips_hwr_names;

static const struct mips_cp0sel_name *lookup_mips_cp0sel_name
  PARAMS ((const struct mips_cp0sel_name *, unsigned int, unsigned int,
	   unsigned int));


void
set_default_mips_dis_options ( unsigned int type )
{
  /* Defaults: mipsIII/r3000 (?!), (o)32-style ("oldabi") GPR names,
     and numeric FPR, CP0 register, and HWR names.  */

  mips_gpr_names = mips_gpr_names_pistachio;
  mips_fpr_names = mips_fpr_names_numeric;

#if CONFIG_CPU_MIPS64_R4X00
  mips_isa = ISA_MIPS3;
  mips_processor =  CPU_R4600;
  mips_cp0sel_names = NULL;
  mips_cp0sel_names_len = 0;
  mips_hwr_names = mips_hwr_names_numeric;
  mips_cp0_names = mips_cp0_names_r4x00;

#elif CONFIG_CPU_MIPS64_RC64574
  mips_isa = ISA_MIPS4;
  mips_processor =  CPU_R5000;
  mips_cp0sel_names = NULL;
  mips_cp0sel_names_len = 0;
  mips_hwr_names = mips_hwr_names_numeric;
  mips_cp0_names = mips_cp0_names_r4x00;

#elif CONFIG_CPU_MIPS64_SB1
  mips_isa = ISA_MIPS64 | INSN_MIPS3D | INSN_SB1;
  mips_processor =  CPU_SB1;
  mips_cp0sel_names = mips_cp0sel_names_sb1;
  mips_cp0sel_names_len = ARRAY_SIZE (mips_cp0sel_names_sb1);
  mips_hwr_names = mips_hwr_names_numeric;
  mips_cp0_names = mips_cp0_names_sb1;

#elif CONFIG_CPU_MIPS64_VR4121
  mips_isa = ISA_MIPS3;
  mips_processor =  CPU_VR4120;
  mips_cp0sel_names = NULL;
  mips_cp0sel_names_len = 0;
  mips_hwr_names = mips_hwr_names_numeric;
  mips_cp0_names = mips_cp0_names_r4x00;

#elif CONFIG_CPU_MIPS64_VR4181
  mips_isa = ISA_MIPS3;
  mips_processor =  CPU_R4111;
  mips_cp0sel_names = NULL;
  mips_cp0sel_names_len = 0;
  mips_hwr_names = mips_hwr_names_numeric;
  mips_cp0_names = mips_cp0_names_r4x00;

#else
#error unknown CPU
#endif

}

static const struct mips_cp0sel_name *
lookup_mips_cp0sel_name( const struct mips_cp0sel_name *names,
	unsigned int len, unsigned int cp0reg,
	unsigned int sel )
{
  unsigned int i;

  for (i = 0; i < len; i++)
    if (names[i].cp0reg == cp0reg && names[i].sel == sel)
      return &names[i];
  return NULL;
}

/* Print insn arguments for 32/64-bit code.  */

static void
print_insn_args ( const char *d,
     unsigned long int l,
     unsigned long pc,
     unsigned long format )
{
  int op, delta;
  unsigned int lsb, msb, msbd;
  unsigned long target;

  lsb = 0;

  for (; *d != '\0'; d++)
    {
      switch (*d)
	{
	case ',':
	case '(':
	case ')':
	case '[':
	case ']':
	  printf("%c", *d);
	  break;

	case '+':
	  /* Extension character; switch for second char.  */
	  d++;
	  switch (*d)
	    {
	    case '\0':
	      /* xgettext:c-format */
	      printf( _("# internal error, incomplete extension sequence (+)"));
	      return;

	    case 'A':
	      lsb = (l >> OP_SH_SHAMT) & OP_MASK_SHAMT;
	      printf ("0x%x", lsb);
	      break;
	
	    case 'B':
	      msb = (l >> OP_SH_INSMSB) & OP_MASK_INSMSB;
	      printf ("0x%x", msb - lsb + 1);
	      break;

	    case 'C':
	      msbd = (l >> OP_SH_EXTMSBD) & OP_MASK_EXTMSBD;
	      printf("0x%x", msbd + 1);
	      break;

	    case 'D':
	      {
		const struct mips_cp0sel_name *n;
		unsigned int cp0reg, sel;

		cp0reg = (l >> OP_SH_RD) & OP_MASK_RD;
		sel = (l >> OP_SH_SEL) & OP_MASK_SEL;

		/* CP0 register including 'sel' code for mtcN (et al.), to be
		   printed textually if known.  If not known, print both
		   CP0 register name and sel numerically since CP0 register
		   with sel 0 may have a name unrelated to register being
		   printed.  */
		n = lookup_mips_cp0sel_name(mips_cp0sel_names,
					    mips_cp0sel_names_len, cp0reg, sel);
		if (n != NULL)
		  printf ( "%s", n->name);
		else
		  printf("$%d,%d", cp0reg, sel);
		break;
	      }

	    default:
	      /* xgettext:c-format */
	      printf( _("# internal error, undefined extension sequence (+%c)"),
				     *d);
	      return;
	    }
	  break;

	case 's':
	case 'b':
	case 'r':
	case 'v':
	  printf("%s", mips_gpr_names[(l >> OP_SH_RS) & OP_MASK_RS]);
	  break;

	case 't':
	case 'w':
	  printf("%s", mips_gpr_names[(l >> OP_SH_RT) & OP_MASK_RT]);
	  break;

	case 'i':
	case 'u':
	  printf("0x%x", (l >> OP_SH_IMMEDIATE) & OP_MASK_IMMEDIATE);
	  break;

	case 'j': /* Same as i, but sign-extended.  */
	case 'o':
	  delta = (l >> OP_SH_DELTA) & OP_MASK_DELTA;
	  if (delta & 0x8000)
	    delta |= ~0xffff;
	  printf("%d", delta);
	  break;

	case 'h':
	  printf ("0x%x", (unsigned int) ((l >> OP_SH_PREFX)
						 & OP_MASK_PREFX));
	  break;

	case 'k':
	  printf("0x%x", (unsigned int) ((l >> OP_SH_CACHE)
						 & OP_MASK_CACHE));
	  break;

	case 'a':
	  target = (((pc + 4) & ~(unsigned long) 0x0fffffff)
			  | (((l >> OP_SH_TARGET) & OP_MASK_TARGET) << 2));
	  print_address (target);
	  break;

	case 'p':
	  /* Sign extend the displacement.  */
	  delta = (l >> OP_SH_DELTA) & OP_MASK_DELTA;
	  if (delta & 0x8000)
	    delta |= ~0xffff;
	  target = (delta << 2) + pc + INSNLEN;
	  print_address(target);
	  break;

	case 'd':
	  printf ("%s", mips_gpr_names[(l >> OP_SH_RD) & OP_MASK_RD]);
	  break;

	case 'U':
	  {
	    /* First check for both rd and rt being equal.  */
	    unsigned int reg = (l >> OP_SH_RD) & OP_MASK_RD;
	    if (reg == ((l >> OP_SH_RT) & OP_MASK_RT))
	      printf("%s",
				     mips_gpr_names[reg]);
	    else
	      {
		/* If one is zero use the other.  */
		if (reg == 0)
		  printf ("%s", mips_gpr_names[(l >> OP_SH_RT) & OP_MASK_RT]);
		else if (((l >> OP_SH_RT) & OP_MASK_RT) == 0)
		  printf("%s", mips_gpr_names[reg]);
		else /* Bogus, result depends on processor.  */
		  printf("%s or %s", mips_gpr_names[reg],
					 mips_gpr_names[(l >> OP_SH_RT) & OP_MASK_RT]);
	      }
	  }
	  break;

	case 'z':
	  printf ( "%s", mips_gpr_names[0]);
	  break;

	case '<':
	  printf( "0x%x", (l >> OP_SH_SHAMT) & OP_MASK_SHAMT);
	  break;

	case 'c':
	  printf ("0x%x", (l >> OP_SH_CODE) & OP_MASK_CODE);
	  break;

	case 'q':
	  printf ("0x%x", (l >> OP_SH_CODE2) & OP_MASK_CODE2);
	  break;

	case 'C':
	  printf("0x%x", (l >> OP_SH_COPZ) & OP_MASK_COPZ);
	  break;

	case 'B':
	  printf ("0x%x", (l >> OP_SH_CODE20) & OP_MASK_CODE20);
	  break;

	case 'J':
	  printf ("0x%x", (l >> OP_SH_CODE19) & OP_MASK_CODE19);
	  break;

	case 'S':
	case 'V':
	  printf ( "%s", mips_fpr_names[(l >> OP_SH_FS) & OP_MASK_FS]);
	  break;

	case 'T':
	case 'W':
	  printf( "%s", mips_fpr_names[(l >> OP_SH_FT) & OP_MASK_FT]);
	  break;

	case 'D':
	  printf ("%s", mips_fpr_names[(l >> OP_SH_FD) & OP_MASK_FD]);
	  break;

	case 'R':
	  printf("%s", mips_fpr_names[(l >> OP_SH_FR) & OP_MASK_FR]);
	  break;

	case 'E':
	  /* Coprocessor register for lwcN instructions, et al.

	     Note that there is no load/store cp0 instructions, and
	     that FPU (cp1) instructions disassemble this field using
	     'T' format.  Therefore, until we gain understanding of
	     cp2 register names, we can simply print the register
	     numbers.  */
	  printf("$%d", (l >> OP_SH_RT) & OP_MASK_RT);
	  break;

	case 'G':
	  /* Coprocessor register for mtcN instructions, et al.  Note
	     that FPU (cp1) instructions disassemble this field using
	     'S' format.  Therefore, we only need to worry about cp0,
	     cp2, and cp3.  */
	  op = (l >> OP_SH_OP) & OP_MASK_OP;
	  if (op == OP_OP_COP0)
	    printf("%s", mips_cp0_names[(l >> OP_SH_RD) & OP_MASK_RD]);
	  else
	    printf("$%d", (l >> OP_SH_RD) & OP_MASK_RD);
	  break;

	case 'K':
	  printf ("%s", mips_hwr_names[(l >> OP_SH_RD) & OP_MASK_RD]);
	  break;

	case 'N':
	  printf("$fcc%d", (l >> OP_SH_BCC) & OP_MASK_BCC);
	  break;

	case 'M':
	  printf ("$fcc%d", (l >> OP_SH_CCC) & OP_MASK_CCC);
	  break;

	case 'P':
	  printf ("%d", (l >> OP_SH_PERFREG) & OP_MASK_PERFREG);
	  break;

	case 'e':
	  printf("%d", (l >> OP_SH_VECBYTE) & OP_MASK_VECBYTE);
	  break;

	case '%':
	  printf("%d", (l >> OP_SH_VECALIGN) & OP_MASK_VECALIGN);
	  break;

	case 'H':
	  printf ("%d", (l >> OP_SH_SEL) & OP_MASK_SEL);
	  break;

	case 'O':
	  printf( "%d", (l >> OP_SH_ALN) & OP_MASK_ALN);
	  break;

	case 'Q':
	  {
	    unsigned int vsel = (l >> OP_SH_VSEL) & OP_MASK_VSEL;
	    if ((vsel & 0x10) == 0)
	      {
		int fmt;
		vsel &= 0x0f;
		for (fmt = 0; fmt < 3; fmt++, vsel >>= 1)
		  if ((vsel & 1) == 0)
		    break;
		printf( "$v%d[%d]",
				       (l >> OP_SH_FT) & OP_MASK_FT,
				       vsel >> 1);
	      }
	    else if ((vsel & 0x08) == 0)
	      {
		printf( "$v%d", (l >> OP_SH_FT) & OP_MASK_FT);
	      }
	    else
	      {
		printf("0x%x", (l >> OP_SH_FT) & OP_MASK_FT);
	      }
	  }
	  break;

	case 'X':
	  printf("$v%d", (l >> OP_SH_FD) & OP_MASK_FD);
	  break;

	case 'Y':
	  printf ( "$v%d", (l >> OP_SH_FS) & OP_MASK_FS);
	  break;

	case 'Z':
	  printf ("$v%d", (l >> OP_SH_FT) & OP_MASK_FT);
	  break;

	default:
	  /* xgettext:c-format */
	  printf( _("# internal error, undefined modifier(%c)"),
				 *d);
	  return;
	}
    }
}

/* Print the mips instruction at address MEMADDR in debugged memory,
   on using INFO.  Returns length of the instruction, in bytes, which is
   always INSNLEN.  BIGENDIAN must be 1 if this is big-endian code, 0 if
   this is little-endian code.  */

static int
print_insn_mips ( unsigned long memaddr,
     unsigned long int word,
     unsigned long format )
{
  register const struct mips_opcode *op;
  static bool init = 0;
  static const struct mips_opcode *mips_hash[OP_MASK_OP + 1];

  /* Build a hash table to shorten the search time.  */
  if (! init)
    {
      unsigned int i;

      for (i = 0; i <= OP_MASK_OP; i++)
	{
	  for (op = mips_opcodes; op < &mips_opcodes[NUMOPCODES]; op++)
	    {
	      if (op->pinfo == INSN_MACRO)
		continue;
	      if (i == ((op->match >> OP_SH_OP) & OP_MASK_OP))
		{
		  mips_hash[i] = op;
		  break;
		}
	    }
	}

	set_default_mips_dis_options ( 0 );

      init = 1;
    }

  op = mips_hash[(word >> OP_SH_OP) & OP_MASK_OP];
  if (op != NULL)
    {
      for (; op < &mips_opcodes[NUMOPCODES]; op++)
	{
	  if (op->pinfo != INSN_MACRO && (word & op->mask) == op->match)
	    {
	      register const char *d;

	      /* We always allow to disassemble the jalx instruction.  */
	      if (! OPCODE_IS_MEMBER (op, mips_isa, mips_processor)
		  && strcmp (op->name, "jalx"))
		continue;

	      /* Figure out instruction type and branch delay information.  */
	      if ((op->pinfo & INSN_UNCOND_BRANCH_DELAY) != 0)
	        {
		}
	      else if ((op->pinfo & (INSN_COND_BRANCH_DELAY
				     | INSN_COND_BRANCH_LIKELY)) != 0)
		{
		}
	      else if ((op->pinfo & (INSN_STORE_MEMORY
				     | INSN_LOAD_MEMORY_DELAY)) != 0)
		;

	      printf( "%s", op->name);

	      d = op->args;
	      if (d != NULL && *d != '\0')
		{
		  printf ("\t");
		  print_insn_args (d, word, memaddr, format);
		}

	      return INSNLEN;
	    }
	}
    }

  printf("0x%x", word);
  return INSNLEN;
}

/* In an environment where we do not know the symbol type of the
   instruction we are forced to assume that the low order bit of the
   instructions' address may mark it as a mips16 instruction.  If we
   are single stepping, or the pc is within the disassembled function,
   this works.  Otherwise, we need a clue.  Sometimes.  */

static int
_print_insn_mips ( word_t insn, unsigned long memaddr, unsigned long format )
{
      return print_insn_mips (memaddr, insn, format);
}

