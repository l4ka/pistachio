/*********************************************************************
 *                
 * Copyright (C) 2003, 2007-2008,  Karlsruhe University
 *                
 * File path:     arch/x86/x64/cpu.cc
 * Description:   X86-64 CPUID features 
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
 * $Id: cpuid.cc,v 1.3 2003/09/24 19:05:26 skoglund Exp $
 *                
 ********************************************************************/
#include <debug.h>
#include INC_ARCH(cpu.h)

x86_x64_cpu_features_t::x86_x64_cpu_features_t(){
    
    /* Check out if CPUID available */
    if (!x86_x64_has_cpuid()){
	return;
    }
    
    u32_t eax, ebx, ecx, edx;
    
    /* Get largest std features function & vendor */
    x86_cpuid(CPUID_MAX_STD_FN_NR,
	  &max_std_fn,
	  ((u32_t *) (&cpu_vendor[0])),
	  ((u32_t *) (&cpu_vendor[8])),
	  ((u32_t *) (&cpu_vendor[4])));
    
    cpu_vendor[13] = 0;
    
    if (max_std_fn >= CPUID_STD_FEATURES){
	/* Get signature & std features */
	x86_cpuid(CPUID_STD_FEATURES,  &eax,  &ebx, &ecx, &edx);


	
	stepping = eax & 0xF;

	model =  (((eax > 4) & 0xF) == 0xF) ? (((eax > 12) & 0xF0) + 0xF) : ((eax > 4) & 0xF);
	family = (((eax > 8) & 0xF) == 0xF) ? (((eax > 20) & 0xFF) + 0xF) : ((eax > 8) & 0xF);
	
	brand_id = (ebx & 0xFF); 
	cflush_size = ((ebx >> 8) & 0xFF);
	apic_id = ((ebx >> 24) & 0xFF);
	
	std_features = edx;
    }
    
    /* Get largest ext function */
    x86_cpuid(CPUID_MAX_EXT_FN_NR, &max_ext_fn, &ebx, &ecx, &edx );
 	
    /* Get amd features */
    if (max_ext_fn < CPUID_AMD_FEATURES)
	return;
    
    x86_cpuid(CPUID_AMD_FEATURES,  &eax,  &ebx, &ecx, &edx);
    
    amd_features = edx;

    if (max_ext_fn < CPUID_CPU_NAME3)
	return;

    /* Get cpu name */
    x86_cpuid(CPUID_CPU_NAME1,
	  ((u32_t *) (&cpu_name[0])),
	  ((u32_t *) (&cpu_name[4])),
	  ((u32_t *) (&cpu_name[8])),
	  ((u32_t *) (&cpu_name[12])));

    x86_cpuid(CPUID_CPU_NAME2,
	  ((u32_t *) (&cpu_name[16])),
	  ((u32_t *) (&cpu_name[20])),
	  ((u32_t *) (&cpu_name[24])),
	  ((u32_t *) (&cpu_name[28])));

    x86_cpuid(CPUID_CPU_NAME3,
	  ((u32_t *) (&cpu_name[32])),
	  ((u32_t *) (&cpu_name[36])),
	  ((u32_t *) (&cpu_name[40])),
	  ((u32_t *) (&cpu_name[44])));
    
    cpu_name[48] = 0;

    /* Get L1 cache features */
    if (max_ext_fn < CPUID_CACHE_FEATURES1)
	return;

    x86_cpuid(CPUID_CACHE_FEATURES1, 
	  &l1_tlb.raw[0], 
	  &l1_tlb.raw[1], 
	  &l1_cache.raw[0], 
	  &l1_cache.raw[1]);
    

    /* Get L2 cache features */
    if (max_ext_fn < CPUID_CACHE_FEATURES2)
	return;

    x86_cpuid(CPUID_CACHE_FEATURES2, 
	  &l2_tlb.raw[0], 
	  &l2_tlb.raw[1], 
	  &l2_cache.raw[0], 
	  &edx);
    
    if (l2_tlb.d.dtlb_2m.entries == 0 && l2_tlb.d.dtlb_2m.assoc == 0){
	l2_tlb.d.dtlb_2m.entries = l2_tlb.d.itlb_2m.entries;
	l2_tlb.d.dtlb_2m.assoc = l2_tlb.d.itlb_2m.assoc;
	l2_2m_tlb_unified = true;
    }
	
    if (l2_tlb.d.dtlb_4k.entries == 0 && l2_tlb.d.dtlb_4k.assoc == 0){
	l2_tlb.d.dtlb_4k.entries = l2_tlb.d.itlb_4k.entries;
	l2_tlb.d.dtlb_4k.assoc = l2_tlb.d.itlb_4k.assoc;
	l2_4k_tlb_unified = true;
    }

    l2_cache.raw[1] = l2_cache.raw[0];
    l2_cache_unified = true;


    /* Get APM features */
    if (max_ext_fn < CPUID_APM_FEATURES)
	return;

    x86_cpuid(CPUID_APM_FEATURES, &eax, &ebx, &ecx, &apm_features);

    /* Get address sizes  */
    if (max_ext_fn < CPUID_ADDRESS_SIZES)
	return;

    x86_cpuid(CPUID_ADDRESS_SIZES, &eax, &ebx, &ecx, &edx);
    
    paddr_bits = (eax & 0xFF);
    vaddr_bits = ( (eax >> 8) & 0xFF);
}

void x86_x64_cpu_features_t::dump_features(){
    printf("CPU features:\n");

    const char* std_feature_list[] = {
	"fpu ", "vme ", "de  ", "pse ", "tsc ", "msr ", "pae ", "mce ",
	"cx8 ", "apic", "?   ", "syee", "mtrr", "pge ", "mca ", "cmov",
	"pat ", "pse2", "psn ", "cfl ", "?   ", "?   ", "?   ", "mmx ",
	"fxsr", "sse ", "sse2", "?   ", "?   ", "?   ", "?   ", "?   " };

    const char* amd_feature_list[] = {
	"fpu ",  "vme ", "de  ", "pse ", "tsc ", "msr ", "pae ", "mce ",
	"cx8 ",  "apic", "?   ", "sycr", "mtrr", "pge ", "mca ", "cmov",
	"pat ",  "pse2", "?   ", "?   ", "nx  ", "?   ", "ammx", "mmx ",
	"fxsr",  "?   ", "?   ", "?   ", "?   ", "lm  ", "3dne", "3dn " };

    printf("vendor\t\t: %s\nname\t\t: %s\n"
	   "family\t\t: %u\nmodel\t\t: %u\nstepping\t: %u\n"
	   "brand_id\t: %u\ncflush_size\t: %d\napic_id\t\t: %u\n",
	   (word_t) cpu_vendor, 
	   (word_t) cpu_name,
	   (word_t) family, 
	   (word_t) model, 
	   (word_t) stepping,
	   (word_t) brand_id, 
	   (word_t) cflush_size, 
	   (word_t) apic_id);
    
    printf("std_features\t: ");
    int j = 0;
    for (int i = 0; i < 32; i++){
	if ((std_features >> i) & 1) {
	    if (!(++j % 10)) printf("\n\t\t  ");
	    printf("%s ", std_feature_list[i]);
	}
    }
    
    printf("\namd_features\t: ");
    for (int i = 0; i < 32; i++){
	if ((amd_features >> i) & 1) {
	    if (!(++j % 10)) printf("\n\t\t  ");
	    printf("%s ", amd_feature_list[i]);
	}
    }

    printf("\n");
    printf("L1_ITLB 2m\t: %u entries, %u-ass.\n"
	   "L1_DTLB 2m\t: %u entries, %u-ass.\n"
	   "L1_ITLB 4k\t: %u entries, %u-ass.\n" 
	   "L1_DTLB 4k\t: %u entries, %u-ass.\n", 
	   (word_t) l1_tlb.d.itlb_2m.entries,
	   (word_t) l1_tlb.d.itlb_2m.assoc,
	   (word_t) l1_tlb.d.dtlb_2m.entries,
	   (word_t) l1_tlb.d.dtlb_2m.assoc,
	   (word_t) l1_tlb.d.itlb_4k.entries,
	   (word_t) l1_tlb.d.itlb_4k.assoc,
	   (word_t) l1_tlb.d.dtlb_4k.entries,
	   (word_t) l1_tlb.d.dtlb_4k.assoc);

    printf("L1_IACHE\t: %u B/line, %u lines/tag, %u-ass., %u KB\n" 
	   "L1_DACHE\t: %u B/line, %u lines/tag, %u-ass., %u KB\n", 
	   (word_t) l1_cache.d.icache.l_size,
	   (word_t) l1_cache.d.icache.l_per_tag,
	   (word_t) l1_cache.d.icache.assoc,
	   (word_t) l1_cache.d.icache.size,
	   (word_t) l1_cache.d.dcache.l_size,
	   (word_t) l1_cache.d.dcache.l_per_tag,
	   (word_t) l1_cache.d.dcache.assoc,
	   (word_t) l1_cache.d.dcache.size);

    if (l2_2m_tlb_unified){
	printf("L2_TLB 2m\t: %u entries, %u-ass.\n",
	       (word_t) l2_tlb.d.itlb_2m.entries,
	       (word_t) l2_tlb.d.itlb_2m.assoc);
    }
    else{
	printf("L2_ITLB 2m\t: %u lines, %u-ass.\n"
	       "L2_DTLB 2m\t: %u lines, %u-ass.\n",
	       (word_t) l2_tlb.d.itlb_2m.entries,
	       (word_t) l2_tlb.d.itlb_2m.assoc,
	       (word_t) l2_tlb.d.dtlb_2m.entries,
	       (word_t) l2_tlb.d.dtlb_2m.assoc);
    }

    if (l2_4k_tlb_unified){
	printf("L2_TLB 4k\t: %u entries, %u-ass.\n",
	       (word_t) l2_tlb.d.itlb_4k.entries,
	       (word_t) l2_tlb.d.itlb_4k.assoc);
    }
    else{
	printf("L2_ITLB 4k\t: %u entries, %u-ass.\n"
	       "L2_DTLB 4k\t: %u entries, %u-ass.\n",
	       (word_t) l2_tlb.d.itlb_4k.entries,
	       (word_t) l2_tlb.d.itlb_4k.assoc,
	       (word_t) l2_tlb.d.dtlb_4k.entries,
	       (word_t) l2_tlb.d.dtlb_4k.assoc);
    }
    printf("L2_CACHE\t: %u B/line, %u lines/tag, %u-ass., %u KB\n",
	   (word_t) l2_cache.d.icache.l_size,
	   (word_t) l2_cache.d.icache.l_per_tag,
	   (word_t) l2_cache.d.icache.assoc,
	   (word_t) l2_cache.d.icache.size);

    printf("Addrlen\t\t: %u physical, %u virtual\n",
	   (word_t) paddr_bits,
	   (word_t) vaddr_bits);

}
    
