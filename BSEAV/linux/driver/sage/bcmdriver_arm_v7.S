#include <linux/linkage.h>
#include <linux/init.h>
#include <asm/assembler.h>
#include <asm/unwind.h>
#include <asm/errno.h>

/*
 * ARMv7 Cache Maintenance
 */

ENTRY(__brcm_cache_line_size)
UNWIND(.fnstart)
/*  Cortex-A8 Technical Reference Manual */
    mrc     p15, 0, r2, c0, c0, 1   /* 3.4.2. c0, Cache Type Register */
    lsr     r2, r2, #16             /* [19:16] DMinLine */
                                    /*   Number of words of smallest line length in L1 or L2 data cache: */
                                    /*   4'b0100 = sixteen 32-bit word data line length. */
    and     r2, r2, #0x0f
    mov     r3, #4                  /*  word size */
    lsl     r0, r3, r2              /* shift word */
    bx      lr
UNWIND(.fnend)
ENDPROC(__brcm_cache_line_size)

ENTRY(__brcm_cpu_dcache_flush)
UNWIND(.fnstart)
    /* r0 - addr , 11 size */
    add     r1, r0, r1          /* x1 - end addr */

                                    /*  Cortex-A8 Technical Reference Manual */
    mrc     p15, 0, r2, c0, c0, 1   /* 3.4.2. c0, Cache Type Register */
    lsr     r2, r2, #16
    and     r2, r2, #0x0f
    mov     r3, #4                  /* word size */
    lsl     r2, r3, r2              /* shift word */
    sub     r3, r2, #1              /* make mask */
    bic     r0, r0, r3              /* align start address */

2:

    /*  Cortex-A8 Technical Reference Manual
     * 3.2.40. c7, Cache operations
     * Table 3.73. Register c7 cache and prefetch buffer maintenance operations
     */
    USER(mcr p15, 0, r0, c7, c14, 1)
    add     r0, r0, r2          /* next line */
    cmp     r0, r1
    blo     2b
    dsb                         /*  DSB - Data Synchronization Barrier */
    mov     r0, #0
    bx      lr

    /* if failed */
9001:
    mov     r0, #-EFAULT
    bx      lr

UNWIND(.fnend)
ENDPROC(__brcm_cpu_dcache_flush)
