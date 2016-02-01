#include <linux/linkage.h>
#include <linux/init.h>
#include <asm/assembler.h>
#include <asm/errno.h>

/*
 * ARMv8 Cache Maintenance
 */

/*
 * _dcache_line_size - get the minimum D-cache line size from the CTR register
 * on ARMv7.
/
    .macro  _dcache_line_size, reg, tmp
    mrc p15, 0, \tmp, c0, c0, 1     @ read ctr
    lsr \tmp, \tmp, #16
    and \tmp, \tmp, #0xf            @ cache line size encoding
    mov \reg, #4                    @ bytes per word
    mov \reg, \reg, lsl \tmp        @ actual cache line size
    .endm
*/
/*
 * dcache_line_size - Gets the D-cache line size from CP15
 *
 * r0: pointer to line size variable
 * r1: pointer to line size shift
 */
ENTRY(dcache_line_size)
/*
 UNWIND(.fnstart)
    _dcache_line_size r2, r3        @ put line size in r2 using r3 as temp
    str r2, [r0]
    str r3, [r1]
    bx lr
 UNWIND(.fnend)
*/
ENDPROC(dcache_line_size)

/*
 * dcache_clean_inval_line - Cleans + invalidates address range from D-cache
 *
 * r0: start address
 */
ENTRY(dcache_clean_inval_line)
/*
 UNWIND(.fnstart)
  USER( mcr p15, 0, r0, c7, c14, 1 )     @ DCCIMVAC
    mov r0, #0                          @ Clear return code
    bx lr                               @ Return to caller
*/
    /*
     * Fault handling for the cache operation above. (Label 9001 jumped to by
     * USER macro.)
     */
/*
9001:
    dsb                                 @ Data sync barrier
    mov r0, #-EFAULT                    @ Set return code
    bx lr                               @ Return to caller
 UNWIND(.fnend)
*/
ENDPROC(dcache_clean_inval_line)
