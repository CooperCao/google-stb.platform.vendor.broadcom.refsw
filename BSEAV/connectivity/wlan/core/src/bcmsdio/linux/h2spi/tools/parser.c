/*
 * SD-SPI parser for Total Phase Beagle SPI analyzer
 * Input is .csv file from Total Phase Data Center App
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

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * FBR is 0x0n00 - 0x0nFF, where is n is func num
 *
 *                        CIA F0
 *                      +--------+
 * 0x000000 - 0x0000FF  |  CCCR  |
 *                      +--------+
 * 0x000100 - 0x0001FF  | F1 FBR |
 *                      +--------+
 * 0x000200 - 0x0002FF  | F2 FBR |
 *                      +--------+
 *                          ....   
 *                      +--------+
 * 0x000700 - 0x0007FF  | F7 FBR |
 *                      +--------+
 *                         ....   
 *                      +--------+
 * 0x001000 - 0x017FFF  |  CIS   |
 *                      +--------+
 *
 *                       F1-F7
 *                       128K Reg Space
 *                      +--------+
 * 0x000000 - 0x01FFFF  |  CIS   |
 *                      +--------+
 */

typedef unsigned char u8_t;
typedef unsigned short u16_t;
typedef unsigned long u32_t;

/*
 * Debug
 */
#define COLS		16
#define DUMP_ERR	(g_dump & 0x01)
#define DUMP_CMD	(g_dump & 0x02)
#define DUMP_RSP	(g_dump & 0x04)
#define DUMP_RAW	(g_dump & 0x08)
#define DUMP_BLK_DAT	(g_dump & 0x10)
#define DUMP_WRSP	(g_dump & 0x20)
#define DUMP_SNOOP	(g_dump & 0x40)

#define pr_out(args)	do {printf args;} while(0)
#define pr_err(args)	do {if (DUMP_ERR) printf args;} while (0)
#define pr_raw(args)	do {if (DUMP_RAW) printf args;} while (0)
#define pr_cmd(args)	do {if (DUMP_CMD) printf args;} while (0)
#define pr_rsp(args)	do {if (DUMP_RSP) printf args;} while (0)
#define pr_blk(args)	do {if (DUMP_BLK_DAT) printf args;} while (0)
#define pr_wrsp(args)	do {if (DUMP_WRSP) printf args;} while (0)
#define pr_snoop(args)	do {if (DUMP_SNOOP) printf args;} while (0)

/*
 * Field Manipulations
 */
#define BITFIELD_MASK(width) \
		(((unsigned)1 << (width)) - 1)
#define GFIELD(val, field) \
		(((val) >> field ## _S) & field ## _M)
#define SFIELD(val, field, bits) \
		(((val) & (~(field ## _M << field ## _S))) | \
		 ((unsigned)(bits) << field ## _S))
#define SWAP(val) \
	((u32_t)((((u32_t)(val) & (u32_t)0x000000ffU) << 24) | \
		(((u32_t)(val) & (u32_t)0x0000ff00U) <<  8) | \
		(((u32_t)(val) & (u32_t)0x00ff0000U) >>  8) | \
		(((u32_t)(val) & (u32_t)0xff000000U) >> 24)))

/*
 * Brcm Defines
 */
#define F2WM		0x10008 /* F2 Water Mark */
#define DEVCTRL		0x10009
#define WIN1		0x1000a /* b15 */
#define WIN2		0x1000b /* b23-b16 */
#define WIN3		0x1000c /* b31-b24 */
#define SDIOFRCTRL	0x1000d
#define CHIP_CLK	0x1000e
#define SDIOPU		0x1000f
#define SDIOWRFRBCL	0x10019
#define SDIOWRFRBCH	0x1001a
#define SDIORDFRBCL	0x1001b
#define SDIORDFRBCH	0x1001c

/*
 * SPI defines
 */
#define R1_IDLE		0x01
#define R1_ERST		0x02
#define R1_ILL		0x04
#define R1_CRC		0x08
#define R1_ESEQ		0x10
#define R1_ADDR		0x20
#define R1_PARAM	0x40
#define R1_ST		0x80 /* zero */
#define R1_ERRS		(R1_ERST | R1_ILL | R1_CRC | R1_ESEQ | R1_ADDR | R1_PARAM)

#define R1MOD_IDLE	0x01
#define R1MOD_RFU1	0x02 /* zero */
#define R1MOD_ILL	0x04
#define R1MOD_CRC	0x08
#define R1MOD_FN	0x10
#define R1MOD_RFU2	0x20 /* zero */
#define R1MOD_PARAM	0x40
#define R1MOD_ST	0x80 /* zero */
#define R1MOD_ERRS	(R1MOD_PARAM | R1MOD_FN | R1MOD_CRC | R1MOD_ILL)

/* Write data response token:
 *    7 6 5 4  3 2 1  0
 *    x x x 0  stats  1
 */
#define WRSP_OK		0x2
#define WRSP_ECRC	0x5
#define WRSP_ERR	0x6
#define	WRSP_STATUS_M	BITFIELD_MASK(3)
#define	WRSP_STATUS_S	1

/* Read data response token:
 *    7 6 5 4  3 2 1 0
 *    0 0 0 0  status
 */
#define RRSP_ERR	0x1
#define RRSP_ECC	0x2
#define RRSP_EECC	0x4
#define RRSP_ERANGE	0x8

/* Start Token:
 *   Single block read, write, and multiple block read
 * Format:
 *   0xFE, 2 - 513 bytes, CRC16
 */
#define BLK_START	0xFE

/* Start Token:
 *   Multiple block write
 * Format:
 *   0xFC, 2 - 513 bytes, CRC16
 */
#define BLK_MSTART	0xFC
#define BLK_MSTOP	0xFD

/*
 * Cmd/Response lengths
 */
#define CMD_LEN		6
#define RSP_LEN		6

/*
 * CCCR fields
 */
#define CCCR_SDREV	0x00
#define CCCR_SPREV	0x01
#define CCCR_IOE	0x02
#define CCCR_IOR	0x03
#define CCCR_INTE	0x04
#define CCCR_INTP	0x05
#define CCCR_IOA	0x06
#define CCCR_BUSIF	0x07
#define CCCR_CC		0x08
#define CCCR_CIS1	0x09
#define CCCR_CIS2	0x0A
#define CCCR_CIS3	0x0B
#define CCCR_BUSSUP	0x0C
#define CCCR_FSEL	0x0D
#define CCCR_EFLG	0x0E
#define CCCR_RFLG	0x0F
#define CCCR_BLSZ1	0x10
#define CCCR_BLSZ2	0x11
#define CCCR_PWR	0x12
#define CCCR_HS		0x13

/*
 * FBR fields
 */
#define FBR_BLSZ1	0x10
#define FBR_BLSZ2	0x11

/*
 * Fields
 */
#define FUNC_M		BITFIELD_MASK(3)
#define FUNC_S		4
#define RDY_M		BITFIELD_MASK(1)
#define RDY_S		7
#define MEM_M		BITFIELD_MASK(1)
#define MEM_S		3
#define RW_M		BITFIELD_MASK(1)
#define RW_S		7
#define BLK_M		BITFIELD_MASK(1)
#define BLK_S		3
#define OPC_M		BITFIELD_MASK(1)
#define OPC_S		2

/*
 * Globals
 */
static u32_t g_dump = 0x47;
static u8_t g_BE = 0;
static u8_t g_mosi[2000];
static u8_t g_miso[2000];
static u8_t g_cmd[CMD_LEN];
static u8_t g_spi_rsp[RSP_LEN];
static u32_t g_base_curr = 0;
static u32_t g_base_addr = 0;
static u8_t *g_pbyte;

static int
is_BE()
{
	u32_t val = 0xbe00001e;
	u8_t *b = (u8_t *)&val;

	return (*b == 0xBE);
}

/*
 * Finds zero start bit to extract response
 */
static int
get_rsp(u8_t miso[], int len, u8_t rsp[])
{
	int j, err = -1;

        for (j = 0; j < len; j++) {
                if ((miso[j] & 0x80) != 0)
                        continue;
                        
		memcpy((void *)rsp, (void *)(miso + j), RSP_LEN);
		err = j+2; /* Skip over 2 byte resp */
		break;
	}

	return err;
}

/*
 * Finds zero start bit to extract cmd
 */
static int
get_cmd(u8_t mosi[], int len, u8_t cmd[])
{
	int j, err = -1;

	for (j = 0; j < len; j++) {
		if ((mosi[j] & 0x80) != 0)
			continue;

		memcpy((void *)cmd, (void *)(mosi + j), CMD_LEN);
		err = j+CMD_LEN; /* Skip over cmd */
		break;
	}

	return err;
}

/*
 * Write block data has response token to indicate status
 */
static int
get_wrsp(u8_t miso[], int len, int index, u8_t *wrsp)
{
        int j, err = 0;
	u8_t status;

	if (index < 0) {
		pr_err(("get_wrsp err = %d\n", index));
		return -1;
	}

        for (j = index; j < len; j++) {
                if ((miso[j] & 0x10) != 0)
                        continue;

		status = GFIELD(miso[j], WRSP_STATUS);

		*wrsp = status;
		err = (status != WRSP_OK);
		break;
        }

        return err;
}

/*
 * Format of block data for both read and write
 *   0xFE, 2 - 513 bytes, CRC16
 */
static int
get_blk(u8_t blk[], int len, int index, int cnt, u8_t **blkstart)
{
        int j, k;
	u32_t data;

        if ((index < 0) || (cnt == 0)) {
                pr_err(("get_blk err = %d %d\n", index, cnt));
                return -1;
        }

	for (j = index; j < len; j++) {
                if (blk[j] != BLK_START)
			continue;

		j++; /* Skip BLK_START */
		*blkstart = blk + j;
		break;
        }

        return 0;
}

/*
 * Dump read/write block data and print chip addr
 */
static int
dump_blk(u8_t *blkstart, int cnt)
{
	int k;

	pr_out(("DATA: "));
	/* cnt excludes 2 byte CRC */
	for (k = 0; k < cnt; k++) {
		if ((k % COLS) == 0)
			pr_out(("\n"));
		pr_out(("%02X ", *blkstart++));
	}
	pr_out(("\n"));
}


/*
 * CCCR addr Range: 0x0 - 0xFF
 */
static int
is_cccr_addr(u32_t addr)
{
	return (addr <= 0xFF); 
}

/*
 * Snoop F0 Card Common Control Registers (CCCR)
 */
static int
snoop_cccr(u32_t addr, u8_t rw, u8_t write, u8_t read)
{
	char *str;

	str = rw ? "W" : "R";
	switch(addr) {
	case CCCR_IOE:
		/* Write to enable FNs */
		pr_snoop(("\tIO Enable: 0x%02X\n", write));
		break;
	case CCCR_IOR:
		/* Polling for ready */
		pr_snoop(("\tIO Ready: 0x%02X\n", read));
		break;
	case CCCR_INTE:
		pr_snoop(("\tINT Enable: 0x%02X\n", write));
		break;
	case CCCR_BUSIF:
		pr_snoop(("\tBus IF: 0x%02X(%s)\n", rw ? write : read, str));
		break;
	case CCCR_HS:
		pr_snoop(("\tHS CTRL: 0x%02X(%s)\n", rw ? write : read, str));
		break;
	case CCCR_IOA:
		pr_snoop(("\tIO Abort: 0x%02X(%s)\n", rw ? write : read, str));
		break;
	case CCCR_INTP:
	case CCCR_SDREV:
	case CCCR_SPREV:
	case CCCR_CC:
	case CCCR_CIS1:
	case CCCR_CIS2:
	case CCCR_CIS3:
	case CCCR_BUSSUP:
	case CCCR_FSEL:
	case CCCR_EFLG:
	case CCCR_RFLG:
	case CCCR_BLSZ1:
	case CCCR_BLSZ2:
	case CCCR_PWR:
	default:
		pr_snoop(("\tCCCR: Unhandled addr:0x%x rw=%d write:0x%x read:0x%x\n",
			addr, rw, write, read));
		break;
	}

}

/*
 * FBR CIS pointer is 0x0n09 - 0x0n0B,
 *     where is n is func num
 */
static int
is_cis_addr(u32_t addr)
{
	return (((addr & 0x9) == 0x9) ||
		((addr & 0xa) == 0xa) ||
		((addr & 0xb) == 0xb));
}

/*
 * Snoop CIS addr
 */
static int
snoop_cis(u32_t addr, u8_t data)
{
	static u32_t cis_addr = 0;
	u8_t offset;
	u8_t fn;

	offset = addr & 0x0F;
	fn = (addr >> 8) & 0xF;

	/* Assumes 0x9-0xB arrive in order */
	switch(offset) {
	case 0x9:
		cis_addr = 0;
		g_pbyte = (u8_t*) &cis_addr;
		*g_pbyte++ = data;
		break;
	case 0xA:
		*g_pbyte++ = data;
		break;
	case 0xB:
		*g_pbyte++ = data;
		cis_addr = g_BE ? SWAP(cis_addr) : cis_addr;
		pr_snoop(("\tF%d CIS: 0x%x\n", fn, cis_addr));
		break;
	}
}

/*
 * FBR addr Range: 0x100 - 0x7FF
 */
static int
is_fbr_addr(u32_t addr)
{
	return ((addr >= 0x100) && (addr <= 0x7FF));
}

/*
 * Snoop F0 Card Common Control Registers (CCCR)
 */
static int
snoop_fbr(u32_t addr, u8_t rw, u8_t write, u8_t read)
{
	static u32_t fbr_blksize = 0;
	char *str;

        str = rw ? "W" : "R";
	switch(addr & 0xFF) {
	case FBR_BLSZ1:
		fbr_blksize = 0;
		g_pbyte = (u8_t*) &fbr_blksize;
		*g_pbyte++ = rw ? write : read;
		break;
	case FBR_BLSZ2:
		*g_pbyte++ = rw ? write : read;
		fbr_blksize = g_BE ? SWAP(fbr_blksize) : fbr_blksize;
		pr_snoop(("\tFBR: blksize=%d(%s)\n",
			fbr_blksize, str));
		break;
	default:
		pr_snoop(("\tFBR: Unhandled addr:0x%x rw=%d write:0x%x read:0x%x\n",
			addr, rw, write, read));
		break;
	}

	return 0;
}

/*
 * Snoop F0 activity
 */
static int
snoop_F0(u32_t addr, u8_t rw, u32_t write, u32_t read, int rsperr)
{
	u32_t val = rw ? write : read;

	/* Snoop CIS address */
	if (is_cis_addr(addr)) {
		snoop_cis(addr, val);
	/* Snoop CCCR */
	} else if (is_cccr_addr(addr))
		snoop_cccr(addr, rw, write, read);
	/* Snoop FBR */
	else if (is_fbr_addr(addr))
		snoop_fbr(addr, rw, write, read);

	return 0;
}

/* 
 * Brcm specific to snoop window base address to decode
 * actual read/write address
 */
static int
snoop_window(u32_t addr, u8_t data)
{
	/* Assumes WIN1-WIN3 arrive in order */
	switch(addr) {
	case WIN1:
		g_base_addr = 0;
		/* Start with LSB */
		g_pbyte = (u8_t*) &g_base_addr;
		g_pbyte++;
		*g_pbyte++ = data & 0x80;
		break;
	case WIN2:
		*g_pbyte++ = data;
		break;
	case WIN3:
		*g_pbyte++ = data;
		g_base_curr = g_BE ? SWAP(g_base_addr) : g_base_addr;
		pr_snoop(("\tnew base: 0x%08X\n",
			(unsigned int)g_base_addr));
	default:
		goto restart;
	}

	return 0;
restart:
	g_base_addr = 0;
	return 0;
}

/*
 * Snoop F1 activity
 */
static int
snoop_F1(u32_t addr, u8_t rw, u32_t write, u32_t read, int cnt, int rsperr)
{
	u32_t val = rw ? write : read;
	char *str = rw ? "W" : "R";

	/* Snoop window base address */
	if ((addr == WIN1) || (addr == WIN2) || (addr == WIN3)) {
		return snoop_window(addr, val);
	} else if (addr == CHIP_CLK) {
		pr_snoop(("\t%s CLK: 0x%x\n",
			rw ? "SET" : "GET", val));
		return 0;
	} else if (addr == SDIOPU) {
		pr_snoop(("\tF1 SDIO Pullup: 0x%x(%s)\n", val, str));
		return 0;
	} else if (addr == F2WM) {
		pr_snoop(("\tF1 F2Watermark: 0x%x(%s)\n", val, str));
		return 0;
	} else if (addr & 0x10000) {
		pr_snoop(("\tF1 unhandled: 0x%x(%s)\n", addr, str));
		return 0;
	}

	if (cnt == 4)
		pr_snoop(("\tF1 Reg Access: 0x%08x 0x%08x(%s)\n",
			(g_base_curr | (addr & 0x7FFF)), val, str));
	else
		pr_snoop(("\tF1 Blk Access: 0x%08x cnt=%d(%s)\n",
			(g_base_curr | (addr & 0x7FFF)), cnt, str));

	return 0;
}

/*
/*
 * R4 response
 * +---------------------------+
 * | MODR1 C FN MEM STUFF  OCR |
 * +---------------------------+
 * |   8   1 3   1    3    24  |
 * +---------------------------+
 */
static int
do_r4(u8_t rsp[])
{
	int j=0, err = 0;
	u8_t r1, rdy, fn, mem;
	u8_t ocr[3];

	r1 = rsp[j];

	rdy = GFIELD(rsp[j+1], RDY);
	fn = GFIELD(rsp[j+1], FUNC);
	mem = GFIELD(rsp[j+1], MEM);

	ocr[0] = rsp[j+2];
	ocr[1] = rsp[j+3];
	ocr[2] = rsp[j+4];

	pr_rsp(("R4: %02X %02X %02X %02X %02X\n", rsp[j],
			rsp[j+1], rsp[j+2], rsp[j+3], rsp[j+4]));

	pr_rsp(("R4: rdy=%d fns=%d mem=%d ocr=%02X%02X%02X\n", rdy, fn,
		mem, ocr[0], ocr[1], ocr[2])); 

	err = r1 & R1MOD_ERRS;
	return err;
}

static int
do_r1(u8_t rsp[])
{
	int j = 0, err = 0;
	u8_t r1, idle;

	r1 = rsp[j];
	pr_rsp(("R1: 0x%02X\n", r1));

	err = r1 & R1_ERRS;
	idle = r1 & R1_IDLE;
	return err;
}

/* 
 * R5 response
 * +--------------------------------+
 * | S PE RFU FE CE IE RFU IDLE DATA|
 * +--------------------------------+
 * | 1  1  1   1  1  1  1   1    8  |
 * +--------------------------------+
 * 
 * PE = param error
 * FE = func error
 * CE = CRC error
 * IE = ill cmd
 * RFU = always 0
 * idle = 1 is idle
 */
static int
do_r5(u8_t rsp[], u8_t rw, int rwlen, u8_t *data)
{
	int j = 0, err = 0;
	u8_t r1;
	u16_t r5;

	r1 = rsp[j];
	r5 = (r1 << 8) | rsp[j+1];

	err = r1 & R1MOD_ERRS;
	if (err == 0)
		*data = r5 & 0xff;

	return err;
}

/*
 * Process SD-SPI transaction
 */
static int
do_trans(u8_t mosi[], u8_t miso[], int len)
{
	int i, j, err, werr;
	u8_t cmd, rw, fn, block, opcode;
	u8_t crc, crc_on_off, wrsp_status;
	u8_t *blkstart;
	u16_t bytecnt;
	u8_t write, read;
	u32_t addroff, chipaddr, val = 0;
	int mosi_next = 0;
	int miso_next = 0;

	/* Save next to extract blk data later */
	mosi_next = get_cmd(mosi, len, g_cmd);
	miso_next = get_rsp(miso, len, g_spi_rsp);

	i = 0;
	cmd = g_cmd[i] & ~0xC0;

	if (DUMP_CMD) {
		pr_cmd(("CMD%d: ", cmd));
		for (j = 0; j < CMD_LEN; j++)
			pr_cmd(("%02X ", g_cmd[j]));
		pr_cmd(("\n"));
	}

	switch (cmd) {
		case 0:
			err = do_r1(g_spi_rsp);
			if (err)
				pr_err(("R1 ERROR: 0x%x\n", err));
			break;
		case 5:
			err = do_r4(g_spi_rsp);
			if (err)
				pr_err(("R4 ERROR: 0x%x\n", err));
			break;
		case 59: /* SPI CRC, off by default except for CMD0 */
			/*
			 * +--------------------------------+
			 * | S D CMD STUFF CRC_ON_OFF CRC E |
			 * +--------------------------------+
			 * | 1 1  6   31        1      7  1 |
			 * +--------------------------------+
			 * 0 : OFF
			 * 1 : ON
			 */
			crc_on_off = g_cmd[i+4] & 0x1;
			crc = g_cmd[i+5];
			pr_cmd(("CMD%d: crc_on_off:%d crc:0x%x\n", cmd, crc_on_off, crc));
			break;
		case 52:
			/*
			 * +--------------------------------------------+
			 * | S D CMD RW FN BLK OP ADDR STUFF DATA CRC E |
			 * +--------------------------------------------+
			 * | 1 1  6   1  3  1   1   17   1     8   7  1 |
			 * +--------------------------------------------+
			 */
			rw = GFIELD(g_cmd[i+1], RW);
			fn = GFIELD(g_cmd[i+1], FUNC);

			addroff = ((g_cmd[i+1] & 0x3) << 16) | (g_cmd[i+2] << 8) | (g_cmd[i+3] & ~0x1);
			addroff = addroff >> 1;
			write = g_cmd[i+4];
			crc = g_cmd[i+5];

			pr_cmd(("CMD%d: rw:%d fn:%d addr:0x%x data:0x%x crc:0x%x\n",
				cmd, rw, fn, (unsigned int)addroff, write, crc));

			err = do_r5(g_spi_rsp, rw, 1, &read);
			if (err) {
				pr_err(("R5 ERROR: 0x%x\n", err));
				break;
			}

			pr_rsp(("\t%s (R5): 0x%02X\n", (rw ? "WRITE" : "READ"), read));

			if (fn == 0)
				snoop_F0(addroff, rw, write, read, err);
			else if (fn == 1)
				snoop_F1(addroff, rw, write, read, 1, err);
			break;
		case 53:
			/*
			 * +-------------------------------------+
			 * | S D CMD RW FN BLK OP ADDR CNT CRC E |
			 * +-------------------------------------+
			 * | 1 1  6   1  3  1   1   17  9   7  1 |
			 * +-------------------------------------+
			 */
			rw = GFIELD(g_cmd[i+1], RW);
			fn = GFIELD(g_cmd[i+1], FUNC);
			block = GFIELD(g_cmd[i+1], BLK);
			opcode = GFIELD(g_cmd[i+1], OPC);


			addroff = ((g_cmd[i+1] & 0x3) << 16) | (g_cmd[i+2] << 8) | (g_cmd[i+3] & ~0x1);
			addroff = addroff >> 1;
			if (addroff & 0x10000) {
				chipaddr = addroff;
				pr_err(("Unexpected addr=0x%x for CMD53\n", addroff));
			} else
				chipaddr = g_base_curr | (addroff & 0x7FFF);

			bytecnt = ((g_cmd[i+3] & 0x1) << 8) | g_cmd[i+4];
			if ((block == 0) && (bytecnt == 0)) {
				bytecnt = 512;
			}

			crc = g_cmd[i+5];

			pr_cmd(("CMD%d: rw:%d fn:%d blk:%d op:%d addr:0x%x cnt:%d crc:0x%x\n",
				cmd, rw, fn, block, opcode,
				(unsigned int)addroff, bytecnt, crc));

			err = werr = 0;

			/* err is for cmd status; to check write blk status, need
			 * to query response token as well
			 */
			err = do_r5(g_spi_rsp, rw, bytecnt, &read);
			if (err) {
				pr_err(("R5 ERROR: 0x%x\n", err));
				break;
			}

			pr_rsp(("\t%s (R5): 0x%02X\n", (rw ? "WRITE" : "READ"), read));

			if (rw == 1) {
				werr = get_wrsp(miso, len, miso_next, &wrsp_status);
				if (werr != 0)
					pr_err(("Block write error\n"));

				pr_wrsp(("\tresp token: 0x%02X\n", wrsp_status));
			}

			if (rw == 1)
				get_blk(mosi, len, mosi_next, bytecnt, &blkstart);
			else
				get_blk(miso, len, miso_next, bytecnt, &blkstart);

			if (bytecnt == 4) {
				memcpy((void *)&val, (void *)blkstart, 4);
				val = g_BE ? SWAP(val) : val;
			}

			if (fn == 1)
				snoop_F1(addroff, rw, val, val, bytecnt, err);

       			if (DUMP_BLK_DAT)
				dump_blk(blkstart, bytecnt);
			break;
		default:
			pr_err(("CMD%d unhandled\n", cmd));	
	}

	return 0;
}

static void
usage(char *name)
{
	pr_out(("Usage: %s [-dh] file.csv\n", name));
	pr_out(("       -h, help\n"));
	pr_out(("       -d 0xff, dump output (0xff for all)\n"));
	pr_out(("          err   - 0x01\n"));
	pr_out(("          cmd   - 0x02\n"));
	pr_out(("          rsp   - 0x04\n"));
	pr_out(("          raw   - 0x08\n"));
	pr_out(("          blk   - 0x10\n"));
	pr_out(("          wrsp  - 0x20\n"));
	pr_out(("          snoop - 0x40\n"));
}

static u8_t g_line[10000];
int
main(int argc, char **argv)
{
	FILE *fp;
	char* fname;
	size_t len = 0;
	int read;
	char *instr;
	int words;
	u32_t lnum = 0;
	u32_t index, data, dbg;

	if (argc == 1) {
		usage(argv[0]);
		exit(0);
	} 

	if (strcmp(argv[1], "-h") == 0) {
		usage(argv[0]);
		exit(0);
	} else if ((strcmp(argv[1], "-d") == 0) && (argc == 4)) {
		g_dump = strtoul(argv[2], (char **)NULL, 16);
		fname = argv[3];
	} else if (argc == 2) {
		fname = argv[1];
	} else {
		usage(argv[0]);
		exit(0);
	}

	g_BE = is_BE();

	if ((fp = fopen(fname, "r")) == NULL) {
		fprintf(stderr, "ERR: unable to open file: %s\n", fname);
		exit(-1);
	}

	while (fgets(g_line, sizeof(g_line), fp) != NULL) {
		lnum++;
		len = strlen(g_line);

		if (len == (sizeof(g_line) - 1)) {
			fprintf(stderr, "ERR: Line %d too long %d\n",
				lnum, len);
			exit(-1);
		}

		if (strncmp(g_line, "#", 1) == 0)
			continue;

		if (strstr(g_line, "Transaction") == NULL)
			continue;

		/* Format: Index,m:s.ms.us.ns,Dur,Len,Err,Level,Record,Data */

		/* Skip over to length */
		instr = strtok(g_line, ","); /* Index */
		instr = strtok(NULL, ","); /* m:s.ms.us.ns */
		instr = strtok(NULL, ","); /* Dur */
		instr = strtok(NULL, ","); /* Len */

		words = atoi(instr);
		if (words == 0)
			continue;

		/* Skip over to data bytes: Err, Level, Record */
		while (instr != NULL) {
			instr = strtok(NULL, ",");
			if (strcmp(instr, "Transaction") == 0) {
				instr = strtok(NULL, ",");
				break;
			}
		}

		/* first word */
		instr = strtok(instr, " ");
		data = strtoul(instr, (char **)NULL, 16);

		index = 0;
		g_mosi[index] = (data >> 8) & 0xff;
		g_miso[index] = (data & 0xff);
		words--;
		index++;

		while (words--) {
			instr = strtok(NULL, " ");
			if (instr == NULL) {
				fprintf(stderr, "ERR: Unexpected end of line: %d %d line=%d\n",
					words, index, lnum);
				exit(-1);
			}
			data = strtoul(instr, (char **)NULL, 16);
			g_mosi[index] = (data >> 8) & 0xff;
			g_miso[index] = (data & 0xff);
			index++;
		}

		if (DUMP_RAW) {
			int i;

			pr_raw(("MOSI: "));
			for (i = 0; i < index; i++) {
				if ((i % COLS) == 0)
					pr_out(("\n"));
				pr_raw(("%02X ", g_mosi[i]));
			}
			pr_raw(("\n"));

			pr_raw(("MISO: "));
			for (i = 0; i < index; i++) {
				if ((i % COLS) == 0)
					pr_raw(("\n"));
				pr_raw(("%02X ", g_miso[i]));
			}
			pr_raw(("\n"));
		}

		do_trans(g_mosi, g_miso, index);
	}

	if (fp)
		fclose(fp);

	return 0;
}
