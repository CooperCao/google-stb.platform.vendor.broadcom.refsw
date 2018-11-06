/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and Broadcom
 * expressly reserves all rights in and to the Software and all intellectual
 * property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use all
 *    reasonable efforts to protect the confidentiality thereof, and to use
 *    this information only in connection with your use of Broadcom integrated
 *    circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 *    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
 *    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
 *    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/
 
#include <typedefs.h>
#include <bcmendian.h>
#include <linuxver.h>
#include <bcmdefs.h>
#include <osl.h>
#include <osl_decl.h>
#include <linux_osl_priv.h>

void osl_set_bus_handle(osl_t *osh, void *bus_handle)
{
	osh->bus_handle = bus_handle;
}
EXPORT_SYMBOL(osl_set_bus_handle);
void* osl_get_bus_handle(osl_t *osh)
{
	return osh->bus_handle;
}
EXPORT_SYMBOL(osl_get_bus_handle);

#if defined(BCM_BACKPLANE_TIMEOUT)
void osl_set_bpt_cb(osl_t *osh, void *bpt_cb, void *bpt_ctx)
{
	if (osh) {
		osh->bpt_cb = (bpt_cb_fn)bpt_cb;
		osh->sih = bpt_ctx;
	}
}
EXPORT_SYMBOL(osl_set_bpt_cb);

inline void osl_bpt_rreg(osl_t *osh, ulong addr, volatile void *v, uint size)
{
	bool poll_timeout = FALSE;
	static int in_si_clear = FALSE;
	switch (size) {
	case sizeof(uint8):
		*(volatile uint8*)v = readb((volatile uint8*)(addr));
		if (*(volatile uint8*)v == 0xff)
			poll_timeout = TRUE;
		break;
	case sizeof(uint16):
		*(volatile uint16*)v = readw((volatile uint16*)(addr));
		if (*(volatile uint16*)v == 0xffff)
			poll_timeout = TRUE;
		break;
	case sizeof(uint32):
		*(volatile uint32*)v = readl((volatile uint32*)(addr));
		if (*(volatile uint32*)v == 0xffffffff)
			poll_timeout = TRUE;
		break;
	case sizeof(uint64):
		*(volatile uint64*)v = *((volatile uint64*)(addr));
		if (*(volatile uint64*)v == 0xffffffffffffffff)
			poll_timeout = TRUE;
		break;
	}

	if (osh && osh->sih && (in_si_clear == FALSE) && poll_timeout && osh->bpt_cb) {
		in_si_clear = TRUE;
		osh->bpt_cb((void *)osh->sih, (void *)addr);
		in_si_clear = FALSE;
	}
}
EXPORT_SYMBOL(osl_bpt_rreg);
#endif	/* BCM_BACKPLANE_TIMEOUT */

/* APIs to set/get specific quirks in OSL layer */
void BCMFASTPATH
osl_flag_set(osl_t *osh, uint32 mask)
{
	osh->flags |= mask;
}
EXPORT_SYMBOL(osl_flag_set);

void
osl_flag_clr(osl_t *osh, uint32 mask)
{
	osh->flags &= ~mask;
}
EXPORT_SYMBOL(osl_flag_clr);

inline bool BCMFASTPATH
osl_is_flag_set(osl_t *osh, uint32 mask)
{
	return (osh->flags & mask);
}
EXPORT_SYMBOL(osl_is_flag_set);

uint32
osl_pci_read_config(osl_t *osh, uint offset, uint size)
{
	ASSERT(osh && (osh->magic == OS_HANDLE_MAGIC) && osh->pdev);
	return 0;
}
EXPORT_SYMBOL(osl_pci_read_config);

void
osl_pci_write_config(osl_t *osh, uint offset, uint size, uint val)
{
	ASSERT(osh && (osh->magic == OS_HANDLE_MAGIC) && osh->pdev);
}
EXPORT_SYMBOL(osl_pci_write_config);

/* return bus # for the pci device pointed by osh->pdev */
uint
osl_pci_bus(osl_t *osh)
{
	ASSERT(osh && (osh->magic == OS_HANDLE_MAGIC) && osh->pdev);
	return 0;
}
EXPORT_SYMBOL(osl_pci_bus);

/* return slot # for the pci device pointed by osh->pdev */
uint
osl_pci_slot(osl_t *osh)
{
	ASSERT(osh && (osh->magic == OS_HANDLE_MAGIC) && osh->pdev);
	return 0;
}
EXPORT_SYMBOL(osl_pci_slot);

/* return domain # for the pci device pointed by osh->pdev */
uint
osl_pcie_domain(osl_t *osh)
{
	ASSERT(osh && (osh->magic == OS_HANDLE_MAGIC) && osh->pdev);

	return 0;
}
EXPORT_SYMBOL(osl_pcie_domain);

/* return bus # for the pci device pointed by osh->pdev */
uint
osl_pcie_bus(osl_t *osh)
{
	ASSERT(osh && (osh->magic == OS_HANDLE_MAGIC) && osh->pdev);

	return 0;
}
EXPORT_SYMBOL(osl_pcie_bus);

/* return the pci device pointed by osh->pdev */
struct pci_dev *
osl_pci_device(osl_t *osh)
{
	ASSERT(osh && (osh->magic == OS_HANDLE_MAGIC) && osh->pdev);

	return NULL;
}
EXPORT_SYMBOL(osl_pci_device);

void
osl_pcmcia_read_attr(osl_t *osh, uint offset, void *buf, int size)
{
}
EXPORT_SYMBOL(osl_pcmcia_read_attr);

void
osl_pcmcia_write_attr(osl_t *osh, uint offset, void *buf, int size)
{
}
EXPORT_SYMBOL(osl_pcmcia_write_attr);

