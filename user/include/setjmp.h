//#define setjmp __builtin_setjmp

//https://github.com/l4ka/pistachio/blob/c775191a0d004eb761e594ed6ea3aacccf6c7d33/kernel/kdb/arch/x86/x32/ia32-dis.c is where this hides for exclusive use of the kernel debugger, usually...

//https://github.com/l4ka/pistachio/blob/c775191a0d004eb761e594ed6ea3aacccf6c7d33/kernel/src/arch/x86/x64/types.h
typedef unsigned int	u32_t;

typedef struct jmp_buf { u32_t eip; u32_t esp; u32_t ebp; int val; } jmp_buf[1];

//Put these in their own source file ASAP...

static inline void longjmp(jmp_buf env, int val)
{
#if 0
printf("%s(%x,%x) called from %x\n", __FUNCTION__, &env, val,
({u32_t x;__asm__ __volatile__("call 0f;0:popl %0":"=r"(x));x;}));
printf("jumping to eip=%x esp=%x, ebp=%x\n",
env->eip, env->esp, env->ebp);
#endif

    env->val = val;
    __asm__ __volatile__ (
"jmp *(%%ebx) \n\t"
"orl %%esp,%%esp \n\t"
:
: "a" (&env->ebp), "c"(&env->esp), "b" (&env->eip)
: "edx", "esi", "edi", "memory");
    while(1);
};
static inline int setjmp(jmp_buf env)
{
#if 0
printf("%s(%x) called from %x\n", __FUNCTION__, &env,
({u32_t x;__asm__ __volatile__("call 0f;0:popl %0":"=r"(x));x;}));
#endif
    env->val = 0;
    __asm__ __volatile__ (
"movl %%ebp, (%0) \n\t"
"movl %%esp, (%1) \n\t"
"movl $0f, (%2) \n\t"
"0: \n\t"
"movl %%esp,%%esp \n\t"
"movl (%1),%%esp \n\t"
"movl (%0),%%ebp \n\t"
:
: "a" (&env->ebp), "c"(&env->esp), "b" (&env->eip)
: "edx", "esi", "edi", "memory");
#if 0
printf("prepared to return to eip=%x esp=%x, ebp=%x\n",
env->eip, env->esp, env->ebp);
#endif
    return env->val;
};




