/*
 * Interface routines to ARM debug coprocessor
 *
 * See:
 *  Cortex-R4 and Cortex-R4F Technical Reference Manual section 11
 *
 * Copyright (C) 2013, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id$
 */

#include <typedefs.h>

#include "debugger.h"
#include "debugger_arm.h"


/* arm_dbg_rr() & arm_dbg_wr()
 *
 * Read and write ARM debug core registers
 */

static uint32 arm_debug_core_base = 0;

static uint32 arm_dbg_rr(int reg) {
	if (!(arm_debug_core_base)) {
		if ((dbg_reg_read(CHIPIDADRESS) & CHIPIDMASK) == CHIPID4350)
			arm_debug_core_base = DEBUG_BASE_4350;
		else
			arm_debug_core_base = DEBUG_BASE_DEFAULT;
	}

	return dbg_reg_read(reg * 4 + arm_debug_core_base);
}

static void arm_dbg_wr(int reg, uint32_t val) {
	dbg_reg_write(reg * 4 + arm_debug_core_base, val);
}

/* wait_for_arm()
 *
 * We the information we need by feeding ARM instructions to the debug core ITR register
 *
 * This routine waits for the execution of these instructions to complete by
 * waiting for bit 24 of DSCR to be set.
 *
 * CR4TRM 11.4.5
 */

static uint32_t wait_for_arm(void) {
	int i;

	for ( i = 0; i < ARMDEBUGTIMEOUT; i++) {
		if (arm_dbg_rr(DSCR) & DSCR_INS_COMP)
			return 0;
		dbg_mdelay(1);
	}

	return 1;
}

/* wait_for_tx()
 *
 * The co-processor sends data to us via DTRTX register (in somecases) after we
 * issue instructions to it via the ITR register.
 *
 * This routine waits for the data by checking the DSCR_TX_FULL bit of DSCR
 *
 * CR4TRM 11.4.5
 */

static uint32_t wait_for_tx(void) {
	int i;

	for ( i = 0; i < ARMDEBUGTIMEOUT; i++) {
		if (arm_dbg_rr(DSCR) & DSCR_TX_FULL)
			return 0;
		dbg_mdelay(1);
	}

	return 1;
}

/* _eai()
 *
 * Execute arm instruction by feeding it to the ITR register
 *
 * CR4TRM 11.4.10 & 11.11
 */

static void _eai(uint32_t inst) {

	wait_for_arm();
	arm_dbg_wr(ITR, inst);
	wait_for_arm();

}

/* dbg_rpc()
 *
 * Return the value of the PC
 *
 * CR4TRM 11.11.6
 */

uint32_t dbg_rpc(void) {
	uint32_t old_r0;
	uint32_t rval;

	old_r0 = dbg_rar(0);

	/* MOV r0, pc */
	_eai(0xE1A0000F);
	rval = dbg_rar(0);

	dbg_war(0, old_r0);

	if (dbg_cpsr() & (1<<5)) {
		rval -= 4;
	} else {
		rval -= 8;
	}

	return rval;
}


/* dbg_wpc()
 *
 * Write the passed value into the PC
 *
 * CR4TRM 11.11.6
 */

void dbg_wpc (uint32_t val) {
	uint32_t old_r0;

	old_r0 = dbg_rar(0);
	dbg_war(0, val);

	/* Execute instruction MCR p14, 0, Rd, c0, c5, 0 through the ITR. */
	_eai(0xE1A0F000);
	dbg_war(0, old_r0);
}

/* dbg_rar()
 *
 * Read ARM register
 *
 * CR4TRM 11.11.6
 */

uint32_t dbg_rar(uint32_t reg) {
	_eai( 0xEE000E15  + ( reg << 12 ));
	wait_for_tx();
	return (arm_dbg_rr(DTRTX));
}

/* dbg_war()
 *
 * Write ARM register
 *
 * CR4TRM 11.11.6
 */

void dbg_war(uint32_t reg, uint32_t val) {
	arm_dbg_wr(DTRRX, val);
	_eai(0xEE100E15 + ( reg << 12 ));
}

/* dbg_cpsr()
 *
 * Return the ARM CPSR
 *
 * CR4TRM 11.11.6
 */

uint32_t dbg_cpsr(void) {
        uint32_t old_r0;
	uint32_t rval;

	old_r0 = dbg_rar(0);
	_eai(0xE10F0000);
	rval = dbg_rar(0);
	dbg_war(0, old_r0);
	return rval;
}

/* dbg_init()
 *
 * Enable the debug co-processor by writing the magic word to the Lock Access register
 *
 * CR4TRM 11.5.3
 */

void dbg_init(void) {
	arm_dbg_wr(LOCKACCESS, 0xC5ACCE55);
}

/* dbg_halt()
 *
 * Halt the ARM and enable the ITR
 *
 * (very important not to have ITR enabled while ARM is running)
 */

void dbg_halt(void) {
	arm_dbg_wr( DRCR, DRCR_HALT);
	arm_dbg_wr( DSCR, arm_dbg_rr(DSCR) | DSCR_HALT | DSCR_ITR_ENA);

	wait_for_arm();
}


/* dbg_read32()
 *
 * Read a word by loading it from the ARM
 * (provides the ARMs view of the memory map)
 */

uint32_t dbg_read32(uint32_t addr) {
	uint32_t old_r0;
	uint32_t rval;

	old_r0 = dbg_rar(0);
	dbg_war(0, addr & ~3);
	_eai(0xED905E00);	/* LDC p14, c5, [R0] L */
	rval = arm_dbg_rr(DTRTX);

	dbg_war(0, old_r0);

	return rval;
}

/* dbg_read16()
 *
 * Read a half-word by loading it from the ARM
 * (provides the ARMs view of the memory map)
 */

unsigned short dbg_read16(uint32_t addr) {
	uint32_t old_r0;
	uint32_t rval;

	old_r0 = dbg_rar(0);
	dbg_war(0, addr & ~3);
	_eai(0xED905E00);	/* LDC p14, c5, [R0] L */
	rval = arm_dbg_rr(DTRTX);

	dbg_war(0, old_r0);

	if (addr & 3)
		rval >>= 16;

	return rval;
}

/* dbg_read_arm_regs()
 *
 * Read all ARM Registers and return them in the passed reg_type structure
 */

void dbg_read_arm_regs(regs_type *r) {
	r->r[0] = dbg_rar(0);
	r->r[1] = dbg_rar(1);
	r->r[2] = dbg_rar(2);
	r->r[3] = dbg_rar(3);
	r->r[4] = dbg_rar(4);
	r->r[5] = dbg_rar(5);
	r->r[6] = dbg_rar(6);
	r->r[7] = dbg_rar(7);
	r->r[8] = dbg_rar(8);
	r->r[9] = dbg_rar(9);
	r->r[10] = dbg_rar(10);
	r->r[11] = dbg_rar(11);
	r->ip = dbg_rar(12);
	r->sp = dbg_rar(13);
	r->lr = dbg_rar(14);
	r->pc = dbg_rpc();
	r->cpsr = dbg_cpsr();
}

/* dbg_get_arm_status()
 *
 * Get the ARM debug state
 */

int dbg_get_arm_status(void) {
	if (arm_dbg_rr(LOCKSTATUS) & LOCKSTATUS_LOCKED) {
		return ARM_NOT_INIT;
	}

	if (!(arm_dbg_rr(DSCR) & DSCR_HALTED)) {
		return ARM_RUNNING;
	}

	/* If we have halted asynchronously (e.g. breakpoint) */
	/* catch it here and the enable ITR */

	arm_dbg_wr( DSCR, arm_dbg_rr(DSCR) | DSCR_ITR_ENA);

	if (! (arm_dbg_rr(DSCR) & 4)) {
		return ARM_HALTED;
	} else {
		if (arm_dbg_rr(67)) {
			return ARM_SS;
		} else {
			return ARM_BP;
		}
	}
}

/* dbg_run()
 *
 * Tell the ARM to go
 */

void dbg_run(void) {

	dbg_cbrk(3); /* Clear SS BP */
	arm_dbg_wr( DSCR, arm_dbg_rr(DSCR) & ~DSCR_ITR_ENA); /* disable the ITR */

	arm_dbg_wr( DRCR, DRCR_RESTART | DRCR_CLEAR);
}


/* dbg_brk()
 *
 * Set Breakpoint
 */

void dbg_brk(uint32_t num, uint32_t addr, uint32_t thumb, uint32_t ss) {
	uint32_t addr_select = 0xf;

	arm_dbg_wr(80 + num, 0);
	arm_dbg_wr(64 + num, addr & 0xfffffffc);

	if (thumb) {
		addr_select = 3 << (addr & 2);
	}

	arm_dbg_wr(80 + num, 1 | (3 << 1) | (addr_select << 5) | (ss << 22));
}

/* dbg_cbrk()
 *
 * Clear breakpoint
 */

void dbg_cbrk(uint32_t num) {
	arm_dbg_wr(80 + num, 0);
	arm_dbg_wr(64 + num, 0);
}

/* dbg_ss()
 *
 * Single Step
 *
 * returns 0 if stepped
 * returns 1 if timeout
 */

uint32_t dbg_ss(void) {

	dbg_brk(3, dbg_rpc(), 1, 1);

	arm_dbg_wr( DSCR, arm_dbg_rr(DSCR) & ~DSCR_ITR_ENA);
	arm_dbg_wr( DRCR, DRCR_RESTART | DRCR_CLEAR);

	if (wait_for_arm()) {
		return 1;
	}

	arm_dbg_wr( DSCR, arm_dbg_rr(DSCR) | DSCR_HALT | DSCR_ITR_ENA);

	return 0;
}
