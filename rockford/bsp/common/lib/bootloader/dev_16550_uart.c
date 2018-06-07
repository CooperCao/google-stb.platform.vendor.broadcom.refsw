/******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 ******************************************************************************/
#include "bolt.h"
#include "lib_types.h"
#include "lib_malloc.h"
#include "lib_printf.h"
#include "iocb.h"
#include "device.h"
#include "ioctl.h"

#include "bsp_config.h"
#include "bchp_uarta.h"
#include "bchp_common.h"

/* Rx status register */
#define RXRDA          0x01
#define RXOVFERR       0x02
#define RXPARERR       0x04
#define RXFRAMERR      0x08

#define THRE				0x20
#define UART_SDW_RBR		0x00
#define UART_SDW_THR		0x00
#define UART_SDW_DLL		0x00
#define UART_SDW_DLH		0x04
#define UART_SDW_IER		0x04
#define UART_SDW_IIR		0x08
#define UART_SDW_FCR		0x08
#define UART_SDW_LCR		0x0c
#define UART_SDW_MCR		0x10
#define UART_SDW_LSR		0x14
#define UART_SDW_MSR		0x18
#define UART_SDW_SCR		0x1c

#define WRITEREG(reg, val) (*((volatile unsigned int *)(BCHP_PHYSICAL_OFFSET | (reg))) = val)
#define READREG(reg) *((volatile unsigned int *)(BCHP_PHYSICAL_OFFSET | (reg)))

#if 0
static void bcm97XXX_uart_probe(bolt_driver_t *drv,
				unsigned long probe_a, unsigned long probe_b,
				void *probe_ptr);
#endif

int bcm97XXX_uart_open(bolt_devctx_t *ctx);
int bcm97XXX_uart_read(bolt_devctx_t *ctx, iocb_buffer_t *buffer);
int bcm97XXX_uart_inpstat(bolt_devctx_t *ctx, iocb_inpstat_t *inpstat);
//int bcm97XXX_uart_write(unsigned char *bptr, int blen);
int bcm97XXX_uart_ioctl(bolt_devctx_t *ctx, iocb_buffer_t *buffer);
int bcm97XXX_uart_close(bolt_devctx_t *ctx);

#if 0
static const bolt_devdisp_t bcm97XXX_uart_dispatch = {
	bcm97XXX_uart_open,
	bcm97XXX_uart_read,
	bcm97XXX_uart_inpstat,
	bcm97XXX_uart_write,
	bcm97XXX_uart_ioctl,
	bcm97XXX_uart_close,
	NULL,
	NULL
};

const bolt_driver_t bcm97XXX_uart = {
	"16550 DUART",
	"uart",
	BOLT_DEV_SERIAL,
	&bcm97XXX_uart_dispatch,
	bcm97XXX_uart_probe
};

struct bcm97XXX_uart_s {
	int baudrate;

	volatile uint32_t *rxstat;
	volatile uint32_t *rxdata;
	volatile uint32_t *txstat;
	volatile uint32_t *txdata;
};

static void bcm97XXX_set_baudrate(struct bcm97XXX_uart_s *softc)
{
	return;
}

static void bcm97XXX_uart_probe(bolt_driver_t *drv,
				unsigned long probe_a, unsigned long probe_b,
				void *probe_ptr)
{
	struct bcm97XXX_uart_s *softc;
	char descr[80];

	/* enable the transmitter interrupt? */

	/*
	 * probe_a is the DUART base address.
	 * probe_b is the channel-number-within-duart (0 or 1)
	 * probe_ptr is unused.
	 */
	softc = (struct bcm97XXX_uart_s *)KMALLOC(
			sizeof(struct bcm97XXX_uart_s), 0);
	if (softc) {
		softc->rxstat = (uint32_t *) (probe_a + UART_SDW_LSR);
		softc->rxdata = (uint32_t *) (probe_a + UART_SDW_RBR);
		softc->txstat = (uint32_t *) (probe_a + UART_SDW_LSR);
		softc->txdata = (uint32_t *) (probe_a + UART_SDW_THR);
		xsprintf(descr, "%s at %#lx channel %lu", drv->drv_description,
			 probe_a, probe_b);
		bolt_attach(drv, softc, NULL, descr);
	}
}

static int bcm97XXX_uart_open(bolt_devctx_t *ctx)
{
	struct bcm97XXX_uart_s *softc = ctx->dev_softc;

	softc->baudrate = CFG_SERIAL_BAUD_RATE;
	bcm97XXX_set_baudrate(softc);

	WRITEREG(softc->rxstat, 0x0);
	WRITEREG(softc->txstat, 0x0);

	return 0;
}
#endif

int bcm97XXX_uart_read(bolt_devctx_t *ctx, iocb_buffer_t *buffer)
{
	unsigned char *bptr;
	int blen;
	uint32_t status;

        (void)ctx;
	bptr = buffer->buf_ptr;
	blen = buffer->buf_length;

	while (blen > 0) {
		status = READREG(BCHP_UARTA_LSR);
		if (status & (RXOVFERR | RXPARERR | RXFRAMERR)) {
			/* Just read the bad character to clear the bit. */
			READREG(BCHP_UARTA_RBR);
		} else if (status & RXRDA) {
			*bptr++ = READREG(BCHP_UARTA_RBR) & 0xFF;
			blen--;
		} else
			break;
	}

	buffer->buf_retlen = buffer->buf_length - blen;
	return 0;
}

#if 0
static int bcm97XXX_uart_inpstat(bolt_devctx_t *ctx, iocb_inpstat_t *inpstat)
{
	struct bcm97XXX_uart_s *softc = ctx->dev_softc;

	inpstat->inp_status = (READREG(softc->rxstat) & RXRDA) ? 1 : 0;

	return 0;
}
#endif

int bcm97XXX_uart_write(char *bptr, int blen)
{
	int blen2;
	uint32_t status;

        blen2 = blen;

	while (blen > 0) {
		do {
			status = READREG(BCHP_UARTA_LSR) & THRE;
		} while (!status);
		WRITEREG(BCHP_UARTA_THR, *bptr);

		bptr++;
		blen--;
	}

	return blen2;
}

#if 0
static int bcm97XXX_uart_ioctl(bolt_devctx_t *ctx, iocb_buffer_t *buffer)
{
	struct bcm97XXX_uart_s *softc = ctx->dev_softc;
	unsigned int *info = (unsigned int *)buffer->buf_ptr;

	switch ((int)buffer->buf_ioctlcmd) {
	case IOCTL_SERIAL_GETSPEED:
		*info = softc->baudrate;
		break;
	case IOCTL_SERIAL_SETSPEED:
		softc->baudrate = *info;
		bcm97XXX_set_baudrate(softc);
		break;
	case IOCTL_SERIAL_GETFLOW:
		*info = SERIAL_FLOW_HARDWARE;
		break;
	case IOCTL_SERIAL_SETFLOW:
	/* Fall through */
	default:
		return -1;
	}

	return 0;
}

static int bcm97XXX_uart_close(bolt_devctx_t *ctx)
{
	return 0;
}
#endif
