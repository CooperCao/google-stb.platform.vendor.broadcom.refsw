/***************************************************************************
 *     Copyright (c) 2007-2009, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *    Wait loop and exception handler for passive standby mode driver
 *    This code must be copied to low memory (i.e near 0x80000000) in
 *    order to jump to it from the interrupt exception handler.
 *    The code is also locked into cache in order that we can put the
 *    memory controller in self refresh mode.
 *
 * Written by: Troy Trammel
 * Revision History: 09/13/07 Created
 ***************************************************************************/

#define _BRCMPMDRV_INTERNAL_		1

/* include line below normally on command line */
#include <linux/autoconf.h>
#include <linux/version.h>

#include <asm/asm.h>
#include <asm/mipsregs.h>
#include <asm/regdef.h>
#include <asm/asm-offsets.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 30)
#include <asm/brcmstb/common/brcmstb.h>
#endif

#include <linux/threads.h>
#include "bchp_common.h"
#include "bchp_hif_cpu_intr1.h"
#include "bchp_irq0.h"
#include "bchp_ldk.h"
#include "bchp_kbd1.h"
#include "bchp_timer.h"
#if (BCHP_CHIP==7125)
#include "bchp_memc_ddr_0.h"
#else
#include "bchp_memc_0_ddr.h"
#endif
#include "bchp_uarta.h"
#include "brcmpmdrv.h"

#define REG_OFFSET		(0xa0000000 | BCHP_PHYSICAL_OFFSET)
#define UPG_USEC_TICKS		(XTALFREQ / 1000000)

/* for best results, turn the UPG 27 clock back on to see debug output */
// #define DEBUG_ASM
// #define DEBUG_NO_UPG27_DISABLE

	.text
	.set	noreorder
	.align	5

# a0 (s0) is 1 to power down pads, 0 to leave them enabled,
#   or -1 for self refresh + pads on
# a1 is icache line size
# a2 is the interrupt vector (or general exception base)

# s1 is saved SCB refresh client setting

	LEAF(asm_wait_routine)
	subu	sp, 32
	sw	ra, 0(sp)
	sw	s0, 4(sp)
	sw	s1, 8(sp)

# lock the asm section into the I-cache
	bal	1f
	addiu	t1, a1, -1
1:
	not	t1
	and	t1, ra

	la	t2, passiveIRQ_end
	la	t3, asm_wait_routine
	subu	t2, t3
	addu	t2, t1

1:	cache	0x1c, 0(t1)
	sltu	t0, t1, t2
	bnez	t0, 1b
	addu	t1, a1

# now lock the interrupt vector
	cache	0x1c, 0(a2)

	move	s0, a0

	bal	dputc
	li	a0, 's'

#if ! HW_STANDBY
# Test for additional pad power down
	beqz	s0,wait_loop
	nop
#endif

# Flush memory transactions and enter self-refresh mode
	sync
#if (BCHP_CHIP == 7125)
	li	t0, REG_OFFSET + BCHP_MEMC_DDR_0_SRPD_CONFIG
	li	t1, 0x1000a
#else
	li	t0, REG_OFFSET + BCHP_MEMC_0_DDR_POWER_DOWN_MODE
	li	t1, 0x300a
#endif
	sw	t1, 0(t0)

#if ! HW_STANDBY

# poll until we are in self-refresh
#if (BCHP_CHIP == 7125)
	li	t0, REG_OFFSET + BCHP_MEMC_DDR_0_POWER_DOWN_STATUS
#else
	li	t0, REG_OFFSET + BCHP_MEMC_0_DDR_POWER_DOWN_STATUS
#endif
1:	lw	t1, 0(t0)
	andi	t1, 0x2
	beqz	t1, 1b
	nop

# block out the SCB refresh client
	li	t1, REFRESH_CLIENT_INFO
	lw	s1, 0(t1)
	li	t0, REFRESH_BLOCK_VALUE
	sw	t0, 0(t1)

	sync

	bal	dputc
	li	a0, 'm'

	li	t1, PPMD_KILL_PADS
	bne	s0, t1, wait_loop
	nop

# wait a loooooong time for the system to settle down
	li	a0, (85000 * UPG_USEC_TICKS)
	bal	delay
	nop
#if (BCHP_CHIP!=7125)
# power down the DDR pads
	lw	t0,REG_OFFSET+DDR_PAD_REG
	ori 	t0,DDR_PAD_MASK
	sw	t0,REG_OFFSET+DDR_PAD_REG
#endif
	bal	dputc
	li	a0, 'p'
#else

#ifndef DEBUG_NO_UPG27_DISABLE
# kill UPG 27 clock
	li	t0, REG_OFFSET + BCHP_CLKGEN_PWRDN_CTRL_3
	lw	t2, 0(t0)
	li	t1, BCHP_CLKGEN_PWRDN_CTRL_3_PWRDN_CLOCK_27_CG_SUN_MASK
	or	t2, t1
	sw	t2, 0(t0)
#endif

# kill MIPS and PLLs, then enter passive standby
	li	t0, REG_OFFSET + BCHP_CLKGEN_PM_CTRL
	sw	zero, 0(t0)
	lw	zero, 0(t0)

	li	t1, 3
	sw	t1, 0(t0)
#endif

wait_loop:
# enable int2 and then wait for an interrupt
	mfc0	t0, CP0_STATUS
	li	t1, ~(ST0_IM | ST0_IE)
	and	t0, t1
	ori	t0, STATUSF_IP2
	mtc0	t0, CP0_STATUS
	nop
	nop
	nop
	ori	t0, ST0_IE
	mtc0	t0, CP0_STATUS
	wait
	nop

#if ! HW_STANDBY
	bal	dputc
	li	a0, 'W'

	li	t1, PPMD_KILL_PADS
	bne	s0, t1, no_pads_1
	nop

# Power up DDR pads
#if (BCHP_CHIP!=7125)
	lw	t0,REG_OFFSET+DDR_PAD_REG
	li	t1, ~DDR_PAD_MASK
	and 	t0,t1
	sw	t0,REG_OFFSET+DDR_PAD_REG
#endif
	bal	dputc
	li	a0, 'P'

# delay 10us
	bal	delay
	li	a0, (10 * UPG_USEC_TICKS)

no_pads_1:
	beqz	s0,rtn_from_wait
	nop

	bal	dputc
	li	a0, 'M'

# unblock refresh client
	li	t1, REFRESH_CLIENT_INFO
	sw	s1, 0(t1)
#else

# MIPS sleep bit is not self-clearing
	li	t0, REG_OFFSET + BCHP_CLKGEN_PM_CTRL
	sw	zero, 0(t0)

# restore UPG 27 clock
	li	t0, REG_OFFSET + BCHP_CLKGEN_PWRDN_CTRL_3
	lw	t2, 0(t0)
	li	t1, ~BCHP_CLKGEN_PWRDN_CTRL_3_PWRDN_CLOCK_27_CG_SUN_MASK
	and	t2, t1
	sw	t2, 0(t0)

#endif

# exit self-refresh mode
#if (BCHP_CHIP == 7125)
	li	t0, REG_OFFSET + BCHP_MEMC_DDR_0_SRPD_CONFIG
#else
	li	t0, REG_OFFSET + BCHP_MEMC_0_DDR_POWER_DOWN_MODE
#endif
	sw	zero, 0(t0)

	sync

rtn_from_wait:
	bal	dputc
	li	a0, 'S'

	lw	s1, 8(sp)
	lw	s0, 4(sp)
	lw	ra, 0(sp)
	addiu	sp, 32

	jr	ra
	nop
	END(asm_wait_routine)

# debug character output

dputc:
#ifdef DEBUG_ASM
	/* INPUT: char in a0 */
	/* USES: t1, t2 */
	li 	t1, (UART_BASE | REG_OFFSET)
1:
	lw 	t2, UART_LSR_OFFSET(t1)
	andi	t2, UART_LSR_MASK
	beqz	t2, 1b
	nop

	sw	a0, UART_TX_OFFSET(t1)

1:	lw 	t2, UART_LSR_OFFSET(t1)
	andi	t2, UART_LSR_MASK
	beqz	t2, 1b
	nop
	jr	ra
	nop
#else
	jr	ra
	nop
#endif

delay:
#if HW_STANDBY
# need to reset TIMER1 since it is not normally used
	li	t2, 0x0
	li	t1, (BCHP_TIMER_TIMER1_CTRL | REG_OFFSET)
	sw	t2, 0(t1)
	lw	t2, 0(t1)

	li	t2, 0xbfffffff
	sw	t2, 0(t1)
	lw	t2, 0(t1)
#endif

	li	t2, 0x3fffffff
	li	t1, (BCHP_TIMER_TIMER1_STAT | REG_OFFSET)
	lw	t0, 0(t1)
	and	t0, t2
	add	t0, a0
	not	t2
	and	t2, t0
	bnez	t2, delay_overflow
	nop

# wait for timer value (t2) to exceed expiry time (t0)
1:	lw	t2, 0(t1)
	sltu	t2, t2, t0
	bnez	t2, 1b
	nop

	jr	ra
	nop

# timer1 overflow (this should never happen)
delay_overflow:
	move	t4, ra
	bal	dputc
	li	a0, 'O'

	jr	t4
	nop

	NESTED(passiveIRQ, PT_SIZE, sp)

# Block interrupts and then return to the wait loop
	mfc0	k0, CP0_STATUS
	li	k1, ~ST0_IE
	and	k0, k1
	mtc0	k0, CP0_STATUS
	eret
	nop

	END(passiveIRQ)
	.globl passiveIRQ_end
passiveIRQ_end:
	nop
