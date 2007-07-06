/*********************************************************************
 *                
 * Copyright (C) 2002-2006,  Karlsruhe University
 *                
 * File path:     api/v4/kernelinterface.cc
 * Description:   defines the kernel interface page
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
 * $Id: kernelinterface.cc,v 1.52 2006/10/27 17:45:55 reichelt Exp $
 *                
 ********************************************************************/

#include <debug.h>
#include INC_API(kernelinterface.h)
#include INC_API(memdesc.h)
#include INC_API(user.h)
#include INC_GLUE(config.h)
#include <version.h>

#if defined(CONFIG_DEBUG) && !defined(KIP_SECONDARY)
extern void kdebug_init();
extern void kdebug_entry(void *);
# define KDEBUG_INIT	kdebug_init
# define KDEBUG_ENTRY	kdebug_entry
#else
# define KDEBUG_INIT	(0)
# define KDEBUG_ENTRY	(0)
#endif

// Experimental Version X.2
#define KIP_API_VERSION   {SHUFFLE2(0x05, 0x84)}

// Thread info (sign. bits (arch-specific), sysbase, userbase)
#define KIP_THREAD_INFO   {SHUFFLE3(VALID_THREADNO_BITS, 0, 0)}

// Put processor descriptors directly after KIP
#define KIP_PROCESSOR_DESC_PTR (sizeof (kernel_interface_page_t))

// Processor info field.
#define KIP_PROCESSOR_INFO {SHUFFLE2(0, KIP_PROC_DESC_LOG2SIZE)}

// Put kernel description after processor descriptors
#define KIP_DESCRIPTION_PTR (sizeof (kernel_interface_page_t) + sizeof(procdesc_t) * CONFIG_SMP_MAX_CPUS)

extern "C" {

// Memory info is calculated by linker script
#if !defined(KIP_MEMDESCS_SIZE)
# define KIP_MEMDESCS_SIZE _memory_descriptors_size
#endif
#if !defined(KIP_MEMDESCS_RAW)
# define KIP_MEMDESCS_RAW _memory_descriptors_raw
#endif
extern word_t KIP_MEMDESCS_SIZE[];
extern word_t KIP_MEMDESCS_RAW[];
#define KIP_MEMORY_INFO {{raw: (addr_word_t) &KIP_MEMDESCS_RAW}}
  
kernel_interface_page_t KIP UNIT(KIP_SECTION) =
{
    {{string:{'L','4',230,'K'}}}, // Magic word
    KIP_API_VERSION,
    KIP_API_FLAGS,		// API flags
    KIP_DESCRIPTION_PTR,	// kernel description pointer

    KDEBUG_INIT,		// kdebug init
    KDEBUG_ENTRY,		// kdebug entry
    {0, 0},			// kdebug memory region

    {0, 0, {0, 0}},		// sigma0
    {0, 0, {0, 0}},		// sigma1
    {0, 0, {0, 0}},		// root task

    0,				// reserved0
    KIP_MEMORY_INFO,		// memory info
    {0, 0},			// kdebug config

    {0, 0},			// main memory
    {0, 0},			// reserved memory 0

    {0, 0},			// reserved memory 1
    {0, 0},			// dedicated memory 0

    {0, 0},			// dedicated memory 1
    {0, 0},			// dedicated memory 2

    {0, 0},			// dedicated memory 3
    {0, 0},			// dedicated memory 4

    {0, 0},			// reserved1
    KIP_UTCB_INFO,		// UTCB info
    KIP_KIP_AREA,		// KIP area info

    {0, 0},			// reserved 2
    0,				// boot info
    KIP_PROCESSOR_DESC_PTR,	// processor description pointer

    {0, 0},			// clock info
    KIP_THREAD_INFO,
    KIP_ARCH_PAGEINFO,		// page info
    KIP_PROCESSOR_INFO,		// processor info

    0, 0, 0, 0,			// privileged syscalls
    0, 0, 0, 0, 0, 0, 0,	// unprivileged syscalls
    {0, 0, 0, 0, 0},          	// reserved3 (5 words)

    0, 0, 0, 0			// architecture specific syscalls
};

}

extern const kernel_descriptor_t kdesc UNIT(KIP_SECTION ".kdesc") =
{
    {SHUFFLE2(subid:2, id:4)},
    { KERNELGENDATE },
    {SHUFFLE3(KERNEL_VERSION,KERNEL_SUBVERSION,KERNEL_SUBSUBVERSION)},
    {{{'U','K','a',' '}}}
};

/*
 * Generate kernel version string.
 */
__asm__(".section .data." KIP_SECTION ".versionparts,\"aw\",%progbits	\n"
	"kernel_version_string:						\n"
	".string \"L4Ka::Pistachio - built on "__DATE__" "__TIME__
	" by "__USER__" using gcc version "__VERSION__"\" 		\n"
	".previous							\n");

/*
 * Terminate the feature string list.
 */
__asm__ (".section .data." KIP_SECTION ".features.end, \"aw\", %progbits	\n"
	 ".string \"\"								\n"
	 ".previous								\n");

/**
 * processor descriptors
 */
procdesc_t processor_descriptors[CONFIG_SMP_MAX_CPUS] UNIT (KIP_SECTION ".pdesc");

procdesc_t * processor_info_t::get_procdesc (word_t num)
{
    return num < CONFIG_SMP_MAX_CPUS ? &processor_descriptors[num] : NULL;
}

/*
 * Make room for at least some memory descriptors.
 */
#if !defined(KIP_MIN_MEMDESCS)
# define KIP_MIN_MEMDESCS (8)
#endif

#if !defined(KIP_MEMDESCS)
# define KIP_MEMDESCS memory_descriptors
#endif

extern "C" {
    memdesc_t KIP_MEMDESCS[KIP_MIN_MEMDESCS] UNIT (KIP_SECTION ".mdesc");
}

/**
 * Grab memory descriptor from kernel interface page.
 *
 * @param num	the descritor number to return
 *
 * @return pointer to memory descriptor, or NULL if NUM is out of range
 */
memdesc_t * memory_info_t::get_memdesc (word_t num)
{
    if (num >= n)
	return NULL;
    return &KIP_MEMDESCS[num];
}


/**
 * Insert a memory descriptor into kernel interface page.
 *
 * @param type		descriptor type
 * @param subtype	descriptor subtype
 * @param virt		true if descriptor is for virtual memory
 * @param low		low address of memory region
 * @param high		high address of memory region
 *
 * @return true if insertion was successful, false otherwise
 */
bool memory_info_t::insert (memdesc_t::type_e type, word_t subtype,
			    bool virt, addr_t low, addr_t high)
{
    const word_t max_desc = (addr_word_t) &KIP_MEMDESCS_SIZE;

    if (n >= max_desc)
    {
	printf ("Memory descriptor overflow (max=%d, n=%d)\n", max_desc, n);
	enter_kdebug ("memdesc overflow");
	return false;
    }

    memdesc_t * md = get_memdesc (n++);
    md->set (type, subtype, virt, low, (addr_t) ((word_t) high - 1));

    return true;
}


extern "C" {
    extern char kernel_version_string[] UNUSED;
}

/**
 * Print kernel version string.
 */
void SECTION(".init") init_hello (void)
{
    printf ("\n" TXT_BRIGHT TXT_FG_YELLOW "%s" TXT_NORMAL "\n",
	    kernel_version_string);
}

#if !defined(ARCH_SYSCALL0)
#define ARCH_SYSCALL0 	0
#endif

#if !defined(ARCH_SYSCALL1)
#define ARCH_SYSCALL1 	0
#endif

#if !defined(ARCH_SYSCALL2)
#define ARCH_SYSCALL2 	0
#endif

#if !defined(ARCH_SYSCALL3)
#define ARCH_SYSCALL3 	0
#endif

void SECTION(".init") kernel_interface_page_t::init()
{
#if defined(KIP_SYSCALL)
#define SET_KIP_SYSCALL(x) \
    this->x##_syscall = KIP_SYSCALL(user_##x)

    SET_KIP_SYSCALL(space_control);
    SET_KIP_SYSCALL(thread_control);
    SET_KIP_SYSCALL(processor_control);
    SET_KIP_SYSCALL(memory_control);
    SET_KIP_SYSCALL(ipc);
    SET_KIP_SYSCALL(lipc);
    SET_KIP_SYSCALL(unmap);
    SET_KIP_SYSCALL(exchange_registers);
    SET_KIP_SYSCALL(system_clock);
    SET_KIP_SYSCALL(thread_switch);
    SET_KIP_SYSCALL(schedule);
#endif

#define SET_KIP_ARCH_SYSCALL(n) \
    this->arch_syscall##n = ARCH_SYSCALL##n

    SET_KIP_ARCH_SYSCALL (0);
    SET_KIP_ARCH_SYSCALL (1);
    SET_KIP_ARCH_SYSCALL (2);
    SET_KIP_ARCH_SYSCALL (3);
}


