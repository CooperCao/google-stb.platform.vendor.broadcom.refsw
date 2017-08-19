/*
 * Firmware debugger UI
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
#include "arm_disassembler.h"

extern char dhd_version[];

/* target_read_u16()
 *
 * Needed by arm_disassembler.c
 */

int target_read_u16(struct target *target, uint32_t address, uint16_t *op) {

	*op = dbg_read16(address);

	return ERROR_OK;
}

static void display_disassembly(void *handle) {
	uint32_t pc;
	struct arm_instruction i;

	pc = dbg_rpc();

	thumb2_opcode(NULL, pc, &i);

	FPUTS(i.text, handle);
	FPUTS("\n", handle);
}

static uint32_t chkdbg(void *handle) {
	switch(dbg_get_arm_status()) {
		case ARM_NOT_INIT:
			FPUTS("Debug mode disabled/locked (type 'init' to initialize)\n", handle);
			return 1;
			break;

		case ARM_RUNNING:
			FPUTS("ARM is currently running (type 'halt' to halt in debug mode and allow access)\n",
				handle);
			return 1;
			break;
	}


	return 0;
}

static void ddr(void *handle) {
	char buf[256];

	if (chkdbg(handle))
		return;

	sprintf(buf, "r0  [%x]  r1  [%x]  r2 [%x]  r3  [%x]\n",
		dbg_rar(0), dbg_rar(1), dbg_rar(2), dbg_rar(3));
	FPUTS(buf, handle);

	sprintf(buf, "r4  [%x]  r5  [%x]  r6 [%x]  r7  [%x]\n",
		dbg_rar(4), dbg_rar(5), dbg_rar(6), dbg_rar(7));
	FPUTS(buf, handle);

	sprintf(buf, "r8  [%x]  r9  [%x]  r10 [%x]  r11  [%x]\n",
		dbg_rar(8), dbg_rar(9), dbg_rar(10), dbg_rar(11));
	FPUTS(buf, handle);

	sprintf(buf, "r12  [%x]  r13  [%x]  r14 [%x]  r15  [%x]\n",
		dbg_rar(12), dbg_rar(13), dbg_rar(14), dbg_rpc());
	FPUTS(buf, handle);

	sprintf(buf, "CPSR %x\n", dbg_cpsr());
	FPUTS(buf, handle);
	display_disassembly(handle);
}

void dstatus(void *handle) {
	switch(dbg_get_arm_status()) {
		case ARM_NOT_INIT:
			FPUTS("Debug mode disabled/locked (type 'init' to initialize)\n", handle);
			break;

		case ARM_RUNNING:
			FPUTS("ARM is currently running (type 'halt' to halt in debug mode and allow access)\n", handle);
			break;
		case ARM_HALTED:
			FPUTS("ARM Halted in debug mode\n", handle);
			break;

		case ARM_SS:
			FPUTS("ARM Halted, Single Stepping\n", handle);
			break;

		case ARM_BP:
			FPUTS("ARM Halted at Break Point\n", handle);
			break;
	}
}

void dhelp(void *handle) {
	FPUTS( "Debugger help\n", handle);
	FPUTS( "-------------\n", handle);
	FPUTS("\n", handle);
	FPUTS( "init            - initialize debugger\n", handle);
	FPUTS( "status          - Display debugger status\n", handle);
	FPUTS( "dump addr <len> - Display Memory\n", handle);
	FPUTS( "dr              - Display ARM Registers\n", handle);
	FPUTS( "rar (reg)       - Read ARM Register (reg)\n", handle);
	FPUTS( "war (reg, val)  - Write ARM Register (reg=val)\n", handle);
	FPUTS( "halt            - Halt ARM and allow debugger access\n", handle);
	FPUTS( "run             - Continue ARM execution\n", handle);
	FPUTS( "rpc             - Read the PC\n", handle);
	FPUTS( "cpsr            - Read the CPSR\n", handle);
	FPUTS( "wpc (val)       - Write the PC (PC=val)\n", handle);
	FPUTS( "brk (num, addr) - Set Breakpoint num to addr\n", handle);
	FPUTS( "cbrk (num)      - Clear breakpoint num\n", handle);
	FPUTS( "ss              - Single Step\n", handle);
	FPUTS("\n", handle);
	FPUTS( "profile <cnt> <delay>  - Profile Firmware\n", handle);
}

/* IO helpers */

static uint32_t dtoi(char *x) {
	uint32_t rval;

	sscanf(x, "%d", &rval);

	return rval;
}

static uint32_t xtoi(char *x) {
	uint32_t rval;

	sscanf(x, "%x", &rval);

	return rval;
}

static void itox(void *handle, uint32_t val) {
	char buf[32];

	sprintf(buf, "%x", val);
	FPUTS(buf, handle);
}


static char *skip_white(char *cp) {
	while (*cp == ' ' || *cp == '\t' || *cp == '\n' || *cp == '\r')
		cp++;

	return cp;
}

static char *trim_white(char *string) {
	char *cp = string + strlen(string) - 1;

	while (*cp == ' ' || *cp == '\t' || *cp == '\n' || *cp == '\r') {
		*cp = 0;
		cp--;
	}

	return string;
}

/* Keyword commands */

void help(void *h, int ac, char **av)	{ dhelp(h); }
void init(void *h, int ac, char **av)	{ dbg_init(); }
void status(void *h, int ac, char **av)	{ dstatus(h); }

void dr(void *h, int ac, char **av)	{ if (chkdbg(h)) return; ddr(h); }
void run(void *h, int ac, char **av)	{ if (chkdbg(h)) return; dbg_run(); }
void rar(void *h, int ac, char **av)	{ if (chkdbg(h)) return; if (ac != 2) return; itox(h, dbg_rar(xtoi(av[1]))); FPUTS("\n", h); }
void war(void *h, int ac, char **av)	{ if (chkdbg(h)) return; if (ac != 3) return; dbg_war(xtoi(av[1]), xtoi(av[2])); }
void halt(void *h, int ac, char **av)	{ dbg_halt(); display_disassembly(h); }
void rpc(void *h, int ac, char **av)	{ if (chkdbg(h)) return; itox(h, dbg_rpc()); FPUTS("\n", h); }
void cpsr(void *h, int ac, char **av)	{ if (chkdbg(h)) return; itox(h, dbg_cpsr()); FPUTS("\n", h); }
void wpc(void *h, int ac, char **av)	{ if (chkdbg(h)) return; if (ac != 2) return; dbg_wpc(xtoi(av[1])); }
void cbrk(void *h, int ac, char **av)	{ if (chkdbg(h)) return; if (ac != 2) return; dbg_cbrk(xtoi(av[1])); }

#define MAXSAMPLES 2000

uint32 profile_samples[MAXSAMPLES];

void profile(void *h, int ac, char **av) {
	uint32 i;
	uint32 delay = 0;
	uint32 count = 100;

	if (dbg_get_arm_status() == ARM_NOT_INIT) {
		FPUTS("Debug mode disabled/locked (type 'init' to initialize)\n", h);
		return;
	}

	if (ac > 3) {
		FPUTS("Usage: profile <count> <delay in ms>\n", h);
		return;
	}

	if (ac > 1)
		count = dtoi(av[1]);

	if (count > MAXSAMPLES) {
		count = MAXSAMPLES;
		FPUTS("Max samples is 2000. Reducing to this sample size.\n", h);
	}

	if (count == 0)
		count = 1;

	if (ac > 2)
		delay = dtoi(av[2]);

	FPUTS("Profiling...", h);

	for (i = 0; i < count; i++) {
		if (!(count % (count / 10)))
			FPUTS(".", h);

		dbg_halt();
		profile_samples[i] = dbg_rpc();
		dbg_run();

		if (delay)
			dbg_mdelay(delay);

	}

	FPUTS("\n-------------- cut here ------------------\n", h);
	FPUTS("FWID 01-xxxxxxxx\n", h);

	for (i = 0; i < count; i++) {
		if (!(i % 8))
			FPUTS("\n:", h);

		itox(h, profile_samples[i]);
		FPUTS(" ", h);
	}

	FPUTS("\n-------------- cut here ------------------\n", h);
	FPUTS("Paste the above output to http://firmware.sj.broadcom.com/trap.php to get a graph\n", h);
}

void dump(void *h, int ac, char **av) {
	int i;
	int len;
	char *cp;
	uint32_t addr;
	char buf[128];

	if (chkdbg(h))
		return;

	if (ac != 2 && ac != 3) {
		FPUTS("Usage: dump addr <len>\nYou must supply an addr. len is optional.\n", h);
		return;
	}

	addr = xtoi(av[1]);

	if (ac == 3)
		len = xtoi(av[2]);
	else
		len = 16;

	for (i = 0; i < len; i += 16) {
		cp = buf;

		cp += sprintf(cp, "%x: ", addr + i);
		cp += sprintf(cp, "%x ", dbg_read32(addr + i));
		cp += sprintf(cp, "%x ", dbg_read32(addr + i + 4));
		cp += sprintf(cp, "%x ", dbg_read32(addr + i + 8));
		cp += sprintf(cp, "%x\n", dbg_read32(addr + i + 12));

		FPUTS(buf, h);
	}
}

void brk(void *h, int ac, char **av) {
	if (ac != 3)
		return;


	if (dbg_get_arm_status() == ARM_RUNNING) {
		dbg_halt();
		dbg_brk(xtoi(av[1]), xtoi(av[2]), 1, 0);
		dbg_run();
	} else {
		dbg_brk(xtoi(av[1]), xtoi(av[2]), 1, 0);
	}
}

void ss(void *h, int ac, char **av) {
	dbg_ss();
	display_disassembly(h);
}

struct key2func {
	char *key;
	void (*funct)(void *h, int ac, char **av);
};

struct key2func key2func[] = {
	{ "init", init },
	{ "status", status },
	{ "dr", dr },
	{ "rar", rar },
	{ "war", war },
	{ "halt", halt },
	{ "rpc", rpc },
	{ "cpsr", cpsr },
	{ "wpc", wpc },
	{ "brk", brk },
	{ "cbrk", cbrk },
	{ "ss", ss },
	{ "run", run },
	{ "dump", dump },
	{ "help", help },
	{ "profile", profile },
	{ NULL }
};

void usage(void *h) {
	FPUTS("Error parsing command line\n", h);
}

void parse_keywords(char *buf, int *acp, char **avp, int buf_size, int av_size) {
	char *cp;
	int ac = 0;
	char **av = avp;

	if (!(*buf)) {
		*acp = 0;
		return;
	}


	for ( cp = av[0] = buf; cp < buf + buf_size; ) {
		if (!(*cp))
			break;

		if (*cp == ' ') {
			*cp = 0;
			cp = skip_white(cp + 1);
			ac++;

			if (ac > av_size)
				break;

			av[ac] = cp;
			continue;
		}

		cp++;
	}

	*acp = ac + 1;
}

void debugger_ui_main(void *handle) {
	char inbuf[256];
	char lastinbuf[256];
	char outbuf[256];
	char *cp;

	strcpy(lastinbuf, "");

	FPUTS("Broadcom Firmware Debugger\n", handle);
	FPUTS(dhd_version, handle);
	FPUTS("\n\n", handle);

	for (;;) {
		FPUTS("> ", handle);

		if (!(FGETS(inbuf, handle)))
			return;

		if (!(strlen(inbuf))) {
			strcpy(inbuf, lastinbuf);
		}

		strcpy(lastinbuf, inbuf);

		cp = skip_white(inbuf);
		trim_white(cp);

		/* Keyword commands */

		if (strlen(cp) > 1) {
			int ac;
			char *av[10];
			struct key2func *kfp = key2func;

			parse_keywords(cp, &ac, av, sizeof(outbuf), sizeof(av));

			while (ac && kfp->key) {
				if (!(strcmp(kfp->key, av[0]))) {
					(*kfp->funct)(handle, ac, av);
					break;
				}

				kfp++;
			}

			if (!kfp->key)
				FPUTS("Unknown command\n", handle);

			continue;
		}

		/* Single letter commands */

		switch (cp[0]) {


			case 'X':
				FPUTS("exit\n", handle);
				FCLOSE(handle);
				do_exit(0);
				return;
				break;
		}
	}
}
