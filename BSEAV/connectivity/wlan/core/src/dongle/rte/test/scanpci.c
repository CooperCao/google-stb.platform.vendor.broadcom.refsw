/*
 * Initialization and support routines for self-booting
 * compressed image.
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
#include <osl.h>
#include <hndsoc.h>
#include <bcmdevs.h>
#include <hndcpu.h>
#include <hndpci.h>
#include <pcicfg.h>
#include <siutils.h>
#include <epivers.h>
#include <bcmutils.h>

#include <rte.h>

void _c_main(unsigned long ra);

static si_t *sih;


void
_c_main(unsigned long ra)
{
	uint slot;
	uint32 w1, w2;
	pci_config_regs *pcr;
	bool vsim, qt;
	char chn[8];

	/* Basic initialization */
	sih = hnd_init();

	vsim = sih->chippkg == HDLSIM_PKG_ID;
	qt = sih->chippkg == HWSIM_PKG_ID;

#ifndef	RTE_UNCACHED
	/* Initialize and turn caches on */
	caches_on();
#endif

	printf("%s ver %s compiled at %s on %s\n",
	       __FILE__, EPI_VERSION_STR, __TIME__, __DATE__);
	printf("Running on a %s%s Rev. %d @ %d/%d/%d/%d Mhz.\n",
	       bcm_chipname(sih->chip, chn, 8), vsim ? "(vsim)" : (qt ? "(qt)" : ""),
	       sih->chiprev, si_cpu_clock(sih) / 1000000, si_mem_clock(sih) / 1000000,
	       si_clock(sih) / 1000000, si_alp_clock(sih) / 1000000);

	pcr = hnd_malloc(SZPCR);
	for (slot = 0; slot < PCI_MAX_DEVICES; slot++) {
		uint32 *p;
		uint i;

		extpci_read_config(sih, 1, slot, 0, PCI_CFG_VID, pcr, 4);
		if (pcr->vendor == PCI_INVALID_VENDORID)
			continue;
		for (p = (uint32 *)((uint)pcr + 4), i = 4; i < SZPCR; p++, i += 4)
			extpci_read_config(sih, 1, slot, 0, i, p, 4);
		printf("DEBUG: pcr @ 0x%p\n", pcr);
		printf("Slot %d:\n  ven/dev: 0x%04x:0x%04x\n", slot, pcr->vendor, pcr->device);
		printf("  ssv/ssd: 0x%04x:0x%04x\n", pcr->subsys_vendor, pcr->subsys_id);
		printf("  cmd/sta: 0x%04x:0x%04x\n", pcr->command, pcr->status);
		printf("  rev: %d, class: 0x%02x-0x%02x-0x%02x\n", pcr->rev_id,
		       pcr->base_class, pcr->sub_class, pcr->prog_if);
		printf("  clsz: %d, ltim: %d, hdr: 0x%02x, bist: 0x%02x\n", pcr->cache_line_size,
		       pcr->latency_timer, pcr->header_type, pcr->bist);
		printf("  bar0: 0x%08x, bar1: 0x%08x, bar2: 0x%08x, bar3: 0x%08x\n", pcr->base[0],
		       pcr->base[1], pcr->base[2], pcr->base[3]);
		printf("  intl: %d, intp: %d, ming: 0x%02x, maxl: 0x%02x\n", pcr->int_line,
		       pcr->int_pin, pcr->min_gnt, pcr->max_lat);
		printf("  b0w: 0x%08x, b1w: 0x%08x, srom: 0x%08x, b1ctl: 0x%08x\n",
		       *(uint32 *)&pcr->bar0_window, *(uint32 *)&pcr->bar1_window,
		       *(uint32 *)&pcr->sprom_control, *(uint32 *)&pcr->dev_dep[0x8c - 0x40]);

		extpci_read_config(sih, 1, slot, 0, PCI_BAR0_WIN, &w1, 4);
		if (w1 != SI_ENUM_BASE(NULL)) {
			w2 = SI_ENUM_BASE(NULL);
			printf("  setting bar0window to 0x%08x\n", w2);
			extpci_write_config(sih, 1, slot, 0, PCI_BAR0_WIN, &w2, 4);
			extpci_read_config(sih, 1, slot, 0, PCI_BAR0_WIN, &w1, 4);
		}
		if (w1 != SI_ENUM_BASE(NULL)) {
			printf("Cannot set bar0window: 0x%08x should be 0x%08x\n", w1, w2);
			continue;
		}
		w1 = SI_PCI_MEM | ((pcr->base[0] & 0xfffffff0) - 0x20000000);
		printf("  attempting to read offset 0 of bar0(0x%08x): ", w1);
		if (BUSPROBE(w2, OSL_UNCACHED(w1)) != 0)
			printf("failed\n");
		else
			printf("0x%08x\n", w2);
		printf("  attempting to read offset 0xffc ");
		if (BUSPROBE(w2, OSL_UNCACHED(w1 + 0xffc)) != 0)
			printf("failed\n");
		else
			printf("0x%08x\n", w2);
	}
}
