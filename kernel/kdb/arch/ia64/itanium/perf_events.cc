/*********************************************************************
 *                
 * Copyright (C) 2002, 2003,  Karlsruhe University
 *                
 * File path:     kdb/arch/ia64/itanium/perf_events.cc
 * Description:   
 *                
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *                
 * $Id: perf_events.cc,v 1.4 2003/09/24 19:05:07 skoglund Exp $
 *                
 ********************************************************************/
#include <kdb/kdb.h>
#include <kdb/input.h>
#include <debug.h>

#include INC_ARCH(itanium_perf.h)

#define PMC_45		((1 << 4) | (1 << 5))
#define PMC_67		((1 << 6) | (1 << 7))
#define PMC_4567	(PMC_45 | PMC_67)

class ia64_perf_event_t
{
public:
    class ia64_umask_t {
    public:
	char * name;
	u8_t umask;
    };


    char *	name;
    u8_t	event_selector;
    word_t	pmc_mask;
    ia64_umask_t umask[5];
};

ia64_perf_event_t perf_events[] =
{
    { "ALAT_REPLACEMENT", 0x38, PMC_4567,
      { { "ALL", 3 }, { "FP", 2 }, { "INT", 1 } }
    },
    { "ALAT_INST_CHKA_LDC", 0x36, PMC_4567,
      { { "ALL", 3 }, { "FP", 2 }, { "INT", 1 } }
    },
    { "ALAT_INST_FAILED_LDC", 0x37, PMC_4567,
      { { "ALL", 3 }, { "FP", 2 }, { "INT", 1 } }
    },
    { "ALL_STOPS_DISPERSED", 0x2f, PMC_4567, { }
    },
    { "BRANCH_EVENT", 0x11, PMC_4567, { } },
    { "BRANCH_MULTIWAY.ALL_PATHS", 0x0e, PMC_4567,
      { { "ALL_PREDICTIONS", 0 },
	{ "CORRECT_PREDICTIONS", 1 },
	{ "WRONG_PATH", 2 },
	{ "WRONG_TARGET", 3 } }
    },
    { "BRANCH_MULTIWAY.NOT_TAKEN", 0x0e, PMC_4567,
      { { "ALL_PREDICTIONS", 0x8 },
	{ "CORRECT_PREDICTIONS", 0x9 },
	{ "WRONG_PATH", 0xa },
	{ "WRONG_TARGET", 0xb } }
    },
    { "BRANCH_MULTIWAY.TAKEN", 0x0e, PMC_4567,
      { { "ALL_PREDICTIONS", 0xc },
	{ "CORRECT_PREDICTIONS", 0xd },
	{ "WRONG_PATH", 0xe },
	{ "WRONG_TARGET", 0xf } }
    },
    { "BRANCH_PATH.ALL", 0x0f, PMC_4567,
      { { "NT_OUTCOMES_CORRECTLY_PREDICTED", 0x2 },
	{ "TK_OUTCOMES_CORRECTLY_PREDICTED", 0x3 },
	{ "NT_OUTCOMES_INCORRECTLY_PREDICTED", 0x0 },
	{ "TK_OUTCOMES_CORRECTLY_PREDICTED", 0x1 } }
    },
    { "BRANCH_PATH.1ST_STAGE", 0x0f, PMC_4567,
      { { "NT_OUTCOMES_CORRECTLY_PREDICTED", 0x6 },
	{ "TK_OUTCOMES_CORRECTLY_PREDICTED", 0x7 },
	{ "NT_OUTCOMES_INCORRECTLY_PREDICTED", 0x4 },
	{ "TK_OUTCOMES_CORRECTLY_PREDICTED", 0x5 } }
    },
    { "BRANCH_PATH.2ND_STAGE", 0x0f, PMC_4567,
      { { "NT_OUTCOMES_CORRECTLY_PREDICTED", 0xa },
	{ "TK_OUTCOMES_CORRECTLY_PREDICTED", 0xb },
	{ "NT_OUTCOMES_INCORRECTLY_PREDICTED", 0x8 },
	{ "TK_OUTCOMES_CORRECTLY_PREDICTED", 0x9 } }
    },
    { "BRANCH_PATH.3RD_STAGE", 0x0f, PMC_4567,
      { { "NT_OUTCOMES_CORRECTLY_PREDICTED", 0xe },
	{ "TK_OUTCOMES_CORRECTLY_PREDICTED", 0xf },
	{ "NT_OUTCOMES_INCORRECTLY_PREDICTED", 0xc },
	{ "TK_OUTCOMES_CORRECTLY_PREDICTED", 0xd } }
    },
    { "BUS_MEMORY", 0x4a, PMC_4567,
      { { "ANY", 1 }, { "SELF", 2 }, { "IO", 4 } }
    },
    { "BUS_PARTIAL", 0x48, PMC_4567,
      { { "ANY", 1 }, { "SELF", 2 }, { "IO", 4 } }
    },
    { "BUS_RD_ALL", 0x4b, PMC_4567, 
      { { "ANY", 1 }, { "SELF", 2 }, { "IO", 4 } }
    },
    { "BUS_RD_DATA", 0x4c, PMC_4567, 
      { { "ANY", 1 }, { "SELF", 2 }, { "IO", 4 } }
    },
    { "BUS_RD_HIT", 0x40, PMC_4567, 
      { { "ANY", 1 }, { "SELF", 2 }, { "IO", 4 } }
    },
    { "BUS_RD_HITM", 0x41, PMC_4567, 
      { { "ANY", 1 }, { "SELF", 2 }, { "IO", 4 } }
    },
    { "BUS_RD_INVAL", 0x4e, PMC_4567, 
      { { "ANY", 1 }, { "SELF", 2 }, { "IO", 4 } }
    },
    { "BUS_RD_INVAL_BST", 0x4f, PMC_4567, 
      { { "ANY", 1 }, { "SELF", 2 }, { "IO", 4 } }
    },
    { "BUS_RD_INVAL_HITM", 0x4e, PMC_4567, 
      { { "ANY", 1 }, { "SELF", 2 }, { "IO", 4 } }
    },
    { "BUS_RD_INVAL_BST_HITM", 0x43, PMC_4567, 
      { { "ANY", 1 }, { "SELF", 2 }, { "IO", 4 } }
    },
    { "BUS_RD_IO", 0x51, PMC_4567, 
      { { "ANY", 1 }, { "SELF", 2 }, { "IO", 4 } }
    },
    { "BUS_RD_PRTL", 0x4d, PMC_4567, 
      { { "ANY", 1 }, { "SELF", 2 }, { "IO", 4 } }
    },
    { "BUS_SNOOPS", 0x46, PMC_4567, 
      { { "ANY", 1 }, { "SELF", 2 }, { "IO", 4 } }
    },
    { "BUS_SNOOPS_HITM", 0x45, PMC_4567, 
      { { "ANY", 1 }, { "SELF", 2 }, { "IO", 4 } }
    },
    { "BUS_SNOOP_STALL_CYCLES", 0x55, PMC_4567, 
      { { "ANY", 1 }, { "SELF", 2 }, { "IO", 4 } }
    },
    { "BUS_SNOOPQ_REQ", 0x56, PMC_45, { } },
    { "BUS_WR_WB", 0x52, PMC_4567, 
      { { "ANY", 1 }, { "SELF", 2 }, { "IO", 4 } }
    },
    { "CPU_CPL_CHANGES", 0x34, PMC_4567, { } },
    { "CPU_CYCLES", 0x12, PMC_4567, { } },

    { "DATA_ACCESS_CYCLE", 0x03, PMC_4567, { } },
    { "DATA_EAR_EVENTS", 0x67, PMC_4567, { } },
    { "DATA_PREFERENCES_RETIRED", 0x63, PMC_4567, { } },
    { "DEPENDENCY_ALL_CYCLE", 0x06, PMC_4567, { } },
    { "DEPENDENCY_SCOREBOARD_CYCLE", 0x02, PMC_4567, { } },
    { "DTC_MISSES", 0x60, PMC_4567, { } },
    { "DTLB_INSERTS_HPW", 0x62, PMC_4567, { } },
    { "DTLB_MISSES", 0x61, PMC_4567, { } },

    { "EXPL_STOP_DISPERSED", 0x2e, PMC_4567, { } },
    { "FP_OPS_RETIRED_HI", 0x0a, PMC_4567, { } },
    { "FP_OPS_RETIRED_LO", 0x09, PMC_4567, { } },
    { "FP_FLUSH_TO_ZERO", 0x0b, PMC_4567, { } },
    { "FP_SIR_FLUSH", 0x0c, PMC_4567, { } },

    { "IA32_INST_RETIRED", 0x15, PMC_4567, { } },
    { "IA64_INST_RETIRED", 0x08, PMC_45, 
      { { "ALL", 0 }, { "PMC8", 0x3}, { "PMC9", 0x2 } }
    },
    { "INST_ACCESS_CYCLE", 0x01, PMC_4567, { } },
    { "INST_DISPERSED", 0x2d, PMC_45, { } },
    { "INST_FAILED_CHKS_RETIRED", 0x35, PMC_4567, 
      { { "ALL", 0x3}, { "FP", 0x2 }, { "INTEGER", 0x1 } }
    },
    { "INSTRUCTION_EAR_EVENTS", 0x23, PMC_4567, { } },
    { "ISA_TRANSITIONS", 0x14, PMC_4567, { } },
    { "ISB_LINES_IN", 0x26, PMC_4567, { } },
    { "ITLB_INSERT_HPW", 0x28, PMC_4567, { } },
    { "ITLB_MISSES_FETCH", 0x27, PMC_4567, { } },
    { "L1_READ_FORCED_MISSES_RETIRED", 0x6b, PMC_4567, { } },
    { "L1_READ_MISSES_RETIRED", 0x66, PMC_4567, { } },
    { "L1_READS_RETIRED", 0x64, PMC_4567, { } },
    { "L1_DEMAND_READS", 0x20, PMC_4567, { } },
    { "L1I_FILLS", 0x21, PMC_4567, { } },
    { "L1I_PREFETCH_READS", 0x24, PMC_4567, { } },
    { "L2_DATA_REFERENCES", 0x69, PMC_4567, 
      { { "ALL", 0x3 }, { "READS", 0x1 }, { "WRITES", 0x2 } }
    },
    { "L2_FLUSH_DETAILS", 0x77, PMC_4567, 
      { { "ALL", 0x7 }, 
	{ "L2_ST_BUFFER_FLUSH", 0x1 }, 
	{ "L2_ADDR_CONFLICT", 0x2 },
	{ "L2_BUS_REJECT", 0x4 }, 
	{ "L2_FULL_FLUSH", 0x8 } }
    },
    { "L2_INST_DEMAND_READS", 0x22, PMC_4567, { } },
    { "L2_INST_PREFETCH_READS", 0x25, PMC_4567, { } },
    { "L2_MISSES", 0x6a, PMC_4567, { } },
    { "L2_REFERENCES", 0x68, PMC_4567, { } },
    { "L3_LINES_REPLACED", 0x7f, PMC_4567, { } },
    { "L3_MISSES", 0x7c, PMC_4567, { } },
    { "L3_READS.ALL_READS", 0x7d, PMC_4567, 
      { { "ALL", 0xf }, { "HIT", 0xd }, { "MISS", 0xe } }
    },
    { "L3_READS.DATA_READS", 0x7d, PMC_4567, 
      { { "ALL", 0xb }, { "HIT", 0x9 }, { "MISS", 0xa } }
    },
    { "L3_READS.INST_READS", 0x7d, PMC_4567, 
      { { "ALL", 0x7 }, { "HIT", 0x5 }, { "MISS", 0x6 } }
    },
    { "L3_REFERENCES", 0x7b, PMC_4567, { } },
    { "L3_WRITES.ALL_WRITES", 0x7e, PMC_4567, 
      { { "ALL", 0xf }, { "HIT", 0xd }, { "MISS", 0xe } }
    },
    { "L3_WRITES.L2_WRITEBACK", 0x7e, PMC_4567, 
      { { "ALL", 0xb }, { "HIT", 0x9 }, { "MISS", 0xa } }
    },
    { "L3_WRITES.DATA_WRITES", 0x7e, PMC_4567, 
      { { "ALL", 0x7 }, { "HIT", 0x5 }, { "MISS", 0x6 } }
    },
    { "LOADS_RETIRED", 0x6c, PMC_4567, { } },
    { "MEMORY_CYCLE", 0x07, PMC_4567, { } },
    { "MISALIGNED_LOADS_RETIRED", 0x70, PMC_4567, { } },
    { "MISALIGNED_STORES_RETIRED", 0x71, PMC_4567, { } },
    { "NOPS_RETIRED", 0x30, PMC_45, { } },
    { "PIPELINE_ALL_FLUSH_CYCLE", 0x4, PMC_4567, { } },
    { "PIPELINE_BACKEND_FLUSH_CYCLE", 0x0, PMC_4567, { } },
    { "PIPELINE_FLUSH", 0x33, PMC_4567, 
      { { "ALL", 0xf }, 
	{ "IEU_FLUSH", 0x8 },
	{ "DTC_FLUSH", 0x4 },
	{ "L1D_WAYMP_FLUSH", 0x2 },
	{ "OTHER_FLUSH", 0x1 } }
    },
    { "PREDICATE_SQUASHED_RETIRED", 0x31, PMC_45, { } },
    { "RSE_LOADS_RETIRED", 0x72, PMC_4567, { } },
    { "RSE_REFERENCES_RETIRED", 0x65, PMC_4567, { } },
    { "STORES_RETIRED", 0x6d, PMC_4567, { } },
    { "UC_LOADS_RETIRED", 0x6e, PMC_4567, { } },
    { "UC_STORES_RETIRED", 0x6f, PMC_4567, { } },
    { "UNSTALLED_BACKEND_CYCLES", 0x05, PMC_4567, { } },
};

bool kdb_get_perfctr (pmc_itanium_t * pmc, word_t * pmc_mask)
{

    for (;;)
    {
	word_t ctr = get_dec ("Performance counter", ~0UL, "list");
	if (ctr == ABORT_MAGIC)
	    return false;

	if (ctr == ~0UL)
	{
	    // List performance counters.
	    word_t size = sizeof (perf_events) / sizeof (ia64_perf_event_t);
	    for (word_t i = 1; i <= (size + 1) / 2; i++)
	    {
		printf ("%3d - %32s", i, perf_events[i-1].name);
		if (i + ((size + 1) / 2) <= size)
		    printf ("%3d - %32s", i + ((size + 1) / 2),
			    perf_events[i + ((size + 1) / 2) - 1].name);
		printf ("\n");
	    }
	}
	else
	{
	    ctr--;

	    ia64_perf_event_t * p = &perf_events[ctr];

	    pmc->es = p->event_selector;
	    pmc->umask = 0;
	    *pmc_mask = p->pmc_mask;

	    // Select umask.
	    if (p->umask[0].name)
	    {
		word_t umask;
		do {
		    for (word_t i = 0; p->umask[i].name && i < 5; i++)
			printf("%d=%s, ", i, perf_events[ctr].umask[i].name);

		    umask = get_dec ("Select umask", 0, NULL);
		    if (umask == ABORT_MAGIC)
			return false;
		} while (umask >= 4 || (! p->umask[umask].name));

		pmc->umask = p->umask[umask].umask;
		printf ("Perf counter: %s.%s\n",
			p->name, p->umask[umask].name);
	    }
	    else
		printf ("Perf counter: %s\n", p->name);

	    return true;
	}
    } 
}

