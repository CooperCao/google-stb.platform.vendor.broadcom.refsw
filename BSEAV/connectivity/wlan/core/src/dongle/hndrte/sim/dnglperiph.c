/*
 * Part of Armulator shared object plug-in to emulate a full dongle plus
 * simulated wireless interface and simulated host bus interface
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>

#include <typedefs.h>
#include <sbconfig.h>
#include <sbhnddma.h>
#include <sbsdram.h>
#include <bcmendian.h>
#include <proto/802.11.h>

#include "traffic.h"
#include "sbsim.h"
#include "utils.h"

#include "dnglperiph.h"

#include "venetc.h"
#include "devbus.h"

extern void d11_sink(frame_sinkhdlr_t hdlr);
extern void usb_sink(frame_sinkhdlr_t hdlr);
extern void sdio_sink(frame_sinkhdlr_t hdlr);

extern bool is_ampdu_delimiter(struct lbuf *frame, uint32 *len);

extern int inject_d11_frame(unsigned char *data, int data_len);
extern int inject_usb_frame(unsigned char *data, int data_len);
extern int inject_sdio_frame(unsigned char *data, int data_len);

/*
 * VENET is a socket-based virtual ethernet for simulated wireless interface.
 * HOSTBUS is a socket-based virtual USB/SDIO interface.
 */

static venetc_t *vt;
static devbus_t *db;

static unsigned char *d11_msg;
static int d11_msglen;
static unsigned char *usb_msg;
static int usb_msglen;


static int
sink_d11(void *param, struct lbuf *frame)
{
	unsigned char *d, *e;
	int len, pktl;
	bool ampdu;
	int offset;
	uint8 tmp[14];

	d = lbdata(frame);
	len = lblen(frame);

	ampdu = is_ampdu_delimiter(frame, &pktl);



	if (vt == NULL) {
		printf("sink_d11: sunk frame length %d\n", len);
		return 0;
	}

	/*
	 * encapsulate 802.11 into Ethernet
	 *
	 * From:                    d+0  +2     +4  +10  +16    +18  +22  +24   +30
	 *                          FC   Durid  DA  SA   BSSID  .    Seq  SNAP  Type/len
	 *                                       .
	 * To:    e+0  +6   +12
	 *        DA   SA   0x9999  FC   Durid  DA  SA   BSSID  .    Seq  SNAP  Type/len
	 */
	e = lbpush(frame, 14);
	bcopy(e, tmp, 14);

	offset = 4 + (ampdu ? 4 : 0);
	bcopy(&d[offset], e, 12);
	e[12] = 0x99;
	e[13] = 0x99;

	venetc_send(vt, e, len + 14);

	bcopy(tmp, e, 14);
	lbpull(frame, 14);

	return 0;
}

static int
sink_usb(void *param, struct lbuf *frame)
{

	if (db == NULL) {
		printf("sink_usb: sunk frame length %d\n", lblen(frame));
		return 0;
	}

	devbus_output(db, lbdata(frame), lblen(frame));

	return 0;
}

static int
sink_sdio(void *param, struct lbuf *frame)
{

	if (db == NULL) {
		printf("sink_sdio: sunk frame length %d\n", lblen(frame));
		return 0;
	}

	devbus_output(db, lbdata(frame), lblen(frame));

	return 0;
}

int
dp_init(void)
{
	char *venet_hostname, *s;
	int venet_port;
	int dongle_port;

	/* Must do these before calling chip_init() */
	d11_sink(sink_d11);
	usb_sink(sink_usb);
	sdio_sink(sink_sdio);

	if ((venet_hostname = getenv("VENET_HOST")) != NULL) {
		if ((s = getenv("VENET_PORT")) == NULL) {
			printf("\ndp_init: environment variable VENET_PORT not set\n");
			return -1;
		}

		venet_port = (int)strtol(s, 0, 0);

		if ((vt = venetc_open(venet_hostname, venet_port)) == NULL) {
			printf("\ndp_init: error connecting to venet server %s:%d\n",
			       venet_hostname, venet_port);
			return -1;
		}

		printf("Connected to switch (local MAC %x:%x:%x:%x:%x:%x)\n",
		       vt->mac[0], vt->mac[1], vt->mac[2],
		       vt->mac[3], vt->mac[4], vt->mac[5]);
	}

	if ((s = getenv("DEVBUS_PORT")) == NULL)
		dongle_port = DEVBUS_PORT_DEFAULT;
	else
		dongle_port = (int)strtol(s, 0, 0);

	if ((db = devbus_attach(dongle_port)) == NULL) {
		printf("\ndp_init: error initializing device bus port\n");
		if (vt != NULL)
			venetc_close(vt);
		return -1;
	}

	return 0;
}

void
dp_poll(uint64 usec)
{
	/*
	 * Frames from network will be injected in the subsequent poll
	 * and the injection will be retried until it succeeds, to cope
	 * with DMA descriptor underrun.
	 */
	if (d11_msglen > 0 && inject_d11_frame(d11_msg, d11_msglen) >= 0) {
		venetc_free(vt, d11_msg);
		d11_msg = NULL;
		d11_msglen = 0;
	}

	if (d11_msglen <= 0 && vt != NULL) {
		struct timeval tv;
		fd_set rfds;

		tv.tv_sec = tv.tv_usec = 0;

		FD_ZERO(&rfds);
		FD_SET(vt->serv_fd, &rfds);

		if (select(vt->serv_fd + 1, &rfds, 0, 0, &tv) < 0) {
			/* It's non-fatal if select is interrupted by SIGALARM, etc. */
			if (errno != EINTR && errno != ERESTART) {
				printf("dp_poll: venet server select error: %s\n",
				       strerror(errno));
				venetc_close(vt);
				vt = NULL;
			}
		} else if (FD_ISSET(vt->serv_fd, &rfds)) {
			if (venetc_recv(vt, &d11_msg, &d11_msglen) <= 0) {
				printf("dp_poll: venet server read error\n");
				venetc_close(vt);
				vt = NULL;
			}
		}
	}

	/*
	 * Frames from bus will be injected in the subsequent poll and
	 * the injection will be retried until it succeeds, to cope with
	 * DMA descriptor underrun.
	 */
	if (usb_msglen > 0 && inject_usb_frame(usb_msg, usb_msglen) >= 0) {
		free(usb_msg);
		usb_msg = NULL;
		usb_msglen = 0;
	}

	if (usb_msglen <= 0 && db != NULL) {
		struct timeval tv;

		tv.tv_sec = tv.tv_usec = 0;

		if ((usb_msglen = devbus_read(db, &usb_msg, &tv)) < 0) {
			printf("dp_poll: devbus client read error\n");
			devbus_close(db);
			db = NULL;
		}
	}
}
