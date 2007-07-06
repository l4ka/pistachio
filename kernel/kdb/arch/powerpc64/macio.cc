/****************************************************************************
 *
 * Copyright (C) 2003-2004,  National ICT Australia (NICTA)
 *
 * File path:	arch/powerpc64/ofio.cc
 * Description:	IBM Open Firmware RTAS and serial IO.
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
 * $Id: macio.cc,v 1.3 2006/11/17 17:16:07 skoglund Exp $
 *
 ***************************************************************************/

#include <debug.h>

#include <kdb/console.h>
#include INC_ARCH(string.h)
#include INC_ARCH(1275tree.h)	// the device tree
#include INC_GLUE(pgent_inline.h)

 #include INC_ARCH(msr.h)

#define NULL_NAME	"NULL"
#define SCCA_NAME	"scca"

static void switch_console( const char *name )
{
    for( int i = 0; kdb_consoles[i].name; i++ )
	if( !strcmp(kdb_consoles[i].name, name) )
	{
	    kdb_current_console = i;
	    return;
	}
}

/****************************************************************************
 *
 *    Null console support
 *
 ****************************************************************************/

volatile u8_t *scca_control = NULL;
volatile u8_t *scca_data = NULL;

static void putc_null( char c )
{
}

static char getc_null( bool block )
{
    return -1u;
}

/****************************************************************************
 *
 *    PowerMac Z85c30 Serial console support
 *
 ****************************************************************************/

SECTION( ".init" )
void init_serial_console()
{
    of1275_device_t *aliases;
    of1275_device_t *serial_dev;
    of1275_device_t *escc_dev;
    of1275_device_t *macio_dev;
    of1275_pci_assigned_addresses *macio_ranges;
    struct macio_addresses {
	u32_t	addr;
	u32_t	size;
    } *scca_ranges;

    char *path, *type;
    u32_t len;
    u64_t scca_phys;

    if (!(aliases = get_of1275_tree()->find( "/aliases" )))
	goto error;

    if (!aliases->get_prop( "scca", &path, &len ))
	goto error;

    if (!(serial_dev = get_of1275_tree()->find( path )))
	goto error;


    if (!(escc_dev = get_of1275_tree()->get_parent( serial_dev )))
	goto error;

    if (!escc_dev->get_prop( "device_type", (char**)&type, &len ))
	goto error;

    if (strncmp(type, "escc", 4)) {
	goto error;
    }

    if (!(macio_dev = get_of1275_tree()->get_parent( escc_dev )))
	goto error;

    if (!macio_dev->get_prop( "device_type", (char**)&type, &len ))
	goto error;

    if (strncmp(type, "mac-io", 6)) {
	goto error;
    }

    if (!macio_dev->get_prop( "assigned-addresses", (char**)&macio_ranges, &len ))
	goto error;

    if (!serial_dev->get_prop( "reg", (char**)&scca_ranges, &len ))
	goto error;

    scca_phys = ((u64_t)macio_ranges[0].pci.addr.a_mid << 32) |
	    macio_ranges[0].pci.addr.a_lo;
    scca_phys += scca_ranges[0].addr;

    {
	pgent_t pg;
	pgent_t::pgsize_e size = pgent_t::size_4k;

	/* Create a page table entry, noexecute, nocache */
	pg.set_entry( get_kernel_space(), size, (addr_t)(scca_phys & ~(0xfff)),
		      6, pgent_t::cache_inhibit, true );
    
	scca_control = (u8_t *)((word_t)scca_phys | DEVICE_AREA_START);
	scca_data = scca_control + 0x10;

	/* Insert the kernel mapping, bolted */
	get_pghash()->insert_mapping( get_kernel_space(),
			(addr_t)((word_t)scca_control & ~(0xfff)), &pg, size, true );
    }

    printf( "Mapping serial at %p\n", (word_t)scca_control );
    switch_console( SCCA_NAME );
    return;

error:
    printf( "Serial init error\n\r" );
    return;
}

/* Read registers */
#define REG_STATUS  0

/* Status register bits */
#define RX_AVAIL    0x01
#define ZERO_COUNT  0x02
#define TX_EMPTY    0x04

static u8_t scca_read_reg( u8_t reg )
{
    if (reg != 0)
	*scca_control = reg;
    return *scca_control;
}

static void scca_write_reg( u8_t reg, u8_t data )
{
    if (reg != 0)
	*scca_control = reg;
    *scca_control = data;
}

static void putc_serial( char c )
{
    while (!(scca_read_reg( REG_STATUS ) & TX_EMPTY));
    
    *scca_data = c;

    if (c == '\n')
	putc_serial('\r');
}

#if 0
extern word_t putcmode_call;
extern word_t origmode_call;
/*static */void putc_serial( char c )
{
    u32_t i;

    for (i = 0; i < 5000000; i++);

word_t msr = ppc64_get_msr();
    /* Jump into real mode */
    asm volatile (
	"   mtsrr0	%0		\n"	/* Set the jump address */
	"   mtsrr1	%1		\n"	/* Set the target msr	*/
	"   rfid			\n"	/* Jump			*/

	"putcmode_call:			\n"	/* Jump target		*/
	:: "r" (PTRRELOC(&putcmode_call)), "r" (MSR_REAL_MODE)
	: "memory"
    );

    __asm__ __volatile__ (
	"   li	    %%r0,   -1		\n"
	"   mfspr   %%r3,   1012	\n" /* HID4*/
	"   rldimi  %%r3,   %%r0, 40, 23  \n"	/* set rm_ci*/
	"   sync			\n"
	"   mtspr   1012,   %%r3	\n"
	"   isync			\n"
	"   sync			\n"
	::: "r0", "r3"
    );
    *scca_data = c;
    __asm__ __volatile__ (
	"   li	    %%r0,   0		\n"
	"   mfspr   %%r3,   1012	\n" /* HID4 */
	"   rldimi  %%r3,   %%r0, 40, 23  \n"	/* clear rm_ci*/
	"   sync			\n"
	"   mtspr   1012,   %%r3	\n"
	"   isync			\n"
	"   sync			\n"
	::: "r0", "r3"
    );
    /* Jump back to orig mode */
    asm volatile (
	"   mtsrr0	%0		\n"	/* Set the jump address */
	"   mtsrr1	%1		\n"	/* Set the target msr	*/
	"   rfid			\n"	/* Jump			*/

	"origmode_call:			\n"	/* Jump target		*/
	:: "r" (&origmode_call), "r" (msr)
	: "memory"
    );
    if (c == '\n')
	putc_serial('\r');
}
#endif

#if 0
extern word_t pputcmode_call;
extern word_t porigmode_call;
/*static */void putc_serial_p( char c )
{
    u32_t i;
    for (i = 0; i < 10000000; i++);
#if 0
    *scca_data = c;
#else

word_t msr = ppc64_get_msr();
    /* Jump into real mode */
    asm volatile (
	"   mtsrr0	%0		\n"	/* Set the jump address */
	"   mtsrr1	%1		\n"	/* Set the target msr	*/
	"   rfid			\n"	/* Jump			*/

	"pputcmode_call:			\n"	/* Jump target		*/
	:: "r" (PTRRELOC(&pputcmode_call)), "r" (MSR_REAL_MODE)
	: "memory"
    );

    __asm__ __volatile__ (
	"   li	    %%r0,   -1		\n"
	"   mfspr   %%r3,   1012	\n" /* HID4*/
	"   rldimi  %%r3,   %%r0, 40, 23  \n"	/* set rm_ci*/
	"   sync			\n"
	"   mtspr   1012,   %%r3	\n"
	"   isync			\n"
	"   sync			\n"
	::: "r0", "r3"
    );
    *scca_data = c;
    __asm__ __volatile__ (
	"   li	    %%r0,   0		\n"
	"   mfspr   %%r3,   1012	\n" /* HID4 */
	"   rldimi  %%r3,   %%r0, 40, 23  \n"	/* clear rm_ci*/
	"   sync			\n"
	"   mtspr   1012,   %%r3	\n"
	"   isync			\n"
	"   sync			\n"
	::: "r0", "r3"
    );
    /* Jump back to orig mode */
    asm volatile (
	"   mtsrr0	%0		\n"	/* Set the jump address */
	"   mtsrr1	%1		\n"	/* Set the target msr	*/
	"   rfid			\n"	/* Jump			*/

	"porigmode_call:			\n"	/* Jump target		*/
	:: "r" (PTRRELOC(&porigmode_call)), "r" (msr)
	: "memory"
    );
#endif
}
#endif

static char getc_serial( bool block )
{
    if (!(scca_read_reg( REG_STATUS ) & RX_AVAIL))
    {
	if (!block)
	    return -1u;

	while (!(scca_read_reg( REG_STATUS ) & RX_AVAIL));
    }
    
    return *scca_data;
}


#if defined(CONFIG_KDB_BREAKIN)
void kdebug_check_breakin (void)
{
    if (getc(false) != (char)-1u)
	enter_kdebug("breakin");
}
#endif

/****************************************************************************
 *
 *    Console registration
 *
 ****************************************************************************/

kdb_console_t kdb_consoles[] = {
    { NULL_NAME, NULL, putc_null, getc_null },
    { SCCA_NAME, NULL, putc_serial, getc_serial },
    KDB_NULL_CONSOLE
};

word_t kdb_current_console = 0;

