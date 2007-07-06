
#if defined(CONFIG_SMP)
#include <debug.h>
#include <kdb/tracepoints.h>
#include INC_API(smp.h)

#include INC_GLUE(config.h)
#include INC_GLUE(idt.h)

#include INC_ARCH(apic.h)
#include INC_ARCH(trapgate.h)

#include INC_ARCH(debug.h)

#include INC_API(tcb.h)

DECLARE_TRACEPOINT(SMP_IPI_SEND);
DECLARE_TRACEPOINT(SMP_IPI_RECEIVE);

static local_apic_t<APIC_MAPPINGS> apic;

AMD64_EXC_NO_ERRORCODE(smp_trigger_ipi, 0)
{
    TRACEPOINT_TB(SMP_IPI_RECEIVE, ("smp ipi receive %d", get_current_cpu()),
		  printf("smp ipi receive %d", get_current_cpu()));
    apic.EOI();
    process_xcpu_mailbox();

}

void smp_xcpu_trigger(cpuid_t cpu)
{
    TRACEPOINT_TB (SMP_IPI_SEND, ("smp ipi send %d->%d", get_current_cpu(), cpu), 
		   printf ("smp ipi send %d->%d", get_current_cpu(), cpu));
    apic.send_ipi(cpu, IDT_LAPIC_IPI);
}

void init_xcpu_handling()
{  
    idt.add_int_gate(IDT_LAPIC_IPI, smp_trigger_ipi);
}

#endif /* CONFIG_SMP */
