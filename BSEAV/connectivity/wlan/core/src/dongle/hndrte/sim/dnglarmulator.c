/*
 * dnglarmulator.c - Armulator cheeseball peripheral model
 *	- Simulates 802.11 chip hardware by trapping memory accesses
 *	  and passing them on to cheeseball chip simulator
 *	- Optionally passes 802.11 packets to/from virtual
 *	  ethernet switch via socket model
 *	- Optionally passes USB/SDIO bus packets to/from virtual
 *	  device bus via socket model
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id$
 */

#include <memory.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "minperip.h"
#include "armul_agent.h"			/* for ARMul_ShowModuleDesc */

#include "wlarmulator.h"
#include "dnglperiph.h"

#include <assert.h>

#define CYCLECOUNT_SPEEDUP	100		/* fake speedup; see below */

#define MEM_MAP_IO_BASE		0x8000000	/* start of area in which to trap mem access */
#define MEM_MAP_IO_SIZE		0x10040000	/* size of area in which to trap mem access */

#define NVRAM_FILE		"NVRAM_FILE"
#define NVRAM_BASE		0x1c000400
#define NVRAM_SIZE		0x00002000

#define DN_BASE			0		/* region to check for dereferencing NULL ptr */
#define DN_SIZE			0x1000

#define DMA_REGION		0x8000000	/* start address of SDRAM buffer */
#define DMA_REGION_SIZE		0x80000		/* SDRAM buffer size */

#define TIMESTAMP_FILE_VAR	"DNGL_TS"
#define INSTR_DUMP_FILE_VAR	"DNGL_DUMP"
#define TRACE_FILE_VAR		"DNGL_TRACE"
#define PROFILE_FILE_VAR	"DNGL_PROF"
#define IMAGE_FILE_VAR		"DNGL_IMAGE"

#define BITGET(x, bs, be)	(((x) << (31 - (be))) >> (31 - (be) + (bs)))

/* Redefine a few things here to avoid clashes between wl and armulator types */
typedef unsigned char uchar;
typedef unsigned int *uintptr;

extern void chip_init(void);
extern void sb_runcores(void);

extern int sbread(uint32 sbaddr, uchar *buf, uint nbytes);
extern int sbwrite(uint32 sbaddr, uchar *buf, uint nbytes);
#ifdef USB_DNGL
extern int connect_usb_host(void);
#endif
#ifdef SDIO_DNGL
extern int connect_sdio_host(void);
#endif

extern uintptr virt_to_phys(void *p);
extern void *phys_to_virt(uintptr pa);

/* Global to allow src/tools/shared/utils.c to display the ARM PC on assertion failure */
uint armulator_last_pc;

/* Broadcom-specific coprocessor 7 implementation in 4328 */

#define COP7_REG_CYCLECOUNT	0
#define COP7_REG_INTTIMER	1
#define COP7_REG_INTMASK	2
#define COP7_REG_INTSTATUS	3

struct cop7 {
	struct ARMul_CoprocessorV5 v5;
	uint32 cyclecounter;
	uint32 inttimer;
	uint32 intmask;
	uint32 intstatus;
};

/*
 * Symbol Table routines
 *
 *   If the symbol table dngl.nm exists, it will be loaded at startup.
 *   dngl.nm can be created using "arm-elf-nm rtesim > dngl.nm".
 *   Sample line from .nm file should look like "0004555c T connect_host"
 */

struct sym_ent {
	unsigned addr;
	char *name;
	unsigned cycles;		/* Profile counter */
};

struct sym_tab {
	int sym_alloc;
	int sym_used;
	struct sym_ent *sym;
};

static int
_sym_compar_addr(const void *a, const void *b)
{
	const struct sym_ent *sym_a = a;
	const struct sym_ent *sym_b = b;

	return (sym_a->addr - sym_b->addr);
}

static int
_sym_compar_cycles(const void *a, const void *b)
{
	const struct sym_ent *sym_a = a;
	const struct sym_ent *sym_b = b;

	return (sym_b->cycles - sym_a->cycles);	/* Reverse order */
}

static struct sym_tab *
sym_load(FILE *fp)
{
	struct sym_tab *st;
	char line[256], *cp;
	unsigned addr;
	struct sym_ent *sym;

	if ((st = malloc(sizeof(struct sym_tab))) == NULL)
		goto out_of_mem;

	st->sym_alloc = 10;
	st->sym_used = 0;
	st->sym = malloc(st->sym_alloc * sizeof(struct sym_ent));

	if (st->sym == NULL)
		goto out_of_mem;

	while (fgets(line, sizeof(line), fp) != NULL) {
		if (st->sym_used == st->sym_alloc) {
			st->sym_alloc *= 2;
			st->sym = realloc(st->sym, st->sym_alloc * sizeof(struct sym_ent));
			if (st->sym == NULL)
				goto out_of_mem;
		}
		sym = &st->sym[st->sym_used];

		line[sizeof(line) - 1] = 0;
		if ((cp = strchr(line, '\n')) != 0)
			*cp = 0;
		addr = (unsigned)strtoul(line, &cp, 16);
		if (addr == 0 || *cp++ != ' ')
			continue;
		if (*cp != 't' && *cp != 'T')
			continue;
		cp++;
		if (*cp++ != ' ' || *cp == 0)
			continue;

		sym->addr = addr;
		if ((sym->name = malloc(strlen(cp) + 1)) == NULL)
			goto out_of_mem;
		strcpy(sym->name, cp);
		sym->cycles = 0;

		st->sym_used++;
	}

	fclose(fp);

	/* Sort by address */
	qsort(st->sym, st->sym_used, sizeof(struct sym_ent), _sym_compar_addr);

	printf("sym_load: loaded %d symbols\n", st->sym_used);
	return st;

out_of_mem:
	fprintf(stderr,
	        "sym_load: Out of memory loading symbol table\n");
	exit(1);
	/*NOTREACHED*/
	return NULL;
}

static void
sym_free(struct sym_tab *st)
{
	int i;

	for (i = 0; i < st->sym_used; i++)
		free(st->sym[i].name);

	free(st->sym);

	st->sym = NULL;
	st->sym_used = 0;
	st->sym_alloc = 0;

	free(st);
}

/* Binary search to turn address into symbol */
static struct sym_ent *
sym_lookup(struct sym_tab *st, unsigned addr)
{
	int k1, k2, kh;
	unsigned sym_max;

	if (st->sym_used == 0 || addr < st->sym[0].addr)
		return 0;

	k1 = -1;
	k2 = st->sym_used;

	while (k2 - k1 > 1) {
		kh = (k1 + k2) / 2;

		sym_max = (kh < st->sym_used - 1) ? st->sym[kh + 1].addr : 0xffffffffU;

		if (addr < sym_max)
			k2 = kh;
		else
			k1 = kh;
	}

	if (k2 < st->sym_used)
		return &st->sym[k2];

	return NULL;
}

static void
sym_profdump(struct sym_tab *st, ARMTime total_cycles, FILE *out_fp)
{
	int i;

	/* Re-sort symbol table by number of cycles used */
	qsort(st->sym, st->sym_used, sizeof(struct sym_ent), _sym_compar_cycles);

	for (i = 0; i < st->sym_used && st->sym[i].cycles != 0; i++)
		fprintf(out_fp,
		        "%10.6f %s\n",
		        100.0 * st->sym[i].cycles / total_cycles,
		        st->sym[i].name);
}

/* Armulator module state */

BEGIN_STATE_DECL(WlSim)
	void *timer_handle;
	ARMul_BusPeripAccessRegistration sb_bpar;
	char *nvram_fname;	/* NULL if none */
	char nvram_buf[NVRAM_SIZE];
	ARMul_BusPeripAccessRegistration nv_bpar;
	ARMul_BusPeripAccessRegistration dn_bpar;
	int dn_enable;
	struct cop7 cop7;
	void *hourglass;
	uint last_pc;
	ARMTime last_cycle;
	unsigned long instr_count;
	uint hz;
	FILE *timestamp_fp;
	FILE *dump_fp;
	FILE *trace_fp;
	FILE *profile_fp;
	struct sym_tab *sym_tab;
	struct sym_ent *sym_last;
END_STATE_DECL(WlSim)

/* Function prototypes */
static void Hourglass(void *handle, ARMword pc, ARMword instr,
                      ARMword cpsr, ARMword condpassed);

static ARMul_BusPeripAccessFunc WlSim_SbAccess;
static ARMul_BusPeripAccessFunc WlSim_NvAccess;
static ARMul_BusPeripAccessFunc WlSim_DnAccess;

/* Detect programming bugs that dereference NULL or other addresses in the low 4K of memory */
int
WlSim_DnInit(WlSimState *state)
{
	int err;
	ARMul_BusPeripAccessRegistration *regn;

	/* Map accesses to first 4k of memory */
	regn = &state->dn_bpar;

	state->dn_enable = 0;

	if ((err = ARMulif_ReadBusRange(&state->coredesc,
	                                state->hostif,
	                                0,
	                                &state->dn_bpar,
	                                DN_BASE,
	                                DN_SIZE,
	                                "")) == 0) {
		regn->access_func = WlSim_DnAccess;
		regn->access_handle = state;
		state->dn_bpar.capabilities = PeripAccessCapability_Typical;
		err = state->dn_bpar.bus->bus_registerPeripFunc(BusRegAct_Insert, regn);
	}

	return err;
}

int
WlSim_SbInit(WlSimState *state)
{
	int err;
	ARMul_BusPeripAccessRegistration *regn;

	/* Map SB accesses */
	regn = &state->sb_bpar;

	if ((err = ARMulif_ReadBusRange(&state->coredesc,
	                                state->hostif,
	                                0,
	                                &state->sb_bpar,
	                                MEM_MAP_IO_BASE,
	                                MEM_MAP_IO_SIZE,
	                                "")) == 0) {
		regn->access_func = WlSim_SbAccess;
		regn->access_handle = state;
		state->sb_bpar.capabilities = PeripAccessCapability_Typical;
		err = state->sb_bpar.bus->bus_registerPeripFunc(BusRegAct_Insert, regn);
	}

	return err;
}

int
WlSim_NvInit(WlSimState *state)
{
	int err;
	ARMul_BusPeripAccessRegistration *regn;
	FILE *fp;

	/* Load nvram image into memory */
	if ((state->nvram_fname = getenv(NVRAM_FILE)) == NULL)
		return 0;

	if ((fp = fopen(state->nvram_fname, "r")) == NULL) {
		printf("dnglarmulator: WlSim_NvInit: could not open \"%s\": %s\n",
		       state->nvram_fname, strerror(errno));
		return RDIError_UnableToInitialise;
	}

	fseek(fp, 0x400, SEEK_SET);
	fread(state->nvram_buf, NVRAM_SIZE, 1, fp);
	fclose(fp);

	/* Map NVRAM accesses */
	regn = &state->nv_bpar;

	if ((err = ARMulif_ReadBusRange(&state->coredesc,
	                                state->hostif,
	                                0,
	                                &state->nv_bpar,
	                                NVRAM_BASE,
	                                NVRAM_SIZE,
	                                "")) == 0) {
		regn->access_func = WlSim_NvAccess;
		regn->access_handle = state;
		state->nv_bpar.capabilities = PeripAccessCapability_Typical;
		err = state->nv_bpar.bus->bus_registerPeripFunc(BusRegAct_Insert, regn);
	}

	return err;
}

static void
set_irq(WlSimState *state)
{
	static int old_active = FALSE;
	extern int sbintpend;
	int active = FALSE;

	if (state->cop7.intmask & state->cop7.intstatus)
		active = TRUE;

	if (sbintpend)
		active = TRUE;

	if (!old_active && active) {
		ARMulif_SetSignal(&state->coredesc, RDIPropID_ARMSignal_IRQ, TRUE);
		old_active = TRUE;
	} else if (old_active && !active) {
		ARMulif_SetSignal(&state->coredesc, RDIPropID_ARMSignal_IRQ, FALSE);
		old_active = FALSE;
	}
}

static int
MRC7(void *handle, int type, ARMword instr, uint32 *data)
{
	WlSimState *state = handle;

	IGNORE(type);

	switch (BITGET(instr, 16, 19)) {
	case COP7_REG_CYCLECOUNT:
		*data = state->cop7.cyclecounter;
		break;
	case COP7_REG_INTTIMER:
		*data = state->cop7.inttimer;
		break;
	case COP7_REG_INTMASK:
		*data = state->cop7.intmask;
		break;
	case COP7_REG_INTSTATUS:
		*data = state->cop7.intstatus;
		break;
	default:
		return ARMul_CP_CANT;
	}

	return ARMul_CP_DONE;
}

static int
MCR7(void *handle, int type, ARMword instr, uint32 *data)
{
	WlSimState *state = handle;

	IGNORE(type);

	switch (BITGET(instr, 16, 19)) {
	case COP7_REG_CYCLECOUNT:
		state->cop7.cyclecounter = *data;
		break;
	case COP7_REG_INTTIMER:
		state->cop7.inttimer = *data;
		break;
	case COP7_REG_INTMASK:
		state->cop7.intmask = *data;
		break;
	case COP7_REG_INTSTATUS:
		state->cop7.intstatus &= ~(*data);
		break;
	default:
		return ARMul_CP_CANT;
	}

	return ARMul_CP_DONE;
}

static void
run_cop7(WlSimState *state, ARMTime now)
{
	uint32 dt;

	dt = (uint32)(now - state->last_cycle);

	/* Speed up the application's sense of time by 'enhancing' the cycle count :-) */
	dt *= CYCLECOUNT_SPEEDUP;

	state->cop7.cyclecounter += dt;

	if (state->cop7.inttimer > 0) {
		if (dt < state->cop7.inttimer)
			state->cop7.inttimer -= dt;
		else {
			state->cop7.inttimer = 0;
			state->cop7.intstatus |= 1;
		}
	}
}

BEGIN_INIT(WlSim)
	Hostif_ConsolePrint(state->hostif, ", WlSim");

	{
		struct ARMul_CoprocessorV5 *cp = &state->cop7.v5;
		int err;
		char *fname;

		/*
		 * Open time stamp file for writing if DNGL_TS environment variable is
		 * set.  A line will be written to the file each time the program writes a
		 * value to the special timestamp address (CBALL_TIMESTAMP_ADDR),
		 * containing the current cycle count, program counter, and the value.
		 */

		if ((fname = getenv(TIMESTAMP_FILE_VAR)) == NULL)
			state->timestamp_fp = NULL;
		else if ((state->timestamp_fp = fopen(fname, "w")) == NULL) {
			fprintf(stderr, "WlSim: couldn't open timestamp file %s: %s\n",
			        fname, strerror(errno));
			exit(1);
		}

		/*
		 * Open instruction dump file for writing if DNGL_DUMP environment
		 * variable is set.  A line will be written to the file for each
		 * instruction executed, containing the current instruction count, program
		 * counter, current instruction, and current cycle count.
		 */

		if ((fname = getenv(INSTR_DUMP_FILE_VAR)) == NULL)
			state->dump_fp = NULL;
		else if ((state->dump_fp = fopen(fname, "w")) == NULL) {
			fprintf(stderr, "WlSim: couldn't open dump file %s: %s\n",
			        fname, strerror(errno));
			exit(1);
		}

		/*
		 * Open function trace file for writing if DNGL_TRACE environment variable
		 * is set.  A line will be written to the file each time the program
		 * counter changes into a different function, containing the current cycle
		 * count, current function name, and function offset.
		 */

		if ((fname = getenv(TRACE_FILE_VAR)) == NULL)
			state->trace_fp = NULL;
		else if ((state->trace_fp = fopen(fname, "w")) == NULL) {
			fprintf(stderr, "WlSim: couldn't open trace file %s: %s\n",
			        fname, strerror(errno));
			exit(1);
		}

		/*
		 * Open profile output file for writing if DNGL_PROF environment variable
		 * is set.  When Armulator exits or is interrupted, profiling information
		 * will be written to the file.
		 */

		if ((fname = getenv(PROFILE_FILE_VAR)) == NULL)
			state->profile_fp = NULL;
		else if ((state->profile_fp = fopen(fname, "w")) == NULL) {
			fprintf(stderr, "WlSim: couldn't open profile file %s: %s\n",
			        fname, strerror(errno));
			exit(1);
		}

		/*
		 * Read symbol table if function tracing or profiling is being used.
		 * (Currently there's no way to check that the symbol file is not out
		 * of date with respect to the ARM executable.)
		 */

		if ((fname = getenv(IMAGE_FILE_VAR)) == NULL)
			state->sym_tab = NULL;
		else {
			char cmd[256];
			FILE *fp;

			snprintf(cmd, sizeof(cmd), "arm-elf-nm %s", fname);

			if ((fp = popen(cmd, "r")) == NULL) {
				fprintf(stderr,
				        "WlSim: could not execute command %s: %s\n",
				        cmd, strerror(errno));
				exit(1);
			}

			state->sym_tab = sym_load(fp);

			pclose(fp);
		}

		if ((state->trace_fp != NULL || state->profile_fp != NULL) &&
		    state->sym_tab == NULL) {
			fprintf(stderr,
			        "WlSim: tracing or profiling requires %s to be set\n",
			        IMAGE_FILE_VAR);
			exit(1);
		}

		state->sym_last = NULL;

		/* Model initialization */

		if ((err = WlSim_SbInit(state)) != 0)
			return err;

		if ((err = WlSim_NvInit(state)) != 0)
			return err;

		if ((err = WlSim_DnInit(state)) != 0)
			return err;

		cp->CDP = NULL;		/* CDPHandler */
		cp->MCR = MCR7;		/* MCRHandler */
		cp->MRC = MRC7;		/* MRCHandler */
		cp->MCRR = NULL;	/* MCRRHandler */
		cp->MRRC = NULL;	/* MRRCHandler */
		cp->LDC = NULL;		/* LDCHandler */
		cp->STC = NULL;		/* STCHandler */
		cp->Desc = NULL;	/* ARMulCopro_DescFn */
		cp->ReadReg = NULL;	/* CPReadRegHandler */
		cp->WriteReg = NULL;	/* CPWriteRegHandler */

		if ((err = ARMulif_InstallCoprocessorV5(&state->coredesc, 7, cp, state)) != 0)
			return err;

		/*
		 * ADS 1.2.1 bug in ARMulif_GetCoreClockFreq(): returns 80000000 if
		 * Armulator is run as 'armsd -clock 80000000', but returns 80 if run
		 * as 'armsd -clock 80m'.  ARMul_GetMCLK() doesn't suffer this problem.
		 */
		state->hz = (uint)ARMul_GetMCLK(state->config);
		state->last_pc = 0xdeadbeef;
		state->last_cycle = 0;
		state->instr_count = 0;

		/* dp_init must come before chip_init() */
		if (dp_init() < 0) {
			printf("dnglarmulator: dp_init() failed\n");
			return RDIError_UnableToInitialise;
		}

		chip_init();
		sb_runcores();

		state->hourglass = ARMulif_InstallHourglass(&state->coredesc, Hourglass, state);
	}
END_INIT(WlSim)

#define CYCLE_TO_USEC(state, c)		((c) * 1000000 / (state)->hz)

/* The Hourglass function is called once per instruction */
static void
Hourglass(void *handle, ARMword pc, ARMword instr,
          ARMword cpsr, ARMword condpassed)
{
	WlSimState *state = handle;
	struct sym_ent *sym;
	ARMTime now;

	IGNORE(cpsr);
	IGNORE(condpassed);

	now = ARMulif_CoreCycles(&state->coredesc);

	dp_poll(CYCLE_TO_USEC(state, now));
	sb_runcores();
	run_cop7(state, now);
	set_irq(state);

	if (state->sym_tab != NULL && (sym = sym_lookup(state->sym_tab, (unsigned)pc)) != NULL) {
		/* Profile */
		if (state->sym_last)
			state->sym_last->cycles += (unsigned)(now - state->last_cycle);

		/* Function trace */
		if (state->trace_fp != NULL && sym != state->sym_last) {
			fprintf(state->trace_fp,
			        "%u %s 0x%x\n",
			        (unsigned)now, sym->name, (unsigned)pc - sym->addr);
		}

		state->sym_last = sym;
	}

	/* Instruction dump */
	if (state->dump_fp != NULL)
		fprintf(state->dump_fp,
		        "%lu %08x %08x %llu\n",
		        state->instr_count, (unsigned)pc, (unsigned)instr, now);

	state->last_pc = (uint)pc;
	state->last_cycle = now;
	state->instr_count++;

	armulator_last_pc = (uint)pc;

#ifdef PC_DUMP
	/*
	 * If an ARM program is hanging in a loop somewhere, displaying
	 * the PC periodically can help pinpoint where it is stuck.
	 */
	if (state->instr_count % 1000000 == 0)
		printf("T%lu: pc=%x\n", state->instr_count, state->last_pc);
#endif
}

BEGIN_EXIT(WlSim)
	if (state->hourglass)
		ARMulif_RemoveHourglass(&state->coredesc, state->hourglass);
	if (state->timestamp_fp != NULL)
		fclose(state->timestamp_fp);
	if (state->dump_fp != NULL)
		fclose(state->dump_fp);
	if (state->trace_fp != NULL)
		fclose(state->trace_fp);
	if (state->profile_fp != NULL) {
		sym_profdump(state->sym_tab, state->last_cycle, state->profile_fp);
		fclose(state->profile_fp);
	}
	if (state->sym_tab != NULL)
		sym_free(state->sym_tab);
END_EXIT(WlSim)

/* Trap SB memory accesses */
static int
WlSim_SbAccess(void *handle, struct ARMul_AccessRequest *req)
{
	ARMWord address = req->req_address[0];
	ARMWord *data = req->req_data;
	unsigned type = req->req_access_type;
	unsigned size = ACCESS_SIZE_IN_BYTES(type);
	WlSimState *state = (WlSimState *)handle;
	void *va;

	assert(address >= state->sb_bpar.range[0].lo &&
	       address <= state->sb_bpar.range[0].hi);

	if (!ACCESS_IS_MREQ(type) || size == 0)
		return PERIP_OK;

	switch (address) {
	case CBALL_CORE_CLOCK_ADDR:
		if (ACCESS_IS_READ(type))
			*data = (ARMWord)state->hz;
		break;
	case CBALL_CORE_CYCLE_LO_ADDR:
		if (ACCESS_IS_READ(type))
			*data = (ARMWord)(ARMulif_CoreCycles(&state->coredesc) & 0xffffffff);
		break;
	case CBALL_CORE_CYCLE_HI_ADDR:
		if (ACCESS_IS_READ(type))
			*data = (ARMWord)(ARMulif_CoreCycles(&state->coredesc) >> 32);
		break;
	case CBALL_CONNECT_HOST_ADDR:
		if (ACCESS_IS_READ(type)) {
#ifdef USB_DNGL
			*data = connect_usb_host();
#endif
#ifdef SDIO_DNGL
			*data = connect_sdio_host();
#endif
		}
		break;
	case CBALL_TIMESTAMP_ADDR:
		if (ACCESS_IS_WRITE(type) && size == 4 && state->timestamp_fp != NULL)
			fprintf(state->timestamp_fp, "%u 0x%x %u\n",
			        (unsigned)ARMulif_CoreCycles(&state->coredesc) & 0xffffffff,
			        state->last_pc,
			        (unsigned)*data);
		break;
	case CBALL_DN_ENABLE:
		/*
		 * Enable Detection of Null pointer dereferences.
		 * The ARM binary enable checking after it writes exception vectors.
		 */
		state->dn_enable = *data ? 1 : 0;
		break;
	default:
		if ((address >= DMA_REGION) &&
		    (address <= DMA_REGION + DMA_REGION_SIZE)) {
			extern int sb_addr_error;
			va = phys_to_virt((unsigned int *)address);
			if (sb_addr_error) {
				printf("WARNING: bad access address 0x%x size %d pc 0x%x\n",
				       (unsigned)address, size, state->last_pc);
			} else if (ACCESS_IS_WRITE(type))
				memcpy(va, (void *)data, size);
			else if (ACCESS_IS_READ(type)) {
				*data = 0;
				memcpy((void *)data, va, size);
			}
		} else {
			if (ACCESS_IS_WRITE(type))
				sbwrite(address, (unsigned char *)data, size);
			else if (ACCESS_IS_READ(type)) {
				*data = 0;
				sbread(address, (unsigned char *)data, size);
			}
		}
		break;
	}

	return PERIP_OK;
}

/* Trap nvram memory accesses */
static int
WlSim_NvAccess(void *handle, struct ARMul_AccessRequest *req)
{
	ARMWord address = req->req_address[0];
	ARMWord *data = req->req_data;
	unsigned type = req->req_access_type;
	unsigned size = ACCESS_SIZE_IN_BYTES(type);
	WlSimState *state = (WlSimState *)handle;

	assert(address >= state->nv_bpar.range[0].lo &&
	       address <= state->nv_bpar.range[0].hi);

	if (ACCESS_IS_MREQ(type) && size > 0) {
		if (ACCESS_IS_READ(type)) {
			uint32 off = address - NVRAM_BASE;
			assert(off + size <= NVRAM_SIZE);
			switch (size) {
			case 1:
				*data = state->nvram_buf[off];
				break;
			case 2:
				*data = (state->nvram_buf[off] |
				         state->nvram_buf[off + 1] << 8);
				break;
			case 4:
				*data = (state->nvram_buf[off] |
				         state->nvram_buf[off + 1] << 8 |
				         state->nvram_buf[off + 2] << 16 |
				         state->nvram_buf[off + 3] << 24);
				break;
			default:
				*data = 0;
			}
		}
	}

	return PERIP_OK;
}

static int
WlSim_DnAccess(void *handle, struct ARMul_AccessRequest *req)
{
	ARMWord address = req->req_address[0];
	ARMWord *data = req->req_data;
	unsigned type = req->req_access_type;
	unsigned size = ACCESS_SIZE_IN_BYTES(type);
	WlSimState *state = (WlSimState *)handle;

	assert(address >= state->dn_bpar.range[0].lo &&
	       address <= state->dn_bpar.range[0].hi);

	if (!ACCESS_IS_MREQ(type) || size == 0)
		return PERIP_OK;

	/* Normal treatment if Detect Null Pointer checking is off */
	if (!state->dn_enable)
		return PERIP_NODECODE;

	/* Normal treatment for instruction fetch of exception vector */
	if (!ACCESS_IS_DATA(type))
		return PERIP_NODECODE;

	/* Normal treatment for execution of exception vector */
	if (/* state->last_pc >= DN_BASE && */ state->last_pc < DN_BASE + DN_SIZE)
		return PERIP_NODECODE;

	/* Invalid data access */
	if (ACCESS_IS_WRITE(type))
		printf("WARNING: wrote address 0x%x size %d value 0x%x pc 0x%x\n",
		       (unsigned)address, size, (unsigned)*data, state->last_pc);
	else if (ACCESS_IS_READ(type))
		printf("WARNING: read address 0x%x size %d pc 0x%x\n",
		       (unsigned)address, size, state->last_pc);

	return PERIP_OK;
}

/* --- <SORDI STUFF>	--- */


#define SORDI_DLL_DESCRIPTION_STRING "SemiHosting operating system"
#define SORDI_RDI_PROCVEC WlSim_AgentRDI
#include "perip_sordi.h"

#include "perip_rdi_agent.h"
	IMPLEMENT_AGENT_PROCS_NOEXE_NOMODULE(WlSim)
	IMPLEMENT_AGENT_PROCVEC_NOEXE(WlSim)

/* --- </> --- */


/* EOF dnglarmulator.c */
