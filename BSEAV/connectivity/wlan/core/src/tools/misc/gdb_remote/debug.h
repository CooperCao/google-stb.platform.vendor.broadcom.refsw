/*
 *  Copyright (C) 2004 Tobias Lorenz
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

/* debugging and register functions */

/* Debug speed */
#define DEBUG_SPEED		0
#define SYSTEM_SPEED		1
#define DEBUG_SPEED_SHORT	2

/* Registers */
typedef struct regs_type {
    /* System and User registers */
    uint32 r    [12];	// System and User  r00..r11
    uint32 ip;		// System and User  r12 = ???             (IP)
    uint32 sp;		// System and User  r13 = Stack Pointer   (SP)
    uint32 lr;		// System and User  r14 = Link Register   (LR)
    uint32 pc;		// System and User  r15 = Process Counter (PC)
    uint32 f    [ 8][3];	// GDBs 3*32 Bit registers F0..F8
    uint32 fps;		// GDBs   32 Bit register  fps
    uint32 cpsr;		// Current Programm Status Register       (CPSR)
} regs_type;
extern regs_type arm_regs;

/* Program status register */
#define PSR_N	(1<<31)		/* Condition code flag: Negative or less than     */
#define PSR_Z	(1<<30)		/* Condition code flag: Zero                      */
#define PSR_C	(1<<29)		/* Condition code flag: Carry or borrow or extend */
#define PSR_V	(1<<28)		/* Condition code flag: Overflow                  */
#define PSR_I	(1<<7)		/* Control bit: IRQ disable */
#define PSR_F	(1<<6)		/* Control bit: FIQ disable */
#define PSR_T	(1<<5)		/* Control bit: State bit   */
#define PSR_M_usr	0x10	/* Mode bit: User       */
#define PSR_M_fiq	0x11	/* Mode bit: FIQ        */
#define PSR_M_irq	0x12	/* Mode bit: IRQ        */
#define PSR_M_svc	0x13	/* Mode bit: Supervisor */
#define PSR_M_abt	0x17	/* Mode bit: Abort      */
#define PSR_M_und	0x1B	/* Mode bit: Undefined  */
#define PSR_M_sys	0x1F	/* Mode bit: System     */


void debug_exec_begin(void);
unsigned long debug_exec(unsigned long cmd, unsigned long data, int type);

void debug_save_regs(void);		// saves registers (after WP) and disable interrupts

void debug_stop(void);
void debug_run(void);
void debug_step(void);
