/*********************************************************************
 *                
 * Copyright (C) 2002,  University of New South Wales
 *                
 * File path:     platform/erpcn01/gt64115.h
 * Description:   Galileo GT64115 definitions
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
 * $Id: gt64115.h,v 1.4 2003/09/24 19:04:55 skoglund Exp $
 *                
 ********************************************************************/
/*
 * Carl van Schaik - Openfuel Pty Ltd.
 */

#ifndef _gt64115_h 
#define _gt64115_h

#define GT64115

/* Important remark:
 * All 0x0 defines have no meaning for the GT64115 and are here for compatability */ 


#define XKPHYS2     0x9000000000000000 /* Uncached */

/****************************************/
/* Default Memory Map					*/
/****************************************/

#define GTREGS_BASE											0xB4000000
#define BOOT_BASE											0xBFC00000
#define SRAM_BASE											0xBC000000
#define SDRAM_BASE											0xA0000000

/****************************************/
/* CPU Configuration 					*/
/****************************************/

#define GT_CPU_INTERFACE_CONFIGURATION						0x000

#define gt_CPU_CFG_WriteMode								(1<<11)
#define gt_CPU_CFG_Endianess								(1<<12)
#define gt_CPU_CFG_PCI_Override(x)							((x & 3)<<13)
#define gt_PCI_NO_OVERIDE									0x00
#define gt_PCI_1GB_MEM										0x01
#define gt_PCI_2GB_MEM										0x02
#define gt_PCI_NO_MATCH										0x03
#define gt_CPU_CFG_Stop_Retry								(1<<15)

/****************************************/
/* Processor Address Space				*/
/****************************************/

#define GT_SCS_1_0_LOW_DECODE_ADDRESS						0x008
#define GT_SCS_1_0_HIGH_DECODE_ADDRESS						0x010
#define GT_SCS_3_2_LOW_DECODE_ADDRESS						0x018
#define GT_SCS_3_2_HIGH_DECODE_ADDRESS						0x020
#define GT_CS_2_0_LOW_DECODE_ADDRESS						0x028
#define GT_CS_2_0_HIGH_DECODE_ADDRESS						0x030
#define GT_CS_3_BOOTCS_LOW_DECODE_ADDRESS					0x038
#define GT_CS_3_BOOTCS_HIGH_DECODE_ADDRESS 					0x040
#define GT_PCI_0I_O_LOW_DECODE_ADDRESS						0x048
#define GT_PCI_0I_O_HIGH_DECODE_ADDRESS						0x050
#define GT_PCI_0MEMORY0_LOW_DECODE_ADDRESS					0x058
#define GT_PCI_0MEMORY0_HIGH_DECODE_ADDRESS					0x060
#define GT_PCI_0MEMORY1_LOW_DECODE_ADDRESS					0x080
#define GT_PCI_0MEMORY1_HIGH_DECODE_ADDRESS					0x088
#define GT_PCI_1I_O_LOW_DECODE_ADDRESS						0x0
#define GT_PCI_1I_O_HIGH_DECODE_ADDRESS						0x0
#define GT_PCI_1MEMORY0_LOW_DECODE_ADDRESS					0x0
#define GT_PCI_1MEMORY0_HIGH_DECODE_ADDRESS					0x0
#define GT_PCI_1MEMORY1_LOW_DECODE_ADDRESS					0x0
#define GT_PCI_1MEMORY1_HIGH_DECODE_ADDRESS					0x0
#define GT_INTERNAL_SPACE_DECODE							0x068
#define GT_CPU_BUS_ERROR_ADDRESS 							0x070
#define GT_SCS_1_0_ADDRESS_REMAP							0x090
#define GT_SCS_3_2_ADDRESS_REMAP							0x098
#define GT_CS_2_0_ADDRESS_REMAP								0x0A0
#define GT_CS_3_BOOTCS_ADDRESS_REMAP						0x0A8
#define GT_PCI_0I_O_ADDRESS_REMAP							0x0B0
#define GT_PCI_0MEMORY0_ADDRESS_REMAP						0x0B8
#define GT_PCI_0MEMORY1_ADDRESS_REMAP						0x0C0
#define GT_PCI_1IO_ADDRESS_REMAP							0x0
#define GT_PCI_1MEMORY0_ADDRESS_REMAP						0x0
#define GT_PCI_1MEMORY1_ADDRESS_REMAP						0x0

/****************************************/
/* SDRAM and Device Address Space		*/
/****************************************/
	
#define GT_SCS_0_LOW_DECODE_ADDRESS							0x400
#define GT_SCS_0_HIGH_DECODE_ADDRESS						0x404
#define GT_SCS_1_LOW_DECODE_ADDRESS							0x408
#define GT_SCS_1_HIGH_DECODE_ADDRESS						0x40C
#define GT_SCS_2_LOW_DECODE_ADDRESS							0x410
#define GT_SCS_2_HIGH_DECODE_ADDRESS						0x414
#define GT_SCS_3_LOW_DECODE_ADDRESS							0x418
#define GT_SCS_3_HIGH_DECODE_ADDRESS						0x41C
#define GT_CS_0_LOW_DECODE_ADDRESS							0x420
#define GT_CS_0_HIGH_DECODE_ADDRESS							0x424
#define GT_CS_1_LOW_DECODE_ADDRESS							0x428
#define GT_CS_1_HIGH_DECODE_ADDRESS							0x42C
#define GT_CS_2_LOW_DECODE_ADDRESS							0x430
#define GT_CS_2_HIGH_DECODE_ADDRESS							0x434
#define GT_CS_3_LOW_DECODE_ADDRESS							0x438
#define GT_CS_3_HIGH_DECODE_ADDRESS							0x43C
#define GT_BOOTCS_LOW_DECODE_ADDRESS						0x440
#define GT_BOOTCS_HIGH_DECODE_ADDRESS						0x444
#define GT_ADDRESS_DECODE_ERROR								0x470
#define GT_ADDRESS_DECODE									0x47C

/****************************************/
/* SDRAM Configuration					*/
/****************************************/

#define GT_SDRAM_CONFIGURATION	 							0x448
#define gt_SDRAM_CFG_RefIntCnt								(x & 0x00003FFF)
#define gt_SDRAM_CFG_RMW									(1<<15)
#define gt_SDRAM_StagRef									(1<<16)
#define gt_SDRAM_Mask										0xF8000000

#define GT_SDRAM_OPERATION_MODE								0x474
#define gt_SDRAM_OP_Normal									0x00
#define gt_SDRAM_OP_NOP										0x01
#define gt_SDRAM_OP_Precharge								0x02
#define gt_SDRAM_OP_Mode_Reg_CMD							0x03
#define gt_SDRAM_OP_CBR_Cycle								0x04

#define GT_SDRAM_ADDRESS_DECODE								0x47C

/****************************************/
/* SDRAM Parameters						*/
/****************************************/
			
#define GT_SDRAM_BANK0PARAMETERS							0x44C
#define GT_SDRAM_BANK1PARAMETERS							0x450
#define GT_SDRAM_BANK2PARAMETERS							0x454
#define GT_SDRAM_BANK3PARAMETERS							0x458

#define gt_SDRAM_PARAM_2CYC									0x01
#define gt_SDRAM_PARAM_3CYC									0x02
#define gt_SDRAM_PARAM_FlowThrough							(1<<2)
#define gt_SDRAM_PARAM_SRAS_Precharge_2CYC					0x00
#define gt_SDRAM_PARAM_SRAS_Precharge_3CYC					(1<<3)
#define gt_SDRAM_PARAM_Interleave_2way						0x00
#define gt_SDRAM_PARAM_Interleave_4way						(1<<5)
#define gt_SDRAM_PARAM_No_Parity							0x00
#define gt_SDRAM_PARAM_Parity								(1<<8)
#define gt_SDRAM_PARAM_No_Bypass							0x00
#define gt_SDRAM_PARAM_Bypass								(1<<9)
#define gt_SDRAM_PARAM_RAS_CAS_2CYC							0x00
#define gt_SDRAM_PARAM_RAS_CAS_3CYC							(1<<10)
#define gt_SDRAM_PARAM_16Mbit								0x00
#define gt_SDRAM_PARAM_128Mbit								(1<<11)
#define gt_SDRAM_PARAM_Burst_8								0x00
#define gt_SDRAM_PARAM_Burst_4								(1<<13)

/****************************************/
/* Device Parameters					*/
/****************************************/

#define GT_DEVICE_BANK0PARAMETERS							0x45C
#define GT_DEVICE_BANK1PARAMETERS							0x460
#define GT_DEVICE_BANK2PARAMETERS							0x464
#define GT_DEVICE_BANK3PARAMETERS							0x468
#define GT_DEVICE_BOOT_BANK_PARAMETERS						0x46C

#define gt_DEV_PARAM_TurnOff(x)								(x & 0x07)
#define gt_DEV_PARAM_AccToFirst(x)							((x & 0x07)<<3)
#define gt_DEV_PARAM_AccToNext(x)							((x & 0x07)<<7)
#define gt_DEV_PARAM_ALEtoWR(x)								((x & 0x07)<<11)
#define gt_DEV_PARAM_WrActive(x)							((x & 0x07)<<14)
#define gt_DEV_PARAM_WrHigh(x)								((x & 0x07)<<17)
#define gt_DEV_PARAM_8_bit									0x00
#define gt_DEV_PARAM_16_bit									(0x01<<20)
#define gt_DEV_PARAM_32_bit									(0x02<<20)
#define gt_DEV_PARAM_No_Parity								0x00
#define gt_DEV_PARAM_Parity									(1<<30)

/****************************************/
/* MPP Configuration					*/
/****************************************/

#define GT_MULTI_PURPOSE_PINS_CONFIGURATION				   	0x480
#define gt_MPP_Config(n,x)								((x & 0x7) << (n*3)
#define gt_MPP_0_3_DMAReq									0x0
#define gt_MPP_EOT											0x1
#define gt_MPP_BypsOE										0x4
#define gt_MPP_4_7_Parity									0x0
#define gt_MPP_0_MREQ										0x3
#define gt_MPP_1_MGNT										0x3
#define gt_MPP_2_TREQ										0x3
#define gt_MPP_4_MREQ										0x3
#define gt_MPP_5_MGNT										0x3
#define gt_MPP_6_TREQ										0x3

/****************************************/
/* DMA Record							*/
/****************************************/

#define GT_CHANNEL0_DMA_BYTE_COUNT							0x800
#define GT_CHANNEL1_DMA_BYTE_COUNT	 						0x804
#define GT_CHANNEL2_DMA_BYTE_COUNT	 						0x808
#define GT_CHANNEL3_DMA_BYTE_COUNT	 						0x80C
#define GT_CHANNEL0_DMA_SOURCE_ADDRESS						0x810
#define GT_CHANNEL1_DMA_SOURCE_ADDRESS						0x814
#define GT_CHANNEL2_DMA_SOURCE_ADDRESS						0x818
#define GT_CHANNEL3_DMA_SOURCE_ADDRESS						0x81C
#define GT_CHANNEL0_DMA_DESTINATION_ADDRESS					0x820
#define GT_CHANNEL1_DMA_DESTINATION_ADDRESS					0x824
#define GT_CHANNEL2_DMA_DESTINATION_ADDRESS					0x828
#define GT_CHANNEL3_DMA_DESTINATION_ADDRESS					0x82C
#define GT_CHANNEL0NEXT_RECORD_POINTER						0x830
#define GT_CHANNEL1NEXT_RECORD_POINTER						0x834
#define GT_CHANNEL2NEXT_RECORD_POINTER						0x838
#define GT_CHANNEL3NEXT_RECORD_POINTER						0x83C
#define GT_CHANNEL0CURRENT_DESCRIPTOR_POINTER				0x870
#define GT_CHANNEL1CURRENT_DESCRIPTOR_POINTER				0x874
#define GT_CHANNEL2CURRENT_DESCRIPTOR_POINTER				0x878
#define GT_CHANNEL3CURRENT_DESCRIPTOR_POINTER				0x87C

/****************************************/
/* DMA Channel Control					*/
/****************************************/

#define GT_CHANNEL0CONTROL									0x840
#define GT_CHANNEL1CONTROL									0x844
#define GT_CHANNEL2CONTROL									0x848
#define GT_CHANNEL3CONTROL									0x84C

#define gt_DMA_CTL_FlyBy									(1<<0)
#define gt_DMA_CTL_RdFly									0x00
#define gt_DMA_CTL_WrFly									(1<<1)
#define gt_DMA_CTL_Src_Inc									0x00
#define gt_DMA_CTL_Src_Dec									(1<<2)
#define gt_DMA_CTL_Src_Hold									(2<<2)
#define gt_DMA_CTL_Dst_Inc									0x00
#define gt_DMA_CTL_Dst_Dec									(1<<4)
#define gt_DMA_CTL_Dst_Hold									(2<<4)
#define gt_DMA_CTL_Limit_1B									(5<<6)
#define gt_DMA_CTL_Limit_2B									(6<<6)
#define gt_DMA_CTL_Limit_4B									(0<<6)
#define gt_DMA_CTL_Limit_8B									(1<<6)
#define gt_DMA_CTL_Limit_16B								(3<<6)
#define gt_DMA_CTL_Limit_32B								(7<<7)
#define gt_DMA_CTL_Chained									0x00
#define gt_DMA_CTL_Non_Chained								(1<<9)
#define gt_DMA_CTL_Int_Normal								0x00
#define gt_DMA_CTL_Int_Chained								(1<<10)
#define gt_DMA_CTL_Transfer_Demand							0x00
#define gt_DMA_CTL_Transfer_Block							(1<<11)
#define gt_DMA_CTL_Ch_Disable								0x00
#define gt_DMA_CTL_Ch_Enable								(1<<12)
#define gt_DMA_CTL_FetNexRec								(1<<13)
#define gt_DMA_CTL_DMAAct_Mask								(1<<14)
#define gt_DMA_CTL_SDA_Src									0x00
#define gt_DMA_CTL_SDA_Dst									(1<<15)
#define gt_DMA_CTL_Mask_DMA									(1<<16)
#define gt_DMA_CTL_CDE										(1<<17)
#define gt_DMA_CTL_EOT_En									(1<<18)
#define gt_DMA_CTL_EOT_Int_En								(1<<19)
#define gt_DMA_CTL_Abort									(1<<20)
#define gt_DMA_CTL_SLP_PCI									(1<<21)
#define gt_DMA_CTL_DLP_PCI									(1<<23)
#define gt_DMA_CTL_RLP_PCI									(1<<25)
#define gt_DMA_CTL_DMA_Request_Ext							0x00
#define gt_DMA_CTL_DMA_Request_Timer						(1<<28)

/****************************************/
/* DMA Arbiter							*/
/****************************************/

#define GT_ARBITER_CONTROL									0x860

#define gt_ARB_10_Round_Robin								0x00
#define gt_ARB_10_Priority1									0x01
#define gt_ARB_10_Priority0									0x02
#define gt_ARB_23_Round_Robin								0x00
#define gt_ARB_23_Priority3									(0x01<<2)
#define gt_ARB_23_Priority2									(0x02<<2)
#define gt_ARB_GRP_Round_Robin								0x00
#define gt_ARB_GRP_Priority23								(0x01<<4)
#define gt_ARB_GRP_Priority01								(0x02<<4)
#define gt_ARB_PrioOpt										(1<<6)

/****************************************/
/* Timer_Counter 						*/
/****************************************/

#define GT_TIMER_COUNTER0									0x850
#define GT_TIMER_COUNTER1									0x854
#define GT_TIMER_COUNTER2									0x858
#define GT_TIMER_COUNTER3									0x85C
#define GT_TIMER_COUNTER_CONTROL							0x864

#define gt_TC_CTL_T0_En										(1<<0)
#define gt_TC_CTL_T0_Counter								0x00
#define gt_TC_CTL_T0_Timer									(1<<1)
#define gt_TC_CTL_T1_En										(1<<2)
#define gt_TC_CTL_T1_Counter								0x00
#define gt_TC_CTL_T1_Timer									(1<<3)
#define gt_TC_CTL_T2_En										(1<<4)
#define gt_TC_CTL_T2_Counter								0x00
#define gt_TC_CTL_T2_Timer									(1<<5)
#define gt_TC_CTL_T3_En										(1<<6)
#define gt_TC_CTL_T3_Counter								0x00
#define gt_TC_CTL_T3_Timer									(1<<7)

/****************************************/
/* PCI Internal  						*/
/****************************************/

#define GT_PCI_0COMMAND										0xC00
#define gt_PCI_0CMD_NoByteSwap								(1<<0)
#define gt_PCI_0CMD_SyncMode_Default						0x00
#define gt_PCI_0CMD_SyncMode_Semi							(0x1<<1)
#define gt_PCI_0CMD_SyncMode_Sync							(0x2<<1)
#define gt_PCI_0CMD_RemapWrDis								(1<<16)

#define GT_PCI_0TIMEOUT_RETRY								0xC04
#define gt_PCI_OTimeout0(x)									(x & 0xFF)
#define gt_PCI_0Timeout1(x)									((x & 0xFF)<<8)
#define gt_PCI_0RetryCtr(x)									((x & 0xFF)<<16)

#define GT_PCI_0SCS_1_0_BANK_SIZE							0xC08
#define GT_PCI_0SCS_3_2_BANK_SIZE							0xC0C
#define GT_PCI_0CS_2_0_BANK_SIZE							0xC10
#define GT_PCI_0CS_3_BOOTCS_BANK_SIZE						0xC14
#define GT_PCI_0BASE_ADDRESS_REGISTERS_ENABLE 				0xC3C
#define gt_PCI_0BAR_DIS_BootCS_SwCS3						(1<<0)
#define gt_PCI_0BAR_DIS_SwSCS3_2							(1<<1)
#define gt_PCI_0BAR_DIS_SwSCS1_0							(1<<2)
#define gt_PCI_0BAR_DIS_IntIO								(1<<3)
#define gt_PCI_0BAR_DIS_IntME								(1<<4)
#define gt_PCI_0BAR_DIS_CS3_BootCS							(1<<5)
#define gt_PCI_0BAR_DIS_CS2_1_0								(1<<6)
#define gt_PCI_0BAR_DIS_SCS3_2								(1<<7)
#define gt_PCI_0BAR_DIS_SCS1_0								(1<<8)

#define GT_PCI_0SCS_1_0_BASE_ADDRESS_REMAP					0xC48
#define GT_PCI_0SCS_3_2_BASE_ADDRESS_REMAP					0xC4C
#define GT_PCI_0CS_2_0_BASE_ADDRESS_REMAP					0xC50
#define GT_PCI_0CS_3_BOOTCS_ADDRESS_REMAP					0xC54
#define GT_PCI_0SWAPPED_SCS_1_0_BASE_ADDRESS_REMAP			0xC58
#define GT_PCI_0SWAPPED_SCS_3_2_BASE_ADDRESS_REMAP			0xC5C
#define GT_PCI_0SWAPPED_CS_3_BOOTCS_BASE_ADDRESS_REMAP		0xC64
#define GT_PCI_0CONFIGURATION_ADDRESS 						0xCF8
#define gt_PCI_0CONF_RegNum(x)								((x & 0x1F)<<2)
#define gt_PCI_0CONF_FunctNum(x)							((x & 0x7)<<8)
#define gt_PCI_0CONF_DevNum(x)								((x & 0x1F)<<11)
#define gt_PCI_0CONF_BusNum(x)								((x & 0xFF)<<16)
#define gt_PCI_0CONF_ConfigEn								(1<<31)

#define GT_PCI_0CONFIGURATION_DATA_VIRTUAL_REGISTER			0xCFC
#define GT_PCI_0INTERRUPT_ACKNOWLEDGE_VIRTUAL_REGISTER		0xC34
#define GT_PCI_1COMMAND										0x0
#define GT_PCI_1TIMEOUT_RETRY								0x0
#define GT_PCI_1SCS_1_0_BANK_SIZE							0x0
#define GT_PCI_1SCS_3_2_BANK_SIZE							0x0
#define GT_PCI_1CS_2_0_BANK_SIZE							0x0
#define GT_PCI_1CS_3_BOOTCS_BANK_SIZE						0x0
#define GT_PCI_1BASE_ADDRESS_REGISTERS_ENABLE 				0x0
#define GT_PCI_1SCS_1_0_BASE_ADDRESS_REMAP					0x0
#define GT_PCI_1SCS_3_2_BASE_ADDRESS_REMAP					0x0
#define GT_PCI_1CS_2_0_BASE_ADDRESS_REMAP					0x0
#define GT_PCI_1CS_3_BOOTCS_ADDRESS_REMAP					0x0
#define GT_PCI_1SWAPPED_SCS_1_0_BASE_ADDRESS_REMAP			0x0
#define GT_PCI_1SWAPPED_SCS_3_2_BASE_ADDRESS_REMAP			0x0
#define GT_PCI_1SWAPPED_CS_3_BOOTCS_BASE_ADDRESS_REMAP		0x0
#define GT_PCI_1CONFIGURATION_ADDRESS 						0x0
#define GT_PCI_1CONFIGURATION_DATA_VIRTUAL_REGISTER			0x0
#define GT_PCI_1INTERRUPT_ACKNOWLEDGE_VIRTUAL_REGISTER		0x0

/****************************************/
/* Interrupts	  						*/
/****************************************/
			
#define GT_CPU_INTERRUPT_CAUSE_REGISTER						0xC18
#define gt_INT_MASK_Sum										(1<<0)
#define gt_INT_MASK_MemOut									(1<<1)
#define gt_INT_MASK_DMAOut									(1<<2)
#define gt_INT_MASK_CPUOut									(1<<3)
#define gt_INT_MASK_DMA0Comp								(1<<4)
#define gt_INT_MASK_DMA1Comp								(1<<5)
#define gt_INT_MASK_DMA2Comp								(1<<6)
#define gt_INT_MASK_DMA3Comp								(1<<7)
#define gt_INT_MASK_T0Exp									(1<<8)
#define gt_INT_MASK_T1Exp									(1<<9)
#define gt_INT_MASK_T2Exp									(1<<10)
#define gt_INT_MASK_T3Exp									(1<<11)
#define gt_INT_MASK_MasRdErr								(1<<12)
#define gt_INT_MASK_SlvWrErr								(1<<13)
#define gt_INT_MASK_MaxWrErr								(1<<14)
#define gt_INT_MASK_SlvRdErr								(1<<15)
#define gt_INT_MASK_AddrErr									(1<<16)
#define gt_INT_MASK_MemErr									(1<<17)
#define gt_INT_MASK_MasAbort								(1<<18)
#define gt_INT_MASK_TarAbort								(1<<19)
#define gt_INT_MASK_RetryCtr								(1<<20)
#define gt_INT_MASK_PMCInt									(1<<21)
#define gt_INT_MASK_CPUInt(x)								((x & 0xF)<<22)
#define gt_INT_MASK_PCIInt0									(1<<26)
#define gt_INT_MASK_PCIInt1									(1<<27)
#define gt_INT_MASK_PCIInt2									(1<<28)
#define gt_INT_MASK_PCIInt3									(1<<29)
#define gt_INT_MASK_CPUIntSum								(1<<30)
#define gt_INT_MASK_PCIIntSum								(1<<31)

#define GT_CPU_INTERRUPT_MASK_REGISTER						0xC1C
#define GT_PCI_0INTERRUPT_CAUSE_MASK_REGISTER				0xC24
#define GT_PCI_0SERR0_MASK									0xC28
#define gt_PCI_0SERR_MASK_AddrErr							(1<<0)
#define gt_PCI_0SERR_MASK_MasWrErr							(1<<1)
#define gt_PCI_0SERR_MASK_MasRdErr							(1<<2)
#define gt_PCI_0SERR_MASK_MemErr							(1<<3)
#define gt_PCI_0SERR_MASK_MasAbort							(1<<4)
#define gt_PCI_0SERR_MASK_TarAbort							(1<<5)

#define GT_PCI_1INTERRUPT_CAUSE_MASK_REGISTER				0x0
#define GT_PCI_1SERR0_MASK									0x0

/****************************************/
/* PCI Configuration   					*/
/****************************************/

#define GT_PCI_0DEVICE_AND_VENDOR_ID 						0x000
#define GT_PCI_0STATUS_AND_COMMAND							0x004
#define GT_PCI_0CLASS_CODE_AND_REVISION_ID 					0x008
#define GT_PCI_0BIST_HEADER_TYPE_LATENCY_TIMER_CACHE_LINE 	0x00C
#define GT_PCI_0SCS_1_0_BASE_ADDRESS	 					0x010
#define GT_PCI_0SCS_3_2_BASE_ADDRESS 						0x014
#define GT_PCI_0CS_2_0_BASE_ADDRESS 						0x018
#define GT_PCI_0CS_3_BOOTCS_BASE_ADDRESS					0x01C
#define GT_PCI_0INTERNAL_REGISTERS_MEMORY_MAPPED_BASE_ADDRESS	0x020
#define GT_PCI_0INTERNAL_REGISTERS_I_OMAPPED_BASE_ADDRESS	0x024
#define GT_PCI_0SUBSYSTEM_ID_AND_SUBSYSTEM_VENDOR_ID		0x02C
#define GT_EXPANSION_ROM_BASE_ADDRESS_REGISTER				0x030
#define GT_PCI_0INTERRUPT_PIN_AND_LINE 						0x03C
#define GT_PMC_REGISTER										0x040
#define GT_PMC_SR_REGISTER									0x044
#define GT_PCI_1DEVICE_AND_VENDOR_ID 						0x0
#define GT_PCI_1STATUS_AND_COMMAND							0x0
#define GT_PCI_1CLASS_CODE_AND_REVISION_ID 					0x0
#define GT_PCI_1BIST_HEADER_TYPE_LATENCY_TIMER_CACHE_LINE 	0x0
#define GT_PCI_1SCS_1_0_BASE_ADDRESS	 					0x0
#define GT_PCI_1SCS_3_2_BASE_ADDRESS 						0x0
#define GT_PCI_1CS_2_0_BASE_ADDRESS 						0x0
#define GT_PCI_1CS_3_BOOTCS_BASE_ADDRESS					0x0
#define GT_PCI_1INTERNAL_REGISTERS_MEMORY_MAPPED_BASE_ADDRESS	0x0
#define GT_PCI_1INTERNAL_REGISTERSI_OMAPPED_BASE_ADDRESS	0x0
#define GT_PCI_1SUBSYSTEM_ID_AND_SUBSYSTEM_VENDOR_ID		0x0
#define GT_PCI_1INTERRUPT_PIN_AND_LINE 						0x0

/****************************************/
/* PCI Configuration, Function 1		*/
/****************************************/

#define GT_PCI_0SWAPPED_SCS_1_0_BASE_ADDRESS 				0x110
#define GT_PCI_0SWAPPED_SCS_3_2_BASE_ADDRESS 				0x114
#define GT_PCI_0SWAPPED_CS_3_BOOTCS_BASE_ADDRESS			0x11C
#define GT_PCI_1SWAPPED_SCS_1_0_BASE_ADDRESS 				0x0
#define GT_PCI_1SWAPPED_SCS_3_2_BASE_ADDRESS 				0x0
#define GT_PCI_1SWAPPED_CS_3_BOOTCS_BASE_ADDRESS			0x0

#endif
