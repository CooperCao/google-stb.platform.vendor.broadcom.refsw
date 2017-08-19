/*
 * SDIO/PCMCIA device API
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

#ifndef	_sdpcmdev_h_
#define	_sdpcmdev_h_

#include <typedefs.h>
#include <osl.h>
#include <bcmutils.h>

struct sdpcmd;

/* Chip operations */
extern struct dngl_bus *sdpcmd_attach(void *drv, uint vendor, uint device, osl_t *osh,
                                      volatile void *regs, uint bus);
extern void sdpcmd_detach(struct dngl_bus *sdpcmd);
extern void sdpcmd_init(struct dngl_bus *sdpcmd);
extern bool sdpcmd_dispatch(struct dngl_bus *sdpcmd);
extern bool sdpcmd_dpc(struct dngl_bus *sdpcmd);
extern void sdpcmd_intrsoff(struct dngl_bus *sdpcmd);
extern void sdpcmd_intrs_deassert(struct dngl_bus *sdpcmd);
extern void sdpcmd_intrson(struct dngl_bus *sdpcmd);
extern void sdpcmd_intrsupd(struct dngl_bus *sdpcmd);
extern void sdpcmd_dumpregs(struct dngl_bus *sdpcmd, struct bcmstrbuf *b);
extern int sdpcmd_loopback(struct dngl_bus *sdpcmd, char *buf, uint count);
#ifdef BCMDBG
extern int sdpcmd_transmit(struct dngl_bus *sdpcmd, char *buf, uint count, uint clen, bool ctl);
#endif /* BCMDBG */
extern void *sdpcmd_dngl(struct dngl_bus *sdpcmd);

extern bool sdpcmd_sdioaccess(int *data, uint16 regspace, uint8 addr, uint32 accesstype,
		osl_t *osh, volatile void *sdio_regs);
/* Bus API operations */
void sdpcmd_bus_softreset(struct dngl_bus *bus);
int sdpcmd_bus_binddev(void *bus, void *dev, uint numslaves);
void sdpcmd_bus_rebinddev(void *bus, void *dev, int ifindex);
int sdpcmd_bus_unbinddev(void *bus, void *dev);
int sdpcmd_bus_tx(struct dngl_bus *bus, void *p);
void sdpcmd_bus_sendctl(struct dngl_bus *bus, void *p);
void sdpcmd_bus_rxflowcontrol(struct dngl_bus *bus, bool state, int prio);
void sdpcmd_set_maxtxpktglom(struct dngl_bus *bus, uint8 txpktglom);
uint32 sdpcmd_bus_iovar(struct dngl_bus *sdpcmd, char *buf,
                        uint32 inlen, uint32 *outlen, bool set);
void sdpcmd_bus_resume(struct dngl_bus *bus);
void sdpcmd_bus_pr46794WAR(struct dngl_bus *bus);

#ifdef BCMDBG
void sdpcmd_bus_dumpregs(void);
void sdpcmd_bus_loopback(void);
void sdpcmd_bus_xmit(int len, int clen, bool ctl);
uint sdpcmd_bus_msgbits(uint *newbits);
#endif

#ifdef DS_PROT
#ifdef BCMDBG
extern void sdpcmd_ds_log(uint16 type, uint8 a, uint8 b, uint8 c);
#endif /* BCMDBG */
extern void _sdpcmd_ds_log(struct dngl_bus *sdpcmd, uint16 type,
	uint state, uint event, uint nextstate);

#define SDPCM_DS_LOG_TYPE_NONE 0
#define SDPCM_DS_LOG_TYPE_FSMT 1
#define SDPCM_DS_LOG_TYPE_ACKWTIM 2
#define SDPCM_DS_LOG_TYPE_GP0M3_SLP 3
#define SDPCM_DS_LOG_TYPE_GP0M3_WAK 4
#define SDPCM_DS_LOG_TYPE_GP0M3_ACK 5
#define SDPCM_DS_LOG_TYPE_HWAIT_TMR 6
#define SDPCM_DS_LOG_TYPE_TMO_NODS 7
#define SDPCM_DS_LOG_TYPE_TMO_DS 8
#define SDPCM_DS_LOG_TYPE_TMO_RETRY 9
#define SDPCM_DS_LOG_TYPE_FORCE_WAK 10
#define SDPCM_DS_LOG_TYPE_FORCE_SLP 11
#define SDPCM_DS_LOG_TYPE_FSM_IGNORE 12
#define SDPCM_DS_LOG_TYPE_NODS_D3_DW 13
#define SDPCM_DS_LOG_TYPE_DEBUG2 14
#define SDPCM_DS_LOG_TYPE_MBOX_RX 15
#define SDPCM_DS_LOG_TYPE_MBOX_TX 16
#define SDPCM_DS_LOG_TYPE_MISSED_PULSE_D 17
#define SDPCM_DS_LOG_TYPE_MISSED_PULSE_A 18
#define SDPCM_DS_LOG_TYPE_DW_CANNOT_DS 19
#define SDPCM_DS_LOG_TYPE_DW_END_DSCHK 20
#define SDPCM_DS_LOG_TYPE_DW_END_HWAIT 21
#define SDPCM_DS_LOG_TYPE_SCAN_RESULT 22
#define SDPCM_DS_LOG_TYPE_TXDMA_FAIL 23
#define SDPCM_DS_LOG_TYPE_TXMB_QUEUED 24
#define SDPCM_DS_LOG_TYPE_TXMB_QSEND 25
#define SDPCM_DS_LOG_TYPE_TXMB_OVFLOW 26
#define SDPCM_DS_LOG_TYPE_COUNTERS 27
#define SDPCM_DS_LOG_TYPE_WAKE_ISR 28
#define SDPCM_DS_LOG_TYPE_CC_ISR 29
#define SDPCM_DS_LOG_TYPE_HOSTWAKE 30	/* HOST_WAKE GPIO toggle */
#define SDPCM_DS_LOG_TYPE_UNUSED0 31
#define SDPCM_DS_LOG_TYPE_UNUSED1 32
#define SDPCM_DS_LOG_TYPE_IOCEND 33	/* end ioctl processing, sent response */
#define SDPCM_DS_LOG_TYPE_UNUSED2 34
#define SDPCM_DS_LOG_TYPE_IOVEND 35	/* end iovar processing, sent response */
#endif /* DS_PROT */

/* WL_ENAB_RUNTIME_CHECK may be set based upon the #define below (for ROM builds). It may also
 * be defined via makefiles (e.g. ROM auto abandon unoptimized compiles).
 */

#ifdef DS_PROT
/* Deep Sleep protocol */
#if defined(WL_ENAB_RUNTIME_CHECK)
	#define DS_PROT_ENAB(sdpcmd)	((sdpcmd)->_ds_prot)
#elif defined(DS_PROT_DISABLED)
	#define DS_PROT_ENAB(pub)	(0)
#else
	#define DS_PROT_ENAB(pub)	(1)
#endif
#else /* DS_PROT */
	#define DS_PROT_ENAB(pub)	(0)
#endif /* DS_PROT */

#endif	/* _sdpcmdev_h_ */
