/*
 * Helper app to run time sensitive parts of software
 * based PAPD cal for tcl/epidiag
 *
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

#include <typedefs.h>
#include <bcmdefs.h>
#include <bcmdevs.h>
#include <osl.h>
#include <siutils.h>
#include <hndcpu.h>
#include <epivers.h>

#include <bcmendian.h>
#include <proto/802.11.h>
#include <sbhndpio.h>
#include <sbhnddma.h>
#include <hnddma.h>
#include <d11regs.h>
#include <hndsoc.h>

#include <rte.h>

void _c_main(unsigned long ra);

#define PHYREG_papdctrl		0x4d0
#define PHYREG_rfoverride3_val	0x4fa

static uint16
read_phy_reg(osl_t *osh, d11regs_t *regs, uint16 addr)
{
	W_REG(osh, &regs->phyregaddr, addr);

	return (R_REG(osh, &regs->phyregdata));
}

static void
write_phy_reg(osl_t *osh, d11regs_t *regs, uint16 addr, uint16 val)
{
	W_REG(osh, &regs->phyregaddr, addr);
	W_REG(osh, &regs->phyregdata, val);
}


void
_c_main(unsigned long ra)
{
	bool vsim, qt;
	si_t *sih;
	osl_t	*osh;
	char chn[8];
	d11regs_t	*d11;
	uint16 papdctrl, rfoverride3_val;
	static int tcl_xface = 0;

	/* Basic initialization */
	sih =	hnd_init();
	osh =	si_osh(sih);

	vsim = sih->chippkg == HDLSIM_PKG_ID;
	qt = sih->chippkg == HWSIM_PKG_ID;

	printf("%s ver %s compiled at %s on %s\n",
	       __FILE__, EPI_VERSION_STR, __TIME__, __DATE__);
	printf("Running on a %s%s Rev. %d @ %d/%d/%d/%d Mhz.\n",
	       bcm_chipname(sih->chip, chn, 8), vsim ? "(vsim)" : (qt ? "(qt)" : ""),
	       sih->chiprev, si_cpu_clock(sih) / 1000000, si_mem_clock(sih) / 1000000,
	       si_clock(sih) / 1000000, si_alp_clock(sih) / 1000000);

	d11 = (d11regs_t *)si_setcore(sih, D11_CORE_ID, 0);

	while (1) {
		/* wait for tcl to trigger cal */
		while (tcl_xface == 0) {
			OSL_DELAY(10);
		}

		/* Run the cal */
		papdctrl = read_phy_reg(osh, d11, PHYREG_papdctrl);
		rfoverride3_val = read_phy_reg(osh, d11, PHYREG_rfoverride3_val);
		write_phy_reg(osh, d11, PHYREG_rfoverride3_val, rfoverride3_val | 1);
		write_phy_reg(osh, d11, PHYREG_papdctrl, papdctrl | 1);
		while (read_phy_reg(osh, d11, PHYREG_papdctrl) & 1) {
			OSL_DELAY(1);
		}
		write_phy_reg(osh, d11, PHYREG_rfoverride3_val, rfoverride3_val & ~1);

		tcl_xface = 0;
	}

}
