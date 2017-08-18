/*
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

#ifndef ARM_DEBUG_CORE_H
#define ARM_DEBUG_CORE_H

#define CHIPIDADRESS 	0x18000000
#define CHIPIDMASK	0xffff
#define CHIPID4350	0X4350

#define DEBUG_BASE_DEFAULT 0x18007000
#define DEBUG_BASE_4350	0x18009000

#define ARMDEBUGTIMEOUT 100

/* CR4TRM 11.3.4 */

#define DTRRX		32	/* Data Transfer Register */

#define ITR		33	/* Instruction Transfer Register */

#define DSCR		34	/* Debug Status and Control Register */

#define DSCR_HALTED	(1 << 0)
#define DSCR_RESTARTED	(1 << 1)
#define DSCR_ITR_ENA	(1 << 13)
#define DSCR_HALT	(1 << 14)
#define DSCR_INS_COMP	(1 << 24)
#define DSCR_TX_FULL	(1 << 29)
#define DSCR_RX_FULL	(1 << 30)

#define DTRTX		35	/* Data Transfer Register */

#define DRCR		36	/* Debug Run Control Register */

#define DRCR_HALT	(1 << 0)	/* Halt request */
#define DRCR_RESTART	(1 << 1)	/* Restart request */
#define DRCR_CLEAR	(1 << 2)	/* Clear sticky exceptions */

#define LOCKACCESS	1004	/* Lock Access Register */

#define LOCKSTATUS	1005	/* Lock Status Register */

#define LOCKSTATUS_LOCKED (1 << 1)

/* ARM Registers */
typedef struct regs_type {
	uint32_t r[12];       // System and User  r00..r11
	uint32_t ip;          // System and User  r12 = ???             (IP)
	uint32_t sp;          // System and User  r13 = Stack Pointer   (SP)
	uint32_t lr;          // System and User  r14 = Link Register   (LR)
	uint32_t pc;          // System and User  r15 = Process Counter (PC)
	uint32_t cpsr;        // Current Programm Status Register       (CPSR)
} regs_type;


#define ARM_NOT_INIT	0
#define ARM_RUNNING	1
#define ARM_HALTED	2
#define ARM_SS		3
#define ARM_BP		4

uint32_t	dbg_rpc(void);
void		dbg_wpc (uint32_t val);
uint32_t	dbg_rar(uint32_t reg);
void		dbg_war(uint32_t reg, uint32_t val);
uint32_t	dbg_cpsr(void);
void		dbg_init(void);
void		dbg_halt(void);
uint32_t	dbg_read32(uint32_t addr);
unsigned short	dbg_read16(uint32_t addr);
void		dbg_read_arm_regs(regs_type *r);
int		dbg_get_arm_status(void);
void		dbg_run(void);
uint32_t	dbg_ss(void);
void		dbg_brk(uint32_t num, uint32_t addr, uint32_t thumb, uint32_t ss);
void		dbg_cbrk(uint32_t num);

#endif /* ARM_DEBUG_CORE_H */
